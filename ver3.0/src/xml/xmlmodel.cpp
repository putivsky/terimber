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
/*
The following code was partially adopted from XERCES parser project
*/

#include "xml/xmlmodel.hpp"
#include "xml/sxml.hpp"
#include "xml/sxs.hpp"

#include "base/common.hpp"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/vector.hpp"
#include "base/stack.hpp"
#include "base/map.hpp"
#include "base/bitset.hpp"
#include "base/string.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)
//////////////////////////////////////////////////////////
content_node::content_node(dfaRule rule) :
	_rule(rule), _max_states(0xffffffff)
{
}

////////////////////////////////////////////////////////////
content_any::content_any(dfaRule rule, size_t pos) : 
	content_node(rule), _pos(pos)
{
	assert(rule == DFA_ANY);
}

// virtual 
void 
content_any::calc_pos(_bitset& state, bool) const
{
	// if epsilon node, then the first pos is an empty set
	if (_pos == os_minus_one) 
		state.reset();
	else
		state.set(_pos, true);
}

// virtual 
bool 
content_any::is_nullable() const
{ 
	return _pos == os_minus_one; 
}

///////////////////////////////////////////////////////////
content_leaf::content_leaf(const elementDecl* decl, size_t pos) :
	content_node(DFA_LEAF), _decl(decl), _pos(pos)
{
}

// virtual 
void 
content_leaf::calc_pos(_bitset& state, bool) const
{
	// If we are an epsilon node, then the first pos is an empty set
	if (_pos == os_minus_one) 
		state.reset();
	else
		state.set(_pos, true);
}

// virtual  
bool 
content_leaf::is_nullable() const
{ 
	return _pos == os_minus_one; 
}

///////////////////////////////////////////////////////////
content_unary::content_unary(dfaRule rule, content_node* child) : 
	content_node(rule), _child(child)
{ 
	assert(rule == DFA_QUESTION || rule == DFA_ASTERISK || rule == DFA_PLUS);
}

// virtual 
bool 
content_unary::is_nullable() const
{ 
	return _rule == DFA_PLUS ? _child->is_nullable() : true; 
}
///////////////////////////////////////////////////////////
content_binary::content_binary(dfaRule rule, content_node* left, content_node* right) :
	content_node(rule), _left(left), _right(right) 
{ 
	assert(rule == DFA_CHOICE || rule == DFA_SEQUENCE);
}

// virtual
void 
content_binary::calc_pos(_bitset& state, bool first) const
{ 
	if (_rule == DFA_CHOICE)
	{
		state = first ? _left->get_firstPos() : _left->get_lastPos();
		state |= first ? _right->get_firstPos() : _right->get_lastPos();
	}
	else if (_rule == DFA_SEQUENCE)
	{
		state = first ? _left->get_firstPos() : _right->get_lastPos();
		if (first ? _left->is_nullable() : _right->is_nullable())
			state |= first ? _right->get_firstPos() : _left->get_lastPos();
	}
}

// virtual 
bool 
content_binary::is_nullable() const
{ 
	return _rule == DFA_CHOICE ?	(_left->is_nullable() || _right->is_nullable()) :
									(_left->is_nullable() && _right->is_nullable());
}


//////////////////////////////////////////////////////////////
// virtual 
content_interface::~content_interface() 
{
}

/////////////////////////////////////////////////////////////
leaf_type::leaf_type() : 
	_decl(0), _rule(dfaRule_MIN)
{
}

////////////////////////////////////////////////////////////
content_mixed::content_mixed(const dfa_token* parent, byte_allocator& tmp_allocator) :
	_tmp_allocator(tmp_allocator) 
{
	build_mixed_list(parent);
}

void 
content_mixed::build_mixed_list(const dfa_token* parent)
{
   // Gets the type of spec node our current node is
	switch (parent->_rule)
	{
		case DFA_LEAF:
		case DFA_ANY:
			_listToken.push_back(_tmp_allocator, parent);
			break;
		case DFA_CHOICE:
		case DFA_SEQUENCE:
			{
				// recurses on the left and right nodes
				build_mixed_list(parent->_first);
				// the last node of a choice or sequence has a null right
				if (parent->_last)
					build_mixed_list(parent->_last);
			}
			break;
		case DFA_QUESTION:
		case DFA_ASTERISK:
		case DFA_PLUS:
			build_mixed_list(parent->_first);
			break;
		default:
			assert(false);
	}
}

void
content_mixed::validate(const xml_element& el)
{
	if (!el.has_children())
		return;

	for (const xml_tree_node* iterElement = el._first_child; iterElement; iterElement = iterElement->_right)
	{
		if (iterElement->_decl->get_type() != ELEMENT_NODE)
			continue; // something which is not element

		const elementDecl* decl = xml_element::cast_to_element(iterElement)->cast_decl();
		bool find = false;
		
		for (_list< const dfa_token* >::const_iterator iterToken = _listToken.begin(); iterToken != _listToken.end(); ++iterToken)
		{
			if ((*iterToken)->_rule == DFA_LEAF)
			{
				if (!(*iterToken)->_decl) // PCDATA
					continue;

				if ((*iterToken)->_decl == decl)
				{
					find = true;
					break;
				}
			}
			else
			{
				// DFA_ANY:
				assert(false);
			}
		} // for

		if (!find)
		{
			string_t ex = "Unknown element found: ";
			ex += decl->_name;
			ex += " beneath parent element: ";
			ex += el._decl->_name;
			exception::_throw(ex);
		}
	} // for
}

/////////////////////////////////////////
content_children::content_children(const dfa_token* parent, byte_allocator& tmp_allocator) :
	_tmp_allocator(tmp_allocator), 
	_leafCount(0), _EOCPos(0),
	_emptyOk(false), _elemMapSize(0),
	_transTableSize(0)
{
	build_dfa(parent);
}

content_node* 
content_children::build_children_tree(const dfa_token* parent)
{
    // Gets the spec type of the passed node
	const dfaRule curRule = parent->_rule;

	switch (curRule)
	{
		case DFA_ANY:
			return new(_tmp_allocator.allocate(sizeof(content_any))) content_any(curRule, _leafCount++);
		case DFA_LEAF:
			return new(_tmp_allocator.allocate(sizeof(content_leaf))) content_leaf(parent->_decl, _leafCount++);
		case DFA_CHOICE:
		case DFA_SEQUENCE:
			{
				content_node* left = build_children_tree(parent->_first);
				content_node* right = build_children_tree(parent->_last);
  				return new(_tmp_allocator.allocate(sizeof(content_binary))) content_binary(curRule, left, right);
			}
		case DFA_QUESTION:
		case DFA_ASTERISK:
		case DFA_PLUS:
			return new(_tmp_allocator.allocate(sizeof(content_unary))) content_unary(curRule, build_children_tree(parent->_first));
		default:
			assert(false);
    }

    return 0;
}

size_t
content_children::post_tree_build_init(content_node* nodeCur, size_t curIndex)
{
    // initiates the maximum states on this node
	nodeCur->init(_tmp_allocator, _leafCount);

    // gets the spec type of the passed node
	dfaRule curRule = nodeCur->get_rule();

    // gets a copy of the index that we can modify
    size_t newIndex = curIndex;

    // processes a rule
	switch (curRule)
	{
		case DFA_ANY:
			_leafList[newIndex] = new(_tmp_allocator.allocate(sizeof(content_leaf))) content_leaf(0, ((content_any*)nodeCur)->get_pos());
			_leafListType[newIndex] = curRule;
			return ++newIndex;
		case DFA_CHOICE:
		case DFA_SEQUENCE:
			// recursion
			newIndex = post_tree_build_init(((content_binary*)nodeCur)->get_left(), newIndex);
			return post_tree_build_init(((content_binary*)nodeCur)->get_right(), newIndex);
		case DFA_ASTERISK:
		case DFA_QUESTION:
		case DFA_PLUS:
			// recursion
			return post_tree_build_init(((content_unary*)nodeCur)->get_child(), newIndex);
		case DFA_LEAF:
			//  puts this node in the leaf list at the current index if it is
			//  a non-epsilon leaf.
			if (((content_leaf*)nodeCur)->get_decl())
			{
				_leafList[newIndex] = new(_tmp_allocator.allocate(sizeof(content_leaf))) content_leaf(((content_leaf*)nodeCur)->get_decl(),
											((content_leaf*)nodeCur)->get_pos());
				_leafListType[newIndex] = curRule;
				return ++newIndex;
			}
			break;
		default:
			assert(false);
			break;
    }
    
	return newIndex;
}

void 
content_children::calc_follow_list(content_node* curNode)
{
    // gets the spec type of the passed node
	switch (curNode->get_rule())
	{
		case DFA_CHOICE:
			// recursion
			calc_follow_list(((content_binary*)curNode)->get_left());
			calc_follow_list(((content_binary*)curNode)->get_right());
			break;
		case DFA_SEQUENCE:
			{
				// recursion
				calc_follow_list(((content_binary*)curNode)->get_left());
				calc_follow_list(((content_binary*)curNode)->get_right());

				//  current level
				const _bitset& last  = ((content_binary*)curNode)->get_left()->get_lastPos();
				const _bitset& first = ((content_binary*)curNode)->get_right()->get_firstPos();

				//  for every position which is in our left child's last set
				//  adds all of the states in our right child's first set to the
				//  follow set for that position.
				for (size_t index = 0; index < _leafCount; ++index)
				{
					if (last.get(index))
						_followList[index] |= first;
				}
			}
			break;
		case DFA_ASTERISK:
		case DFA_PLUS:
			{
				// recursion
				calc_follow_list(((content_unary*)curNode)->get_child());

				//  current level. We use our own first and last position
				//  sets, so get them up front.
				const _bitset& first = curNode->get_firstPos();
				const _bitset& last  = curNode->get_lastPos();

				//  for every position which is in our last position set, adds all
				//  of our first position states to the follow set for that
				//  position.
				for (size_t index = 0; index < _leafCount; ++index)
				{
					if (last.get(index))
						_followList[index] |= first;
				}
		    }
			break;
		case DFA_QUESTION:
			// recursion
			calc_follow_list(((content_unary*)curNode)->get_child());
			break;
		default:
			break;
    } // switch
}

void 
content_children::build_dfa(const dfa_token* parent)
{
   size_t index;
	//  The conversions done are:
	//
	//  x+ -> (x|x*)
	//  x? -> (x|empty)
    //  Note that, during this operation, we set each non-epsilon leaf node's
    //  DFA state position and count the number of such leafs, which is left
    //  in the _leafCount member.

	elementDecl fakeDecl(0, &_tmp_allocator);
   	content_leaf* nodeEOC = new(_tmp_allocator.allocate(sizeof(content_leaf))) content_leaf(&fakeDecl, 0xffffffff);
    content_node* nodeOrgContent = build_children_tree(parent);
	content_node* headNode = new(_tmp_allocator.allocate(sizeof(content_binary)))content_binary(DFA_SEQUENCE, nodeOrgContent, nodeEOC);

    //
    //  And handle specifically the EOC node, which must also be numbered and
    //  counted as a non-epsilon leaf node. It could not be handled in the
    //  above tree build because it was created before all that started. We
    //  save the EOC position since its used during the DFA building loop.
    //
    _EOCPos = _leafCount;
    nodeEOC->set_pos(_leafCount++);

    //
    //  Ok, so now we have to iterate the new tree and do a little more work
    //  now that we know the leaf count. One thing we need to do is to
    //  calculate the first and last position sets of each node. This is
    //  cached away in each of the nodes.
    //
    //  Along the way we also set the leaf count in each node as the maximum
    //  state count. They must know this in order to create their first/last
    //  position sets.
    //
    //  We also need to build an array of references to the non-epsilon
    //  leaf nodes. Since we iterate here the same way as we did during the
    //  initial tree build (which built their position numbers, we will put
    //  them in the array according to their position values.
    //
	_leafList.resize(_tmp_allocator, _leafCount);
	_leafListType.resize(_tmp_allocator, _leafCount);

    post_tree_build_init(headNode, 0);

    //
    //  And, moving on... We now need to build the follow position sets
    //  for all the nodes, so we allocate an array of pointers to state sets,
    //  one for each leaf node (i.e. each significant DFA position.)
    //
	_followList.resize(_tmp_allocator, _leafCount);
    
	for (index = 0; index < _leafCount; ++index)
		_followList[index].resize(_tmp_allocator, _leafCount);

    calc_follow_list(headNode);

    //
    //  Checks to see whether this content model can handle an empty content,
    //  which is something we need to optimize by looking before we
    //  throw away the info that would tell us that.
    //
    //  If the left node of the head (the top level of the original content)
    //  is nullable, then its true.
    //
    _emptyOk = nodeOrgContent->is_nullable();

    //
    //  And finally the big push... Now we build the DFA using all the states
    //  and the tree we've built up. First we set up the various data
    //  structures we are going to use while we do this.
    //
    //  First of all we need an array of unique element ids in our content
    //  model. For each transition table entry, we need a set of contiguous
    //  indices to represent the transitions for a particular input element.
    //  So we need to a zero based range of indexes that map to element types.
    //  This element map provides that mapping.
    //
    _elemMap.resize(_tmp_allocator, _leafCount);
	_elemMapType.resize(_tmp_allocator, _leafCount);
    _elemMapSize = 0;

	bool init_LeafNameTypeVector = false;
    for (size_t outIndex = 0; outIndex < _leafCount; ++outIndex)
    {
        if (_leafListType[outIndex] != DFA_LEAF)
            if (!init_LeafNameTypeVector)
                init_LeafNameTypeVector = true;

        // Gets the current leaf's element index
		const elementDecl* decl = _leafList[outIndex]->get_decl();

        // Sees if the current leaf node's element index is in the list
        size_t inIndex = 0;

		for (; inIndex < _leafCount; ++inIndex)
		{
			const elementDecl* inDecl = _elemMap[inIndex];
			if (inDecl == decl) 
				break;
        }

        // If it was not in the list, then add it and bump the map size
        if (inIndex == _leafCount)
        {
			_elemMap[_elemMapSize] = decl;
			_elemMapType[_elemMapSize] = _leafListType[outIndex];
            ++_elemMapSize;
        }

    }

	_elemMap.reduce(_elemMapSize);
	_elemMapType.reduce(_elemMapSize);


    // sets up the fLeafNameTypeVector object if there is one.
    if (init_LeafNameTypeVector) 
	{
		_leafNameTypeVector.resize(_tmp_allocator, _elemMapSize);
		for (size_t indexCopy = 0; indexCopy < _elemMapSize; ++indexCopy)
		{
			_leafNameTypeVector[indexCopy]._decl = _elemMap[indexCopy];
			_leafNameTypeVector[indexCopy]._rule = _elemMapType[indexCopy];
		}
	}

	_vector< size_t > leafSorter;
	leafSorter.resize(_tmp_allocator, _leafCount + _elemMapSize);
    size_t sortCount = 0;

    for (size_t elemIndex = 0; elemIndex < _elemMapSize; ++elemIndex)
    {
        const elementDecl* decl = _elemMap[elemIndex];
		
		for (size_t leafIndex = 0; leafIndex < _leafCount; ++leafIndex)
		{
			const elementDecl* leaf = _leafList[leafIndex]->get_decl();
			if (leaf == decl) 
				leafSorter[sortCount++] = leafIndex;
        }

        leafSorter[sortCount++] = os_minus_one;
    }

	//
    //  Next lets create some arrays, some that that hold transient info
    //  during the DFA build and some that are permament. These are kind of
    //  sticky since we cannot know how big they will get, but we don't want
    //  to use any collection type classes because of performance.
    //
    //  Basically they will probably be about _leafCount*2 on average, but can
    //  be as large as 2^(_leafCount*2), worst case. So we start with
    //  _leafCount*4 as a middle ground. This will be very unlikely to ever
    //  have to expand though, it if does, the overhead will be somewhat ugly.
    //
    size_t curArraySize = _leafCount * 4;
	_vector< _bitset > statesToDo;
	statesToDo.resize(_tmp_allocator, curArraySize);
    
	_finalStateFlags.resize(_tmp_allocator, curArraySize);

	_transTable.resize(_tmp_allocator, curArraySize); 

	//
    //  We start with the initial set as the first pos set of the head node
    //  (which is the seq node that holds the content model and the EOC node.)
    //
	_bitset setT;
	setT.assign(_tmp_allocator, headNode->get_firstPos());

    //
    //  Inits our two state flags. Basically the unmarked state counter is
    //  always chasing the current state counter. When it catches up, that
    //  means we made a pass through that did not add any new states to the
    //  lists, at which time we are done. We could have used a expanding array
    //  of flags which we used to mark off states as we complete them, but
    //  this is easier though less readable maybe.
    //
    size_t unmarkedState = 0;
    size_t curState = 0;

    //
    //  Inits the first transition table entry, and puts the initial state
    //  into the states to do list, then bumps the current state.
    //
    make_def_state_list(_transTable[curState]);
	statesToDo[curState].assign(_tmp_allocator, setT);
    ++curState;

    //
    // the stateTable is an auxiliary means to fast
    // identification of new state created (instead
    // of squential loop statesToDo to find out),
    // while the role that statesToDo plays remain unchanged.
	_map< _bitset, size_t > stateTable;
    
    //
    //  Ok, almost done with the algorithm from hell... We now enter the
    //  loop where we go until the states done counter catches up with
    //  the states to do counter.
    //
    _bitset newSet;
    while (unmarkedState < curState)
    {
        //
        //  Gets the next unmarked state out of the list of states to do
        //  and get the associated transition table entry.
        //
		setT.assign(_tmp_allocator, statesToDo[unmarkedState]);
        _vector< size_t >& transEntry = _transTable[unmarkedState];

        // Marks this one final if it contains the EOC state
		_finalStateFlags[unmarkedState] = setT.get(_EOCPos);

        // Bumps up the unmarked state count, marking this state done
        ++unmarkedState;

        size_t sorterIndex = 0;

        // Loops through each possible input symbol in the element map
        for (size_t elemIndex = 0; elemIndex < _elemMapSize; ++elemIndex)
        {
            //
            //  Builds up a set of states which is the union of all of the
            //  follow sets of DFA positions that are in the current state. If
            //  we gave away the new set last time through then create a new
            //  one. Otherwise, zero out the existing one.
            //
			if (newSet.empty())
				newSet.resize(_tmp_allocator, _leafCount);
            else
				newSet.reset();

            size_t leafIndex = leafSorter[sorterIndex++];

            while (leafIndex != os_minus_one)
            {
                // If this leaf index (DFA position) is in the current set...
				if (setT.get(leafIndex))
                {
                    //
                    //  If this leaf is the current input symbol, then we
                    //  want to add its follow list to the set of states to
                    //  transition to from the current state.
                    //
                    newSet |= _followList[leafIndex];
                }

                leafIndex = leafSorter[sorterIndex++];
            } // while (leafIndex != os_minus_one)

            //
            //  If this new set is not empty, then see if its in the list
            //  of states to do. If not, then add it.
            //
			if (!newSet.empty())
            {
                //
                //  Searches the 'states to do' list to see if this new
                //  state set is already in there.
                //

				_map< _bitset, size_t >::iterator iter = stateTable.find(newSet);
                // If we did not find it, then add it
				size_t stateIndex = curState;

                if (iter == stateTable.end())
                {
                    //
                    //  Puts this new state into the states to do and inits
                    //  a new entry at the same index in the transition
                    //  table.
                    //
					statesToDo[curState].assign(_tmp_allocator, newSet);
                    make_def_state_list(_transTable[curState]);
					iter = stateTable.insert(_tmp_allocator, newSet, curState).first;
					
                    // We now have a new state to do so bump the count
                    ++curState;

                    //
                    //  Null out the new set to indicate we adopted it. This
                    //  will cause the creation of a new set on the next time
                    //  around the loop.
                    //
					newSet.clear();
                }
				else
					stateIndex = *iter;


                //
                //  Sets this state in the transition table's entry for this
                //  element (using its index), with the DFA state we will move
                //  to from the current state when we see this input element.
                //
                transEntry[elemIndex] = stateIndex;

                // Expand the arrays if we're full
                if (curState == curArraySize)
                {
                    //
                    //  Yikes, we overflowed the initial array size, so we
                    //  need to expand all of these arrays. So adjust the
                    //  size up by 100% and allocate new arrays.
                    //
					curArraySize *= 4;
					_finalStateFlags.resize(_tmp_allocator, curArraySize);
					_transTable.resize(_tmp_allocator, curArraySize);
					statesToDo.resize(_tmp_allocator, curArraySize);
	            } // if (curState == curArraySize)
            } // if 
        } // for elemIndex
    } // while

	// Stores the current state count in the trans table size
	_transTableSize = curState;
 }

void
content_children::validate(const xml_element& el)
{
  //
    //  If there are no children, then either we fail on the 0th element
    //  or we return success. It depends upon whether this content model
    //  accepts empty content, which we determined earlier.
    //
	if (!el.has_children())
    {
        // success os_minus_one
        if (!_emptyOk)
		{
			string_t ex = "Parent element: ";
			ex += el._decl->_name;
			ex += " must contain child elements";
			exception::_throw(ex);
		}

		return;
    }

    //
    //  Lets loop through the children in the array and move our way
    //  through the states. Note that we use the _elemMap array to map
    //  an element index to a state index.
    //
    size_t curState = 0;
    size_t nextState = 0;
	// look for children
	for (const xml_tree_node* iterElement = el._first_child; iterElement; iterElement = iterElement->_right)
	{
		if (iterElement->_decl->get_type() != ELEMENT_NODE)
			continue; // something that is not element
	
		// Gets the current element index out
		const elementDecl* curDecl = xml_element::cast_to_element(iterElement)->cast_decl();

		if (curDecl->_content == CONTENT_ANY)
			continue;
	
		// Looks up this child in our element map
		size_t elemIndex = 0;
        for (; elemIndex < _elemMapSize; ++elemIndex)
        {
            const elementDecl* inElem  = _elemMap[elemIndex];
			if (inElem == curDecl)
			{
				nextState = _transTable[curState][elemIndex];
				if (nextState != os_minus_one)
					break;
            }
        }//for elemIndex

        // If "nextState" is os_minus_one, we have found a match, but the transition is invalid
        if (nextState == os_minus_one)
		{
			string_t ex("Invalid child element order: ");
			ex += curDecl->_name;
			ex += " beneath parent element: ";
			ex += el._decl->_name;
			exception::_throw(ex);
		}

        // If we didn't find it, then obviously not valid
		if (elemIndex == _elemMapSize)
		{
			string_t ex("Unexpected child element: ");
			ex += curDecl->_name;
			ex += " beneath parent element: ";
			ex += el._decl->_name;
			exception::_throw(ex);
		}

        curState = nextState;
        nextState = 0;
    }

    //
    //  We transitioned all the way through the input list. However, that
    //  does not mean that we ended in a final state. So check whether
    //  our ending state is a final state.
    //
    if (!_finalStateFlags[curState])
	{
		string_t ex("Expected more child elements beneath parent element: ");
		ex += el._decl->_name;
		exception::_throw(ex);
	}
}

#pragma pack()
END_TERIMBER_NAMESPACE

