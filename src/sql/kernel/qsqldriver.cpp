/****************************************************************************
**
** Implementation of QSqlDriver class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsqldriver.h"

#ifndef QT_NO_SQL

#include "qdatetime.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qstringlist.h"
#include "private/qobject_p.h"

#define d d_func()
#define q q_func()

class QSqlDriverPrivate: public QObjectPrivate
{
public:
    QSqlDriverPrivate();
    virtual ~QSqlDriverPrivate();
public:
    QSqlDriver *q;
    uint isOpen: 1;
    uint isOpenError: 1;
    QSqlError error;
};

inline QSqlDriverPrivate::QSqlDriverPrivate()
    : QObjectPrivate(), isOpen(false), isOpenError(false)
{
}

QSqlDriverPrivate::~QSqlDriverPrivate()
{
}

/*!
    \class QSqlDriver qsqldriver.h
    \brief The QSqlDriver class is an abstract base class for accessing
    SQL databases.

    \ingroup database
    \module sql

    This class should not be used directly. Use QSqlDatabase instead.

    If you want to create your own driver you can subclass this class
    and reimplement its pure virtual functions, and those virtual
    functions that you need.
*/

/*!
    Default constructor. Creates a new driver with the given \a parent.
*/

QSqlDriver::QSqlDriver(QObject * parent)
    : QObject(*new QSqlDriverPrivate, parent)
{
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlDriver::~QSqlDriver()
{
}

/*!
    \fn bool QSqlDriver::open(const QString& db, const QString& user, const QString& password, const QString& host, int port, const QString& connOpts)

    Derived classes must reimplement this pure virtual function in
    order to open a database connection on database \a db, using user
    name \a user, password \a password, host \a host, port \a port and
    connection options \a connOpts.

    The function \e must return true on success and false on failure.

    \sa setOpen()
*/

/*!
    \fn bool QSqlDriver::close()

    Derived classes must reimplement this pure virtual function in
    order to close the database connection. Return true on success,
    false on failure.

    \sa open() setOpen()
*/

/*!
    \fn QSqlQuery QSqlDriver::createQuery() const

    Creates an empty SQL result on the database. Derived classes must
    reimplement this function and return a QSqlQuery object
    appropriate for their database to the caller.
*/

//void QSqlDriver::destroyResult(QSqlResult* r) const
//{
//    if (r)
//        delete r;
//}

/*!
    Returns true if the database connection is open; otherwise returns
    false.
*/

bool QSqlDriver::isOpen() const
{
    return d->isOpen;
}

/*!
    Returns true if the there was an error opening the database
    connection; otherwise returns false.
*/

bool QSqlDriver::isOpenError() const
{
    return d->isOpenError;
}

/*!
    \enum QSqlDriver::DriverFeature

    This enum contains a list of features a driver might support. Use
    hasFeature() to query whether a feature is supported or not.

    \value Transactions  whether the driver supports SQL transactions
    \value QuerySize  whether the database is capable of reporting the size
    of a query. Note that some databases do not support returning the size
    (i.e. number of rows returned) of a query, in which case
    QSqlQuery::size() will return -1
    \value BLOB  whether the driver supports Binary Large Object fields
    \value Unicode  whether the driver supports Unicode strings if the
    database server does
    \value PreparedQueries  whether the driver supports prepared query execution
    \value NamedPlaceholders  whether the driver supports the use of named placeholders
    \value PositionalPlaceholders  whether the driver supports the use of positional placeholders

    More information about supported features can be found in the
    \link sql-driver.html Qt SQL driver\endlink documentation.

    \sa hasFeature()
*/

/*!
    \fn bool QSqlDriver::hasFeature(DriverFeature f) const

    Returns true if the driver supports feature \a f; otherwise
    returns false.

    Note that some databases need to be open() before this can be
    determined.

    \sa DriverFeature
*/

/*!
    This function sets the open state of the database to \a o. Derived
    classes can use this function to report the status of open().

    \sa open(), setOpenError()
*/

void QSqlDriver::setOpen(bool o)
{
    d->isOpen = o;
}

/*!
    This function sets the open error state of the database to \a e.
    Derived classes can use this function to report the status of
    open(). Note that if \a e is true the open state of the database
    is set to closed (i.e. isOpen() returns false).

    \sa open(), setOpenError()
*/

void QSqlDriver::setOpenError(bool e)
{
    d->isOpenError = e;
    if (e)
        d->isOpen = false;
}

/*!
    This function is called to begin a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa commitTransaction(), rollbackTransaction()
*/

bool QSqlDriver::beginTransaction()
{
    return false;
}

/*!
    This function is called to commit a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa beginTransaction(), rollbackTransaction()
*/

bool QSqlDriver::commitTransaction()
{
    return false;
}

/*!
    This function is called to rollback a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa beginTransaction(), commitTransaction()
*/

bool QSqlDriver::rollbackTransaction()
{
    return false;
}

/*!
    This function is used to set the value of the last error, \a e,
    that occurred on the database.

    \sa lastError()
*/

void QSqlDriver::setLastError(const QSqlError& e)
{
    d->error = e;
}

/*!
    Returns a QSqlError object which contains information about the
    last error that occurred on the database.
*/

QSqlError QSqlDriver::lastError() const
{
    return d->error;
}

/*!
    Returns a list of the names of the tables in the database. The
    default implementation returns an empty list.

    The \a tableType argument describes what types of tables
    should be returned. Due to binary compatibility, the string
    contains the value of the enum QSql::TableTypes as text.
    An empty string should be treated as QSql::Tables for
    backward compatibility.

    \sa QSql::TableType
*/

QStringList QSqlDriver::tables(QSql::TableType) const
{
    return QStringList();
}

/*!
    Returns the primary index for table \a tableName. Returns an empty
    QSqlIndex if the table doesn't have a primary index. The default
    implementation returns an empty index.
*/

QSqlIndex QSqlDriver::primaryIndex(const QString&) const
{
    return QSqlIndex();
}


/*!
    Returns a QSqlRecord populated with the names of the fields in
    table \a tableName. If no such table exists, an empty record is
    returned. The default implementation returns an empty record.
*/

QSqlRecord QSqlDriver::record(const QString& ) const
{
    return QSqlRecord();
}

/*!
*/

QString QSqlDriver::sqlStatement(StatementType type, const QString &tableName,
                                 const QSqlRecord &rec, bool preparedStatement) const
{
    int i;
    QString s;
    s.reserve(128);
    switch (type) {
    case SelectStatement:
        for (i = 0; i < rec.count(); ++i) {
            if (rec.isGenerated(i))
                s.append(rec.fieldName(i)).append(", ");
        }
        if (s.isEmpty())
            return s;
        s.chop(2);
        s.prepend("SELECT ").append(" FROM ").append(tableName);
        break;
    case WhereStatement:
        if (preparedStatement) {
            for (i = 0; i < rec.count(); ++i) {
                s.append(rec.fieldName(i));
                QString val = formatValue(rec.field(i));
                if (val == QLatin1String("NULL"))
                    s.append(QLatin1String(" IS NULL"));
                else
                    s.append(" = ").append(val);
                s.append(" AND ");
            }
        } else {
            for (int i = 0; i < rec.count(); ++i)
                s.append(rec.fieldName(i)).append(" = ? AND ");
        }
        if (!s.isEmpty())
            s.chop(5); // remove trailing AND
        break;
    case UpdateStatement:
        s.append("UPDATE TABLE ").append(tableName).append(" SET ");
        for (i = 0; i < rec.count(); ++i) {
            if (!rec.isGenerated(i))
                continue;
            s.append(rec.fieldName(i)).append("=");
            if (preparedStatement)
                s.append("?");
            else
                s.append(formatValue(rec.field(i)));
            s.append(", ");
        }
        if (s.endsWith(", "))
            s.chop(2);
        else
            s = QString();
        break;
    case DeleteStatement:
        s.append("DELETE FROM ").append(tableName);
        break;
    case InsertStatement: {
        s.append("INSERT INTO ").append(tableName).append(" (");
        QString vals;
        for (i = 0; i < rec.count(); ++i) {
            if (!rec.isGenerated(i))
                continue;
            s.append(rec.fieldName(i)).append(", ");
            if (preparedStatement)
                vals.append("?");
            else
                vals.append(formatValue(rec.field(i)));
            vals.append(", ");
        }
        if (vals.isEmpty()) {
            s = QString();
        } else {
            vals.chop(2); // remove trailing comma
            s[s.length() - 2] = QLatin1Char(')');
            s.append("VALUES (").append(vals).append(")");
        }
        break; }
    }
    return s;
}

/*!
    Returns a string representation of the \a field value for the
    database. This is used, for example, when constructing INSERT and
    UPDATE statements.

    The default implementation returns the value formatted as a string
    according to the following rules:

    \list

    \i If \a field is character data, the value is returned enclosed
    in single quotation marks, which is appropriate for many SQL
    databases. Any embedded single-quote characters are escaped
    (replaced with two single-quote characters). If \a trimStrings is
    true (the default is false), all trailing whitespace is trimmed
    from the field.

    \i If \a field is date/time data, the value is formatted in ISO
    format and enclosed in single quotation marks. If the date/time
    data is invalid, NULL is returned.

    \i If \a field is \link QByteArray bytearray\endlink data, and the
    driver can edit binary fields, the value is formatted as a
    hexadecimal string.

    \i For any other field type, toString() is called on its value
    and the result of this is returned.

    \endlist

    \sa QCoreVariant::toString().

*/
QString QSqlDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
    static const QString nullTxt("NULL");

    QString r;
    if (field.isNull())
        r = nullTxt;
    else {
        switch (field.type()) {
        case QCoreVariant::Int:
        case QCoreVariant::UInt:
            if (field.value().type() == QCoreVariant::Bool)
                r = field.value().toBool() ? "1" : "0";
            else
                r = field.value().toString();
            break;
        case QCoreVariant::Date:
            if (field.value().toDate().isValid())
                r = "'" + field.value().toDate().toString(Qt::ISODate) + "'";
            else
                r = nullTxt;
            break;
        case QCoreVariant::Time:
            if (field.value().toTime().isValid())
                r = "'" + field.value().toTime().toString(Qt::ISODate) + "'";
            else
                r = nullTxt;
            break;
        case QCoreVariant::DateTime:
            if (field.value().toDateTime().isValid())
                r = "'" +
                    field.value().toDateTime().toString(Qt::ISODate) + "'";
            else
                r = nullTxt;
            break;
        case QCoreVariant::String:
        {
            QString result = field.value().toString();
            if (trimStrings) {
                int end = result.length() - 1;
                while (end && result[end].isSpace()) /* skip white space from end */
                    end--;
                result.truncate(end);
            }
            /* escape the "'" character */
            result.replace(QChar('\''), "''");
            r = "'" + result + "'";
            break;
        }
        case QCoreVariant::Bool:
            if (field.value().toBool())
                r = "1";
            else
                r = "0";
            break;
        case QCoreVariant::ByteArray : {
            if (hasFeature(BLOB)) {
                QByteArray ba = field.value().toByteArray();
                QString res;
                static const char hexchars[] = "0123456789abcdef";
                for (int i = 0; i < ba.size(); ++i) {
                    uchar s = (uchar) ba[(int)i];
                    res += hexchars[s >> 4];
                    res += hexchars[s & 0x0f];
                }
                r = "'" + res + "'";
                break;
            }
        }
        default:
            r = field.value().toString();
            break;
        }
    }
    return r;
}

#endif // QT_NO_SQL
