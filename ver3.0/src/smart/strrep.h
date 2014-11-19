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

#ifndef _terimber_strrep_h_
#define _terimber_strrep_h_

#include "../base/ns.h"
#include "../base/list.h"
#include "../base/map.h"
#include "../base/string.h"

BEGIN_TERIMBER_NAMESPACE

// class supports string memory management
// allocated block will have 4 hidden leading bytes 
// 1st byte = backet number [4-12], 2nd - byte = offset/state [0-255], 3rd-4th bytes = page number [0-64K], 
// for allocation out of backet

template < unsigned char N >
class backet_tank
{
	typedef ub8_t rooms_type_t;	
public:
	enum BACKET { LENGTH = (1<<N) + 4, ROOMS = 8*sizeof(rooms_type_t) };

	backet_tank(byte_allocator& all);

	inline bool full() const;
	inline bool empty() const ;
	inline char* get_chunk();
	inline void put_chunk(char* ptr);
	inline void reset();

private:	
	char*			_head;
	rooms_type_t	_states; // bit = 1 - busy, bit = 0 free
};

template < unsigned char N, int M >
class string_backet
{
public:
	string_backet();
	~string_backet();

	inline char* allocate();
	inline void deallocate(char* ptr);
	inline void reset();
	inline void release();

private:
	byte_allocator		_all;
	backet_tank< N >*	_pages[M];
	int					_page;
};

class string_repository
{
public:
	inline char* allocate(size_t len);
	inline void deallocate(char* ptr);

private:
	string_backet< 4, 64*1024 >	_backet4; // page = (16 + 4)*256 = 4K + array = 4 * 64K = 256K = (256K + N*4K) - max N = 64K -> max memory 256M
	string_backet< 5, 32*1024 >	_backet5; // page = (32 + 4)*256 = 8K + array = 4 * 32K = 128K = (128K + N*8K) - max N = 32K -> max memory 256M
	string_backet< 6, 16*1024 >	_backet6; // page = (16 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 7, 8*1024 >	_backet7; // page = (8 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 8, 4*1024 >	_backet8; // page = (4 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 9, 2*1024 >	_backet9; // page = (2 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 10, 1*1024 >	_backet10; // page = (1 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 11, 512 >	_backet11; // page = (1/2 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 12, 256 >	_backet12; // page = (1/4 + 4)*256 = 4K + array = 4 * 16K = 64K = (64K + N*16K) - max N = 16K -> max memory 256M
	string_backet< 13, 128 >	_backet13; // page = 8K*256 = 2M + array = 4 * 128 = 512 = (512 + N*2M) - max N = 128 -> max memory 256M
};


END_TERIMBER_NAMESPACE

#endif // _terimber_strrep_h_