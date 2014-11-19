#include "allinc.h"
#include "dbmysql/termysql.h"
#include <stdio.h>

int dbmysql_unittest(const char* login, size_t wait, terimber_log* log)
{
	mysql_factory factory;
	dbserver* server = factory.get_dbserver(0);

	//	UID=;PWD=;HOST=;PORT=;DB=;

	bool res = server->connect(false, "UID=boris;PWD=boris;HOST=terirack01;DB=mysql");

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
