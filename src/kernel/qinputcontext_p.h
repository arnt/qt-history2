#ifndef QINPUTCONTEXT_P_H
#define QINPUTCONTEXT_P_H

#if defined(Q_WS_X11)

#include "qwindowdefs.h"
#include "qt_x11.h"

class QKeyEvent;
class QWidget;

class QInputContext
{
public:
    QInputContext(QWidget *); // should be a toplevel widget
    ~QInputContext();

    void setFocus();
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void reset();

#if defined(Q_WS_X11)
    int lookupString(XKeyEvent *, QCString &, KeySym *, Status *) const;
    void setXFontSet(QFont *);
#endif // Q_WS_X11

    void *ic;
    QString text, lastcompose;
    QWidget *focusWidget;
    bool composing;
#ifdef    Q_WS_X11
    QFont font;
    XFontSet fontset;
#endif // Q_WS_X11
};

#else if defined(Q_WS_WIN)

#include <qt_windows.h>

class QInputContext
{
public:
    static void init();
    static void shutdown();

    static void TranslateMessage( const MSG *msg);
    static LRESULT DefWindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );

    static HIMC getContext( HWND wnd );
    static void releaseContext( HWND wnd, HIMC imc );
    static void notifyIME( HIMC imc, DWORD dwAction, DWORD dwIndex, DWORD dwValue );
    static QString getCompositionString( HIMC imc, DWORD dwindex, int *pos = 0 );
};


#endif


#endif // QINPUTCONTEXT_P_H
