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
 
#include "crypt/base64.h"
 
BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static const char g_code[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
// the out buffer is supposed to be larger then the in buffer on 1/3
void encode_base64(ub1_t* out, const ub1_t* in, size_t& len)
{
	size_t pos_in = 0; // current position in 
	size_t pos_out = 0; // current position out
	
	while (pos_in + 2 < len)
	{
		// encodes
		out[pos_out] =		g_code[in[pos_in] >> 2];
		out[pos_out + 1] =	g_code[(in[pos_in] & 0x3) << 4 | in[pos_in + 1] >> 4];
		out[pos_out + 2] =	g_code[(in[pos_in + 1] & 0xF) << 2 | in[pos_in + 2] >> 6];
		out[pos_out + 3] =	g_code[in[pos_in + 2] & 0x3F];

		pos_in += 3;
		pos_out += 4;
	}

	if (pos_in < len) // some bytes exceed triplets
	{
		out[pos_out] = g_code[in[pos_in] >> 2];
		if (pos_in + 1 == len)
		{
			out[pos_out + 1] = g_code[(in[pos_in] & 0x3) << 4];
			out[pos_out + 2] = '=';
		}
		else
		{
			out[pos_out + 1] = g_code[(in[pos_in] & 0x3) << 4 | in[pos_in + 1] >> 4];
			out[pos_out + 2] = g_code[(in[pos_in + 1] & 0xF) << 2];
		}

		out[pos_out + 3] = '=';
		pos_out += 4;
	}

	len = pos_out;
}

void decode_base64(ub1_t* inout, size_t& len)
{
	size_t pos_in = 0; // current position in 
	size_t pos_out = 0; // current position out

	while (pos_in < len)
	{
		size_t tail = 0;
		for (size_t index = pos_in; index < pos_in + 4; ++index)
		{
			if (inout[index] >= 'A' && inout[index] <= 'Z')
				inout[index] -= 'A';
			else if (inout[index] >= 'a' && inout[index] <= 'z')
			{
				inout[index] += 26; 
				inout[index] -= 'a';
			}
			else if (inout[index] >= '0' && inout[index] <= '9')
			{
				inout[index] += 52;
				inout[index] -= '0';
			}
			else if (inout[index] == '+') inout[index] = 62;
			else if (inout[index] == '/') inout[index] = 63;
			else if (inout[index] == '=') 
			{
				inout[index] = 0;
				++tail;
			}
		}

		// decode
		inout[pos_out] = inout[pos_in] << 2 | inout[pos_in + 1] >> 4;
		inout[pos_out + 1] = inout[pos_in + 1] << 4 | inout[pos_in + 2] >> 2;
		inout[pos_out + 2] = inout[pos_in + 2] << 6 | inout[pos_in + 3];
		
		pos_in += 4;
		pos_out += 3 - tail;
	}

	len = pos_out;
}

#pragma pack()
END_TERIMBER_NAMESPACE
