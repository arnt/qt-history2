#include "qwindowsxpstyle.h"

#ifndef QT_NO_STYLE_WINDOWSXP

#include <qpainter.h>
#include <qwidget.h>

#if defined (Q_WS_WIN)
#include <qt_windows.h>
#include <qlibrary.h>
#endif

class QWindowsXPStyle::Private
{
public:
    Private()
    {
#if defined(Q_WS_WIN)
	if ( qWinVersion() == WV_XP && !init_xp ) {
	    init_xp = TRUE;
	    uxtheme = new QLibrary( "uxtheme" );
	    IsThemeActive = (IsThemeActiveFnc)uxtheme->resolve( "IsThemeActive" );

	    if ( !IsThemeActive ) {
		delete uxtheme;
	    } else {

		use_xp = IsThemeActive();
	    }
	}
	if ( use_xp )
	    ref++;
#endif
    }
    ~Private()
    {
#if defined(Q_WS_WIN)
	if ( use_xp ) {
	    if ( !--ref ) {
		delete uxtheme;
		uxtheme = 0;
	    }
	}
#endif
    }

    typedef BOOL(*IsThemeActiveFnc)();
    static IsThemeActiveFnc IsThemeActive;

    static bool use_xp;

private:
    static QLibrary *uxtheme;
    static ulong ref;
    static bool init_xp;
};

QLibrary *QWindowsXPStyle::Private::uxtheme = NULL;
ulong QWindowsXPStyle::Private::ref = 0;
bool QWindowsXPStyle::Private::use_xp  = FALSE;
bool QWindowsXPStyle::Private::init_xp = FALSE;

QWindowsXPStyle::Private::IsThemeActiveFnc QWindowsXPStyle::Private::IsThemeActive = 0;

QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new Private;
}

QWindowsXPStyle::~QWindowsXPStyle()
{
    delete d;
}

#if defined(Q_WS_WIN)
    #define Q_GET_HANDLES( x )			    \
	    HDC hdc = x->device()->handle();	    \
	    QWidget *widget = (QWidget*)x->device();\
	    HWND hwnd = widget->winId();
#else
    #define Q_GET_HANDLES( x )			    \
	    void *hdc;				    \
	    void *hwnd;				    \
	    return;
#endif

void QWindowsXPStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                 const QColorGroup &g, bool sunken, const QBrush *fill )
{
    if ( !d->use_xp ) {
	QWindowsStyle::drawButton( p, x, y, w, h, g, sunken, fill );
	return;
    }
    Q_GET_HANDLES( p )
}

#endif //QT_NO_STYLE_WINDOWSXP
