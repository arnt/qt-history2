#include "qtrayicon.h"
#include <qwidget.h>

class QTrayIcon::QTrayIconPrivate : public QWidget
{
    Q_OBJECT

public:
    QTrayIconPrivate()
	: QWidget()
    {
    }
};

#include "qtrayicon_x11.moc"

void QTrayIcon::sysInstall()
{
    d = new QTrayIconPrivate();
}

void QTrayIcon::sysRemove()
{
    delete d;
}

void QTrayIcon::sysUpdateIcon()
{
}

void QTrayIcon::sysUpdateToolTip()
{
}
