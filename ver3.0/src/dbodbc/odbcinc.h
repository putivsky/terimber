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

#ifndef _terimber_odbcinc_h_
#define _terimber_odbcinc_h_

#if OS_TYPE == OS_WIN32

#include <sqlext.h>  // ODBC
#include <odbcss.h>
#else

#define ODBCVER 0x0350
// defines
#define DWORD ub4_t
#define WORD ub2_t
#define BYTE ub1_t
#define LPWSTR wchar_t*
#define VOID void
#define GUID guid_t
#define CHAR char
#define WCHAR wchar_t
#define UINT ub4_t
#define LONG sb4_t
#define LPBYTE BYTE*

#include <sqlext.h>  // ODBC
#include <sqlucode.h>  // ODBC

#define SQL_COPT_SS_BASE				1200
#define SQL_INTEGRATED_SECURITY 		(SQL_COPT_SS_BASE+3) // Force integrated security on login
#define SQL_COPT_SS_CONNECTION_DEAD		(SQL_COPT_SS_BASE+9) // dbdead SQLGetConnectOption only
#define SQL_IS_OFF		0L			//	Integrated security isn't used
#define SQL_IS_ON		1L			//	Integrated security is used

#endif

#endif // _terimber_odbcinc_h_
