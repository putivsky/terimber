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

#include "daemon/daemon.h"
#include "ossock.h"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/vector.hpp"
#include "base/map.hpp"
#include "base/list.hpp"
#include "base/date.h"
#include "base/xmlconfig.h"
#include "xml/sxs.hpp"

#if OS_TYPE == OS_WIN32
///////////////////////////////////////////////////////////////////////////////
// WinMain entry point
extern "C" int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, char* /*lpCmdLine*/, int /*nShowCmd*/)
{
	int argc = 0;
	// gets command line
	LPWSTR wcmd = ::GetCommandLineW();
	// parses command line arguments
	LPWSTR* wargv = CommandLineToArgvW(wcmd, &argc);

	// allocates utf-8 array of string
	TERIMBER::vector< TERIMBER::string_t > strargv;
	strargv.resize(argc);
	// allocates char pointers array
	TERIMBER::vector< char* > argv;
	argv.resize(argc, 0);

	// converts to utf-8 array
	for (int i = 0; i < argc; ++ i)
		argv[i] = (char*)TERIMBER::str_template::unicode_to_multibyte(strargv[i], wargv[i], os_minus_one);

	// cleans up
	::GlobalFree(wargv);

	// calls static main function
	return TERIMBER::daemon::s_daemon_main(argc, &argv[0]);
}
///////////////////////////////////////////////////////////////////////////////
#else

#include <syslog.h>
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	// calls static main function
	return TERIMBER::daemon::s_daemon_main(argc, argv);
}
///////////////////////////////////////////////////////////////////////////////
#endif

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// static
daemon* daemon::g_daemon = 0;


// daemon construction
daemon::daemon()
{
	// caller MUST create global instance of daemon, and only ONCE
    assert(!g_daemon);

	// assigns this pointer to global daemon pointer
    g_daemon = this;
	// resets daemon flag
    _isDaemon = false;
	// resets logging flag
	_severity = 0;

#if OS_TYPE == OS_WIN32
	// resets Windows status
    _hStatus = 0;
    memset(&_pStatus, 0, sizeof(_pStatus));
    _pStatus.dwServiceType = SERVICE_WIN32;
    _pStatus.dwCurrentState	= SERVICE_START_PENDING;
    _pStatus.dwControlsAccepted	= SERVICE_ACCEPT_STOP;
#endif
}

daemon::~daemon()
{
	// resets global daemon pointer
	g_daemon = 0;
}

///////////////////////////////////////////////////////////////////////////////
// winservice WinMain entry point
// static
int
daemon::s_daemon_main(unsigned long argc, char* argv[])
{
	// global daemon pointer must be assigned
    assert(g_daemon);
	// call daemon main function
    return g_daemon->daemon_main(argc, argv);
}

int daemon::daemon_main(unsigned long argc, char* argv[])
{
	// anylizes command line agruments
	if (argc > 1 && argv && argv[1])
	{
		if (!str_template::strcmp((const char*)argv[1], "uninstall", os_minus_one))
		{
			uninstall();
			return 0;
		}
		else if (!str_template::strcmp((const char*)argv[1], "install", os_minus_one))
		{
			uninstall();
			install();
			return 0;
		}
		else if (!str_template::strcmp((const char*)argv[1], "noservice", os_minus_one)) // useful for debug
		{
			_isDaemon = false;
		}
		else
			_isDaemon = is_installed();
	}
	else
		_isDaemon = is_installed();

#if OS_TYPE == OS_WIN32
	if (_isDaemon) // registers service entry table
	{
		SERVICE_TABLE_ENTRY pTable[] = { { (char*)(const char*)_daemonName, s_daemon_start }, { 0, 0 } };

		if (!StartServiceCtrlDispatcher(pTable))
			_pStatus.dwWin32ExitCode = GetLastError();

		// leaves main function here - Windows will call "s_daemon_start" function
		return _pStatus.dwWin32ExitCode;
	}

#else
	// changes file mode mask
	umask(0);

	// forks off the parent process if not daemon
	if (!_isDaemon)
	{

		log_message(false, "starting as a cmd line program...");
		for (int i = 1; i < argc; ++i)
			log_message(false, "cmd parameter %d: %s", i, argv[i]);
	}
	else
	{
		// forks and gets parent process id
		pid_t pid = fork();

		if (pid < 0)
			exit(EXIT_FAILURE);
		else if (pid > 0) // this is parent process
			exit(EXIT_SUCCESS);


		// sets session id
		pid_t sid = setsid();

		if (sid < 0)
			exit(EXIT_FAILURE);

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		// checks if alredy running
		TERIMBER::string_t lockfilename("./");
		lockfilename += _daemonName;
		lockfilename += ".pid";

		int fd = ::open(lockfilename, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (fd < 0)
		{
			log_message(true, "can not open: %s, error: %s", (const char*)lockfilename, strerror( errno ));
			exit(EXIT_FAILURE);
		}

		// locks file
		struct flock fl;
		fl.l_type = F_WRLCK;
		fl.l_start = 0;
		fl.l_whence = SEEK_SET;
		fl.l_len = 0;

		if (::fcntl(fd, F_SETLK, &fl) < 0)
		{
			::close(fd);
			if (errno == EACCES || errno == EAGAIN)
				log_message(true, "service %s is alreay running", (const char*)lockfilename);
			else
				log_message(true, "can not lock file %s", (const char*)lockfilename);

			exit(EXIT_FAILURE);
		}

		char fbuf[32];
		if (-1 == ftruncate(fd, 0))
			exit(EXIT_FAILURE);

		str_template::strprint(fbuf, sizeof(fbuf), "%ld", (long)getpid());
		if (-1 == ::write(fd, fbuf, strlen(fbuf) + 1))
			exit(EXIT_FAILURE);

		// set event handler
		struct sigaction sa;
		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		if (sigaction(SIGHUP, &sa, 0) < 0)
			log_message(true, "sigaction: %s", strerror( errno ));

		// forks again to escapes from group of processes
		if ((pid = fork()) < 0)
			exit(EXIT_FAILURE);
		else if (pid > 0) // this is a parent process
			exit(EXIT_SUCCESS);
	}
	
	static const int signal_list[] =
	{
		SIGALRM,
		SIGCHLD,
		SIGHUP, SIGUSR1, SIGUSR2,
		SIGINT, SIGTERM, SIGQUIT,
		SIGABRT,
		SIGBUS, SIGFPE, SIGILL, SIGSEGV,
		SIGPIPE, SIGTSTP, SIGTTIN, SIGTTOU,
		0
	};

	//  registers signal callback
	for (const int *signal_ptr = signal_list; *signal_ptr; ++signal_ptr)
	{
		// set event handler
		struct sigaction sa;
		sa.sa_handler = s_daemon_handler;
		sigemptyset(&sa.sa_mask);
		sigaddset(&sa.sa_mask, *signal_ptr);
		sa.sa_flags = 0;
		if (sigaction(*signal_ptr, &sa, 0) < 0)
			log_message(true, "sigaction: %s", strerror( errno ));
	}

#endif

	// calls daemon start function
	daemon_start(argc, argv);

#if OS_TYPE == OS_WIN32
	return _pStatus.dwWin32ExitCode;
#else
	exit(EXIT_SUCCESS);
#endif
}

///////////////////////////////////////////////////////////////////////////////
#if OS_TYPE == OS_WIN32
// winservice state update
bool daemon::set_state(DWORD nState)
{
    _pStatus.dwCurrentState = nState;
    return _hStatus && SetServiceStatus(_hStatus, &_pStatus);
}
#endif
///////////////////////////////////////////////////////////////////////////////
// winservice service installation functions
bool daemon::is_installed()
{
    if (!_daemonName.length())
	{
		log_message(true, "daemon name is not assigned");
        return false;
	}

#if OS_TYPE == OS_WIN32
	if (SC_HANDLE hSCM = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS))
    {
		if (SC_HANDLE hService = ::OpenService(hSCM, _daemonName, SERVICE_QUERY_CONFIG))
        {
			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCM);
			return true;
        }
		else
		{
			log_message(true, "can not open service for name: %s", (const char*)_daemonName);
			_pStatus.dwWin32ExitCode = GetLastError();
		}

		::CloseServiceHandle(hSCM);
    }
	else
	{
		log_message(true, "can not open service manager");
		_pStatus.dwWin32ExitCode = GetLastError();
	}


    return false;
#else
#ifdef _NDEBUG
    return true;
#else
	return false;
#endif
#endif
}

bool daemon::install()
{
    if (is_installed())
        return true;
    if (!_daemonName.length())
        return false;

#if OS_TYPE == OS_WIN32
    char path[MAX_PATH + 2];
	DWORD len = ::GetModuleFileName(NULL, path + 1, MAX_PATH);
    if (!len || len == MAX_PATH)
        return false;

    path[0]	= '\"';
    path[len + 1] = '\"';
    path[len + 2] = 0;

	if (SC_HANDLE hSCM = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS))
    {
		if (SC_HANDLE hService = ::CreateService(hSCM, _daemonName, _daemonName,
                                 SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                                 SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                                 path, 0, 0, "RPCSS\0", 0, 0))
		{
			SERVICE_DESCRIPTION d;
			d.lpDescription = (LPSTR)(const char*)_daemonDesc;
			::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &d);
			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCM);
			return true;
        }
		else
		{
			log_message(true, "can not open service for name: %s", (const char*)_daemonName);
			_pStatus.dwWin32ExitCode = GetLastError();
		}


		::CloseServiceHandle(hSCM);
    }
	else
	{
		log_message(true, "can not open service manager");
		_pStatus.dwWin32ExitCode = GetLastError();
	}


    return false;
#else
	return true;
#endif
}

bool daemon::uninstall()
{
    if (!is_installed())
        return true;

#if OS_TYPE == OS_WIN32
	if (SC_HANDLE hSCM = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS))
    {
		if (SC_HANDLE hService = ::OpenService(hSCM, _daemonName, SERVICE_STOP | DELETE))
        {
            bool okay = true;
            SERVICE_STATUS pStatus;

			if (!::ControlService(hService, SERVICE_CONTROL_STOP, &pStatus))
            {
				DWORD err = ::GetLastError();

                if (err != ERROR_SERVICE_NOT_ACTIVE
					&& (err != ERROR_SERVICE_CANNOT_ACCEPT_CTRL
						|| pStatus.dwCurrentState != SERVICE_STOP_PENDING)
					)
                    okay = false;
            }

			okay = okay && ::DeleteService(hService) == TRUE;
			::CloseServiceHandle(hService);
			::CloseServiceHandle(hSCM);
			return okay;
        }
		else
		{
			log_message(true, "can not open service for name: %s", (const char*)_daemonName);
			_pStatus.dwWin32ExitCode = GetLastError();
		}


		::CloseServiceHandle(hSCM);
    }
	else
	{
		log_message(true, "can not open service manager");
		_pStatus.dwWin32ExitCode = GetLastError();
	}

    return false;
#else
	return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// service/daemon entry point
void STATIC_FUNCTION_DECL daemon::s_daemon_start(unsigned long argc, char* argv[])
{
	// global daemon pointer must be assigned
    assert(g_daemon);
	// calls daemon start function
    g_daemon->daemon_start(argc, argv);
}

void daemon::daemon_start(unsigned long argc, char* argv[])
{
#if OS_TYPE == OS_WIN32
	_pStatus.dwWin32ExitCode = ERROR_SUCCESS;

	if (_isDaemon)
	{
		// registers service handlers
		_hStatus = ::RegisterServiceCtrlHandler(_daemonName, s_daemon_handler);
		if (!_hStatus)
		{
			log_message(true, "can not register service handlers for %s", (const char*)_daemonName);
			_pStatus.dwWin32ExitCode = ERROR_ACCESS_DENIED;
			set_state(SERVICE_STOPPED);
			return;
		}
	}

	// sets state
	set_state(SERVICE_START_PENDING);
#endif

	// activates timer
	_log_timer.activate(this, 0, 15000); // reread config every 15 seconds

	// calsl on start up - initialization
	if (on_startup(argc, argv))
	{
		log_message(false, "Service started");

#if OS_TYPE == OS_WIN32
		set_state(SERVICE_RUNNING);
#endif
		// blocking call - until daemon gets stop signal
		on_run();

#if OS_TYPE == OS_WIN32
		set_state(SERVICE_STOP_PENDING);
#endif
	}
	else
	{
		log_message(true, "Can not start daemon");
	#if OS_TYPE == OS_WIN32
		_pStatus.dwWin32ExitCode = ERROR_ACCESS_DENIED;
	#endif
	}

	// deactivates timer
	_log_timer.deactivate();
	// calls on shut down
    on_shutdown();
}

///////////////////////////////////////////////////////////////////////////////
// daemon static handler entry point

void STATIC_FUNCTION_DECL daemon::s_daemon_handler(DAEMON_HANDLE_FUNCTION_ARG cmd)
{
	// must be aasigned
    assert(g_daemon);
	// calls this daemon handler
    g_daemon->daemon_handler(cmd);
}

void daemon::daemon_handler(DAEMON_HANDLE_FUNCTION_ARG cmd)
{
    switch (cmd)
    {
#if OS_TYPE == OS_WIN32
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
#else
		case SIGABRT:
		case SIGBUS:
		case SIGFPE:
		case SIGILL:
		case SIGSEGV:
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
#endif
			log_message(false, "Stop signal: %d received", cmd);
			// calls on stop
			on_stop();
			break;
#if OS_TYPE == OS_WIN32
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_INTERROGATE:
#else
		case SIGALRM:
		case SIGCHLD:
		case SIGHUP:
		case SIGUSR1:
		case SIGUSR2:
		case SIGPIPE:
		case SIGTSTP:
		case SIGTTIN:
		case SIGTTOU:
#endif
			log_message(false, "Handler signal: %d received", cmd);
			// calls handler processor
			v_on_handler(cmd);
			break;
	}
}

/////////////////////////////////////////////////////////////
bool daemon::on_startup(unsigned long argc, char* argv[])
{
	// initializes socket
	if (_sockStartup())
        return false;

	string_t cfg_file, err;
	// gets config file name
	if (v_find_cfg_file(cfg_file))
	{
		// checks logging flag
		xmlconfig settings(cfg_file, 0, 0);
		int severity = 0;
		if (settings.get(0, "severity", severity))
		{
			_severity = severity;
		}
	}

	// calls init function
	if (!v_on_init(argc, argv, cfg_file, err))
	{
		log_message(true, "init failed: %s", (const char*)err);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
void daemon::on_run()
{
	// waits on event
	_hShutdown.wait(INFINITE);
}

void daemon::on_stop()
{
	// sends signal to wakeup on_run
    _hShutdown.set();
}

void daemon::on_shutdown()
{
	// uninit reasources
	v_on_uninit();
	// socket cleans up
	_sockCleanup();

	log_message(false, "Service stopped");

#if OS_TYPE == OS_WIN32
    set_state(SERVICE_STOPPED);
#endif
}

////////////////////////////////////////////////////////////////////////////////////
// virtual
void
daemon::notify(size_t ident,							//!< timer ident
				size_t interval,						//!< repeatition interval in milliseconds
				size_t multiplier						//!< multiplier coefficient for repeatition interval
				)
{
	string_t cfg_file;
	// gets config file name
	if (v_find_cfg_file(cfg_file))
	{
		// checks logging flag
		xmlconfig settings(cfg_file, 0, 0);
		int severity = 0;
		if (settings.get(0, "severity", severity))
		{
			_severity = severity;
		}
	}
}

//virtual
bool
daemon::v_is_logging(size_t module, const char* file, size_t line, terimber_log_severity severity) const
{
	return (severity & _severity) != 0;
}

//virtual
void
daemon::v_do_logging(size_t module, const char* file, size_t line, terimber_log_severity severity, const char* msg) const
{
	log_message(severity == en_log_error, "file: %s, line: %d, msg: %s", file, line, msg);
}

void
daemon::log_message(bool err, const char* format, ...) const
{
	if (!_severity)
		return;

	char buffer[4096];
	va_list pArg;

	va_start(pArg, format);
#if OS_TYPE == OS_WIN32
	_vsnprintf(buffer, sizeof(buffer), format, pArg);
#else
	vsnprintf(buffer, sizeof(buffer), format, pArg);
#endif

	va_end(pArg);

	char dbuf[64];
	date now;
	now.get_date(dbuf);

	if (!_isDaemon)
	{
		// logs to the file at current directory if not a daemon
		char logName[1024];
		char* start = 0;

		size_t len = sizeof(logName) - 5;
		find_full_path(logName, start, len);
		strcat(start, ".log");

		FILE* f = fopen(start, "at+");
		if (f)
		{
			fprintf(f, err ? "%s: error: %s\n" : "%s: info: %s\n", dbuf, buffer);
			fclose(f);
		}
	}
	else
	{
#if OS_TYPE == OS_WIN32
		// logs to Windows Event Log
		if (HANDLE hEventSource = RegisterEventSource(0, _daemonName))
		{
			char* arr[2] = {buffer, dbuf};
			ReportEvent(hEventSource, err ? EVENTLOG_ERROR_TYPE : EVENTLOG_INFORMATION_TYPE, 0, 0, 0, 2, 0, (const char**)arr, 0);
			DeregisterEventSource(hEventSource);
		}
#else
		// logs to Linux event log
		openlog((const char*)_daemonName, LOG_PID, LOG_USER);
		syslog(LOG_USER | (err ? LOG_ERR : LOG_INFO), err ? "%s: error: %s\n" : "%s: info: %s\n", dbuf, buffer);
		closelog();
#endif
	}
}

// virtual
bool
daemon::v_find_cfg_file(string_t& cfg_file)
{
	char path[2048];
	char* start = 0;

	size_t len = sizeof(path) - 5;
	find_full_path(path, start, len);

	strcat(start, ".cfg");
	cfg_file = start;
	format_logging(0, __FILE__, __LINE__, en_log_info, "config file detected at the location: %s", start);
	return true;
}

//static
void
daemon::find_full_path(	char* path,								//!< pre-allocated buffer
						char*& prefix,
				size_t& len								//!< [in, out] buffer length
				)
{
	assert(len > 5);
#if OS_TYPE == OS_WIN32
	// config file is supposed to have the same name as daemon with ".cfg" extension
	static const char* unc_prefix = "\\\\?\\";
	::GetModuleFileName(0, path, (DWORD)len);
	prefix = path + ((strstr(path, unc_prefix) == path) ? strlen(unc_prefix) : 0);
#else
	ssize_t written = 0;
	if (-1 == (written = readlink("/proc/self/exe", path, len - 1)))
		return;

	path[written] = 0;
	prefix = path;
#endif
	char* posfix = 0;
	if ((posfix = strrchr(path, '.')) != 0
		&& __max(strrchr(path, '/'), strrchr(path, '\\')) < posfix // no slashes or dot is the last one
		)
	{
		*posfix = 0;
	}

	len = str_template::strlen(prefix);
}


END_TERIMBER_NAMESPACE
