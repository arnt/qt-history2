/****************************************************************************
**
** Implementation of shared Qt SQL module classes
**
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsql_result.h"
#include <qptrvector.h>
#include <qdatetime.h>

QSqlClientData::~QSqlClientData()
{
}

QVariant QSqlClientData::format( void* buf, int /*size*/, QVariant::Type type ) const
{
    QVariant v;
    switch ( type ) {
    case QVariant::String:
	v = QString( (char*)buf );
	break;
    case QVariant::Int:
	v = *( (int*)buf );
	break;
    case QVariant::UInt:
	v = *( (uint*)buf );
	break;
    case QVariant::Double:
	v = *( (double*)buf );
	break;
    case QVariant::Date:
	v = *( (QDate*)buf );
	break;
    case QVariant::Time:
	v = *( (QTime*)buf );
	break;
    case QVariant::DateTime:
	v = *( (QDateTime*)buf );
	break;
    default:
	v = QString( (char*)buf );
	break;
    }
    return v;
}

QSqlClientData* QSqlClientData::clone()
{
    return new QSqlClientData();
}

/////////////

QSqlClientNullData::~QSqlClientNullData()
{
}

bool QSqlClientNullData::isNull() const
{
    return FALSE;
}

void* QSqlClientNullData::binder() const
{
    return 0;
}

QSqlClientNullData* QSqlClientNullData::clone()
{
    return new QSqlClientNullData();
}

//////////////

class QSqlClientResultBuffer::Private
{
public:
    Private( int )
    {
	frm = new QSqlClientData();
    }
    ~Private()
    {
	clear();
	delete frm;
    }
    void clear()
    {
	for ( uint i=0; i < buf.count(); ++i ) {
	    delete [] buf[i]->data;
	    delete buf[i]->nullBinder;
	    delete buf[i];
	}
	buf.clear();
    }
    Private( const Private& other )
    {
	if ( this == &other )
		return;
	for ( uint i=0; i < other.buf.count(); ++i ) {
	    char* c = (char*)append( other.buf[i]->size, other.buf[i]->type, other.buf[i]->nullBinder->clone() );
	    memcpy ( c, other.buf[i]->data, other.buf[i]->size );
	}
	frm = other.format()->clone();
    }
    void* append( int size, QVariant::Type type, QSqlClientNullData* nd )
    {
	Buf * b = new Buf;
	b->size = size;
	b->type = type;
	b->data = new char[ size ];
	if ( !nd )
	    b->nullBinder = new QSqlClientNullData();
	else {	
	    b->nullBinder = nd->clone();
	    delete nd;
	}
 	if ( buf.count() + 1 > buf.size() )
	    buf.resize( buf.size() + 10 );
	buf.insert( buf.count(), b );
	return (void*)b->data;
    }

    QSqlClientNullData* nullData( int fieldNumber ) const
    {
	if ( fieldNumber < (int)buf.count() )
	    return buf[ fieldNumber ]->nullBinder;
#ifdef QT_CHECK_RANGE
	qWarning("QSqlClientResultBuffer::Private:no such field: " + QString::number( fieldNumber ) );
#endif
	return 0;
    }
    void* data( int fieldNumber )
    {
	if ( fieldNumber < (int)buf.count() )
	    return (void*)buf[ fieldNumber ]->data;
#ifdef QT_CHECK_RANGE
	qWarning("QSqlClientResultBuffer::Private:no such field: " + QString::number( fieldNumber ) );
#endif
	return 0;
    }
    int size( int fieldNumber )
    {
	if ( fieldNumber < (int)buf.count() )
	    return buf[ fieldNumber ]->size;
#ifdef QT_CHECK_RANGE
	qWarning("QSqlClientResultBuffer::Private:no such field: " + QString::number( fieldNumber ) );
#endif
	return -1;
    }
    QVariant::Type type( int fieldNumber )
    {
	if ( fieldNumber < (int)buf.count() )
	    return buf[ fieldNumber ]->type;
#ifdef QT_CHECK_RANGE
	qWarning("QSqlClientResultBuffer::Private:no such field: " + QString::number( fieldNumber ) );
#endif
	return QVariant::Invalid;
    }

    bool isNull( int fieldNumber ) const
    {
	if ( fieldNumber < (int)buf.count() )
	    return buf[ fieldNumber ]->nullBinder->isNull();
#ifdef QT_CHECK_RANGE
	qWarning("QSqlClientResultBuffer::Private:no such field: " + QString::number( fieldNumber ) );
#endif
	return FALSE;
    }

    QSqlClientData* format() const
    {
	return frm;
    }
    void installFormat( QSqlClientData* format )
    {
	if ( frm )
	    delete frm;
	frm = format;
    }
    int count() const
    {
	return buf.count();
    }
private:
    Private& operator=( const Private& other );

    struct Buf {
	char* data;
	int size;
	QVariant::Type type;
	QSqlClientNullData* nullBinder;
    };
    QPtrVector<Buf> buf;
    QSqlClientData* frm;
};

QSqlClientResultBuffer::QSqlClientResultBuffer()
{
    d = new Private( 0 );
}

QSqlClientResultBuffer::QSqlClientResultBuffer( int numFields )
{
    d = new Private( numFields );
}

QSqlClientResultBuffer::QSqlClientResultBuffer( const QSqlClientResultBuffer& other )
{
    d = new Private( *other.d );
}

QSqlClientResultBuffer& QSqlClientResultBuffer::operator=( const QSqlClientResultBuffer& other )
{
    if ( this != &other ) {
	delete d;
	d = new Private( *other.d );
    }
    return *this;
}

QSqlClientResultBuffer::~QSqlClientResultBuffer()
{
    delete d;
}

void* QSqlClientResultBuffer::append( int size, QVariant::Type type, QSqlClientNullData* nd )
{
    return d->append( size, type, nd );
}

void QSqlClientResultBuffer::clear()
{
    d->clear();
}

QVariant QSqlClientResultBuffer::data( int fieldNumber ) const
{
    if ( !d->data( fieldNumber ) )
	return QVariant();
    QVariant v = d->format()->format( d->data( fieldNumber ),
				      d->size( fieldNumber ),
				      d->type( fieldNumber ) );
    return v;
}

bool QSqlClientResultBuffer::isNull( int fieldNumber ) const
{
    return d->isNull( fieldNumber );
}

QSqlClientNullData* QSqlClientResultBuffer::nullData ( int fieldNumber ) const
{
    return d->nullData( fieldNumber );
}

int QSqlClientResultBuffer::size( int fieldNumber ) const
{
    return d->size( fieldNumber );
}

QVariant::Type QSqlClientResultBuffer::type( int fieldNumber ) const
{
    return d->type( fieldNumber );
}

void QSqlClientResultBuffer::installDataFormat( QSqlClientData* format )
{
    d->installFormat( format );
}

QSqlClientData* QSqlClientResultBuffer::dataFormat() const
{
    return d->format();
}

int QSqlClientResultBuffer::count() const
{
    return d->count();
}

/////////////

class QSqlClientResultSet::Private
{
public:
    Private() : atPos( -1 ) { set.setAutoDelete( TRUE ); }
    ~Private() {}

    void append( const QSqlClientResultBuffer& buf )
    {
 	if ( set.count() + 1 > set.size() )
	    set.resize( set.size() + 1000 );
	set.insert( set.count(), new QSqlClientResultBuffer( buf ) );
    }
    bool seek( int row )
    {
	if ( row < 0 || row > (int)set.count()-1 )
	    return FALSE;
	atPos = row;
	return TRUE;
    }
    const QSqlClientResultBuffer* buffer() const
    {
	if ( atPos < 0 || atPos > (int)set.count() )
	    return 0;
	return set[ atPos ];
    }
    int size() const
    {
	return set.count();
    }
    int at() const
    {
	return atPos;
    }


private:
    QPtrVector<QSqlClientResultBuffer> set;
    int atPos;
};

QSqlClientResultSet::QSqlClientResultSet()
{
    d = new Private();
}

QSqlClientResultSet::~QSqlClientResultSet()
{
    delete d;
}

void QSqlClientResultSet::append( const QSqlClientResultBuffer& buf )
{
    d->append( buf );
}

void QSqlClientResultSet::clear()
{
    delete d;
    d = new Private();
}

bool QSqlClientResultSet::seek( int row )
{
    return d->seek( row );
}

const QSqlClientResultBuffer* QSqlClientResultSet::buffer() const
{
    return d->buffer();
}

int QSqlClientResultSet::size() const
{
    return d->size();
}

int QSqlClientResultSet::at() const
{
    return d->at();
}

/////////////


QSqlCachedResult::QSqlCachedResult(const QSqlDriver * db ): QSqlResult ( db )
{
   set = new QSqlClientResultSet();
   buf = new QSqlClientResultBuffer();
}

QSqlCachedResult::~QSqlCachedResult()
{
    delete set;
    delete buf;
}

bool QSqlCachedResult::fetch( int i )
{
    if ( ( !isActive() ) || ( i < 0 ) ) {
	return FALSE;
    }
    if ( at() == i )
	return TRUE;
    if ( set->seek( i ) ) {
	setAt( i );
	return TRUE;
    }
    setAt( set->size() - 1 );
    while ( at() < i ) {
	if ( !cacheNext() )
	    return FALSE;
	setAt( at() + 1 );
    }
    return TRUE;
}

bool QSqlCachedResult::fetchNext()
{
    if ( !isForwardOnly() && set->seek( at() + 1 ) ) {
	setAt( at() + 1 );
	return TRUE;
    }
    if ( cacheNext() ) {
	setAt( at() + 1 );
	return TRUE;
    }
    return FALSE;
}

bool QSqlCachedResult::fetchPrev()
{
    return fetch( at() - 1 );
}

bool QSqlCachedResult::fetchFirst()
{
    if ( isForwardOnly() && at() != QSql::BeforeFirst ) {
	return FALSE;
    }
    if ( !isForwardOnly() && set->seek( 0 ) ) {
	setAt( 0 );
	return TRUE;
    }
    if ( cacheNext() ) {
	setAt( 0 );
	return TRUE;
    }
    return FALSE;
}

bool QSqlCachedResult::fetchLast()
{
    if ( !isForwardOnly() && at() == QSql::AfterLast && set->size() > 0 ) {
	setAt( set->size() - 1 );
	return TRUE;
    }
    if ( at() >= QSql::BeforeFirst ) {
	while ( fetchNext() )
	    ; /* brute force */
	if ( isForwardOnly() && at() == QSql::AfterLast ) {
	    setAt( at() - 1 );
	    return TRUE;
	} else
	    return fetch( set->size() - 1 );
    }
    return FALSE;
}

QVariant QSqlCachedResult::data( int i )
{
    return set->buffer()->data( i );
}

bool QSqlCachedResult::isNull( int field )
{
    return set->buffer()->isNull( field );
}

void QSqlCachedResult::cleanup()
{
    setAt( -1 );
    setActive( FALSE );
    set->clear();
    buf->clear();
}

bool QSqlCachedResult::cacheNext()
{
    if ( gotoNext() ) {
	set->append( *buf );
	set->seek( set->size() - 1 );
	return TRUE;
    }
    return FALSE;
}
