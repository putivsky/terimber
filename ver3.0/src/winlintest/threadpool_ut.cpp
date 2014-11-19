#include "base/primitives.h"
#include "threadpool/threadpoolfactory.h"

class processing_task
{
public:
	processing_task(int id) : _id(id) {}
	processing_task(const processing_task& x) : _id(x._id) {}

	void process() { _id = -_id; }

private:
	int _id;
};


class ter_thread_client : public terimber_thread_employer
{
public:
	ter_thread_client(terimber_log* log, size_t count, bool use_mtx) : _use_mutex(use_mtx), _count(count)
	{
		terimber_threadpool_factory factory;
		_stats = new size_t[_count];
		_pool = factory.get_thread_pool(log, _count, 10000); // 10 sec keep in pool
	}

	~ter_thread_client()
	{
		_start = false;
		_pool->revoke_client(this);
		// we must destroy pool here
		delete _pool;
		delete [] _stats;
	}

	// until function returns true - follow function will be called immediately
	virtual bool v_has_job(size_t ident, void* data)
	{
		if (_use_mutex)
		{
			terimber::mutexKeeper keeper(_mtx_queue);
			return (_start && _stats[ident] < 1000000000);
		}
		else
			return _start && _stats[ident] < 1000000000;
	}
	// real job should be done inside this function called in separate thread
	virtual void v_do_job(size_t ident, void* data)
	{
		_event.wait(0);
		if (_use_mutex)
		{
			terimber::mutexKeeper keeper(_mtx_queue);
			++_stats[ident];
		}
		else
			++_stats[ident];
	}


	void start()
	{
		_start = true;
		for (size_t index  = 0; index < 10; ++index)
		{
			_stats[index] = 0;
			_pool->borrow_thread(index, 0, this, 100); // 100 msec and return back to pool
		}
	}

	void print()
	{
		for (size_t index  = 0; index < _count; ++index)
			printf("thread %d has processed %d times\r\n", (int)index, (int)_stats[index]);
	}

private:
	volatile bool					_start;
	bool							_use_mutex;
	size_t							_count;
	size_t*							_stats;
	TERIMBER::mutex					_mtx_queue;
	terimber_threadpool*			_pool;
	TERIMBER::event					_event;
};

int threadpool_unittest(size_t wait, terimber_log* log)
{
	ter_thread_client client(log, 10, true);
	client.start();

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

	client.print();

	return 0;
}
