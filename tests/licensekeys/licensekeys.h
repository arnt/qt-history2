/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Oct 24 10:36:04 2001
 */
/* Compiler settings for C:\depot\qt\main\tests\licensekeys\licensekeys.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __licensekeys_h__
#define __licensekeys_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ILicenseKeyGenerator_FWD_DEFINED__
#define __ILicenseKeyGenerator_FWD_DEFINED__
typedef interface ILicenseKeyGenerator ILicenseKeyGenerator;
#endif 	/* __ILicenseKeyGenerator_FWD_DEFINED__ */


#ifndef __LicenseKeyGenerator_FWD_DEFINED__
#define __LicenseKeyGenerator_FWD_DEFINED__

#ifdef __cplusplus
typedef class LicenseKeyGenerator LicenseKeyGenerator;
#else
typedef struct LicenseKeyGenerator LicenseKeyGenerator;
#endif /* __cplusplus */

#endif 	/* __LicenseKeyGenerator_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ILicenseKeyGenerator_INTERFACE_DEFINED__
#define __ILicenseKeyGenerator_INTERFACE_DEFINED__

/* interface ILicenseKeyGenerator */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ILicenseKeyGenerator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E19839D3-A96B-4732-957A-D857F1F90EBB")
    ILicenseKeyGenerator : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dummy( 
            /* [retval][out] */ BSTR __RPC_FAR *str) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE reset( 
            BSTR keyHome) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE newKey( 
            BSTR keyHome,
            BSTR expiryDate,
            int us,
            int enterprise,
            int windows,
            int unix,
            int embedded,
            int mac,
            int extra1,
            int extra2,
            /* [retval][out] */ BSTR __RPC_FAR *key) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE decodeExpiryDate( 
            BSTR key,
            /* [retval][out] */ BSTR __RPC_FAR *expDate) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksUS( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksEnterprise( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksWindows( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksUnix( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksEmbedded( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksMac( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksExtra1( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE unlocksExtra2( 
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILicenseKeyGeneratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILicenseKeyGenerator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILicenseKeyGenerator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *dummy )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *str);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *reset )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR keyHome);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *newKey )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR keyHome,
            BSTR expiryDate,
            int us,
            int enterprise,
            int windows,
            int unix,
            int embedded,
            int mac,
            int extra1,
            int extra2,
            /* [retval][out] */ BSTR __RPC_FAR *key);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *decodeExpiryDate )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ BSTR __RPC_FAR *expDate);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksUS )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksEnterprise )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksWindows )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksUnix )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksEmbedded )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksMac )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksExtra1 )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *unlocksExtra2 )( 
            ILicenseKeyGenerator __RPC_FAR * This,
            BSTR key,
            /* [retval][out] */ int __RPC_FAR *valid);
        
        END_INTERFACE
    } ILicenseKeyGeneratorVtbl;

    interface ILicenseKeyGenerator
    {
        CONST_VTBL struct ILicenseKeyGeneratorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILicenseKeyGenerator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILicenseKeyGenerator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILicenseKeyGenerator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILicenseKeyGenerator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILicenseKeyGenerator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILicenseKeyGenerator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILicenseKeyGenerator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILicenseKeyGenerator_dummy(This,str)	\
    (This)->lpVtbl -> dummy(This,str)

#define ILicenseKeyGenerator_reset(This,keyHome)	\
    (This)->lpVtbl -> reset(This,keyHome)

#define ILicenseKeyGenerator_newKey(This,keyHome,expiryDate,us,enterprise,windows,unix,embedded,mac,extra1,extra2,key)	\
    (This)->lpVtbl -> newKey(This,keyHome,expiryDate,us,enterprise,windows,unix,embedded,mac,extra1,extra2,key)

#define ILicenseKeyGenerator_decodeExpiryDate(This,key,expDate)	\
    (This)->lpVtbl -> decodeExpiryDate(This,key,expDate)

#define ILicenseKeyGenerator_unlocksUS(This,key,valid)	\
    (This)->lpVtbl -> unlocksUS(This,key,valid)

#define ILicenseKeyGenerator_unlocksEnterprise(This,key,valid)	\
    (This)->lpVtbl -> unlocksEnterprise(This,key,valid)

#define ILicenseKeyGenerator_unlocksWindows(This,key,valid)	\
    (This)->lpVtbl -> unlocksWindows(This,key,valid)

#define ILicenseKeyGenerator_unlocksUnix(This,key,valid)	\
    (This)->lpVtbl -> unlocksUnix(This,key,valid)

#define ILicenseKeyGenerator_unlocksEmbedded(This,key,valid)	\
    (This)->lpVtbl -> unlocksEmbedded(This,key,valid)

#define ILicenseKeyGenerator_unlocksMac(This,key,valid)	\
    (This)->lpVtbl -> unlocksMac(This,key,valid)

#define ILicenseKeyGenerator_unlocksExtra1(This,key,valid)	\
    (This)->lpVtbl -> unlocksExtra1(This,key,valid)

#define ILicenseKeyGenerator_unlocksExtra2(This,key,valid)	\
    (This)->lpVtbl -> unlocksExtra2(This,key,valid)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_dummy_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *str);


void __RPC_STUB ILicenseKeyGenerator_dummy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_reset_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR keyHome);


void __RPC_STUB ILicenseKeyGenerator_reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_newKey_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR keyHome,
    BSTR expiryDate,
    int us,
    int enterprise,
    int windows,
    int unix,
    int embedded,
    int mac,
    int extra1,
    int extra2,
    /* [retval][out] */ BSTR __RPC_FAR *key);


void __RPC_STUB ILicenseKeyGenerator_newKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_decodeExpiryDate_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ BSTR __RPC_FAR *expDate);


void __RPC_STUB ILicenseKeyGenerator_decodeExpiryDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksUS_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksUS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksEnterprise_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksEnterprise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksWindows_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksWindows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksUnix_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksUnix_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksEmbedded_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksEmbedded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksMac_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksMac_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksExtra1_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksExtra1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicenseKeyGenerator_unlocksExtra2_Proxy( 
    ILicenseKeyGenerator __RPC_FAR * This,
    BSTR key,
    /* [retval][out] */ int __RPC_FAR *valid);


void __RPC_STUB ILicenseKeyGenerator_unlocksExtra2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILicenseKeyGenerator_INTERFACE_DEFINED__ */



#ifndef __LICENSEKEYSLib_LIBRARY_DEFINED__
#define __LICENSEKEYSLib_LIBRARY_DEFINED__

/* library LICENSEKEYSLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_LICENSEKEYSLib;

EXTERN_C const CLSID CLSID_LicenseKeyGenerator;

#ifdef __cplusplus

class DECLSPEC_UUID("3F868798-3D8A-4186-8B6C-4AF8E238357B")
LicenseKeyGenerator;
#endif
#endif /* __LICENSEKEYSLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
