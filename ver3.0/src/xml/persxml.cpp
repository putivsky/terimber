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

#include "xml/persxml.h"
#include "xml/defxml.hpp"
#include "xml/storexml.hpp"
#include "xml/sxml.hpp"
#include "xml/sxs.hpp"

#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/template.hpp"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////////////
xml_persistor::xml_persistor(byte_consumer& stream,
							 const xml_document& doc,
							 mem_pool_t& small_pool,
							 mem_pool_t& big_pool,
							 bool validate,
							 bool save_doc_type,
							 size_t size) :
	_xml_size(__max(size, os_def_size)),
	_stream(stream),
	_doc(doc),
	_small_pool(small_pool),
	_big_pool(big_pool),
	_validate(validate),
	_save_doc_type(save_doc_type)
{
	_tmp_allocator = _small_pool.loan_object();
	if (_xml_size <= os_def_size)
		_stack_allocator = _small_pool.loan_object(_xml_size);
	else
		_stack_allocator = _big_pool.loan_object();
}

xml_persistor::~xml_persistor()
{
	_small_pool.return_object(_tmp_allocator);
	if (_xml_size <= os_def_size)
		_small_pool.return_object(_stack_allocator);
	else
		_big_pool.return_object(_stack_allocator);
}

const char* 
xml_persistor::get_error() const
{ 
	return _error; 
}

bool
xml_persistor::persist()
{
	try
	{
		_element_stack.clear();
		_stack_allocator->reset();
		_element_stack.push(*_stack_allocator, &_doc);
		_in_doc_type = false;
		while (!_element_stack.empty())
		{
			switch (_element_stack.top()->_decl->get_type())
			{
				case DOCUMENT_NODE:	
					persistDocument();
					break;
				case ELEMENT_NODE:
					_in_doc_type ? persistElementDecl() : persistElement();
					break;
				case ATTRIBUTE_NODE:
					assert(!_in_doc_type);
					persistAttributes();
					break;
				case TEXT_NODE:
					persistText();
					break;
				case CDATA_SECTION_NODE:
					persistCDATA();
					break;
				case PROCESSING_INSTRUCTION_NODE:
					persistPI();
					break;
				case COMMENT_NODE:
					persistComment();
					break;
				case DOCUMENT_TYPE_NODE:
					persistDocType();
					break;
				case ENTITY_NODE:
					assert(_in_doc_type);
					persistEntityDecl();
					break;
				case NOTATION_NODE:
					assert(_in_doc_type);
					persistNotationDecl();
					break;
				default:
					assert(false);
			} // switch

			// looks for the next child
		} // while	

		_stream.flush();
	}
	catch (exception& x)
	{
		_error = x.what();
		return false;
	}
	catch (...)
	{
		_error = "Unexpected exception has been thrown";
		return false;
	}

	return true;
}

void 
xml_persistor::restore_stack(bool makeShift)
{
	while (!_element_stack.empty())
	{
		const xml_tree_node* node = _element_stack.top();
		const xml_element* el = 0;
		// checks for children and type element
		switch (node->_decl->get_type())
		{
			case ELEMENT_NODE:
                if (!_in_doc_type && (el = static_cast< const xml_element* >(node)) && el->has_children())
				{
					if (makeShift)
					{
						size_t shift = _element_stack.size() - 1;
						while (--shift)
							_stream.push(ch_hor_tab);
					}
					else
						makeShift = true;
						
					_stream.push(ch_open_angle);
					_stream.push(ch_forward_slash);
					_stream.push(el->_decl->_name);
					_stream.push(ch_close_angle);
					_stream.push(ch_cr);
					_stream.push(ch_lf);
				}
				break;
			case DOCUMENT_TYPE_NODE:
				if (_save_doc_type)
				{
					_stream.push(ch_close_square);
					_stream.push(ch_close_angle);
					_stream.push(ch_cr);
					_stream.push(ch_lf);
				}

				_in_doc_type = false;
				break;
			default:
				break;
		} // switch

		// removes the current
		_element_stack.pop();

		// if there is a next sibling, pushes to stack
		if (node->_right)
		{
			// pushes the next
			_element_stack.push(*_stack_allocator, node->_right);
			return;
		}
	} // if

	_stack_allocator->reset();
}

void 
xml_persistor::persistDocument()
{
	// [23]    XMLDecl    ::=    '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
	static const char* xmlDeclStandaloneYes = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
	static const char* xmlDeclStandaloneNo = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
	 
	_stream.push(_doc._standalone == 0 ? xmlDeclStandaloneNo : xmlDeclStandaloneYes);

	_stream.push(ch_cr);
	_stream.push(ch_lf);
	const xml_container* el = static_cast< const xml_container* >(_element_stack.top());
	if (el->_first_child)
	{
		_element_stack.push(*_stack_allocator, el->_first_child);
		return;
	}
	else
		restore_stack();
}

void 
xml_persistor::persistDocType()
{	
	if (!_save_doc_type)
	{
		restore_stack();
		return;
	}

	// [28] doctypedecl    ::=    '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | DeclSep)* ']' S?)? '>' 
	_stream.push(ch_open_angle);
	_stream.push(ch_bang);
	_stream.push(str_DOCTYPE);
	_stream.push(ch_space);
	_stream.push(_doc._doc_name);
	_stream.push(ch_space);
	
	if (_doc._system_id.length()) // internal doc type
	{
		// 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral 
		if (_doc._public_id.length())
		{
			_stream.push(str_PUBLIC);
			_stream.push(ch_space);
			_stream.push(ch_double_quote);
			_stream.push(_doc._public_id);
			_stream.push(ch_double_quote);
			_stream.push(ch_space);
		}
		else
		{
			_stream.push(str_SYSTEM);
			_stream.push(ch_space);
		}

		_stream.push(ch_double_quote);
		_stream.push(_doc._system_id);
		_stream.push(ch_double_quote);

		const xml_tree_node* el = _element_stack.top();
		// removes the current
		_element_stack.pop();
		// if there is a next sibling, pushes to stack
		if (el->_right)
		{
			// pushes the next
			_element_stack.push(*_stack_allocator, el->_right);
			_stream.push(ch_close_angle);
			_stream.push(ch_cr);
			_stream.push(ch_lf);
			return;
		}

		_stream.push(ch_close_angle);
		_stream.push(ch_cr);
		_stream.push(ch_lf);
		restore_stack();
	}
	else if (!_doc.is_on_fly())
	{
		// ('[' (markupdecl | DeclSep)* ']' S?)
		_in_doc_type = true;
		_stream.push(ch_open_square);
		_stream.push(ch_cr);
		_stream.push(ch_lf);
		const xml_container* el = static_cast< const xml_container* >(_element_stack.top());

		if (el->_first_child)
		{
			_element_stack.push(*_stack_allocator, el->_first_child);
			return;
		}

		restore_stack();
	} // else
	else
	{
		_stream.push(ch_open_square);
		_stream.push(ch_cr);
		_stream.push(ch_lf);

		// exports
		const entity_decl_map_t& en_map = _doc.get_entityMap();
		for (entity_decl_map_t::const_iterator iterEn = en_map.begin(); iterEn != en_map.end(); ++iterEn)
			persistEntityDecl(&*iterEn);

		const notation_decl_map_t& not_map = _doc.get_notationMap();
		for (notation_decl_map_t::const_iterator iterNot = not_map.begin(); iterNot != not_map.end(); ++iterNot)
			persistNotationDecl(&*iterNot);

		// pushes element decl with attribute decl
		const element_decl_map_t& el_map = _doc.get_elementMap();
		for (element_decl_map_t::const_iterator iterEl = el_map.begin(); iterEl != el_map.end(); ++iterEl)
			persistElementDecl(&*iterEl);

		restore_stack();
	}
}

void 
xml_persistor::persistEntityDecl(const entityDecl* decl_)
{
	const entityDecl* decl = decl_ ? decl_ : static_cast< const entityDecl* >(_element_stack.top()->_decl);

	if (decl->_is_encoded_char)
		return;
	// [70]    EntityDecl    ::=    GEDecl | PEDecl 
	// [73]    EntityDef    ::=    EntityValue | (ExternalID NDataDecl?) 
	// [74]    PEDef    ::=    EntityValue | ExternalID 
	// [9]    EntityValue    ::=    '"' ([^%&"] | PEReference | Reference)* '"'  |  "'" ([^%&'] | PEReference | Reference)* "'" 
	// [75]    ExternalID    ::=    'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral  
	// [76]    NDataDecl    ::=    S 'NDATA' S Name 
	_stream.push(ch_open_angle);
	_stream.push(ch_bang);
	_stream.push(str_ENTITY);
	_stream.push(ch_space);
	if (decl->_is_parameter)
	{
		// [72]    PEDecl    ::=    '<!ENTITY' S '%' S Name S PEDef S? '>' 
		_stream.push(ch_percent);
		_stream.push(ch_space);
		_stream.push(decl->_name);
		_stream.push(ch_space);
		// [74]    PEDef    ::=    EntityValue | ExternalID 
		if (decl->_is_in_subset)
		{
			// [75]    ExternalID    ::=    'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral  
			if (decl->_publicId.length())
			{
				_stream.push(str_PUBLIC);
				_stream.push(ch_space);
				_stream.push(decl->_publicId);
				_stream.push(ch_space);
			}

			_stream.push(str_SYSTEM);
			_stream.push(ch_space);
			_stream.push(decl->_systemId);
		}
		else
		{
			// [9]    EntityValue    ::=    '"' ([^%&"] | PEReference | Reference)* '"'  |  "'" ([^%&'] | PEReference | Reference)* "'" 
			_stream.push(ch_double_quote);
			persistValue(decl->_value);
			_stream.push(ch_double_quote);
		}
	}
	else
	{
		// [71]    GEDecl    ::=    '<!ENTITY' S Name S EntityDef S? '>' 
		_stream.push(decl->_name);
		_stream.push(ch_space);
		// [73]    EntityDef    ::=    EntityValue | (ExternalID NDataDecl?) 
		if (decl->_value.length())
		{
			_stream.push(ch_double_quote);
			persistValue(decl->_value);
			_stream.push(ch_double_quote);
		}
		else
		{
			if (decl->_publicId.length())
			{
				_stream.push(str_PUBLIC);
				_stream.push(ch_space);
				_stream.push(decl->_publicId);
				_stream.push(ch_space);
			}

			_stream.push(str_SYSTEM);
			_stream.push(ch_space);
			_stream.push(decl->_systemId);

			if (decl->_notation.length())
			{
				// [76]    NDataDecl    ::=    S 'NDATA' S Name 
				_stream.push(ch_space);
				_stream.push(str_NDATA);
				_stream.push(ch_space);
				_stream.push(decl->_notation);
			} // if
		} // else
	} // else

	_stream.push(ch_close_angle);
	_stream.push(ch_cr);
	_stream.push(ch_lf);
	if (!decl_)
		restore_stack();
}

void 
xml_persistor::persistNotationDecl(const notationDecl* decl_)
{
	const notationDecl* decl = decl_ ? decl_ : static_cast< const notationDecl* >(_element_stack.top()->_decl);
	// [82]    NotationDecl    ::=    '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
	_stream.push(ch_open_angle);
	_stream.push(ch_bang);
	_stream.push(str_NOTATION);
	_stream.push(ch_space);
	_stream.push(decl->_name);
	if (decl->_publicId.length())
	{
		_stream.push(ch_space);
		_stream.push(str_PUBLIC);
		_stream.push(ch_space);
		_stream.push(decl->_publicId);
	}
	
	if (decl->_systemId.length())
	{
		_stream.push(ch_space);
		_stream.push(str_SYSTEM);
		_stream.push(ch_space);
		_stream.push(decl->_systemId);
	}

	_stream.push(ch_close_angle);
	_stream.push(ch_cr);
	_stream.push(ch_lf);
	if (!decl_)
		restore_stack();
}

void 
xml_persistor::persistElementDecl(const elementDecl* decl_)
{
	const elementDecl* decl = decl_ ? decl_ : static_cast< const elementDecl* >(_element_stack.top()->_decl);
	// <!ELEMENT' S Name S contentspec S? '>
	_stream.push(ch_open_angle);
	_stream.push(ch_bang);
	_stream.push(str_ELEMENT);
	_stream.push(ch_space);
	_stream.push(decl->_name);
	_stream.push(ch_space);
	switch (decl->_content)
	{
		case CONTENT_EMPTY:
			_stream.push(str_EMPTY);
			break;
		case CONTENT_ANY:
			_stream.push(str_ANY);
			break;
		case CONTENT_MIXED:
			{
				_stream.push(ch_open_paren);
				if (decl->_token->_rule == DFA_LEAF)
				{
					persistMixed(decl->_token);
					_stream.push(ch_close_paren);
				}
				else
				{
					persistMixed(decl->_token->_first);
					_stream.push(ch_close_paren);
					_stream.push(ch_asterisk);
				}
			} 
			break;
		case CONTENT_CHILDREN:
			switch (decl->_token->_rule)
			{
				case DFA_QUESTION:
				_stream.push(ch_open_paren);
				persistChildren(decl->_token->_first, decl->_token->_first->_rule);
				_stream.push(ch_close_paren);
				_stream.push(ch_question);
				break;
			case DFA_ASTERISK:
				_stream.push(ch_open_paren);
				persistChildren(decl->_token->_first, decl->_token->_first->_rule);
				_stream.push(ch_close_paren);
				_stream.push(ch_asterisk);
				break;
			case DFA_PLUS:
				_stream.push(ch_open_paren);
				persistChildren(decl->_token->_first, decl->_token->_first->_rule);
				_stream.push(ch_close_paren);
				_stream.push(ch_plus);
				break;
			default:
				_stream.push(ch_open_paren);
				persistChildren(decl->_token, decl->_token->_rule);
				_stream.push(ch_close_paren);
			break;
			} // switch
		default:
			break;
	}
	_stream.push(ch_close_angle);
	_stream.push(ch_cr);
	_stream.push(ch_lf);

	if (!decl->_attributes.empty())
	{
		// '<!ATTLIST' S Name AttDef* S? '>' 
		_stream.push(ch_open_angle);
		_stream.push(ch_bang);
		_stream.push(str_ATTRLIST);
		_stream.push(ch_space);
		_stream.push(decl->_name);

		// attributes
		for (attribute_decl_map_t::const_iterator iterAttr = decl->_attributes.begin(); iterAttr != decl->_attributes.end(); ++iterAttr)
			persistAttributeDecl(*iterAttr);

		_stream.push(ch_cr);
		_stream.push(ch_lf);
		_stream.push(ch_close_angle);
		_stream.push(ch_cr);
		_stream.push(ch_lf);
	} // if

	if (!decl_)
		restore_stack();
}

void 
xml_persistor::persistMixed(const dfa_token* parent)
{
	switch (parent->_rule)
	{
		case DFA_LEAF:
		case DFA_ANY:
			if (parent->_decl)
			{
				_stream.push(ch_pipe);
				_stream.push(parent->_decl->_name);
			}
			else
                _stream.push(str__PCDATA);
			break;
		case DFA_CHOICE:
		case DFA_SEQUENCE:
			{
				// Recurses on the left and right nodes
				persistMixed(parent->_first);
				// The last node of a choice or sequence has a null right
				if (parent->_last)
					persistMixed(parent->_last);
			}
			break;
		case DFA_QUESTION:
			_stream.push(ch_question);
			persistMixed(parent->_first);
			break;
		case DFA_ASTERISK:
			_stream.push(ch_asterisk);
			persistMixed(parent->_first);
			break;
		case DFA_PLUS:
			_stream.push(ch_plus);
			persistMixed(parent->_first);
			break;
		default:
			assert(false);
	}
}

void 
xml_persistor::persistChildren(const dfa_token* parent, dfaRule prevRule)
{
    // Gets the spec type of the passed node
	dfaRule curRule = parent->_rule;

	switch (curRule)
	{
		case DFA_ANY:
			_stream.push(str_ANY);
			break;
		case DFA_LEAF:
			_stream.push(parent->_decl->_name);
			break;
		case DFA_CHOICE:
			if (DFA_CHOICE != prevRule) _stream.push(ch_open_paren);
			persistChildren(parent->_first, curRule);
			_stream.push(ch_pipe);
			persistChildren(parent->_last, curRule);
			if (DFA_CHOICE != prevRule) _stream.push(ch_close_paren);
			break;
		case DFA_SEQUENCE:
			if (DFA_SEQUENCE != prevRule) _stream.push(ch_open_paren);
			persistChildren(parent->_first, curRule);
			_stream.push(ch_comma);
			persistChildren(parent->_last, curRule);
			if (DFA_SEQUENCE != prevRule) _stream.push(ch_close_paren);
			break;
		case DFA_QUESTION:
			persistChildren(parent->_first, prevRule);
			_stream.push(ch_question);
			break;
		case DFA_ASTERISK:
			persistChildren(parent->_first, prevRule);
			_stream.push(ch_asterisk);
			break;
		case DFA_PLUS:
			persistChildren(parent->_first, prevRule);
			_stream.push(ch_plus);
			break;
		default:
			assert(false);
    }
}

void 
xml_persistor::persistAttributeDecl(const attributeDecl& decl)
{
	// S Name S AttType S DefaultDecl 
	_stream.push(ch_cr);
	_stream.push(ch_lf);
	_stream.push(decl._name);
	_stream.push(ch_space);
	switch (decl._atype)
	{
		case ATTR_TYPE_CDATA:
			_stream.push(str_CDATA);
			break;
		case ATTR_TYPE_ID:
			_stream.push(str_ID);
			break;
		case ATTR_TYPE_IDREF:
			_stream.push(str_IDREF);
			break;
		case ATTR_TYPE_IDREFS:
			_stream.push(str_IDREFS);
			break;
		case ATTR_TYPE_ENTITY:
			_stream.push(str_ENTITY);
			break;
		case ATTR_TYPE_ENTITIES:
			_stream.push(str_ENTITIES);
			break;
		case ATTR_TYPE_NMTOKEN:
			_stream.push(str_NMTOKEN);
			break;
		case ATTR_TYPE_NMTOKENS:
			_stream.push(str_NMTOKENS);
			break;
		case ATTR_TYPE_NOTATION:
			_stream.push(str_NOTATION);
			_stream.push(ch_space);
		case ATTR_TYPE_ENUMERATION:
			if (!decl._enum.empty())
			{
				_stream.push(ch_open_paren);
				for (_list< enumNodeDecl >::const_iterator iterEn = decl._enum.begin(); iterEn != decl._enum.end(); ++iterEn)
				{
					if (iterEn != decl._enum.begin())
						_stream.push(ch_pipe);
					_stream.push(iterEn->_value);
				}

				_stream.push(ch_close_paren);
			}
			break;
		default:
			assert(false);
	} // switch

	_stream.push(ch_space);
	switch (decl._rule)
	{					
		case ATTR_RULE_REQUIRED:
			_stream.push(ch_pound);
			_stream.push(str_REQUIRED);
			break;
		case ATTR_RULE_IMPLIED:
			_stream.push(ch_pound);
			_stream.push(str_IMPLIED);
			break;
		case ATTR_RULE_FIXED:
			_stream.push(ch_pound);
			_stream.push(str_FIXED);
			break;
		case ATTR_RULE_DEFAULT:
			break;
		default:
			assert(false);
	}

	if (decl._defval.length())
	{
		_stream.push(ch_space);
		_stream.push(ch_double_quote);
		_stream.push(decl._defval);
		_stream.push(ch_double_quote);
	}
}

void 
xml_persistor::persistElement()
{
	const xml_element* el = static_cast< const xml_element* >(_element_stack.top());

	size_t shift = _element_stack.size() - 1;
	while (--shift)
		_stream.push(ch_hor_tab);

	_stream.push(ch_open_angle);
	_stream.push(el->_decl->_name);
	if (el->has_attributes())
		persistAttributes();
	if (el->has_children())
	{
		_stream.push(ch_close_angle);

		if (el->_first_child->_decl->get_type() != TEXT_NODE)
		{
			_stream.push(ch_cr);
			_stream.push(ch_lf);
		}

		if (el->_first_child)
		{
			_element_stack.push(*_stack_allocator, el->_first_child);
			return;
		}
	}
	else
	{
		_stream.push(ch_forward_slash);
		_stream.push(ch_close_angle);
		_stream.push(ch_cr);
		_stream.push(ch_lf);
	}

	// must remove from stack and leave
	restore_stack();
}

void 
xml_persistor::persistAttributes()
{
	const xml_element* el = static_cast< const xml_element* >(_element_stack.top());
	for (xml_tree_node* I = el->_first_attr; I; I = I->_right)
	{		
		_stream.push(ch_space);
		const attributeDecl* decl = I->cast_to_attribute();
		_stream.push(decl->_name);
		_stream.push(ch_equal);
		_stream.push(ch_double_quote);
		persistValue(decl->persist_attribute(xml_value_node::cast_to_node_value(I)->_value, _tmp_allocator));
		_tmp_allocator->reset();
		_stream.push(ch_double_quote);
	}
}

void 
xml_persistor::persistText()
{
	persistValue(xml_value_node::cast_to_node_value(_element_stack.top())->_value.strVal, true);
	restore_stack(false);
}

void 
xml_persistor::persistCDATA()
{
	// [18]    CDSect    ::=    CDStart CData CDEnd 
	// [19]    CDStart    ::=    '<![CDATA[' 
	// [20]    CData    ::=    (Char* - (Char* ']]>' Char*))  
	// [21]    CDEnd    ::=    ']]>' 

	_stream.push(ch_open_angle);
	_stream.push(ch_bang);
	_stream.push(ch_open_square);
	_stream.push(str_CDATA);
	_stream.push(ch_open_square);
	_stream.push(xml_value_node::cast_to_node_value(_element_stack.top())->_value.strVal);
	_stream.push(ch_close_square);
	_stream.push(ch_close_square);
	_stream.push(ch_close_angle);
	_stream.push(ch_cr);
	_stream.push(ch_lf);
	restore_stack();
}

void 
xml_persistor::persistComment()
{
	// [15]    Comment    ::=    '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
	_stream.push(ch_open_angle);
	_stream.push(ch_bang);
	_stream.push(ch_dash);
	_stream.push(ch_dash);
	_stream.push(xml_value_node::cast_to_node_value(_element_stack.top())->_value.strVal);
	_stream.push(ch_dash);
	_stream.push(ch_dash);
	_stream.push(ch_close_angle);
	_stream.push(ch_cr);
	_stream.push(ch_lf);
	restore_stack();
}

void 
xml_persistor::persistPI()
{
	// [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
	// [17]    PITarget    ::=    Name - (('X' | 'x') ('M' | 'm') ('L' | 'l')) 
	_stream.push(ch_open_angle);
	_stream.push(ch_question);
	_stream.push(_element_stack.top()->_decl->_name);
	const char* value = xml_value_node::cast_to_node_value(_element_stack.top())->_value.strVal;
	if (value)
	{
		_stream.push(ch_space);
		_stream.push(value);
	}
	_stream.push(ch_question);
	_stream.push(ch_close_angle);
	_stream.push(ch_cr);
	_stream.push(ch_lf);
	restore_stack();
}

void 
xml_persistor::persistValue(const char* value, bool charData)
{
	if (!value)
		return;

	while (*value)
	{
		switch (*value)
		{
			case ch_ampersand:
				_stream.push(ch_ampersand);
				_stream.push(str_amp);
				_stream.push(ch_semicolon);
				break;
			case ch_double_quote:
				if (charData)
					_stream.push(*value);
				else
				{
					_stream.push(ch_ampersand);
					_stream.push(str_quote);
					_stream.push(ch_semicolon);
				}
				break;
			case ch_single_quote:
				if (charData)
					_stream.push(*value);
				else
				{
					_stream.push(ch_ampersand);
					_stream.push(str_apos);
					_stream.push(ch_semicolon);
				}
				break;
			case ch_open_angle:
				_stream.push(ch_ampersand);
				_stream.push(str_lt);
				_stream.push(ch_semicolon);
				break;
			case ch_close_angle:
				if (charData)
					_stream.push(*value);
				else
				{
					_stream.push(ch_ampersand);
					_stream.push(str_gt);
					_stream.push(ch_semicolon);
				}
				break;
			default:
				_stream.push(*value);
				break;
		} // switch 

		++value;
	}
}

#pragma pack()
END_TERIMBER_NAMESPACE

