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

#ifndef _terimber_xmlmodel_h_
#define _terimber_xmlmodel_h_

#include "xml/sxml.h"
#include "base/bitset.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class content_node 
//! \brief DFA content node
class content_node
{
public:
	//! \brief destructor
	virtual
	~content_node() {}
	
	//! \brief constructor
	content_node(	dfaRule rule							//!< DFA rule
					);
	inline 
	void 
	init(			byte_allocator& allocator_,				//!< external allocator
					size_t sizeBit							//!< bit size
					);
	//! \brief returns the DFA ruke
	inline 
	dfaRule 
	get_rule() const;
	//! \brief returns the first bitset 
	inline 
	const _bitset& 
	get_firstPos() const;
	//! \brief returns the last bitset
	inline 
	const _bitset& 
	get_lastPos() const;

	//! \brief checks if the node is nullable
	virtual 
	bool 
	is_nullable() const = 0;
protected:
	//! \brief claculates position
	virtual 
	void 
	calc_pos(		_bitset& state,							//!< bitset of states
					bool first								//!< flag first or last
					) const = 0;

protected:
	dfaRule				_rule;								//!< DFA rule
	size_t				_max_states;						//!< max rules
	_bitset				_firstPos;							//!< first bitset
	_bitset				_lastPos;							//!< last bitset
};

//! \class content_any
//! \brief implements ANY content
class content_any : public content_node
{
public:
	//! \brief destructor
	virtual
	~content_any() {}
	
	//! \brief constructor
	content_any(	dfaRule rule,							//!< DFA rule
					size_t pos								//!< position
					);
	//! \brief returns position
	inline 
	size_t 
	get_pos() const;
	//! \brief checks if the node is nullable
	virtual 
	bool 
	is_nullable() const;

protected:
	//! \brief calculates position
	virtual 
	void 
	calc_pos(		_bitset& state,							//!< bitset
					bool
					) const;
private:
	size_t		_pos;										//!< position
};

//! \class content_leaf
//! \brief implements leaf node
class content_leaf : public content_node
{
public:
	//! \brief destructor
	virtual
	~content_leaf() {}
	
	content_leaf(	const elementDecl* decl,				//!< element declaration
					size_t pos								//!< position
					);
	//! \brief returns element declaration
	inline 
	const 
	elementDecl* 
	get_decl() const;
	//! \brief returns position
	inline 
	size_t 
	get_pos() const;
	//! \brief sets postition
	inline 
	void 
	set_pos(		size_t pos								//!< position
					);

	//! \brief checks if the node is nullable
	virtual 
	bool 
	is_nullable() const;

protected:
	//! \brief calculates postition
	virtual 
	void 
	calc_pos(		_bitset& state,							//!< bitset
					bool
					) const;

protected:
	const elementDecl*	_decl;								//!< element declaration
    size_t				_pos;								//!< position
};

//! \class content_unary
//! \brief implements unary node with one child
class content_unary : public content_node
{
public:
	//! \brief destructor
	virtual
	~content_unary() {}
	
	//! \brief constructor
	content_unary(	dfaRule rule,							//!< DFA rule
					content_node* child						//!< pointer to the child node
					);

	//! \brief returns child node
	inline 
	content_node* 
	get_child() const;
	//! \brief checks if node is nullable
	virtual 
	bool 
	is_nullable() const;
protected:
	//! \brief calculates position
	virtual 
	void 
	calc_pos(		_bitset& state,							//!< bitset
					bool first								//!< flag first or last
					) const;
private:
	content_node*	_child;									//!< pointer to the child node
};

//! \class content_binary
//! \brief implements binary node with two children
class content_binary : public content_node
{
public:
	//! \brief destructor
	virtual
	~content_binary() {}
	
	//! \brief constructor
	content_binary(	dfaRule rule,							//!< DFA rule
					content_node* left,						//!< pointer to the left node
					content_node* right						//!< pointer to the right node
					);
	//! \brief returns left node
	inline 
	content_node* 
	get_left() const;
	//! \brief returns right node
	inline 
	content_node* 
	get_right() const;

	//! \brief checks if the node is nullable
	virtual 
	bool 
	is_nullable() const;

protected:
	//! \brief calculates position
	virtual 
	void 
	calc_pos(		_bitset& state,							//!< bitset
					bool first								//!< flag first of last
					) const;
private:
	content_node*	_left;									//!< pointer to the left node
	content_node*	_right;									//!< pointer to the right node
};

//! \class content_interface
//! \brief abstract interface for validation
class content_interface
{
public:
	//! \brief destructor
	virtual 
	~content_interface();
	//! \brief validates element
	virtual 
	void 
	validate(		const xml_element& _el					//!< xml element
					) = 0;
};

//! \class content_mixed
//! \brief validator for mixed content
class content_mixed : public content_interface
{
public:	
	//! \brief constructor
	content_mixed(	const dfa_token* parent,				//!< parent DFA token
					byte_allocator& tmp_allocator			//!< temporary allocator
					);

	//! \brief validates xml element
	void 
	validate(		const xml_element& _el					//!< xml element
					);
private:
	//! \brief builds mixed list
	void 
	build_mixed_list(const dfa_token* parent				//!< parent DFA token
					);
private:
	byte_allocator&				_tmp_allocator;				//!< temporary allocator
	_list< const dfa_token* >	_listToken;					//!< list of DFA tokens
};

//! \class leaf_type
//! \brief keeps content for leaf
class leaf_type
{
public:
	//! \brief constructor
	leaf_type();
public:
	const elementDecl*	_decl;								//!< element declaration
	dfaRule				_rule;								//!< DFA rule
};

//! \class content_children
//! \brief implements children content validator
class content_children : public content_interface
{
public:	
	//! \brief constructor
	content_children(const dfa_token* parent,				//!< parent DFA token
					byte_allocator& tmp_allocator			//!< temporary allocator
					);
	//! \brief validates element
	virtual 
	void 
	validate(		const xml_element& el					//!< xml element
					);

private:
	//! \brief builds children tree
	content_node* 
	build_children_tree(const dfa_token* parent				//!< parent DFA token
					);
	//! \brief initializes current node by index
	size_t 
	post_tree_build_init(content_node* nodeCur,				//!< pointer to the content node
					size_t curIndex							//!< index
					);
	//! \brief calculates follow-up list
	void 
	calc_follow_list(content_node* curNode					//!< pointer to the content node
					);
	//! \brief constructs DFA
	void 
	build_dfa(		const dfa_token* parent					//!< parent DFA token
					);
	//! \brief creates default state list
	inline 
	void 
	make_def_state_list(_vector< size_t >& x				//! [out] state list
					) const;
private:
	byte_allocator&					_tmp_allocator;			//!< temporary allocator
	_vector< content_leaf* >		_leafList;				//!< list of content leaves
	_vector< dfaRule >				_leafListType;			//!< list of DFA rules
	_vector< _bitset >				_followList;			//!< list of state
	size_t							_leafCount;				//!< number of leaves
	size_t							_EOCPos;				//!< end of content flag
	bool							_emptyOk;				//!< flag if empty content is allowed

	_vector< const elementDecl* >	_elemMap;				//!< list of element declarations
	_vector< dfaRule >				_elemMapType;			//!< list of DFA rules for elements
	size_t							_elemMapSize;			//!< size of element list
	_vector< leaf_type >			_leafNameTypeVector;	//!< list of leaf types	
	_vector< _vector< size_t > >	_transTable;			//!< transaction table
	size_t							_transTableSize;		//!< size of transaction table
	_vector< bool >					_finalStateFlags;		//!< list of final states
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_xmlmodel_h_ 
