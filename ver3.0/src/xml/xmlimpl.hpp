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

#ifndef _terimber_xmlimpl_hpp_
#define _terimber_xmlimpl_hpp_

#include "xml/xmlimpl.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline 
xmlNodeType 
xml_designer_impl::get_cur_type() const 
{ 
	return _cur_node->_decl->get_type(); 
}

inline 
bool 
xml_designer_impl::is_attribute() const 
{ 
	switch (get_cur_type()) 
	{ 
		case ATTRIBUTE_NODE:
			return true;
		default:
			return false;
	}
}

inline 
bool 
xml_designer_impl::is_element() const 
{ 
	switch (get_cur_type()) 
	{ 
		case ELEMENT_NODE:
			return true;
		default:
			return false;
	}
}

inline 
bool 
xml_designer_impl::is_container() const 
{ 
	switch (get_cur_type()) 
	{ 
		case ELEMENT_NODE:
		case DOCUMENT_NODE:
			return true;
		default:
			return false;
	}
}


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_xmlimpl_hpp_


