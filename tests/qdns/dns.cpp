#include <iostream.h>
#include "qapplication.h"
#include "qdns.h"


typedef QValueList<QHostAddress> HostAddresses;
typedef QValueList<QDns::MailServer> MailServer;
typedef QValueList<QDns::Server> Server;


class Reporter : public QObject
{
    Q_OBJECT
public:
    Reporter() {}
    ~Reporter() {}

    QDns *dns;

public slots:
    void reportA()
    {
	HostAddresses list = dns->addresses();
	cout << "Found " << list.count() << " results" << endl;
	HostAddresses::Iterator it;
	for( it = list.begin(); it != list.end(); ++it )
	    cout << (*it).toString().latin1() << endl;
	qApp->quit();
    }

    void reportAaaa()
    {
	HostAddresses list = dns->addresses();
	cout << "Found " << list.count() << " results" << endl;
	HostAddresses::Iterator it;
	for( it = list.begin(); it != list.end(); ++it )
	    cout << (*it).toString().latin1() << endl;
	qApp->quit();
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
	qApp->quit();
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
	qApp->quit();
    }

    void reportCname()
    {
	QString cname = dns->canonicalName();
	cout << "Found cname: " << cname.latin1() << endl;
	qApp->quit();
    }

    void reportPtr()
    {
	QStringList list = dns->hostNames();
	cout << "Found " << list.count() << " results" << endl;
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << (*it).latin1() << endl;
	}
	qApp->quit();
    }

    void reportTxt()
    {
	QStringList list = dns->texts();
	cout << "Found " << list.count() << " results" << endl;
	QStringList::Iterator it;
	for( it = list.begin(); it != list.end(); ++it ) {
	    cout << (*it).latin1() << endl;
	}
	qApp->quit();
    }
};


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    if ( argc != 3 ) {
	cerr << "you have to specify two arguments" << endl;
	return -1;
    }

    QDns dns;
    Reporter reporter;
    reporter.dns = &dns;
    dns.setLabel( argv[1] );
    // what record type? and start query
    if        ( strcmp( argv[2], "a" ) == 0) {
	dns.setRecordType( QDns::A );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportA()) );
    } else if ( strcmp( argv[2], "aaaa" ) == 0) {
	dns.setRecordType( QDns::Aaaa );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportAaaa()) );
    } else if ( strcmp( argv[2], "mx" ) == 0) {
	dns.setRecordType( QDns::Mx );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportMx()) );
    } else if ( strcmp( argv[2], "srv" ) == 0) {
	dns.setRecordType( QDns::Srv );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportSrv()) );
    } else if ( strcmp( argv[2], "cname" ) == 0) {
	dns.setRecordType( QDns::Cname );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportCname()) );
    } else if ( strcmp( argv[2], "ptr" ) == 0) {
	QHostAddress address;
	if ( address.setAddress( argv[1] ) )
	    dns.setLabel( address );
	dns.setRecordType( QDns::Ptr );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportPtr()) );
    } else if ( strcmp( argv[2], "txt" ) == 0) {
	dns.setRecordType( QDns::Txt );
	QObject::connect( &dns, SIGNAL(resultsReady()),
		&reporter, SLOT(reportTxt()) );
    } else {
	cerr << "unknown record type" << endl;
	return -1;
    }

    // report qualifiedNames
    QStringList list = dns.qualifiedNames();
    cout << "Qualified Names: " << endl;
    QStringList::Iterator it;
    for( it = list.begin(); it != list.end(); ++it ) {
	cout << "  " << (*it).latin1() << endl;
    }

    return a.exec();
}

#include "dns.moc"
