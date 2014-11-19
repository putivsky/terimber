

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __imsg_h__
#define __imsg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IMSG_INTERFACE_DEFINED__
#define __IMSG_INTERFACE_DEFINED__

/* interface IMSG */
/* [explicit_handle][unique][version][uuid] */ 

/* client prototype */
void ClientConnect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char *sender,
    /* [out] */ unsigned long *szrecv,
    /* [size_is][size_is][out] */ unsigned char **receiver);
/* server prototype */
void ServerConnect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char *sender,
    /* [out] */ unsigned long *szrecv,
    /* [size_is][size_is][out] */ unsigned char **receiver);
/* switch prototype */
void Connect( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long szsend,
    /* [size_is][in] */ unsigned char *sender,
    /* [out] */ unsigned long *szrecv,
    /* [size_is][size_is][out] */ unsigned char **receiver);

/* client prototype */
void ClientPostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char *buf,
    /* [string][out][in][unique] */ unsigned char **reply);
/* server prototype */
void ServerPostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char *buf,
    /* [string][out][in][unique] */ unsigned char **reply);
/* switch prototype */
void PostBlock( 
    /* [in] */ handle_t handle,
    /* [in] */ unsigned long sz,
    /* [size_is][in] */ unsigned char *buf,
    /* [string][out][in][unique] */ unsigned char **reply);



typedef struct _IMSG_SERVER_EPV
{
    void ( *Connect )( 
        /* [in] */ handle_t handle,
        /* [in] */ unsigned long szsend,
        /* [size_is][in] */ unsigned char *sender,
        /* [out] */ unsigned long *szrecv,
        /* [size_is][size_is][out] */ unsigned char **receiver);
    void ( *PostBlock )( 
        /* [in] */ handle_t handle,
        /* [in] */ unsigned long sz,
        /* [size_is][in] */ unsigned char *buf,
        /* [string][out][in][unique] */ unsigned char **reply);
    
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


