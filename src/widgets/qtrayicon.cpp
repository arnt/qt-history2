#include "qtrayicon.h"
#include "qpopupmenu.h"

/*!
  \class QTrayWidget qtraywidget.h
  \brief The QTrayWidget class provides an API to add icons to the system tray.
*/

/*!
  Creates a QTrayIcon object. \a parent and \a name are propagated
  to the QObject constructor. The icon is initially invisible.

  \sa show
*/
QTrayIcon::QTrayIcon( QObject *parent, const char *name )
: QObject(parent, name), pop(0), d(0)
{
}

/*!
  Creates a QTrayIcon object displaying \a icon and \a tooltip, and opening
  \a popup when clicked with the right mousebutton. \a parent and \a name are 
  propagated to the QObject constructor. The icon is initially invisible.

  \sa show
*/
QTrayIcon::QTrayIcon( const QPixmap &icon, const QString &tooltip, QPopupMenu *popup, QObject *parent, const char *name ) 
: QObject(parent, name), pop(popup), pm(icon), tip(tooltip), d(0)
{
}

/*!
  Removes the icon from the system tray and frees all allocated resources.
*/
QTrayIcon::~QTrayIcon()
{
    sysRemove();
}

/*!
  Sets the context menu to \a popup. The context menu will pop up when the
  user clicks the system tray entry with the right mouse button.
*/
void QTrayIcon::setPopup( QPopupMenu* p )
{
    pop = p;
}

/*!
  Returns the current popup menu.

  \sa setPopup
*/
QPopupMenu* QTrayIcon::popup() const
{
    return pop;
}

/*!
  Sets the system tray icon to \a icon and replaces any previous entry in the tray.
*/
void QTrayIcon::setIcon( const QPixmap &icon )
{
    pm = icon;
    sysUpdateIcon();
}

/*!
  Returns the current icon.

  \sa setIcon
*/
QPixmap QTrayIcon::icon() const
{
    return pm;
}

/*!
  Sets the tooltip for the system tray entry to \a tip. On some systems, 
  the tooltip's length is limited and will be truncated as necessary.
*/
void QTrayIcon::setToolTip( const QString &tooltip )
{
    tip = tooltip;
    sysUpdateToolTip();
}

/*!
  Returns the current tooltip.

  \sa setToolTip
*/
QString QTrayIcon::toolTip() const
{
    return tip;
}

/*!
  Shows the icon in the system tray.

  \sa hide
*/
void QTrayIcon::show()
{
    sysInstall();
}

/*!
  Hides the system tray entry.
*/
void QTrayIcon::hide()
{
    sysRemove();
}

/*!
  \reimp
*/
bool QTrayIcon::event( QEvent *e )
{
    switch ( e->type() ) {
    case QEvent::MouseMove:
	mouseMoveEvent( (QMouseEvent*)e );
	break;

    case QEvent::MouseButtonPress:
	mousePressEvent( (QMouseEvent*)e );
	break;

    case QEvent::MouseButtonRelease:
	mouseReleaseEvent( (QMouseEvent*)e );
	break;

    case QEvent::MouseButtonDblClick:
	mouseDoubleClickEvent( (QMouseEvent*)e );
	break;
    default:
	return QObject::event( e );
    }

    return TRUE;
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse move events for the system tray entry.

  \sa mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),  QMouseEvent
*/
void QTrayIcon::mouseMoveEvent( QMouseEvent *e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse press events for the system tray entry.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), QMouseEvent
*/
void QTrayIcon::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse release events for the system tray entry.

  The default implementations opens the context menu when the entry
  has been clicked with the right mouse button.

  \sa setPopup(), mousePressEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), QMouseEvent
*/
void QTrayIcon::mouseReleaseEvent( QMouseEvent *e )
{
    switch ( e->button() ) {
    case RightButton:
	if ( pop ) {
	    // Those lines are necessary to make keyboard focus
	    // and menu closing work on Windows.
	    pop->setActiveWindow();
	    pop->grabMouse();
	    pop->exec( e->globalPos() );
	    pop->releaseMouse();
	    pop->setActiveWindow();
	    e->accept();
	}
	break;
    case LeftButton:
	emit clicked( e->globalPos() );
	break;
    }
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse double click events for the system tray entry.

  Note that the system tray entry gets a mousePressEvent() and a 
  mouseReleaseEvent() before the mouseDoubleClickEvent().

  \sa mousePressEvent(), mouseReleaseEvent(),
  mouseMoveEvent(), QMouseEvent
*/
void QTrayIcon::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	emit doubleClicked( e->globalPos() );
    e->ignore();
}

/*!
  \fn void QTrayIcon::clicked( const QPoint &p )

  This signal is emitted when the user clicks the system tray icon
  with the left mouse button, with \a p being the global mouse position 
  at that moment.
*/

/*!
  \fn void QTrayIcon::doubleClicked( const QPoint &p )
  
  This signal is emitted when the user double clicks the system tray
  icon with the left mouse button, with \a p being the global mouse position 
  at that moment.
*/
