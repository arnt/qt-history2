/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Fri Nov 23 18:56:09 2001
 */
/* Compiler settings for C:\depot\qt\3.0\tests\distsystem\licproc_com\licproc_com.idl:
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

#ifndef __licproc_com_h__
#define __licproc_com_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ILicProc_FWD_DEFINED__
#define __ILicProc_FWD_DEFINED__
typedef interface ILicProc ILicProc;
#endif 	/* __ILicProc_FWD_DEFINED__ */


#ifndef __IBase64Codec_FWD_DEFINED__
#define __IBase64Codec_FWD_DEFINED__
typedef interface IBase64Codec IBase64Codec;
#endif 	/* __IBase64Codec_FWD_DEFINED__ */


#ifndef __LicProc_FWD_DEFINED__
#define __LicProc_FWD_DEFINED__

#ifdef __cplusplus
typedef class LicProc LicProc;
#else
typedef struct LicProc LicProc;
#endif /* __cplusplus */

#endif 	/* __LicProc_FWD_DEFINED__ */


#ifndef __Base64Codec_FWD_DEFINED__
#define __Base64Codec_FWD_DEFINED__

#ifdef __cplusplus
typedef class Base64Codec Base64Codec;
#else
typedef struct Base64Codec Base64Codec;
#endif /* __cplusplus */

#endif 	/* __Base64Codec_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ILicProc_INTERFACE_DEFINED__
#define __ILicProc_INTERFACE_DEFINED__

/* interface ILicProc */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ILicProc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4881DDEE-ECE8-4496-9F4A-FD33FC500FED")
    ILicProc : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dummy( 
            DWORD bar) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE publishLicense( 
            DWORD licenseId,
            BSTR custId,
            BSTR licensee,
            BSTR licenseeEmail,
            BSTR login,
            BSTR password,
            BSTR expiryDate,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE publishLine( 
            DWORD licenseId,
            BSTR itemId,
            DWORD usLicense,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE setServer( 
            BSTR srv,
            BSTR user,
            BSTR password,
            BSTR db) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE updateDb( 
            DWORD licenseId,
            BSTR versionTag,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE publishVersionTag( 
            BSTR tag,
            BSTR versionString,
            BSTR subDir,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE publishFilemap( 
            BSTR tag,
            BSTR itemId,
            BSTR fileName,
            BSTR fileDesc,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE clearVersionTags( 
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE clearFilemap( 
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE deleteVersionTag( 
            BSTR tag,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE deleteFilemap( 
            BSTR tag,
            BSTR itemId,
            BSTR fileName,
            BSTR companyId) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE deleteLicense( 
            BSTR licenseId,
            BSTR companyId) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILicProcVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILicProc __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILicProc __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILicProc __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ILicProc __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ILicProc __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ILicProc __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ILicProc __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *dummy )( 
            ILicProc __RPC_FAR * This,
            DWORD bar);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *publishLicense )( 
            ILicProc __RPC_FAR * This,
            DWORD licenseId,
            BSTR custId,
            BSTR licensee,
            BSTR licenseeEmail,
            BSTR login,
            BSTR password,
            BSTR expiryDate,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *publishLine )( 
            ILicProc __RPC_FAR * This,
            DWORD licenseId,
            BSTR itemId,
            DWORD usLicense,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *setServer )( 
            ILicProc __RPC_FAR * This,
            BSTR srv,
            BSTR user,
            BSTR password,
            BSTR db);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *updateDb )( 
            ILicProc __RPC_FAR * This,
            DWORD licenseId,
            BSTR versionTag,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *publishVersionTag )( 
            ILicProc __RPC_FAR * This,
            BSTR tag,
            BSTR versionString,
            BSTR subDir,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *publishFilemap )( 
            ILicProc __RPC_FAR * This,
            BSTR tag,
            BSTR itemId,
            BSTR fileName,
            BSTR fileDesc,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *clearVersionTags )( 
            ILicProc __RPC_FAR * This,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *clearFilemap )( 
            ILicProc __RPC_FAR * This,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteVersionTag )( 
            ILicProc __RPC_FAR * This,
            BSTR tag,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteFilemap )( 
            ILicProc __RPC_FAR * This,
            BSTR tag,
            BSTR itemId,
            BSTR fileName,
            BSTR companyId);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *deleteLicense )( 
            ILicProc __RPC_FAR * This,
            BSTR licenseId,
            BSTR companyId);
        
        END_INTERFACE
    } ILicProcVtbl;

    interface ILicProc
    {
        CONST_VTBL struct ILicProcVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILicProc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILicProc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILicProc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILicProc_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILicProc_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILicProc_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILicProc_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILicProc_dummy(This,bar)	\
    (This)->lpVtbl -> dummy(This,bar)

#define ILicProc_publishLicense(This,licenseId,custId,licensee,licenseeEmail,login,password,expiryDate,companyId)	\
    (This)->lpVtbl -> publishLicense(This,licenseId,custId,licensee,licenseeEmail,login,password,expiryDate,companyId)

#define ILicProc_publishLine(This,licenseId,itemId,usLicense,companyId)	\
    (This)->lpVtbl -> publishLine(This,licenseId,itemId,usLicense,companyId)

#define ILicProc_setServer(This,srv,user,password,db)	\
    (This)->lpVtbl -> setServer(This,srv,user,password,db)

#define ILicProc_updateDb(This,licenseId,versionTag,companyId)	\
    (This)->lpVtbl -> updateDb(This,licenseId,versionTag,companyId)

#define ILicProc_publishVersionTag(This,tag,versionString,subDir,companyId)	\
    (This)->lpVtbl -> publishVersionTag(This,tag,versionString,subDir,companyId)

#define ILicProc_publishFilemap(This,tag,itemId,fileName,fileDesc,companyId)	\
    (This)->lpVtbl -> publishFilemap(This,tag,itemId,fileName,fileDesc,companyId)

#define ILicProc_clearVersionTags(This,companyId)	\
    (This)->lpVtbl -> clearVersionTags(This,companyId)

#define ILicProc_clearFilemap(This,companyId)	\
    (This)->lpVtbl -> clearFilemap(This,companyId)

#define ILicProc_deleteVersionTag(This,tag,companyId)	\
    (This)->lpVtbl -> deleteVersionTag(This,tag,companyId)

#define ILicProc_deleteFilemap(This,tag,itemId,fileName,companyId)	\
    (This)->lpVtbl -> deleteFilemap(This,tag,itemId,fileName,companyId)

#define ILicProc_deleteLicense(This,licenseId,companyId)	\
    (This)->lpVtbl -> deleteLicense(This,licenseId,companyId)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_dummy_Proxy( 
    ILicProc __RPC_FAR * This,
    DWORD bar);


void __RPC_STUB ILicProc_dummy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_publishLicense_Proxy( 
    ILicProc __RPC_FAR * This,
    DWORD licenseId,
    BSTR custId,
    BSTR licensee,
    BSTR licenseeEmail,
    BSTR login,
    BSTR password,
    BSTR expiryDate,
    BSTR companyId);


void __RPC_STUB ILicProc_publishLicense_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_publishLine_Proxy( 
    ILicProc __RPC_FAR * This,
    DWORD licenseId,
    BSTR itemId,
    DWORD usLicense,
    BSTR companyId);


void __RPC_STUB ILicProc_publishLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_setServer_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR srv,
    BSTR user,
    BSTR password,
    BSTR db);


void __RPC_STUB ILicProc_setServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_updateDb_Proxy( 
    ILicProc __RPC_FAR * This,
    DWORD licenseId,
    BSTR versionTag,
    BSTR companyId);


void __RPC_STUB ILicProc_updateDb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_publishVersionTag_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR tag,
    BSTR versionString,
    BSTR subDir,
    BSTR companyId);


void __RPC_STUB ILicProc_publishVersionTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_publishFilemap_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR tag,
    BSTR itemId,
    BSTR fileName,
    BSTR fileDesc,
    BSTR companyId);


void __RPC_STUB ILicProc_publishFilemap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_clearVersionTags_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR companyId);


void __RPC_STUB ILicProc_clearVersionTags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_clearFilemap_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR companyId);


void __RPC_STUB ILicProc_clearFilemap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_deleteVersionTag_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR tag,
    BSTR companyId);


void __RPC_STUB ILicProc_deleteVersionTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_deleteFilemap_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR tag,
    BSTR itemId,
    BSTR fileName,
    BSTR companyId);


void __RPC_STUB ILicProc_deleteFilemap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ILicProc_deleteLicense_Proxy( 
    ILicProc __RPC_FAR * This,
    BSTR licenseId,
    BSTR companyId);


void __RPC_STUB ILicProc_deleteLicense_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILicProc_INTERFACE_DEFINED__ */


#ifndef __IBase64Codec_INTERFACE_DEFINED__
#define __IBase64Codec_INTERFACE_DEFINED__

/* interface IBase64Codec */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IBase64Codec;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1D51A88-5C83-4F00-86DB-A4B32B42FFA4")
    IBase64Codec : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Encode( 
            int numBytes) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Decode( 
            int numBytes) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_EncodedData( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_EncodedData( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DecodedData( 
            /* [retval][out] */ BYTE __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DecodedData( 
            /* [in] */ BYTE __RPC_FAR *newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IBase64CodecVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IBase64Codec __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IBase64Codec __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IBase64Codec __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IBase64Codec __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IBase64Codec __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IBase64Codec __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IBase64Codec __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Encode )( 
            IBase64Codec __RPC_FAR * This,
            int numBytes);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Decode )( 
            IBase64Codec __RPC_FAR * This,
            int numBytes);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EncodedData )( 
            IBase64Codec __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EncodedData )( 
            IBase64Codec __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DecodedData )( 
            IBase64Codec __RPC_FAR * This,
            /* [retval][out] */ BYTE __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DecodedData )( 
            IBase64Codec __RPC_FAR * This,
            /* [in] */ BYTE __RPC_FAR *newVal);
        
        END_INTERFACE
    } IBase64CodecVtbl;

    interface IBase64Codec
    {
        CONST_VTBL struct IBase64CodecVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBase64Codec_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBase64Codec_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBase64Codec_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBase64Codec_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBase64Codec_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBase64Codec_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBase64Codec_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBase64Codec_Encode(This,numBytes)	\
    (This)->lpVtbl -> Encode(This,numBytes)

#define IBase64Codec_Decode(This,numBytes)	\
    (This)->lpVtbl -> Decode(This,numBytes)

#define IBase64Codec_get_EncodedData(This,pVal)	\
    (This)->lpVtbl -> get_EncodedData(This,pVal)

#define IBase64Codec_put_EncodedData(This,newVal)	\
    (This)->lpVtbl -> put_EncodedData(This,newVal)

#define IBase64Codec_get_DecodedData(This,pVal)	\
    (This)->lpVtbl -> get_DecodedData(This,pVal)

#define IBase64Codec_put_DecodedData(This,newVal)	\
    (This)->lpVtbl -> put_DecodedData(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IBase64Codec_Encode_Proxy( 
    IBase64Codec __RPC_FAR * This,
    int numBytes);


void __RPC_STUB IBase64Codec_Encode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IBase64Codec_Decode_Proxy( 
    IBase64Codec __RPC_FAR * This,
    int numBytes);


void __RPC_STUB IBase64Codec_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IBase64Codec_get_EncodedData_Proxy( 
    IBase64Codec __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IBase64Codec_get_EncodedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IBase64Codec_put_EncodedData_Proxy( 
    IBase64Codec __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IBase64Codec_put_EncodedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IBase64Codec_get_DecodedData_Proxy( 
    IBase64Codec __RPC_FAR * This,
    /* [retval][out] */ BYTE __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IBase64Codec_get_DecodedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IBase64Codec_put_DecodedData_Proxy( 
    IBase64Codec __RPC_FAR * This,
    /* [in] */ BYTE __RPC_FAR *newVal);


void __RPC_STUB IBase64Codec_put_DecodedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IBase64Codec_INTERFACE_DEFINED__ */



#ifndef __LICPROC_COMLib_LIBRARY_DEFINED__
#define __LICPROC_COMLib_LIBRARY_DEFINED__

/* library LICPROC_COMLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_LICPROC_COMLib;

EXTERN_C const CLSID CLSID_LicProc;

#ifdef __cplusplus

class DECLSPEC_UUID("5A084CF4-C6E2-42B5-A918-1FAD5EA08F38")
LicProc;
#endif

EXTERN_C const CLSID CLSID_Base64Codec;

#ifdef __cplusplus

class DECLSPEC_UUID("B5FDF0C6-247C-4056-9B2E-BB9DEC497CA8")
Base64Codec;
#endif
#endif /* __LICPROC_COMLib_LIBRARY_DEFINED__ */

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
