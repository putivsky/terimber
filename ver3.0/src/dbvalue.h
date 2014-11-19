/*
 * The Software License
 * ====================================================================
 * Copyright (c) 2003-2009 The Terimber Corporation. All rights
 * reserved.
 * ====================================================================
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE TERIMBER CORPORATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
*/

#ifndef _terimber_dbvalue_h_
#define _terimber_dbvalue_h_

#include "value.h"
#pragma pack(4)

// terimber_db_value is a variant substitute
class terimber_db_value
{
public:
	// value itself
	terimber_xml_value		val;
	// indicator of NULL value - database specific
	bool					nullVal;
};

#pragma pack()

#endif // _terimber_dbvalue_h_

