// MAX len of string table entries
#define STR_LEN   50

// String table constants
#define IDS_PROGNAME        1

// Function prototypes
int PASCAL WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
BOOL InitApplication (HINSTANCE);
BOOL InitInstance (HINSTANCE, int);
extern "C" LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);  

#define ID_EDIT 1 

class FAR CDropTarget : public IDropTarget
{
public:    
    CDropTarget();

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
