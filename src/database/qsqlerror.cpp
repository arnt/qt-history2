#include "qsqlerror.h"

/*!
    \class QSqlError qsql_base.h
    \brief Class used for reporting errors from a SQL database

    \module database

     This class is used to report database-specific errors.  An error
     description and (if appropriate) a database-specific error number
     can be recovered using this class.

*/

/*!  Creates a QSqlError contianing the error text \a text, database-specific error
     text \databaseText, of type \a type and the optional error number \a number.

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