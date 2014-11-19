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

#ifndef _terimber_osdef_h_
#define _terimber_osdef_h_

//typedef unsigned short wchar_t;
#include "osdetect.h"

#ifdef OS_64BIT
typedef long unsigned int size_t;
#else
typedef unsigned int size_t;
#endif

const size_t os_def_size = 4096;
const size_t os_minus_one = ~(size_t)0;
#define INFINITE os_minus_one
//#define INFINITE64 0xffffffffffffffff
#define INVALID_SOCKET os_minus_one
#define ERROR_INVALID_HANDLE os_minus_one
#define WAIT_OBJECT_0 0

#define WSABASEERR		10000
#define WSAEFAULT		(WSABASEERR+14)

#define WSAENOTSOCK		(WSABASEERR+38)
#define WSAETIMEDOUT	(WSABASEERR+60)
#define WAIT_FAILED     os_minus_one
#define WAIT_TIMEOUT	0x00000102L
#define SOCKET_ERROR    os_minus_one
#define WSAECONNRESET	(WSABASEERR+54)
#define WSAEMSGSIZE		(WSABASEERR+40)
#define _MAX_PATH       260
#define _O_RDONLY       O_RDONLY
#define _O_BINARY       0x0L
#define _S_IREAD        S_IREAD
#define _O_WRONLY       O_WRONLY
#define _O_CREAT        O_CREAT
#define _O_TRUNC        O_TRUNC
#define _S_IWRITE       S_IWRITE         

#define ERROR_CAN_NOT_COMPLETE 1003L

#define _open open
#define _close close
#define _read read
#define _write write
#define _lrotl _rotl
#define _lrotr _rotr

template < class T > inline T __min(T x, T y) { return x < y ? x : y; }
template < class T > inline T __max(T x, T y) { return x > y ? x : y; }

#define INADDR_NONE 0xffffffff

// depending on compiler let define typename
#define TYPENAME typename

#define I64u "%llu"
#define I64d "%lld"
#define I64x "%llx"
#define I64X "%llX"

#endif // _terimber_osdef_h_
 
