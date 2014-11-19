#include "dirscan.h"
#include "base/string.hpp"
#include "base/memory.hpp"


dirscan::dirscan() :
	_first(false),
	_dirdesc(0),
	_eof(true)
{
}

dirscan::~dirscan()
{
	close();
}

void
dirscan::close()
{
	if (_dirdesc)
	{
#if OS_TYPE == OS_WIN32
		_findclose(_dirdesc);
#else
		closedir(_dirdesc);
#endif
		_dirdesc = 0;
		_eof = true;
	}
}

bool 
dirscan::init(const char* dirname, const char* ext_filter)
{
	close();

	if (!dirname || !*dirname)
		return false;

	_dirname = dirname;
	_extfilter = ext_filter;
	_first = true;
	_eof = false;
	return true;
}

bool
dirscan::next(TERIMBER::string_t& filename)
{
	if (_eof)
		return false;


	bool found = false;
#if OS_TYPE == OS_WIN32
	_finddata_t fileinfo;
	if (_first)
	{
		if (!_dirname.length())
			return false;

		TERIMBER::string_t nameDirMask(_dirname);

		//create the directory name
		if (_extfilter.length())
		{
			nameDirMask += "\\*.";
			nameDirMask += _extfilter;
		}
		else
		{
			nameDirMask += "\\*.*";
		}

		_first = false;

		_dirdesc = _findfirst(nameDirMask, &fileinfo);
		if (-1 == _dirdesc)
		{
			_eof = true;
			_dirdesc = 0;
			return false;
		}
	}
	else
	{
		if (_eof = (0 != _findnext(_dirdesc, &fileinfo)))
			return false;
	}

	do
	{
		if (fileinfo.attrib & (_A_HIDDEN|_A_SUBDIR|_A_SYSTEM))
			continue;

		// make full name
		filename = _dirname;
		filename += "/";
		filename += fileinfo.name;
		found = true;
	}
	while(!found && !(_eof = (0 != _findnext(_dirdesc, &fileinfo))));

#else
	if (_first)
	{
		if (!_dirname.length())
			return false;

		TERIMBER::string_t nameDirMask(_dirname);

		_first = false;

		_dirdesc = opendir(nameDirMask);
		if (!_dirdesc)
		{
			_eof = true;
			return false;
		}
	}

	struct dirent *d;
	while (!found && !(_eof = (0 == (d = readdir(_dirdesc)))))
	{
		// make full name
		filename = _dirname;
		filename += "/";
		filename += d->d_name;
		struct stat st;
		const char* ext = strrchr(d->d_name, '.');
		
		if ((_extfilter.length() && (!ext || TERIMBER::str_template::strnocasecmp(ext + 1, (const char*)_extfilter, os_minus_one)))
			|| -1 == ::stat(filename, &st)
			|| !S_ISREG(st.st_mode)
			)
			continue;

		found = true;
	}
#endif

	return found;
}

