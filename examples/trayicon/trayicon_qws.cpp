#include "trayicon.h"
#include <qwsdisplay_qws.h>
#include <unistd.h>

#if 0 // needs stuff from Qt 2.3.0 to be integrated in

static int nextid=0;

class TrayIcon::TrayIconPrivate : public QCopChannel {
    Q_OBJECT
public:
    TrayIconPrivate(QObject* parent) :
	QCopChannel("Qt/Tray",parent)
    {
	id = (getpid()<<16)+nextid++;
    }

    ~TrayIconPrivate()
    {
	QByteArray data;
	QDataStream stream( data, IO_WriteOnly );
	stream << id;
qDebug("remove %d",id);
	QCopChannel::send("Qt/Tray", "remove(int)", data);
    }

    void receive( const QCString &msg, const QByteArray &data )
    {
	QDataStream stream( data, IO_ReadOnly );
	int i;
	QPoint pos;
	stream >> i;
	if ( i == id ) {
	    if ( msg == "popup(int,QPoint)" ) {
		stream >> pos;
		emit popup(pos);
	    } else if ( msg == "clicked(int,QPoint)" ) {
		stream >> pos;
		emit clicked(pos);
	    } else if ( msg == "doubleClicked(int,QPoint)" ) {
		stream >> pos;
		emit doubleClicked(pos);
	    }
	}
    }

    void setIcon(const QPixmap& p)
    {
	QByteArray data;
	QDataStream stream( data, IO_WriteOnly );
	stream << id << p;
	QCopChannel::send("Qt/Tray", "setIcon(int,QPixmap)", data);
    }

    void setToolTip(const QString& t)
    {
	QByteArray data;
	QDataStream stream( data, IO_WriteOnly );
	stream << id << t;
	QCopChannel::send("Qt/Tray", "setToolTip(int,QString)", data);
    }

    int id;

signals:
    void popup(const QPoint&);
    void clicked(const QPoint&);
    void doubleClicked(const QPoint&);
};

void TrayIcon::sysInstall()
{
    d = new TrayIconPrivate(this);
    connect(d, SIGNAL(popup(const QPoint&)), this, SLOT(doPopup(const QPoint&)));
    connect(d, SIGNAL(clicked(const QPoint&)), this, SIGNAL(clicked(const QPoint&)));
    connect(d, SIGNAL(doubleClicked(const QPoint&)), this, SIGNAL(doubleClicked(const QPoint&)));
}

void TrayIcon::sysRemove()
{
    delete d;
}

void TrayIcon::sysUpdateIcon()
{
    d->setIcon(pm);
}

void TrayIcon::sysUpdateToolTip()
{
    d->setToolTip(tip);
}

#include "qtrayicon_qws.moc"

#else

void TrayIcon::sysInstall()
{
}

void TrayIcon::sysRemove()
{
}

void TrayIcon::sysUpdateIcon()
{
}

void TrayIcon::sysUpdateToolTip()
{
}

#endif
