#include "base/keymaker.h"
#include "threadpool/thread.h"
#include "base/date.h"
#include "base/template.hpp"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"

const size_t client_number = 10;

const size_t client_wait = 1000;
const size_t client_update = 50;
const size_t client_pause = 50;
const size_t server_wait = 1000;
const size_t server_update = 50;
const size_t server_pause = 200;

class testlock : public terimber_thread_employer
{
public:
	testlock() : _start(false) {}
	~testlock() { stop(); }
	void start()
	{
		_start = true;

		_server1.start();
		TERIMBER::job_task stask1(this, 0, 60000, 0);
		_server1.assign_job(stask1);

		_server2.start();
		TERIMBER::job_task stask2(this, 1, 60000, 0);
		_server2.assign_job(stask2);

		for (size_t index = 0; index < client_number; ++index)
		{
			_clients[index].start();
			TERIMBER::job_task task(this, index + 2, 60000, 0);
			_clients[index].assign_job(task);
		}
	}

	void stop()
	{
		_start = false;

		_server1.cancel_job();
		_server1.stop();

		_server2.cancel_job();
		_server2.stop();

		for (size_t index = 0; index < client_number; ++index)
		{
			_clients[index].cancel_job();
			_clients[index].stop();
		}
	}

	virtual bool v_has_job(size_t ident, void* data)
	{
		return _start;
	}

	virtual void v_do_job(size_t ident, void* data)
	{
		TERIMBER::pool_object_keeper< TERIMBER::event_pool_t > waiter(&_event_pool, 0, 10000);
		switch (ident)
		{
			case 0: // server
			case 1:
				{
		      waiter->wait(server_pause);
					TERIMBER::date begin;
					char bbuf[128], ebuf[128];
					TERIMBER::keylockerWriter keeper(_locker, server_wait);
					if (!keeper)
					{
						TERIMBER::date end;
						printf("server %d timeout: begin: %s, end: %s\r\n", (int)ident, begin.get_date(bbuf), end.get_date(ebuf));
					}
					else
					{
						TERIMBER::date end;
            //printf("server %d success: begin: %s, end: %s\r\n", (int)ident, begin.get_date(bbuf), end.get_date(ebuf));					}
						waiter->wait(server_update);
          }
				}
				break;
			default: // clients
				{
          waiter->wait(client_pause);
					TERIMBER::date begin;
					char bbuf[128], ebuf[128];
					TERIMBER::keylockerReader keeper(_locker, client_wait);
					if (!keeper)
					{
						TERIMBER::date end;
						printf("client %d timeout: begin: %s, end: %s\r\n", (int)ident - 2, begin.get_date(bbuf), end.get_date(ebuf));
					}
					else
					{
						TERIMBER::date end;
						//printf("client %d success: begin: %s, end: %s\r\n", (int)ident - 2, begin.get_date(bbuf), end.get_date(ebuf));
						waiter->wait(client_update);
					}
				}
				break;
		}
	}

private:
	TERIMBER::thread		_clients[client_number];
	TERIMBER::thread		_server1;
	TERIMBER::thread		_server2;
	TERIMBER::keylocker		_locker;
	bool					_start;
	TERIMBER::event_pool_t	_event_pool;
};

int keymaker_unittest(size_t wait, terimber_log* log)
{
	testlock locker;
	TERIMBER::event ev;
	locker.start();

	// wait for a while
	size_t loops = wait;
	while (loops--)
	{
		ev.wait(1000);

		printf("working... %d\n", (int)loops);
	}

	locker.stop();
	return 0;
}
