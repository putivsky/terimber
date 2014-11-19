#ifndef _mapfile_h_
#define _mapfile_h_

#include "threadpool/thread.h"

class filememmapper
{
public:
	filememmapper();
	~filememmapper();

	bool memmapfile(const char* name);
	void memunmapfile();
	void touch();
	void* getaddress() const;

	size_t getfilesize() const;

	static unsigned int getpagesize();


private:
#if OS_TYPE == OS_WIN32
	HANDLE
#else
	int
#endif
			_fdesc;
	void*	_mapinfo;
	void*	_addr;
	size_t	_size;
};

#endif
