/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2009 The Terimber Corporation. All rights reserved.
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

#ifndef _terimber_value_h_
#define _terimber_value_h_

#include "ostypes.h"
#include "osdetect.h"

#pragma pack(4)

typedef union _terimber_xml_value_
{
	bool					boolVal;
	sb1_t					cVal;
	ub1_t					bVal; 
	sb2_t					iVal;
	ub2_t					uiVal;
	sb4_t					lVal;
	ub4_t					ulVal;
	float					fltVal;
#ifdef OS_64BIT
	double					dblVal;
	sb8_t					intVal; // int64 & data in millisecond from 1 A.D. (negative date is B.C.)
	ub8_t					uintVal;
#else
	const double*			dblVal;
	const sb8_t*			intVal; // int64 & data in millisecond from 1 A.D. (negative date is B.C.)
	const ub8_t*			uintVal;
#endif
	const long double*		ldblVal; // reserved
	const guid_t*			guidVal;
	const char*				strVal; // narrow string
	const wchar_t*			wstrVal;
	const ub1_t*			bufVal; // binary data (first size_t bytes contain the length) 
									// decimal & numeric values - special internal format close to oracle NUMERIC
									// use numeric class (parse_oracle, persist_oracle functions to convert from/to string representation
} terimber_xml_value;

#pragma pack()

#endif // _terimber_value_h_

