#include "parser.h"
#include <qobject.h>
#include <qstringlist.h>

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

QString Parser::cleanArgs( const QString &func )
{
    QString slot( func );
    int begin = slot.find( "(" ) + 1;
    QString args = slot.mid( begin );
    args = args.left( args.find( ")" ) );
    QStringList lst = QStringList::split( ',', args );
    QString res = slot.left( begin );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( it != lst.begin() )
	    res += ",";
	QString arg = *it;
	int pos = 0;
	if ( ( pos = arg.find( "&" ) ) != -1 ) {
	    arg = arg.left( pos + 1 );
	} else if ( ( pos = arg.find( "*" ) ) != -1 ) {
	    arg = arg.left( pos + 1 );
	} else {
	    arg = arg.simplifyWhiteSpace();
	    QStringList l = QStringList::split( ' ', arg );
	    if ( l.count() == 2 ) {
		if ( l[ 0 ] != "const" && l[ 0 ] != "unsigned" && l[ 0 ] != "var" )
		    arg = l[ 0 ];
	    } else if ( l.count() == 3 ) {
		arg = l[ 0 ] + " " + l[ 1 ];
	    }
	}
	res += arg;
    }	
    res += ")";

    return QString::fromLatin1( NormalizeObject::normalizeSignalSlot( res.latin1() ) );
}
