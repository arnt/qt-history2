/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_win.cpp#7 $
**
** WM_FILES implementation for Qt.
**
** Created : 980320
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapplication.h"
#include "qwidget.h"
#include "qdragobject.h"

#include <windows.h>
#include <ole2.h>          


void QDragManager::cancel()
{
    debug( "c " );
    if ( object ) {
	beingCancelled = TRUE;
	if ( object->autoDelete() )
	    delete object;
	object = 0;
    }

    // insert cancel code here

    if ( restoreCursor ) {
	QApplication::restoreOverrideCursor();
	restoreCursor = FALSE;
    }
}


void QDragManager::move( const QPoint & globalPos )
{
    // tbi
}


void QDragManager::drop()
{
    // tbi
}



const char * QDragMoveEvent::format( int  )
{
    return 0;
}


QByteArray qt_olednd_obtain_data( const char *format )
{
    QByteArray tmp;

    fmtetc.cfFormat = CF_TEXT;
    fmtetc.ptd = NULL;
    fmtetc.dwAspect = DVASPECT_CONTENT;  
    fmtetc.lindex = -1;
    fmtetc.tymed = TYMED_HGLOBAL;       
    
    // User has dropped on us. Get the CF_TEXT data from drag source
    HRESULT hr = pDataObj->GetData(&fmtetc, &medium);
    if (!FAILED(hr)) {
	// Display the data and release it.
	hText = medium.hGlobal;
	pszText = (LPSTR)GlobalLock(hText);
	tmp.resize(medium.hLength);
	memcpy(tmp.data(),pszText,medium.hLength);
	GlobalUnlock(hText);
	ReleaseStgMedium(&medium);
    }
    return tmp;
}

QByteArray QDragMoveEvent::data( const char * format )
{
    return qt_olednd_obtain_data( format );
}

QByteArray QDropEvent::data( const char * format )
{
    return qt_olednd_obtain_data( format );
}


void QDragManager::startDrag( QDragObject * o )
{
    if ( object == o ) {
	debug( "meaningless" );
	return;
    }

    if ( object ) {
	cancel();
	dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    object = o;
    dragSource = (QWidget *)(object->parent());

    DWORD dwEffect;
    QOleDropSource *src = new QOleDropSource(dragSource);
    QOleDataObject *obj = new QOleDataObject(o);
    // This drag source only allows copying of data.
    // Move and link is not allowed.
    DoDragDrop(obj, src, DROPEFFECT_COPY, &dwEffect);     
    obj->Release();
}

void qt_olednd_unregister( QWidget* widget, QOleDropTarget *dst )
{
    RevokeDragDrop(widget->winId());
    dst->widget = 0;
    dst->Release();
    CoLockObjectExternal(dst, FALSE, TRUE);
}

QOleDropTarget* qt_olednd_register( QWidget* widget )
{
    QOleDropTarget* dst = new QOleDropTarget( this );
    CoLockObjectExternal(dst, TRUE, TRUE);
    RegisterDragDrop(widget->winId(), dst);
    return dst;
}


/***********************************************************
 * Standard implementation of IEnumFormatEtc. This code
 * is from \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
 ***********************************************************/
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize);
STDAPI_(void) OleStdFree(LPVOID pmem);
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc);
STDAPI_(LPENUMFORMATETC)
  OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc);

#define STRICT  1


typedef struct tagOleStdEnumFmtEtc {
  IEnumFORMATETCVtbl FAR* lpVtbl;
  ULONG m_dwRefs;       /* referance count */
  ULONG m_nIndex;       /* current index in list */
  ULONG m_nCount;       /* how many items in list */
  LPFORMATETC m_lpEtc;  /* list of formatetc */
} OLESTDENUMFMTETC, FAR* LPOLESTDENUMFMTETC;

VOID  OleStdEnumFmtEtc_Destroy(LPOLESTDENUMFMTETC pEtc);

STDMETHODIMP OleStdEnumFmtEtc_QueryInterface(
        LPENUMFORMATETC lpThis, REFIID riid, LPVOID FAR* ppobj);
STDMETHODIMP_(ULONG)  OleStdEnumFmtEtc_AddRef(LPENUMFORMATETC lpThis);
STDMETHODIMP_(ULONG)  OleStdEnumFmtEtc_Release(LPENUMFORMATETC lpThis);
STDMETHODIMP  OleStdEnumFmtEtc_Next(LPENUMFORMATETC lpThis, ULONG celt,
                                  LPFORMATETC rgelt, ULONG FAR* pceltFetched);
STDMETHODIMP  OleStdEnumFmtEtc_Skip(LPENUMFORMATETC lpThis, ULONG celt);
STDMETHODIMP  OleStdEnumFmtEtc_Reset(LPENUMFORMATETC lpThis);
STDMETHODIMP  OleStdEnumFmtEtc_Clone(LPENUMFORMATETC lpThis,
                                     LPENUMFORMATETC FAR* ppenum);

static IEnumFORMATETCVtbl g_EnumFORMATETCVtbl = {
        OleStdEnumFmtEtc_QueryInterface,
        OleStdEnumFmtEtc_AddRef,
        OleStdEnumFmtEtc_Release,
        OleStdEnumFmtEtc_Next,
        OleStdEnumFmtEtc_Skip,
        OleStdEnumFmtEtc_Reset,
        OleStdEnumFmtEtc_Clone,
};

/////////////////////////////////////////////////////////////////////////////


STDAPI_(LPENUMFORMATETC)
  OleStdEnumFmtEtc_Create(ULONG nCount, LPFORMATETC lpEtc)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPMALLOC lpMalloc=NULL;
  LPOLESTDENUMFMTETC lpEF=NULL;
  DWORD dwSize;
  WORD i;
  HRESULT hRes;

  hRes = CoGetMalloc(MEMCTX_TASK, &lpMalloc);
  if (hRes != NOERROR) {
    return NULL;
  }

  lpEF = (LPOLESTDENUMFMTETC)lpMalloc->lpVtbl->Alloc(lpMalloc,
                                                 sizeof(OLESTDENUMFMTETC));
  if (lpEF == NULL) {
    goto errReturn;
  }

  lpEF->lpVtbl = &g_EnumFORMATETCVtbl;
  lpEF->m_dwRefs = 1;
  lpEF->m_nCount = nCount;
  lpEF->m_nIndex = 0;

  dwSize = sizeof(FORMATETC) * lpEF->m_nCount;

  lpEF->m_lpEtc = (LPFORMATETC)lpMalloc->lpVtbl->Alloc(lpMalloc, dwSize);
  if (lpEF->m_lpEtc == NULL)
    goto errReturn;

  lpMalloc->lpVtbl->Release(lpMalloc);

  for (i=0; i<nCount; i++) {
    OleStdCopyFormatEtc(
            (LPFORMATETC)&(lpEF->m_lpEtc[i]), (LPFORMATETC)&(lpEtc[i]));
  }

  return (LPENUMFORMATETC)lpEF;

errReturn:
  if (lpEF != NULL)
    lpMalloc->lpVtbl->Free(lpMalloc, lpEF);

  if (lpMalloc != NULL)
    lpMalloc->lpVtbl->Release(lpMalloc);

  return NULL;

} /* OleStdEnumFmtEtc_Create()
   */


VOID
  OleStdEnumFmtEtc_Destroy(LPOLESTDENUMFMTETC lpEF)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
    LPMALLOC lpMalloc=NULL;
    WORD i;

    if (lpEF != NULL) {

        if (CoGetMalloc(MEMCTX_TASK, &lpMalloc) == NOERROR) {

            /* OLE2NOTE: we MUST free any memory that was allocated for
            **    TARGETDEVICES contained within the FORMATETC elements.
            */
            for (i=0; i<lpEF->m_nCount; i++) {
                OleStdFree(lpEF->m_lpEtc[i].ptd);
            }

            if (lpEF->m_lpEtc != NULL) {
                lpMalloc->lpVtbl->Free(lpMalloc, lpEF->m_lpEtc);
            }

            lpMalloc->lpVtbl->Free(lpMalloc, lpEF);
            lpMalloc->lpVtbl->Release(lpMalloc);
        }
    }

} /* OleStdEnumFmtEtc_Destroy()
   */


STDMETHODIMP
  OleStdEnumFmtEtc_QueryInterface(
                LPENUMFORMATETC lpThis, REFIID riid, LPVOID FAR* ppobj)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  *ppobj = NULL;

  if (IsEqualIID(riid,&IID_IUnknown) || IsEqualIID(riid,&IID_IEnumFORMATETC)){
    *ppobj = (LPVOID)lpEF;
  }

  if (*ppobj == NULL) return ResultFromScode(E_NOINTERFACE);
  else{
    OleStdEnumFmtEtc_AddRef(lpThis);
    return NOERROR;
  }

} /* OleStdEnumFmtEtc_QueryInterface()
   */


STDMETHODIMP_(ULONG)
  OleStdEnumFmtEtc_AddRef(LPENUMFORMATETC lpThis)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  return lpEF->m_dwRefs++;

} /* OleStdEnumFmtEtc_AddRef()
   */


STDMETHODIMP_(ULONG)
  OleStdEnumFmtEtc_Release(LPENUMFORMATETC lpThis)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  DWORD dwRefs = --lpEF->m_dwRefs;

  if (dwRefs == 0)
    OleStdEnumFmtEtc_Destroy(lpEF);

  return dwRefs;

} /* OleStdEnumFmtEtc_Release()
   */


STDMETHODIMP
  OleStdEnumFmtEtc_Next(LPENUMFORMATETC lpThis, ULONG celt, LPFORMATETC rgelt,
                      ULONG FAR* pceltFetched)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  ULONG i=0;
  ULONG nOffset;

  if (rgelt == NULL) {
    return ResultFromScode(E_INVALIDARG);
  }

  while (i < celt) {
    nOffset = lpEF->m_nIndex + i;

    if (nOffset < lpEF->m_nCount) {
      OleStdCopyFormatEtc(
            (LPFORMATETC)&(rgelt[i]), (LPFORMATETC)&(lpEF->m_lpEtc[nOffset]));
      i++;
    }else{
      break;
    }
  }

  lpEF->m_nIndex += (WORD)i;

  if (pceltFetched != NULL) {
    *pceltFetched = i;
  }

  if (i != celt) {
    return ResultFromScode(S_FALSE);
  }

  return NOERROR;
} /* OleStdEnumFmtEtc_Next()
   */


STDMETHODIMP
  OleStdEnumFmtEtc_Skip(LPENUMFORMATETC lpThis, ULONG celt)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  ULONG i=0;
  ULONG nOffset;

  while (i < celt) {
    nOffset = lpEF->m_nIndex + i;

    if (nOffset < lpEF->m_nCount) {
      i++;
    }else{
      break;
    }
  }

  lpEF->m_nIndex += (WORD)i;

  if (i != celt) {
    return ResultFromScode(S_FALSE);
  }

  return NOERROR;
} /* OleStdEnumFmtEtc_Skip()
   */


STDMETHODIMP
  OleStdEnumFmtEtc_Reset(LPENUMFORMATETC lpThis)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;
  lpEF->m_nIndex = 0;

  return NOERROR;
} /* OleStdEnumFmtEtc_Reset()
   */


STDMETHODIMP
  OleStdEnumFmtEtc_Clone(LPENUMFORMATETC lpThis, LPENUMFORMATETC FAR* ppenum)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
{
  LPOLESTDENUMFMTETC lpEF = (LPOLESTDENUMFMTETC)lpThis;

  if (ppenum == NULL) {
    return ResultFromScode(E_INVALIDARG);
  }

  *ppenum = OleStdEnumFmtEtc_Create(lpEF->m_nCount, lpEF->m_lpEtc);

  // make sure cloned enumerator has same index state as the original
  if (*ppenum) {
      LPOLESTDENUMFMTETC lpEFClone = (LPOLESTDENUMFMTETC)*ppenum;
      lpEFClone->m_nIndex = lpEF->m_nIndex;
      return NOERROR;
  } else
      return ResultFromScode(E_OUTOFMEMORY);

} /* OleStdEnumFmtEtc_Clone()
   */
   
/*
 * OleStdCopyTargetDevice()
 *
 * Purpose:
 *  duplicate a TARGETDEVICE struct. this function allocates memory for
 *  the copy. the caller MUST free the allocated copy when done with it
 *  using the standard allocator returned from CoGetMalloc.
 *  (OleStdFree can be used to free the copy).
 *
 * Parameters:
 *  ptdSrc      pointer to source TARGETDEVICE
 *
 * Return Value:
 *    pointer to allocated copy of ptdSrc
 *    if ptdSrc==NULL then retuns NULL is returned.
 *    if ptdSrc!=NULL and memory allocation fails, then NULL is returned
 */
STDAPI_(DVTARGETDEVICE FAR*) OleStdCopyTargetDevice(DVTARGETDEVICE FAR* ptdSrc)
{
  DVTARGETDEVICE FAR* ptdDest = NULL;

  if (ptdSrc == NULL) {
    return NULL;
  }

  if ((ptdDest = (DVTARGETDEVICE FAR*)OleStdMalloc(ptdSrc->tdSize)) != NULL) {
    _fmemcpy(ptdDest, ptdSrc, (size_t)ptdSrc->tdSize);
  }

  return ptdDest;
}


/*
 * OleStdCopyFormatEtc()
 *
 * Purpose:
 *  Copies the contents of a FORMATETC structure. this function takes
 *  special care to copy correctly copying the pointer to the TARGETDEVICE
 *  contained within the source FORMATETC structure.
 *  if the source FORMATETC has a non-NULL TARGETDEVICE, then a copy
 *  of the TARGETDEVICE will be allocated for the destination of the
 *  FORMATETC (petcDest).
 *
 *  OLE2NOTE: the caller MUST free the allocated copy of the TARGETDEVICE
 *  within the destination FORMATETC when done with it
 *  using the standard allocator returned from CoGetMalloc.
 *  (OleStdFree can be used to free the copy).
 *
 * Parameters:
 *  petcDest      pointer to destination FORMATETC
 *  petcSrc       pointer to source FORMATETC
 *
 * Return Value:
 *    returns TRUE is copy is successful; retuns FALSE if not successful
 */
STDAPI_(BOOL) OleStdCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc)
{
  if ((petcDest == NULL) || (petcSrc == NULL)) {
    return FALSE;
  }

  petcDest->cfFormat = petcSrc->cfFormat;
  petcDest->ptd      = OleStdCopyTargetDevice(petcSrc->ptd);
  petcDest->dwAspect = petcSrc->dwAspect;
  petcDest->lindex   = petcSrc->lindex;
  petcDest->tymed    = petcSrc->tymed;

  return TRUE;

}

/* OleStdFree
** ----------
**    free memory using the currently active IMalloc* allocator
*/
STDAPI_(void) OleStdFree(LPVOID pmem)
{
    LPMALLOC pmalloc;

    if (pmem == NULL)
        return;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
        return;
    }

    pmalloc->lpVtbl->Free(pmalloc, pmem);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }
}

/* OleStdMalloc
** ------------
**    allocate memory using the currently active IMalloc* allocator
*/
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize)
{
    LPVOID pout;
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
        return NULL;
    }

    pout = (LPVOID)pmalloc->lpVtbl->Alloc(pmalloc, ulSize);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }

    return pout;
}

/***********************************************************
 * End of Standard implementation of IEnumFormatEtc.
 ***********************************************************/



class QOleDropSource : public IDropSource
{
    QWidget* src;
public:
    QOleDropSource( QWidget* w ) :
	src(w)
    {
    }

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    /* IDropSource methods */
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);
 
private:
    ULONG m_refs;     
};  


class QOleDataObject : public IDataObject
{
public:
    QOleDataObject( const QByteArray data, const char* fmt );
   
    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    /* IDataObject methods */    
    STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium );
    STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium );
    STDMETHOD(QueryGetData)(LPFORMATETC pformatetc );
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
    STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                       BOOL fRelease);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf, 
                      LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);
    
private:
    ULONG m_refs;   
    QString format;
    const QByteArray data;
};
   
   

class QOleDropTarget : public IDropTarget
{
public:    
    QWidget* widget;

    QOleDropTarget( QWidget* w );

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDropTarget methods */
    STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect); 
    
    /* Utility function to read type of drag from key state*/
    STDMETHOD_(BOOL, QueryDrop)(DWORD grfKeyState, LPDWORD pdwEffect);
 
private:
    ULONG m_refs;  
    BOOL m_bAcceptFmt;
};

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDropSource::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDropSource)
    {
      *ppv = this;
      ++m_refs;
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


ULONG
QOleDropSource::AddRef(void)
{
    return ++m_refs;
}


ULONG
QOleDropSource::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}  

//---------------------------------------------------------------------
//                    IDropSource Methods
//---------------------------------------------------------------------  

STDMETHODIMP
QOleDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{  
     if (fEscapePressed)
        return ResultFromScode(DRAGDROP_S_CANCEL);
    else if (!(grfKeyState & MK_LBUTTON))
        return ResultFromScode(DRAGDROP_S_DROP);
    else
        return NOERROR;                  
}

STDMETHODIMP
QOleDropSource::GiveFeedback(DWORD dwEffect)
{
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}


//---------------------------------------------------------------------
//                    QOleDataObject Constructor
//---------------------------------------------------------------------        

QOleDataObject::QOleDataObject( const QByteArray d, const char* fmt )
{
    m_refs = 1;    
    data = d;
    format = fmt;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDataObject)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


ULONG
QOleDataObject::AddRef(void)
{
    return ++m_refs;
}


ULONG
QOleDataObject::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}  

//---------------------------------------------------------------------
//                    IDataObject Methods    
//  
// The following methods are NOT supported for data transfer using the
// clipboard or drag-drop: 
//
//      IDataObject::SetData    -- return E_NOTIMPL
//      IDataObject::DAdvise    -- return OLE_E_ADVISENOTSUPPORTED
//                 ::DUnadvise
//                 ::EnumDAdvise
//      IDataObject::GetCanonicalFormatEtc -- return E_NOTIMPL
//                     (NOTE: must set pformatetcOut->ptd = NULL)
//---------------------------------------------------------------------  

STDMETHODIMP 
QOleDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium) 
{   
    HGLOBAL hText; 
    LPSTR pszText;
    
    pmedium->tymed = NULL;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = NULL;
    
    // This method is called by the drag-drop target to obtain the text
    // that is being dragged.
    if (pformatetc->cfFormat == CF_TEXT &&
       pformatetc->dwAspect == DVASPECT_CONTENT &&
       pformatetc->tymed == TYMED_HGLOBAL)
    {
        hText = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, data.size());    
        if (!hText)
            return ResultFromScode(E_OUTOFMEMORY);
        pszText = (LPSTR)GlobalLock(hText);
        lstrcpy(pszText, data.data());
        GlobalUnlock(hText);
        
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hText; 
 
        return ResultFromScode(S_OK);
    }
    return ResultFromScode(DATA_E_FORMATETC);
}
   
STDMETHODIMP 
QOleDataObject::GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)  
{
    return ResultFromScode(DATA_E_FORMATETC);    
}     

STDMETHODIMP 
QOleDataObject::QueryGetData(LPFORMATETC pformatetc) 
{   
    // This method is called by the drop target to check whether the source
    // provides data is a format that the target accepts.
    if (pformatetc->cfFormat == CF_TEXT 
        && pformatetc->dwAspect == DVASPECT_CONTENT
        && pformatetc->tymed & TYMED_HGLOBAL)
        return ResultFromScode(S_OK); 
    else return ResultFromScode(S_FALSE);
}

STDMETHODIMP 
QOleDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{ 
    pformatetcOut->ptd = NULL; 
    return ResultFromScode(E_NOTIMPL);
}        

STDMETHODIMP 
QOleDataObject::SetData(LPFORMATETC pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{   
    // A data transfer object that is used to transfer data
    //    (either via the clipboard or drag/drop does NOT
    //    accept SetData on ANY format.
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP 
QOleDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
{ 
    // A standard implementation is provided by OleStdEnumFmtEtc_Create
    // which can be found in \ole2\samp\ole2ui\enumfetc.c in the OLE 2 SDK.
    // This code from ole2ui is copied to the enumfetc.c file in this sample.
    
    SCODE sc = S_OK;
    FORMATETC fmtetc;
    
    fmtetc.cfFormat = CF_TEXT;
    fmtetc.dwAspect = DVASPECT_CONTENT;
    fmtetc.tymed = TYMED_HGLOBAL;
    fmtetc.ptd = NULL;
    fmtetc.lindex = -1;

    if (dwDirection == DATADIR_GET){
        *ppenumFormatEtc = OleStdEnumFmtEtc_Create(1, &fmtetc);
        if (*ppenumFormatEtc == NULL)
            sc = E_OUTOFMEMORY;

    } else if (dwDirection == DATADIR_SET){
        // A data transfer object that is used to transfer data
        //    (either via the clipboard or drag/drop does NOT
        //    accept SetData on ANY format.
        sc = E_NOTIMPL;
        goto error;
    } else {
        sc = E_INVALIDARG;
        goto error;
    }

error:
    return ResultFromScode(sc);
}

STDMETHODIMP 
QOleDataObject::DAdvise(FORMATETC FAR* pFormatetc, DWORD advf, 
                       LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
   

STDMETHODIMP 
QOleDataObject::DUnadvise(DWORD dwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP 
QOleDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}


         
          
         
QOleDropTarget::QOleDropTarget( QWidget* w ) :
    widget(w)
{
   m_refs = 1; 
   m_bAcceptFmt = FALSE;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
QOleDropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
{
    if(iid == IID_IUnknown || iid == IID_IDropTarget)
    {
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
QOleDropTarget::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
QOleDropTarget::Release(void)
{
    if(--m_refs == 0)
    {
      delete this;
      return 0;
    }
    return m_refs;
}  

//---------------------------------------------------------------------
//                    IDropTarget Methods
//---------------------------------------------------------------------  

STDMETHODIMP
QOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{  
    FORMATETC fmtetc;
       
    fmtetc.cfFormat = CF_TEXT;
    fmtetc.ptd      = NULL;
    fmtetc.dwAspect = DVASPECT_CONTENT;  
    fmtetc.lindex   = -1;
    fmtetc.tymed    = TYMED_HGLOBAL; 
    
    // Does the drag source provide CF_TEXT, which is the only format we accept.    
    m_bAcceptFmt = (NOERROR == pDataObj->QueryGetData(&fmtetc)) ? TRUE : FALSE;    
    
    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::DragLeave()
{   
    m_bAcceptFmt = FALSE;   
    return NOERROR;
}

STDMETHODIMP
QOleDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)  
{   
    FORMATETC fmtetc;   
    STGMEDIUM medium;   
    HGLOBAL hText;
    LPSTR pszText;
    HRESULT hr;
     
    if (QueryDrop(grfKeyState, pdwEffect))
    {      
        fmtetc.cfFormat = CF_TEXT;
        fmtetc.ptd = NULL;
        fmtetc.dwAspect = DVASPECT_CONTENT;  
        fmtetc.lindex = -1;
        fmtetc.tymed = TYMED_HGLOBAL;       
        
        // User has dropped on us. Get the CF_TEXT data from drag source
        hr = pDataObj->GetData(&fmtetc, &medium);
        if (FAILED(hr))
            goto error; 
        
        // Display the data and release it.
        hText = medium.hGlobal;
        pszText = (LPSTR)GlobalLock(hText);

	QDropEvent de( QPoint(0,0)/*###*/ );
	QApplication::sendEvent( widget, &de );

        GlobalUnlock(hText);
        ReleaseStgMedium(&medium);
    }
    return NOERROR;      
    
error:
    *pdwEffect = DROPEFFECT_NONE;
    return hr; 
}   

/* OleStdGetDropEffect
** -------------------
**
** Convert a keyboard state into a DROPEFFECT.
**
** returns the DROPEFFECT value derived from the key state.
**    the following is the standard interpretation:
**          no modifier -- Default Drop     (0 is returned)
**          CTRL        -- DROPEFFECT_COPY
**          SHIFT       -- DROPEFFECT_MOVE
**          CTRL-SHIFT  -- DROPEFFECT_LINK
**
**    Default Drop: this depends on the type of the target application.
**    this is re-interpretable by each target application. a typical
**    interpretation is if the drag is local to the same document
**    (which is source of the drag) then a MOVE operation is
**    performed. if the drag is not local, then a COPY operation is
**    performed.
*/
#define OleStdGetDropEffect(grfKeyState)    \
    ( (grfKeyState & MK_CONTROL) ?          \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) :  \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 ) )

//---------------------------------------------------------------------
// QOleDropTarget::QueryDrop: Given key state, determines the type of 
// acceptable drag and returns the a dwEffect. 
//---------------------------------------------------------------------   
STDMETHODIMP_(BOOL)
QOleDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{  
    DWORD dwOKEffects = *pdwEffect; 
    
    if (!m_bAcceptFmt)
        goto dropeffect_none; 
     
    *pdwEffect = OleStdGetDropEffect(grfKeyState);
    if (*pdwEffect == 0) {
        // No modifier keys used by user while dragging. Try in order: MOVE, COPY.
        if (DROPEFFECT_MOVE & dwOKEffects)
            *pdwEffect = DROPEFFECT_MOVE;
        else if (DROPEFFECT_COPY & dwOKEffects)
            *pdwEffect = DROPEFFECT_COPY; 
        else goto dropeffect_none;   
    } 
    else {
        // Check if the drag source application allows the drop effect desired by user.
        // The drag source specifies this in DoDragDrop
        if (!(*pdwEffect & dwOKEffects))
            goto dropeffect_none; 
        // We don't accept links
        if (*pdwEffect == DROPEFFECT_LINK)
            goto dropeffect_none; 
    }  
    return TRUE;

dropeffect_none:
    *pdwEffect = DROPEFFECT_NONE;
    return FALSE;
}   
    
                                                                             
                                                                           
