#include "qdesktopwidget.h"
#include "qt_x11.h"

extern Display* appDpy;
extern int qt_x11_create_desktop_on_screen; // defined in qwidget_x11.cpp

class QDesktopWidgetPrivate
{
    QDesktopWidgetPrivate();
    ~QDesktopWidgetPrivate();

    int appScreen;
    int screenCount;
    Window appRootWin;

    QWidget **screens;
    QArray<QRect> rects;
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = DefaultScreen(appDpy);
    screenCount = ScreenCount(appDpy);
    appRootWin = RootWindow(appDpy,appScreen);
    rects.resize( screenCount );
    //### Read the list of rects and put it into rects

    screens = 0;
};

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
    for ( int i = 0; i < screenCount; ++i ) {
	delete screens[ i ];
	screens[ i ] = 0;
    }
    delete[] screens;
}

// the QDesktopWidget itself will be created on the default screen
// as qt_x11_create_desktop_on_screen defaults to -1
QDesktopWidget::QDesktopWidget()
: QWidget( 0, "desktop", WType_Desktop )
{
    d = new QDesktopWidgetPrivate;
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

int QDesktopWidget::primaryScreen() const
{
    return d->appScreen;
}

int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

QWidget *QDesktopWidget::screen( int screen )
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->appScreen;
    
    if ( !d->screens ) {
	memset( (d->screens = new QWidget*[screenCount] ), 0, d->screenCount * sizeof( QWidget*) );
	d->screens[appScreen] = this;
    }

    if ( !d->screens[screen] ||			// not created yet
	 !d->screens[screen]->isDesktop() ) {	// reparented away
	qt_x11_create_desktop_on_screen = screen;
	d->screens[screen] = new QWidget( 0, "desktop", WType_Desktop );
	qt_x11_create_desktop_on_screen = -1;
    }
    return d->screens[screen];
}

QRect QDesktopWidget::geometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->appScreen;

    return d->rects[ screen ];
}
