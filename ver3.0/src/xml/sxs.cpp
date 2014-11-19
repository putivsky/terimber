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

#include "xml/sxs.hpp"
#include "xml/defxml.hpp"
#include "xml/declxml.hpp"

#include "base/common.hpp"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/string.hpp"
#include "base/template.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////
nodeDecl::~nodeDecl()
{
}

nodeDecl::nodeDecl(xmlNodeType type) :
	_type(type)
{
}

nodeDecl::nodeDecl(const nodeDecl& x) :
	_type(x._type)
{
}

/////////////////////////////////
namedNode::namedNode(const char* name, byte_allocator* allocator_) :
	_name(name, allocator_)
{
}

namedNode::namedNode(const namedNode& x) : _name(x._name)
{
}

void 
namedNode::clear()
{
	_name = 0;
}

/////////////////////////////////
namedNodeDecl::namedNodeDecl(xmlNodeType type, const char* name, byte_allocator* allocator_) :
	nodeDecl(type),
	namedNode(name, allocator_)
{
}

namedNodeDecl::namedNodeDecl(const namedNodeDecl& x) :
	nodeDecl(x), namedNode(x)
{
}

void 
namedNodeDecl::clear()
{
	namedNode::clear();
}

/////////////////////////////////
enumNodeDecl::enumNodeDecl(byte_allocator* allocator_) :
	_id(0), _value(allocator_)
{
}

enumNodeDecl::enumNodeDecl(const enumNodeDecl& x) :
	_id(x._id), _value(x._value)
{
}

//////////////////////////
attributeDecl::attributeDecl(const char* name, byte_allocator* allocator_) : 
	namedNodeDecl(ATTRIBUTE_NODE, name, allocator_),
	_atype(attributeType_MIN), 
	_ctype(vt_unknown), 
	_defval(allocator_),
	_rule(attributeRule_MIN)
{
}

attributeDecl::attributeDecl(const attributeDecl& x) :
	namedNodeDecl(x),
	_atype(x._atype), 
	_ctype(x._ctype), 
	_enum(x._enum),
	_defval(x._defval),
	_rule(x._rule)
{
}

const char* 
attributeDecl::persist_attribute(const terimber_xml_value& value, byte_allocator* allocator_) const
{
	if (_ctype == vt_enum)
	{
		for (_list< enumNodeDecl >::const_iterator iter = _enum.begin(); iter != _enum.end(); ++iter)
			if (iter->_id == value.lVal)
			{
				terimber_xml_value dummy;
				dummy.strVal = iter->_value;
				return persist_value(vt_string, dummy, allocator_);
			}

		return 0;
	}
	else
		return persist_value(_ctype, value, allocator_);

}

/////////////////////////////////////////////
notationDecl::notationDecl(const char* name, byte_allocator* allocator_) :
	namedNodeDecl(NOTATION_NODE, name, allocator_),
	_systemId(allocator_),
	_publicId(allocator_) 
{
}


notationDecl::notationDecl(const notationDecl& x) :
	namedNodeDecl(x),
	_systemId(x._systemId),
	_publicId(x._publicId) 
{
}

/////////////////////////////////////////////
entityDecl::entityDecl(const char* name, byte_allocator* allocator_) :
	namedNodeDecl(ENTITY_NODE, name, allocator_),
	_systemId(allocator_), 
	_publicId(allocator_), 
	_notation(allocator_), 
	_value(allocator_),
	_is_in_subset(false), 
	_is_parameter(false), 
	_is_encoded_char(false), 
	_is_unparsed(false)
{
}

entityDecl::entityDecl(const entityDecl& x) :
	namedNodeDecl(x),
	_systemId(x._systemId), 
	_publicId(x._publicId), 
	_notation(x._notation), 
	_value(x._value),
	_is_in_subset(x._is_in_subset), 
	_is_parameter(x._is_parameter), 
	_is_encoded_char(x._is_encoded_char), 
	_is_unparsed(x._is_unparsed)
{
}

/////////////////////////////////////////////
attributeRefDecl::attributeRefDecl(const char* name, byte_allocator* allocator_) :
	namedNode(name, allocator_),
	_declared(false),
	_refered(false)
{
}

attributeRefDecl::attributeRefDecl(const attributeRefDecl& x) :
	namedNode(x),
	_declared(x._declared),
	_refered(x._refered)
{
}

///////////////////////////////////////////
elementDecl::elementDecl(const char*name, byte_allocator* allocator_) :
	namedNodeDecl(ELEMENT_NODE, name, allocator_), 
	_content(contentSpec_MIN), 
	_token(0),
	_attributes()
{
}

elementDecl::elementDecl(const elementDecl& x) :
	namedNodeDecl(x), 
	_content(x._content), 
	_token(x._token),
	_attributes(x._attributes)
{
}


/////////////////////////////////////////////
dfa_token::dfa_token() :
	_rule(dfaRule_MIN), _decl(0), _first(0), _last(0)
{
}

dfa_token::dfa_token(byte_allocator& allocator_, const dfa_token& x) :
	_rule(x._rule), _decl(x._decl), _first(x._first), _last(x._last)
{	
}

dfa_token::dfa_token(dfaRule rule, const elementDecl* decl, dfa_token* first, dfa_token* last) :
	_rule(rule), _decl(decl), _first(first), _last(last)
{
}

dfa_token::~dfa_token()
{
}

void 
dfa_token::assign(byte_allocator& allocator_, const dfa_token& x)
{
	_decl = x._decl;
	_rule = x._rule;
	// allocates first
	if (x._first)
	{
		_first = new(check_pointer(allocator_.allocate(sizeof(dfa_token)))) dfa_token(allocator_, *x._first);
		// calls recursive
		_first->assign(allocator_, *x._first);
	}
	// allocates first
	if (x._last)
	{
		_last = new(check_pointer(allocator_.allocate(sizeof(dfa_token)))) dfa_token(allocator_, *x._last);
		// calls recursive
		_last->assign(allocator_, *x._last);
	}
}

bool
dfa_token::find_any_resursively() const
{
	if (_first 
		&& (_first->_decl && _first->_decl->_content == CONTENT_ANY
			|| _first->find_any_resursively())
		)
		return true;

	if (_last
		&& (_last->_decl && _last->_decl->_content == CONTENT_ANY
			|| _last->find_any_resursively())
		)
		return true;

	return false;
}

///////////////////////////////////////////////////
xml_grammar::xml_grammar(mem_pool_t& small_manager, mem_pool_t& big_manager, size_t xml_size) : 
	_small_manager(small_manager),
	_big_manager(big_manager),
	_xml_size(__max(xml_size, os_def_size)),
	_data_allocator(xml_size <= os_def_size ? *small_manager.loan_object() : *big_manager.loan_object()),
	_tmp_allocator(*small_manager.loan_object()),
	_document_decl(DOCUMENT_NODE, 0, 0), 
	_doctype_decl(DOCUMENT_TYPE_NODE, 0, 0),
	_text_decl(TEXT_NODE, 0, 0),
	_comment_decl(COMMENT_NODE, 0, 0), 
	_cdata_decl(CDATA_SECTION_NODE, 0, 0),
	_read_only(false),
	_doc_name(&_data_allocator),
	_public_id(&_data_allocator),
	_system_id(&_data_allocator)
{
}

xml_grammar::~xml_grammar()
{
	clear();
	_xml_size <= os_def_size ? _small_manager.return_object(&_data_allocator) : _big_manager.return_object(&_data_allocator);
	_small_manager.return_object(&_tmp_allocator);
}

xml_grammar& 
xml_grammar::copy(const xml_grammar& x)
{
	if (this != &x)
	{
		_doc_name = x._doc_name;
		_public_id = x._public_id;
		_system_id = x._system_id;

		// we can't assign maps directly
		notationDecl dummy_notation(0, &_data_allocator);
		_notationMap.clear(); 	
		for (notation_decl_map_t::const_iterator INot = x._notationMap.begin(); INot != x._notationMap.end(); ++INot) 
		{
			notationDecl& decl = *_notationMap.insert(_data_allocator, INot.key(), dummy_notation).first;
			decl._name = INot->_name;
			decl._publicId = INot->_publicId;
			decl._systemId = INot->_systemId;
		}

		entityDecl dummy_entity(0, &_data_allocator);
		_entityMap.clear(); 	
		for (entity_decl_map_t::const_iterator IEnt = x._entityMap.begin(); IEnt != x._entityMap.end(); ++IEnt) 
		{
			entityDecl& decl = *_entityMap.insert(_data_allocator, IEnt.key(), dummy_entity).first;
			decl._is_encoded_char = IEnt->_is_encoded_char;
			decl._is_in_subset = IEnt->_is_in_subset;
			decl._is_parameter = IEnt->_is_parameter;
			decl._is_unparsed = IEnt->_is_unparsed;
			decl._name = IEnt->_name;
			decl._notation = IEnt->_notation;
			decl._publicId = IEnt->_publicId;
			decl._systemId = IEnt->_systemId;
			decl._value = IEnt->_value;
		}

		attributeRefDecl dummy_attributeRef(0, &_data_allocator);
		_attributeRefMap.clear(); 	
		for (attribute_ref_decl_map_t::const_iterator iterAttRef = x._attributeRefMap.begin(); iterAttRef != x._attributeRefMap.end(); ++iterAttRef) 
		{
			attributeRefDecl& decl = *_attributeRefMap.insert(_data_allocator, iterAttRef.key(), dummy_attributeRef).first;
			decl._declared = iterAttRef->_declared;
			decl._name = iterAttRef->_name;
			decl._refered = iterAttRef->_refered;
		}

		namedNodeDecl dummy_pi(PROCESSING_INSTRUCTION_NODE, 0, &_data_allocator);
		_piMap.clear(); 	
		for (namednode_decl_map_t::const_iterator IPi = x._piMap.begin(); IPi != x._piMap.end(); ++IPi) 
		{
			namedNodeDecl& decl = *_piMap.insert(_data_allocator, IPi.key(), dummy_pi).first;
			decl._name = IPi->_name;
		}

		elementDecl dummy_element(0, &_data_allocator);
		attributeDecl dummy_attribute(0, &_data_allocator);
		enumNodeDecl dummy_enum(&_data_allocator);
		_elementMap.clear(); 	
		for (element_decl_map_t::const_iterator IEl = x._elementMap.begin(); IEl != x._elementMap.end(); ++IEl) 
		{
			elementDecl& decl = *_elementMap.insert(_data_allocator, IEl.key(), dummy_element).first;
			
			// copies full name
			decl._name = IEl->_name;

			// copies attributes
			for (attribute_decl_map_t::const_iterator IAttr = IEl->_attributes.begin(); IAttr != IEl->_attributes.end(); ++IAttr) 
			{
				attributeDecl& declAttr = *decl._attributes.insert(_data_allocator, IAttr.key(), dummy_attribute).first;
				declAttr._atype = IAttr->_atype;
				declAttr._ctype = IAttr->_ctype;
				declAttr._defval = IAttr->_defval;
				declAttr._name = IAttr->_name;
				declAttr._rule = IAttr->_rule;

				// enum is a list
				for (_list< enumNodeDecl >::const_iterator IEn = IAttr->_enum.begin(); IEn != IAttr->_enum.end(); ++IEn)
				{
					enumNodeDecl& entryEn = *declAttr._enum.push_back(_data_allocator, dummy_enum);
					entryEn._id = IEn->_id;
					entryEn._value = IEn->_value;
				}
			}
			
			// copies content
			decl._content = IEl->_content;
			if (IEl->_token)
			{
				decl._token = new(check_pointer(_data_allocator.allocate(sizeof(dfa_token)))) dfa_token(_data_allocator, *IEl->_token);
				decl._token->assign(_data_allocator, *IEl->_token);
			}
		}

		_read_only = true;
	}
	return *this;
}

elementDecl& 
xml_grammar::add_element_decl(const char* name, bool on_fly, bool fromDecl, bool modelAny)
{
	// we can't modify read only gramma
	check_readonly();

	// hash is a non qulified name
	size_t hash_value = do_hash((const char*)name, os_minus_one);
	element_decl_map_t::pairii_t range = _elementMap.equal_range(hash_value);
	element_decl_map_t::iterator start(range.first);

	if (start != range.second) // can be found
		for (start = range.first; start != range.second; ++start)
			if (start->_name == name) // checks name directly
			{
				if (fromDecl && start->_content != contentSpec_MIN)
					xml_exception_throw("Dublicate element name: ", 
										(const char*)name,
										0);
				return *start;
			} // if
	
	
	// inserts new element declaration
	elementDecl decl(0, &_data_allocator);
	start = _elementMap.insert(_data_allocator, hash_value, decl).first;
	if (start == _elementMap.end())
		xml_exception_throw("Not enough memory", 0);

	start->_name = name;

	if (on_fly || fromDecl || modelAny || (name[0] == 'A' && name[1] == 'N' && name[2] == 'Y' && name[3] == 0))
		start->_content = CONTENT_ANY;

	return *start;
}

const elementDecl* 
xml_grammar::find_element_decl(const char* name) const
{
	if (_elementMap.empty())
		return 0;
	
	element_decl_map_t::paircc_t range = _elementMap.equal_range(do_hash(name, os_minus_one));
	for (element_decl_map_t::const_iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}

elementDecl* 
xml_grammar::find_element_decl(const char* name)
{
	check_readonly();
	if (_elementMap.empty())
		return 0;
	
	element_decl_map_t::pairii_t range = _elementMap.equal_range(do_hash(name, os_minus_one));
	for (element_decl_map_t::iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}

notationDecl& 
xml_grammar::add_notation_decl(const char* name)
{
	check_readonly();
	size_t hash_value = do_hash(name, os_minus_one);
	notation_decl_map_t::pairii_t range = _notationMap.equal_range(hash_value);
	notation_decl_map_t::iterator start(range.first);

	if (start != range.second) // found
		for (start = range.first; start != range.second; ++start)
			if (start->_name == name) // checks name directly
				xml_exception_throw("Dublicate notation name: ",
									(const char*)name,
									0);

	// inserts new notation
	notationDecl decl(0, &_data_allocator);
	start = _notationMap.insert(_data_allocator, hash_value, decl).first;
	if (start == _notationMap.end())
		xml_exception_throw("Not enough memory", 0);

	start->_name = name;
	return *start;
}

const notationDecl* 
xml_grammar::find_notation_decl(const char* name) const
{
	if (_notationMap.empty())
		return 0;

	notation_decl_map_t::paircc_t range = _notationMap.equal_range(do_hash(name, os_minus_one));

	for (notation_decl_map_t::const_iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}

entityDecl& 
xml_grammar::add_entity_decl(const char* name, bool& wasAdded)
{
	check_readonly();
	wasAdded = false;
	size_t hash_value = do_hash(name, os_minus_one);
	entity_decl_map_t::pairii_t range = _entityMap.equal_range(hash_value);
	entity_decl_map_t::iterator start(range.first);

	if (start != range.second) // can be found
		for (start = range.first; start != range.second; ++start)
			if (start->_name == name) // checks name directly
				return *start;

	// inserts new entity
	entityDecl decl(0, &_data_allocator);
	start = _entityMap.insert(_data_allocator, hash_value, decl).first;
	if (start == _entityMap.end())
		xml_exception_throw("Not enough memory", 0);

	start->_name = name;
	wasAdded = true;

	return *start;
}

const entityDecl* 
xml_grammar::find_entity_decl(const char* name) const
{
	if (_entityMap.empty())
		return 0;

	entity_decl_map_t::paircc_t range = _entityMap.equal_range(do_hash(name, os_minus_one));

	for (entity_decl_map_t::const_iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}

entityDecl* 
xml_grammar::find_entity_decl(const char* name)
{
	check_readonly();

	if (_entityMap.empty())
		return 0;

	entity_decl_map_t::pairii_t range = _entityMap.equal_range(do_hash(name, os_minus_one));

	for (entity_decl_map_t::iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}

attributeDecl& 
xml_grammar::add_attribute_decl(const elementDecl& decl, const char* name, bool mustPresent, bool& wasAdded)
{
	// we can't modify read only gramma
	check_readonly();

	elementDecl& decl_ = const_cast< elementDecl& >(decl);
	wasAdded = false;
	// looking for attribute
	size_t hash_value = do_hash(name, os_minus_one);
	attribute_decl_map_t::pairii_t range = decl_._attributes.equal_range(hash_value);
	attribute_decl_map_t::iterator start(range.first);

	if (start != range.second) // can be found
		for (start = range.first; start != range.second; ++start)
			if (start->_name == name) // checks name directly
				return *start;

	if (mustPresent && decl._content != CONTENT_ANY)
	{
		xml_exception_throw("Unknown attribute found: ",
					name,
					" beneath parent element: ",
					(const char*)decl._name,
					0);
	}
	// inserts new attribute
	attributeDecl adecl(0, &_data_allocator);
	start = decl_._attributes.insert(_data_allocator, hash_value, adecl).first;
	if (start == decl_._attributes.end())
		xml_exception_throw("Not enough memory", 0);

	start->_name = name;
	start->_atype = ATTR_TYPE_CDATA;
	start->_ctype = vt_string;
	start->_rule = ATTR_RULE_IMPLIED;

	wasAdded = true;

	return *start;
}

const attributeDecl* 
xml_grammar::find_attribute_decl(const elementDecl& decl, const char* qname) const
{
	if (decl._attributes.empty())
		return 0;

	attribute_decl_map_t::paircc_t range = decl._attributes.equal_range(do_hash(qname, os_minus_one));
	for (attribute_decl_map_t::const_iterator start = range.first; start != range.second; ++start)
		if (start->_name == qname) // checks name directly
			return &*start;

	return 0;
}

attributeRefDecl& 
xml_grammar::add_reference_decl(const char* name, bool fromID)
{
	check_readonly();
	size_t hash_value = do_hash(name, os_minus_one);
	attribute_ref_decl_map_t::pairii_t range = _attributeRefMap.equal_range(hash_value);
	attribute_ref_decl_map_t::iterator start(range.first);

	if (start != range.second) // found
		for (start = range.first; start != range.second; ++start)
			if (start->_name == name) // checks name directly
				xml_exception_throw("Dublicate attribute reference name: ",
									(const char*)name,
									0);

	// inserts new notation
	attributeRefDecl decl(0, &_data_allocator);
	start = _attributeRefMap.insert(_data_allocator, hash_value, decl).first;
	if (start == _attributeRefMap.end())
		xml_exception_throw("Not enough memory", 0);

	start->_name = name;
	(fromID ? start->_declared : start->_refered) = true;

	return *start;
}

attributeRefDecl* 
xml_grammar::find_reference_decl(const char* name)
{
	if (_attributeRefMap.empty())
		return 0;

	size_t hash_value = do_hash(name, os_minus_one);
	attribute_ref_decl_map_t::pairii_t range = _attributeRefMap.equal_range(hash_value);

	for (attribute_ref_decl_map_t::iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}

namedNodeDecl& 
xml_grammar::add_pi_decl(const char* name)
{
	check_readonly();
	size_t hash_value = do_hash(name, os_minus_one);
	namednode_decl_map_t::pairii_t range = _piMap.equal_range(hash_value);
	namednode_decl_map_t::iterator start(range.first);

	if (start != range.second) // can be found
		for (start = range.first; start != range.second; ++start)
			if (start->_name == name) // checks name directly
				return *start;

	// insert new entity
	namedNodeDecl decl(PROCESSING_INSTRUCTION_NODE, 0, &_data_allocator);
	start = _piMap.insert(_data_allocator, hash_value, decl).first;
	if (start == _piMap.end())
		xml_exception_throw("Not enough memory", 0);

	start->_name = name;
	return *start;
}

const namedNodeDecl* 
xml_grammar::find_pi_decl(const char* name) const
{
	if (_piMap.empty())
		return 0;

	namednode_decl_map_t::paircc_t range = _piMap.equal_range(do_hash(name, os_minus_one));

	for (namednode_decl_map_t::const_iterator start = range.first; start != range.second; ++start)
		if (start->_name == name) // checks name directly
			return &*start;

	return 0;
}


void 
xml_grammar::resolve_references()
{
	for (attribute_ref_decl_map_t::const_iterator iter = _attributeRefMap.begin(); iter != _attributeRefMap.end(); ++iter)
		if (!iter->_declared && iter->_refered)
			xml_exception_throw("Undeclared ID: ", (const char*)iter->_name, 0);
}


/////////////////////////////////
void xml_exception_throw(const char* first, ...)
{
	va_list va;
	va_start(va, first);
	const char* entry = first;
	string_t ex;
	while (entry)
	{
		ex += entry;
		entry = va_arg(va, const char*);
	}
	va_end(va);

	exception::_throw(ex);
}

#pragma pack()
END_TERIMBER_NAMESPACE

