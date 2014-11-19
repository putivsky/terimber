#include "mapfile.h"

#if OS_TYPE != OS_WIN32
	#include <sys/mman.h>
#endif


filememmapper::filememmapper() :
	_fdesc(0),
	_mapinfo(0),
	_addr(0),
	_size(0)
{
}

filememmapper::~filememmapper()
{
	memunmapfile();
}

bool 
filememmapper::memmapfile(const char* name)
{
	memunmapfile();

	if (!name)
		return false;

#if OS_TYPE == OS_WIN32
	if (INVALID_HANDLE_VALUE == (_fdesc = ::CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0))
		|| !(_mapinfo = ::CreateFileMapping(_fdesc, 0, PAGE_READONLY, 0, 0, 0))
		|| !(_addr = ::MapViewOfFile(_mapinfo, FILE_MAP_READ, 0, 0, 0))
		) 
	{
		memunmapfile();
		return false;
	}

	_size = ::GetFileSize(_fdesc, 0);

#else
	int ret;
	struct stat sb;

	if (-1 == (_fdesc = open (name, O_RDONLY, 0))
		|| -1 == (ret = fstat(_fdesc, &sb))
		|| !(_addr = mmap(0, (int)(_mapinfo = (void*)sb.st_size), PROT_READ, MAP_SHARED, _fdesc, 0))
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
filememmapper::memunmapfile()
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
filememmapper::getaddress() const
{
	return _addr;
}

size_t 
filememmapper::getfilesize() const
{
	return _size;
}

void 
filememmapper::touch()
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
filememmapper::getpagesize() 
{
#if OS_TYPE == OS_WIN32
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	return sys_info.dwPageSize;
#else /* !WIN32 */
	return ::getpagesize();
#endif /* !WIN32 */
}

