/****************************************************************************
**
** Implementation of the QSqlExtension class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsqlextension_p.h"

#ifndef QT_NO_SQL
QSqlExtension::QSqlExtension()
    : bindm( BindByPosition ), bindCount( 0 )
{
}

QSqlExtension::~QSqlExtension()
{
}

bool QSqlExtension::prepare( const QString& /*query*/ )
{
    return FALSE;
}

bool QSqlExtension::exec()
{
    return FALSE;
}

void QSqlExtension::bindValue( const QString& placeholder, const QVariant& val, QSql::ParameterType tp )
{
    bindm = BindByName;
    // if the index has already been set when doing emulated named
    // bindings - don't reset it
    if ( index[ values.count() ].isEmpty() ) {
	index[ values.count() ] = placeholder;
    }
    values[ placeholder ] = Param( val, tp );
}

void QSqlExtension::bindValue( int pos, const QVariant& val, QSql::ParameterType tp )
{
    bindm = BindByPosition;
    index[ pos ] = QString::number( pos );
    QString nm = QString::number( pos );
    values[ nm ] = Param( val, tp );
}

void QSqlExtension::addBindValue( const QVariant& val, QSql::ParameterType tp )
{
    bindm = BindByPosition;
    bindValue( bindCount++, val, tp );
}

void QSqlExtension::clearValues()
{
    values.clear();
    bindCount = 0;
}

void QSqlExtension::resetBindCount()
{
    bindCount = 0;
}

void QSqlExtension::clearIndex()
{
    index.clear();
    holders.clear();
}

void QSqlExtension::clear()
{
    clearValues();
    clearIndex();
}

QVariant QSqlExtension::parameterValue( const QString& holder )
{
    return values[ holder ].value;
}

QVariant QSqlExtension::parameterValue( int pos )
{
    return values[ index[ pos ] ].value;
}

QVariant QSqlExtension::boundValue( const QString& holder ) const
{
    return values[ holder ].value;
}

QVariant QSqlExtension::boundValue( int pos ) const
{
    return values[ index[ pos ] ].value;
}

QMap<QString, QVariant> QSqlExtension::boundValues() const
{
    QMap<QString, Param>::ConstIterator it;
    QMap<QString, QVariant> m;
    if ( bindm == BindByName ) {
	for ( it = values.begin(); it != values.end(); ++it )
	    m.insert( it.key(), it.data().value );
    } else {
	QString key, tmp, fmt;
	fmt.sprintf( "%%0%dd", QString::number( values.count()-1 ).length() );
	for ( it = values.begin(); it != values.end(); ++it ) {
	    tmp.sprintf( fmt.ascii(), it.key().toInt() );
	    m.insert( tmp, it.data().value );
	}
    }
    return m;
}

QSqlExtension::BindMethod QSqlExtension::bindMethod()
{
    return bindm;
}

QSqlDriverExtension::QSqlDriverExtension()
{
}

QSqlDriverExtension::~QSqlDriverExtension()
{
}

QSqlOpenExtension::QSqlOpenExtension()
{
}

QSqlOpenExtension::~QSqlOpenExtension()
{
}
#endif
