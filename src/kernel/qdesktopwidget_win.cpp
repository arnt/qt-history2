#include "qdesktopwidget.h"
#define WINVER 0x0500
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qarray.h"

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();
    ~QDesktopWidgetPrivate();

    static int screenCount;
    static int appScreen;

    static QArray<QRect> rects;
};

int QDesktopWidgetPrivate::screenCount = 0;
int QDesktopWidgetPrivate::appScreen = 0;
QArray<QRect> QDesktopWidgetPrivate::rects = QArray<QRect>();

#if defined(GetMonitorInfo)

BOOL CALLBACK enumCallback( HMONITOR hMonitor, HDC, LPRECT, LPARAM )
{
    static int sn = 0;

    MONITORINFO info;
    memset( &info, 0, sizeof(MONITORINFO) );
    info.cbSize = sizeof(MONITORINFO);
    BOOL res = GetMonitorInfoA( hMonitor, &info );

    RECT r = info.rcMonitor;
    QRect qr( QPoint( r.left, r.top ), QPoint( r.right - 1, r.bottom - 1 ) );
    QDesktopWidgetPrivate::rects.at( sn ) = qr;

    if ( info.dwFlags & MONITORINFOF_PRIMARY )
	QDesktopWidgetPrivate::appScreen = sn;

    ++sn;
    return ( sn != QDesktopWidgetPrivate::screenCount );
}

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = 0;
    if ( qt_winver & Qt::WV_98 || qt_winver & Qt::WV_2000 ) {
	screenCount = GetSystemMetrics( 80 );  // SM_CMONITORS
	rects.resize( screenCount );
	EnumDisplayMonitors( 0, 0, enumCallback, 0 );
    } else 
	screenCount = 1;
}

#else
#pragma message( "Compile " __FILE__ " with WINVER >= 0x0500 defined to enable full multihead support for Win98/2000" )

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
    appScreen = 0;
    screenCount = 1;
}

#endif

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
}

// creates a widget with the size of the virtual desktop
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

QWidget *QDesktopWidget::screen( int /*screen*/ )
{
    // It seems that a WType_Desktop cannot be moved?
    return this;
/*
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->appScreen;

    if ( !d->screens )
    	memset( ( screens = new QWidget*[screenCount] ), 0, screenCount * sizeof( QWidget*) );

    if ( !d->screens[ screen ] ) {
	QWidget *w= new QWidget();
	QRect r = d->rects[ screen ];
	w->move( r.topLeft() );
	w->resize( r.size() );

	d->screens[ screen ] = w;
    }

    return d->screens[ screen ];
*/
}

QRect QDesktopWidget::geometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->appScreen;

    return d->rects[ screen ];
}

int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( !widget )
	return d->appScreen;
    QRect frame = widget->frameGeometry();

    int maxSize = -1;
    int maxScreen = d->appScreen;

    for ( int i = 0; i < d->screenCount; ++i ) {
	QRect sect = d->rects[i].intersect( frame );
	int size = sect.width() * sect.height();
	if ( size > maxSize ) {
	    maxSize = size;
	    maxScreen = i;
	}
    }
    return maxScreen;
}

int QDesktopWidget::screenNumber( const QPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
	if ( d->rects[i].contains( point ) )
	    return i;
    }
    return d->appScreen;
}
