/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue May 07 17:09:25 2002
 */
/* Compiler settings for C:\depot\qt\3.0\tests\qgraph\QGraph.idl:
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

#ifndef __QGraph_h__
#define __QGraph_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IHistogram_FWD_DEFINED__
#define __IHistogram_FWD_DEFINED__
typedef interface IHistogram IHistogram;
#endif 	/* __IHistogram_FWD_DEFINED__ */


#ifndef __IGraph_FWD_DEFINED__
#define __IGraph_FWD_DEFINED__
typedef interface IGraph IGraph;
#endif 	/* __IGraph_FWD_DEFINED__ */


#ifndef __Histogram_FWD_DEFINED__
#define __Histogram_FWD_DEFINED__

#ifdef __cplusplus
typedef class Histogram Histogram;
#else
typedef struct Histogram Histogram;
#endif /* __cplusplus */

#endif 	/* __Histogram_FWD_DEFINED__ */


#ifndef __Graph_FWD_DEFINED__
#define __Graph_FWD_DEFINED__

#ifdef __cplusplus
typedef class Graph Graph;
#else
typedef struct Graph Graph;
#endif /* __cplusplus */

#endif 	/* __Graph_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IHistogram_INTERFACE_DEFINED__
#define __IHistogram_INTERFACE_DEFINED__

/* interface IHistogram */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IHistogram;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("13E2D1FB-B599-4C58-8C5B-FDC925008261")
    IHistogram : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumBars( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NumBars( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumSegments( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NumSegments( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaxValue( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaxValue( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SegmentColor( 
            long SegmentNum,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SegmentColor( 
            long SegmentNum,
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Value( 
            long BarNumber,
            long SegmentNumber,
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Value( 
            long BarNumber,
            long SegmentNumber,
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BarWidth( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BarWidth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Picture( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showExplanation( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showExplanation( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SegmentName( 
            long SegmentNumber,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SegmentName( 
            long SegmentNumber,
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BarName( 
            long BarNumber,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BarName( 
            long BarNumber,
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showBarNames( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showBarNames( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackgroundColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackgroundColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showHorizontalGrid( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showHorizontalGrid( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GridColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GridColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showVerticalGrid( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showVerticalGrid( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowValues( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowValues( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TextColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TextColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MimeType( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GridSpacing( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GridSpacing( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showScale( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showScale( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_totalForBar( 
            /* [in] */ long BarNumber,
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ScaleFormat( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ScaleFormat( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GraphTitle( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GraphTitle( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showGraphTitle( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showGraphTitle( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showTotal( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showTotal( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WorksheetColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WorksheetColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisplayMode( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisplayMode( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IHistogramVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IHistogram __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IHistogram __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IHistogram __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NumBars )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_NumBars )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NumSegments )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_NumSegments )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MaxValue )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MaxValue )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SegmentColor )( 
            IHistogram __RPC_FAR * This,
            long SegmentNum,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SegmentColor )( 
            IHistogram __RPC_FAR * This,
            long SegmentNum,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            IHistogram __RPC_FAR * This,
            long BarNumber,
            long SegmentNumber,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )( 
            IHistogram __RPC_FAR * This,
            long BarNumber,
            long SegmentNumber,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BarWidth )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BarWidth )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Picture )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showExplanation )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showExplanation )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SegmentName )( 
            IHistogram __RPC_FAR * This,
            long SegmentNumber,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SegmentName )( 
            IHistogram __RPC_FAR * This,
            long SegmentNumber,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BarName )( 
            IHistogram __RPC_FAR * This,
            long BarNumber,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BarName )( 
            IHistogram __RPC_FAR * This,
            long BarNumber,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showBarNames )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showBarNames )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BackgroundColor )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BackgroundColor )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showHorizontalGrid )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showHorizontalGrid )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GridColor )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GridColor )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showVerticalGrid )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showVerticalGrid )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowValues )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowValues )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TextColor )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TextColor )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MimeType )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GridSpacing )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GridSpacing )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IHistogram __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showScale )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showScale )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_totalForBar )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ long BarNumber,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ScaleFormat )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ScaleFormat )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GraphTitle )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GraphTitle )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showGraphTitle )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showGraphTitle )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showTotal )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showTotal )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WorksheetColor )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_WorksheetColor )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayMode )( 
            IHistogram __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DisplayMode )( 
            IHistogram __RPC_FAR * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IHistogramVtbl;

    interface IHistogram
    {
        CONST_VTBL struct IHistogramVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHistogram_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHistogram_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHistogram_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHistogram_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IHistogram_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IHistogram_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IHistogram_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IHistogram_get_NumBars(This,pVal)	\
    (This)->lpVtbl -> get_NumBars(This,pVal)

#define IHistogram_put_NumBars(This,newVal)	\
    (This)->lpVtbl -> put_NumBars(This,newVal)

#define IHistogram_get_NumSegments(This,pVal)	\
    (This)->lpVtbl -> get_NumSegments(This,pVal)

#define IHistogram_put_NumSegments(This,newVal)	\
    (This)->lpVtbl -> put_NumSegments(This,newVal)

#define IHistogram_get_MaxValue(This,pVal)	\
    (This)->lpVtbl -> get_MaxValue(This,pVal)

#define IHistogram_put_MaxValue(This,newVal)	\
    (This)->lpVtbl -> put_MaxValue(This,newVal)

#define IHistogram_get_SegmentColor(This,SegmentNum,pVal)	\
    (This)->lpVtbl -> get_SegmentColor(This,SegmentNum,pVal)

#define IHistogram_put_SegmentColor(This,SegmentNum,newVal)	\
    (This)->lpVtbl -> put_SegmentColor(This,SegmentNum,newVal)

#define IHistogram_get_Value(This,BarNumber,SegmentNumber,pVal)	\
    (This)->lpVtbl -> get_Value(This,BarNumber,SegmentNumber,pVal)

#define IHistogram_put_Value(This,BarNumber,SegmentNumber,newVal)	\
    (This)->lpVtbl -> put_Value(This,BarNumber,SegmentNumber,newVal)

#define IHistogram_get_Width(This,pVal)	\
    (This)->lpVtbl -> get_Width(This,pVal)

#define IHistogram_put_Width(This,newVal)	\
    (This)->lpVtbl -> put_Width(This,newVal)

#define IHistogram_get_Height(This,pVal)	\
    (This)->lpVtbl -> get_Height(This,pVal)

#define IHistogram_get_BarWidth(This,pVal)	\
    (This)->lpVtbl -> get_BarWidth(This,pVal)

#define IHistogram_put_BarWidth(This,newVal)	\
    (This)->lpVtbl -> put_BarWidth(This,newVal)

#define IHistogram_get_Picture(This,pVal)	\
    (This)->lpVtbl -> get_Picture(This,pVal)

#define IHistogram_get_showExplanation(This,pVal)	\
    (This)->lpVtbl -> get_showExplanation(This,pVal)

#define IHistogram_put_showExplanation(This,newVal)	\
    (This)->lpVtbl -> put_showExplanation(This,newVal)

#define IHistogram_get_SegmentName(This,SegmentNumber,pVal)	\
    (This)->lpVtbl -> get_SegmentName(This,SegmentNumber,pVal)

#define IHistogram_put_SegmentName(This,SegmentNumber,newVal)	\
    (This)->lpVtbl -> put_SegmentName(This,SegmentNumber,newVal)

#define IHistogram_get_BarName(This,BarNumber,pVal)	\
    (This)->lpVtbl -> get_BarName(This,BarNumber,pVal)

#define IHistogram_put_BarName(This,BarNumber,newVal)	\
    (This)->lpVtbl -> put_BarName(This,BarNumber,newVal)

#define IHistogram_get_showBarNames(This,pVal)	\
    (This)->lpVtbl -> get_showBarNames(This,pVal)

#define IHistogram_put_showBarNames(This,newVal)	\
    (This)->lpVtbl -> put_showBarNames(This,newVal)

#define IHistogram_get_BackgroundColor(This,pVal)	\
    (This)->lpVtbl -> get_BackgroundColor(This,pVal)

#define IHistogram_put_BackgroundColor(This,newVal)	\
    (This)->lpVtbl -> put_BackgroundColor(This,newVal)

#define IHistogram_get_showHorizontalGrid(This,pVal)	\
    (This)->lpVtbl -> get_showHorizontalGrid(This,pVal)

#define IHistogram_put_showHorizontalGrid(This,newVal)	\
    (This)->lpVtbl -> put_showHorizontalGrid(This,newVal)

#define IHistogram_get_GridColor(This,pVal)	\
    (This)->lpVtbl -> get_GridColor(This,pVal)

#define IHistogram_put_GridColor(This,newVal)	\
    (This)->lpVtbl -> put_GridColor(This,newVal)

#define IHistogram_get_showVerticalGrid(This,pVal)	\
    (This)->lpVtbl -> get_showVerticalGrid(This,pVal)

#define IHistogram_put_showVerticalGrid(This,newVal)	\
    (This)->lpVtbl -> put_showVerticalGrid(This,newVal)

#define IHistogram_get_ShowValues(This,pVal)	\
    (This)->lpVtbl -> get_ShowValues(This,pVal)

#define IHistogram_put_ShowValues(This,newVal)	\
    (This)->lpVtbl -> put_ShowValues(This,newVal)

#define IHistogram_get_TextColor(This,pVal)	\
    (This)->lpVtbl -> get_TextColor(This,pVal)

#define IHistogram_put_TextColor(This,newVal)	\
    (This)->lpVtbl -> put_TextColor(This,newVal)

#define IHistogram_get_MimeType(This,pVal)	\
    (This)->lpVtbl -> get_MimeType(This,pVal)

#define IHistogram_get_GridSpacing(This,pVal)	\
    (This)->lpVtbl -> get_GridSpacing(This,pVal)

#define IHistogram_put_GridSpacing(This,newVal)	\
    (This)->lpVtbl -> put_GridSpacing(This,newVal)

#define IHistogram_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IHistogram_get_showScale(This,pVal)	\
    (This)->lpVtbl -> get_showScale(This,pVal)

#define IHistogram_put_showScale(This,newVal)	\
    (This)->lpVtbl -> put_showScale(This,newVal)

#define IHistogram_get_totalForBar(This,BarNumber,pVal)	\
    (This)->lpVtbl -> get_totalForBar(This,BarNumber,pVal)

#define IHistogram_get_ScaleFormat(This,pVal)	\
    (This)->lpVtbl -> get_ScaleFormat(This,pVal)

#define IHistogram_put_ScaleFormat(This,newVal)	\
    (This)->lpVtbl -> put_ScaleFormat(This,newVal)

#define IHistogram_get_GraphTitle(This,pVal)	\
    (This)->lpVtbl -> get_GraphTitle(This,pVal)

#define IHistogram_put_GraphTitle(This,newVal)	\
    (This)->lpVtbl -> put_GraphTitle(This,newVal)

#define IHistogram_get_showGraphTitle(This,pVal)	\
    (This)->lpVtbl -> get_showGraphTitle(This,pVal)

#define IHistogram_put_showGraphTitle(This,newVal)	\
    (This)->lpVtbl -> put_showGraphTitle(This,newVal)

#define IHistogram_get_showTotal(This,pVal)	\
    (This)->lpVtbl -> get_showTotal(This,pVal)

#define IHistogram_put_showTotal(This,newVal)	\
    (This)->lpVtbl -> put_showTotal(This,newVal)

#define IHistogram_get_WorksheetColor(This,pVal)	\
    (This)->lpVtbl -> get_WorksheetColor(This,pVal)

#define IHistogram_put_WorksheetColor(This,newVal)	\
    (This)->lpVtbl -> put_WorksheetColor(This,newVal)

#define IHistogram_get_DisplayMode(This,pVal)	\
    (This)->lpVtbl -> get_DisplayMode(This,pVal)

#define IHistogram_put_DisplayMode(This,newVal)	\
    (This)->lpVtbl -> put_DisplayMode(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_NumBars_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_NumBars_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_NumBars_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IHistogram_put_NumBars_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_NumSegments_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_NumSegments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_NumSegments_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IHistogram_put_NumSegments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_MaxValue_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_MaxValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_MaxValue_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IHistogram_put_MaxValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_SegmentColor_Proxy( 
    IHistogram __RPC_FAR * This,
    long SegmentNum,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_SegmentColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_SegmentColor_Proxy( 
    IHistogram __RPC_FAR * This,
    long SegmentNum,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IHistogram_put_SegmentColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_Value_Proxy( 
    IHistogram __RPC_FAR * This,
    long BarNumber,
    long SegmentNumber,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_Value_Proxy( 
    IHistogram __RPC_FAR * This,
    long BarNumber,
    long SegmentNumber,
    /* [in] */ double newVal);


void __RPC_STUB IHistogram_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_Width_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_Width_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IHistogram_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_Height_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_BarWidth_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_BarWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_BarWidth_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IHistogram_put_BarWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_Picture_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_Picture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showExplanation_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showExplanation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showExplanation_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showExplanation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_SegmentName_Proxy( 
    IHistogram __RPC_FAR * This,
    long SegmentNumber,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_SegmentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_SegmentName_Proxy( 
    IHistogram __RPC_FAR * This,
    long SegmentNumber,
    /* [in] */ BSTR newVal);


void __RPC_STUB IHistogram_put_SegmentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_BarName_Proxy( 
    IHistogram __RPC_FAR * This,
    long BarNumber,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_BarName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_BarName_Proxy( 
    IHistogram __RPC_FAR * This,
    long BarNumber,
    /* [in] */ BSTR newVal);


void __RPC_STUB IHistogram_put_BarName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showBarNames_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showBarNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showBarNames_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showBarNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_BackgroundColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_BackgroundColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_BackgroundColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IHistogram_put_BackgroundColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showHorizontalGrid_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showHorizontalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showHorizontalGrid_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showHorizontalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_GridColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_GridColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_GridColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IHistogram_put_GridColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showVerticalGrid_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showVerticalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showVerticalGrid_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showVerticalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_ShowValues_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_ShowValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_ShowValues_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_ShowValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_TextColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_TextColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_TextColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IHistogram_put_TextColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_MimeType_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_MimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_GridSpacing_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_GridSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_GridSpacing_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IHistogram_put_GridSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IHistogram_Reset_Proxy( 
    IHistogram __RPC_FAR * This);


void __RPC_STUB IHistogram_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showScale_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showScale_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_totalForBar_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ long BarNumber,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_totalForBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_ScaleFormat_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_ScaleFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_ScaleFormat_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IHistogram_put_ScaleFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_GraphTitle_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_GraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_GraphTitle_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IHistogram_put_GraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showGraphTitle_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showGraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showGraphTitle_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showGraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_showTotal_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_showTotal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_showTotal_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IHistogram_put_showTotal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_WorksheetColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_WorksheetColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_WorksheetColor_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IHistogram_put_WorksheetColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IHistogram_get_DisplayMode_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IHistogram_get_DisplayMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IHistogram_put_DisplayMode_Proxy( 
    IHistogram __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IHistogram_put_DisplayMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IHistogram_INTERFACE_DEFINED__ */


#ifndef __IGraph_INTERFACE_DEFINED__
#define __IGraph_INTERFACE_DEFINED__

/* interface IGraph */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IGraph;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2286FA16-F08B-4B77-B3E9-BFD3484A138D")
    IGraph : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GraphTitle( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GraphTitle( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showGraphTitle( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showGraphTitle( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Picture( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TextColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TextColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackgroundColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackgroundColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GridColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GridColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showHorizontalGrid( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showHorizontalGrid( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showVerticalGrid( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showVerticalGrid( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VerticalGridSpacing( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VerticalGridSpacing( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showHorizontalScale( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showHorizontalScale( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MimeType( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_HorizontalGridSpacing( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_HorizontalGridSpacing( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumValues( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NumValues( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumGraphs( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NumGraphs( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MaxValue( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MaxValue( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showExplanation( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showExplanation( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GraphColor( 
            /* [in] */ long GraphNumber,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GraphColor( 
            /* [in] */ long GraphNumber,
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GraphName( 
            long GraphNumber,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GraphName( 
            long GraphNumber,
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Value( 
            long GraphNum,
            long Index,
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Value( 
            long GraphNum,
            long Index,
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_showVerticalScale( 
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_showVerticalScale( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_scaleFormat( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_scaleFormat( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ValueName( 
            long Index,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ValueName( 
            long Index,
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WorksheetColor( 
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WorksheetColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGraphVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGraph __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGraph __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGraph __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IGraph __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IGraph __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IGraph __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IGraph __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IGraph __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IGraph __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GraphTitle )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GraphTitle )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showGraphTitle )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showGraphTitle )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IGraph __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Picture )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TextColor )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TextColor )( 
            IGraph __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BackgroundColor )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BackgroundColor )( 
            IGraph __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GridColor )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GridColor )( 
            IGraph __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showHorizontalGrid )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showHorizontalGrid )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showVerticalGrid )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showVerticalGrid )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VerticalGridSpacing )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VerticalGridSpacing )( 
            IGraph __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showHorizontalScale )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showHorizontalScale )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MimeType )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HorizontalGridSpacing )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_HorizontalGridSpacing )( 
            IGraph __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NumValues )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_NumValues )( 
            IGraph __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NumGraphs )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_NumGraphs )( 
            IGraph __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MaxValue )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MaxValue )( 
            IGraph __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showExplanation )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showExplanation )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GraphColor )( 
            IGraph __RPC_FAR * This,
            /* [in] */ long GraphNumber,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GraphColor )( 
            IGraph __RPC_FAR * This,
            /* [in] */ long GraphNumber,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GraphName )( 
            IGraph __RPC_FAR * This,
            long GraphNumber,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GraphName )( 
            IGraph __RPC_FAR * This,
            long GraphNumber,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            IGraph __RPC_FAR * This,
            long GraphNum,
            long Index,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Value )( 
            IGraph __RPC_FAR * This,
            long GraphNum,
            long Index,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_showVerticalScale )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_showVerticalScale )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_scaleFormat )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_scaleFormat )( 
            IGraph __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ValueName )( 
            IGraph __RPC_FAR * This,
            long Index,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ValueName )( 
            IGraph __RPC_FAR * This,
            long Index,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_WorksheetColor )( 
            IGraph __RPC_FAR * This,
            /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_WorksheetColor )( 
            IGraph __RPC_FAR * This,
            /* [in] */ OLE_COLOR newVal);
        
        END_INTERFACE
    } IGraphVtbl;

    interface IGraph
    {
        CONST_VTBL struct IGraphVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGraph_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGraph_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGraph_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGraph_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGraph_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGraph_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGraph_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGraph_get_Width(This,pVal)	\
    (This)->lpVtbl -> get_Width(This,pVal)

#define IGraph_put_Width(This,newVal)	\
    (This)->lpVtbl -> put_Width(This,newVal)

#define IGraph_get_Height(This,pVal)	\
    (This)->lpVtbl -> get_Height(This,pVal)

#define IGraph_put_Height(This,newVal)	\
    (This)->lpVtbl -> put_Height(This,newVal)

#define IGraph_get_GraphTitle(This,pVal)	\
    (This)->lpVtbl -> get_GraphTitle(This,pVal)

#define IGraph_put_GraphTitle(This,newVal)	\
    (This)->lpVtbl -> put_GraphTitle(This,newVal)

#define IGraph_get_showGraphTitle(This,pVal)	\
    (This)->lpVtbl -> get_showGraphTitle(This,pVal)

#define IGraph_put_showGraphTitle(This,newVal)	\
    (This)->lpVtbl -> put_showGraphTitle(This,newVal)

#define IGraph_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IGraph_get_Picture(This,pVal)	\
    (This)->lpVtbl -> get_Picture(This,pVal)

#define IGraph_get_TextColor(This,pVal)	\
    (This)->lpVtbl -> get_TextColor(This,pVal)

#define IGraph_put_TextColor(This,newVal)	\
    (This)->lpVtbl -> put_TextColor(This,newVal)

#define IGraph_get_BackgroundColor(This,pVal)	\
    (This)->lpVtbl -> get_BackgroundColor(This,pVal)

#define IGraph_put_BackgroundColor(This,newVal)	\
    (This)->lpVtbl -> put_BackgroundColor(This,newVal)

#define IGraph_get_GridColor(This,pVal)	\
    (This)->lpVtbl -> get_GridColor(This,pVal)

#define IGraph_put_GridColor(This,newVal)	\
    (This)->lpVtbl -> put_GridColor(This,newVal)

#define IGraph_get_showHorizontalGrid(This,pVal)	\
    (This)->lpVtbl -> get_showHorizontalGrid(This,pVal)

#define IGraph_put_showHorizontalGrid(This,newVal)	\
    (This)->lpVtbl -> put_showHorizontalGrid(This,newVal)

#define IGraph_get_showVerticalGrid(This,pVal)	\
    (This)->lpVtbl -> get_showVerticalGrid(This,pVal)

#define IGraph_put_showVerticalGrid(This,newVal)	\
    (This)->lpVtbl -> put_showVerticalGrid(This,newVal)

#define IGraph_get_VerticalGridSpacing(This,pVal)	\
    (This)->lpVtbl -> get_VerticalGridSpacing(This,pVal)

#define IGraph_put_VerticalGridSpacing(This,newVal)	\
    (This)->lpVtbl -> put_VerticalGridSpacing(This,newVal)

#define IGraph_get_showHorizontalScale(This,pVal)	\
    (This)->lpVtbl -> get_showHorizontalScale(This,pVal)

#define IGraph_put_showHorizontalScale(This,newVal)	\
    (This)->lpVtbl -> put_showHorizontalScale(This,newVal)

#define IGraph_get_MimeType(This,pVal)	\
    (This)->lpVtbl -> get_MimeType(This,pVal)

#define IGraph_get_HorizontalGridSpacing(This,pVal)	\
    (This)->lpVtbl -> get_HorizontalGridSpacing(This,pVal)

#define IGraph_put_HorizontalGridSpacing(This,newVal)	\
    (This)->lpVtbl -> put_HorizontalGridSpacing(This,newVal)

#define IGraph_get_NumValues(This,pVal)	\
    (This)->lpVtbl -> get_NumValues(This,pVal)

#define IGraph_put_NumValues(This,newVal)	\
    (This)->lpVtbl -> put_NumValues(This,newVal)

#define IGraph_get_NumGraphs(This,pVal)	\
    (This)->lpVtbl -> get_NumGraphs(This,pVal)

#define IGraph_put_NumGraphs(This,newVal)	\
    (This)->lpVtbl -> put_NumGraphs(This,newVal)

#define IGraph_get_MaxValue(This,pVal)	\
    (This)->lpVtbl -> get_MaxValue(This,pVal)

#define IGraph_put_MaxValue(This,newVal)	\
    (This)->lpVtbl -> put_MaxValue(This,newVal)

#define IGraph_get_showExplanation(This,pVal)	\
    (This)->lpVtbl -> get_showExplanation(This,pVal)

#define IGraph_put_showExplanation(This,newVal)	\
    (This)->lpVtbl -> put_showExplanation(This,newVal)

#define IGraph_get_GraphColor(This,GraphNumber,pVal)	\
    (This)->lpVtbl -> get_GraphColor(This,GraphNumber,pVal)

#define IGraph_put_GraphColor(This,GraphNumber,newVal)	\
    (This)->lpVtbl -> put_GraphColor(This,GraphNumber,newVal)

#define IGraph_get_GraphName(This,GraphNumber,pVal)	\
    (This)->lpVtbl -> get_GraphName(This,GraphNumber,pVal)

#define IGraph_put_GraphName(This,GraphNumber,newVal)	\
    (This)->lpVtbl -> put_GraphName(This,GraphNumber,newVal)

#define IGraph_get_Value(This,GraphNum,Index,pVal)	\
    (This)->lpVtbl -> get_Value(This,GraphNum,Index,pVal)

#define IGraph_put_Value(This,GraphNum,Index,newVal)	\
    (This)->lpVtbl -> put_Value(This,GraphNum,Index,newVal)

#define IGraph_get_showVerticalScale(This,pVal)	\
    (This)->lpVtbl -> get_showVerticalScale(This,pVal)

#define IGraph_put_showVerticalScale(This,newVal)	\
    (This)->lpVtbl -> put_showVerticalScale(This,newVal)

#define IGraph_get_scaleFormat(This,pVal)	\
    (This)->lpVtbl -> get_scaleFormat(This,pVal)

#define IGraph_put_scaleFormat(This,newVal)	\
    (This)->lpVtbl -> put_scaleFormat(This,newVal)

#define IGraph_get_ValueName(This,Index,pVal)	\
    (This)->lpVtbl -> get_ValueName(This,Index,pVal)

#define IGraph_put_ValueName(This,Index,newVal)	\
    (This)->lpVtbl -> put_ValueName(This,Index,newVal)

#define IGraph_get_WorksheetColor(This,pVal)	\
    (This)->lpVtbl -> get_WorksheetColor(This,pVal)

#define IGraph_put_WorksheetColor(This,newVal)	\
    (This)->lpVtbl -> put_WorksheetColor(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_Width_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_Width_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IGraph_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_Height_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_Height_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IGraph_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_GraphTitle_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_GraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_GraphTitle_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IGraph_put_GraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_showGraphTitle_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_showGraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_showGraphTitle_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IGraph_put_showGraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGraph_Reset_Proxy( 
    IGraph __RPC_FAR * This);


void __RPC_STUB IGraph_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_Picture_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_Picture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_TextColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_TextColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_TextColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IGraph_put_TextColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_BackgroundColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_BackgroundColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_BackgroundColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IGraph_put_BackgroundColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_GridColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_GridColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_GridColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IGraph_put_GridColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_showHorizontalGrid_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_showHorizontalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_showHorizontalGrid_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IGraph_put_showHorizontalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_showVerticalGrid_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_showVerticalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_showVerticalGrid_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IGraph_put_showVerticalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_VerticalGridSpacing_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_VerticalGridSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_VerticalGridSpacing_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IGraph_put_VerticalGridSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_showHorizontalScale_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_showHorizontalScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_showHorizontalScale_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IGraph_put_showHorizontalScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_MimeType_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_MimeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_HorizontalGridSpacing_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_HorizontalGridSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_HorizontalGridSpacing_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IGraph_put_HorizontalGridSpacing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_NumValues_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_NumValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_NumValues_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IGraph_put_NumValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_NumGraphs_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_NumGraphs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_NumGraphs_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IGraph_put_NumGraphs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_MaxValue_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_MaxValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_MaxValue_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IGraph_put_MaxValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_showExplanation_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_showExplanation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_showExplanation_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IGraph_put_showExplanation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_GraphColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ long GraphNumber,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_GraphColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_GraphColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ long GraphNumber,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IGraph_put_GraphColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_GraphName_Proxy( 
    IGraph __RPC_FAR * This,
    long GraphNumber,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_GraphName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_GraphName_Proxy( 
    IGraph __RPC_FAR * This,
    long GraphNumber,
    /* [in] */ BSTR newVal);


void __RPC_STUB IGraph_put_GraphName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_Value_Proxy( 
    IGraph __RPC_FAR * This,
    long GraphNum,
    long Index,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_Value_Proxy( 
    IGraph __RPC_FAR * This,
    long GraphNum,
    long Index,
    /* [in] */ double newVal);


void __RPC_STUB IGraph_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_showVerticalScale_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BOOL __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_showVerticalScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_showVerticalScale_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BOOL newVal);


void __RPC_STUB IGraph_put_showVerticalScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_scaleFormat_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_scaleFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_scaleFormat_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IGraph_put_scaleFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_ValueName_Proxy( 
    IGraph __RPC_FAR * This,
    long Index,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_ValueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_ValueName_Proxy( 
    IGraph __RPC_FAR * This,
    long Index,
    /* [in] */ BSTR newVal);


void __RPC_STUB IGraph_put_ValueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGraph_get_WorksheetColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [retval][out] */ OLE_COLOR __RPC_FAR *pVal);


void __RPC_STUB IGraph_get_WorksheetColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGraph_put_WorksheetColor_Proxy( 
    IGraph __RPC_FAR * This,
    /* [in] */ OLE_COLOR newVal);


void __RPC_STUB IGraph_put_WorksheetColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGraph_INTERFACE_DEFINED__ */



#ifndef __QGRAPHLib_LIBRARY_DEFINED__
#define __QGRAPHLib_LIBRARY_DEFINED__

/* library QGRAPHLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_QGRAPHLib;

EXTERN_C const CLSID CLSID_Histogram;

#ifdef __cplusplus

class DECLSPEC_UUID("36E7035C-E313-4E93-BF4A-43C3E07F288D")
Histogram;
#endif

EXTERN_C const CLSID CLSID_Graph;

#ifdef __cplusplus

class DECLSPEC_UUID("3A4724BB-729D-45BA-9743-B058CF95D1A1")
Graph;
#endif
#endif /* __QGRAPHLib_LIBRARY_DEFINED__ */

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
