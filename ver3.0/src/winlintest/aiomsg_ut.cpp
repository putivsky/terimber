#include "osdetect.h"

#if OS_TYPE == OS_WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1200) 
#include <winsock2.h>
#endif
#endif

#include "allinc.h"
#include "aiomsg/aiomsgfactory.h"
#include "aiomsg/msg_comm.h"

#include "base/list.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/template.hpp"

const size_t buf_size_max = 1024;

static const char* ini_xml_format_rpc =\
"<msgPort address=\"%s\"><listeners><listener type=\"rpc\" ping=\"1000000\" /></listeners><connections><connection address=\"%s\" type=\"rpc\" ping=\"1000000\" /></connections></msgPort>";

static const char* ini_xml_format_sock =\
"<msgPort address=\"%s\"><listeners><listener network=\"localhost\" port=\"%d\" type=\"sock\" ping=\"1000000\" /></listeners><connections><connection address=\"%s\" network=\"localhost\" port=\"%d\" type=\"sock\" ping=\"1000000\" /></connections></msgPort>";

static const char* post_init = "post initiated";
static const char* send_init = "send initiated";
static const char* send_async_init = "send async initiated";
static const char* send_reply = "send reply";

static ub4_t post_init_id = 1;
static ub4_t send_init_id = 2;
static ub4_t send_async_init_id = 3;
static ub4_t send_reply_id = 4;

const size_t max_timeout = 1000*100;

//! \class msg_creator
//! \brief msg resource keeper
class aio_msg_creator : public TERIMBER::proto_creator< aio_msg_creator, msg_t, size_t >
{
public:
	//! \brief constructor 
	aio_msg_creator(	terimber_aiomsg* communicator			//!< pointer to communicator
					) : 
		_communicator(communicator) 
	{
	}
	//! \brief creator function
	msg_t* 
	create(			size_t n								//!< message size
					) 
	{ 
		return _communicator->construct(n); 
	}
	//! \brief destroy function
	void 
	destroy(		msg_t* obj
					) 
	{ 
		if (obj) 
			_communicator->destroy(obj); 
	}
private:
	terimber_aiomsg*	_communicator;						//!< pointer to communicator
};

//! \typedef msg_pointer_t
//! \brief smart pointer to message creator
typedef TERIMBER::smart_pointer< aio_msg_creator > aio_msg_pointer_t;



class ter_aiomsg_client : public msg_callback_notify
{
public:
	ter_aiomsg_client(	const guid_t& myself,
						const guid_t& buddy,
						unsigned short myport,
						unsigned short buddyport,
						terimber_log* log)
		: 
		_myself(myself), _buddy(buddy), _count_send(0), _count_recv(0), _stop(false)
	{
		char buf[1024];
		char g1[33];
		char g2[33];
		TERIMBER::str_template::strprint(buf, sizeof(buf), ini_xml_format_rpc, TERIMBER::guid_to_string(g1, _myself), TERIMBER::guid_to_string(g2, _buddy));
		//TERIMBER::str_template::strprint(buf, sizeof(buf), ini_xml_format_sock, TERIMBER::guid_to_string(g1, _myself), myport, TERIMBER::guid_to_string(g2, _buddy), buddyport);
		aiomsgfactory factory;
		_communicator = factory.get_aiomsg(log);
		size_t len = strlen(buf);
		bool res = _communicator->init(buf, len);
		assert(res);
		res = _communicator->start(this, 3);
		assert(res);
	}

	~ter_aiomsg_client()
	{
		if (_communicator)
		{
			_communicator->stop();
			_communicator->uninit();
			delete _communicator;
		}
	}

	void stop()
	{
		_stop = true;
	}

	//! \brief callback function for incoming messages 
	//! if sender used post function then reply will be null - sender isn't waiting for reply
	//! if function return true then caller isn't responsible for msg destruction anymore
	//! internal code will posses the msg memory
	virtual 
	bool 
	incoming_callback(	msg_t* msg,							//!< pointer to the incoming message
						msg_t* reply						//!< pointer to the outgoing message
						)
	{
		if (_stop)
			return false;


		++_count_recv;
		msg->majver = 1;
		msg->minver = 0;
		msg->priority = 0;
		msg->timeout = max_timeout;

		if (!_communicator->set_receiver(msg, _buddy))
			return false;


	// check if it's posty
		if (reply) // send
		{
			size_t len = strlen(post_init);
			if (!_communicator->resize(msg, len))
				return false;
			
			if (!_communicator->write_buffer(msg, 0, post_init, len))
				return false;

			msg->msgid = post_init_id;
			if (!_communicator->post(false, msg))
				return false;

			_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "post: %d", this);


			len = strlen(send_reply);
			if (!_communicator->resize(reply, len))
				return true;

			reply->msgid = send_reply_id;
			if (!_communicator->write_buffer(reply, 0, send_reply, len))
				return true;

			++_count_send;
		}
		else // post
		{
			size_t len = strlen(send_async_init);
			if (!_communicator->resize(msg, len))
				return false;

			if (!_communicator->write_buffer(msg, 0, send_async_init, len))
				return false;
		
			msg->msgid = send_async_init_id;
			guid_t ret = _communicator->send_async(false, msg);
			if (!memcmp(&ret, &null_uuid, sizeof(null_uuid)))
				return false;

			_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "sendasync: %d", this);

			++_count_send;
		}

		return true;
	}
	//! \brief callback function for asynchronous replies
	//! if function return true then caller isn't responsible for reply destruction anymore
	//! internal code will posses the reply memory 
	virtual 
	bool 
	async_callback(		msg_t* reply,						//!< pointer to the asynchronous reply
						const guid_t& ident					//!< unique ident for asynchronous replies
						)
	{
		if (_stop)
			return false;

		return false;
		/*
		++_count_send;

		size_t len = strlen(send_init);
		if (!_communicator->resize(reply, len))
			return false;

		reply->majver = send_init_id;
		reply->minver = 0;
		reply->priority = 0;
		reply->timeout = max_timeout;

		if (!_communicator->set_receiver(reply, _buddy))
			return false;


		if (!_communicator->write_buffer(reply, 0, send_init, len))
			return false;


		
		aio_msg_creator cr(_communicator);
		aio_msg_pointer_t msg(cr);

		if (!_communicator->send(false, reply, &msg))
			return false;

		_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "send: %d", this);

		++_count_send;
		return true;
		*/
	}

	// - activate post(1) -> on_post(2)->send_async(2)->on_async(1)->send(1)->on_send(2)->post(2)->async, ...
	void activate()
	{
		size_t len = strlen(post_init);
		aio_msg_creator cr(_communicator);
		aio_msg_pointer_t msg(cr, len);

		msg->majver = post_init_id;
		msg->minver = 0;
		msg->priority = 0;
		msg->timeout = max_timeout;
		
		if (!_communicator->set_receiver(msg, _buddy))
			return;

		if (!_communicator->write_buffer(msg, 0, post_init, len))
			return;
		
		if (!_communicator->post(false, msg))
			return;

		_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "post: %d", this);
		
		msg.detach();

		++_count_send;
	}

public:
	size_t						_count_send;
	size_t						_count_recv;
private:
	terimber_aiomsg*			_communicator;
	bool						_stop;
	int							_ident;
	const guid_t				_buddy;
	const guid_t				_myself;
};

int aiomsg_unittest(size_t wait, terimber_log* log)
{
const guid_t gclient1 = {0x7cf7d181, 0x63c1, 0x41a1, {0x96, 0x1d, 0x2f, 0x5e, 0x6f, 0xbf, 0x2d, 0xf1}};

// {757B8924-E6E4-4f53-BD68-29D056966903}
//0x757b8924, 0xe6e4, 0x4f53, 0xbd, 0x68, 0x29, 0xd0, 0x56, 0x96, 0x69, 0x3);
const guid_t gclient2 = {0x757b8924, 0xe6e4, 0x4f53, {0xbd, 0x68, 0x29, 0xd0, 0x56, 0x96, 0x69, 0x03}};

	ter_aiomsg_client client1(gclient1, gclient2, 3333, 4444, 0);//log);
	ter_aiomsg_client client2(gclient2, gclient1, 4444, 3333, 0);//log);

	client1.activate();
	//client2.activate();

	TERIMBER::event ev;

	printf("wait on event\n");
	// wait for a while
	size_t loops = wait;
	while (loops--)
	{
		ev.wait(1000);
		//client1.activate();
		//client2.activate();

		printf("working... %d\n", loops);
	}

	printf("wake up wait on event\n");


	client1.stop();
	client2.stop();

	printf("client1 send %d, recv %d\r\n", client1._count_send, client1._count_recv);
	printf("client2 send %d, recv %d\r\n", client2._count_send, client2._count_recv);  

	return 0;
}
