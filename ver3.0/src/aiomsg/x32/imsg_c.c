

/* this ALWAYS GENERATED file contains the RPC client stubs */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Sun Sep 14 22:19:13 2008
 */
/* Compiler settings for imsg.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, oldnames, robust
    error checks: allocation ref bounds_check enum stub_data , use_epv, no_default_epv
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */
#pragma warning( disable: 4211 )  /* redefine extent to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma optimize("", off ) 

#include <string.h>

#include "imsg.h"

#define TYPE_FORMAT_STRING_SIZE   51                                
#define PROC_FORMAT_STRING_SIZE   99                                
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

#define GENERIC_BINDING_TABLE_SIZE   0            


/* Standard interface: IMSG, ver. 1.0,
   GUID={0xDDD05070,0xFC62,0x4c40,{0x9C,0xC3,0x8A,0x5A,0x80,0xCE,0xBB,0xD9}} */



static const RPC_CLIENT_INTERFACE IMSG___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xDDD05070,0xFC62,0x4c40,{0x9C,0xC3,0x8A,0x5A,0x80,0xCE,0xBB,0xD9}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE ClientIMSG_ClientIfHandle = (RPC_IF_HANDLE)& IMSG___RpcClientInterface;

extern const MIDL_STUB_DESC IMSG_StubDesc;

static RPC_BINDING_HANDLE IMSG__MIDL_AutoBindHandle;


void ClientConnect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char *sender,
    /* [out] */ unsigned long *szrecv,
    /* [size_is][size_is][out] */ unsigned char **receiver)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IMSG_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&handle);
    
}


void ClientPostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char *buf,
    /* [string][out][in][unique] */ unsigned char **reply)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IMSG_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[52],
                  ( unsigned char * )&handle);
    
}


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need a Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
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
/*  8 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x8 ),	/* 8 */
/* 16 */	NdrFcShort( 0x1c ),	/* 28 */
/* 18 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x4,		/* 4 */
/* 20 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 22 */	NdrFcShort( 0x1 ),	/* 1 */
/* 24 */	NdrFcShort( 0x1 ),	/* 1 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter handle */

/* 28 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 30 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 32 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter szsend */

/* 34 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Parameter sender */

/* 40 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 42 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 44 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter szrecv */

/* 46 */	NdrFcShort( 0x2013 ),	/* Flags:  must size, must free, out, srv alloc size=8 */
/* 48 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 50 */	NdrFcShort( 0x16 ),	/* Type Offset=22 */

	/* Procedure PostBlock */


	/* Parameter receiver */

/* 52 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 54 */	NdrFcLong( 0x0 ),	/* 0 */
/* 58 */	NdrFcShort( 0x1 ),	/* 1 */
/* 60 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 62 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 64 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 66 */	NdrFcShort( 0x8 ),	/* 8 */
/* 68 */	NdrFcShort( 0x0 ),	/* 0 */
/* 70 */	0x43,		/* Oi2 Flags:  srv must size, clt must size, has ext, */
			0x3,		/* 3 */
/* 72 */	0x8,		/* 8 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 74 */	NdrFcShort( 0x0 ),	/* 0 */
/* 76 */	NdrFcShort( 0x1 ),	/* 1 */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter handle */

/* 80 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 82 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 84 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter sz */

/* 86 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 88 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 90 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */

	/* Parameter buf */

/* 92 */	NdrFcShort( 0x201b ),	/* Flags:  must size, must free, in, out, srv alloc size=8 */
/* 94 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 96 */	NdrFcShort( 0x2a ),	/* Type Offset=42 */

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
/* 12 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
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
/* 36 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
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
    52
    };


static const MIDL_STUB_DESC IMSG_StubDesc = 
    {
    (void *)& IMSG___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &IMSG__MIDL_AutoBindHandle,
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
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

