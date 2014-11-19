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
#ifndef _terimber_daemon_h_
#define _terimber_daemon_h_

#include "allinc.h"
#include "base/string.h"
#include "base/primitives.h"
#include "threadpool/timer.h"
#include "log.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

#if OS_TYPE == OS_WIN32
#define STATIC_FUNCTION_DECL WINAPI
#define DAEMON_HANDLE_FUNCTION_ARG DWORD
#else
#define STATIC_FUNCTION_DECL
#define DAEMON_HANDLE_FUNCTION_ARG int
#endif	

//! \class daemon
//! \brief implements Windows Service or Linux/Unix daemon base functionality
class daemon : public terimber_log,
				public timer_callback
{
protected:
	//! \brief default constructor
	daemon();
	//! \brief destructor
    virtual ~daemon();

	//! \brief overrides log function to set the level of severity
	virtual 
	bool 
	v_is_logging(	size_t module,							//!< module ident
					const char* file,						//!< file name
					size_t line,							//!< line
					terimber_log_severity severity			//!< severity level
					) const;
	//! \brief ovverrides log function to do real logging
	virtual 
	void 
	v_do_logging(	size_t module,							//!< module ident
					const char* file,						//!< file name __FILE__
					size_t line,							//!< line number __LINE__
					terimber_log_severity severity,			//!< severity level
					const char* msg							//!< logging message
					) const;

	
protected:
	string_t			_daemonName;						//!< daemon name
    string_t			_daemonDesc;						//!< daemon description

    // Operations
public:
	//! \brief static main function
    static 
	int 
	s_daemon_main(	unsigned long argc,						//!< number command line arguments
					char* argv[]							//!< command line arguments
					);
	//! \brief this instance main function
    int	
	daemon_main(	unsigned long argc,						//!< number command line arguments 
					char* argv[]							//!< arguments		
					);


protected:
	//! \brief construct config file name:
	//! gets current executable directory and executable name 
	//! adds .cfg extension (if executable has extension it will be removed)
	virtual 
	bool 
	v_find_cfg_file(string_t& cfg_file						//!< [out] config file
					);
	//! \brief initialize daemon
	//! \return bool must return true if succeeded
	virtual 
	bool 
	v_on_init(		unsigned long argc,						//!< number command line arguments 
					char* argv[],							//!< command line arguments
					const char* cfg_file,					//!< config file
					TERIMBER::string_t& err					//!< [out] error description
					) = 0;
	//! \brief uninintialize daemon
	virtual 
	void 
	v_on_uninit() = 0;
	//! \brief default handler
    virtual 
	void 
	v_on_handler(	DAEMON_HANDLE_FUNCTION_ARG code			//!< event code
					) 
	{
	}

	//! \brief periodically notifies user in a separate thread
	virtual 
	void 
	notify(			size_t ident,							//!< timer ident
					size_t interval,						//!< repeatition interval in milliseconds
					size_t multiplier						//!< multiplier coefficient for repeatition interval 
					);

	//! \brief invokes on starting daemon
	bool 
	on_startup(		unsigned long argc,						//!< number command line arguments
					char* argv[]							//!< command line arguments
					);
	//! \brief invokes on stopping daemon
	void 
	on_shutdown();
	void 
	on_stop();

private:
	//! \brief waits until daemon is running
	void 
	on_run();
	//! \brief checks if daemon has been installed (Windows as Service, Linux as Daemon)
	bool 
	is_installed();
	//! \brief installs daemon (for Windows only)
    bool 
	install();
	//! \brief uninstalls daemon (for Windows only)
    bool 
	uninstall();
	//! \brief starts daemon
	void 
	daemon_start(	unsigned long argc,						//!< number command line arguments
					char* argv[]							//!< command line arguments
					);
	//! \brief default daemon event handler
    void 
	daemon_handler(DAEMON_HANDLE_FUNCTION_ARG cmd			//!< event code
					);
	//! \brief logging message
	void 
	log_message(	bool err,								//!< flag error or info
					const char* format,						//!< format
					...										//!< arguments
					) const;

#if OS_TYPE == OS_WIN32
	//! \brief sets state (Windows only)
	bool 
	set_state(		DWORD nState							//!< service state
					);
#endif
private:
	//! \brief static daemon handler for starting daemon
    static 
	void 
	STATIC_FUNCTION_DECL 
	s_daemon_start(	unsigned long argc,						//!< number command line arguments
					char* argv[]							//!< command line arguments
					);
	//! \brief static daemon handler for processing events
    static 
	void 
	STATIC_FUNCTION_DECL 
	s_daemon_handler(DAEMON_HANDLE_FUNCTION_ARG cmd			//!< event code
					);

	//! \brief finds the program full path without extention
	static 
	void 
	find_full_path(	char* path,								//!< pre-allocated buffer
					char*& start,							//!< actual start
					size_t& len								//!< [in, out] buffer length
					);

protected:
	size_t						_severity;					//!< flag does logging

private:
    static daemon*				g_daemon;					//!< global instance pointer
	bool						_isDaemon;					//!< flag if this is this a daemon
    event						_hShutdown;					//!< shut down event

	timer						_log_timer;					//!< timer

#if OS_TYPE == OS_WIN32
    SERVICE_STATUS_HANDLE		_hStatus;					//!< status handle (Windows only)
    SERVICE_STATUS				_pStatus;					//!< status (Windows only)
#endif
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif
