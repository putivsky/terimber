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

#ifndef _terimber_template_h_
#define _terimber_template_h_

#include "base/primitives.h"
#include "base/list.h"
#include "base/keymaker.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)
//! \class pool
//! \brief class implementating the template pool of objects
template < class C > //! creator - responsable for creation and deleting objects
class pool
{
public:
	// VC 6 specific
	//! \typedef CREATOR
	//! \brief responsible for creation and deletion of objects
	typedef C CREATOR;
	//! \typedef TYPE
	//! \brief type of object in pool
	typedef TYPENAME C::TYPE TYPE;
	//! \typedef ARG
	//! \brief argument for creator
	typedef TYPENAME C::ARG ARG;
private:
	//! \class pool_entry
	//! \brief pointer to a pool entry object
	//! we can't remove internl class into hpp file
	//! Microsoft specific
	class pool_entry
	{
		//! \friend pool< C >
		//! \brief gives pool full access
		friend class pool< C >;
	public:
		//! \brief constructor 
		pool_entry(	TYPE* obj								//!< pointer to object
					) : 
			_obj(obj), 
			_rest(0) 
		{
		}
		//! \brief copy constructor
		pool_entry(const pool_entry& x) 
		{ 
			*this = x; 
		}
		//! \brief assign operator
		pool_entry& 
		operator=(const pool_entry& x)
		{
			if (this != &x)
			{
				_obj = x._obj;
				_rest = x._rest;
			}
			return *this;
		}

	private:
		TYPE*			_obj;								//!< keep pointer to the object
		sb8_t			_rest;								//!< when last time in the rest
	};


	//! \typedef list_pool_entry_t
	//! \brief keeps the object entries as a list
	typedef list< pool_entry > list_pool_entry_t;

public:
	//! \brief constructor
	pool< C >(		C& creator = C::static_constructor(),	//!< creator
					size_t pool_size = os_def_size			//!< max pool capacity
					);
	//! \brief destructor
	~pool< C >();
	//! \brief returns object if it's in a pool within specified timeout
	inline 
	TYPE* 
	loan_object(	const ARG& arg = C::get_default_arg(),	//!< argument for creator
					size_t timeout = C::get_default_timeout() //!< timeout in milliseconds
					);
	//! \brief returns object back to pool
	inline 
	void 
	return_object(	TYPE* obj,								//!< pointer to object
					const ARG& arg = C::get_default_arg()	//!< argument for creator
					);
	//! \brief clears pool
	inline 
	void 
	clear(			const ARG& arg = C::get_default_arg()	//!< argument for creator
					);
	//! \brief deactivates unused objects in pool
	inline 
	void 
	deactivate(		size_t maxrest,							//!< max time allowed for object to rest in milliseconds
					const ARG& arg = C::get_default_arg()	//!< argument for creator
					);
    //! \brief purges(deletes) unused objects in pool
	inline 
	void 
	purge(			size_t maxrest,							//!< max time allowed for object to rest in milliseconds
					const ARG& arg = C::get_default_arg()	//!< argument for creator
					);

	//! \brief gets statistics
	inline 
	void 
	get_stats(		size_t& free_objects,					//!< available objects
					size_t& busy_objects					//!< objects in use
					) const;
private:
	C&						_creator;						//!< creator reference
	keylocker				_locker;						//!< read/write locker
	mutex					_mtx;							//!< controls multithreaded access to pool
	list_pool_entry_t		_busy_objects;					//!< pool of taken objects
	list_pool_entry_t		_free_objects;					//!< pool of available objects
};

//! \class smart_pointer
//! \brief template class supports smart allocation and deallocation the pointer to the objects
//! creator must provide two function T* create(size_t) & destroy(T*)
template < class C > //! type of creator
class smart_pointer
{
public:
	//! \typedef TYPE
	//! \brief type of object
	typedef TYPENAME C::TYPE TYPE;
	//! \typedef ARG
	//! \brief argumnet for creator
	typedef TYPENAME C::ARG ARG;
	//! \brief constructor
	//! object internally created
	explicit 
	smart_pointer< C >(C& crt,								//!< creator reference
					const ARG& n							//!< argument
					);
	//! \brief constructor
	//! object will later assign explicitly
	explicit 
	smart_pointer< C >(C& crt								//!< creator reference
					);
	//! \brief destructor
	~smart_pointer< C >();
	//! \brief assign operator
	smart_pointer< C >& operator=(const TYPE* x);
	//! \brief operator TYPE*
	operator TYPENAME smart_pointer< C >::TYPE*() 
	{ 
		return _ptr; 
	}
	//! \brief operator const TYPE*
	operator 
	const TYPENAME smart_pointer< C >::TYPE*() const 
	{ 
		return _ptr; 
	}
	//! \brief operator->
	TYPENAME smart_pointer< C >::TYPE* operator->();
	//! \brief operator-> (const)
	const TYPENAME smart_pointer< C >::TYPE* operator->() const;
	//! \brief access operator to the address of pointer to object
	//! interface support
	TYPENAME smart_pointer< C >::TYPE** operator&();
	//! \brief checks object
	bool 
	operator!() const;
	//! \brief destroy object
	void 
	clear();
	//! \brief detaches object
	TYPENAME smart_pointer< C >::TYPE* 
	detach();
	//! \brief attaches a new object and frees the old object if specified
	void 
	attach(			TYPE* obj,								//!< new object pointer
					bool free = true						//!< flag to free the old object
					);
private:
	TYPENAME smart_pointer< C >::TYPE*	_ptr;				//!< keeps poiter to object
	C&									_crt;				//!< keeps reference to creator
};

//! \class pool_object_keeper
//! \brief object from pool keeper 
template < class P >
class pool_object_keeper
{
public:
	//! \typedef TYPE
	//! \brief type of object
	typedef TYPENAME P::CREATOR::TYPE TYPE;
	//! \typedef ARG
	//! \brief argument for creator
	typedef TYPENAME P::CREATOR::ARG ARG;
	//! \brief constructor takes and keeps external object
	explicit 
	pool_object_keeper< P >(P* pool_,						//!< pointer to pool
					TYPE* obj								//!< pointer to external object
					);
	//! \brief constructor object will be created inside
	explicit 
	pool_object_keeper< P >(P* pool_,						//!< pointer to pool
					const ARG& arg,							//!< creator argument
					size_t timeout							//!< timeout in milliseconds
					);
	//! \brief destructor
	~pool_object_keeper();
	//! \brief operator->
	inline 
	TYPENAME pool_object_keeper< P >::TYPE* operator->();
	//! \brief operator-> (const)
	inline 
	const TYPENAME pool_object_keeper< P >::TYPE* operator->() const;
	//! \brief operator TYPE*
	inline 
	operator TYPENAME pool_object_keeper< P >::TYPE*() 
	{
		return _obj; 
	}
	//! \brief operator TYPE* (const)
	inline 
	operator const TYPENAME pool_object_keeper< P >::TYPE*() const 
	{ 
		return _obj; 
	}
	//! \brief checks object
	bool operator!() const;
private:
	P*										_pool;			//!< pointer to pool
	TYPENAME pool_object_keeper< P >::TYPE*	_obj;			//!< pointer to object
};

//! \class event_creator
//! \brief event creator
class event_creator : public proto_creator< event_creator, event, size_t > 
{
};

//! \typedef event_pool_t
//! \brief event pool
typedef pool< event_creator > event_pool_t;

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_template_h_
