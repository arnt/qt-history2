#include <qserversocket.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtextstream.h>

class HttpDaemon : public QServerSocket {
    Q_OBJECT
public:
    HttpDaemon( QObject* parent=0 ) :
	QServerSocket(8080,0,parent)
    {
	if ( !ok() ) {
	    qWarning("Failed to bind to port 8080");
	    exit(1);
	}
    }

    void newConnection( int socket )
    {
	qDebug("new connection");
	QSocket* s = new QSocket(this);
	connect(s,SIGNAL(readyRead()),this,SLOT(readClient()));
	connect(s,SIGNAL(delayedCloseFinished()),this,SLOT(discardClient()));
	s->setSocket(socket);
	s->setMode(QSocket::Ascii);
    }

private slots:
    void readClient()
    {
	QSocket* socket = (QSocket*)sender();
	if (socket->canReadLine()) {
	    QStringList tokens = QStringList::split(QRegExp("[ \n\r][ \n\r]*"),socket->readLine());
	    if ( tokens[0] == "GET" ) {
		QTextStream os(socket);
		os << "<h1>Nothing to see here</h1>\n";
		socket->close();
	    }
	}
    }
    void discardClient()
    {
	QSocket* socket = (QSocket*)sender();
	delete socket;
    }
};

main(int argc, char** argv)
{
    QApplication app(argc,argv);
    HttpDaemon httpd;
    return app.exec();
}

#include "httpd.moc"
