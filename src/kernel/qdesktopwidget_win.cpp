#include "qdesktopwidget.h"
#include "qt_windows.h"
#include "qapplication_p.h"
#include "qmemarray.h"

class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate( QDesktopWidget *that );
    ~QDesktopWidgetPrivate();

    static int screenCount;
    static int primaryScreen;

    static QMemArray<QRect> *rects;

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

int QDesktopWidgetPrivate::screenCount = 1;
int QDesktopWidgetPrivate::primaryScreen = 0;
QDesktopWidgetPrivate::EnumFunc QDesktopWidgetPrivate::enumDisplayMonitors = 0;
QDesktopWidgetPrivate::InfoFunc QDesktopWidgetPrivate::getMonitorInfo = 0;
HMODULE QDesktopWidgetPrivate::user32hnd = 0;
QArray<QRect> *QDesktopWidgetPrivate::rects = 0;
static int screen_number = 0;

BOOL CALLBACK enumCallback( HMONITOR hMonitor, HDC, LPRECT, LPARAM )
{
    // Get the MONITORINFO block
    QDesktopWidgetPrivate::MONITORINFO info;
    memset( &info, 0, sizeof(QDesktopWidgetPrivate::MONITORINFO) );
    info.cbSize = sizeof(QDesktopWidgetPrivate::MONITORINFO);
    BOOL res = QDesktopWidgetPrivate::getMonitorInfo( hMonitor, &info );
    if ( !res ) {
	QDesktopWidgetPrivate::rects->at( screen_number ) = QRect();
	return TRUE;
    }

    // Fill list of rects
    RECT r = info.rcMonitor;
    QRect qr( QPoint( r.left, r.top ), QPoint( r.right - 1, r.bottom - 1 ) );
    QDesktopWidgetPrivate::rects->at( screen_number ) = qr;

    if ( info.dwFlags & 0x00000001 ) //MONITORINFOF_PRIMARY
	QDesktopWidgetPrivate::primaryScreen = screen_number;

    ++screen_number;
    // Stop the enumeration if we have them all
    return ( screen_number != QDesktopWidgetPrivate::screenCount );
}

QDesktopWidgetPrivate::QDesktopWidgetPrivate( QDesktopWidget *that )
{
    rects = new QArray<QRect>();
    if ( qt_winver & Qt::WV_98 || qt_winver & Qt::WV_2000 || qt_winver == Qt::WV_XP ) {
	screenCount = GetSystemMetrics( 80 );  // SM_CMONITORS
	rects->resize( screenCount );
	// Trying to get the function pointers to Win98/2000 only functions
	user32hnd = LoadLibraryA( "user32.dll" );
	if ( !user32hnd )
	    return;
#ifdef Q_OS_TEMP
	enumDisplayMonitors = (EnumFunc)GetProcAddress( user32hnd, L"EnumDisplayMonitors" );
    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, L"GetMonitorInfoW" );
#else
	enumDisplayMonitors = (EnumFunc)GetProcAddress( user32hnd, "EnumDisplayMonitors" );
	if ( qt_winver & Qt::WV_NT_based )
#if defined(UNICODE)
	    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, "GetMonitorInfoW" );
#else
	    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, "GetMonitorInfoA" );
#endif
	else
	    getMonitorInfo = (InfoFunc)GetProcAddress( user32hnd, "GetMonitorInfoA" );
#endif

	if ( !enumDisplayMonitors || !getMonitorInfo ) {
	    for ( int i = 0; i < screenCount; ++i )
		rects->at( i ) = that->rect();
	    return;
	}
	// Calls enumCallback
	enumDisplayMonitors( 0, 0, enumCallback, 0 );
	enumDisplayMonitors = 0;
	getMonitorInfo = 0;
	FreeLibrary( user32hnd );
    } else {
	rects->resize( 1 );
	rects->at( 0 ) = that->rect();
    }
}

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
    screen_number = 0;
    screenCount = 1;
    primaryScreen = 0;
    enumDisplayMonitors = 0;
    getMonitorInfo = 0;
    user32hnd = 0;
    delete rects;
    rects = 0;
}

/*!
  \class QDesktopWidget qdesktopwidget.h
  \brief The QDesktopWidget class provides access to screen information on multi-head systems.

  \ingroup advanced
  \ingroup environment

  Systems with more than one graphics card and monitor can manage the
  physical screen space available either as multiple desktops, or as a
  large virtual desktop, which usually has the size of the bounding
  rectangle of all the screens (see isVirtualDesktop()). For an
  application, one of the available screens is the primary screen, i.e.
  the screen where the main widget resides (see primaryScreen()). All
  windows opened in the context of the application have to be
  constrained to the boundaries of the primary screen; for example, it
  would be inconvenient if a dialog box popped up on a different screen,
  or split over two screens.

  The QDesktopWidget provides information about the geometry of the
  available screens with screenGeometry(). The number of screens
  available is returned by numScreens(). The screen number that a
  particular point or widget is located in is returned by
  screenNumber().

    Widgets provided by Qt use this class, for example, to place
    tooltips, menus and dialog boxes according to the parent or
    application widget.

  Applications can use this class to save window positions, or to place
  child widgets on one screen.
*/

/*!
  Creates the desktop widget.

  If the system supports a virtual desktop, this widget will have the
  size of the virtual desktop; otherwise this widget will have the size
  of the primary screen.

*/
QDesktopWidget::QDesktopWidget()
: QWidget( 0, "desktop", WType_Desktop )
{
    d = new QDesktopWidgetPrivate( this );
}

/*!
  Destroy the object and free allocated resources.
*/
QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

/*!
  Returns TRUE if the system manages the available screens in a virtual
  desktop; otherwise returns FALSE.

  For virtual desktops, screen() will always return the same widget.
  The size of the virtual desktop is the size of this desktop widget.
*/
bool QDesktopWidget::isVirtualDesktop() const
{
    return TRUE;
}

/*!
  Returns the index of the primary screen.

  \sa numScreens()
*/
int QDesktopWidget::primaryScreen() const
{
    return d->primaryScreen;
}

/*!
  Returns the number of available screens.

  \sa primaryScreen()
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

  If the system uses a virtual desktop, the returned widget will have
  the geometry of the entire virtual desktop i.e. bounding every \a
  screen.

  \sa primaryScreen(), numScreens(), isVirtualDesktop()
*/
QWidget *QDesktopWidget::screen( int /*screen*/ )
{
    // It seems that a WType_Desktop cannot be moved?
    return this;
}

/*!
  Returns the geometry of the screen with index \a screen.

  \sa screenNumber()
*/
const QRect& QDesktopWidget::screenGeometry( int screen ) const
{
    if ( qt_winver & Qt::WV_98 || qt_winver & Qt::WV_2000 || qt_winver == Qt::WV_XP ) {
	if ( screen < 0 || screen >= d->screenCount )
	    screen = d->primaryScreen;

	return d->rects->at( screen );
    } else {
	return d->rects->at( d->primaryScreen );
    }
}

/*!
  Returns the index of the screen that contains the largest
  part of \a widget, or -1 if the widget not on a screen.

  \sa primaryScreen()
*/
int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( qt_winver & Qt::WV_98 || qt_winver & Qt::WV_2000 || qt_winver == Qt::WV_XP ) {
	if ( !widget )
	    return d->primaryScreen;
	QRect frame = widget->frameGeometry();
	if ( !widget->isTopLevel() )
	    frame.moveTopLeft( widget->mapToGlobal( QPoint( 0,0 ) ) );

	int maxSize = -1;
	int maxScreen = -1;

	for ( int i = 0; i < d->screenCount; ++i ) {
	    QRect sect = d->rects->at(i).intersect( frame );
	    int size = sect.width() * sect.height();
	    if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
		maxSize = size;
		maxScreen = i;
	    }
	}
	return maxScreen;
    } else {
	return d->primaryScreen;
    }
}

/*!
    \overload
  Returns the index of the screen that contains \a point, or -1 if no
  screen contains the point.

  \sa primaryScreen()
*/
int QDesktopWidget::screenNumber( const QPoint &point ) const
{
    if ( qt_winver & Qt::WV_98 || qt_winver & Qt::WV_2000 || qt_winver == Qt::WV_XP ) {
	for ( int i = 0; i < d->screenCount; ++i ) {
	    if ( d->rects->at(i).contains( point ) )
		return i;
	}
    }
    return -1;
}
