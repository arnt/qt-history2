// MAX len of string table entries
#define STR_LEN   50

// String table constants
#define IDS_PROGNAME        1

// Function prototypes
int PASCAL WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
BOOL InitApplication (HINSTANCE);
BOOL InitInstance (HINSTANCE, int);
extern "C" LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);

#define BUF_LEN 50
#define ID_EDIT 1 

class FAR CDropSource : public IDropSource
{
public:    
    CDropSource();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDropSource methods */
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);
 
private:
    ULONG m_refs;     
};  

class FAR CDataObject : public IDataObject
{
public:
    CDataObject(LPSTR psz);     
   
   /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

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
    char m_szBuffer[BUF_LEN];            // Text being transferred
};
   
   
