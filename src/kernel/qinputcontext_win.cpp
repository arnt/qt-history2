#include "qinputcontext_p.h"
#include <qfont.h>
#include <qwidget.h>
#include <qapplication.h>

//#define Q_IME_DEBUG

extern Qt::WindowsVersion qt_winver;

/* Active Input method support on Win95/98/NT */
#include <objbase.h>
#include <initguid.h>


DEFINE_GUID(IID_IActiveIMMApp, 
0x08c0e040, 0x62d1, 0x11d1, 0x93, 0x26, 0x0, 0x60, 0xb0, 0x67, 0xb8, 0x6e);



DEFINE_GUID(CLSID_CActiveIMM,
0x4955DD33, 0xB159, 0x11d0, 0x8F, 0xCF, 0x0, 0xAA, 0x00, 0x6B, 0xCC, 0x59);



DEFINE_GUID(IID_IActiveIMMMessagePumpOwner,
0xb5cf2cfa, 0x8aeb, 0x11d1, 0x93, 0x64, 0x0, 0x60, 0xb0, 0x67, 0xb8, 0x6e);



interface IEnumRegisterWordW;
interface IEnumInputContext;

#define IFMETHOD HRESULT STDMETHODCALLTYPE

interface IActiveIMMApp : public IUnknown
{
public:
    virtual IFMETHOD AssociateContext( HWND hWnd, HIMC hIME, HIMC __RPC_FAR *phPrev) = 0;
    virtual IFMETHOD dummy_ConfigureIMEA( ) = 0;
    virtual IFMETHOD ConfigureIMEW( HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDW __RPC_FAR *pData) = 0;
    virtual IFMETHOD CreateContext( HIMC __RPC_FAR *phIMC) = 0;
    virtual IFMETHOD DestroyContext( HIMC hIME) = 0;
    virtual IFMETHOD dummy_EnumRegisterWordA( ) = 0;
    virtual IFMETHOD EnumRegisterWordW( HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister, LPVOID pData, 
	IEnumRegisterWordW __RPC_FAR *__RPC_FAR *pEnum) = 0;
    virtual IFMETHOD dummy_EscapeA( ) = 0;
    virtual IFMETHOD EscapeW( HKL hKL, HIMC hIMC, UINT uEscape, LPVOID pData, LRESULT __RPC_FAR *plResult) = 0;
    virtual IFMETHOD dummy_GetCandidateListA( ) = 0;
    virtual IFMETHOD GetCandidateListW( HIMC hIMC, DWORD dwIndex, UINT uBufLen, CANDIDATELIST __RPC_FAR *pCandList, 
	UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD dummy_GetCandidateListCountA( ) = 0;
    virtual IFMETHOD GetCandidateListCountW( HIMC hIMC, DWORD __RPC_FAR *pdwListSize, DWORD __RPC_FAR *pdwBufLen) = 0;
    virtual IFMETHOD GetCandidateWindow( HIMC hIMC, DWORD dwIndex, CANDIDATEFORM __RPC_FAR *pCandidate) = 0;
    virtual IFMETHOD dummy_GetCompositionFontA( ) = 0;
    virtual IFMETHOD GetCompositionFontW( HIMC hIMC, LOGFONTW __RPC_FAR *plf) = 0;
    virtual IFMETHOD dummy_GetCompositionStringA( ) = 0;
    virtual IFMETHOD GetCompositionStringW( HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LONG __RPC_FAR *plCopied, LPVOID pBuf) = 0;
    virtual IFMETHOD GetCompositionWindow( HIMC hIMC, COMPOSITIONFORM __RPC_FAR *pCompForm) = 0;
    virtual IFMETHOD GetContext( HWND hWnd, HIMC __RPC_FAR *phIMC) = 0;
    virtual IFMETHOD dummy_GetConversionListA( ) = 0;
    virtual IFMETHOD GetConversionListW( HKL hKL, HIMC hIMC, LPWSTR pSrc, UINT uBufLen, UINT uFlag, 
	CANDIDATELIST __RPC_FAR *pDst, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD GetConversionStatus( HIMC hIMC, DWORD __RPC_FAR *pfdwConversion, DWORD __RPC_FAR *pfdwSentence) = 0;
    virtual IFMETHOD GetDefaultIMEWnd( HWND hWnd, HWND __RPC_FAR *phDefWnd) = 0;
    virtual IFMETHOD dummy_GetDescriptionA( ) = 0;
    virtual IFMETHOD GetDescriptionW( HKL hKL, UINT uBufLen, LPWSTR szDescription, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD dummy_GetGuideLineA( ) = 0;
    virtual IFMETHOD GetGuideLineW( HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LPWSTR pBuf, DWORD __RPC_FAR *pdwResult) = 0;
    virtual IFMETHOD dummy_GetIMEFileNameA( ) = 0;
    virtual IFMETHOD GetIMEFileNameW( HKL hKL, UINT uBufLen, LPWSTR szFileName, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD GetOpenStatus( HIMC hIMC) = 0;
    virtual IFMETHOD GetProperty( HKL hKL, DWORD fdwIndex, DWORD __RPC_FAR *pdwProperty) = 0;
    virtual IFMETHOD dummy_GetRegisterWordStyleA( ) = 0;
    virtual IFMETHOD GetRegisterWordStyleW( HKL hKL, UINT nItem, STYLEBUFW __RPC_FAR *pStyleBuf, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD GetStatusWindowPos( HIMC hIMC, POINT __RPC_FAR *pptPos) = 0;
    virtual IFMETHOD GetVirtualKey( HWND hWnd, UINT __RPC_FAR *puVirtualKey) = 0;
    virtual IFMETHOD dummy_InstallIMEA( ) = 0;
    virtual IFMETHOD InstallIMEW( LPWSTR szIMEFileName, LPWSTR szLayoutText, HKL __RPC_FAR *phKL) = 0;
    virtual IFMETHOD IsIME( HKL hKL) = 0;
    virtual IFMETHOD dummy_IsUIMessageA( ) = 0;
    virtual IFMETHOD IsUIMessageW( HWND hWndIME, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
    virtual IFMETHOD NotifyIME( HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue) = 0;
    virtual IFMETHOD dummy_RegisterWordA( ) = 0;
    virtual IFMETHOD RegisterWordW( HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister) = 0;
    virtual IFMETHOD ReleaseContext( HWND hWnd, HIMC hIMC) = 0;
    virtual IFMETHOD SetCandidateWindow( HIMC hIMC, CANDIDATEFORM __RPC_FAR *pCandidate) = 0;
    virtual IFMETHOD SetCompositionFontA( HIMC hIMC, LOGFONTA __RPC_FAR *plf ) = 0;
    virtual IFMETHOD SetCompositionFontW( HIMC hIMC, LOGFONTW __RPC_FAR *plf) = 0;
    virtual IFMETHOD dummy_SetCompositionStringA( ) = 0;
    virtual IFMETHOD SetCompositionStringW( HIMC hIMC, DWORD dwIndex, LPVOID pComp, DWORD dwCompLen, 
	LPVOID pRead, DWORD dwReadLen) = 0;
    virtual IFMETHOD SetCompositionWindow( HIMC hIMC, COMPOSITIONFORM __RPC_FAR *pCompForm) = 0;
    virtual IFMETHOD SetConversionStatus( HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence) = 0;
    virtual IFMETHOD SetOpenStatus( HIMC hIMC, BOOL fOpen) = 0;
    virtual IFMETHOD SetStatusWindowPos( HIMC hIMC, POINT __RPC_FAR *pptPos) = 0;
    virtual IFMETHOD SimulateHotKey( HWND hWnd, DWORD dwHotKeyID) = 0;
    virtual IFMETHOD dummy_UnregisterWordA( ) = 0;
    virtual IFMETHOD UnregisterWordW( HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szUnregister) = 0;
    virtual IFMETHOD Activate( BOOL fRestoreLayout) = 0;
    virtual IFMETHOD Deactivate( void) = 0;
    virtual IFMETHOD OnDefWindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT __RPC_FAR *plResult) = 0;
    virtual IFMETHOD FilterClientWindows( ATOM __RPC_FAR *aaClassList, UINT uSize) = 0;
    virtual IFMETHOD dummy_GetCodePageA( ) = 0;
    virtual IFMETHOD GetLangId( HKL hKL, LANGID __RPC_FAR *plid) = 0;
    virtual IFMETHOD AssociateContextEx( HWND hWnd, HIMC hIMC, DWORD dwFlags) = 0;
    virtual IFMETHOD DisableIME( DWORD idThread) = 0;
    virtual IFMETHOD dummy_GetImeMenuItemsA( ) = 0;
    virtual IFMETHOD GetImeMenuItemsW( HIMC hIMC, DWORD dwFlags, DWORD dwType, /*IMEMENUITEMINFOW*/ void __RPC_FAR *pImeParentMenu,
        /*IMEMENUITEMINFOW*/ void __RPC_FAR *pImeMenu, DWORD dwSize, DWORD __RPC_FAR *pdwResult) = 0;
    virtual IFMETHOD EnumInputContext( DWORD idThread, IEnumInputContext __RPC_FAR *__RPC_FAR *ppEnum) = 0;
};

interface IActiveIMMMessagePumpOwner : public IUnknown
{
public:
    virtual IFMETHOD Start( void) = 0;
    virtual IFMETHOD End( void) = 0;
    virtual IFMETHOD OnTranslateMessage( const MSG __RPC_FAR *pMsg) = 0;
    virtual IFMETHOD Pause( DWORD __RPC_FAR *pdwCookie) = 0;
    virtual IFMETHOD Resume( DWORD dwCookie) = 0;
};

static IActiveIMMApp *aimm = 0;
static IActiveIMMMessagePumpOwner *aimmpump = 0;
static QString *imeComposition = 0;
static int	imePosition    = -1;

void QInputContext::init()
{
    if ( qt_winver < Qt::WV_2000 ) {
	// try to get the Active IMM COM object on Win95/98/NT, where english versions don't
	// support the regular Windows input methods.
	if ( CoCreateInstance(CLSID_CActiveIMM, NULL, CLSCTX_INPROC_SERVER,
	    IID_IActiveIMMApp, (LPVOID *)&aimm) != S_OK ) {
	    aimm = 0;
	}
	if ( aimm && (aimm->QueryInterface(IID_IActiveIMMMessagePumpOwner, (LPVOID *)&aimmpump) != S_OK ||
			aimm->Activate( TRUE ) != S_OK) ) {
	    aimm->Release();
	    aimm = 0;
	}
	aimmpump->Start();
    }
}

void QInputContext::shutdown()
{
    // release active input method if we have one
    if ( aimm ) {
	aimmpump->End();
	aimmpump->Release();
	aimm->Deactivate();
	aimm->Release();
	aimm = 0;
	aimmpump = 0;
    }
    delete imeComposition;
}

static HIMC getContext( HWND wnd )
{
    HIMC imc;
    if ( aimm )
	aimm->GetContext( wnd, &imc );
    else
	imc = ImmGetContext( wnd );

    return imc;
}

static void releaseContext( HWND wnd, HIMC imc )
{
    if ( aimm )
	aimm->ReleaseContext( wnd, imc );
    else
	ImmReleaseContext( wnd, imc );
}

static void notifyIME( HIMC imc, DWORD dwAction, DWORD dwIndex, DWORD dwValue )
{
    if ( aimm )
	aimm->NotifyIME( imc, dwAction, dwIndex, dwValue );
    else
	ImmNotifyIME( imc, dwAction, dwIndex, dwValue );
}

static LONG getCompositionString( HIMC himc, DWORD dwIndex, LPVOID lpbuf, DWORD dBufLen, bool *unicode = 0 )
{
    LONG len = 0;
    if ( unicode ) 
	*unicode = TRUE;
#ifdef Q_OS_TEMP
    buflen = ImmGetCompositionString( himc, dwIndex, lpbuf, dBufLen );
#else
    if ( aimm )
	aimm->GetCompositionStringW( himc, dwIndex, dBufLen, &len, lpbuf );
#ifdef UNICODE
    else if ( qt_winver != Qt::WV_95 )
	len = ImmGetCompositionStringW( himc, dwIndex, lpbuf, dBufLen );
#endif
    else {
	len = ImmGetCompositionStringA( himc, dwIndex, lpbuf, dBufLen );
	if ( unicode ) 
	    *unicode = FALSE;
    }
#endif
    return len;
}

static int getCursorPosition( HIMC himc )
{
    return getCompositionString( himc, GCS_CURSORPOS, 0, 0 );
}

static QString getString( HIMC himc, DWORD dwindex, int *selStart = 0, int *selLength = 0 )
{
    static char *buffer = 0;
    static int buflen = 0;

    int len = getCompositionString( himc, dwindex, 0, 0 ) + 1;
    if ( !buffer || len > buflen ) {
	delete [] buffer;
	buflen = QMIN( len, 256 );
	buffer = new char[ buflen ];
    }

    bool unicode = TRUE;
    len = getCompositionString( himc, dwindex, buffer, buflen, &unicode );

    if ( selStart ) {
	static char *attrbuffer = 0;
	static int attrbuflen = 0;
	int attrlen = getCompositionString( himc, dwindex, 0, 0 ) + 1;
	if ( !attrbuffer || attrlen> attrbuflen ) {
	    delete [] attrbuffer;
	    attrbuflen = QMIN( attrlen, 256 );
	    attrbuffer = new char[ attrbuflen ];
	}
	attrlen = getCompositionString( himc, GCS_COMPATTR, attrbuffer, attrbuflen );
	if ( unicode ) {
	    // rather simple, we just use every second char in the attr array
	    *selStart = attrlen+1;
	    *selLength = -1;
	    for ( int i = 0; i < attrlen; i++ ) {
		if ( attrbuffer[i] & ATTR_TARGET_CONVERTED ) {
		    *selStart = QMIN( *selStart, i );
		    *selLength = QMAX( *selLength, i );
		}
	    }
	    *selLength = QMAX(0, *selLength - *selStart + 1);
	} else {
	    // ### TODO: Make it work on Win95/Asian edition
	    *selStart = 0;
	    *selLength = 0;
	}
    }

    if ( len <= 0 )
	return QString::null;
    if ( unicode ) {
	return QString( (QChar *)buffer, len/sizeof(QChar) );
    } else {
	buffer[len] = 0;
	WCHAR *wc = new WCHAR[len+1];
	int l = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
	    buffer, len, wc, len+1);
	QString res = QString( (QChar *)wc, l );
	delete [] wc;
	return res;
    }
}

void QInputContext::TranslateMessage( const MSG *msg)
{
    if ( !aimmpump || aimmpump->OnTranslateMessage( msg ) != S_OK )
	::TranslateMessage( msg );
}

LRESULT QInputContext::DefWindowProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    LRESULT retval;
#ifdef Q_OS_TEMP
    retval = DefWindowProc( hwnd, msg, wParam, lParam );
#else
    if ( !aimm || aimm->OnDefWindowProc( hwnd, msg, wParam, lParam, &retval ) != S_OK ) {
#if defined(UNICODE)
	if ( qt_winver & Qt::WV_NT_based )
	    retval = ::DefWindowProc( hwnd, msg, wParam, lParam );
	else
#endif
	    retval = ::DefWindowProcA( hwnd,msg, wParam, lParam );
#endif
    }
    return retval;
}


void QInputContext::setFont( const QWidget *w, const QFont &f )
{
    HFONT hf;
    hf = f.handle();

    HIMC imc = getContext( w->winId() );
#ifdef Q_OS_TEMP
    LOGFONT lf;
    if ( GetObject( hf, sizeof(lf), &lf ) )
	ImmSetCompositionFont( imc, &lf );
#else
#ifdef UNICODE
    if ( qt_winver & Qt::WV_NT_based ) {
	LOGFONT lf;
	if ( GetObject( hf, sizeof(lf), &lf ) )
	    if ( aimm )
		aimm->SetCompositionFontW( imc, &lf );
	    else
		ImmSetCompositionFont( imc, &lf );
    } else
#endif
    {
	LOGFONTA lf;
	if ( GetObjectA( hf, sizeof(lf), &lf ) )
	    if ( aimm )
		aimm->SetCompositionFontA( imc, &lf );
	    else
		ImmSetCompositionFontA( imc, &lf );
    }
#endif
    releaseContext( w->winId(), imc );
}


void QInputContext::setFocusHint( int x, int y, int width, int height, const QWidget *w )
{
    COMPOSITIONFORM cf;
    // ### need X-like inputStyle config settings
    cf.dwStyle = CFS_FORCE_POSITION;
    cf.ptCurrentPos.x = x;
    cf.ptCurrentPos.y = y;

    CANDIDATEFORM candf;
    candf.dwIndex = 0;
    candf.dwStyle = CFS_FORCE_POSITION;
    candf.ptCurrentPos.x = x;
    candf.ptCurrentPos.y = y + height + 3;
    candf.rcArea.left = 0;
    candf.rcArea.top = 0;
    candf.rcArea.right = 0;
    candf.rcArea.bottom = 0;


    HIMC imc = getContext( w->winId() );
    if ( aimm ) {
	aimm->SetCompositionWindow( imc, &cf );
	aimm->SetCandidateWindow( imc, &candf );
    } else {
	ImmSetCompositionWindow( imc, &cf );
	ImmSetCandidateWindow( imc, &candf );
    }
    releaseContext( w->winId(), imc );
}


bool QInputContext::endComposition( QWidget *fw )
{
    bool result = TRUE;
#ifdef Q_IME_DEBUG
    qDebug("endComposition!");
#endif

    if ( !fw ) {
	fw = qApp->focusWidget();
	if ( fw && imePosition != -1 ) {
	    QIMEvent e( QEvent::IMEnd, QString::null, -1 );
	    result = qt_sendSpontaneousEvent( fw, &e );
	}
    } else if ( imeComposition && !imeComposition->isNull() ) {
	QIMEvent e( QEvent::IMEnd, QString::null, -1 );
	QApplication::sendEvent( fw, &e );
    
	HIMC imc = getContext( fw->winId() );
	notifyIME( imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
	releaseContext( fw->winId(), imc );
    }
    *imeComposition = QString::null;
    imePosition = -1;

    return result;
}

bool QInputContext::startComposition()
{
#ifdef Q_IME_DEBUG
    qDebug("startComposition" );
#endif

    if ( !imeComposition )
	imeComposition = new QString();

    bool result = TRUE;

    QWidget *fw = qApp->focusWidget();
    if ( fw ) {
	QIMEvent e( QEvent::IMStart, QString::null, -1 );
	result = qt_sendSpontaneousEvent( fw, &e );
	imePosition = 0;
    }
    return result;
}

bool QInputContext::composition( LPARAM lParam )
{
#ifdef Q_IME_DEBUG
    QString str;
    if ( lParam & GCS_RESULTSTR )
	str += "RESULTSTR ";
    if ( lParam & GCS_COMPSTR )
	str += "COMPSTR ";
    if ( lParam & GCS_COMPATTR )
	str += "COMPATTR ";
    if ( lParam & GCS_CURSORPOS )
	str += "CURSORPOS ";
    if ( lParam & GCS_COMPCLAUSE )
	str += "COMPCLAUSE ";
    qDebug( "composition, lParam=%s", str.latin1() );
#endif

    bool result = TRUE;

    QWidget *fw = qApp->focusWidget();
    if ( fw ) {
	if ( imePosition == -1 ) {
	    // need to send a start event
	    startComposition();
	}

	HIMC imc = getContext( fw->winId() );
	if (lParam & GCS_RESULTSTR ) {
	    // a fixed result, return the converted string
	    *imeComposition = getString( imc, GCS_RESULTSTR );
	    imePosition = -1;
	    QIMEvent e( QEvent::IMEnd, *imeComposition, imePosition );
	    *imeComposition = QString::null;
	    result = qt_sendSpontaneousEvent( fw, &e );
	} else if ( lParam & (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS) ) {
	    // some intermediate composition result
	    int selStart, selLength;
	    *imeComposition = getString( imc, GCS_COMPSTR, &selStart, &selLength );
	    imePosition = getCursorPosition( imc );

	    if ( selLength != 0 )
		imePosition = selStart;

#ifdef Q_IME_DEBUG
	    qDebug("imecomposition: cursor pos at %d, str=%x", imePosition, str[0].unicode() );
#endif
	    QIMComposeEvent e( QEvent::IMCompose, *imeComposition, imePosition, selLength );
	    result = qt_sendSpontaneousEvent( fw, &e );
	}
        releaseContext( fw->winId(), imc );
    }
    return result;
}
