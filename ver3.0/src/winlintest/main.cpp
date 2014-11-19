#include "allinc.h"
#include "log.h"
#include "threadpool_ut.h"
#include "socketport_ut.h"
#include "socketudp_ut.h"
#include "stargate_ut.h"
#include "file_ut.h"
#include "keymaker_ut.h"
#include "crypt_ut.h"
#include "dbmysql_ut.h"
#include "dborcl_ut.h"
//#include "aiomsg_ut.h"
#include "voice_ut.h"
#include "base/date.h"
#include "base/primitives.h"
#include "db/dbaccess.h"
#include <vector>

class db_notifier : public async_db_notify
{
public:
	// constructor
	db_notifier() : _noerrors(true) {}
	// destructor
	virtual ~db_notifier() {}

	bool wait(size_t timeout)
	{
		return WAIT_OBJECT_0 == _event.wait(timeout); 
	}

	// override base class function
	virtual void notify_async(bool noerrors, size_t ident)
	{
		// assign errors
		_noerrors = noerrors;
		
		// and fire event
		_event.set();
	}

	// reset for reusing
	void reset()
	{
		_event.reset();
		_noerrors = true;
	}

	operator bool() { return _noerrors; }
	bool operator!() { return !_noerrors; }

private:
	// terimber provides C++ implementation for event class
	TERIMBER::event _event;
	bool _noerrors;
};

static const size_t wait = 30; // 1 minute

class test_log : public terimber_log
{
public:
	virtual bool v_is_logging(size_t module, const char* file, size_t line, terimber_log_severity severity) const
	{
		return true;
	}

	virtual void v_do_logging(size_t module, const char* file, size_t line, terimber_log_severity severity, const char* msg) const
	{
		TERIMBER::date now;
		char buf[128];
		switch (severity)
		{
			case en_log_info:
				printf("info in %s at %d at %s: %s\n", file, (int)line, now.get_date(buf), msg);
				break;
			case en_log_warning:
				printf("warning in %s at %d at %s: %s\n", file, (int)line, now.get_date(buf), msg);
				break;
			case en_log_error:
				printf("error in %s at %d at %s: %s\n", file, (int)line, now.get_date(buf), msg);
				break;
			default:
				break;
		}
	}
};

#include <string>
#include <list>

#include "base/list.hpp"
#include "base/string.hpp"
#include "xml/xmlimpl.h"
#include "dbodbc/terodbc.h"
#include "dborcl/terorcl.h"
#include "dbmysql/termysql.h"

#define mysqlCase

#include <time.h>
#include <map>
#include <vector>

#include <iostream>
#include <functional>


int main()
{
	test_log log;
	test_log* plog = &log;

	printf("voice test started\n");
	voice_unittest(wait, plog);
	printf("voice test completed\n");
  
	printf("crypt test started\n");
	crypt_unittest(wait, plog);
	printf("crypt test completed\n");


	printf("thread pool test started\n");
	threadpool_unittest(wait, plog);
	printf("thread pool test completed\n");

	printf("crypt test started\n");
	crypt_unittest(wait, plog);
	printf("crypt test completed\n");
	//return 0;

	TERIMBER::xml_designer_impl p(4096);
	//p.load("\\\\server\\path\\file.ext", 0);
	p.load("c:\\path\\file.ext", 0);
	
/*
	printf("orcl test started\n");
	dborcl_unittest(0, wait, &log);
	printf("orcl test completed\n");

	printf("mysql test started\n");
	dbmysql_unittest(0, wait, &log);
	printf("mysql test completed\n");
*/
	printf("file test started\n");
	file_unittest("./unittest.dat", wait, plog);
	printf("file test completed\n");

   
	printf("socket port test started\n");
	socketport_unittest(wait, 0);
	printf("socket port test completed\n");

  /*
	printf("aiomsg test started\n");
	aiomsg_unittest(wait, plog);
	printf("aiomsg test completed\n");
  */
  printf("stargate test started\n");
	stargate_unittest(wait, 0);
	printf("stargate test completed\n");
	printf("socket port test started\n");
	socketport_unittest(wait, 0);
	printf("socket port test completed\n");


	printf("socket udp test started\n");
	socketudp_unittest(wait, 0);
	printf("socket udp test completed\n");

	printf("thread keymaker test started\n");
	keymaker_unittest(wait, plog);
	printf("thread keymaker test completed\n");
	  

	return 0;
}

