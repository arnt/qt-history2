#include "qtrayicon.h"
#include "qwsdisplay_qws.h"
#include <unistd.h>

static int nextid=0;

class QTrayIcon::QTrayIconPrivate : public QCopChannel {
    Q_OBJECT
public:
    QTrayIconPrivate(QObject* parent) :
	QCopChannel("Qt/Tray",parent)
    {
	id = (getpid()<<16)+nextid++;
    }

    ~QTrayIconPrivate()
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

void QTrayIcon::sysInstall()
{
    d = new QTrayIconPrivate(this);
    connect(d, SIGNAL(popup(const QPoint&)), this, SLOT(doPopup(const QPoint&)));
    connect(d, SIGNAL(clicked(const QPoint&)), this, SIGNAL(clicked(const QPoint&)));
    connect(d, SIGNAL(doubleClicked(const QPoint&)), this, SIGNAL(doubleClicked(const QPoint&)));
}

void QTrayIcon::sysRemove()
{
    delete d;
}

void QTrayIcon::sysUpdateIcon()
{
    d->setIcon(pm);
}

void QTrayIcon::sysUpdateToolTip()
{
    d->setToolTip(tip);
}

#include "qtrayicon_qws.moc"
