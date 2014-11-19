#ifndef _dirscan_h_
#define _dirscan_h_

#include "threadpool/thread.h"
#include "base/string.h"

#if OS_TYPE != OS_WIN32h
#include <dirent.h>
#include <sys/stat.h>
#endif

class dirscan
{
public:
	dirscan();
	~dirscan();

	bool 
	init(const char* dirname, const char* ext_filter);

	bool
		next(TERIMBER::string_t& filename);
private:
	void close();
private:
	bool	_first;
	bool	_eof;
#if OS_TYPE == OS_WIN32
	intptr_t
#else
	DIR*
#endif
						_dirdesc;
	TERIMBER::string_t	_dirname;
	TERIMBER::string_t	_extfilter;
};

#endif
