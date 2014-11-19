/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2010 The Terimber Corporation. All rights reserved.
 * =================================================================================
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * The end-user documentation included with the redistribution, if any, 
 * must include the following acknowledgment:
 * "This product includes software developed by the Terimber Corporation."
 * =================================================================================
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL THE TERIMBER CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ================================================================================
*/

#include "base/date.h"
#include "base/string.hpp"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////////////////////////
// date 
// static
const sb8_t date::_70years = 25569;
//static 
const sb8_t date::_1899years = 693959;
// static
const sb8_t date::_daymillisecond = 86400000;

// static 
ub2_t date::_month_days[13] =
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

date::date(bool utc)
{
	timeb cur_time;
	ftime(&cur_time);

	// UTC requested
	if (utc)
	{
		cur_time.timezone = 0;
		cur_time.dstflag = 0;
	}
		

	// seconds since January 1, 1970 midnight 
	_ticks = (_1899years + _70years) * _daymillisecond +
			((sb8_t)cur_time.dstflag * 60 * 60 + (sb8_t)cur_time.time - cur_time.timezone * 60) * 1000 + 
			(sb8_t)cur_time.millitm;
}

date::date(const char* x, size_t len) :
	_ticks(0)
{
	if (!x)
		return;
	
	len = (len == os_minus_one) ? str_template::strlen(x) : len;
	char buf[64] = {0};
	str_template::strcpy(buf, x, __min(len, sizeof(buf) - 1));
	set_date(buf);
}

date::date(const wchar_t* x, size_t len) :
	_ticks(0)
{
	if (!x)
		return;
	
	len = (len == os_minus_one) ? str_template::strlen(x) : len;
	wchar_t buf[64] = {0};
	str_template::strcpy(buf, x, __min(len, sizeof(buf) - 1));
	set_date(buf);
}

date::date(sb8_t x) :
	_ticks(x)
{
	_ticks = x;
}

date::date(const double* x) :
	_ticks(0)
{
	if (x)
		set_double(*x);
}

date::date(double x) :
	_ticks(0)
{
	set_double(x);
}

date::operator double() const
{
	sb8_t date_part = _ticks / _daymillisecond;
	sb8_t time_part = _ticks % _daymillisecond;

	return (double)(date_part - _1899years) + (double)(time_part) / _daymillisecond;
}

void 
date::set_double(double x)
{
	double date_part = floor(x);
	_ticks = ((sb8_t)date_part + _1899years) * _daymillisecond;
	_ticks += (sb8_t)((x - date_part) * _daymillisecond);
}

date::operator sb8_t() const
{
	return _ticks;
}

bool 
date::operator<(const date& x) const
{
	return _ticks < x._ticks;
}

bool 
date::operator!=(const date& x) const
{
	return _ticks != x._ticks;
}

bool 
date::operator==(const date& x) const
{
	return _ticks == x._ticks;
}

bool 
date::operator<=(const date& x) const
{
	return _ticks <= x._ticks;
}

bool 
date::operator>(const date& x) const
{
	return _ticks > x._ticks;
}

bool 
date::operator>=(const date& x) const
{
	return _ticks >= x._ticks;
}

bool 
date::is_time_over(sb8_t x) const
{
	return get_difference(_ticks) >= x;
}

// in second
// static
sb8_t 
date::get_difference(sb8_t old_date)
{
	return (sb8_t)date() - old_date;
}

date& 
date::operator+=(sb8_t interval)
{
	_ticks += interval;
	return *this;
}

date& 
date::operator-=(sb8_t interval)
{
	_ticks -= interval;
	return *this;
}

// static 
bool 
date::convert_to(	ub4_t year, 
					ub1_t month, 
					ub1_t day, 
					ub1_t hour, 
					ub1_t minute, 
					ub1_t second, 
					ub2_t millisec,
					sb8_t& dt)
{
	// validates input date
	if (year > 1000000 
		||	month < 1 || month > 12 
		||	day < 1 || day > 31
		||	hour > 23
		||	minute > 59
		||	second > 59
		||	millisec > 999 )
		return false;

	//  checks for leap year
	bool leap_year = ((year & 3) == 0) && ((year % 100) != 0 || (year % 400) == 0);

	// sets the number of days in the month
	ub2_t days_in_month =
		_month_days[month] - _month_days[month - 1] +
		((leap_year && day == 29 && month == 2) ? 1 : 0);

	// checks the day in the month
	if (day > days_in_month)
		return false;

	// splits the date in days and time
	sb8_t date_part;
	sb8_t time_part;

	// January 1, 1AD be 1st day
	// calculates the number of days
	date_part = year * 365 + year/4 - year/100 + year/400 +
		_month_days[month - 1] + day;

	//  if leap year and it's before March, subtracts 1:
	if (month <= 2 && leap_year)
		--date_part;

	// calculates time part
	time_part = ((hour * 60 + minute) * 60 + second) * 1000 + millisec;

	// combines all parts together
	dt = date_part * _daymillisecond + time_part;
	return true;
}

// static 
bool 
date::convert_from(sb8_t dt, 
					ub4_t& year, 
					ub1_t& month, 
					ub1_t& day, 
					ub1_t& hour, 
					ub1_t& minute, 
					ub1_t& second,
					ub2_t& millisec,
					ub1_t& wday, 
					ub2_t& yday)
{
	sb8_t date_part = dt / _daymillisecond;
	sb8_t time_part = dt % _daymillisecond;
	
	// calculates the day of week (sun=1, mon=2...)
	//  -1 because January 1, 1 A.D. is Monday.  +1 because we want 1-based
	wday = (ub1_t)((date_part - 1) % 7) + 1;
	
	// Leap years every 4 yrs except centuries not multiples of 100.
	sb8_t years400 = date_part / 146097; // Number of 400 year increments since 1/1/0

	// sets date_part to day within 400-year block
	date_part %= 146097;

	// centuries in 400 years block
	// -1 because first century has extra day
	sb8_t years100 = (date_part - 1) / 36524;

	// 4 years blocks in 400 years block 
	sb8_t years4 = date_part / 1461;
	sb8_t day4 = date_part % 1461;
	bool leap_year = true;
	
	// non-leap century
	if (years100 != 0)
	{
		// sets date_part to day within century
		date_part = (date_part - 1) % 36524;

		// +1 because 1st 4 year increment has 1460 days
		years4 = (date_part + 1) / 1461;

		if (years4 != 0)
			day4 = (date_part + 1) % 1461;
		else
		{
			leap_year = false;
			day4 = date_part;
		}
	}

	// 0-3 additional years
	sb8_t yr;

	if (leap_year)
	{
		// -1 because one of the year has 366 days
		yr = (day4 - 1) / 365;

		if (yr)
			day4 = (day4 - 1) % 365;
	}
	else
	{
		yr = day4 / 365;
		day4 %= 365;
	}

	// day4 is now 0-based day of year
	// saves 1-based day of year, year number
	yday = (ub2_t)day4 + 1;
	year = (ub4_t)(years400 * 400 + years100 * 100 + years4 * 4 + yr);

	// Handles leap year: before, on, and after Feb. 29.
	if (!yr && leap_year && day4 == 59)
	{
		// Feb. 29
		month = 2;
		day = 29;
	}
	else
	{
		// makes day4 a 1-based day of non-leap year and compute
		// month/day for everything but Feb. 29.
		if (yr || !leap_year || day4 < 60)
			++day4;

		// month number always >= n/32, so save some loop time
		for (month = ((ub1_t)day4 >> 5) + 1; (ub2_t)day4 > _month_days[month] && month <= 12; ++month);

		day = (ub1_t)(day4 - _month_days[month - 1]);
	}

	if (!time_part)
	{
		hour = minute = second = 0;
		millisec = 0;
	}
	else
	{
		millisec = (ub2_t)(time_part % 1000);
		time_part /= 1000;
		second = (ub1_t)(time_part % 60);
		time_part /= 60;
		minute = (ub1_t)(time_part % 60);
		hour = (ub1_t)(time_part / 60);
	}

	return true;
}

//////////////////////////////////////////
const char* 
date::get_date(char* buffer) const
{
	ub4_t year = 1900;
	ub1_t month = 1, day = 1, hour = 0, minute = 0, second = 0, wday = 0;
	ub2_t millisec = 0, yday = 0;

	convert_from(_ticks, year, month, day, hour, minute, second, millisec, wday, yday);
	str_template::strprint(buffer, 64, "%d-%02d-%02d %02d:%02d:%02d.%03d", 
			year, month, day, hour, minute, second, millisec);

	return buffer;
}

const wchar_t* 
date::get_date(wchar_t* buffer) const
{
	ub4_t year = 1900;
	ub1_t month = 1, day = 1, hour = 0, minute = 0, second = 0, wday = 0;
	ub2_t millisec = 0, yday = 0;

	wchar_t buf[64] = {0};
        
	convert_from(_ticks, year, month, day, hour, minute, second, millisec, wday, yday);
	str_template::strprint(buffer, 64, str_template::strformat(buf, "%d-%02d-%02d %02d:%02d:%02d.%03d"), 
			year, month, day, hour, minute, second, millisec);

	return buffer;
}


const char* 
date::get_date_only(char* buffer, const char* format) const
{
	ub4_t year = 1900;
	ub1_t month = 1, day = 1, hour = 0, minute = 0, second = 0, wday = 0;
	ub2_t millisec = 0, yday = 0;

	convert_from(_ticks, year, month, day, hour, minute, second, millisec, wday, yday);
	str_template::strprint(buffer, 64, format ? format : "%d-%02d-%02d", 
			year, month, day);

	return buffer;
}

const wchar_t* 
date::get_date_only(wchar_t* buffer, const wchar_t* format) const
{
	ub4_t year = 1900;
	ub1_t month = 1, day = 1, hour = 0, minute = 0, second = 0, wday = 0;
	ub2_t millisec = 0, yday = 0;

	wchar_t buf[64] = {0};
	
	convert_from(_ticks, year, month, day, hour, minute, second, millisec, wday, yday);
	str_template::strprint(buffer, 64, format ? format : str_template::strformat(buf, "%d-%02d-%02d"), 
			year, month, day);

	return buffer;
}

// static 
bool 
date::scan_buffer(const char*& x, size_t count, size_t delimeter_size, sb4_t& dest)
{
	if (str_template::strscan(x, count, "%d", &dest))
	{
		x += count + delimeter_size;
		return true;
	}
	else
		return false;
}

// static 
bool 
date::scan_buffer(const wchar_t*& x, size_t count, size_t delimeter_size, sb4_t& dest)
{
	if (str_template::strscan(x, count, L"%d", &dest))
	{
		x += count + delimeter_size;
		return true;
	}
	else
		return false;
}

void 
date::set_date(const char* x)
{
	const char* ptr = x;
	sb4_t year = 1900, month = 1, day = 1, hour = 0, minute = 0, second = 0, millisec = 0;

	if (x && x[0])
	{
		// gets year, etc
		if (scan_buffer(ptr, 4, 1, year)
			&& scan_buffer(ptr, 2, 1, month)
			&& scan_buffer(ptr, 2, 1, day)
			&& scan_buffer(ptr, 2, 1, hour)
			&& scan_buffer(ptr, 2, 1, minute)
			&& scan_buffer(ptr, 2, 1, second)
			&& scan_buffer(ptr, 3, 1, millisec))
		{
			;
		}
		convert_to(year, month, day, hour, minute, second, millisec, _ticks);
	}
	else
		_ticks = 0;
}

void 
date::set_date(const wchar_t* x)
{
	const wchar_t* ptr = x;
	sb4_t year = 1900, month = 1, day = 1, hour = 0, minute = 0, second = 0, millisec = 0;

	if (x && x[0])
	{
		// gets year, etc
		if (scan_buffer(ptr, 4, 1, year)
			&& scan_buffer(ptr, 2, 1, month)
			&& scan_buffer(ptr, 2, 1, day)
			&& scan_buffer(ptr, 2, 1, hour)
			&& scan_buffer(ptr, 2, 1, minute)
			&& scan_buffer(ptr, 2, 1, second)
			&& scan_buffer(ptr, 3, 1, millisec))
		{
			;
		}
		convert_to(year, month, day, hour, minute, second, millisec, _ticks);
	}
	else
		_ticks = 0;
}

#pragma pack()
END_TERIMBER_NAMESPACE

