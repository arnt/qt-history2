#include "qtrayicon.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qimage.h"
#include "qcursor.h"

#include "qt_windows.h"

static uint MYWM_TASKBARCREATED = 0;
#define MYWM_NOTIFYICON	(WM_APP+101)

extern Qt::WindowsVersion qt_winver;

class QTrayIcon::QTrayIconPrivate : public QWidget
{
    Q_OBJECT

public:
    QTrayIconPrivate( QTrayIcon *object ) 
	: QWidget( 0 ), hIcon( 0 ), iconObject( object )
    {
	if ( !MYWM_TASKBARCREATED ) {
#if defined(UNICODE)
	    if ( qt_winver & Qt::WV_NT_based )
		MYWM_TASKBARCREATED = RegisterWindowMessageW( (const unsigned short*)"TaskbarCreated" );
	    else
#endif
		MYWM_TASKBARCREATED = RegisterWindowMessageA( "TaskbarCreated" );
	}
    }

    ~QTrayIconPrivate()
    {
	if ( hIcon )
	    DestroyIcon( hIcon );
    }

    // the unavoidable A/W versions. Don't forget to keep them in sync!
    bool trayMessageA( DWORD msg ) 
    {
	bool res;

	NOTIFYICONDATAA tnd;
	memset( &tnd, 0, sizeof(NOTIFYICONDATAA) );
	tnd.cbSize		= sizeof(NOTIFYICONDATAA);
	tnd.hWnd		= winId();

	if ( msg != NIM_DELETE ) {
	    tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	    tnd.uCallbackMessage= MYWM_NOTIFYICON;
	    tnd.hIcon		= hIcon;
	    if ( !iconObject->toolTip().isNull() ) {
		// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
		QString tip = iconObject->toolTip().left( 63 ) + QChar();
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
	tnd.hWnd		= winId();

	if ( msg != NIM_DELETE ) {
	    tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	    tnd.uCallbackMessage= MYWM_NOTIFYICON;
	    tnd.hIcon		= hIcon;
	    if ( !iconObject->toolTip().isNull() ) {
		// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
		QString tip = iconObject->toolTip().left( 63 ) + QChar();
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

    bool winEvent( MSG *m )
    {
	switch(m->message) {
	case WM_DRAWITEM:
	    return iconDrawItem( (LPDRAWITEMSTRUCT)m->lParam );
	case MYWM_NOTIFYICON:
	    {
		QMouseEvent *e = 0;
		QPoint gpos = QCursor::pos();
		switch (m->lParam)
		{
		case WM_MOUSEMOVE:
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
		    e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
		    break;
		default:
		    break;
		}
		if ( e ) {
		    bool res = QApplication::sendEvent( iconObject, e );
		    delete e;
		    return res;
		}
	    }
	    break;
	default:
	    if ( m->message == MYWM_TASKBARCREATED ) {
		#if defined(UNICODE)
		    if ( qt_winver & Qt::WV_NT_based )
			trayMessageW( NIM_ADD );
		    else
		#endif
			trayMessageA( NIM_ADD );
	    }
	    break;
	}

	return QWidget::winEvent( m );
    }

    HICON		hIcon;
    QTrayIcon		*iconObject;
};

#include "qtrayicon_win.moc"

void QTrayIcon::sysInstall()
{
    if ( d )
	return;

    d = new QTrayIconPrivate( this );
    QImage img = pm.convertToImage();
    d->hIcon = CreateIcon( qWinAppInst(), img.width(), img.height(), 1, img.depth(), 0, img.bits() );

#if defined(UNICODE)
    if ( qt_winver & Qt::WV_NT_based )
	d->trayMessageW( NIM_ADD );
    else
#endif
	d->trayMessageA( NIM_ADD );
}

void QTrayIcon::sysRemove()
{
    if ( !d )
	return;

#if defined(UNICODE)
    if ( qt_winver & Qt::WV_NT_based )
	d->trayMessageW( NIM_DELETE );
    else
#endif
	d->trayMessageA( NIM_DELETE );

    delete d;
    d = 0;
}

void QTrayIcon::sysUpdateIcon()
{
    if ( !d )
	return;

    if ( d->hIcon )
	DestroyIcon( d->hIcon );

    QImage img = pm.convertToImage();
    d->hIcon = CreateIcon( qWinAppInst(), img.width(), img.height(), 1, img.depth(), 0, img.bits() );

#if defined(UNICODE)
    if ( qt_winver & Qt::WV_NT_based )
	d->trayMessageW( NIM_MODIFY );
    else
#endif
	d->trayMessageA( NIM_MODIFY );
}

void QTrayIcon::sysUpdateToolTip()
{
    if ( !d )
	return;

#if defined(UNICODE)
    if ( qt_winver & Qt::WV_NT_based )
	d->trayMessageW( NIM_MODIFY );
    else
#endif
	d->trayMessageA( NIM_MODIFY );
}
