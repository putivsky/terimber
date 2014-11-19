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

#include "cache/cache.h"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "smart/varmap.hpp"
#include "smart/varvalue.hpp"
#include "xml/sxs.hpp"
#include "base/xmlconfig.h"

TERIMBER::cache_daemon g_cache_daemon;

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const char* usage = "Usage: cache.exe [ip] [port] {max_connections} {number of acceptors}";

cache_daemon::cache_daemon() : _stargate(3, 60000), _listener_ident(0)
{
	_daemonName = "TerimberCache";
	_daemonDesc = "Terimber Searchable Cache Daemon";
}

// virtual 
bool 
cache_daemon::v_on_init(unsigned long argc, char* argv[], const char* cfg_file, string_t& err)
{

	string_t address("localhost");
	int port = 3333;
	size_t max_connection = 3; // default values
	size_t buffered_acceptors = 32; // default value

	xmlconfig settings(cfg_file, "cache", 0);

	if (!settings.get(0, "address", address)
		|| !settings.get(0, "port", port))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "xmlconfig failed, config file: %s", cfg_file);
	}

	_stargate.log_on(this);

	if (!_stargate.on())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "Can not start Terimber aiogate object");
		return false;
	}

	// starts listeners here
	_listener_ident = _stargate.listen(address, (unsigned short)port, max_connection, (unsigned short)buffered_acceptors, &_factory, 0);

	if (!_listener_ident)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "Can not start listener on address %s and port %hu", (const char*)address, port);
		return false;
	}

	return true;
}

// virtual 
void 
cache_daemon::v_on_uninit()
{
	// destroys stargate, if any
	_stargate.off();
	_stargate.log_on(0);
}

// virtual 
void 
cache_daemon::v_on_handler(DAEMON_HANDLE_FUNCTION_ARG code)
{
#if OS_TYPE != OS_WIN32
	switch (code)
	{
		case SIGHUP: // reloads
			on_shutdown();
			on_startup(0, 0);
			break;
	}
#endif
}
#pragma pack()
END_TERIMBER_NAMESPACE
