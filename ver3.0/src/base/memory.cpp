/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2010 The Terimber Corporation. All rights reserved.
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

#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

////////////////////////////
byte_allocator::byte_allocator(size_t capacity) :
	_capacity(capacity > os_def_size ? capacity : os_def_size), _count(0), _free_pos(0), _start_chunk(0), _using_chunk(0)
{
}
   
byte_allocator::~byte_allocator()
{ 
	clear_all(); 
}

void* 
byte_allocator::new_chunk(size_t size)
{
	size_t new_size = size > _capacity ? size : _capacity;

	// we are here - it means we didn't find an available chunk, 
	// need to allocate a new one
	mem_chunk* chunk = (mem_chunk*)::malloc(sizeof(mem_chunk) - 
                                          mem_chunk::MMC_ALIGN * sizeof(uint8_t) + 
                                          new_size);

	if (!chunk) // problems with system memory allocation
		return 0;
				
	// sets new chunk properties
	chunk->_chunk_size = new_size;
	chunk->_next_chunk = 0;
	
	if (_start_chunk) // there was previous allocation
	{
		_using_chunk->_next_chunk = chunk;
		_using_chunk = chunk;
	}
	else // never allocated before
		_start_chunk = _using_chunk = chunk;

	// increment count of blocks
	++_count;

	// sets current chunk position
	_free_pos = chunk->_mem + size;
	// returns memory pointer
	return chunk->_mem;
}

void* 
byte_allocator::next_chunk(size_t size)
{
	mem_chunk* chunk = _using_chunk ? _using_chunk->_next_chunk : 0;
	unsigned char* pos = chunk ? chunk->_mem : 0;
	while (chunk) // while chunks are available
	{
		// sets new current chunk
		_using_chunk = chunk;
		// compares remained and requested sizes
		if (chunk->_chunk_size - (pos - chunk->_mem) >= size)
		{ _free_pos = pos + size; return pos; }			
		// looks for next chunk
		chunk = chunk->_next_chunk;
		// sets new position
		pos = chunk ? chunk->_mem : 0;
	}
	// needs to allocate a new chunk
	return new_chunk(size);
}

void 
byte_allocator::clear_extra(bool secure)
{ 
	// frees only extra chunks
	mem_chunk* remove_chunk = _start_chunk ? _start_chunk->_next_chunk : 0; // save chunk pointer

	if (_start_chunk) // cuts long tail
		_start_chunk->_next_chunk = 0;

	while (remove_chunk) // while memory is there
	{
		_using_chunk = remove_chunk; // saves chunk pointer
		remove_chunk = remove_chunk->_next_chunk; // moves to the next chunk
		// free memory
		if (secure)
			memset(_using_chunk->_mem, 0, _using_chunk->_chunk_size);

		::free(_using_chunk);
		--_count;
	}

	reset(); // resets memory
}

void 
byte_allocator::clear_all(bool secure)
{ 
	while (_start_chunk) // while the memory is there
	{
		_using_chunk = _start_chunk; // saves the chunk pointer
		_start_chunk = _start_chunk->_next_chunk; // move to next chunk
		// frees memory
		if (secure)
			memset(_using_chunk->_mem, 0, _using_chunk->_chunk_size);

		::free(_using_chunk);
	}

	_count = 0;
	reset(); // resets memory
}

/////////////////////////////////////////////////////////////////
rep_allocator::rep_allocator(size_t capacity) : 
	byte_allocator(capacity)
{
}

rep_allocator::~rep_allocator()
{
	_rep.clear();
}

////////////////////////////////////////////////////
// static
byte_allocator* 
byte_allocator_creator::create(size_t size)
{
	// call constructor
	return new byte_allocator(size);
}

//static
bool 
byte_allocator_creator::find(byte_allocator* obj, size_t size)
{
	// finds an allocator with proper size
	return obj->capacity() >= size;
}

//static 
void 
byte_allocator_creator::back(byte_allocator* obj, size_t)
{
	// if count of chunks is more than 1 - frees memeory, unless reset
	obj->count() > 1 ? obj->clear_extra() : obj->reset();
}

// static 
void 
byte_allocator_creator::deactivate(byte_allocator* obj, size_t)
{
	// frees memory
	obj->clear_all();
}



#pragma pack()
END_TERIMBER_NAMESPACE

