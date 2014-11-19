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

#ifndef _terimber_vardatabase_factory_h_
#define _terimber_vardatabase_factory_h_

#include "xml/xmlaccss.h"

/*
const char* request_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ENTITY % ddltype \"(CREATE | DROP)\"> \
<!ENTITY % dmltype \"(SELECT | INSERT | UPDATE | DELETE)\"> \
<!ENTITY % logic \"(AND | OR)\"> \
<!ENTITY % condition \"(GT | GE | LT | LE | EQ | NE | PM | FM)\"> \
<!ENTITY % quality \"(low | normal | high)\"> \
<!-- PM partial match, FM - fuzzy match --> \
<!ENTITY % vtype \"(bool | sb1 | ub1 | sb2 | ub2 | sb4 | ub4 | sb8 | ub8 | float | double | numeric | guid | string | binary | mpart | mfuzzy)\"> \
<!ELEMENT request (table | query)>\
<!ELEMENT table (desc)*>\
<!ELEMENT desc EMPTY>\
<!ELEMENT query ((returns?, where) | (returns?, values) | (returns?, values, where) | (returns?, where))> \
<!ELEMENT returns (col)*> \
<!ELEMENT where (group | cond)> \
<!ELEMENT values (col)*> \
<!ELEMENT group (cond)+> \
<!ELEMENT cond EMPTY> \
<!ELEMENT col EMPTY> \
<!ATTLIST request \
timeout CTYPE vt_ub4 \"5000\"> \
<!ATTLIST table \
what %ddltype; #REQUIRED \
name CDATA #REQUIRED> \
<!ATTLIST desc \
name CDATA #REQUIRED \
type %vtype; #REQUIRED> \
<!ATTLIST query \
what %dmltype; #REQUIRED \
name CDATA #REQUIRED> \
<!ATTLIST group \
join %logic; #REQUIRED> \
<!ATTLIST cond \
how %condition; #REQUIRED \
pq %quality; #IMPLIED \
nq %quality; #IMPLIED \
deep CTYPE vt_bool #IMPLIED \
name CDATA #IMPLIED \
val CDATA #REQUIRED > \
<!ATTLIST col \
name CDATA #REQUIRED \
val CDATA #IMPLIED>";

const char* response_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ELEMENT response row*> \
<!ELEMENT row (col)*> \
<!ELEMENT col EMPTY> \
<!ATTLIST response \
errCode CTYPE vt_ub4 #REQUIRED \
errDesc CDATA #IMPLIED> \
<!ATTLIST row \
rowid CTYPE vt_ub4 #REQUIRED> \
<!ATTLIST col \
name CDATA #REQUIRED \
val CDATA #IMPLIED>";
*/
/////////////////////////////////////////////////////////////////////////////
// examples
// 1. Creates table "customers"
// <request timeout="1000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<table what="CREATE" name="customers">
//			<desc name="cid" type="ub4" />
//			<desc name="firstName" type="mpart" />
//			<desc name="lastName" type="mpart" />
//			<desc name="middleName" type="string" />
//			<desc name="salary" type="double" />
//			<desc name="bossid" type="ub4" />
//			<desc name="guid" type="guid" />
//			<desc name="cryptpassword" type="binary" />
//		</table>
// </request>

// <response errCode="-1" errDesc="Table already exists in database" />
// OR
// <response errCode="0" /> <!-- success -->


// 2. Drops table "customers"
// <request timeout="1000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<table what="DROP" name="customers" />
// </request>

// <response errCode="-1" errDesc="Table customers does not exist in database" />
// OR
// <response errCode="0" /> <!-- success -->

// 3. Inserts into table "customers"
// <request timeout="2000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<query what="INSERT" name="customers">
//			<returns /> <!-- no row will be returned, returns is an optional elements, however, if returns is specified without columns row element with rowid attribute will be returned -->
//			<values>
//				<col name="cid" val="17" />	
//				<col name="firstName" val="John" />
//				<col name="lastName" val="Smith" />
//				<!-- if column, middleName for instance is not specified it is considered as NULL value -->
//				<col name="salary" val="128735.78" /> <!-- format numeric is suppose to get decimal point as a delimeter -->
//				<col name="bossid" val="1" />
//				<col name="guid" val="036CDCC332864507847393F151AA3791" /> <!-- just heximal representation of 16 bytes -->
//				<col name="cryptpassword" val="FF3D56D7" /> <!-- just heximal representation bytes (2 chars per byte)
//			</values>
//		</query>
// </request>


// <response errCode="-1" errDesc="Table customers does not exist in database" />
// OR
// <response errCode="0" /> <!-- success -->
// OR
// <response errCode="0"> <!-- success -->
//		<row rowid="1" /> <!-- this is internal rowid identificator -->
// </response>

// 4. Insert into table "customers" with returns
// <request timeout="2000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<query what="INSERT" name="customers">
//			<returns>
//				<col name="cid" />
//				<col name="firstName" />
//			</returns>
//			<values>
//				<col name="cid" val="17" />	
//				<col name="firstName" val="John" />
//				<col name="lastName" val="Smith" />
//				<!-- if column, middleName for instance is not specified it is considered as NULL value -->
//				<col name="salary" val="128735.78" /> <!-- format double, float, and numeric is suppose to get decimal point as a delimeter -->
//				<col name="bossid" val="1" />
//			</values>
//		</query>
// </request>

// <response errCode="-1" errDesc="Table customers does not exist in database" />
// OR
// <response errCode="0"> <!-- success -->
//		<row rowid="1"> <!-- this is internal rowid identificator -->
//			<col name="cid" val="17" />
//			<col name="firstName" val="John" />
//		</row>
// </response>

// 5. Update table "customers"
// <request timeout="2000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<query what="UPDATE" name="customers">
//			<returns /> <!-- no row will be returned, returns is an optional elements -->
//			<where> <!-- where cid=17 or lastName like '%Smith%' -->
//				<group join="OR">
//					<cond how="EQ" name="cid" val="17" /> 
//					<cond how="PM" name="lastName" val="Smith" /> 
//				</group>
//			</where>
//		</query>
// </request>

// <response errCode="-1" errDesc="Table customers does not exist in database" />
// OR
// <response errCode="0"> <!-- success -->
//		<row rowid="1" /> <!-- this is internal rowid identificator -->
// </response>

// 6. Delete from table "customers"
// <request timeout="2000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<query what="DELETE" name="customers">
//			<returns /> <!-- no row will be returned, returns is an optional elements -->
//			<values>
//				<col name="lastName" val="Smith Jr." />
//			</values>
//			<where> <!-- where cid=17 or lastName='Smith' -->
//				<group join="OR">
//					<cond how="EQ" name="cid" val="17" /> 
//					<cond how="EQ" name="lastName" val="Smith" /> 
//				</group>
//			</where>
//		</query>
// </request>

// <response errCode="-1" errDesc="Table customers does not exist in database" />
// OR
// <response errCode="0"> <!-- success -->
//		<row rowid="1" /> <!-- this is internal rowid identificator -->
// </response>

// 7. Select from table "customers"
// <request timeout="2000"> <!-- timeout in milliseconds, default 5 seconds -->
//		<query what="SELECT" name="customers">
//			<returns>
//				<col name="firstName" />
//				<col name="lastName" />
//				<col name="salary" />
//			</returns>
//			<where> <!-- where cid=17 or lastName like '%Smith%' -->
//				<group join="OR">
//					<cond how="EQ" name="cid" val="17" /> 
//					<cond how="PM" name="lastName" val="Smith" /> 
//				</group>
//			</where>
//		</query>
// </request>

// <response errCode="-1" errDesc="Table customers does not exist in database" />
// OR
// <response errCode="0"> <!-- success -->
//		<row rowid="1"> <!-- this is internal rowid identificator -->
//			<col name="firstName" val="John" />
//			<col name="lastName" val="Smith Jr." />
//			<col name="salary" val="128735.78" />
//		</row>
// </response>


/////////////////////////////////////////////////////////////////////////////
//! \class terimber_vardatabase
//! \brief abstract interface for table in memory
class terimber_vardatabase
{
public:
	//! \brief destructor
	virtual 
	~terimber_vardatabase() 
	{
	}

	//! methonds

	//! \brief requests must comply with request_dtd
	//! the result will be inside the parser and will comply with response_dtd
	//! returns false if not enough memory
	virtual 
	bool 
	process_xml_request(const char* request,				//!< db/xml request
					size_t len,								//!< request length
					xml_designer* parser					//!< xml designer
					) = 0;
};

//! \class terimber_vardatabase_factory
//! \brief class factory to create terimber_vardatabase
class terimber_vardatabase_factory 
{
public:
	//! \brief creates empty database
	//! caller is responsible for destroying the object
	terimber_vardatabase* 
	get_vardatabase();
};

#endif
