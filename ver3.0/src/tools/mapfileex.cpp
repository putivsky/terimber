#include "mapfileex.h"

#if OS_TYPE != OS_WIN32
	#include <sys/mman.h>
#endif


writable_filememmapper::writable_filememmapper() :
	_fdesc(0),
	_mapinfo(0),
	_addr(0),
	_size(0)
{
}

writable_filememmapper::~writable_filememmapper()
{
	memunmapfile();
}

bool 
writable_filememmapper::memmapfile(const char* name)
{
	memunmapfile();

	if (!name)
		return false;

#if OS_TYPE == OS_WIN32
	if (INVALID_HANDLE_VALUE == (_fdesc = ::CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0))
		|| !(_mapinfo = ::CreateFileMapping(_fdesc, 0, PAGE_READWRITE, 0, 0, 0))
		|| !(_addr = ::MapViewOfFile(_mapinfo, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0))
		) 
	{
		memunmapfile();
		return false;
	}

	_size = ::GetFileSize(_fdesc, 0);

#else
	int ret;
	struct stat sb;

	if (-1 == (_fdesc = open (name, O_RDWR, 0))
		|| -1 == (ret = fstat(_fdesc, &sb))
		|| !(_addr = mmap(0, (int)(_mapinfo = (void*)sb.st_size), PROT_READ | PROT_WRITE, MAP_SHARED, _fdesc, 0))
		) 
	{
		memunmapfile();
		return false;
	}

	_size = sb.st_size;
#endif

	return true;
}

void 
writable_filememmapper::memunmapfile()
{
#if OS_TYPE == OS_WIN32
	if (_addr)
		::UnmapViewOfFile(_addr), _addr = 0;
	if (_mapinfo)
		::CloseHandle(_mapinfo), _mapinfo = 0;
	if (_fdesc)
		::CloseHandle(_fdesc), _fdesc = 0;
#else
	if (_addr)
		munmap(_addr, (int)_mapinfo), _addr = 0, _mapinfo = 0;
	if (_fdesc)
		close(_fdesc), _fdesc = 0;
#endif

	_size = 0;
}

void* 
writable_filememmapper::getaddress() const
{
	return _addr;
}

void 
writable_filememmapper::touch()
{
	unsigned int page = getpagesize();

	if (page && _size && _size != 0xFFFFFFFF)
	{
		// Calculate the number of pages to touch
		unsigned int pages = _size / page;
		if (_size % page > 0)
		{
			++pages;
		}

		// Read the first byte of each page
		for (unsigned int n = 0; n < pages; ++n)
		{
			// Read a byte from the file to force a file map to
			// be loaded into memory.  We use "volatile" to make
			// sure that the compiler does not optimize this line out.

			volatile unsigned char dummy = *(((unsigned char*)_addr) + n * page);
		}
	}
}
// static
unsigned int
writable_filememmapper::getpagesize() 
{
#if OS_TYPE == OS_WIN32
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	return sys_info.dwPageSize;
#else /* !WIN32 */
	return ::getpagesize();
#endif /* !WIN32 */
}
