#include "qtraywidget.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qimage.h>
#include <qpopupmenu.h>

#include <qt_windows.h>

#define MYWM_NOTIFYICON	(WM_APP+101) // WM_APP+101 will be unique

class QTrayWidget::QTrayWidgetPrivate 
{
public:
    QTrayWidgetPrivate() : has_icon( FALSE ), wm_taskbarcreated( 0 ), current_popup( 0 ), hIcon( 0 ), hWnd( 0 )
    {
#if defined(UNICODE)
	if ( qWinVersion() & Qt::WV_NT_based )
	    wm_taskbarcreated = RegisterWindowMessageW( (const unsigned short*)"TaskbarCreated" );
	else
#endif
	    wm_taskbarcreated = RegisterWindowMessageA( "TaskbarCreated" );
    }

    ~QTrayWidgetPrivate()
    {
	remove();
	if ( hIcon )
	    DestroyIcon( hIcon );
    }

    bool update( bool show = FALSE )
    {
	if ( !has_icon && !show )
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
	tnd.hWnd		= hWnd;

	if ( msg != NIM_DELETE ) {	    
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
	tnd.hWnd		= hWnd;

	if ( msg != NIM_DELETE ) {
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
    uint		wm_taskbarcreated;

    QString		current_tip;
    QPixmap		current_icon;
    QPopupMenu*		current_popup;

    HICON		hIcon;
    HWND		hWnd;
};

/*!
  \class QTrayWidget qtraywidget.h
  \brief The QTrayWidget class provides an API to add icons to the system tray.
*/

/*!
  Creates a QTrayWidget object. \a parent and \a name are propagated
  to the QApplication destructor.
*/
QTrayWidget::QTrayWidget( QWidget *parent, const char *name )
: QWidget( parent, name )
{
    d = new QTrayWidgetPrivate;
    d->hWnd = winId();
}

/*!
  Removes the icon from the system tray and frees all allocated resources.
*/
QTrayWidget::~QTrayWidget()
{
    delete d;
}

/*!
  Shows the current icon in the system tray if \a on is TRUE, otherwise hides
  the tray icon.
*/
void QTrayWidget::show()
{
    d->update( TRUE );
    if ( d->has_icon )
	setWState( WState_Visible );
}

void QTrayWidget::hide()
{
    d->remove();
    if ( !d->has_icon )
	clearWState( WState_Visible );
}

/*!
  Sets the system tray icon to \a icon and replaces any previous entry in the tray.
*/
void QTrayWidget::setIcon( const QPixmap &icon )
{
    if ( d->hIcon )
	DestroyIcon( d->hIcon );

    d->current_icon = icon;

    QImage img = icon.convertToImage();
    d->hIcon = CreateIcon( qWinAppInst(), icon.width(), icon.height(), 1, img.depth(), 0, img.bits() );

    d->update();
}

/*!
  Returns the current system tray icon.
*/
QPixmap QTrayWidget::icon() const
{
    return d->current_icon;
}

/*!
  Sets the context menu to \a popup. The context menu will pop up when the
  user clicks the system tray icon with the right mouse button.
*/
void QTrayWidget::setPopup( QPopupMenu *popup )
{
    d->current_popup = popup;

    d->update();
}

/*!
  Returns the current context menu.
*/
QPopupMenu *QTrayWidget::popup() const
{
    return d->current_popup;
}

/*!
  Sets the tooltip for the system tray entry to \a tip. On some systems, the 
  tooltip's length is limited and will be truncated as necessary.
*/
void QTrayWidget::setToolTip( const QString &tip )
{
    d->current_tip = tip;

    d->update();
}

/*!
  Returns the current tray tooltip.
*/
QString QTrayWidget::toolTip() const
{
    return d->current_tip;
}

/*!
  \reimp
*/
bool QTrayWidget::winEvent( MSG *m )
{
    if (!d->has_icon )
	return QWidget::winEvent( m );

    switch(m->message) {
    case WM_DRAWITEM:
	return d->iconDrawItem( (LPDRAWITEMSTRUCT)m->lParam );
    case MYWM_NOTIFYICON:
	{
	    QMouseEvent *e = 0;
	    QPoint gpos = QCursor::pos();
	    switch (m->lParam)
	    {
	    case WM_MOUSEMOVE:
		if ( hasMouseTracking() )
		    e = new QMouseEvent( QEvent::MouseMove, mapFromGlobal( gpos ), gpos, 0, 0 );
		break;
	    case WM_LBUTTONDOWN:
		e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, LeftButton, LeftButton );
		break;
	    case WM_LBUTTONUP:
		e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, LeftButton, LeftButton );
		break;
	    case WM_LBUTTONDBLCLK:
		e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, LeftButton, LeftButton );
		break;
	    case WM_RBUTTONDOWN:
		e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
		break;
	    case WM_RBUTTONUP:
		e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
		break;
	    case WM_RBUTTONDBLCLK:
		e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
		break;
	    case WM_MBUTTONDOWN:
		e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, MidButton, MidButton );
		break;
	    case WM_MBUTTONUP:
		e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, MidButton, MidButton );
		break;
	    case WM_MBUTTONDBLCLK:
		e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, MidButton, MidButton );
		break;
	    case WM_CONTEXTMENU:
		if ( d->current_popup ) {
    		    d->current_popup->grabMouse();
		    if ( qApp->mainWidget() )
			qApp->mainWidget()->setActiveWindow();
		    d->current_popup->exec( QCursor::pos() );
		    d->current_popup->releaseMouse();
		}
		break;
	    default:
		break;
	    }
	    if ( e ) {
		QApplication::sendEvent( this, e );
		delete e;
	    }
	}
	break;
    default:
	if ( m->message == d->wm_taskbarcreated && d->has_icon ) {
	    d->has_icon = FALSE;
	    d->update( TRUE );
	}
	break;
    }

    return QWidget::winEvent( m );
}

/*!
  \reimp

  Removes the entry from the system tray.
*/
void QTrayWidget::closeEvent( QCloseEvent *e )
{
    hide();

    QWidget::closeEvent( e );
}

/*!
  \reimp
*/
void QTrayWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( e->button() == e->state() == LeftButton )
	emit doubleClicked( e->globalPos() );

    QWidget::mouseDoubleClickEvent( e );
}

/*!
  \reimp
*/
void QTrayWidget::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() == RightButton ) {
	if ( d->current_popup ) {
	    if ( qApp->mainWidget() )
		qApp->mainWidget()->setActiveWindow();
	    d->current_popup->grabMouse();
	    d->current_popup->exec( e->globalPos() );
	    d->current_popup->releaseMouse();
	    e->accept();
	    return;
	}
    } else if ( e->button() == e->state() == LeftButton ) {
	emit clicked( e->globalPos() );
    }

    QWidget::mouseReleaseEvent( e );
}

/*!
  \fn void QTrayWidget::clicked( const QPoint &p )

  This signal is emitted when the user clicks the system tray icon
  with the left mouse button, with \a p being the global mouse position 
  at that moment.
*/

/*!
  \fn void QTrayWidget::doubleClicked( const QPoint &p )
  
  This signal is emitted when the user doubleclicks the system tray
  icon with the left mouse button, with \a p being the global mouse position 
  at that moment.
*/
