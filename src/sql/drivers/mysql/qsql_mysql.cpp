/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_mysql.h"

#include <qcorevariant.h>
#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlrecord.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvector.h>

#ifdef Q_OS_WIN32
// comment the next line out if you want to use MySQL/embedded on Win32 systems.
// note that it will crash if you don't statically link to the mysql/e library!
# define Q_NO_MYSQL_EMBEDDED
#endif

class QMYSQLDriverPrivate
{
public:
    QMYSQLDriverPrivate() : mysql(0), tc(0) {}
    MYSQL*     mysql;
    QTextCodec *tc;
};

class QMYSQLResultPrivate : public QMYSQLDriverPrivate
{
public:
    QMYSQLResultPrivate() : QMYSQLDriverPrivate(), result(0), tc(QTextCodec::codecForLocale()) {}

    MYSQL_RES* result;
    MYSQL_ROW  row;
    QVector<QCoreVariant::Type> fieldTypes;
    QTextCodec *tc;
};

static QTextCodec* codec(MYSQL* mysql)
{
#if MYSQL_VERSION_ID >= 40000
    QTextCodec* heuristicCodec = QTextCodec::codecForName(mysql_character_set_name(mysql), 2);
    if (heuristicCodec)
        return heuristicCodec;
#endif
    return QTextCodec::codecForLocale();
}

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type,
                            const QMYSQLDriverPrivate* p)
{
    const char *cerr = mysql_error(p->mysql);
    return QSqlError(QLatin1String("QMYSQL3: ") + err,
                     p->tc ? p->tc->toUnicode(cerr) : QString::fromLatin1(cerr),
                     type, mysql_errno(p->mysql));
}

static QCoreVariant::Type qDecodeMYSQLType(int mysqltype, uint flags)
{
    QCoreVariant::Type type;
    switch (mysqltype) {
    case FIELD_TYPE_TINY :
    case FIELD_TYPE_SHORT :
    case FIELD_TYPE_LONG :
    case FIELD_TYPE_INT24 :
        type = (flags & UNSIGNED_FLAG) ? QCoreVariant::UInt : QCoreVariant::Int;
        break;
    case FIELD_TYPE_YEAR :
        type = QCoreVariant::Int;
        break;
    case FIELD_TYPE_LONGLONG :
        type = (flags & UNSIGNED_FLAG) ? QCoreVariant::ULongLong : QCoreVariant::LongLong;
        break;
    case FIELD_TYPE_DECIMAL :
    case FIELD_TYPE_FLOAT :
    case FIELD_TYPE_DOUBLE :
        type = QCoreVariant::Double;
        break;
    case FIELD_TYPE_DATE :
        type = QCoreVariant::Date;
        break;
    case FIELD_TYPE_TIME :
        type = QCoreVariant::Time;
        break;
    case FIELD_TYPE_DATETIME :
    case FIELD_TYPE_TIMESTAMP :
        type = QCoreVariant::DateTime;
        break;
    case FIELD_TYPE_BLOB :
    case FIELD_TYPE_TINY_BLOB :
    case FIELD_TYPE_MEDIUM_BLOB :
    case FIELD_TYPE_LONG_BLOB :
        type = (flags & BINARY_FLAG) ? QCoreVariant::ByteArray : QCoreVariant::CString;
        break;
    default:
    case FIELD_TYPE_ENUM :
    case FIELD_TYPE_SET :
    case FIELD_TYPE_STRING :
    case FIELD_TYPE_VAR_STRING :
        type = QCoreVariant::String;
        break;
    }
    return type;
}

static QSqlField qToField(MYSQL_FIELD *field, QTextCodec *tc)
{
    QSqlField f(tc->toUnicode(field->name),
                qDecodeMYSQLType(int(field->type), field->flags));
    f.setRequired(IS_NOT_NULL(field->flags));
    f.setLength(field->length);
    f.setPrecision(field->decimals);
    f.setSqlType(field->type);
    return f;
}

QMYSQLResult::QMYSQLResult(const QMYSQLDriver* db)
: QSqlResult(db)
{
    d = new QMYSQLResultPrivate();
    d->mysql = db->d->mysql;
    d->tc = db->d->tc;
}

QMYSQLResult::~QMYSQLResult()
{
    cleanup();
    delete d;
}

MYSQL_RES* QMYSQLResult::result()
{
    return d->result;
}

void QMYSQLResult::cleanup()
{
    if (d->result) {
        mysql_free_result(d->result);
    }
    d->result = NULL;
    d->row = NULL;
    setAt(-1);
    setActive(false);
}

bool QMYSQLResult::fetch(int i)
{
    if (isForwardOnly()) { // fake a forward seek
        if (at() < i) {
            int x = i - at();
            while (--x && fetchNext());
            return fetchNext();
        } else {
            return false;
        }
    }
    if (at() == i)
        return true;
    mysql_data_seek(d->result, i);
    d->row = mysql_fetch_row(d->result);
    if (!d->row)
        return false;
    setAt(i);
    return true;
}

bool QMYSQLResult::fetchNext()
{
    d->row = mysql_fetch_row(d->result);
    if (!d->row)
        return false;
    setAt(at() + 1);
    return true;
}

bool QMYSQLResult::fetchLast()
{
    if (isForwardOnly()) { // fake this since MySQL can't seek on forward only queries
        bool success = fetchNext(); // did we move at all?
        while (fetchNext());
        return success;
    }
    my_ulonglong numRows = mysql_num_rows(d->result);
    if (!numRows)
        return false;
    return fetch(numRows - 1);
}

bool QMYSQLResult::fetchFirst()
{
    if (isForwardOnly()) // again, fake it
        return fetchNext();
    return fetch(0);
}

QCoreVariant QMYSQLResult::data(int field)
{
    if (!isSelect() || field >= int(d->fieldTypes.count())) {
        qWarning("QMYSQLResult::data: column %d out of range", field);
        return QCoreVariant();
    }

    if (d->row[field] == NULL) {
        // NULL value
        return QCoreVariant(d->fieldTypes.at(field));
    }

    QString val;
    if (d->fieldTypes.at(field) != QCoreVariant::ByteArray)
        val = d->tc->toUnicode(d->row[field]);
    switch (d->fieldTypes.at(field)) {
    case QCoreVariant::LongLong:
        return QCoreVariant(val.toLongLong());
    case QCoreVariant::ULongLong:
        return QCoreVariant(val.toULongLong());
    case QCoreVariant::Int:
        return QCoreVariant(val.toInt());
    case QCoreVariant::UInt:
        return QCoreVariant(val.toUInt());
    case QCoreVariant::Double:
        return QCoreVariant(val.toDouble());
    case QCoreVariant::Date:
        if (val.isEmpty()) {
            return QCoreVariant(QDate());
        } else {
            return QCoreVariant(QDate::fromString(val, Qt::ISODate) );
        }
    case QCoreVariant::Time:
        if (val.isEmpty()) {
            return QCoreVariant(QTime());
        } else {
            return QCoreVariant(QTime::fromString(val, Qt::ISODate));
        }
    case QCoreVariant::DateTime:
        if (val.isEmpty())
            return QCoreVariant(QDateTime());
        if (val.length() == 14u)
            // TIMESTAMPS have the format yyyyMMddhhmmss
            val.insert(4, QLatin1Char('-')).insert(7, QLatin1Char('-')).insert(10,
                    QLatin1Char('T')).insert(13, QLatin1Char(':')).insert(16, QLatin1Char(':'));
        return QCoreVariant(QDateTime::fromString(val, Qt::ISODate));
    case QCoreVariant::ByteArray: {
        unsigned long* fl = mysql_fetch_lengths(d->result);
        QByteArray ba(d->row[field], fl[field]);
        return QCoreVariant(ba);
    }
    default:
    case QCoreVariant::String:
        return QCoreVariant(val);
    }
    qWarning("QMYSQLResult::data: unknown data type");
    return QCoreVariant();
}

bool QMYSQLResult::isNull(int field)
{
    if (d->row[field] == NULL)
        return true;
    return false;
}

bool QMYSQLResult::reset (const QString& query)
{
    if (!driver())
        return false;
    if (!driver()-> isOpen() || driver()->isOpenError())
        return false;
    cleanup();
    const QByteArray encQuery(d->tc->fromUnicode(query));
    if (mysql_real_query(d->mysql, encQuery.data(), encQuery.length())) {
        setLastError(qMakeError(QLatin1String("Unable to execute query"),
                                QSqlError::StatementError, d));
        return false;
    }
    if (isForwardOnly()) {
        if (isActive() || isValid()) // have to empty the results from previous query
            fetchLast();
        d->result = mysql_use_result(d->mysql);
    } else {
        d->result = mysql_store_result(d->mysql);
    }
    if (!d->result && mysql_field_count(d->mysql) > 0) {
        setLastError(qMakeError(QLatin1String("Unable to store result"),
                                QSqlError::StatementError, d));
        return false;
    }
    int numFields = mysql_field_count(d->mysql);
    setSelect(!(numFields == 0));
    d->fieldTypes.resize(numFields);
    if (isSelect()) {
        for(int i = 0; i < numFields; i++) {
            MYSQL_FIELD* field = mysql_fetch_field_direct(d->result, i);
            if (field->type == FIELD_TYPE_DECIMAL)
                d->fieldTypes[i] = QCoreVariant::String;
            else
                d->fieldTypes[i] = qDecodeMYSQLType(field->type, field->flags);
        }
    }
    setActive(true);
    return true;
}

int QMYSQLResult::size()
{
    return isSelect() ? int(mysql_num_rows(d->result)) : -1;
}

int QMYSQLResult::numRowsAffected()
{
    return int(mysql_affected_rows(d->mysql));
}

QSqlRecord QMYSQLResult::record() const
{
    QSqlRecord info;
    if (!isActive() || !isSelect())
        return info;
    if (!mysql_errno(d->mysql)) {
        MYSQL_FIELD* field = mysql_fetch_field(d->result);
        while(field) {
            info.append(qToField(field, d->tc));
            field = mysql_fetch_field(d->result);
        }
    }
    mysql_field_seek(d->result, 0);
    return info;
}


/////////////////////////////////////////////////////////

static void qServerInit()
{
#ifndef Q_NO_MYSQL_EMBEDDED
# if MYSQL_VERSION_ID >= 40000
    static bool init = false;
    if (init)
        return;

    // this should only be called once
    // has no effect on client/server library
    // but is vital for the embedded lib
    if (mysql_server_init(0, 0, 0)) {
        qWarning("QMYSQLDriver::qServerInit: unable to start server.");
    }
    init = true;
# endif // MYSQL_VERSION_ID
#endif // Q_NO_MYSQL_EMBEDDED
}

QMYSQLDriver::QMYSQLDriver(QObject * parent)
    : QSqlDriver(parent)
{
    init();
    qServerInit();
}

/*!
    Create a driver instance with the open connection handle, \a con.
    The instance's parent (owner) is \a parent.
*/

QMYSQLDriver::QMYSQLDriver(MYSQL * con, QObject * parent)
    : QSqlDriver(parent)
{
    init();
    if (con) {
        d->mysql = (MYSQL *) con;
        d->tc = codec(con);
        setOpen(true);
        setOpenError(false);
    } else {
        qServerInit();
    }
}

void QMYSQLDriver::init()
{
    d = new QMYSQLDriverPrivate();
    d->mysql = 0;
}

QMYSQLDriver::~QMYSQLDriver()
{
    delete d;
#ifndef Q_NO_MYSQL_EMBEDDED
# if MYSQL_VERSION_ID > 40000
    mysql_server_end();
# endif
#endif
}

bool QMYSQLDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
// CLIENT_TRANSACTION should be defined in all recent mysql client libs > 3.23.34
#ifdef CLIENT_TRANSACTIONS
        if (d->mysql) {
            if ((d->mysql->server_capabilities & CLIENT_TRANSACTIONS) == CLIENT_TRANSACTIONS)
                return true;
        }
#endif
        return false;
    case QuerySize:
        return true;
    case BLOB:
        return true;
    case Unicode:
        return false;
    default:
        return false;
    }
}

static void setOptionFlag(uint &optionFlags, const QString &opt)
{
    if (opt == QLatin1String("CLIENT_COMPRESS"))
        optionFlags |= CLIENT_COMPRESS;
    else if (opt == QLatin1String("CLIENT_FOUND_ROWS"))
        optionFlags |= CLIENT_FOUND_ROWS;
    else if (opt == QLatin1String("CLIENT_IGNORE_SPACE"))
        optionFlags |= CLIENT_IGNORE_SPACE;
    else if (opt == QLatin1String("CLIENT_INTERACTIVE"))
        optionFlags |= CLIENT_INTERACTIVE;
    else if (opt == QLatin1String("CLIENT_NO_SCHEMA"))
        optionFlags |= CLIENT_NO_SCHEMA;
    else if (opt == QLatin1String("CLIENT_ODBC"))
        optionFlags |= CLIENT_ODBC;
    else if (opt == QLatin1String("CLIENT_SSL"))
        optionFlags |= CLIENT_SSL;
    else
        qWarning("QMYSQLDriver::open: Unknown connect option '%s'", opt.latin1());
}

bool QMYSQLDriver::open(const QString& db,
                         const QString& user,
                         const QString& password,
                         const QString& host,
                         int port,
                         const QString& connOpts)
{
    if (isOpen())
        close();

    unsigned int optionFlags = 0;
    const QStringList opts(connOpts.split(QLatin1Char(';'), QString::SkipEmptyParts));

    // extract the real options from the string
    for (int i = 0; i < opts.count(); ++i) {
        QString tmp(opts.at(i).simplified());
        int idx;
        if ((idx = tmp.indexOf(QLatin1Char('='))) != -1) {
            QString val(tmp.mid(idx + 1).simplified());
            if (val == QLatin1String("TRUE") || val == QLatin1String("1"))
                setOptionFlag(optionFlags, tmp.left(idx).simplified());
            else
                qWarning("QMYSQLDriver::open: Illegal connect option value '%s'", tmp.latin1());
        } else {
            setOptionFlag(optionFlags, tmp);
        }
    }

    if ((d->mysql = mysql_init((MYSQL*) 0)) &&
            mysql_real_connect(d->mysql,
                                host.local8Bit(),
                                user.local8Bit(),
                                password.local8Bit(),
                                db.isNull() ? "" : db.local8Bit(),
                                (port > -1) ? port : 0,
                                NULL,
                                optionFlags))
    {
        if (mysql_select_db(d->mysql, db.local8Bit())) {
            setLastError(qMakeError(QLatin1String("Unable open database '") + db +
                        QLatin1Char('\''), QSqlError::ConnectionError, d));
            mysql_close(d->mysql);
            setOpenError(true);
            return false;
        }
    } else {
            setLastError(qMakeError(QLatin1String("Unable to connect"),
                                    QSqlError::ConnectionError, d));
            mysql_close(d->mysql);
            setOpenError(true);
            return false;
    }
    d->tc = codec(d->mysql);
    setOpen(true);
    setOpenError(false);
    return true;
}

void QMYSQLDriver::close()
{
    if (isOpen()) {
        mysql_close(d->mysql);
        setOpen(false);
        setOpenError(false);
    }
}

QSqlQuery QMYSQLDriver::createQuery() const
{
    return QSqlQuery(new QMYSQLResult(this));
}

QStringList QMYSQLDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;
    if (!(type & QSql::Tables))
        return tl;

    MYSQL_RES* tableRes = mysql_list_tables(d->mysql, NULL);
    MYSQL_ROW row;
    int i = 0;
    while (tableRes) {
        mysql_data_seek(tableRes, i);
        row = mysql_fetch_row(tableRes);
        if (!row)
            break;
        tl.append(d->tc->toUnicode(row[0]));
        i++;
    }
    mysql_free_result(tableRes);
    return tl;
}

QSqlIndex QMYSQLDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex idx;
    if (!isOpen())
        return idx;
    QSqlQuery i = createQuery();
    QString stmt(QLatin1String("show index from %1;"));
    QSqlRecord fil = record(tablename);
    i.exec(stmt.arg(tablename));
    while (i.isActive() && i.next()) {
        if (i.value(2).toString() == QLatin1String("PRIMARY")) {
            idx.append(fil.field(i.value(4).toString()));
            idx.setCursorName(i.value(0).toString());
            idx.setName(i.value(2).toString());
        }
    }
    return idx;
}

QSqlRecord QMYSQLDriver::record(const QString& tablename) const
{
    QSqlRecord info;
    if (!isOpen())
        return info;
    MYSQL_RES* r = mysql_list_fields(d->mysql, tablename.local8Bit(), 0);
    if (!r) {
        return info;
    }
    MYSQL_FIELD* field;
    while ((field = mysql_fetch_field(r)))
        info.append(qToField(field, d->tc));
    mysql_free_result(r);
    return info;
}

MYSQL* QMYSQLDriver::mysql()
{
     return d->mysql;
}

bool QMYSQLDriver::beginTransaction()
{
#ifndef CLIENT_TRANSACTIONS
    return false;
#endif
    if (!isOpen()) {
        qWarning("QMYSQLDriver::beginTransaction: Database not open");
        return false;
    }
    if (mysql_query(d->mysql, "BEGIN WORK")) {
        setLastError(qMakeError(QLatin1String("Unable to begin transaction"),
                                QSqlError::StatementError, d));
        return false;
    }
    return true;
}

bool QMYSQLDriver::commitTransaction()
{
#ifndef CLIENT_TRANSACTIONS
    return false;
#endif
    if (!isOpen()) {
        qWarning("QMYSQLDriver::commitTransaction: Database not open");
        return false;
    }
    if (mysql_query(d->mysql, "COMMIT")) {
        setLastError(qMakeError(QLatin1String("Unable to commit transaction"),
                                QSqlError::StatementError, d));
        return false;
    }
    return true;
}

bool QMYSQLDriver::rollbackTransaction()
{
#ifndef CLIENT_TRANSACTIONS
    return false;
#endif
    if (!isOpen()) {
        qWarning("QMYSQLDriver::rollbackTransaction: Database not open");
        return false;
    }
    if (mysql_query(d->mysql, "ROLLBACK")) {
        setLastError(qMakeError(QLatin1String("Unable to rollback transaction"),
                                QSqlError::StatementError, d));
        return false;
    }
    return true;
}

QString QMYSQLDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
    QString r;
    if (field.isNull()) {
        r = QLatin1String("NULL");
    } else {
        switch(field.type()) {
        case QCoreVariant::ByteArray: {

            const QByteArray ba = field.value().toByteArray();
            // buffer has to be at least length*2+1 bytes
            char* buffer = new char[ba.size() * 2 + 1];
            int escapedSize = int(mysql_escape_string(buffer, ba.data(), ba.size()));
            r.reserve(escapedSize + 3);
            r.append(QLatin1Char('\'')).append(d->tc->toUnicode(buffer)).append(QLatin1Char('\''));
            delete[] buffer;
        }
        break;
        case QCoreVariant::String:
            // Escape '\' characters
            r = QSqlDriver::formatValue(field, trimStrings);
            r.replace(QLatin1String("\\"), QLatin1String("\\\\"));
            break;
        default:
            r = QSqlDriver::formatValue(field, trimStrings);
        }
    }
    return r;
}
