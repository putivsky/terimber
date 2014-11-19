#include "base/primitives.h"
#include "base/memory.hpp"
#include "base/string.hpp"
#include "base/list.hpp"
#include "aiogate/aiogatefactory.h"
#include <stdlib.h>
#include <time.h>

#include "aiogate/aiogate.h"



//////////////////////////////////////////////////////
static const int MAX_MSG_SIZE = 4096;

typedef class test_msg_header_t 
{
public:
	// For portability, in case a platform has sizeof(short)!=2
	u_short size;
	u_short disconnect;
} 
test_msg_header;

typedef class test_msg_buffer_t
{
public:
	test_msg_header header;
	char buffer[MAX_MSG_SIZE];
}

test_msg_buffer;

static const int min_msg_length = 0;
static const int max_msg_length = MAX_MSG_SIZE;
static const int local_disconnect_threshold = (int)(0.01 * RAND_MAX);
static const int disconnect_req_threshold = (int)(0.01 * RAND_MAX);

#if OS_TYPE == OS_WIN32
#define PRINTF_64 "I64"
typedef __int64 ulong64;
#else
#define PRINTF_64 "ll"
typedef long long ulong64;
#endif

class test_stats 
{
public:
	test_stats(const char *name)
	{
		TERIMBER::str_template::strcpy(_name, name, sizeof(_name)-1);
		_name[sizeof(_name)-1] = '\0';
		_data_sent = 0;
		_data_rcvd = 0;
		_bad_data_rcvd = 0;
		_packets_sent = 0;
		_packets_rcvd = 0;
		_bad_packets_rcvd = 0;
		_connections_established = 0;
		_connections_closed = 0;
		_close_requests = 0;
		_localy_closed = 0;
	}

	void sent(size_t len)
	{
		TERIMBER::mutexKeeper keeper(_mutex);

		_packets_sent++;
		_data_sent += len;
	}

	void rcvd(bool good, size_t len)
	{
		TERIMBER::mutexKeeper keeper(_mutex);

		_packets_rcvd++;
		if (!good) {
			_bad_packets_rcvd++;
			_bad_data_rcvd += len;
		}
		_data_rcvd += len;
	}

	void established()
	{
		TERIMBER::mutexKeeper keeper(_mutex);
		_connections_established++;
	}

	void accepted()
	{
		TERIMBER::mutexKeeper keeper(_mutex);
		_connections_accepted++;
	}

	ulong64 established_count() const
	{
        TERIMBER::mutexKeeper keeper(_mutex);
		return _connections_established;
	}

	ulong64 accepted_count() const
	{
        TERIMBER::mutexKeeper keeper(_mutex);
		return _connections_accepted;
	}

	void closed()
	{
		TERIMBER::mutexKeeper keeper(_mutex);
		_connections_closed++;
	}

	ulong64 closed_count() const
	{
        TERIMBER::mutexKeeper keeper(_mutex);
        return _connections_closed;
	}

	void close_request()
	{
        TERIMBER::mutexKeeper keeper(_mutex);
		_close_requests++;
	}

	void localy_closed() 
	{
		TERIMBER::mutexKeeper keeper(_mutex);
		_localy_closed++;
	}

	void dump()
	{
		printf("%s\n", _name);
		printf("Data bytes sent %" PRINTF_64 "u, received %" PRINTF_64 "u, out of them bad %" PRINTF_64 "u\n",
			_data_sent, _data_rcvd, _bad_data_rcvd);
		printf("Messages sent %" PRINTF_64 "u, received %" PRINTF_64 "u, out of them bad %" PRINTF_64 "u\n",
			_packets_sent, _packets_rcvd, _bad_packets_rcvd);
		printf("Connections established %" PRINTF_64 "u, accepted %" PRINTF_64 "u, closed %" PRINTF_64 "u\n",
			   _connections_established, _connections_accepted, _connections_closed);
		printf("Rcved reqs to close connection %" PRINTF_64 "u, localy closed %" PRINTF_64 "u\n",
			   _close_requests, _localy_closed);
	}

private:
	ulong64 _data_sent;
	ulong64 _data_rcvd;
	ulong64 _bad_data_rcvd;
	ulong64 _packets_sent;
	ulong64 _packets_rcvd;
	ulong64 _bad_packets_rcvd;
	ulong64 _connections_accepted;
	ulong64 _connections_established;
	ulong64 _connections_closed;
	ulong64 _close_requests;
	ulong64 _localy_closed;
	TERIMBER::mutex _mutex;
	char _name[60];
};

static test_stats west_stats("West stats:");
static test_stats east_stats("East stats:");

class stargate_pin_impl : public terimber_aiogate_pin
{
public:
	stargate_pin_impl(bool west) :  _ident(0), _west(west), _gate(0), _offset(0) 
	{
		memset(&_buf.header, 0, sizeof(_buf.header));
	}

	// accept new incoming pin
	virtual void on_accept(	const sockaddr_in& local, // ip/port of this pin
							const sockaddr_in& remote, // ip/port of remote peer
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // stargate itself
							)
	{
		_west ? west_stats.accepted() : east_stats.accepted();
		_ident = ident;
		_gate = callback;
		// initiate recv
		_gate->recv(_ident, true, 0);
		message_out();
	}

	// connect to the new pin
	virtual void on_connect(const sockaddr_in& local,  // ip/port of this pin
							const sockaddr_in& remote, // ip/port of remote peer
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // stargate itself
							)
	{
		_west ? west_stats.established() : east_stats.established();
		_ident = ident;
		_gate = callback;
		// initiate recv
		_gate->recv(_ident, true, 0);
		message_out();
	}
	
	// stargate calls this callback when bytes came from peer
	// return true if immediately more bytes is expected stargate automatically invoke recv method - just for performance optimization
	virtual bool on_recv(	const void* buf,  // array of bytes for sending
							size_t len, // the length of array of bytes
							const sockaddr_in& peeraddr,
							bool& expected_more
							)
	{
		size_t msgs = 0;
		size_t processed = 0;
		
		while (processed < len)
		{
			bool ok = false, complete = false;
	
			size_t msg_size = message_ok(buf, len, ok, complete);
			processed += msg_size;

			if (!ok)
				break;

			if (!complete)
				continue;

			++msgs;
			
			_buf.header.size = 0;
			_buf.header.disconnect = 0;
			_offset = 0;
			_west ? west_stats.rcvd(ok, msg_size) : east_stats.rcvd(ok, msg_size);
		}

		expected_more = (msgs != 0 && processed == len);

		if (msgs)
			message_out();
		// always request new recv
		return true;
	}

	virtual void on_bind(const sockaddr_in& local,  // ip/port of this pin
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // aiogate itself
							)
	{
		assert(false);
	}

	// stargate invokes this callback only if all bytes have been sent
	virtual void on_send(const sockaddr_in& peeraddr)
	{
		// nothing to do
	}
	
	// stargate invoke this function when pin connection is deactivate
	// only internal action can be taken - no stargate calls anymore for this pin
	// @mask - see above definitions
	virtual void on_close(ub4_t mask)
	{
		// TO DO - report error
		switch (mask)
		{
			case aiogate_accept_mask:
				printf("pin on %s side can not accept to its peer\n", _west ? "west" : "east");
				break;
			case aiogate_connect_mask:
				printf("pin on %s side can not connect to its peer\n", _west ? "west" : "east");
				break;
			case aiogate_send_mask:
				printf("pin on %s side can not send a message\n", _west ? "west" : "east");
				break;
			case aiogate_recv_mask:
				printf("pin on %s side can not receive a message\n", _west ? "west" : "east");
				break;
		} // switch
	}
private:
	void message_out()
	{
		size_t random = rand();
		size_t len;
		char start = (char)((random >> 4) & 0xff);
		bool local_disconnect = false;
			//random < local_disconnect_threshold;
		bool disconnect_req = false;
			//!local_disconnect && (random < (local_disconnect_threshold + disconnect_req_threshold));

		if (local_disconnect) 
		{
			_west ? west_stats.localy_closed() : east_stats.localy_closed();
			_gate->close(_ident);
			return;
		}

		len = min_msg_length;
		len += random % (max_msg_length - min_msg_length);

		test_msg_buffer_t buf;

		buf.header.size = htons((u_short) len);
		buf.header.disconnect = disconnect_req ? 1 : 0;
		for (size_t i = 0; i < len; ++i) 
		{
			buf.buffer[i] = start++;
		}

		size_t msg_len = sizeof(buf.header) + len;
		// initiate send

		if (_gate->send(_ident, &buf, msg_len, 0))
		{
			_west ? west_stats.sent(msg_len) : east_stats.sent(msg_len);
		}
	}

	size_t message_ok(const void* msg, size_t len, bool& ok, bool& complete)
	{
		ok = false;
		complete = false;
		size_t consumed = 0;

		if (_offset == 0 && len < sizeof(_buf.header))// || (len + _offset > MAX_MSG_SIZE + sizeof(buf.header))) 
		{
			assert(false);
			return 0;
		}

		if (_offset == 0)
		{
			memcpy(&_buf.header.size, msg, sizeof(_buf.header.size));
			memcpy(&_buf.header.disconnect, (unsigned const char*)msg + sizeof(_buf.header.size), sizeof(_buf.header.disconnect));
			_offset += sizeof(_buf.header);
			consumed += sizeof(_buf.header);
		}

		size_t expected_size = ntohs(_buf.header.size);
		size_t actual_length = __min(expected_size + sizeof(_buf.header) - _offset , len - consumed);
		memcpy(_buf.buffer + _offset - sizeof(_buf.header), (unsigned char*)msg + consumed, actual_length);
		_offset += actual_length;
		consumed += actual_length;

		// Just in case if, on the target platform, short is not 16 bits
		// This @#$%^ MS compiler is not C99 compliant, so there is no
		// int16_t type
		// The first 2 bytes is the size
		size_t size = ntohs(_buf.header.size);
		
		if (_offset - sizeof(_buf.header) < size) 
		{
			ok = true;
			return consumed;
		}

		if (size) 
		{
			char start = _buf.buffer[0];
			for (size_t i = 1; i < size; ++i) 
			{
				if (_buf.buffer[i] != ++start) 
				{
					printf("expected %d, received %d\n", _buf.buffer[i], start);
					//assert(false);
					ok = false;
					complete = true;
					return consumed;
				}
			}
		}

		if (_buf.header.disconnect == 1) 
		{
			_west ? west_stats.close_request() : east_stats.close_request();
			_gate->close(_ident);
			ok = false;
			complete = true;
			return consumed;
		}

		ok = true;
		complete = true;
		return consumed;
	}

private:
	size_t				_ident;
	bool				_west;
	terimber_aiogate*	_gate;
	test_msg_buffer_t	_buf;
	size_t				_offset;
};

class pin_factory_impl : public terimber_aiogate_pin_factory
{
	typedef TERIMBER::node_allocator< pin_factory_impl > pin_allocator_t;
public:
	virtual ~pin_factory_impl()
	{
	}
	// creates new pin
	// @arg - user data for internal use
	// stargate will send it as a parameter
	virtual terimber_aiogate_pin* create(void* arg)
	{
		stargate_pin_impl* obj = (stargate_pin_impl*)_all.allocate();
		if (obj)
			new (obj) stargate_pin_impl((size_t)arg == 1);

		return obj;
	}
	
	// destroy pin
	virtual void destroy(terimber_aiogate_pin* pin)
	{
		pin->~terimber_aiogate_pin();
		_all.deallocate(pin);
	}

private:
	pin_allocator_t	_all;
};

static const char* west_address = "localhost";
static const char* east_address = "localhost";
static const unsigned short west_port = 7333;
static const unsigned short east_port = 7444;

int stargate_unittest(size_t wait, terimber_log* log)
{
	// initiate srand library
	srand((unsigned int)time(0));

	pin_factory_impl west_pin_factory;
	pin_factory_impl east_pin_factory;

	// create stargate
	terimber_aiogate_factory factory;
	terimber_aiogate* westgate = factory.get_terimber_aiogate(log, 3);
	terimber_aiogate* eastgate = factory.get_terimber_aiogate(log, 3);

	size_t id_west = westgate->listen(west_address, west_port, 16, 16, &west_pin_factory, (void*)1);
	if (id_west == 0)
	{
		printf("can not start listener on addess: %s, port: %d\n", west_address, west_port);
		delete westgate;
		// destroy east gate
		delete eastgate;
		return -1;
	}

	if (!eastgate->connect(west_address, west_port, 0, 0, 5000, &east_pin_factory, (void*)2))
	{
		printf("can not connect to addess: %s, port: %d\n", west_address, west_port);
		westgate->deaf(id_west);
		delete westgate;
		// destroy east gate
		delete eastgate;
		return -1;
	}

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

	// destroy west gate
	// stop listener
	printf("wake listener deaf\n");
	westgate->deaf(id_west);

	printf("destroy west gate\n");
	delete westgate;


	printf("destroy east gate\n");
	// destroy east gate
	delete eastgate;

	// print stats
	west_stats.dump();
	east_stats.dump();


	return 0;
}
