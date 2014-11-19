/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Apr 15 22:22:54 2009
 */
/* Compiler settings for imsg.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation ref bounds_check enum stub_data , use_epv, no_default_epv
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __imsg_h__
#define __imsg_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IMSG_INTERFACE_DEFINED__
#define __IMSG_INTERFACE_DEFINED__

/* interface IMSG */
/* [explicit_handle][unique][version][uuid] */ 

/* client prototype */
void ClientConnect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char __RPC_FAR *sender,
    /* [out] */ unsigned long __RPC_FAR *szrecv,
    /* [size_is][size_is][out] */ unsigned char __RPC_FAR *__RPC_FAR *receiver);
/* server prototype */
void ServerConnect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char __RPC_FAR *sender,
    /* [out] */ unsigned long __RPC_FAR *szrecv,
    /* [size_is][size_is][out] */ unsigned char __RPC_FAR *__RPC_FAR *receiver);
/* switch prototype */
void Connect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char __RPC_FAR *sender,
    /* [out] */ unsigned long __RPC_FAR *szrecv,
    /* [size_is][size_is][out] */ unsigned char __RPC_FAR *__RPC_FAR *receiver);

/* client prototype */
void ClientPostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char __RPC_FAR *buf,
    /* [string][out][in][unique] */ unsigned char __RPC_FAR *__RPC_FAR *reply);
/* server prototype */
void ServerPostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char __RPC_FAR *buf,
    /* [string][out][in][unique] */ unsigned char __RPC_FAR *__RPC_FAR *reply);
/* switch prototype */
void PostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char __RPC_FAR *buf,
    /* [string][out][in][unique] */ unsigned char __RPC_FAR *__RPC_FAR *reply);



typedef struct _IMSG_SERVER_EPV
{
    void ( __RPC_FAR *ServerConnect )( 
        /* [in] */ handle_t handle,
        /* [in] */ unsigned long szsend,
        /* [size_is][in] */ unsigned char __RPC_FAR *sender,
        /* [out] */ unsigned long __RPC_FAR *szrecv,
        /* [size_is][size_is][out] */ unsigned char __RPC_FAR *__RPC_FAR *receiver);
    void ( __RPC_FAR *ServerPostBlock )( 
        /* [in] */ handle_t handle,
        /* [in] */ unsigned long sz,
        /* [size_is][in] */ unsigned char __RPC_FAR *buf,
        /* [string][out][in][unique] */ unsigned char __RPC_FAR *__RPC_FAR *reply);
    
    } IMSG_SERVER_EPV;

extern RPC_IF_HANDLE ClientIMSG_ClientIfHandle;
extern RPC_IF_HANDLE IMSG_ClientIfHandle;
extern RPC_IF_HANDLE ServerIMSG_ServerIfHandle;
#endif /* __IMSG_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
