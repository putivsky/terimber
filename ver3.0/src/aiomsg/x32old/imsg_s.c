/* this ALWAYS GENERATED file contains the RPC server stubs */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Apr 15 22:22:54 2009
 */
/* Compiler settings for imsg.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref bounds_check enum stub_data , use_epv, no_default_epv
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#include "imsg.h"

#define TYPE_FORMAT_STRING_SIZE   47                                
#define PROC_FORMAT_STRING_SIZE   33                                

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

extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: IMSG, ver. 1.0,
   GUID={0xDDD05070,0xFC62,0x4c40,{0x9C,0xC3,0x8A,0x5A,0x80,0xCE,0xBB,0xD9}} */


extern RPC_DISPATCH_TABLE IMSG_DispatchTable;

static const RPC_SERVER_INTERFACE IMSG___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xDDD05070,0xFC62,0x4c40,{0x9C,0xC3,0x8A,0x5A,0x80,0xCE,0xBB,0xD9}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &IMSG_DispatchTable,
    0,
    0,
    (IMSG_SERVER_EPV *) 0xffffffff,
    0,
    0
    };
RPC_IF_HANDLE ServerIMSG_ServerIfHandle = (RPC_IF_HANDLE)& IMSG___RpcServerInterface;

extern const MIDL_STUB_DESC IMSG_StubDesc;

void __RPC_STUB
IMSG_Connect(
    PRPC_MESSAGE _pRpcMessage )
{
    unsigned long _M5;
    unsigned char __RPC_FAR *_M6;
    MIDL_STUB_MESSAGE _StubMsg;
    handle_t handle;
    unsigned char __RPC_FAR *__RPC_FAR *receiver;
    unsigned char __RPC_FAR *sender;
    unsigned long __RPC_FAR *szrecv;
    unsigned long szsend;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &IMSG_StubDesc);
    
    handle = _pRpcMessage->Handle;
    ( unsigned char __RPC_FAR * )sender = 0;
    ( unsigned long __RPC_FAR * )szrecv = 0;
    ( unsigned char __RPC_FAR *__RPC_FAR * )receiver = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            szsend = *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrConformantArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                          (unsigned char __RPC_FAR * __RPC_FAR *)&sender,
                                          (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                          (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        szrecv = &_M5;
        receiver = &_M6;
        _M6 = 0;
        
        ((IMSG_SERVER_EPV *)(_pRpcMessage->ManagerEpv)) -> ServerConnect(
                 handle,
                 szsend,
                 sender,
                 szrecv,
                 receiver);
        
        _StubMsg.BufferLength = 4U + 8U;
        _StubMsg.MaxCount = szrecv ? *szrecv : 0;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)receiver,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[20] );
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = *szrecv;
        
        _StubMsg.MaxCount = szrecv ? *szrecv : 0;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)receiver,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[20] );
        
        }
    RpcFinally
        {
        _StubMsg.MaxCount = szrecv ? *szrecv : 0;
        
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)receiver,
                        &__MIDL_TypeFormatString.Format[20] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}

void __RPC_STUB
IMSG_PostBlock(
    PRPC_MESSAGE _pRpcMessage )
{
    MIDL_STUB_MESSAGE _StubMsg;
    unsigned char __RPC_FAR *buf;
    handle_t handle;
    unsigned char __RPC_FAR *__RPC_FAR *reply;
    unsigned long sz;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &IMSG_StubDesc);
    
    handle = _pRpcMessage->Handle;
    ( unsigned char __RPC_FAR * )buf = 0;
    ( unsigned char __RPC_FAR *__RPC_FAR * )reply = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[18] );
            
            sz = *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrConformantArrayUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                          (unsigned char __RPC_FAR * __RPC_FAR *)&buf,
                                          (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                          (unsigned char)0 );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&reply,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[38],
                                  (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        
        ((IMSG_SERVER_EPV *)(_pRpcMessage->ManagerEpv)) -> ServerPostBlock(
                   handle,
                   sz,
                   buf,
                   reply);
        
        _StubMsg.BufferLength = 20U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)reply,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[38] );
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)reply,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[38] );
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char __RPC_FAR *)reply,
                        &__MIDL_TypeFormatString.Format[38] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


static const MIDL_STUB_DESC IMSG_StubDesc = 
    {
    (void __RPC_FAR *)& IMSG___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x10001, /* Ndr library version */
    0,
    0x50100a4, /* MIDL Version 5.1.164 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    1,  /* Flags */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION IMSG_table[] =
    {
    IMSG_Connect,
    IMSG_PostBlock,
    0
    };
RPC_DISPATCH_TABLE IMSG_DispatchTable = 
    {
    2,
    IMSG_table
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/*  2 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/*  4 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  6 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */
/*  8 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 10 */	NdrFcShort( 0x10 ),	/* Type Offset=16 */
/* 12 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 14 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */
/* 16 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */
/* 18 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 20 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 22 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 24 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */
/* 26 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 28 */	NdrFcShort( 0x26 ),	/* Type Offset=38 */
/* 30 */	0x5b,		/* FC_END */
			0x5c,		/* FC_PAD */

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
#ifndef _ALPHA_
/* 12 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 14 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 16 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 18 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 20 */	
			0x11, 0x14,	/* FC_RP [alloced_on_stack] */
/* 22 */	NdrFcShort( 0x2 ),	/* Offset= 2 (24) */
/* 24 */	
			0x12, 0x0,	/* FC_UP */
/* 26 */	NdrFcShort( 0x2 ),	/* Offset= 2 (28) */
/* 28 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 30 */	NdrFcShort( 0x1 ),	/* 1 */
/* 32 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x54,		/* FC_DEREFERENCE */
#ifndef _ALPHA_
/* 34 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 36 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 38 */	
			0x12, 0x10,	/* FC_UP */
/* 40 */	NdrFcShort( 0x2 ),	/* Offset= 2 (42) */
/* 42 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 44 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */

			0x0
        }
    };
