#include <iostream.h>
#include <qapplication.h>
#include <qdns.h>
#include <qtimer.h>
#include <qvaluelist.h>

typedef QList<QHostAddress> HostAddresses;
typedef QList<QDns::MailServer> MailServer;
typedef QList<QDns::Server> Server;


class Reporter : public QObject
{
    Q_OBJECT
public:
    Reporter( const char *l, const char *recType, bool synchronous ) :
	dns(0), recordType(recType), label(l), sync(synchronous)
    {
    }
    ~Reporter()
    {
	delete dns;
    }

public slots:
    void start()
    {
	dns = new QDns;
	dns->setLabel( label );
	// what record type? and start query
	if        ( recordType == "a" ) {
	    dns->setRecordType( QDns::A );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportA()) );
	} else if ( recordType == "aaaa" ) {
	    dns->setRecordType( QDns::Aaaa );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportAaaa()) );
	} else if ( recordType == "mx" ) {
	    dns->setRecordType( QDns::Mx );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportMx()) );
	} else if ( recordType == "srv" ) {
	    dns->setRecordType( QDns::Srv );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportSrv()) );
	} else if ( recordType == "cname" ) {
	    dns->setRecordType( QDns::Cname );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportCname()) );
	} else if ( recordType == "ptr" ) {
	    QHostAddress address;
	    if ( address.setAddress( label ) )
		dns->setLabel( address );
	    dns->setRecordType( QDns::Ptr );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportPtr()) );
	} else if ( recordType == "txt" ) {
	    dns->setRecordType( QDns::Txt );
	    QObject::connect( dns, SIGNAL(resultsReady()),
		    SLOT(reportTxt()) );
	} else {
	    cerr << "unknown record type" << endl;
	}

	// report qualifiedNames
	QStringList list = dns->qualifiedNames();
	cout << "Qualified Names: " << endl;
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << "  " << (*it).latin1() << endl;
	}
    }

    void reportA()
    {
	HostAddresses list = dns->addresses();
	cout << "Found " << list.count() << " results" << endl;
	HostAddresses::Iterator it;
	for( it = list.begin(); it != list.end(); ++it )
	    cout << (*it).toString().latin1() << endl;
	cout << endl;
	reportCname();
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

    void reportAaaa()
    {
	HostAddresses list = dns->addresses();
	cout << "Found " << list.count() << " results" << endl;
	HostAddresses::Iterator it;
	for( it = list.begin(); it != list.end(); ++it )
	    cout << (*it).toString().latin1() << endl;
	cout << endl;
	reportCname();
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

    void reportMx()
    {
	MailServer list = dns->mailServers();
	cout << "Found " << list.count() << " results" << endl;
	MailServer::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << (*it).name.latin1() <<
		" (" << (*it).priority << ")" << endl;
	}
	cout << endl;
	reportCname();
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

    void reportSrv()
    {
	Server list = dns->servers();
	cout << "Found " << list.count() << " results" << endl;
	Server::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << (*it).name.latin1() << ":" << (*it).port <<
		" (" << (*it).priority << ")" <<
		"weight: " << (*it).weight << endl;
	}
	cout << endl;
	reportCname();
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

    void reportCname()
    {
	QString cname = dns->canonicalName();
	cout << "Found cname: " << cname.latin1() << endl;
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

    void reportPtr()
    {
	QStringList list = dns->hostNames();
	cout << "Found " << list.count() << " results" << endl;
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << (*it).latin1() << endl;
	}
	cout << endl;
	reportCname();
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

    void reportTxt()
    {
	QStringList list = dns->texts();
	cout << "Found " << list.count() << " results" << endl;
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << (*it).latin1() << endl;
	}
	cout << endl;
	reportCname();
	if ( !sync ) {
	    if ( !dns->isWorking() )
		qApp->quit();
	}
    }

private:
    QDns *dns;
    QString recordType;
    QString label;
    bool sync;
};


int main( int argc, char **argv )
{
    if ( argc < 3 ) {
	cerr << "you have to specify two arguments" << endl;
	return -1;
    }

    int ret = 0;
    if ( argc==3 ) {
	// do an asynchronous lookup
	cout << "Asynchronous lookup" << endl;
	cout << "-------------------" << endl;
	QApplication a( argc, argv );
	Reporter reporter1( argv[1], argv[2], FALSE );
	QTimer::singleShot( 0, &reporter1, SLOT(start()) );
	ret = a.exec();
	cout << endl;
    } else {
	// do a synchronous lookup
	cout << "Synchronous lookup" << endl;
	cout << "------------------" << endl;
	Reporter reporter2( argv[1], argv[2], TRUE );
	reporter2.start();
	cout << endl;
    }
    return ret;
}

#include "dns.moc"
