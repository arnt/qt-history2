#include "qtrayapplication.h"
#include <qcursor.h>
#include <qimage.h>
#include <qpopupmenu.h>

#include <qt_windows.h>

#define MYWM_NOTIFYICON	(WM_APP+101) // WM_APP+101 will be unique

class QTrayApplication::QTrayApplicationPrivate 
{
public:
    QTrayApplicationPrivate() : has_icon( FALSE ), current_popup( 0 ), hIcon( 0 ), hWnd( 0 )
    {
    }

    ~QTrayApplicationPrivate()
    {
	remove();
	if ( hIcon )
	    DestroyIcon( hIcon );
    }

    bool update( bool show = FALSE )
    {
	if ( !has_icon && !show || !(current_popup || qApp->mainWidget()) )
	    return FALSE;

	bool res;
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    res = trayMessageW( has_icon ? NIM_MODIFY : NIM_ADD );
	else
#endif
	    res = trayMessageA( has_icon ? NIM_MODIFY : NIM_ADD );

	if ( res )
	    has_icon = TRUE;

	return res;
    }

    bool remove()
    {
	if ( !has_icon )
	    return TRUE;

	has_icon = FALSE;
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    return trayMessageW( NIM_DELETE );
	else
#endif
	    return trayMessageA( NIM_DELETE );
    }

    // the unavoidable A/W versions. Don't forget to keep them in sync!
    bool trayMessageA( DWORD msg ) 
    {
	bool res;

	NOTIFYICONDATAA tnd;
	memset( &tnd, 0, sizeof(NOTIFYICONDATAA) );
	tnd.cbSize		= sizeof(NOTIFYICONDATAA);
	tnd.hWnd		= hWnd ? hWnd : ( current_popup ? current_popup->winId() : 
						( qApp->mainWidget() ? qApp->mainWidget()->winId() : 0 ) );
	hWnd = tnd.hWnd;

	if ( msg != NIM_DELETE ) {	    
	    if ( !tnd.hWnd )
		qFatal( "No widget available for notification! Call setMainWidget or setPopup first!" );

	    tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	    tnd.uCallbackMessage= MYWM_NOTIFYICON;
	    tnd.hIcon		= hIcon;
	    if ( !current_tip.isNull() ) {
		// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
		QString tip = current_tip.left( 63 ) + QChar();
		lstrcpynA(tnd.szTip, (const char*)tip, QMIN( tip.length()+1, 64 ) );
	    }
	}

	res = Shell_NotifyIconA(msg, &tnd);

	return res;
    }

    bool trayMessageW( DWORD msg ) 
    {
	bool res;

	NOTIFYICONDATAW tnd;
	memset( &tnd, 0, sizeof(NOTIFYICONDATAW) );
	tnd.cbSize		= sizeof(NOTIFYICONDATAW);
	tnd.hWnd		= hWnd ? hWnd : ( current_popup ? current_popup->winId() : 
						( qApp->mainWidget() ? qApp->mainWidget()->winId() : 0 ) );
	hWnd = tnd.hWnd;

	if ( msg != NIM_DELETE ) {
	    if ( !tnd.hWnd )
		qFatal( "No widget available for notification! Call setMainWidget or setPopup first!" );

	    tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	    tnd.uCallbackMessage= MYWM_NOTIFYICON;
	    tnd.hIcon		= hIcon;
	    if ( !current_tip.isNull() ) {
		// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
		QString tip = current_tip.left( 63 ) + QChar();
		lstrcpynW(tnd.szTip, (const unsigned short*)qt_winTchar( tip, FALSE ), QMIN( tip.length()+1, 64 ) );
	    }
	}

	res = Shell_NotifyIconW(msg, &tnd);

	return res;
    }

    bool iconDrawItem(LPDRAWITEMSTRUCT lpdi)
    {
	if (!hIcon)
	    return FALSE;

	DrawIconEx(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, hIcon,
		0, 0, 0, NULL, DI_NORMAL );

	return TRUE;
    }

    bool		has_icon;

    QString		current_tip;
    QPixmap		current_icon;
    QPopupMenu*		current_popup;

    HICON		hIcon;
    HWND		hWnd;
};

/*!
  \class QTrayApplication qtrayapplication.h
  \brief The QTrayApplication class provides an API to add a Qt application to the system tray.
*/

/*!
  Creates a QTrayApplication object. \a argc and \a argv are propagated
  to the QApplication destructor.
*/
QTrayApplication::QTrayApplication( int argc, char **argv )
: QApplication( argc, argv )
{
    d = new QTrayApplicationPrivate;
}

/*!
  Removes the icon from the system tray and frees all allocated resources.
*/
QTrayApplication::~QTrayApplication()
{
    delete d;
}

/*!
  Shows the current icon in the system tray if \a on is TRUE, otherwise hides
  the tray icon.
*/
void QTrayApplication::showInTray( bool on )
{
    if ( on )
	d->update( TRUE );
    else
	d->remove();
}

/*!
  Returns whether the system tray icon is currently visible.
*/
bool QTrayApplication::isInTray() const
{
    return d->has_icon;
}

/*!
  Sets the system tray icon to \a icon and replaces any previous entry in the tray.
  Setting the icon to a NULL pixmap removes the system tray entry.
*/
void QTrayApplication::setTrayIcon( const QPixmap &icon )
{
    if ( d->hIcon )
	DestroyIcon( d->hIcon );

    d->current_icon = icon;

    if ( icon.isNull() ) {
	d->remove();
    } else {
	QImage img = icon.convertToImage();
	d->hIcon = CreateIcon( qWinAppInst(), icon.width(), icon.height(), 1, img.depth(), 0, img.bits() );

	d->update();
    }
}

/*!
  Returns the current system tray icon.
*/
QPixmap QTrayApplication::trayIcon() const
{
    return d->current_icon;
}

/*!
  Sets the context menu to \a popup. The context menu will pop up when the
  user clicks the system tray icon with the right mouse button.
*/
void QTrayApplication::setPopup( QPopupMenu *popup )
{
    d->current_popup = popup;
    d->hWnd = 0;

    d->update();
}

/*!
  Returns the current context menu.
*/
QPopupMenu *QTrayApplication::popup() const
{
    return d->current_popup;
}

/*!
  Sets the tooltip for the system tray entry to \a tip. On some systems, the 
  tooltip's length is limited and will be truncated as necessary.
*/
void QTrayApplication::setToolTip( const QString &tip )
{
    d->current_tip = tip;

    d->update();
}

/*!
  Returns the current tray tooltip.
*/
QString QTrayApplication::toolTip() const
{
    return d->current_tip;
}

/*!
  \reimp
*/
bool QTrayApplication::winEventFilter( MSG *m )
{
    if (!d->has_icon || m->hwnd != d->hWnd )
	return QApplication::winEventFilter( m );

    switch(m->message) {
    case WM_DRAWITEM:
	return d->iconDrawItem( (LPDRAWITEMSTRUCT)m->lParam );
    case MYWM_NOTIFYICON:
	switch (m->lParam)
	{
	case WM_LBUTTONUP:
	    emit clicked( QCursor::pos() );
	    break;
	case WM_LBUTTONDBLCLK:
	    emit doubleClicked( QCursor::pos() );
	    break;
	case WM_RBUTTONUP:
	case WM_CONTEXTMENU:
	    if ( d->current_popup ) {
		d->current_popup->grabMouse();
		if ( mainWidget() )
		    mainWidget()->setActiveWindow();
		d->current_popup->exec( QCursor::pos() );
		d->current_popup->releaseMouse();
	    }
	    break;
	default:
	    break;
	}
	break;
    default:
	break;
    }

    return QApplication::winEventFilter( m );
}

/*!
  \reimp

  Removes the entry from the system tray and quits the application.
*/
void QTrayApplication::quit()
{
    d->remove();

    QApplication::quit();
}

/*!
  \reimp
*/
void QTrayApplication::setMainWidget( QWidget *w )
{
    d->hWnd = 0;
    d->update();

    QApplication::setMainWidget( w );
}

/*!
  \fn void QTrayApplication::clicked( const QPoint &p )

  This signal is emitted when the user clicks the system tray icon
  with the left mouse button, with \a p being the global mouse position 
  at that moment.
*/

/*!
  \fn void QTrayApplication::doubleClicked( const QPoint &p )
  
  This signal is emitted when the user doubleclicks the system tray
  icon with the left mouse button, with \a p being the global mouse position 
  at that moment.
*/
