//*-------------------------------------------------------------------------
//| Title:
//|    drgdrpt.cpp
//|
//| Purpose:
//|      Demonstrates implementing an OLE drag-drop target. This drag-drop target
//|      accepts data of format CF_TEXT dragged from drag-drop sources.  
//|
//| Written by Microsoft Product Support Services, Windows Developer Support
//| (c) Copyright Microsoft Corp. 1994 All Rights Reserved                                                    
//|
//*-------------------------------------------------------------------------

#define STRICT
#include <windows.h>
#include <ole2.h>          
#include "drgdrpt.h"          

// Globals
HINSTANCE ghInst;               // Instance of application
HWND  ghwnd;                    // Toplevel window handle

char szTitle[STR_LEN];


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
   LoadString(hInstance, IDS_PROGNAME, szTitle, STR_LEN);
   
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
//****************************************************************************

BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
  
   ghInst = hInstance;
   // Create Main Window
   ghwnd = CreateWindow("MainWndClass", szTitle,
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
// Comments:
//
//****************************************************************************
LRESULT CALLBACK __export MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{  
   static LPDROPTARGET pDropTarget;    
   static HWND hwndEdit;
   
   switch (msg)
   { 
      case WM_CREATE:    
         // Create IDropTarget implementation and register the window as a drop target.
         pDropTarget = (LPDROPTARGET) new CDropTarget; 
         // The IDropTarget interface is marshaled by OLE using MSHLFLAGS_TABLEWEAK.
         // (See the MSHLFLAGS enumeration documentation for details). This means 
         // that OLE does not keep a strong lock on IDropTarget. This will result
         // in the interface being released after the first drop. To prevent this
         // from happening we keep a strong lock on IDropTarget by calling 
         // CoLockObjectExternal.           
         CoLockObjectExternal(pDropTarget, TRUE, TRUE);
         RegisterDragDrop(hwnd, pDropTarget);    
         
         hwndEdit = CreateWindow("Edit", "",
                       WS_CHILD | WS_VISIBLE | WS_BORDER,
                       10, 10, 300, 50,
                       hwnd, (HMENU)ID_EDIT, ghInst, NULL);
         return 0L;      
         
      case WM_CLOSE:   
          // Revoke the window as a drop target and release the strong lock
          // using CoLockObjectExternal.          
          RevokeDragDrop(hwnd);
          pDropTarget->Release();  
          CoLockObjectExternal(pDropTarget, FALSE, TRUE);
          return DefWindowProc(hwnd, msg, wParam, lParam);
          
         
      case WM_DESTROY:                
         PostQuitMessage(0);
         break;

      default:                         
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
   
   return NULL;
}

CDropTarget::CDropTarget()
{
   m_refs = 1; 
   m_bAcceptFmt = FALSE;
}   

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------


STDMETHODIMP
CDropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppv) 
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
CDropTarget::AddRef(void)
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
CDropTarget::Release(void)
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
CDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
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
CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    QueryDrop(grfKeyState, pdwEffect);
    return NOERROR;
}

STDMETHODIMP
CDropTarget::DragLeave()
{   
    m_bAcceptFmt = FALSE;   
    return NOERROR;
}

STDMETHODIMP
CDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)  
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
        SetDlgItemText(ghwnd, ID_EDIT, pszText);
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
// CDropTarget::QueryDrop: Given key state, determines the type of 
// acceptable drag and returns the a dwEffect. 
//---------------------------------------------------------------------   
STDMETHODIMP_(BOOL)
CDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
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
    
                                                                             
                                                                           
