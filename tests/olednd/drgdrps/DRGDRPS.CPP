//*-------------------------------------------------------------------------
//| Title:
//|    drgdrps.cpp
//|
//| Purpose:
//|      Demonstrates implementing an OLE drag-drop source. This drag source
//|      allows data of format CF_TEXT to be dragged to drag-drop targets.  
//|
//| Written by Microsoft Product Support Services, Windows Developer Support
//| (c) Copyright Microsoft Corp. 1994 All Rights Reserved                                                    
//|
//*-------------------------------------------------------------------------

#define STRICT
#include <windows.h>
#include <ole2.h>          
#include "drgdrps.h"
#include "enumfetc.h"          

// Globals
HINSTANCE ghInst;               // Instance of application
HWND  ghwnd;                    // Toplevel window handle

char gszTitle[STR_LEN];


//***********************************************************************
// Function: WinMain
//
// Purpose: Called by Windows on app startup.  Initializes everything,
//          and enters a message loop.
//
// Parameters:
//    hInstance     == Handle to this instance.
//    hPrevInstance == Handle to last instance of app.
//    lpCmdLine     == Command Line passed into app.
//    nCmdShow      == How app should come up (i.e. minimized/normal)
//
// Returns: Return value from PostQuitMessage.
//****************************************************************************
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   MSG msg; 
   
   //  It is recommended that all OLE applications set    
   //  their message queue size to 96. This improves the capacity
   //  and performance of OLE's LRPC mechanism.
   int cMsg = 96;                  // Recommend msg queue size for OLE
   while (cMsg && !SetMessageQueue(cMsg))  // take largest size we can get.
       cMsg -= 8;
   if (!cMsg)
       return -1;                  // ERROR: we got no message queue
   LoadString(hInstance, IDS_PROGNAME, gszTitle, STR_LEN);
   
   if (!hPrevInstance)
      if (!InitApplication(hInstance))
         return (FALSE); 
   
   if (NOERROR != OleInitialize(NULL))
       return FALSE; 

   if (!InitInstance(hInstance, nCmdShow))
      return (FALSE);

   while (GetMessage(&msg, NULL, NULL, NULL))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }  
   
   OleUninitialize();
   
   return (msg.wParam); // Returns the value from PostQuitMessage
}


//****************************************************************************
// Function: InitApplication
//
// Purpose: Called by WinMain on first instance of app.  Registers
//          the window class.
//
// Parameters:
//    hInstance == Handle to this instance.
//
// Returns: TRUE on success, FALSE otherwise.
//
//****************************************************************************

BOOL InitApplication (HINSTANCE hInstance)
{
   WNDCLASS wc;

   wc.style = CS_DBLCLKS;
   wc.lpfnWndProc = MainWndProc;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon(hInstance, "GenericIcon");
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wc.lpszMenuName = NULL;
   wc.lpszClassName = "MainWndClass";
     
   return RegisterClass(&wc);
 
}

//****************************************************************************
// Function: InitInstance
//
// Purpose: Called by WinMain on instance startup.  Creates and
//            displays the main, overlapped window.
//
// Parameters:
//    hInstance     == Handle to this instance.
//    nCmdShow      == How app should come up (i.e. minimized/normal)
//
// Returns: TRUE on success, FALSE otherwise.
//
// Comments:
//
//****************************************************************************

BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
  
   ghInst = hInstance;
   // Create Main Window
   ghwnd = CreateWindow("MainWndClass", gszTitle,
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       350, 200,
                       NULL, NULL, hInstance, NULL);
   if (!ghwnd)
      return FALSE;
   
   ShowWindow(ghwnd, nCmdShow);    // Show the window                        
   UpdateWindow(ghwnd);            // Sends WM_PAINT message  
   return TRUE;
}      

//****************************************************************************
// Function: MainWndProc
//
// Purpose: Message handler for main overlapped window.
//
// Parameters:
//    hwnd    == Handle to this window.
//    msg     == Message to process.
//    wParam  == Depends on message
//    lParam  == Depends on message
//
// Returns: Depends on message.
//
//****************************************************************************
LRESULT CALLBACK __export MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{  
   static LPDROPSOURCE pDropSource; 
   static HWND hwndEdit;
   
   switch (msg)
   { 
      case WM_CREATE:
          // Create IDropSource implementation.
          pDropSource = (LPDROPSOURCE) new CDropSource;
         
          hwndEdit = CreateWindow("Edit", "test",
                       WS_CHILD | WS_VISIBLE | WS_BORDER,
                       10, 10, 300, 50,
                       hwnd, (HMENU)ID_EDIT, ghInst, NULL);
          SendMessage(hwndEdit, EM_LIMITTEXT, (WPARAM)BUF_LEN-1, 0L);                  
          return 0L;     
         
      case WM_LBUTTONDOWN:      
      {
          LPDATAOBJECT pDataObject; 
          char sz[BUF_LEN];
          DWORD dwEffect;
           
          // Create IDataObject implementation and do drag-drop.
          GetDlgItemText(hwnd, ID_EDIT, sz, BUF_LEN);
          pDataObject = (LPDATAOBJECT) new CDataObject(sz);     
          // This drag source only allows copying of data. Move and link is not allowed.
          DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);     
          pDataObject->Release();
          return 0L;  
      }
         
      case WM_CLOSE:
          pDropSource->Release();
          return DefWindowProc(hwnd, msg, wParam, lParam);
                  
      case WM_DESTROY:                
         PostQuitMessage(0);
         break;

      default:                         
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
   
   return NULL;
}

//---------------------------------------------------------------------
//                    CDropSource Constructor
//---------------------------------------------------------------------        
 
CDropSource::CDropSource()
{
    m_refs = 1;  
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
CDropSource::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
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


STDMETHODIMP_(ULONG)
CDropSource::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
CDropSource::Release(void)
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
CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{  
     if (fEscapePressed)
        return ResultFromScode(DRAGDROP_S_CANCEL);
    else if (!(grfKeyState & MK_LBUTTON))
        return ResultFromScode(DRAGDROP_S_DROP);
    else
        return NOERROR;                  
}

STDMETHODIMP
CDropSource::GiveFeedback(DWORD dwEffect)
{
    return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

//---------------------------------------------------------------------
//                    CDataObject Constructor
//---------------------------------------------------------------------        

CDataObject::CDataObject(LPSTR psz)
{
   m_refs = 1;    
   lstrcpy(m_szBuffer, psz);
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
CDataObject::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
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


STDMETHODIMP_(ULONG)
CDataObject::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
CDataObject::Release(void)
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
CDataObject::GetData(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium) 
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
        hText = GlobalAlloc(GMEM_SHARE | GMEM_ZEROINIT, sizeof(m_szBuffer)+1);    
        if (!hText)
            return ResultFromScode(E_OUTOFMEMORY);
        pszText = (LPSTR)GlobalLock(hText);
        lstrcpy(pszText, m_szBuffer);
        GlobalUnlock(hText);
        
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hText; 
 
        return ResultFromScode(S_OK);
    }
    return ResultFromScode(DATA_E_FORMATETC);
}
   
STDMETHODIMP 
CDataObject::GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)  
{
    return ResultFromScode(DATA_E_FORMATETC);    
}     

STDMETHODIMP 
CDataObject::QueryGetData(LPFORMATETC pformatetc) 
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
CDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
{ 
    pformatetcOut->ptd = NULL; 
    return ResultFromScode(E_NOTIMPL);
}        

STDMETHODIMP 
CDataObject::SetData(LPFORMATETC pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{   
    // A data transfer object that is used to transfer data
    //    (either via the clipboard or drag/drop does NOT
    //    accept SetData on ANY format.
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP 
CDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc)
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
CDataObject::DAdvise(FORMATETC FAR* pFormatetc, DWORD advf, 
                       LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}
   

STDMETHODIMP 
CDataObject::DUnadvise(DWORD dwConnection)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP 
CDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{ 
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}



                                                      
