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

#include <qdebug.h>

#ifdef Q_OS_WIN32
// comment the next line out if you want to use MySQL/embedded on Win32 systems.
// note that it will crash if you don't statically link to the mysql/e library!
# define Q_NO_MYSQL_EMBEDDED
#endif

class QMYSQLDriverPrivate
{
public:
    QMYSQLDriverPrivate() : mysql(0), tc(0), preparedQuerys(false) {}
    MYSQL *mysql;
    QTextCodec *tc;

    bool preparedQuerys;
};

class QMYSQLResultPrivate : public QMYSQLDriverPrivate
{
public:
    QMYSQLResultPrivate() : QMYSQLDriverPrivate(), result(0), tc(QTextCodec::codecForLocale()),
        rowsAffected(0), hasBlobs(false)
#if MYSQL_VERSION_ID >= 40108
        , stmt(0), meta(0), inBinds(0), outBinds(0)
#endif
        {}

    MYSQL_RES *result;
    MYSQL_ROW row;
    QTextCodec *tc;

    int rowsAffected;

    bool bindInValues();
    void bindBlobs();

    bool hasBlobs;
    struct QMyField
    {
        QMyField()
            : outField(0), nullIndicator(false), bufLength(0u),
              myField(0), type(QCoreVariant::Invalid)
        {}
        char *outField;
        my_bool nullIndicator;
        ulong bufLength;
        MYSQL_FIELD *myField;
        QCoreVariant::Type type;
    };

    QVector<QMyField> fields;

#if MYSQL_VERSION_ID >= 40108
    MYSQL_STMT* stmt;
    MYSQL_RES* meta;

    MYSQL_BIND *inBinds;
    MYSQL_BIND *outBinds;
#endif
};

static QTextCodec* codec(MYSQL* mysql)
{
#if MYSQL_VERSION_ID >= 40000
    QTextCodec* heuristicCodec = QTextCodec::codecForName(mysql_character_set_name(mysql));
    if (heuristicCodec)
        return heuristicCodec;
#endif
    return QTextCodec::codecForLocale();
}

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type,
                            const QMYSQLDriverPrivate* p)
{
    const char *cerr = mysql_error(p->mysql);
    return QSqlError(QLatin1String("QMYSQL: ") + err,
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
 //       type = (flags & BINARY_FLAG) ? QCoreVariant::ByteArray : QCoreVariant::CString; ### FIXME
        type = QCoreVariant::ByteArray;
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

#if MYSQL_VERSION_ID >= 40108

static QSqlError qMakeStmtError(const QString& err, QSqlError::ErrorType type,
                            MYSQL_STMT* stmt)
{
    const char *cerr = mysql_stmt_error(stmt);
    return QSqlError(QLatin1String("QMYSQL3: ") + err,
                     QString::fromLatin1(cerr),
                     type, mysql_stmt_errno(stmt));
}

static bool qIsBlob(int t)
{
    return t == MYSQL_TYPE_TINY_BLOB
           || t == MYSQL_TYPE_BLOB
           || t == MYSQL_TYPE_MEDIUM_BLOB
           || t == MYSQL_TYPE_LONG_BLOB;
}

void QMYSQLResultPrivate::bindBlobs()
{
    int i;
    MYSQL_FIELD *fieldInfo;
    MYSQL_BIND  *bind;
//    Q_ASSERT(meta);

    for(i = 0; i < fields.count(); ++i) {
        fieldInfo = fields.at(i).myField;
        if (qIsBlob(inBinds[i].buffer_type) && meta && fieldInfo) {
            bind = &inBinds[i];
            bind->buffer_length = fieldInfo->max_length;
            delete static_cast<char*>(bind->buffer); // ### use a vector...?
            bind->buffer = new char[fieldInfo->max_length];
            fields[i].outField = static_cast<char*>(bind->buffer);
            bind->buffer_type = MYSQL_TYPE_STRING;
        }
    }
}

bool QMYSQLResultPrivate::bindInValues()
{
    MYSQL_BIND *bind;
    char *field;
    int i = 0;

    if (!meta)
        meta = mysql_stmt_result_metadata(stmt);
    if (!meta)
        return false;

    fields.resize(mysql_num_fields(meta));

    inBinds = new MYSQL_BIND[fields.size()];

    MYSQL_FIELD *fieldInfo;

    while((fieldInfo = mysql_fetch_field(meta))) {
        QMyField &f = fields[i];
        f.myField = fieldInfo;
        if (fieldInfo->type == FIELD_TYPE_DECIMAL)
            f.type = QCoreVariant::String;
        else
            f.type = qDecodeMYSQLType(fieldInfo->type, fieldInfo->flags);

        if (qIsBlob(fieldInfo->type)) {
            // the size of a blob-field is available as soon as we call
            // mysql_stmt_store_result()
            // after mysql_stmt_exec() in QMYSQLResult::exec()
            fieldInfo->length = 0;
            hasBlobs = true;
        } else {
            fieldInfo->type = MYSQL_TYPE_STRING;
        }
        bind = &inBinds[i];
        field = new char[fieldInfo->length + 1];
        memset(field, 0, fieldInfo->length + 1);

        bind->buffer_type = fieldInfo->type;
        bind->buffer = field;
        bind->buffer_length = f.bufLength = fieldInfo->length + 1;
        bind->is_null = &f.nullIndicator;
        bind->length = &f.bufLength;
        f.outField=field;

        ++i;
    }
    return true;
}
#endif

QMYSQLResult::QMYSQLResult(const QMYSQLDriver* db)
: QSqlResult(db)
{
    d = new QMYSQLResultPrivate();
    d->mysql = db->d->mysql;
    d->tc = db->d->tc;
    d->preparedQuerys = db->d->preparedQuerys;
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
    if (d->result)
        mysql_free_result(d->result);

#if MYSQL_VERSION_ID >= 40108
    if (d->stmt) {
        if (mysql_stmt_close(d->stmt))
            qWarning("QMYSQLResult::cleanup: unable to free statement handle");
        d->stmt = 0;
    }

    if (d->meta) {
        mysql_free_result(d->meta);
        d->meta = 0;
    }

    int i;
    for (i = 0; i < d->fields.count(); ++i)
        delete[] d->fields[i].outField;

    if (d->outBinds) {
        delete[] d->outBinds;
        d->outBinds = 0;
    }

    if (d->inBinds) {
        delete[] d->inBinds;
        d->inBinds = 0;
    }

//    for(i = 0; i < d->outFields.size(); ++i)
//        delete[] d->outFields.at(i);
#endif
    d->hasBlobs = false;
    d->fields.clear();
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
    if (d->preparedQuerys) {
#if MYSQL_VERSION_ID >= 40108
        mysql_stmt_data_seek(d->stmt, i);

        if (mysql_stmt_fetch(d->stmt)) {
            setLastError(qMakeStmtError(QLatin1String("Unable to fetch data"),
                         QSqlError::StatementError, d->stmt));
            return false;
        }
#else
        return false;
#endif
    } else {
        mysql_data_seek(d->result, i);
        d->row = mysql_fetch_row(d->result);
        if (!d->row)
            return false;
    }

    setAt(i);
    return true;
}

bool QMYSQLResult::fetchNext()
{
    if (d->preparedQuerys) {
#if MYSQL_VERSION_ID >= 40108
        if (mysql_stmt_fetch(d->stmt))
            return false;
#else
        return false;
#endif
    } else {
    d->row = mysql_fetch_row(d->result);
    if (!d->row)
        return false;
    }
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

    my_ulonglong numRows;
    if (d->preparedQuerys) {
#if MYSQL_VERSION_ID >= 40108
        numRows = mysql_stmt_num_rows(d->stmt);
#else
        numRows = 0;
#endif
    } else {
        numRows = mysql_num_rows(d->result);
    }
    if (at() == int(numRows))
        return true;
    if (!numRows)
        return false;
    return fetch(numRows - 1);
}

bool QMYSQLResult::fetchFirst()
{
    if (at() == 0)
        return true;

    if (isForwardOnly())
        return (at() == QSql::BeforeFirstRow) ? fetchNext() : false;
    return fetch(0);
}

QCoreVariant QMYSQLResult::data(int field)
{

    if (!isSelect() || field >= d->fields.count()) {
        qWarning("QMYSQLResult::data: column %d out of range", field);
        return QCoreVariant();
    }

    const QMYSQLResultPrivate::QMyField &f = d->fields.at(field);
    QString val;
    if (d->preparedQuerys) {
        if (f.nullIndicator)
            return QCoreVariant(f.type);

        if (f.type != QCoreVariant::ByteArray)
            val = d->tc->toUnicode(f.outField);
    } else {
        if (d->row[field] == NULL) {
            // NULL value
            return QCoreVariant(f.type);
        }
        if (f.type != QCoreVariant::ByteArray)
            val = d->tc->toUnicode(d->row[field]);
    }

    switch(f.type) {
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

        QByteArray ba;
        if (d->preparedQuerys) {
            ba = QByteArray(f.outField, f.bufLength);
        } else {
            unsigned long* fl = mysql_fetch_lengths(d->result);
            ba = QByteArray(d->row[field], fl[field]);
        }
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
   if (d->preparedQuerys)
       return d->fields.at(field).nullIndicator;
   else
       return d->row[field] == NULL;
}

bool QMYSQLResult::reset (const QString& query)
{
    if (d->preparedQuerys) {
        if (!prepare(query))
            return false;
        return exec();
    }
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;

    cleanup();
    const QByteArray encQuery(d->tc->fromUnicode(query));
    if (mysql_real_query(d->mysql, encQuery.data(), encQuery.length())) {
        setLastError(qMakeError(QLatin1String("Unable to execute query"),
                     QSqlError::StatementError, d));
        return false;
    }
    d->result = mysql_store_result(d->mysql);
    if (!d->result && mysql_field_count(d->mysql) > 0) {
        setLastError(qMakeError(QLatin1String("Unable to store result"),
                    QSqlError::StatementError, d));
        return false;
    }
    int numFields = mysql_field_count(d->mysql);
    setSelect(numFields != 0);
    d->fields.resize(numFields);
    d->rowsAffected = mysql_affected_rows(d->mysql);
    if (isSelect()) {
        for(int i = 0; i < numFields; i++) {
            MYSQL_FIELD* field = mysql_fetch_field_direct(d->result, i);
            if (field->type == FIELD_TYPE_DECIMAL)
                d->fields[i].type = QCoreVariant::String;
            else
                d->fields[i].type = qDecodeMYSQLType(field->type, field->flags);
        }
    }
    setActive(true);
    return true;
}

int QMYSQLResult::size()
{
    if (isSelect())
        if (d->preparedQuerys)
#if MYSQL_VERSION_ID >= 40108
            return int(mysql_stmt_num_rows(d->stmt));
#else
            return -1;
#endif
        else
            return int(mysql_num_rows(d->result));
    else
        return -1;
}

int QMYSQLResult::numRowsAffected()
{
    return d->rowsAffected;
}

QSqlRecord QMYSQLResult::record() const
{
    QSqlRecord info;
    MYSQL_RES *res;
    if (!isActive() || !isSelect())
        return info;

#if MYSQL_VERSION_ID >= 40108
    res = d->preparedQuerys ? d->meta : d->result;
#else
    res = d->result;
#endif

    if (!mysql_errno(d->mysql)) {
        mysql_field_seek(res, 0);
        MYSQL_FIELD* field = mysql_fetch_field(res);
        while(field) {
            info.append(qToField(field, d->tc));
            field = mysql_fetch_field(res);
        }
    }
    mysql_field_seek(res, 0);
    return info;
}


#if MYSQL_VERSION_ID >= 40108

static MYSQL_TIME *toMySqlDate(QDate date, QTime time, QCoreVariant::Type type)
{
    Q_ASSERT(type == QCoreVariant::Time || type == QCoreVariant::Date
             || type == QCoreVariant::DateTime);

    MYSQL_TIME *myTime = new MYSQL_TIME;
    memset(myTime, 0, sizeof(MYSQL_TIME));

    if (type == QCoreVariant::Time || type == QCoreVariant::DateTime) {
        myTime->hour = time.hour();
        myTime->minute = time.minute();
        myTime->second = time.second();
        myTime->second_part = time.msec();
    }
    if (type == QCoreVariant::Date || type == QCoreVariant::DateTime) {
        myTime->year = date.year();
        myTime->month = date.month();
        myTime->day = date.day();
    }

    return myTime;
}

bool QMYSQLResult::prepare(const QString& query)
{
    if (!d->preparedQuerys)
        return QSqlResult::prepare(query);

    int r;
    cleanup();

    if (query.isEmpty())
        return false;

    if (!d->stmt)
        d->stmt = mysql_stmt_init(d->mysql);
    if (!d->stmt) {
        qWarning("QMYSQLResult::prepare: unable to prepare statement");
        setLastError(qMakeError(QLatin1String("Unable to prepare statement"),
                     QSqlError::StatementError, d));
        return false;
    }

    const QByteArray encQuery(d->tc->fromUnicode(query));
    r = mysql_stmt_prepare(d->stmt, encQuery.constData(), encQuery.length());
    if (r != 0) {
        qWarning("QMYSQLResult::prepare: unable to prepare statement");
        setLastError(qMakeStmtError(QLatin1String("Unable to prepare statement"),
                     QSqlError::StatementError, d->stmt));
        cleanup();
        return false;
    }

    if (mysql_stmt_param_count(d->stmt) > 0) {// allocate memory for outvalues
        d->outBinds = new MYSQL_BIND[mysql_stmt_param_count(d->stmt)];
    }

    setSelect(d->bindInValues());
    return true;
}

bool QMYSQLResult::exec()
{
    if (!d->preparedQuerys)
        return QSqlResult::exec();
    if (!d->stmt)
        return false;

    int r = 0;
    MYSQL_BIND* currBind;
    QVector<MYSQL_TIME *> timeVector;
    QVector<QByteArray> stringVector;
    QVector<my_bool> nullVector;

    const QVector<QCoreVariant> values = boundValues();

    r = mysql_stmt_reset(d->stmt);
    if (r != 0) {
        qWarning("QMYSQLResult::exec: unable to reset statement");
        setLastError(qMakeStmtError(QLatin1String("Unable to reset statement"), QSqlError::StatementError, d->stmt));
        return false;
    }

    if (mysql_stmt_param_count(d->stmt) > 0 &&
        mysql_stmt_param_count(d->stmt) == (uint)values.count()) {

        nullVector.resize(values.count());
        for (int i = 0; i < values.count(); ++i) {
            const QCoreVariant &val = boundValues().at(i);
            void *data = const_cast<void *>(val.constData());

            currBind = &d->outBinds[i];

            nullVector[i] = static_cast<my_bool>(val.isNull());
            currBind->is_null = &nullVector[i];
            currBind->length = 0;

            switch (val.type()) {
                case QCoreVariant::ByteArray:
                currBind->buffer_type = MYSQL_TYPE_BLOB;
                currBind->buffer = const_cast<char *>(val.toByteArray().constData());
                currBind->buffer_length = val.toByteArray().size();
                break;

                case QCoreVariant::Time:
                case QCoreVariant::Date:
                case QCoreVariant::DateTime: {
                    MYSQL_TIME *myTime = toMySqlDate(val.toDate(), val.toTime(), val.type());
                    timeVector.append(myTime);

                    currBind->buffer = myTime;
                    switch(val.type()) {
                    case QCoreVariant::Time:
                        currBind->buffer_type = MYSQL_TYPE_TIME;
                        myTime->time_type = MYSQL_TIMESTAMP_TIME;
                        break;
                    case QCoreVariant::Date:
                        currBind->buffer_type = MYSQL_TYPE_DATE;
                        myTime->time_type = MYSQL_TIMESTAMP_DATE;
                        break;
                    case QCoreVariant::DateTime:
                        currBind->buffer_type = MYSQL_TYPE_DATETIME;
                        myTime->time_type = MYSQL_TIMESTAMP_DATETIME;
                        break;
                    }
                    currBind->buffer_length = sizeof(MYSQL_TIME);
                    currBind->length = 0;
                    break; }
                case QCoreVariant::UInt:
                case QCoreVariant::Int:
                    currBind->buffer_type =  MYSQL_TYPE_LONG;
                    currBind->buffer = data;
                    currBind->buffer_length = sizeof(uint);
                    currBind->is_unsigned = (val.type() == QCoreVariant::UInt);
                    break;
                case QCoreVariant::Double:
                    currBind->buffer_type =  MYSQL_TYPE_DOUBLE;
                    currBind->buffer = data;
                    currBind->buffer_length = sizeof(double);
                    currBind->is_unsigned = 0;
                    break;
                case QCoreVariant::LongLong:
                case QCoreVariant::ULongLong:
                    currBind->buffer_type =  MYSQL_TYPE_LONGLONG;
                    currBind->buffer = data;
                    currBind->buffer_length = sizeof(Q_LONGLONG);
                    currBind->is_unsigned = (val.type() == QCoreVariant::ULongLong);
                    break;
                case QCoreVariant::String:
                default: {
                    QByteArray ba = d->tc->fromUnicode(val.toString());
                    stringVector.append(ba);
                    currBind->buffer_type =  MYSQL_TYPE_STRING;
                    currBind->buffer = const_cast<char *>(ba.constData());
                    currBind->buffer_length = ba.length();
                    currBind->is_unsigned = 0;
                    break; }
            }
        }

        r = mysql_stmt_bind_param(d->stmt, d->outBinds);
        if (r != 0) {
            qWarning("QMYSQLResult::exec: unable to bind value");
            setLastError(qMakeStmtError(QLatin1String("Unable to bind value"),
                         QSqlError::StatementError, d->stmt));
            qDeleteAll(timeVector);
            return false;
        }
    }
    r = mysql_stmt_execute(d->stmt);

    qDeleteAll(timeVector);

    if (r != 0) {
        qWarning("QMYSQLResult::exec: unable to execute statement");
        setLastError(qMakeStmtError(QLatin1String("Unable to execute statement"),
                     QSqlError::StatementError, d->stmt));
        return false;
    }
    //if there is meta-data there is also data
    setSelect(d->meta);

    d->rowsAffected = mysql_stmt_affected_rows(d->stmt);

    if (isSelect()) {
        my_bool update_max_length = true;

        r = mysql_stmt_bind_result(d->stmt, d->inBinds);
        if (r != 0) {
            qWarning("QMYSQLResult::prepare: unable to bind outvalues");
            setLastError(qMakeStmtError(QLatin1String("Unable to bind outvalues"),
                         QSqlError::StatementError, d->stmt));
            return false;
        }
        if (d->hasBlobs)
            mysql_stmt_attr_set(d->stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &update_max_length);

        r = mysql_stmt_store_result(d->stmt);
        if (r != 0) {
            qWarning("QMYSQLResult::exec: unable to store statement results");
            setLastError(qMakeStmtError(QLatin1String("Unable to store statement results"),
                        QSqlError::StatementError, d->stmt));
            return false;
        }

        if (d->hasBlobs) {
            // mysql_stmt_store_result() with STMT_ATTR_UPDATE_MAX_LENGTH set to true crashes
            // when called without a preceding call to mysql_stmt_bind_result()
            // in versions < 4.1.8
            d->bindBlobs();
            r = mysql_stmt_bind_result(d->stmt, d->inBinds);
            if (r != 0) {
                qWarning("QMYSQLResult::prepare: unable to bind outvalues");
                setLastError(qMakeStmtError(QLatin1String("Unable to bind outvalues"),
                            QSqlError::StatementError, d->stmt));
                return false;
            }
        }
        setAt(QSql::BeforeFirstRow);
    }
    setActive(true);
    return true;
}
#endif
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
#if MYSQL_VERSION_ID >= 40108
    case PreparedQueries:
    case PositionalPlaceholders:
        return d->preparedQuerys;
#endif
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
        qWarning("QMYSQLDriver::open: Unknown connect option '%s'", opt.toLocal8Bit().constData());
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
                qWarning("QMYSQLDriver::open: Illegal connect option value '%s'",
                         tmp.toLocal8Bit().constData());
        } else {
            setOptionFlag(optionFlags, tmp);
        }
    }

    if ((d->mysql = mysql_init((MYSQL*) 0)) &&
            mysql_real_connect(d->mysql,
                               host.toLocal8Bit().constData(),
                               user.toLocal8Bit().constData(),
                               password.toLocal8Bit().constData(),
                               db.isNull() ? "" : db.toLocal8Bit().constData(),
                               (port > -1) ? port : 0,
                               NULL,
                               optionFlags))
    {
        if (mysql_select_db(d->mysql, db.toLocal8Bit().constData())) {
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

#if MYSQL_VERSION_ID >= 40108
    d->preparedQuerys = mysql_get_client_version() >= 40108
                        && mysql_get_server_version(d->mysql) >= 40100;
#else
    d->preparedQuerys = false;
#endif

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

QSqlResult *QMYSQLDriver::createResult() const
{
    return new QMYSQLResult(this);
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
    bool prepQ;
    if (!isOpen())
        return idx;

    prepQ = d->preparedQuerys;
    d->preparedQuerys = false;

    QSqlQuery i(createResult());
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

    d->preparedQuerys = prepQ;
    return idx;
}

QSqlRecord QMYSQLDriver::record(const QString& tablename) const
{
    QSqlRecord info;
    if (!isOpen())
        return info;
    MYSQL_RES* r = mysql_list_fields(d->mysql, tablename.toLocal8Bit().constData(), 0);
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
