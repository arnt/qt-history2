/*
  metaresolver.cpp
*/

#include "metaresolver.h"

QString MetaResolver::resolve( const QString& name ) const
{
    QString link = r->resolve( name );
    if ( !link.isEmpty() )
	return link;

    int k = name.find( QString("::") );
    if ( k == -1 )
	return QString::null;

    QString className = name.left( k );
    QString member = name.mid( k + 2 );

    QMap<QString, QMap<QString, QString> >::ConstIterator f =
	    mfunctions.find( className );
    QMap<QString, QString>::ConstIterator g;

    if ( f != mfunctions.end() &&
	 (g = (*f).find(member + QString("()"))) != (*f).end() ) {
	return *g;
    } else if ( cinherits.contains(className) ) {
	StringSet ss = cinherits[className];
	QStringList::ConstIterator s = ss.begin();
	while ( s != ss.end() ) {
	    link = r->resolve( *s + QString("::") + member );
	    if ( !link.isEmpty() )
		return link;
	    ++s;
	}
    }
    return QString::null;
}
