#include "qtrayicon.h"
#include "qpopupmenu.h"

QTrayIcon::QTrayIcon( QObject *parent, const char *name ) :
    QObject(parent, name),
    pop(0), d(0)
{
    sysInstall();
}

QTrayIcon::~QTrayIcon()
{
    sysRemove();
}

/*!
    Set \a p to be the popup menu invoked by the icon.
*/
void QTrayIcon::setPopup( QPopupMenu* p )
{
    pop = p;
}

QPopupMenu* QTrayIcon::popup() const
{
    return pop;
}

QPixmap QTrayIcon::icon() const
{
    return pm;
}

QString QTrayIcon::toolTip() const
{
    return tip;
}

void QTrayIcon::setIcon( const QPixmap &icon )
{
    pm = icon;
    sysUpdateIcon();
}

void QTrayIcon::setToolTip( const QString &tooltip )
{
    tip = tooltip;
    sysUpdateToolTip();
}

void QTrayIcon::doPopup( const QPoint& pos )
{
    QSize sz = pop->sizeHint();
    QPoint p = pos-QPoint(0,sz.height());
    pop->popup(p,-1);
}
