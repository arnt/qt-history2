/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Oct 24 19:58:52 2001
 */
/* Compiler settings for C:\depot\qt\main\tests\axaptamail\axaptamail.idl:
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

#ifndef __axaptamail_h__
#define __axaptamail_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ITemplateParser_FWD_DEFINED__
#define __ITemplateParser_FWD_DEFINED__
typedef interface ITemplateParser ITemplateParser;
#endif 	/* __ITemplateParser_FWD_DEFINED__ */


#ifndef __TemplateParser_FWD_DEFINED__
#define __TemplateParser_FWD_DEFINED__

#ifdef __cplusplus
typedef class TemplateParser TemplateParser;
#else
typedef struct TemplateParser TemplateParser;
#endif /* __cplusplus */

#endif 	/* __TemplateParser_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ITemplateParser_INTERFACE_DEFINED__
#define __ITemplateParser_INTERFACE_DEFINED__

/* interface ITemplateParser */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ITemplateParser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E514925C-6823-419D-AF2B-9BA1B746E8B5")
    ITemplateParser : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE addVariable( 
            BSTR name,
            BSTR value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE parseTemplate( 
            BSTR inStream,
            /* [retval][out] */ BSTR __RPC_FAR *outStream) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE dropDictionary( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITemplateParserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITemplateParser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITemplateParser __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITemplateParser __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITemplateParser __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITemplateParser __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITemplateParser __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITemplateParser __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *addVariable )( 
            ITemplateParser __RPC_FAR * This,
            BSTR name,
            BSTR value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *parseTemplate )( 
            ITemplateParser __RPC_FAR * This,
            BSTR inStream,
            /* [retval][out] */ BSTR __RPC_FAR *outStream);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *dropDictionary )( 
            ITemplateParser __RPC_FAR * This);
        
        END_INTERFACE
    } ITemplateParserVtbl;

    interface ITemplateParser
    {
        CONST_VTBL struct ITemplateParserVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITemplateParser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITemplateParser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITemplateParser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITemplateParser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITemplateParser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITemplateParser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITemplateParser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITemplateParser_addVariable(This,name,value)	\
    (This)->lpVtbl -> addVariable(This,name,value)

#define ITemplateParser_parseTemplate(This,inStream,outStream)	\
    (This)->lpVtbl -> parseTemplate(This,inStream,outStream)

#define ITemplateParser_dropDictionary(This)	\
    (This)->lpVtbl -> dropDictionary(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ITemplateParser_addVariable_Proxy( 
    ITemplateParser __RPC_FAR * This,
    BSTR name,
    BSTR value);


void __RPC_STUB ITemplateParser_addVariable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ITemplateParser_parseTemplate_Proxy( 
    ITemplateParser __RPC_FAR * This,
    BSTR inStream,
    /* [retval][out] */ BSTR __RPC_FAR *outStream);


void __RPC_STUB ITemplateParser_parseTemplate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ITemplateParser_dropDictionary_Proxy( 
    ITemplateParser __RPC_FAR * This);


void __RPC_STUB ITemplateParser_dropDictionary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITemplateParser_INTERFACE_DEFINED__ */



#ifndef __AXAPTAMAILLib_LIBRARY_DEFINED__
#define __AXAPTAMAILLib_LIBRARY_DEFINED__

/* library AXAPTAMAILLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_AXAPTAMAILLib;

EXTERN_C const CLSID CLSID_TemplateParser;

#ifdef __cplusplus

class DECLSPEC_UUID("74013298-9F5A-41F5-BC49-6304B05921DC")
TemplateParser;
#endif
#endif /* __AXAPTAMAILLib_LIBRARY_DEFINED__ */

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
