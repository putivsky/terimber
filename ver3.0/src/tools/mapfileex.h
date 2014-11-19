#ifndef _mapfileex_h_
#define _mapfileex_h_

#include "threadpool/thread.h"

class writable_filememmapper
{
public:
	writable_filememmapper();
	~writable_filememmapper();

	bool memmapfile(const char* name);
	void memunmapfile();
	void touch();
	void* getaddress() const;

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
