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

    if ( mfunctions[className].contains(member) ) {
	return QString( "#f" ) + mfunctions[className][member];
    } else if ( cinherits.contains(className) ) {
	QStringList::ConstIterator s = cinherits[className].begin();
	while ( s != cinherits[className].end() ) {
	    link = r->resolve( *s + QString("::") + member );
	    if ( !link.isEmpty() )
		return link;
	    ++s;
	}
    }
    return QString::null;
}
