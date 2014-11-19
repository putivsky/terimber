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

#ifndef _terimber_vardatabase_h_
#define _terimber_vardatabase_h_

#include "allinc.h"
#include "smart/varaccess.h"
#include "smart/varobj.h"
#include "base/map.h"
#include "base/keymaker.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class vardatabase 
//! \brief class imlements interface terimber_vardatabase
class vardatabase : public terimber_vardatabase
{
	//! \class vartable
	//! \brief var table
	class vartable
	{
		//! \brief hidden assign operator
		vartable& 
		operator=(const vartable& x);
	public:
		//! \brief constructor
		vartable() : 
			_tbl(_schema) 
		{
		}
		//! \brief copy constructor
		vartable(const vartable& x) : 
			_tbl(_schema) 
		{
		} 

		byte_allocator							_all;		//!< data allocator
		byte_allocator							_tmp_all;	//!< temporary allocator
		byte_allocator							_inl_all;	//!< inline allocator
		var_container::sorted_container_allocator_t	_con_all; //!< container allocator
		var_object_schema						_schema;	//!< table schema
		var_object_repository					_tbl;		//!< table itself
		keylocker								_key;		//!< read/write locker
	};

	//! \typedef table_map_t
	//! \brief maps table name to table object
	typedef map< string_t, vartable >	table_map_t;
public:
	//! \brief constructor
	vardatabase();
	//! \brief destructor
	virtual 
	~vardatabase();

	//! \brief process d/xml request
	virtual 
	bool 
	process_xml_request(const char* request,				//!< db/xml request
					size_t len,								//!< request length
					xml_designer* parser					//!< xml designer
					);

private:
	//! \brief creates a table according to the schema
	bool 
	fill_schema(	vartable& tbl,							//!< table
					xml_designer* parser					//!< xml designer
					);
private:
	keylocker		_masterkey;								//!< locker on DDL level
	mutex			_table_mtx;								//!< table access mutex
	table_map_t		_table_map;								//!< table map
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif
