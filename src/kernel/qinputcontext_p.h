#ifndef QINPUTCONTEXT_P_H
#define QINPUTCONTEXT_P_H

#include <qglobal.h>

class QKeyEvent;
class QWidget;
class QFont;
class QString;


#ifdef Q_WS_X11
#include "qarray.h"
#include "qwindowdefs.h"
#include "qt_x11.h"
#endif

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

#ifdef Q_WS_QWS
class QWSIMEvent;
#endif

class QInputContext
{
public:
#ifdef Q_WS_X11
    QInputContext(QWidget *); // should be a toplevel widget
    ~QInputContext();

    void setFocus();
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void reset();

    int lookupString(XKeyEvent *, QCString &, KeySym *, Status *) const;
    void setXFontSet(QFont *);

    void *ic;
    QString text, lastcompose;
    QWidget *focusWidget;
    bool composing;
    QFont font;
    XFontSet fontset;
    QMemArray<bool> selectedChars;
#endif // Q_WS_X11

#ifdef Q_WS_QWS
    static void translateIMEvent( QWSIMEvent *, QWidget * );
    static void reset();
private:    
    static QWidget* focusWidget;
    static QString* composition;
#endif //Q_WS_QWS

#ifdef Q_WS_WIN
    static void init();
    static void shutdown();

    static void TranslateMessage( const MSG *msg);
    static LRESULT DefWindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );

    static void setFont( const QWidget *w, const QFont & );
    static void setFocusHint( int x, int y, int w, int h, const QWidget *widget );
    static bool startComposition();
    static bool endComposition( QWidget *fw = 0 );
    static bool composition( LPARAM lparam );
#endif
};

#endif // QINPUTCONTEXT_P_H
