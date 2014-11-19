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

#ifndef _terimber_log_h_
#define _terimber_log_h_

//! \enum terimber_log_severity
enum terimber_log_severity
{
	en_log_paranoid = 1,									//!< anything
	en_log_info = 2,										//!< information
	en_log_warning = 4,										//!< warning
	en_log_error = 8,										//!< error
	en_log_xray = 16										//!< xray
};

//! \class terimber_log
//! \brief abstract class for logging information
class terimber_log
{
public:
	//! \brief destructor
	virtual ~terimber_log() {}
	//! \brief checks if the severity level should be logging
	virtual 
	bool 
	v_is_logging(	size_t module,
					const char* file,						//!< __FILE__
					size_t line,							//!< __LINE__
					terimber_log_severity severity			//!< severity
					) const = 0;
	//! \brief does the logging
	virtual 
	void 
	v_do_logging(	size_t module,
					const char* file,						//!< __FILE__
					size_t line,							//!< __LINE__
					terimber_log_severity severity,			//!< severity
					const char* msg							//!< message text
					) const = 0;
	//! \brief helps in logging formatted message with parameters
	void 
	format_logging(	size_t module,
					const char* file,						//!< __FILE__
					size_t line,							//!< __LINE__
					terimber_log_severity severity,			//!< severity
					const char* format,						//!< format
					...										//!< list of arguments correspondent to format
					) const;
};

// \class terimber_log_helper
//! \brief class encapsulates the external log provider
//! can be used as a parent log for aggregated other classes supported logging
class terimber_log_helper : public terimber_log // inheritance for delegation
{
public:
	//! \brief constructor
	inline 
	terimber_log_helper() : _log(0) 
	{
	}

	//! \brief turns on, off, or changes the desirable log output
	inline 
	terimber_log* 
	log_on(		terimber_log* log								//!< pointer to external log
				)
	{
		terimber_log* old = _log;
		_log = log;
		return old;
	}

	//! \brief checks logging severity
	inline 
	bool 
	v_is_logging(	size_t module,
					const char* file,						//!< __FILE__
					size_t line,							//!< __LINE__
					terimber_log_severity severity			//!< severity
					) const
	{
		// make atomic copy
		terimber_log* log = _log;
		return log ? log->v_is_logging(module, file, line, severity) : false;
	}
	//! \brief does the actual logging
	inline 
	void 
	v_do_logging(	size_t module,
					const char* file,						//!< __FILE__
					size_t line,							//!< __LINE__
					terimber_log_severity severity,			//!< severity
					const char* msg							//!< message text
					) const
	{
		// make atomic copy
		terimber_log* log = _log;

		// if log is set and severity level required logging
		if (log)
			log->v_do_logging(module, file, line, severity, msg); // call external log function
	}

private:
	terimber_log * volatile		_log;						//!< external log pointer
};

#endif



