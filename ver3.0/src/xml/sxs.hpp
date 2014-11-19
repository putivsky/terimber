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

#ifndef _terimber_sxs_hpp_
#define _terimber_sxs_hpp_

#include "xml/sxs.h"
#include "base/except.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline 
xmlNodeType 
nodeDecl::get_type() const
{ 
	return _type; 
}

/////////////////////////////////////////////////////
inline 
byte_allocator& 
xml_grammar::get_data_allocator() 
{ 
	return _data_allocator; 
}

inline 
byte_allocator& 
xml_grammar::get_tmp_allocator() 
{ 
	return _tmp_allocator; 
}

inline 
const namedNodeDecl* 
xml_grammar::get_document_decl() const 
{ 
	return &_document_decl; 
}
	
inline 
const namedNodeDecl* 
xml_grammar::get_doctype_decl() const 
{ 
	return &_doctype_decl; 
}

inline 
const namedNodeDecl* 
xml_grammar::get_comment_decl() const 
{ 
	return &_comment_decl; 
}

inline 
const namedNodeDecl* 
xml_grammar::get_cdata_decl() const 
{ 
	return &_cdata_decl; 
}

inline 
void 
xml_grammar::clear()
{ 
	_elementMap.clear();
	_notationMap.clear();
	_entityMap.clear();
	_attributeRefMap.clear();
	_piMap.clear();
	_doc_name = 0;
	_public_id = 0;
	_system_id = 0;
	_data_allocator.clear_extra();
	_tmp_allocator.clear_extra();
}

inline
const element_decl_map_t& 
xml_grammar::get_elementMap() const 
{ 
	return _elementMap; 
}

inline
const namednode_decl_map_t& 
xml_grammar::get_piMap() const 
{ 
	return _piMap; 
}

inline
const entity_decl_map_t& 
xml_grammar::get_entityMap() const 
{ 
	return _entityMap; 
}

inline
const notation_decl_map_t& 
xml_grammar::get_notationMap() const 
{ 
	return _notationMap; 
}

xml_forceinline 
void 
xml_grammar::check_readonly()
{ 
	if (_read_only) exception::_throw("External grammar has been provided"); 
}


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_sxs_hpp_ 
