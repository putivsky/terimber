#include "osdetect.h"

#if OS_TYPE == OS_WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1200) 
#include <winsock2.h>
#endif
#endif

#include "allinc.h"
#include "aiosock/aiosockfactory.h"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"

const size_t buf_size_max = 128;//1024;

class ter_aioport_client : public terimber_aiosock_callback
{
public:
	ter_aioport_client(terimber_aiosock* sp, size_t ident, ter_aioport_client* sub) : 
		_count_send(0), _count_recv(0),  _sp(sp), _stop(false), _ident(ident), _sub(sub)
	{
	}

	~ter_aioport_client()
	{
	}

	void stop()
	{
		_stop = true;
	}

	void clear()
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);
		for (TERIMBER::list<size_t>::const_iterator iter = _list_handle_accepted.begin(); iter != _list_handle_accepted.end(); ++iter)
			_sp->close(*iter);

		_list_handle_accepted.clear();

	}

	// pool will call function after error occured
	virtual void v_on_error(size_t handle, int err, aiosock_type mask, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);
		printf("error: %d on handle %d\n", err, (int)handle);
		if (_stop)
			return;
	}

	// pool will call function after error occured
	virtual void v_on_connect(size_t handle, const sockaddr_in& peeraddr, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);
		if (_stop)
			return;

		_connect_handle = handle;
		++_count_send;
		_sp->send(handle, _buf, buf_size_max, INFINITE, 0, 0);
	}

	// pool will call function after successful sending buffer to socket
	virtual void v_on_send(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);

		if (_stop)
			return;

		if (!processed)
		{
			printf("send failed - zero bytes, handle %d\n", (int)handle);
			return;
		}

	
		if (processed < requested)
		{
			++_count_send;
			printf("incomplete send need more %d bytes, handle %d\n", (int)(requested-processed),  (int)handle);
			if (int err = _sp->send(handle, (char*)_buf, requested - processed, INFINITE, 0, 0))
				printf("send failed: %d handle %d\n", err, (int)handle);
		}
		else
		{
			++_count_recv;
			if (int err = _sp->receive(handle, (char*)_buf1, buf_size_max, INFINITE, 0, 0))
				printf("recv failed: %d handle %d\n", err, (int)handle);
		}
	}

	// pool will call function after successful receiving buffer from socket
	virtual void v_on_receive(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);

		if (_stop)
			return;

		if (!processed)
		{
			printf("recv failed - zero bytes, handle %d\n", (int)handle);
			return;
		}

		if (_sub)
		{
			printf("v_on_receive sever\n");
		}

		if (processed < requested)
		{
			++_count_recv;
			if (int err = _sp->receive(handle, _buf1, requested - processed, INFINITE, 0, 0))
				printf("rec failed: %d handle %d\n", err, (int)handle);
		}
		else
		{
			++_count_send;
			if (int err = _sp->send(handle, _buf, buf_size_max, INFINITE, 0, 0))
				printf("send failed: %d handle %d\n", err, (int)handle);
		}

	}
	

	// pool will call function after successful accepting the new incoming connection
	virtual void v_on_accept(size_t handle, size_t handle_accepted, terimber_aiosock_callback*& callback, const sockaddr_in& peeraddr, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);

		if (_stop)
			return;
		{
			//TERIMBER::mutexKeeper keeper(_list_mtx);
			_list_handle_accepted.push_back(handle_accepted);
		}

		if (_sub)
		{
			callback = _sub;
			_sub->_connect_handle = handle_accepted;
			++_sub->_count_recv;
		}
		else
		{
			_connect_handle = handle_accepted;
			++_count_recv;
		}

		++_count_recv;
		_sp->receive(handle_accepted, _buf1, buf_size_max, INFINITE, 0, 0);
	}

	void activate()
	{
		if (int err = _sp->receive(_connect_handle, _abuf1, buf_size_max, INFINITE, 0, 0))
			printf("recv activate failed: %d handle %d\n", err, (int)_connect_handle);
 

		if (int err = _sp->send(_connect_handle, _abuf, buf_size_max, INFINITE, 0, 0))
			printf("send activate failed: %d handle %d\n", err, (int)_connect_handle);
	}
public:
	char _buf[buf_size_max];
	char _buf1[buf_size_max];
	char _abuf[buf_size_max];
	char _abuf1[buf_size_max];
	
	size_t						_count_send;
	size_t						_count_recv;
	TERIMBER::mutex				_list_mtx;
	TERIMBER::list<	size_t	>	_list_handle_accepted;
private:
	terimber_aiosock*			_sp;
	bool						_stop;
	size_t					_ident;
	ter_aioport_client*			_sub;
	size_t 						_connect_handle;
};

static const char* server_address = "localhost";
//static const char* server_address = "192.168.10.31";
static const unsigned short server_port = 8333;

int socketport_unittest(size_t wait, terimber_log* log)
{
	int err = 0;
	terimber_aiosock_factory acc;
	terimber_aiosock* _port = acc.get_aiosock(log, 3, 60000);
	terimber_aiosock* _port2 = acc.get_aiosock(log, 3, 60000);
	

	ter_aioport_client aio_server(_port, 10, 0);

	ter_aioport_client aio_listener(_port, 0, &aio_server);
	size_t listener = _port->create(&aio_listener, true);
	
	if (!listener)
	{
		printf("can not create listener\n");
		delete _port;
		delete _port2;
		return -1;
	}

	int aioint = _port->listen(listener, server_port, 1, 0, 8, 0);
	if (aioint)
	{
		printf("can not start listener: %d\n", aioint);
		delete _port;
		delete _port2;
		return -1;
	}

	ter_aioport_client aio_west(_port2, 1, 0);
	size_t west_sender = _port2->create(&aio_west, true);

	printf("c?%d->%d\n", 1, 10);
	aioint = _port2->connect(west_sender, server_address, server_port, 60000, 0);

	///*
	ter_aioport_client aio_east(_port2, 2, 0);
	size_t east_sender = _port2->create(&aio_east, true);
	printf("c?%d->%d\n", 2, 10);
	aioint = _port2->connect(east_sender, server_address, server_port, 60000, 0);
	//*/
	TERIMBER::event ev;

	printf("wait on event\n");
	// wait for a while
	size_t loops = wait;
	while (loops--)
	{
		ev.wait(1000);

		printf("working... %d\n", (int)loops);

		//aio_west.activate();
		//aio_east.activate();
		//aio_server.activate();

	}

	printf("wake up wait on event\n");

	aio_listener.stop();
	aio_west.stop();
	aio_east.stop();

	_port->close(listener);
	
	_port2->close(east_sender);
	_port2->close(west_sender);
	// print results
	printf("listener accepted %d\r\n", (int)aio_listener._list_handle_accepted.size());
	printf("server send %d, recv %d\r\n", (int)aio_server._count_send, (int)aio_server._count_recv);
	printf("east client send %d, recv %d\r\n", (int)aio_east._count_send, (int)aio_east._count_recv);
	printf("west client send %d, recv %d\r\n", (int)aio_west._count_send, (int)aio_west._count_recv);  

	aio_listener.clear();

	printf("destroy port 1\r\n");
	delete _port;
	printf("destroy port 2\r\n");
	delete _port2;

	printf("done\r\n");


	return 0;
}
