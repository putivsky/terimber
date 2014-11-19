#ifndef _filelock_h_
#define _filelock_h_

#include "threadpool/thread.h"
#include "base/string.h"

class filelocker
{
public:
	filelocker(const char* filename);
	~filelocker();

	bool operator!() const
	{
		return !_locked;
	}

	operator bool() const
	{
		return _locked;
	}

private:
	bool lock();
	void unlock();
private:
	bool				_locked;
#if OS_TYPE == OS_WIN32
	HANDLE
#else
	FILE*
#endif
						_fdesc;
	TERIMBER::string_t	_filename;
};

#endif
