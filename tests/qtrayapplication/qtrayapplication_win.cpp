#include "qtrayapplication.h"
#include <qcursor.h>
#include <qimage.h>
#include <qpopupmenu.h>

#include <qt_windows.h>

#define MYWM_NOTIFYICON	(WM_APP+101) // WM_APP+101 will be unique

class QTrayApplication::QTrayApplicationPrivate 
{
public:
    QTrayApplicationPrivate() : has_icon( FALSE ), current_popup( 0 ), hIcon( 0 ) 
    {
    }

    ~QTrayApplicationPrivate()
    {
	if ( hIcon )
	    DestroyIcon( hIcon );
    }

    bool update()
    {
	if ( current_popup || qApp->mainWidget() ) {
	    bool res = trayMessage( has_icon ? NIM_MODIFY : NIM_ADD );
	    has_icon = TRUE;
	    return res;
	}

	return FALSE;
    }

    bool remove()
    {
	if ( !has_icon )
	    return TRUE;

	has_icon = FALSE;
	return trayMessage( NIM_DELETE );	
    }

    bool trayMessage( DWORD msg ) 
    {
	bool res;

	NOTIFYICONDATA tnd;
	memset( &tnd, 0, sizeof(NOTIFYICONDATA) );

	tnd.cbSize		= sizeof(NOTIFYICONDATA);
	tnd.hWnd		= current_popup ? current_popup->winId() : ( qApp->mainWidget() ? qApp->mainWidget()->winId() : 0 );
	if ( !tnd.hWnd )
	    qFatal( "No widget available for notification! Call setMainWidget or setPopup first!" );

	tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon		= hIcon;
	if ( !current_tip.isNull() ) {
	    lstrcpyn(tnd.szTip, (TCHAR*)qt_winTchar( current_tip, TRUE ), sizeof(tnd.szTip));
	} else {
	    tnd.szTip[0] = '\0';
	}

	res = Shell_NotifyIcon(msg, &tnd);

	return res;
    }

    bool iconDrawItem(LPDRAWITEMSTRUCT lpdi)
    {
	if (!hIcon)
	    return FALSE;

	DrawIconEx(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, hIcon,
		16, 16, 0, NULL, DI_NORMAL);

	return TRUE;
    }

    bool		has_icon;
    QString		current_tip;
    QPixmap		current_icon;
    QPopupMenu*		current_popup;

    HICON		hIcon;    
};

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
    d->remove();
    delete d;
}

/*!
  Sets the system tray icon to \a icon.
  If \a icon is a NULL pixmap, the current icon will be removed from the system tray.
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
    return d->current_tip;
}

/*!
  Sets the context menu to \a popup. The context menu will pop up when the
  user clicks the system tray icon with the right mouse button.
*/
void QTrayApplication::setPopup( QPopupMenu *popup )
{
    d->current_popup = popup;

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
  Sets the tooltip for the system tray entry to \a tip.
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
    if (!d->has_icon)
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
  Removes the icon from the system tray and quits the application.
*/
void QTrayApplication::quit()
{
    d->remove();

    QApplication::quit();
}

/*!
  \fn void QTrayApplication::clicked( const QPoint& )

  This signal is emitted when the user clicks the system tray icon
  with the left mouse button.
*/

/*!
  \fn void QTrayApplication::doubleClicked( const QPoint& )
  
  This signal is emitted when the user doubleclicks the system tray
  icon with the left mouse button.
*/
