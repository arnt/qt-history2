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
	bool res = FALSE;

	if ( current_popup || qApp->mainWidget() ) {
	    res = trayMessage( has_icon ? NIM_MODIFY : NIM_ADD );
	    has_icon = TRUE;
	}

	return res;
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

QTrayApplication::QTrayApplication( int argc, char **argv )
: QApplication( argc, argv )
{
    d = new QTrayApplicationPrivate;
}

void QTrayApplication::setIcon( const QPixmap &icon )
{
    if ( d->hIcon )
	DestroyIcon( d->hIcon );

    d->current_icon = icon;

    QImage img = icon.convertToImage();
    d->hIcon = CreateIcon( qWinAppInst(), icon.width(), icon.height(), 1, img.depth(), 0, img.bits() );

    d->update();
}

QPixmap QTrayApplication::icon() const
{
    return d->current_tip;
}

void QTrayApplication::setPopup( QPopupMenu *popup )
{
    d->current_popup = popup;

    d->update();
}

QPopupMenu *QTrayApplication::popup() const
{
    return d->current_popup;
}

void QTrayApplication::setToolTip( const QString &tip )
{
    d->current_tip = tip;

    d->update();
}

QString QTrayApplication::toolTip() const
{
    return d->current_tip;
}

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

void QTrayApplication::quit()
{
    d->trayMessage( NIM_DELETE );
    d->has_icon = FALSE;

    QApplication::quit();
}
