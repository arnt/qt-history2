/****************************************************************************
**
** Implementation of QSqlDriver class
**
** Created : 001103
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

#include "qsqldriver.h"
#include "qdatetime.h"

#ifndef QT_NO_SQL

// database states
#define DBState_Open	    	0x0001
#define DBState_OpenError   	0x0002

/*!
  \class QSqlDriver qsqldriver.h
  \brief Class used for accessing databases

  \module sql

  This is an abstract base class which defines an interface for accessing SQL databases.  This
  class should not be used directly.  Use QSqlDatabase instead.

*/

/*!  Default constructor.

*/

QSqlDriver::QSqlDriver( QObject * parent, const char * name )
: QObject(parent, name),
  dbState(0),
  error()
{
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlDriver::~QSqlDriver()
{
}

/*! \fn bool QSqlDriver::open( const QString& db, const QString& user, const QString& password, const QString& host )
    Derived classes must override this abstract virtual method in order to open the database.
    Return TRUE on success, FALSE on failure.

    \sa setOpen()

*/

/*! \fn bool QSqlDriver::close()
    Derived classes must override this abstract virtual method in order to close the database.
    Return TRUE on success, FALSE on failure.

    \sa setOpen()

*/

/*! \fn QSqlQuery QSqlDriver::createQuery() const
    Creates an empty SQL result on the database.  Derived classes must override this method
    and return a QSqlQuery object appropriate for their database to the caller.

*/

//void QSqlDriver::destroyResult( QSqlResult* r ) const
//{
//    if ( r )
//    	delete r;
//}

/*!  Returns TRUE if the database state is open, FALSE otherwise.

*/

bool QSqlDriver::isOpen() const
{
    return ((dbState & DBState_Open) == DBState_Open);
}

/*!  Returns TRUE if the there was an error opening the database, FALSE otherwise.

*/

bool QSqlDriver::isOpenError() const
{
    return ((dbState & DBState_OpenError) == DBState_OpenError);
}

/*! \fn bool QSqlDriver::hasTransactionSupport() const

  Returns TRUE if the database supports transactions, FALSE otherwise.
  Note that some databases need to be open() before this can be
  determined.

*/

/*! \fn bool QSqlDriver::hasQuerySizeSupport() const

  Returns TRUE if the database supports reporting information about
  the size of a query, FALSE otherwise.  Note that some databases do
  not support returning the size (in number of rows returned) of a
  query, so therefore QSql::size() will return -1.

*/

/*! \fn bool QSqlDriver::canEditBinaryFields() const

  Returns TRUE if the database can save binary fields information to
  the database, FALSE otherwise.

*/

/*! Protected method which sets the open state of the database to \a o.
    Derived classes can use this method to report the status of open().

    \sa open(), setOpenError()

*/

void QSqlDriver::setOpen( bool o )
{
    if ( o )
	dbState |= DBState_Open;
    else
	dbState &= ~DBState_Open;
}

/*! Protected method which sets the open error state of the database to \a e.
    Derived classes can use this method to report the status of open().
    Note that if \a e is TRUE the open state of the database is set to closed
    (i.e., isOpen() returns FALSE).

    \sa open(), setOpenError()

*/

void QSqlDriver::setOpenError( bool e )
{
    if ( e ) {
	dbState |= DBState_OpenError;
	dbState &= ~DBState_Open;
    }
    else
	dbState &= ~DBState_OpenError;
}

/*! Protected method which derived classes can override to begin a transaction.
    If successful, return TRUE, otherwise return FALSE.  The default implementation returns FALSE.

    \sa setTransactionSupport(), transaction(), commit(), rollback()

*/

bool QSqlDriver::beginTransaction()
{
    return FALSE;
}

/*! Protected method which derived classes can override to commit a transaction.
    If successful, return TRUE, otherwise return FALSE. The default implementation returns FALSE.

    \sa setTransactionSupport(), transaction(), commit(), rollback()

*/

bool QSqlDriver::commitTransaction()
{
    return FALSE;
}

/*! Protected method which derived classes can override to rollback a transaction.
    If successful, return TRUE, otherwise return FALSE.  The default implementation returns FALSE.

    \sa setTransactionSupport(), transaction(), commit(), rollback()

*/

bool QSqlDriver::rollbackTransaction()
{
    return FALSE;
}

/*! Protected method which allows derived classes to set the value of the last error that occurred
    on the database.

    \sa lastError()

*/

void QSqlDriver::setLastError( const QSqlError& e )
{
    error = e;
}

/*! Returns a QSqlError object which contains information about the last error that occurred on the
    database.

*/

QSqlError QSqlDriver::lastError() const
{
    return error;
}

/*!
  Returns a list of tables in the database.  The default
  implementation returns an empty list.
*/

QStringList QSqlDriver::tables( const QString&  ) const
{
    return QStringList();
}

/*!
  Returns the primary index for table \a tablename.  If no
  such index exists, the QSqlIndex that is returned will be
  empty.  The default implementation returns an empty index.

*/

QSqlIndex QSqlDriver::primaryIndex( const QString&  ) const
{
    return QSqlIndex();
}


/*!
  Returns a list of fields for table \a tablename.  If no
  such table exists, an empty list is returned.  The default
  implementation returns an empty record.

*/

QSqlRecord QSqlDriver::record( const QString&  ) const
{
    return QSqlRecord();
}

/*!  Returns a list of fields for the SQL \a query. The default
implementation returns an empty record.

*/

QSqlRecord QSqlDriver::record( const QSqlQuery& ) const
{
   return QSqlRecord();
}

/*!  Returns a string representation of the 'NULL' value for the
  database.  This is used, for example, when constructing INSERT and
  UPDATE statements.  The default implementation returns 'NULL'.

*/

QString QSqlDriver::nullText() const
{
    return "NULL";
}

/*!  Returns a string representation of the \a field value for the
  database.  This is used, for example, when constructing INSERT and
  UPDATE statements.

  The default implementation returns the value formatted as a string
  according to the following rules:

  If \a field is null, nullText() is returned.

  If \a field is character data, the value is returned enclosed by
  single quotation marks, which is appropriate for many SQL databases.

  If \a field is date/time data, the value is formatted in ISO format
  and enclosed by single quotation marks.

  If \a field is bytearray data, and the driver can edit binary
  fields, the value is formatted as a hexadecimal string.

  \sa QVariant::toString().

*/
QString QSqlDriver::formatValue( const QSqlField* field ) const
{
    QString r;
    if ( field->isNull() )
	r = nullText();
    else {
	switch ( field->type() ) {
	case QVariant::Date:
	    r = "'" + field->value().toDate().toString( Qt::ISODate ) + "'";
	    break;
	case QVariant::Time:
	    r = "'" + field->value().toTime().toString( Qt::ISODate ) + "'";
	    break;
	case QVariant::DateTime:
	    r = "'" + field->value().toDateTime().toString( Qt::ISODate ) + "'";
	    break;
	case QVariant::String:
	case QVariant::CString:
	    r = "'" + field->value().toString() + "'";
	    break;
	case QVariant::ByteArray : {
	    if ( canEditBinaryFields() ) {
		QByteArray ba = field->value().toByteArray();
		QString res;
		static const char hexchars[] = "0123456789abcdef";
		for ( uint i = 0; i < ba.size(); ++i ) {
		    uchar s = (uchar) ba[(int)i];
		    res += hexchars[s >> 4];
		    res += hexchars[s & 0x0f];
		}
		r = "'" + res + "'";
		break;
	    }
	}
	default:
	    r = field->value().toString();
	    break;
	}
    }
    return r;
}

#endif // QT_NO_SQL










