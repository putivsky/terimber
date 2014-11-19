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

#ifndef _terimber_varfactory_h_
#define _terimber_varfactory_h_

#include "smart/varvalue.h"
#include "smart/byterep.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class variant_factory
//! \brief class as a set of memory factories
class variant_factory
{
public:
	//! \brief constructor
	variant_factory();
	//! \brief destructor
	~variant_factory();

	//! \brief allocates variant value
	bool 
	create(			vt_types type,							//!< variant type
					var_value& val,							//!< variant value
					size_t len								//!< length
					);
	//! \brief makes a copy
	bool 
	clone(			vt_types type,							//!< variant type
					var_value& out,							//!< output variant value
					const var_value& in						//!< input variant value
					);
	//! \brief destroys variant value
	void 
	destroy(		vt_types type,							//!< variant type
					var_value& val							//!< variant value
					);
	
	//! \brief reuse memory
	void
	reset();

private:
	
	byte_repository_factory			_byte_allocator;		//!< responsible for string/binary
	node_allocator< double >		_double_allocator;		//!< responsible for 8 bytes allocation (double, long int, unsigned long int)
	node_allocator< long double >	_long_double_allocator;	//!< responsible for long double bytes
	node_allocator< guid_t >		_guid_allocator;		//!< responsible for 16 bytes guid
	byte_allocator					_tmp_allocator;			//!< for temporary allocations
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_varfactory_h_
