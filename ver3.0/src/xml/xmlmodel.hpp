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

#ifndef _terimber_xmlmodel_hpp_
#define _terimber_xmlmodel_hpp_

#include "xml/xmlmodel.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

////////////////////////////////////////////////////////
inline 
void 
content_node::init(byte_allocator& allocator_, size_t sizeBit)
{
	_firstPos.resize(allocator_, sizeBit);
	_lastPos.resize(allocator_, sizeBit);
}

inline 
dfaRule 
content_node::get_rule() const
{ 
	return _rule; 
}

inline 
const _bitset& 
content_node::get_firstPos() const
{ 
	if (_firstPos.empty())
	{
		_bitset& first = const_cast< _bitset& >(_firstPos);
		calc_pos(first, true);
	}

	return _firstPos;
}

inline 
const _bitset& 
content_node::get_lastPos() const
{
	if (_lastPos.empty())
	{
		_bitset& last = const_cast< _bitset& >(_lastPos);
		calc_pos(last, false);
	}

	return _lastPos; 
}

///////////////////////////////////////////////////////
inline 
size_t 
content_any::get_pos() const
{ 
	return _pos; 
}

///////////////////////////////////////////////////////////////////
inline 
const elementDecl* 
content_leaf::get_decl() const
{ 
	return _decl; 
}

inline 
size_t 
content_leaf::get_pos() const
{ 
	return _pos; 
}

inline 
void 
content_leaf::set_pos(size_t pos)
{ 
	_pos = pos; 
}


/////////////////////////////////////////////////////////////
inline 
content_node* 
content_unary::get_child() const
{ 
	return _child; 
}

inline
void 
content_unary::calc_pos(_bitset& state, bool first) const
{ 
	state = first ? _child->get_firstPos() : _child->get_lastPos();
}

//////////////////////////////////////////////////////////////////
inline 
content_node* 
content_binary::get_left() const
{ 
	return _left; 
}

inline 
content_node* 
content_binary::get_right() const
{ 
	return _right; 
}

//////////////////////////////////////////////////////
inline 
void
content_children::make_def_state_list(_vector< size_t >& x) const
{
	x.resize(_tmp_allocator, _elemMapSize);
	for (size_t index = 0; index < _elemMapSize; ++index)
		x[index] = 0xffffffff;
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_xmlmodel_hpp_ 
