#include "qsqlextension_p.h"

#ifndef QT_NO_SQL

QSqlExtension::~QSqlExtension()
{
}

bool QSqlExtension::prepare( const QString& /*query*/ )
{
    clearValues();
    return FALSE;
}

bool QSqlExtension::exec()
{
    return FALSE;
}

void QSqlExtension::bindValue( const QString& placeholder, const QVariant& val )
{
    values[ placeholder ] = val;
}

void QSqlExtension::bindValue( int pos, const QVariant& val )
{
    index[ pos ] = QString::number( pos );
    values[ QString::number( pos ) ] = val;
}

void QSqlExtension::addBindValue( const QVariant& val )
{
    bindValue( index.count(), val );
}

void QSqlExtension::clearValues()
{
    index.clear();
    values.clear();
}

QVariant QSqlExtension::value( const QString& holder )
{
    QMapConstIterator<QString, QVariant> it;
    if ( (it = values.find( holder )) != values.end() ) {
	return it.data();
    }
    return QVariant();
}

QVariant QSqlExtension::value( int i )
{
    QMapConstIterator<int, QString> it;
    if ( (it = index.find( i )) != index.end() ) {
	return values[ it.data() ];
    }
    return QVariant();
}
#endif
