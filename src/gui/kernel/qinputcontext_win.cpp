/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qinputcontext_p.h"

#include "qfont.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qlibrary.h"
#include "qevent.h"
#include "qtextformat.h"

//#define Q_IME_DEBUG

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
    virtual IFMETHOD AssociateContext(HWND hWnd, HIMC hIME, HIMC __RPC_FAR *phPrev) = 0;
    virtual IFMETHOD dummy_ConfigureIMEA() = 0;
    virtual IFMETHOD ConfigureIMEW(HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDW __RPC_FAR *pData) = 0;
    virtual IFMETHOD CreateContext(HIMC __RPC_FAR *phIMC) = 0;
    virtual IFMETHOD DestroyContext(HIMC hIME) = 0;
    virtual IFMETHOD dummy_EnumRegisterWordA() = 0;
    virtual IFMETHOD EnumRegisterWordW(HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister, LPVOID pData,
        IEnumRegisterWordW __RPC_FAR *__RPC_FAR *pEnum) = 0;
    virtual IFMETHOD dummy_EscapeA() = 0;
    virtual IFMETHOD EscapeW(HKL hKL, HIMC hIMC, UINT uEscape, LPVOID pData, LRESULT __RPC_FAR *plResult) = 0;
    virtual IFMETHOD dummy_GetCandidateListA() = 0;
    virtual IFMETHOD GetCandidateListW(HIMC hIMC, DWORD dwIndex, UINT uBufLen, CANDIDATELIST __RPC_FAR *pCandList,
        UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD dummy_GetCandidateListCountA() = 0;
    virtual IFMETHOD GetCandidateListCountW(HIMC hIMC, DWORD __RPC_FAR *pdwListSize, DWORD __RPC_FAR *pdwBufLen) = 0;
    virtual IFMETHOD GetCandidateWindow(HIMC hIMC, DWORD dwIndex, CANDIDATEFORM __RPC_FAR *pCandidate) = 0;
    virtual IFMETHOD dummy_GetCompositionFontA() = 0;
    virtual IFMETHOD GetCompositionFontW(HIMC hIMC, LOGFONTW __RPC_FAR *plf) = 0;
    virtual IFMETHOD dummy_GetCompositionStringA() = 0;
    virtual IFMETHOD GetCompositionStringW(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LONG __RPC_FAR *plCopied, LPVOID pBuf) = 0;
    virtual IFMETHOD GetCompositionWindow(HIMC hIMC, COMPOSITIONFORM __RPC_FAR *pCompForm) = 0;
    virtual IFMETHOD GetContext(HWND hWnd, HIMC __RPC_FAR *phIMC) = 0;
    virtual IFMETHOD dummy_GetConversionListA() = 0;
    virtual IFMETHOD GetConversionListW(HKL hKL, HIMC hIMC, LPWSTR pSrc, UINT uBufLen, UINT uFlag,
        CANDIDATELIST __RPC_FAR *pDst, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD GetConversionStatus(HIMC hIMC, DWORD __RPC_FAR *pfdwConversion, DWORD __RPC_FAR *pfdwSentence) = 0;
    virtual IFMETHOD GetDefaultIMEWnd(HWND hWnd, HWND __RPC_FAR *phDefWnd) = 0;
    virtual IFMETHOD dummy_GetDescriptionA() = 0;
    virtual IFMETHOD GetDescriptionW(HKL hKL, UINT uBufLen, LPWSTR szDescription, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD dummy_GetGuideLineA() = 0;
    virtual IFMETHOD GetGuideLineW(HIMC hIMC, DWORD dwIndex, DWORD dwBufLen, LPWSTR pBuf, DWORD __RPC_FAR *pdwResult) = 0;
    virtual IFMETHOD dummy_GetIMEFileNameA() = 0;
    virtual IFMETHOD GetIMEFileNameW(HKL hKL, UINT uBufLen, LPWSTR szFileName, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD GetOpenStatus(HIMC hIMC) = 0;
    virtual IFMETHOD GetProperty(HKL hKL, DWORD fdwIndex, DWORD __RPC_FAR *pdwProperty) = 0;
    virtual IFMETHOD dummy_GetRegisterWordStyleA() = 0;
    virtual IFMETHOD GetRegisterWordStyleW(HKL hKL, UINT nItem, STYLEBUFW __RPC_FAR *pStyleBuf, UINT __RPC_FAR *puCopied) = 0;
    virtual IFMETHOD GetStatusWindowPos(HIMC hIMC, POINT __RPC_FAR *pptPos) = 0;
    virtual IFMETHOD GetVirtualKey(HWND hWnd, UINT __RPC_FAR *puVirtualKey) = 0;
    virtual IFMETHOD dummy_InstallIMEA() = 0;
    virtual IFMETHOD InstallIMEW(LPWSTR szIMEFileName, LPWSTR szLayoutText, HKL __RPC_FAR *phKL) = 0;
    virtual IFMETHOD IsIME(HKL hKL) = 0;
    virtual IFMETHOD dummy_IsUIMessageA() = 0;
    virtual IFMETHOD IsUIMessageW(HWND hWndIME, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
    virtual IFMETHOD NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue) = 0;
    virtual IFMETHOD dummy_RegisterWordA() = 0;
    virtual IFMETHOD RegisterWordW(HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister) = 0;
    virtual IFMETHOD ReleaseContext(HWND hWnd, HIMC hIMC) = 0;
    virtual IFMETHOD SetCandidateWindow(HIMC hIMC, CANDIDATEFORM __RPC_FAR *pCandidate) = 0;
    virtual IFMETHOD SetCompositionFontA(HIMC hIMC, LOGFONTA __RPC_FAR *plf) = 0;
    virtual IFMETHOD SetCompositionFontW(HIMC hIMC, LOGFONTW __RPC_FAR *plf) = 0;
    virtual IFMETHOD dummy_SetCompositionStringA() = 0;
    virtual IFMETHOD SetCompositionStringW(HIMC hIMC, DWORD dwIndex, LPVOID pComp, DWORD dwCompLen,
        LPVOID pRead, DWORD dwReadLen) = 0;
    virtual IFMETHOD SetCompositionWindow(HIMC hIMC, COMPOSITIONFORM __RPC_FAR *pCompForm) = 0;
    virtual IFMETHOD SetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence) = 0;
    virtual IFMETHOD SetOpenStatus(HIMC hIMC, BOOL fOpen) = 0;
    virtual IFMETHOD SetStatusWindowPos(HIMC hIMC, POINT __RPC_FAR *pptPos) = 0;
    virtual IFMETHOD SimulateHotKey(HWND hWnd, DWORD dwHotKeyID) = 0;
    virtual IFMETHOD dummy_UnregisterWordA() = 0;
    virtual IFMETHOD UnregisterWordW(HKL hKL, LPWSTR szReading, DWORD dwStyle, LPWSTR szUnregister) = 0;
    virtual IFMETHOD Activate(BOOL fRestoreLayout) = 0;
    virtual IFMETHOD Deactivate(void) = 0;
    virtual IFMETHOD OnDefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT __RPC_FAR *plResult) = 0;
    virtual IFMETHOD FilterClientWindows(ATOM __RPC_FAR *aaClassList, UINT uSize) = 0;
    virtual IFMETHOD dummy_GetCodePageA() = 0;
    virtual IFMETHOD GetLangId(HKL hKL, LANGID __RPC_FAR *plid) = 0;
    virtual IFMETHOD AssociateContextEx(HWND hWnd, HIMC hIMC, DWORD dwFlags) = 0;
    virtual IFMETHOD DisableIME(DWORD idThread) = 0;
    virtual IFMETHOD dummy_GetImeMenuItemsA() = 0;
    virtual IFMETHOD GetImeMenuItemsW(HIMC hIMC, DWORD dwFlags, DWORD dwType, /*IMEMENUITEMINFOW*/ void __RPC_FAR *pImeParentMenu,
        /*IMEMENUITEMINFOW*/ void __RPC_FAR *pImeMenu, DWORD dwSize, DWORD __RPC_FAR *pdwResult) = 0;
    virtual IFMETHOD EnumInputContext(DWORD idThread, IEnumInputContext __RPC_FAR *__RPC_FAR *ppEnum) = 0;
};

interface IActiveIMMMessagePumpOwner : public IUnknown
{
public:
    virtual IFMETHOD Start(void) = 0;
    virtual IFMETHOD End(void) = 0;
    virtual IFMETHOD OnTranslateMessage(const MSG __RPC_FAR *pMsg) = 0;
    virtual IFMETHOD Pause(DWORD __RPC_FAR *pdwCookie) = 0;
    virtual IFMETHOD Resume(DWORD dwCookie) = 0;
};

static IActiveIMMApp *aimm = 0;
static IActiveIMMMessagePumpOwner *aimmpump = 0;
static QString *imeComposition = 0;
static int        imePosition    = -1;
bool qt_use_rtl_extensions = false;

#ifndef LGRPID_INSTALLED
#define LGRPID_INSTALLED          0x00000001  // installed language group ids
#define LGRPID_SUPPORTED          0x00000002  // supported language group ids
#endif

#ifndef LGRPID_ARABIC
#define LGRPID_WESTERN_EUROPE        0x0001   // Western Europe & U.S.
#define LGRPID_CENTRAL_EUROPE        0x0002   // Central Europe
#define LGRPID_BALTIC                0x0003   // Baltic
#define LGRPID_GREEK                 0x0004   // Greek
#define LGRPID_CYRILLIC              0x0005   // Cyrillic
#define LGRPID_TURKISH               0x0006   // Turkish
#define LGRPID_JAPANESE              0x0007   // Japanese
#define LGRPID_KOREAN                0x0008   // Korean
#define LGRPID_TRADITIONAL_CHINESE   0x0009   // Traditional Chinese
#define LGRPID_SIMPLIFIED_CHINESE    0x000a   // Simplified Chinese
#define LGRPID_THAI                  0x000b   // Thai
#define LGRPID_HEBREW                0x000c   // Hebrew
#define LGRPID_ARABIC                0x000d   // Arabic
#define LGRPID_VIETNAMESE            0x000e   // Vietnamese
#define LGRPID_INDIC                 0x000f   // Indic
#define LGRPID_GEORGIAN              0x0010   // Georgian
#define LGRPID_ARMENIAN              0x0011   // Armenian
#endif

void QInputContext::init()
{
    if (QSysInfo::WindowsVersion < QSysInfo::WV_2000) {
        // try to get the Active IMM COM object on Win95/98/NT, where english versions don't
        // support the regular Windows input methods.
        if (CoCreateInstance(CLSID_CActiveIMM, NULL, CLSCTX_INPROC_SERVER,
            IID_IActiveIMMApp, (LPVOID *)&aimm) != S_OK) {
            aimm = 0;
        }
        if (aimm && (aimm->QueryInterface(IID_IActiveIMMMessagePumpOwner, (LPVOID *)&aimmpump) != S_OK ||
                        aimm->Activate(true) != S_OK)) {
            aimm->Release();
            aimm = 0;
            if (aimmpump)
                aimmpump->Release();
            aimmpump = 0;
        }
        if (aimmpump)
            aimmpump->Start();
    }

#ifndef Q_OS_TEMP
    // figure out whether a RTL language is installed
    typedef BOOL(WINAPI *PtrIsValidLanguageGroup)(DWORD,DWORD);
    PtrIsValidLanguageGroup isValidLanguageGroup = (PtrIsValidLanguageGroup)QLibrary::resolve("kernel32", "IsValidLanguageGroup");
    if (isValidLanguageGroup) {
        qt_use_rtl_extensions = isValidLanguageGroup(LGRPID_ARABIC, LGRPID_INSTALLED)
                             || isValidLanguageGroup(LGRPID_HEBREW, LGRPID_INSTALLED);
    }
    qt_use_rtl_extensions |= IsValidLocale(MAKELCID(MAKELANGID(LANG_ARABIC, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED)
                          || IsValidLocale(MAKELCID(MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED)
#ifdef LANG_SYRIAC
                          || IsValidLocale(MAKELCID(MAKELANGID(LANG_SYRIAC, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED)
#endif
                          || IsValidLocale(MAKELCID(MAKELANGID(LANG_FARSI, SUBLANG_DEFAULT), SORT_DEFAULT), LCID_INSTALLED);
#else
    qt_use_rtl_extensions = false;
#endif // Q_OS_TEMP
}

void QInputContext::shutdown()
{
    // release active input method if we have one
    if (aimm) {
        aimmpump->End();
        aimmpump->Release();
        aimm->Deactivate();
        aimm->Release();
        aimm = 0;
        aimmpump = 0;
    }
    delete imeComposition;
    imeComposition = 0;
}

static HIMC getContext(HWND wnd)
{
    HIMC imc;
    if (aimm)
        aimm->GetContext(wnd, &imc);
    else
        imc = ImmGetContext(wnd);

    return imc;
}

static void releaseContext(HWND wnd, HIMC imc)
{
    if (aimm)
        aimm->ReleaseContext(wnd, imc);
    else
        ImmReleaseContext(wnd, imc);
}

static void notifyIME(HIMC imc, DWORD dwAction, DWORD dwIndex, DWORD dwValue)
{
    if (!imc)
        return;
    if (aimm)
        aimm->NotifyIME(imc, dwAction, dwIndex, dwValue);
    else
        ImmNotifyIME(imc, dwAction, dwIndex, dwValue);
}

static LONG getCompositionString(HIMC himc, DWORD dwIndex, LPVOID lpbuf, DWORD dBufLen, bool *unicode = 0)
{
    LONG len = 0;
    if (unicode)
        *unicode = true;
#ifndef Q_OS_TEMP
    if (aimm)
        aimm->GetCompositionStringW(himc, dwIndex, dBufLen, &len, lpbuf);
    else
#endif
    {
        if(QSysInfo::WindowsVersion != QSysInfo::WV_95) {
            len = ImmGetCompositionStringW(himc, dwIndex, lpbuf, dBufLen);
        }
#ifndef Q_OS_TEMP
        else {
            len = ImmGetCompositionStringA(himc, dwIndex, lpbuf, dBufLen);
            if (unicode)
                *unicode = false;
        }
#endif
    }
    return len;
}

static int getCursorPosition(HIMC himc)
{
    return getCompositionString(himc, GCS_CURSORPOS, 0, 0);
}

static QString getString(HIMC himc, DWORD dwindex, int *selStart = 0, int *selLength = 0)
{
    static char *buffer = 0;
    static int buflen = 0;

    int len = getCompositionString(himc, dwindex, 0, 0) + 1;
    if (!buffer || len > buflen) {
        delete [] buffer;
        buflen = qMin(len, 256);
        buffer = new char[buflen];
    }

    bool unicode = true;
    len = getCompositionString(himc, dwindex, buffer, buflen, &unicode);

    if (selStart) {
        static char *attrbuffer = 0;
        static int attrbuflen = 0;
        int attrlen = getCompositionString(himc, dwindex, 0, 0) + 1;
        if (!attrbuffer || attrlen> attrbuflen) {
            delete [] attrbuffer;
            attrbuflen = qMin(attrlen, 256);
            attrbuffer = new char[attrbuflen];
        }
        attrlen = getCompositionString(himc, GCS_COMPATTR, attrbuffer, attrbuflen);
        *selStart = attrlen+1;
        *selLength = -1;
        for (int i = 0; i < attrlen; i++) {
            if (attrbuffer[i] & ATTR_TARGET_CONVERTED) {
                *selStart = qMin(*selStart, i);
                *selLength = qMax(*selLength, i);
            }
        }
        *selLength = qMax(0, *selLength - *selStart + 1);
    }

    if (len <= 0)
        return QString::null;
    if (unicode) {
        return QString((QChar *)buffer, len/sizeof(QChar));
    }
//#ifdef Q_OS_TEMP
    else {
        buffer[len] = 0;
        WCHAR *wc = new WCHAR[len+1];
        int l = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
            buffer, len, wc, len+1);
        QString res = QString((QChar *)wc, l);
        delete [] wc;
        return res;
    }
//#endif
}

void QInputContext::TranslateMessage(const MSG *msg)
{
    if (!aimmpump || aimmpump->OnTranslateMessage(msg) != S_OK)
        ::TranslateMessage(msg);
}

LRESULT QInputContext::DefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT retval;
#ifndef Q_OS_TEMP
    if (!aimm || aimm->OnDefWindowProc(hwnd, msg, wParam, lParam, &retval) != S_OK)
#endif
    {
        QT_WA({
            retval = ::DefWindowProc(hwnd, msg, wParam, lParam);
        } , {
            retval = ::DefWindowProcA(hwnd,msg, wParam, lParam);
        });
    }
    return retval;
}


void QInputContext::setFont(const QWidget *w, const QFont &f)
{
    HFONT hf;
    hf = f.handle();

    HIMC imc = getContext(w->winId());

    if (!imc)
        return;

#ifdef Q_OS_TEMP
    LOGFONT lf;
    if (GetObject(hf, sizeof(lf), &lf))
        ImmSetCompositionFont(imc, &lf);
#else
    QT_WA({
        LOGFONT lf;
        if (GetObject(hf, sizeof(lf), &lf))
            if (aimm)
                aimm->SetCompositionFontW(imc, &lf);
            else
                ImmSetCompositionFont(imc, &lf);
    } , {
        LOGFONTA lf;
        if (GetObjectA(hf, sizeof(lf), &lf))
            if (aimm)
                aimm->SetCompositionFontA(imc, &lf);
            else
                ImmSetCompositionFontA(imc, &lf);
    });
#endif
    releaseContext(w->winId(), imc);
}


void QInputContext::setFocusHint(int x, int y, int width, int height, const QWidget *w)
{
    COMPOSITIONFORM cf;
    // ### need X-like inputStyle config settings
    cf.dwStyle = CFS_FORCE_POSITION;
    cf.ptCurrentPos.x = x;
    cf.ptCurrentPos.y = y;

    CANDIDATEFORM candf;
    candf.dwIndex = 0;
    candf.dwStyle = CFS_EXCLUDE;
    candf.ptCurrentPos.x = x;
    candf.ptCurrentPos.y = y + height;
    candf.rcArea.left = x;
    candf.rcArea.top = y;
    candf.rcArea.right = x+width;
    candf.rcArea.bottom = y+height;


    HIMC imc = getContext(w->winId());
    if (imc) {
        if (aimm) {
            aimm->SetCompositionWindow(imc, &cf);
            aimm->SetCandidateWindow(imc, &candf);
        } else {
            ImmSetCompositionWindow(imc, &cf);
            ImmSetCandidateWindow(imc, &candf);
        }
        releaseContext(w->winId(), imc);
    }
}


bool QInputContext::endComposition(QWidget *fw)
{
#ifdef Q_IME_DEBUG
    qDebug("endComposition! fw = %s", fw ? fw->className() : "(null)");
#endif
    bool result = true;
    if(imePosition == -1)
        return result;

    if (fw) {
        HIMC imc = getContext(fw->winId());
        notifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        releaseContext(fw->winId(), imc);
    }

    if (!fw)
        fw = qApp->focusWidget();

    if (fw) {
        QInputMethodEvent e(QString::null, QString::null, QList<QInputMethodEvent::Attribute>());
        result = qt_sendSpontaneousEvent(fw, &e);
    }

    if (imeComposition)
        *imeComposition = QString::null;
    imePosition = -1;

    return result;
}

void QInputContext::accept(QWidget *fw)
{
    if (!fw)
        fw = qApp->focusWidget();
#ifdef Q_IME_DEBUG
    qDebug("sending accept to focus widget %s", fw ? fw->className() : "(null)");
#endif

    if (fw && imePosition != -1) {
        QInputMethodEvent e(QString::null, imeComposition ? *imeComposition : QString(), 
                            QList<QInputMethodEvent::Attribute>());
         qt_sendSpontaneousEvent(fw, &e);
    }

    if (imeComposition)
        *imeComposition = QString::null;
    imePosition = -1;

    if (fw) {
        HIMC imc = getContext(fw->winId());
        notifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
        releaseContext(fw->winId(), imc);
    }

}


bool QInputContext::startComposition()
{
#ifdef Q_IME_DEBUG
    qDebug("startComposition");
#endif

    if (!imeComposition)
        imeComposition = new QString();

    bool result = true;

    QWidget *fw = qApp->focusWidget();
    if (fw) {
        imePosition = 0;
    }
    return fw != 0;
}

enum StandardFormat {
    PreeditFormat,
    SelectionFormat
};

static QTextFormat standardFormat(StandardFormat s, QWidget *focusWidget)
{
    const QPalette &pal = focusWidget ? focusWidget->palette() : qApp->palette();

    QTextCharFormat fmt;
    QColor bg;
    switch (s) {
    case PreeditFormat: {
        fmt.setFontUnderline(true);
        int h1, s1, v1, h2, s2, v2;
        pal.color(QPalette::Base).getHsv(&h1, &s1, &v1);
        pal.color(QPalette::Background).getHsv(&h2, &s2, &v2);
        bg.setHsv(h1, s1, (v1 + v2) / 2);
        break;
    }
    case SelectionFormat: {
        bg = pal.text().color();
        fmt.setTextColor(pal.background().color());
        break;
    }
    }
    fmt.setBackgroundColor(bg);
    return fmt;
}


bool QInputContext::composition(LPARAM lParam)
{
#ifdef Q_IME_DEBUG
    QString str;
    if (lParam & GCS_RESULTSTR)
        str += "RESULTSTR ";
    if (lParam & GCS_COMPSTR)
        str += "COMPSTR ";
    if (lParam & GCS_COMPATTR)
        str += "COMPATTR ";
    if (lParam & GCS_CURSORPOS)
        str += "CURSORPOS ";
    if (lParam & GCS_COMPCLAUSE)
        str += "COMPCLAUSE ";
    if (lParam & CS_INSERTCHAR)
       str += "INSERTCHAR ";
    if (lParam & CS_NOMOVECARET)
       str += "NOMOVECARET ";
    qDebug("composition, lParam=(%x) %s imePosition=%d", lParam, str.latin1(), imePosition);
#endif

    bool result = true;

    if(!lParam)
        // bogus event
        return true;

    QWidget *fw = qApp->focusWidget();
    if (fw) {
        HIMC imc = getContext(fw->winId());
        if (lParam & GCS_RESULTSTR) {
            if(imePosition == -1)
                startComposition();
            // a fixed result, return the converted string
            *imeComposition = getString(imc, GCS_RESULTSTR);
            imePosition = -1;
            QInputMethodEvent e(QString(), *imeComposition, QList<QInputMethodEvent::Attribute>());
            *imeComposition = QString::null;
            result = qt_sendSpontaneousEvent(fw, &e);
        }
        else if (lParam & (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS)) {
            if (imePosition == -1) {
                // need to send a start event
                startComposition();
            }
            // some intermediate composition result
            int selStart, selLength;
            *imeComposition = getString(imc, GCS_COMPSTR, &selStart, &selLength);
            imePosition = getCursorPosition(imc);
            if (lParam & CS_INSERTCHAR  && lParam & CS_NOMOVECARET) {
                // make korean work correctly. Hope this is correct for all IMEs
                selStart = 0;
                selLength = imeComposition->length();
            }

           if (selLength != 0)
                imePosition = selStart;

           QList<QInputMethodEvent::Attribute> attrs;
           if (imePosition > 0)
               attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, imePosition,
                                                     standardFormat(PreeditFormat, fw));
           if (selLength)
               attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, imePosition, selLength,
                                                     standardFormat(SelectionFormat, fw));
           if (imePosition + selLength < imeComposition->length())
               attrs << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, imePosition + selLength, 
                                                     imeComposition->length() - imePosition - selLength,
                                                     standardFormat(PreeditFormat, fw));
           QInputMethodEvent e(*imeComposition, QString(), attrs);
           result = qt_sendSpontaneousEvent(fw, &e);
        }
        releaseContext(fw->winId(), imc);
    }
#ifdef Q_IME_DEBUG
    qDebug("imecomposition: cursor pos at %d, str=%x", imePosition, str[0].unicode());
#endif
    return result;
}

static HIMC defaultContext = 0;

void QInputContext::enable(QWidget *w, bool b)
{
    // ######################## do on focus in now!
#ifdef Q_IME_DEBUG
    qDebug("enable: w=%s, enable = %s", w ? w->className() : "(null)" , b ? "true" : "false");
#endif
    if(aimm) {
        HIMC oldimc;
        if (!b) {
            aimm->AssociateContext(w->winId(), 0, &oldimc);
            if (!defaultContext)
                defaultContext = oldimc;
        } else if (defaultContext) {
            aimm->AssociateContext(w->winId(), defaultContext, &oldimc);
        }
    } else {
        if (!b) {
            HIMC oldimc = ImmAssociateContext(w->winId(), 0);
            if (!defaultContext)
                defaultContext = oldimc;
        } else if (defaultContext) {
            ImmAssociateContext(w->winId(), defaultContext);
        }
    }
}
