

/* this ALWAYS GENERATED file contains the RPC server stubs */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Sun Sep 14 22:19:31 2008
 */
/* Compiler settings for imsg.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
    protocol : dce , ms_ext, c_ext, oldnames, robust
    error checks: allocation ref bounds_check enum stub_data , use_epv, no_default_epv
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if defined(_M_IA64) 


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extent to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#include <string.h>
#include "imsg.h"

#define TYPE_FORMAT_STRING_SIZE   51                                
#define PROC_FORMAT_STRING_SIZE   103                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};

extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: IMSG, ver. 1.0,
   GUID={0xDDD05070,0xFC62,0x4c40,{0x9C,0xC3,0x8A,0x5A,0x80,0xCE,0xBB,0xD9}} */


extern const MIDL_SERVER_INFO IMSG_ServerInfo;

extern RPC_DISPATCH_TABLE IMSG_DispatchTable;

static const RPC_SERVER_INTERFACE IMSG___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xDDD05070,0xFC62,0x4c40,{0x9C,0xC3,0x8A,0x5A,0x80,0xCE,0xBB,0xD9}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &IMSG_DispatchTable,
    0,
    0,
    (IMSG_SERVER_EPV *) 0xffffffffffffffff,
    &IMSG_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE ServerIMSG_ServerIfHandle = (RPC_IF_HANDLE)& IMSG___RpcServerInterface;

extern const MIDL_STUB_DESC IMSG_StubDesc;


#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure Connect */

			0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x28 ),	/* ia64 Stack size/offset = 40 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* ia64 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x8 ),	/* 8 */
/* 16 */	NdrFcShort( 0x1c ),	/* 28 */
/* 18 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x4,		/* 4 */
/* 20 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 22 */	NdrFcShort( 0x1 ),	/* 1 */
/* 24 */	NdrFcShort( 0x1 ),	/* 1 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */
/* 28 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter handle */

/* 30 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 32 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 34 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter szsend */

/* 36 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 38 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 40 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Parameter sender */

/* 42 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 44 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 46 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter szrecv */

/* 48 */	NdrFcShort( 0x2013 ),	/* Flags:  must size, must free, out, srv alloc size=8 */
/* 50 */	NdrFcShort( 0x20 ),	/* ia64 Stack size/offset = 32 */
/* 52 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Procedure PostBlock */


	/* Parameter receiver */

/* 54 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 56 */	NdrFcLong( 0x0 ),	/* 0 */
/* 60 */	NdrFcShort( 0x1 ),	/* 1 */
/* 62 */	NdrFcShort( 0x20 ),	/* ia64 Stack size/offset = 32 */
/* 64 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 66 */	NdrFcShort( 0x0 ),	/* ia64 Stack size/offset = 0 */
/* 68 */	NdrFcShort( 0x8 ),	/* 8 */
/* 70 */	NdrFcShort( 0x0 ),	/* 0 */
/* 72 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x3,		/* 3 */
/* 74 */	0xa,		/* 10 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 76 */	NdrFcShort( 0x0 ),	/* 0 */
/* 78 */	NdrFcShort( 0x1 ),	/* 1 */
/* 80 */	NdrFcShort( 0x0 ),	/* 0 */
/* 82 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter handle */

/* 84 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 86 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 88 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter sz */

/* 90 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 92 */	NdrFcShort( 0x10 ),	/* ia64 Stack size/offset = 16 */
/* 94 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Parameter buf */

/* 96 */	NdrFcShort( 0x201b ),	/* Flags:  must size, must free, in, out, srv alloc size=8 */
/* 98 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 100 */	NdrFcShort( 0x2a ),	/* Type Offset=42 */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x0,	/* FC_RP */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/*  8 */	NdrFcShort( 0x1 ),	/* 1 */
/* 10 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0x8 ),	/* ia64 Stack size/offset = 8 */
/* 14 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 16 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 18 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 20 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 22 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] [pointer_deref] */
/* 24 */	NdrFcShort( 0x2 ),	/* Offset= 2 (26) */
/* 26 */	
			0x12, 0x0,	/* FC_UP */
/* 28 */	NdrFcShort( 0x2 ),	/* Offset= 2 (30) */
/* 30 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 32 */	NdrFcShort( 0x1 ),	/* 1 */
/* 34 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x54,		/* FC_DEREFERENCE */
/* 36 */	NdrFcShort( 0x18 ),	/* ia64 Stack size/offset = 24 */
/* 38 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 40 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 42 */	
			0x12, 0x14,	/* FC_UP [alloced_on_stack] [pointer_deref] */
/* 44 */	NdrFcShort( 0x2 ),	/* Offset= 2 (46) */
/* 46 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 48 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const unsigned short IMSG_FormatStringOffsetTable[] =
    {
    0,
    54
    };


static const MIDL_STUB_DESC IMSG_StubDesc = 
    {
    (void *)& IMSG___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x600016e, /* MIDL Version 6.0.366 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION IMSG_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    0
    };
RPC_DISPATCH_TABLE IMSG_DispatchTable = 
    {
    2,
    IMSG_table
    };

static const SERVER_ROUTINE IMSG_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)ServerConnect,
    (SERVER_ROUTINE)ServerPostBlock
    };

static const MIDL_SERVER_INFO IMSG_ServerInfo = 
    {
    &IMSG_StubDesc,
    IMSG_ServerRoutineTable,
    __MIDL_ProcFormatString.Format,
    IMSG_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_IA64) */

