#include "qdesktopwidget.h"
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qarray.h"

class QDesktopWidget::Private
{
public:
    Private();
    ~Private();

    static int screenCount;
    static int primaryScreen;

    static QArray<QRect> rects;

    struct MONITORINFO
    {
	DWORD   cbSize;
	RECT    rcMonitor;
	RECT    rcWork;
	DWORD   dwFlags;
    };

    typedef BOOL (WINAPI *InfoFunc)( HMONITOR, MONITORINFO* );
    typedef BOOL (CALLBACK *EnumProc)( HMONITOR, HDC, LPRECT, LPARAM );
    typedef BOOL (WINAPI *EnumFunc)(HDC, LPCRECT, EnumProc, LPARAM );

    static EnumFunc enumDisplayMonitors;
    static InfoFunc getMonitorInfo;
    static HMODULE user32hnd;
};

int QDesktopWidget::Private::screenCount = 1;
int QDesktopWidget::Private::primaryScreen = 0;
QDesktopWidget::Private::EnumFunc QDesktopWidget::Private::enumDisplayMonitors = 0;
QDesktopWidget::Private::InfoFunc QDesktopWidget::Private::getMonitorInfo = 0;
HMODULE QDesktopWidget::Private::user32hnd = 0;
QArray<QRect> QDesktopWidget::Private::rects = QArray<QRect>();

BOOL CALLBACK enumCallback( HMONITOR hMonitor, HDC, LPRECT, LPARAM )
{
    static int sn = 0;

    // Get the MONITORINFO block
    QDesktopWidget::Private::MONITORINFO info;
    memset( &info, 0, sizeof(QDesktopWidget::Private::MONITORINFO) );
    info.cbSize = sizeof(QDesktopWidget::Private::MONITORINFO);
    BOOL res = QDesktopWidget::Private::getMonitorInfo( hMonitor, &info );
    if ( !res ) {
	QDesktopWidget::Private::rects.at( sn ) = QRect();
	return TRUE;
    }

    // Fill list of rects
    RECT r = info.rcMonitor;
    QRect qr( QPoint( r.left, r.top ), QPoint( r.right - 1, r.bottom - 1 ) );
    QDesktopWidget::Private::rects.at( sn ) = qr;

    if ( info.dwFlags & 0x00000001 ) //MONITORINFOF_PRIMARY
	QDesktopWidget::Private::primaryScreen = sn;

    ++sn;
    // Stop the enumeration if we have them all
    return ( sn != QDesktopWidget::Private::screenCount );
}

QDesktopWidget::Private::Private()
{
    if ( qt_winver & Qt::WV_98 || qt_winver & Qt::WV_2000 ) {
	screenCount = GetSystemMetrics( 80 );  // SM_CMONITORS
	rects.resize( screenCount );
	// Trying to get the function pointers to Win98/2000 only functions
	user32hnd = LoadLibraryA( "user32.dll" );
	if ( !user32hnd )
	    return;
	enumDisplayMonitors = (EnumFunc)GetProcAddress( user32hnd, "EnumDisplayMonitors" );
	if ( qt_winver & Qt::WV_NT_based )
#if defined(UNICODE)
	    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, "GetMonitorInfoW" );
#else
	    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, "GetMonitorInfoA" );
#endif
	else
	    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, "GetMonitorInfoA" );

	if ( !enumDisplayMonitors || !getMonitorInfo )
	    return;
	// Calls enumCallback
	enumDisplayMonitors( 0, 0, enumCallback, 0 );
	enumDisplayMonitors = 0;
	getMonitorInfo = 0;
	FreeLibrary( user32hnd );
    }
}

QDesktopWidget::Private::~Private()
{
}

/*!
  \class QDesktopWidget qdesktopwidget.h
  \brief The QDesktopWidget class provides an API to access the screen information on multi-head systems.
*/

/*!
  Creates the desktop widget.
  If supported, this widget will have the size of the virtual desktop. Otherwise it
  represents the primary screen.
*/
QDesktopWidget::QDesktopWidget()
: QWidget( 0, "desktop", WType_Desktop )
{
    d = new Private;
}

/*!
  Destroy the object and free allocated resources.
*/
QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

/*!
  Returns the index of the primary screen.

  \sa numScreens
*/
int QDesktopWidget::primaryScreen() const
{
    return d->primaryScreen;
}

/*!
  Returns the total number of available screens.

  \sa primaryScreen
*/
int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

/*!
  Returns a widget that represents the screen with index \a screen.
  This widget can be used to draw directly on the desktop, using an
  unclipped painter like this:

  \code

  QPainter paint( QApplication::desktop()->screen( 0 ), TRUE );
  paint.draw...
  ...
  paint.end();

  \endcode

  \sa primaryScreen, numScreens
*/
QWidget *QDesktopWidget::screen( int screen )
{
    Q_UNUSED( screen );
    // It seems that a WType_Desktop cannot be moved?
    return this;
/*
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->primaryScreen;

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

/*!
  Returns the geometry of the display with index \a screen.

  \sa screenNumber( QWidget* )
*/
const QRect& QDesktopWidget::screenGeometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->primaryScreen;

    return d->rects[ screen ];
}

/*!
  Returns the index of the screen that contains the biggest
  part of \a widget.

  \sa primaryScreen
*/
int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( !widget )
	return d->primaryScreen;
    QRect frame = widget->frameGeometry();
    if ( !widget->isTopLevel() )
	frame.moveTopLeft( widget->mapToGlobal( frame.topLeft() ) );

    int maxSize = -1;
    int maxScreen = d->primaryScreen;

    for ( int i = 0; i < d->screenCount; ++i ) {
	QRect sect = d->rects[i].intersect( frame );
	int size = sect.width() * sect.height();
	if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
	    maxSize = size;
	    maxScreen = i;
	}
    }
    return maxScreen;
}

/*!
  Returns the index of the screen that contains \a point.

  \sa primaryScreen
*/
int QDesktopWidget::screenNumber( const QPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
	if ( d->rects[i].contains( point ) )
	    return i;
    }
    return d->primaryScreen;
}
