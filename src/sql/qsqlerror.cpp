/****************************************************************************
**
** Implementation of QSqlError class
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

#include "qsqlerror.h"

#ifndef QT_NO_SQL

/*! \class QSqlError qsqlerror.h

    \brief Class used for reporting errors from a SQL database

    \module database

     This class is used to report database-specific errors.  An error
     description and (if appropriate) a database-specific error number
     can be recovered using this class.

*/

/*!  Creates a QSqlError containing the error text \a text, database-specific
     error text \a databaseText, of type \a type and the optional error number
     \a number.
*/

QSqlError::QSqlError(  const QString& driverText,
    		const QString& databaseText,
		int type,
		int number )
: driverError(driverText),
  databaseError(databaseText),
  errorType(type),
  errorNumber(number)
{
}

/*!  Creates a copy of \a n.

*/

QSqlError::QSqlError(const QSqlError& n)
: driverError(n.driverError),
  databaseError(n.databaseError),
  errorType(n.errorType),
  errorNumber(n.errorNumber)
{
}

/*!  Sets the value of this object to the value of \a n.

*/

QSqlError& QSqlError::operator=(const QSqlError& n)
{
    driverError = n.driverError;
    databaseError = n.databaseError;
    errorType = n.errorType;
    errorNumber = n.errorNumber;
    return *this;
}

/*! Destroys the object and frees any allocated resources.

*/

QSqlError::~QSqlError()
{
}

/*!  Returns the text of the error as reported by the driver.  This may contain database-specific
     descriptions.

*/
QString QSqlError::driverText() const
{
    return driverError;
}

/*!  Sets the driver error text to the value of \a driverText.

*/

void QSqlError::setDriverText( const QString& driverText )
{
    driverError = driverText;
}

/*!  Returns the text of the error as reported by the database.  This may contain database-specific
     descriptions.

*/

QString QSqlError::databaseText() const
{
    return databaseError;
}

/*!  Sets the database error text to the value of \a databaseText.

*/

void QSqlError::setDatabaseText( const QString& databaseText )
{
    databaseError = databaseText;
}

/*!  Returns the error type, or -1 if the type cannot the be determined.  See QSqlError::Type.

*/

int QSqlError::type() const
{
    return errorType;
}

/*!  Sets the error type to the value of \a type.

*/

void QSqlError::setType( int type )
{
    errorType = type;
}

/*!  Returns the database-specific error number, or -1 if it cannot
     be determined.

*/

int QSqlError::number() const
{
    return errorNumber;
}

/*!  Sets the database-specific error number to the value of \a type.

*/

void QSqlError::setNumber( int number )
{
    errorNumber = number;
}

#endif // QT_NO_SQL
