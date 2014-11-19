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

class ter_adgport_client : public terimber_aiosock_callback
{
public:
	ter_adgport_client(terimber_aiosock* sp) : 
		_count_send(0), _count_recv(0), _sp(sp), _stop(false), _connect_handle(0)
	{
	}

	~ter_adgport_client()
	{
	}

	void stop()
	{
		_stop = true;
	}

	// pool will call function after error occured
	virtual void v_on_error(size_t handle, int err, aiosock_type mask, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);
		printf("error: %d on handle %d\n", err, (int)handle);
		if (_stop)
			return;
	}

	// pool will call function after successful sending buffer to socket
	virtual void v_on_send(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& toaddr, void* userdata)
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
			//printf("incomplete send need more %d bytes, handle %d\n", requested-processed,  handle);
			if (int err = _sp->send(handle, (char*)_buf, requested - processed, INFINITE, &toaddr, 0))
				printf("send failed: %d handle %d\n", err, (int)handle);

		}
		else
		{
			//printf("send succeeded: handle %d\n", handle);
			++_count_recv;
			if (int err = _sp->receive(handle, (char*)_buf1, buf_size_max, INFINITE, &toaddr, 0))
				printf("recv failed: %d handle %d\n", err, (int)handle);
		}
	}

	// pool will call function after successful receiving buffer from socket
	virtual void v_on_receive(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& fromaddr, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_list_mtx);
		if (_stop)
			return;

		if (!processed)
		{
			printf("recv failed - zero bytes, handle %d\n", (int)handle);
			return;
		}

		if (processed < requested)
		{
			//printf("incomplete recv need more %d bytes, handle %d\n", requested-processed,  handle);
			++_count_recv;
			if (int err = _sp->receive(handle, _buf1, requested - processed, INFINITE, &fromaddr, 0))
				printf("rec failed: %d handle %d\n", err, (int)handle);
		}
		else
		{
			//printf("recv succeeded: handle %d\n", handle);
			++_count_send;
			if (int err = _sp->send(handle, _buf, buf_size_max, INFINITE, &fromaddr, 0))
				printf("send failed: %d handle %d\n", err, (int)handle);
		}

	}

	// port will call function after successful connection to socket
	virtual void v_on_connect(size_t handle, const sockaddr_in& peeraddr, void* userdata)
	{
		assert(false);
	}

	virtual void v_on_accept(size_t handle, size_t handle_accepted, terimber_aiosock_callback*& callback, const sockaddr_in& peeraddr, void* userdata)
	{
		assert(false);
	}


	void activate(bool send_or_recv, size_t connect_handle, const sockaddr_in& fromaddr, const sockaddr_in& toaddr)
	{
		_connect_handle = connect_handle;

		if (!send_or_recv)
		{
			if (int err = _sp->receive(_connect_handle, _abuf1, buf_size_max, INFINITE, &fromaddr, 0))
				printf("recv activate failed: %d handle %d\n", err, (int)_connect_handle);
		}
		else
		{
			if (int err = _sp->send(_connect_handle, _abuf, buf_size_max, INFINITE, &toaddr, 0))
				printf("send activate failed: %d handle %d\n", err, (int)_connect_handle);
		}
	}
public:
	char _buf[buf_size_max];
	char _buf1[buf_size_max];
	char _abuf[buf_size_max];
	char _abuf1[buf_size_max];
	
	size_t						_count_send;
	size_t						_count_recv;
private:
	TERIMBER::mutex				_list_mtx;
	terimber_aiosock*			_sp;
	bool						_stop;
	size_t 						_connect_handle;
};

static const char* server_address = "localhost";
//static const char* server_address = "192.168.10.31";
static const unsigned short server_port = 8333;

int socketudp_unittest(size_t wait, terimber_log* log)
{
	int err = 0;
	terimber_aiosock_factory acc;
	terimber_aiosock* _port1 = acc.get_aiosock(log, 3, 60000);
	terimber_aiosock* _port2 = acc.get_aiosock(log, 3, 60000);

	ter_adgport_client adg1(_port1);
	ter_adgport_client adg2(_port1);

	size_t id1 = _port1->create(&adg1, false);
	
	if (!id1)
	{
		printf("can not create id1\n");
		delete _port1;
		delete _port2;
		return -1;
	}

	size_t id2 = _port1->create(&adg2, false);

	if (!id2)
	{
		printf("can not create id2\n");
		delete _port1;
		delete _port2;
		return -1;
	}


	if (_port1->bind(id1, "localhost", 3333))
	{
		printf("can not bind id1\n");
		delete _port1;
		delete _port2;
		return -1;
	}

	if (_port1->bind(id2, "localhost", 4444))
	{
		printf("can not bind id2\n");
		delete _port1;
		delete _port2;
		return -1;
	}


	// get the sock address
	sockaddr_in addr1, addr2;
	if (_port1->getsockaddr(id1, addr1))
	{
		printf("can not get address id1\n");
		delete _port1;
		delete _port2;
		return -1;
	}

	if (_port1->getsockaddr(id2, addr2))
	{
		printf("can not get address id2\n");
		delete _port1;
		delete _port2;
		return -1;
	}

	adg1.activate(false, id1, addr1, addr2);
	adg2.activate(false, id2, addr2, addr1);

	adg1.activate(true, id1, addr1, addr2);
	adg2.activate(true, id2, addr2, addr1);

	TERIMBER::event ev;

	printf("wait on event\n");
	// wait for a while
	size_t loops = wait;
	while (loops--)
	{
		ev.wait(1000);

		printf("working... %d\n", (int)loops);
	}

	printf("wake up wait on event\n");


	adg1.stop();
	adg2.stop();

	_port1->close(id1);
	_port1->close(id2);

	// print results
	printf("1 client send %d, recv %d\r\n", (int)adg1._count_send, (int)adg1._count_recv);
	printf("2 client send %d, recv %d\r\n", (int)adg2._count_send, (int)adg2._count_recv);  

	delete _port1;
	delete _port2;

	return 0;
}
