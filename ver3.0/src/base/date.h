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

#ifndef _terimber_date_h_
#define _terimber_date_h_

#include "allinc.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class date
//! \brief datetime up to milliseconds
class date
{
public:
	//! \brief default constructor
	date(bool utc = false);
	//! \brief constructor scans narrow string format
	date(			const char* x,						//!< input
					size_t len = os_minus_one				//!< input length
					);
	//! \brief constructor scans wide string format
	date(			const wchar_t* x,						//!< input 
					size_t len = os_minus_one				//!< input length
					);
	//! \brief constructor from pointer to double
	date(			const double* x						//!< input
					);
	//! \brief constructor from double
	date(			double x								//!< input
					);
	//! \brief constructor from ticks in milliseconds from 1 A.D.
	date(			sb8_t x								//!< input
					);
	//! \brief converts to double
	operator double() const;
	//! \brief converts to ticks in milliseconds from 1 A.D.
	operator sb8_t() const;

	//! \brief converts from struct members to ticks
	static 
	bool 
	convert_to(		ub4_t year,								//!< year [0 : 1000000]
					ub1_t month,							//!< month [1-12]
					ub1_t day,								//!< day [1-31]
					ub1_t hour,								//!< hour [0-23]
					ub1_t minute,							//!< minute [0-59]
					ub1_t second,							//!< second [0-59]
					ub2_t millisec,							//!< millisecond [0-999]
					sb8_t& dt								//!< [out] ticks in miiliseconds since 1A.D.
					);
	//! \brief converts from ticks to struct members
	static 
	bool 
	convert_from(	sb8_t dt,								//!< ticks ticks in miiliseconds since 1A.D.
					ub4_t& year,							//!< [out] year [0 : 1000000]
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisecond [0-999]
					ub1_t& wday,							//!< [out] day of week [1-7]
					ub2_t& yday								//!< [out] day of year [1-366]
					);
	//! \brief less operator
	bool 
	operator<(const date& x) const;
	//! \brief eqaul operator
	bool 
	operator==(const date& x) const;
	//! \brief non equal operator
	bool 
	operator!=(const date& x) const;
	//! \brief less or equal operator
	bool 
	operator<=(const date& x) const;
	//! \brief greater operator
	bool 
	operator>(const date& x) const;
	//! \brief greater or equal operator
	bool 
	operator>=(const date& x) const;
	//! \brief checks if current time exceed the specified one
	bool 
	is_time_over(	sb8_t x								//!< input time
					) const;
	//! \brief gets the difference between current time and the specified one
	static 
	sb8_t 
	get_difference(	sb8_t old_date							//!< input time
					);
	//! \brief plus assign operator	
	date& operator+=(sb8_t interval);
	//! \brief minus assign operator
	date& operator-=(sb8_t interval);
	//! \brief narrow string format data
	const char* 
	get_date(		char* buffer							//!< [out] preallocated buffer
					) const;
	//! \brief wide string format data
	const wchar_t* 
	get_date(		wchar_t* buffer							//!< [out] preallocated buffer
					) const;

	//! \brief narrow string format data
	const char* 
	get_date_only(	char* buffer,							//!< [out] preallocated buffer
					const char* format						//!< format for 3 integers, year, month, day like "%d-%02d-%02d"
					) const;
	//! \brief wide string format data
	const wchar_t* 
	get_date_only(	wchar_t* buffer,						//!< [out] preallocated buffer
					const wchar_t* format					//!< format for 3 integers, year, month, day like "%d-%02d-%02d"
					) const;

private:
	//! \brief sets the date from formatted narrow string "%d-%02d-%02d %02d-%02d-%02d.%04d"
	void 
	set_date(		const char* x							//!< input
					);
	//! \brief sets the date from formatted wide string "%d-%02d-%02d %02d-%02d-%02d.%04d"
	void 
	set_date(		const wchar_t* x						//!< input
					);

	//! \brief scans buffer of narrow chars moving pointer on count chars forward
	static 
	bool 
	scan_buffer(	const char*& x,						//!< input narrow string
					size_t count,							//!< chars for scan
					size_t delimeter_size,					//!< delimeter size
					sb4_t& dest								//!< [out] signed integer
					);

	//! \brief scans buffer of narrow chars moving pointer on count chars forward
	static 
	bool 
	scan_buffer(	const wchar_t*& x,					//!< input wide string
					size_t count,							//!< chars for scan
					size_t delimeter_size,					//!< delimeter size
					sb4_t& dest								//!< [out] signed integer
					);
private:
	//! \brief sets date from double
	void 
	set_double(		double x								//!< input
					);
private:
	sb8_t _ticks;											//!< ticks in millisecond after 1 January 0000, midnight (negative value means before)
	static ub2_t _month_days[13];							//!< static array of days in month
	static const sb8_t _70years;							//!< days in 70 years 
	static const sb8_t _1899years;							//!< days from 0 year A.D. to 12 December 1899
	static const sb8_t _daymillisecond;						//!< millisecond in one day
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_date_h_

