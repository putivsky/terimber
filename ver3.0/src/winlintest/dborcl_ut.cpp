#include "allinc.h"
#include "dborcl/terorcl.h"
#include <stdio.h>

int dborcl_unittest(const char* login, size_t wait, terimber_log* log)
{
	orcl_factory factory;
	dbserver* server = factory.get_dbserver(0);

	//	UID/PWD@SID

	bool res = server->connect(false, "boris/boris@orcl");

	res = server->open_sql(false, "select * from user");

	res = server->fetch_data(false, 0, 0xffffffff, true);

	while (server->next())
	{
		printf("column: %s\n", server->get_value_as_string(0));
	} 

	server->close_sql();
	server->disconnect();

	delete server;

	return 0;
}