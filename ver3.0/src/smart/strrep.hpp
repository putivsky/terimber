/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2009 The Terimber Corporation. All rights reserved.
 * =================================================================================
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * The end-user documentation included with the redistribution, if any, 
 * must include the following acknowledgment:
 * "This product includes software developed by the Terimber Corporation."
 * =================================================================================
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL THE TERIMBER CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ================================================================================
*/

#ifndef _terimber_strrep_hpp_
#define _terimber_strrep_hpp_

#include "strrep.h"

BEGIN_TERIMBER_NAMESPACE

// class supports string memory management
// allocated block will have 4 hidden leading bytes 
// 1st byte = backet number [4-12], 2nd - byte = offset/state [0-255], 3rd-4th bytes = page number [0-64K], 
// for allocation out of backet

template < unsigned char N >
backet_tank< N >::backet_tank(byte_allocator& all) : _states(0)
{
	_head = (char*)all.allocate(ROOMS * LENGTH);  
}

template < unsigned char N >
bool 
backet_tank< N >::full() const 
{
	return _states == 0;
}
	
template < unsigned char N >
bool 
backet_tank< N >::empty() const 
{ 
	return _states == (rooms_type_t)-1; 
}

template < unsigned char N >
char*
backet_tank< N >::get_chunk()
{
#ifdef DEBUG
	if (empty())
	{
		assert(false);
		return 0;
	}
#endif

	// code can be optimized
	unsigned char offset = 0x0;

	rooms_type_t lowbit = ~_states & ~(~_states - 1);
	_states |= lowbit;
	
	if (!(lowbit & 0xffffffff))
	{
		offset += 32;
		lowbit >>= 32;
	}

	if (!(lowbit & 0xffff))
	{
		offset += 16;
		lowbit >>= 16;
	}

	if (!(lowbit & 0xff))
	{
		offset += 8;
		lowbit >>= 8;
	}

	if (!(lowbit & 0xf))
	{
		offset += 4;
		lowbit >>= 4;
	}

	switch (lowbit & 0xf)
	{
		case 0:
			assert(false);
			break;
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
		case 11:
		case 13:
		case 15:
			break;
		case 2:
		case 6:
		case 10:
		case 14:
			offset += 1;
			break;
		case 4:
		case 12:
			offset += 2;
			break;
		case 8:
			offset += 3;
			break;
	}


	char* ret = _head + (offset * LENGTH);
	*ret = (char)N;
	*(ret + 1) = (char)offset;

	return ret;
}

template < unsigned char N >
void
backet_tank< N >::put_chunk(char* ptr)
{
	unsigned char* chunk = (unsigned char*)(ptr - 4);
#ifdef DEBUG
	if (*chunk != N)
	{
		assert(false);
		return;
	}
#endif
	unsigned char offset = *(chunk + 1);

	rooms_type_t mask = 0x01 << offset;

#ifdef DEBUG
	if (!(_states & mask))
	{
		assert(false);
		return;
	}
#endif

	_states &= ~mask;
}

template < unsigned char N >
void
backet_tank< N >::reset()
{
	_states = 0;
}

template < unsigned char N, int M >
string_backet< N, M >::string_backet() : _all( backet_tank< N >::LENGTH * M), _page(0)
{
	reset();
}

template < unsigned char N, int M >
string_backet< N, M >::~string_backet() 
{
}

template < unsigned char N, int M >
char*
string_backet< N, M >::allocate()
{
	char* ret = 0;

	while (_page < M && _pages[_page] && _pages[_page]->empty())
		++_page;

	if (_page == M)
		return 0;

	if (!_pages[_page])
	{
		_pages[_page] = new (_all.allocate(sizeof(backet_tank< N >))) backet_tank< N >(_all);
		if (!_pages[_page])
			return 0;
	}

	ret = _pages[_page]->get_chunk(); // no enough memory in chunk
	
#ifdef DEBUG
	assert(ret != 0);
#endif
	// assign page
	*(ret + 2) = (char)((_page & 0xff00) >> 8);
	*(ret + 3) = (char)(_page & 0x00ff);

	ret += 4;
	return ret;
}

template < unsigned char N, int M >
void
string_backet< N, M >::deallocate(char* ptr)
{
	unsigned char* backet = (unsigned char*)ptr - 4;
#ifdef DEBUG
	if (*backet != N) // error
	{
		assert(false);
		return;
	}
#endif
	unsigned short page_high = (unsigned short)*(backet + 2) & 0x00ff;
	unsigned short page_low = (unsigned short)*(backet + 3) & 0x00ff;
	unsigned short p = (page_high << 8 | page_low);

#ifdef DEBUG
	if (p >= M || !_pages[p])
	{
		assert(false);
		return;
	}
#endif
	_pages[p]->put_chunk(ptr);
	if (p < _page)
		_page = p;
}

template < unsigned char N, int M >
void
string_backet< N, M >::reset()
{
	memset(_pages, 0, sizeof(void*) * M);
	_all.reset();
}

template < unsigned char N, int M >
void
string_backet< N, M >::release()
{
	for (int p = M; p > 0; --p)
		if (_pages[p - 1] && !_pages[p - 1]->full())
			return;

	reset();
}


inline
char* 
string_repository::allocate(size_t len)
{
	if (!len)
		return 0;

	char* ret = 0;
	size_t rounded_size = 4;
	int shift = len >> rounded_size;

	while (shift > 1)
	{
		++rounded_size;
		shift >>= 1;
	}

	switch (rounded_size)
	{
		case 4:
			if (ret = _backet4.allocate())
				break;
		case 5:
			if (ret = _backet5.allocate())
				break;
		case 6:
			if (ret = _backet6.allocate())
				break;
		case 7:
			if (ret = _backet7.allocate())
				break;
		case 8:
			if (ret = _backet8.allocate())
				break;
		case 9:
			if (ret = _backet9.allocate())
				break;
		case 10:
			if (ret = _backet10.allocate())
				break;
		case 11:
			if (ret = _backet11.allocate())
				break;
		case 12:
			if (ret = _backet12.allocate())
				break;
		case 13:
			if (ret = _backet13.allocate())
				break;
		default:
			ret = new char[len + 4];
			if (ret)
			{
				memset(ret, 0, 4);
				ret += 4;
			}				
	}

	return ret;
}

inline
void
string_repository::deallocate(char* ptr)
{
	unsigned char* backet = (unsigned char*)ptr - 4;
	switch (*backet)
	{
		case 4:
			 _backet4.deallocate(ptr);
			break;
		case 5:
			 _backet5.deallocate(ptr);
			break;
		case 6:
			 _backet6.deallocate(ptr);
			break;
		case 7:
			 _backet7.deallocate(ptr);
			break;
		case 8:
			 _backet8.deallocate(ptr);
			break;
		case 9:
			 _backet9.deallocate(ptr);
			break;
		case 10:
			 _backet10.deallocate(ptr);
			break;
		case 11:
			 _backet11.deallocate(ptr);
			break;
		case 12:
			 _backet12.deallocate(ptr);
			break;
		case 13:
			 _backet13.deallocate(ptr);
			break;
		case 0:
			delete [] backet;
			break;
		default:
			assert(false);
	}
}


END_TERIMBER_NAMESPACE

#endif // _terimber_strrep_hpp_