#ifndef QRESOLVER_H
#define QRESOLVER_H

#include <qobject.h>
#include <qhostaddress.h>
#include <qlist.h>

class Q_NETWORK_EXPORT QResolver
{
public:
    enum Error { NoError, HostNotFound, UnknownError };

    struct HostInfo
    {
	HostInfo() : error( QResolver::NoError ), errorString( "Unknown error" )
	{}

	HostInfo( const HostInfo &d ) : error( d.error ), errorString( d.errorString ), addresses( d.addresses )
	{}

	Error error;
	QString errorString;
	QList<QHostAddress> addresses;
    };


    static void getHostByName( const QString& name, const QObject * receiver, const char * resultsReady );
};

#endif // QRESOLVER_H
