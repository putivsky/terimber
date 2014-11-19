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

#ifndef _terimber_sxs_h_
#define _terimber_sxs_h_

#include "xml/defxml.h"
#include "xml/xmlaccss.h"
#include "xmltypes.h"

#include "alg/algorith.h"
#include "base/map.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \brief throw exception formatting output
void xml_exception_throw(const char* first, ...);
//! \brief big xml size
const size_t big_xml_size = 1024*1024;

//! \enum attributeType
//! \brief possible type for attributes, entities, notations, and others - except elements
enum attributeType
{
	attributeType_MIN,										//!< lower bounder
	ATTR_TYPE_CDATA,										//!< string
	ATTR_TYPE_ID,											//!< ident
	ATTR_TYPE_IDREF,										//!< reference to ident
	ATTR_TYPE_IDREFS,										//!< references to ident
	ATTR_TYPE_ENTITY,										//!< entity
	ATTR_TYPE_ENTITIES,										//!< entities
	ATTR_TYPE_NOTATION,										//!< notation
	ATTR_TYPE_NMTOKEN,										//!< nmToken
	ATTR_TYPE_NMTOKENS,										//!< nmTokens
	ATTR_TYPE_ENUMERATION,									//!< enumeration
	attributeType_MAX										//!< upper bounder
};

//! \enum attributeRule
//! \brief types for rules
enum attributeRule
{
	attributeRule_MIN,										//!< lower bounder
	ATTR_RULE_DEFAULT,										//!< default
	ATTR_RULE_REQUIRED,										//!< required
	ATTR_RULE_IMPLIED,										//!< implied
	ATTR_RULE_FIXED,										//!< fixed
	attributeRule_MAX										//!< upper bounder	
};

//! \enum contentSpec
//! \brief contentSpec will add additional logic rules
//! 'EMPTY' | 'ANY' | Mixed | children
enum contentSpec
{
	contentSpec_MIN,										//!< lower bounder
	CONTENT_EMPTY,											//!< EMPTY
	CONTENT_ANY,											//!< ANY
	//! \brief [51] Mixed ::= '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' | '(' S? '#PCDATA' S? ')'
	CONTENT_MIXED,											//!< mixed
	//! \brief [47] children ::= (choice | seq) ('?' | '*' | '+')? 
	//! [48] cp ::= (Name | choice | seq) ('?' | '*' | '+')? 
	//! [49] choice ::= '(' S? cp ( S? '|' S? cp )+ S? ')'
	//! [50] seq ::= '(' S? cp ( S? ',' S? cp )* S? ')'
	CONTENT_CHILDREN,										//!< children content
	contentSpec_MAX											//!< upper bounder
};

//! \enum dfaRule
//! \brief modelRule describes the DFA
enum dfaRule
{
	dfaRule_MIN,											//!< lower bounder
	DFA_LEAF,												//!< leaf
	DFA_QUESTION,											//!< '?' - Zero or One 
	DFA_ASTERISK,											//!< '*' - Zero or More
	DFA_PLUS,												//!< '+' - One or More
	DFA_CHOICE,												//!< (a|b)
	DFA_SEQUENCE,											//!< (a,b)
	DFA_ANY,												//!< ANY
	dfaRule_MAX												//!< upper bounder
};

//! \class nodeDecl
//! \brief base class for all xml nodes
class nodeDecl
{
	//! \brief private assign operator
	nodeDecl& operator=(const nodeDecl& x);
public:
	//! \brief constructor
	nodeDecl(		xmlNodeType type						//!< xml type
					);
	//! \brief copy constructor
	nodeDecl(const nodeDecl& x);
	//! \brief destructor
	~nodeDecl();

	//! \brief gets xml type
	inline 
	xmlNodeType 
	get_type() const;

protected:
	xmlNodeType		_type;									//!< xml type
};

//! \class namedNode
//! \brief xml node with name
class namedNode
{
	//! \brief private assign operator
	namedNode& operator=(const namedNode& x);
public:
	//! \brief constructor
	namedNode(		const char* name,						//!< node name
					byte_allocator* allocator				//!< optional external allocator
					);
	//! \brief copy constructor
	namedNode(const namedNode& x);
	//! \brief clear
	void 
	clear();

public:
	string_t	_name;										//!< xml node name
};

//! \class namedNodeDecl
//! \brief xml node with name and type
class namedNodeDecl : public nodeDecl, 
						public namedNode
{
	//! \brief private assign operator
	namedNodeDecl& operator=(const namedNodeDecl& x);
public:
	//! \brief constructor
	namedNodeDecl(	xmlNodeType type,						//!< xml type
					const char* name,						//!< node name
					byte_allocator* allocator_				//!< optional external allocator
					);
	//! \brief copy constructor
	namedNodeDecl(const namedNodeDecl& x);
	//! \brief clears resources
	void 
	clear();
};

//! \class enumNodeDecl
//! \brief if attribute value is enum type we are going to keep the correspondent ident
class enumNodeDecl
{
	//! \brief private assign operator
	enumNodeDecl& operator=(const enumNodeDecl& x);
public:
	//! \brief constructor
	enumNodeDecl(	byte_allocator* allocator_				//!< optional external allocator
					);
	//! \brief copy constructor
	enumNodeDecl(const enumNodeDecl& x);

public:
	
	size_t		_id;										//!< ident that corresponds to the name from enum
	string_t	_value;										//!< enumaration value
};

//! \class attributeDecl
//! \brief attribute description
class attributeDecl : public namedNodeDecl
{
	//! \brief private assign operator
	attributeDecl& operator=(const attributeDecl& x);
public:
	//! \brief constructor
	attributeDecl(	const char* name,						//!< attribute name
					byte_allocator* allocator_				//!< optional external operator
					);
	//! \brief copy constructor
	attributeDecl(const attributeDecl& x);
	//! \brief persists attribute value correctly
	const char* 
	persist_attribute(const terimber_xml_value& value,		//!< input xml value
					byte_allocator* allocator_				//!< external allocator
					) const;
	
public:
	attributeType			_atype;							//!< attrbute type CDATA, ENTITY, ...
	vt_types				_ctype;							//!< types like vt_sb4, ...
	_list< enumNodeDecl >	_enum;							//!< enum of values if applied
	string_t				_defval;						//!< default value if applied
	attributeRule			_rule;							//!< rule
};

//! \class notationDecl
//! \brief notation description
class notationDecl : public namedNodeDecl
{
	//! \brief assign operator
	notationDecl& operator=(const notationDecl& x);
public:
	//! \brief constructor
	notationDecl(	const char* name,						//!< notation name
					byte_allocator* allocator_				//!< optional external allocator
					);
	//! \brief copy constructor
	notationDecl(const notationDecl& x);
public:
	string_t			_systemId;							//!< system ident
	string_t			_publicId;							//!< public ident
};

//! \class entityDecl
//! \brief entity description
class entityDecl : public namedNodeDecl
{
	//! \brief assign operator
	entityDecl& operator=(const entityDecl& x);
public:
	//! \brief constructor
	entityDecl(		const char* name,						//!< entity name
					byte_allocator* allocator_				//!< optional external allocator
					);
	//! \brief copy constructor
	entityDecl(const entityDecl& x);

	string_t			_systemId;							//!< system ident
	string_t			_publicId;							//!< public ident
    string_t			_notation;							//!< notation
	string_t			_value;								//!< value
    
	bool				_is_in_subset;						//!< flag in subset
    bool				_is_parameter;						//!< flag entity parameter
    bool				_is_encoded_char;					//!< flag encoded char
	bool				_is_unparsed;						//!< flag unparsed entity
};

// forward declaration
class dfa_token;
//! \typedef attribute_decl_map_t
//! \brief maps attribute declaration ident to the attribute declaration
typedef _map< size_t, attributeDecl, byte_allocator, less< size_t >, true > attribute_decl_map_t;

//! \class elementDecl
//! \brief element schema declaration
class elementDecl : public namedNodeDecl
{
	//! \brief private assign operator
	elementDecl& operator=(const elementDecl& x);
public:
	//! \brief constructor
	elementDecl(	const char* name,						//!< element name
					byte_allocator* allocator_				//!< optional external allocator
					);
	//! \brief copy constructor
	elementDecl(const elementDecl& x);

public:
	contentSpec				_content;						//!< content
	dfa_token*				_token;							//!< list of rule for child' elements
	attribute_decl_map_t	_attributes;					//!< attributes
};

//! \class attributeRefDecl
//! \brief map for id/idref/idrefs declarations
class attributeRefDecl : public namedNode
{
	//! \brief private assign operator
	attributeRefDecl& operator=(const attributeRefDecl& x);
public:
	//! \brief constructor
	attributeRefDecl(const char* name,						//!< attribute name
					byte_allocator* allocator_				//!< optional external allocator
					);
	//! \brief copy constructor
	attributeRefDecl(const attributeRefDecl& x);

public:
	bool	_declared;										//!< flag attribute is declared
	bool	_refered;										//!< flag attribute has a reference
};

//! \class dfa_token
//! \brief class contains rule token
class dfa_token
{
public:
	//! \brief default constructor
	dfa_token();
	//! \brief destructor
	~dfa_token();
	//! \brief copy constructor with external allocator
	dfa_token(		byte_allocator& allocator_,				//!< external allocator
					const dfa_token& x						//!< input value
					);
	//! \brief constructor from created rule
	dfa_token(		dfaRule rule,							//!< external rule 
					const elementDecl* decl,				//!< element declaration
					dfa_token* first,						//!< pointer to the first child
					dfa_token* last							//!< pointer to the last child
					);

	//! \brief assigns
	void 
	assign(			byte_allocator& allocator_,				//!< external allocator
					const dfa_token& x						//!< input value
					);

	//! \brief find ANY recursively
	bool
	find_any_resursively() const;
	
	dfaRule				_rule;								//!< DFA rule for this token
	const elementDecl*	_decl;								//!< pointer to the element schema if applied
	dfa_token*		_first;									//!< first dfa entry
	dfa_token*		_last;									//!< last dfa entry
};

//! \typedef namednode_decl_map_t
//! \brief maps ident to the named node declaration
typedef _map< size_t, namedNodeDecl, byte_allocator, less< size_t >, true > namednode_decl_map_t;
//! \typedef element_decl_map_t
//! \brief maps ident to the element node declaration
typedef _map< size_t, elementDecl, byte_allocator, less< size_t >, true > element_decl_map_t;
//! \typedef notation_decl_map_t
//! \brief maps ident to the notation node declaration
typedef _map< size_t, notationDecl, byte_allocator, less< size_t >, true > notation_decl_map_t;
//! \typedef entity_decl_map_t
//! \brief maps ident to the entity node declaration
typedef _map< size_t, entityDecl, byte_allocator, less< size_t >, true > entity_decl_map_t;
//! \typedef attribute_ref_decl_map_t
//! \brief maps ident to the attribute reference node declaration
typedef _map< size_t, attributeRefDecl, byte_allocator, less< size_t >, true > attribute_ref_decl_map_t;

//! \class xml_grammar
//! \brief DTD rules internal presentation
class xml_grammar
{
	//! prevent copy
	//! \brief copy constructor
	xml_grammar(const xml_grammar& x);
	//! \brief assign operator
	xml_grammar& operator=(const xml_grammar& x);

public:
	//! \brief constructor
	xml_grammar(	mem_pool_t& small_manager,				//!< small memory pool
					mem_pool_t& big_manager,				//!< big memory pool
					size_t xml_size							//!< xml size - just a tip
					);
	//! \brief destructor
	~xml_grammar();
	//! \brief makes a full copy of external gramma
	xml_grammar& 
	copy(			const xml_grammar& x					//!< input value
					);
	//! \brief adds element declaration to the grammar
	elementDecl& 
	add_element_decl(const char* name,						//!< element name
					bool on_fly,							//!< flag to create grammar on a fly
					bool fromDecl,							//!< flag creating from declaration
					bool modelAny							//!< flag of ANY rules are allowed
					);
	//! \brief tries to find the element declaration by name
	elementDecl* 
	find_element_decl(const char* name						//!< element declaration name
					);
	//! \brief tries to find the const element declaration by name
	const elementDecl* 
	find_element_decl(const char* name						//!< element declaration name
					) const;
	//! \brief adds the notation declaration to the grammar
	notationDecl& 
	add_notation_decl(const char* name						//!< notation declaration name
					);
	//! \brief tries to find the const by name
	const notationDecl* 
	find_notation_decl(const char* name						//!< notation declaration name
					) const;
	//! \brief adds the entity declaration to the grammar
	entityDecl& 
	add_entity_decl(const char* name,						//!< entity declaration name
					bool& wasAdded
					);
	//! \brief finds the entity declaration by name
	entityDecl* 
	find_entity_decl(const char* name						//!< entity declaration name
					);
	//! \brief finds the const entity declaration by name
	const entityDecl* 
	find_entity_decl(const char* name						//!< entity declaration name
					) const;
	//! \brief adds/finds the attribute declaration to the grammar
	attributeDecl& 
	add_attribute_decl(const elementDecl& decl,				//!< parent element declaration
					const char* name,						//!< attibute name
					bool mustPresent,						//!< flag attribute must be found 
					bool& wasAdded							//!< flag if attribute has been created
					);
	//! \brief finds the attribute by name and parent element declaration
	const attributeDecl* 
	find_attribute_decl(const elementDecl& decl,			//!< parent element declaration
					const char* name						//!< attibute name
					) const;
	//! \brief adds the reference to the declaration
	attributeRefDecl& 
	add_reference_decl(const char* name,					//!< attribute name
					bool fromID								//!< declared or refrenced
					);
	//! \brief finds the attribute refrence declaration
	attributeRefDecl* 
	find_reference_decl(const char* name					//!< attribute name
					);
	//! \brief resolves all refrences to the their attribute
	void 
	resolve_references();
	//! \brief adds PI declaration
	namedNodeDecl& 
	add_pi_decl(	const char* name						//!< pi name
					);
	//! \brief tries to find the pi declaration by pi name
	const namedNodeDecl* 
	find_pi_decl(	const char* name						//!< pi name
					) const;
	//! \brief gets the data allocator
	inline 
	byte_allocator& 
	get_data_allocator();
	//! \brief gets the temporary allocator
	inline 
	byte_allocator& 
	get_tmp_allocator();
	//! \brief gets the document declaration
	inline 
	const namedNodeDecl* 
	get_document_decl() const;
	//! \brief gets the DTD node declaration
	inline 
	const namedNodeDecl* 
	get_doctype_decl() const;
	//! \brief gets the comment declaration
	inline 
	const namedNodeDecl* 
	get_comment_decl() const;
	//! \brief gets the cdata declaration
	inline 
	const namedNodeDecl* 
	get_cdata_decl() const;
	//! \brief clears resources
	inline 
	void 
	clear();
	//! \brief returns element map
	inline 
	const element_decl_map_t& 
	get_elementMap() const;
	//! \brief returns pi map
	inline 
	const namednode_decl_map_t& 
	get_piMap() const;
	//! \brief returns entity map
	inline 
	const entity_decl_map_t& 
	get_entityMap() const;
	//! \brief gets the notation map
	inline 
	const notation_decl_map_t& 
	get_notationMap() const;
private:
	//! \brief checks if this grammar is read-only
	xml_forceinline 
	void 
	check_readonly();
	
protected:
	mem_pool_t&					_small_manager;				//!< small memory pool
	mem_pool_t&					_big_manager;				//!< big memory pool
	size_t						_xml_size;					//!< xml size - just a tip
	byte_allocator&				_data_allocator;			//!< data allocator
	byte_allocator&				_tmp_allocator;				//!< temporary allocator
	element_decl_map_t			_elementMap;				//!< element map
	notation_decl_map_t			_notationMap;				//!< notation map
	entity_decl_map_t			_entityMap;					//!< entity map
	attribute_ref_decl_map_t	_attributeRefMap;			//!< attribute refrence map
	namednode_decl_map_t		_piMap;						//!< pi map

	//! node declaration we need as a single instance
	namedNodeDecl				_document_decl;				//!< document node declaration
	namedNodeDecl				_doctype_decl;				//!< dtd node declaration	
	namedNodeDecl				_text_decl;					//!< text node declaration
	namedNodeDecl				_comment_decl;				//!< comment node declaration
	namedNodeDecl				_cdata_decl;				//!< cdata node declaration
	bool						_read_only;					//!< flag read-only grammar

public:
	string_t					_doc_name;					//!< document name
	string_t					_public_id;					//!< public ident
	string_t					_system_id;					//!< system ident
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_sxs_h_ 
