#include "qsqlextension_p.h"

#ifndef QT_NO_SQL
QSqlExtension::QSqlExtension()
    : bindm( BindByPosition )
{
}

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
    bindm = BindByName;
    values[ placeholder ] = val;
}

void QSqlExtension::bindValue( int pos, const QVariant& val )
{
    bindm = BindByPosition;
    index[ pos ] = QString::number( pos );
    values[ QString::number( pos ) ] = val;
}

void QSqlExtension::addBindValue( const QVariant& val )
{
    bindm = BindByPosition;
    bindValue( index.count(), val );
}

void QSqlExtension::clearValues()
{
    index.clear();
    values.clear();
}

QSqlExtension::BindMethod QSqlExtension::bindMethod()
{
    return bindm;
}
#endif
