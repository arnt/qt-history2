#include "qtrayicon.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qimage.h"
#include "qcursor.h"

class QTrayIcon::QTrayIconPrivate : public QWidget
{
    Q_OBJECT

public:
    QTrayIconPrivate(QTrayIcon *) : QWidget()
    {
    }
};

#include "qtrayicon_mac.moc"

void QTrayIcon::sysInstall()
{
    d = new QTrayIconPrivate(this);
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
