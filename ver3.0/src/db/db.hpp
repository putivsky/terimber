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

#include "db/db.h"
#include "base/numeric.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline 
bool 
binder::set_as_null(db_param_type in_out, dbtypes type)
{
	deallocate_value();

	_in_out = in_out;
	_type = type;
	_value.nullVal = true;
	memset(&_value.val, 0, sizeof(terimber_xml_value));
	return true;
}

inline 
bool 
binder::set_as_bool(db_param_type in_out, bool val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_bool;
	_value.nullVal = false;
	_value.val.boolVal = val;
	return true;
}

inline 
bool 
binder::set_as_char(db_param_type in_out, sb1_t val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_sb1;
	_value.nullVal = false;
	_value.val.cVal = val;
	return true;
}

inline 
bool 
binder::set_as_byte(db_param_type in_out, ub1_t val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_ub1;
	_value.nullVal = false;
	_value.val.bVal = val;
	return true;
}

inline 
bool 
binder::set_as_short(db_param_type in_out, sb2_t val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_sb2;
	_value.nullVal = false;
	_value.val.iVal = val;
	return true;
}

inline 
bool 
binder::set_as_word(db_param_type in_out, ub2_t val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_ub2;
	_value.nullVal = false;
	_value.val.uiVal = val;
	return true;
}

inline 
bool 
binder::set_as_long(db_param_type in_out, sb4_t val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_sb4;
	_value.nullVal = false;
	_value.val.lVal = val;
	return true;
}

inline 
bool 
binder::set_as_dword(db_param_type in_out, ub4_t val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_ub4;
	_value.nullVal = false;
	_value.val.ulVal = val;
	return true;
}

inline 
bool 
binder::set_as_float(db_param_type in_out, float val)
{
	deallocate_value();

	_in_out = in_out;
	_type = db_float;
	_value.nullVal = false;
	_value.val.fltVal = val;
	return true;
}

inline 
bool 
binder::set_as_double(db_param_type in_out, double val)
{
#ifdef OS_64BIT
	deallocate_value();
	_type = db_double;
	_value.val.dblVal = val;
#else
	if (_type == db_double && _value.val.dblVal)
	{
		*const_cast< double* >(_value.val.dblVal) = val;
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_double;
			*(double*)allocate_value(0) = val;
		}
		catch (exception&)
		{
		//	_error = x.what();
		//	_code = x.get_code();
			return false;
		}
	}
#endif

	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_long64(db_param_type in_out, sb8_t val)
{
#ifdef OS_64BIT
	deallocate_value();
	_type = db_sb8;
	_value.val.intVal = val;
#else
	if ((_type == db_date || _type == db_sb8 || _type == db_ub8)
		&& _value.val.intVal)
	{
		_type = db_sb8;
		*const_cast< sb8_t* >(_value.val.intVal) = val;
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_sb8;
			*(sb8_t*)allocate_value(0) = val;
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}
#endif

	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_dword64(db_param_type in_out, ub8_t val)
{
#ifdef OS_64BIT
	deallocate_value();
	_type = db_ub8;
	_value.val.uintVal = val;
#else
	if ((_type == db_date || _type == db_sb8 || _type == db_ub8)
		&& _value.val.uintVal)
	{
		_type = db_ub8;
		*const_cast< ub8_t* >(_value.val.uintVal) = val;
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_ub8;
			*(ub8_t*)allocate_value(0) = val;
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}
#endif

	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_guid(db_param_type in_out, const guid_t& val)
{
	if (_type == db_guid && _value.val.guidVal)
	{
		_type = db_guid;
		*const_cast< guid_t* >(_value.val.guidVal) = val;
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_guid;
			*(guid_t*)allocate_value(0) = val;
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}

	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_numeric(db_param_type in_out, const char* val, char delimeter)
{
	numeric num(val, delimeter);
	
	if ((_type == db_numeric || _type == db_decimal) 
		&& _value.val.bufVal
		)
	{
		// checks current length
		numeric old_num;
		if (old_num.parse_orcl((const ub1_t*)_value.val.bufVal)
			&& old_num.orcl_len() >= num.orcl_len()) // simple copy
		{
			_type = db_numeric;
			_in_out = in_out;
			_value.nullVal = false;
			return num.persist_orcl((ub1_t*)_value.val.bufVal);
		}
	}

	try
	{
		deallocate_value();
		_type = db_numeric;
		_in_out = in_out;
		_value.nullVal = false;
		return num.persist_orcl((ub1_t*)allocate_value(num.orcl_len()));
	}
	catch (exception&)
	{
		//_error = x.what();
		//_code = x.get_code();
		return false;
	}

	return true;
}

inline 
bool 
binder::set_as_decimal(db_param_type in_out, const char* val, char delimeter)
{
	numeric num(val, delimeter);
	
	if ((_type == db_numeric || _type == db_decimal) 
		&& _value.val.bufVal
		)
	{
		// checks current length
		numeric old_num;
		if (old_num.parse_orcl((const ub1_t*)_value.val.bufVal)
			&& old_num.orcl_len() >= num.orcl_len()) // simple copy
		{
			_type = db_decimal;
			_in_out = in_out;
			_value.nullVal = false;
			return num.persist_orcl((ub1_t*)_value.val.bufVal);
		}
	}

	try
	{
		deallocate_value();
		_type = db_decimal;
		_in_out = in_out;
		_value.nullVal = false;

		return num.persist_orcl((ub1_t*)allocate_value(num.orcl_len()));
	}
	catch (exception&)
	{
		//_error = x.what();
		//_code = x.get_code();
		return false;
	}

	return true;
}

bool 
binder::set_as_date(db_param_type in_out, ub4_t year, ub1_t month, ub1_t day, ub1_t hour, ub1_t minute, ub1_t second, ub2_t millisec)
{
	sb8_t dt = 0;
	if (!date::convert_to(year, month, day, hour, minute, second, millisec, dt))
		return false;
#ifdef OS_64BIT
	deallocate_value();
	_type = db_date;
	_value.val.intVal = dt;
#else
	if ((_type == db_date || _type == db_sb8 || _type == db_ub8)
		&& _value.val.intVal)
	{
		_type = db_date;
		*const_cast< sb8_t* >(_value.val.intVal) = dt;
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_date;
			*(sb8_t*)allocate_value(0) = dt;
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}
#endif

	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_string(db_param_type in_out, const char* val, size_t len)
{
	size_t len_in = len == os_minus_one ? str_template::strlen(val) : len;

	if (_type == db_string && _value.val.strVal && str_template::strlen(_value.val.strVal) >= len_in)
	{
		str_template::strcpy((char*)_value.val.strVal, val, len_in);
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_string;
			str_template::strcpy((char*)allocate_value(len_in), val, len_in); 
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}

	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_string_ptr(const char* val)
{
	deallocate_value();

	_value.val.strVal = val;
	_in_out = db_param_in;
	_value.nullVal = false;
	_type = db_string;
	return true;
}

bool 
binder::set_as_wstring(db_param_type in_out, const wchar_t* val, size_t len)
{
	size_t len_in = len == os_minus_one ? str_template::strlen(val) : len;
	if (_type == db_wstring && _value.val.wstrVal && str_template::strlen(_value.val.wstrVal) >= len_in)
	{
		str_template::strcpy((wchar_t*)_value.val.wstrVal, val, len_in);
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_wstring;
			str_template::strcpy((wchar_t*)allocate_value(len_in), val, len_in); 
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}
	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

inline 
bool 
binder::set_as_wstring_ptr(const wchar_t* val)
{
	deallocate_value();

	_value.val.wstrVal = val;
	_in_out = db_param_in;
	_value.nullVal = false;
	_type = db_wstring;
	return true;
}

inline 
bool 
binder::set_as_binary(db_param_type in_out, const ub1_t* val, size_t len)
{
	if (_type == db_binary && _value.val.bufVal && *(size_t*)_value.val.bufVal >= len)
	{
		*(size_t*)_value.val.bufVal = len;
		memcpy((ub1_t*)_value.val.bufVal + sizeof(size_t), val, len);
	}
	else
	{
		try
		{
			deallocate_value();
			_type = db_binary;
			memcpy((ub1_t*)allocate_value(len), val, len); 
		}
		catch (exception&)
		{
			//_error = x.what();
			//_code = x.get_code();
			return false;
		}
	}
	_in_out = in_out;
	_value.nullVal = false;
	return true;
}

bool 
binder::set_as_binary_ptr(const ub1_t* val)
{
	deallocate_value();

	_value.val.bufVal = val;
	_in_out = db_param_in;
	_value.nullVal = false;
	_type = db_binary;
	return true;
}


///////////////////////////////////////////////////////////////////
inline
terimber_xml_value 
dbserver_impl::get_param_as_value(size_t index, vt_types type) const
{
	try
	{
		if (index >= _params.size())
			exception::_throw("Out of index");
		vt_types type_ = convert_types(_params[index]._type);
		_temp_allocator->reset();
		return type_ != type ? parse_value(type, 
										persist_value(type_, _params[index]._value.val, _temp_allocator), 
										0xffffffff, 
										_temp_allocator) : _params[index]._value.val;
	}
	catch (...)
	{
		terimber_xml_value val;
		memset(&val, 0, sizeof(terimber_xml_value));
		return val;
	}
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
inline
terimber_xml_value 
dbserver_impl::get_value_as_value(size_t index, vt_types type) const
{
	try
	{
		if (_iter == get_iter_end() || index >= _cols.size() || (*_iter)[index].nullVal)
			exception::_throw("Out of range");

		vt_types type_ = convert_types(_cols[index]._type);
		_temp_allocator->reset();
		return type_ != type ? parse_value(type, 
										persist_value(type_, (*_iter)[index].val, _temp_allocator), 
										0xffffffff, 
										_temp_allocator) : (*_iter)[index].val;
	}
	catch (...)
	{
		terimber_xml_value val;
		memset(&val, 0, sizeof(terimber_xml_value));
		return val;
	}
}

// static
inline
vt_types 
dbserver_impl::convert_types(dbtypes type)
{
	switch (type)
	{
		case db_unknown:	return vt_unknown;
		case db_bool:		return vt_bool;
		case db_sb1:		return vt_sb1;
		case db_ub1:		return vt_ub1;
		case db_sb2:		return vt_sb2;
		case db_ub2:		return vt_ub2;
		case db_sb4:		return vt_sb4;
		case db_ub4:		return vt_ub4; 
		case db_float:		return vt_float;
		case db_double:		return vt_double;
		case db_sb8:		return vt_sb8;
		case db_ub8:		return vt_ub8;
		case db_date:		return vt_date;
		case db_string:		return vt_string;
		case db_wstring:	return vt_wstring;
		case db_decimal:	return vt_decimal;
		case db_numeric:	return vt_numeric;
		case db_binary:		return vt_binary;
		case db_guid:		return vt_guid;
		default:			return vt_unknown;
	}
}

/////////////////////////////////////////////////////////
// pool support
//! \brief activates object
//! nothing to do
//static 
template < class T >
void 
db_creator< T >::activate(		db_entry* obj,							//!< object pointer 
				const db_arg& arg
				)
{
	if (!obj->_obj->is_connect())
		obj->_obj->connect(arg._trusted, arg._login_string);
}

//! \brief finds object
//static 
template < class T >
bool 
db_creator< T >::find(db_entry* obj,							//!< object pointer 
				const db_arg& arg
				)
{
	return static_cast< const db_arg& >(*obj) == arg;
}

//! \brief backing object
//static 
template < class T >
void 
db_creator< T >::back(db_entry* obj,							//!< object pointer 
				const db_arg& arg
				)
{
	// stop transaction
	if (obj->_obj->is_in_transaction())
		obj->_obj->rollback();

	// close query
	if (obj->_obj->is_open_sql())
		obj->_obj->close_sql();
}

//! \brief destroys object
//static
template < class T >
void 
db_creator< T >::destroy(db_entry* obj,							//!< object pointer
				const db_arg& arg
				)
{
	deactivate(obj, arg); // close resources

	// destroy
	delete obj->_obj;
	delete obj;
}

//! \brief deactivates object - nothing to do
//static 
template < class T >
void 
db_creator< T >::deactivate(db_entry* obj,							//!< object pointer 
				const db_arg& arg
				)
{
	back(obj, arg); // close resources

	// disconnect if any
	if (obj->_obj->is_connect())
		obj->_obj->disconnect();
}

//! \typedef event_pool_t
//! \brief event pool
//typedef pool< db_creator > db_pool_t;


#pragma pack()
END_TERIMBER_NAMESPACE
