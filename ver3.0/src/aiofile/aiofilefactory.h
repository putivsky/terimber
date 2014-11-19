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

#ifndef _terimber_aiofile_factory_h_
#define _terimber_aiofile_factory_h_

#include "log.h"

//! \enum aiofile_type
//! \brief asynchronous file actions
enum aiofile_type
{
	AIOFILE_UNKNOWN = 0,									//!< unknown action
	AIOFILE_READ,											//!< read action
	AIOFILE_WRITE											//!< write action
};

//! \class terimber_file_callback
//! \brief abstract interface for user of file port
class terimber_aiofile_callback
{
public:
	//! \brief destructor
	virtual
	~terimber_aiofile_callback() {}
	
	//! \brief port will call function after error occured
	virtual 
	void 
	v_on_error(	size_t handle,								//!< file ident
				int err,									//!< error code
				aiofile_type type,							//!< action type
				void* userdata								//!< user defined data
				) = 0;
	//! \brief port will call function after successfully sending buffer to file
	virtual 
	void 
	v_on_write(	size_t handle,								//!< file ident
				void* buf,									//!< buffer
				size_t requested,							//!< requested bytes to write
				size_t processed,							//!< actual bytes processed
				void* userdata								//!< user defined data
				) = 0;
	//! \brief port will call function after successfully receiving buffer from file
	virtual 
	void 
	v_on_read(	size_t handle,								//!< file ident
				void* buf,									//!< buffer
				size_t requested,							//!< requested bytes to read
				size_t processed,							//!< actual bytes processed
				void* userdata								//!< user defined data
				) = 0;
};

//! \class terimber_file_port
//! \brief abstract interface for the file port
class terimber_aiofile : public terimber_log_helper
{
public:
	//! \brief destructor
	virtual ~terimber_aiofile() {}
	//! \brief open file
	virtual size_t open(const char* file_name,				//!< file name
						bool read_write,					//!< open for read or write only
						terimber_aiofile_callback* callback	//!< user callback class
						) = 0;
	//! \brief close file
	virtual void close(	size_t handle						//!< valid file ident
						) = 0;
	//! \brief sends buffer to specified file asynchronously
	virtual int write(	size_t handle,						//!< valid file ident
						size_t offset,						//!< file offset
						const void* buf,					//!< buffer 
						size_t len,							//!< length of buffer
						size_t timeout,						//!< timeout in milliseconds
						void* userdata						//!< user defined data
						) = 0;
	//! \brief receives buffer of bytes from specified file asynchronously
	virtual int read(	size_t handle,						//!< valid file ident
						size_t offset,						//!< file offset
						void* buf,							//!< buffer 
						size_t len,							//!< length of buffer
						size_t timeout,						//!< timeout in milliseconds
						void* userdata						//!< user defined data
						) = 0;	

	//! \brief makes the snapshot of internal state
	virtual void doxray() = 0;
};
 
//! \class terimber_file_port_access
//! \brief factory for aiofile class
class terimber_aiofile_factory
{
public:
	//! \brief constructor
	terimber_aiofile_factory();
	//! \brief  destructor
	~terimber_aiofile_factory();

	//! \brief create terimber file port object
	// caller is responsible for destroying it
	// capacity means how many threads can be opened and used simultaniously
	// deactivate_time_msec is the interval in milliseconds (minimum 100 msec) 
	// after that all unused threads returned to the pool will be closed
	terimber_aiofile* 
	get_aiofile(	terimber_log* log = 0,					//!< pointer to log 
					size_t capacity = 3,					//!< max additional working threads
					size_t deactivate_time_msec = 60000		//!< timeout in milliseconds for deactivation of unused threads
					);
};



#endif // _terimber_file_port_access_h_
