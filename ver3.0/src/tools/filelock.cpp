#include "filelock.h"
#include "base/string.hpp"
#include "base/memory.hpp"

#if OS_TYPE != OS_WIN32
#include <sys/file.h>
#endif


filelocker::filelocker(const char* filename) :
	_locked(false),
	_fdesc(0)
{
	// construct full file name
	if (filename)
	{
		char path[1024];

#if OS_TYPE == OS_WIN32
		if (sizeof(path) < ::GetTempPath(sizeof(path), path))
		{
			assert(false);
			return;
		}

		_filename = path;
#else
		_filename = "/usr/tmp";
#endif
		_filename += "/";
		_filename += filename;
	
		lock();
	}
}

filelocker::~filelocker()
{
	unlock();
}

bool 
filelocker::lock()
{
	unlock();
	if (!_filename.length())
		return false;

#if OS_TYPE == OS_WIN32
	// open file
	if (INVALID_HANDLE_VALUE == (_fdesc = ::CreateFile((const char*)_filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0))) 
	{
		unlock();
		return false;
	}

	// check the file size
	DWORD magic = 0x12345678;
	DWORD processed = 0;
	DWORD len = GetFileSize(_fdesc, &processed);
	if (!len && !::WriteFile(_fdesc, &magic, sizeof(magic), &processed, 0))
	{
		unlock();
		return false;
	}

	if (!::LockFile(_fdesc, 0, 0, sizeof(magic), 0))
	{
		unlock();
		return false;
	}

	_locked = true;

#else
	if (!(_fdesc = ::fopen((const char*)_filename, "ab")))
	{
		unlock();
		//printf("can not open file: %s\n", (const char*)_filename);
		return false;
	}

	// check the file size
	struct stat sb;

	if (-1 == ::fstat(::fileno(_fdesc), &sb))
	{
		unlock();
		//printf("can not get stats for file: %s\n", (const char*)_filename);
		return false;
	}

	size_t magic = 0x12345678;

	if (!sb.st_size && 1 != ::fwrite(&magic, sizeof(magic), 1, _fdesc))
	{
		unlock();
		//printf("can not write to file: %s\n", (const char*)_filename);
		return false;
	}

	_locked = (0 == ::flock(::fileno(_fdesc), LOCK_EX | LOCK_NB));
	
	//if (!_locked) printf("can not lock file: %s\n", (const char*)_filename);

#endif

	return _locked;
}

void 
filelocker::unlock()
{
#if OS_TYPE == OS_WIN32
	if (_fdesc && INVALID_HANDLE_VALUE != _fdesc)
	{
		if (_locked)
			::UnlockFile(_fdesc, 0, 0, 16, 0);
		::CloseHandle(_fdesc);
	}
#else
	if (_fdesc)
	{
		if (_locked)
			::flock(::fileno(_fdesc), LOCK_UN);
		::fclose(_fdesc);
	}
#endif

	_fdesc = 0;
}

