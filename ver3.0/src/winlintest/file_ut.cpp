#include "allinc.h"
#include "aiofile/aiofilefactory.h"
#include "base/primitives.h"

const size_t BUFFER_SIZE = 1024;

class file_unittest_callback : public terimber_aiofile_callback, public terimber_log_helper
{
public:  
	file_unittest_callback(terimber_log* log, terimber_aiofile* port, size_t len, bool readwrite) : _port(port), _len(len), _offset(0), _readwrite(readwrite), _done(false), _err(0)
	{
		log_on(log);
	}

	~file_unittest_callback()
	{
		log_on(0);
	}

	// port will call function after error occured
	virtual void v_on_error(size_t handle, int err, aiofile_type type, void* userdata)
	{
		TERIMBER::mutexKeeper keeper(_mtx);
		_err = err;
		_done = true;
	}

	// port will call function after successful sending buffer to file
	virtual void v_on_write(size_t handle, void* buf, size_t requested, size_t processed, void* userdata)
	{
		assert(_readwrite == false);
		TERIMBER::mutexKeeper keeper(_mtx);
		if (_done)
			return;

		// adjust offset
		_offset += processed;

		if (_offset >= _len)
		{
			_done = true;
			return;
		}

		keeper.unlock();
		_port->write(handle, _offset, _buf, __min(BUFFER_SIZE, _len - _offset), INFINITE, 0);
	}

	// port will call function after successful receiving buffer from file
	virtual void v_on_read(size_t handle, void* buf, size_t requested, size_t processed, void* userdata)
	{
		assert(_readwrite == true);
		TERIMBER::mutexKeeper keeper(_mtx);
		if (_done)
			return;

		// adjust offset
		_offset += processed;

		if (_offset >= _len)
		{
			_done = true;
			return;
		}

		keeper.unlock();
		_port->read(handle, _offset, _buf, __min(BUFFER_SIZE, _len - _offset), INFINITE, 0);
	}


	void begin(size_t handle)
	{
		_offset = 0;
		if (_readwrite)
		{
			_port->read(handle, _offset, _buf, __min(BUFFER_SIZE, _len - _offset), INFINITE, 0);
		}
		else
		{
			_port->write(handle, _offset, _buf, __min(BUFFER_SIZE, _len - _offset), INFINITE, 0);
		}
	}

	bool done()
	{
		TERIMBER::mutexKeeper keeper(_mtx);
		return _done;
	}

	int err()
	{
		TERIMBER::mutexKeeper keeper(_mtx);
		return _err;
	}

	void stop()
	{
		TERIMBER::mutexKeeper keeper(_mtx);
		_done = true;
	}
private:
	terimber_aiofile*	_port;
	size_t				_len;
	char				_buf[BUFFER_SIZE];
	size_t				_offset;
	bool				_readwrite;
	bool				_done;
	int					_err;
	TERIMBER::mutex		_mtx;
};

int file_unittest(const char* name, size_t wait, terimber_log* log)
{
	terimber_aiofile_factory acc;
	// create write file
	terimber_aiofile* afile = acc.get_aiofile(log);
	file_unittest_callback writer(log, afile, 1024*1024, false), reader(log, afile, 1024*1024, true);

	size_t fw = afile->open(name, false, &writer);
	if (!fw)
	{
		printf("can not open file for writing: %s\n", name);
		return -1;
	}

	writer.begin(fw);

	TERIMBER::event ev;

	printf("writing bytes...\n");
	// wait for a while
	size_t loops = wait;
	while (loops--)
	{
		ev.wait(1000);
		printf("working... %d\n", (int)loops);
		if (writer.done())
			break;
	}

	printf("writer stop\n");
	writer.stop();
	printf("close file\n");
	afile->close(fw);
	printf("writing is done with error code: %d\n", writer.err());

	size_t fr = afile->open(name, true, &reader);
	if (!fr)
	{
		printf("can not open file for reading: %s\n", name);
		return -1;
	}

	reader.begin(fr);

	printf("reading bytes...\n");
	// wait for a while
	loops = wait;
	while (loops--)
	{
		ev.wait(1000);
		printf("working... %d\n", (int)loops);
		if (reader.done())
			break;
	}

	reader.stop();
	afile->close(fr);
	delete afile;
	printf("reading is done with error code: %d\n", reader.err());
	return 0;
}
