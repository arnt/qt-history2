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

#include "qsql_psql.h"

#include <math.h>

#include <qcorevariant.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlrecord.h>
#include <qstringlist.h>

#include <libpq-fe.h>

// PostgreSQL header <utils/elog.h> included by <postgres.h> redefines DEBUG.
#if defined(DEBUG)
# undef DEBUG
#endif
#include <postgres.h>
#include <libpq/libpq-fs.h>
// PostgreSQL header <catalog/pg_type.h> redefines errno erroneously.
#if defined(errno)
# undef errno
#endif
#define errno qt_psql_errno
#include <catalog/pg_type.h>
#undef errno

class QPSQLDriverPrivate
{
public:
    QPSQLDriverPrivate(): connection(0), isUtf8(false), pro(QPSQLDriver::Version6) {}
    PGconn *connection;
    bool isUtf8;
    QPSQLDriver::Protocol pro;

    void appendTables(QStringList &tl, QSqlQuery &t, QChar type);
};

void QPSQLDriverPrivate::appendTables(QStringList &tl, QSqlQuery &t, QChar type)
{
    QString query;
    if (pro >= QPSQLDriver::Version73) {
        query = QString::fromLatin1("select pg_class.relname, pg_namespace.nspname from pg_class "
                  "left join pg_namespace on (pg_class.relnamespace = pg_namespace.oid) "
                  "where (pg_class.relkind = '%1') and (pg_class.relname !~ '^Inv') "
                  "and (pg_namespace.nspname != 'information_schema')").arg(type);
    } else {
        query = QString::fromLatin1("select relname, null from pg_class where (relkind = 'r') "
                  "and (relname !~ '^Inv') "
                  "and (relname !~ '^pg_') ");
    }
    t.exec(query);
    while (t.next()) {
        QString schema = t.value(1).toString();
        if (schema.isEmpty() || schema == QLatin1String("public"))
            tl.append(t.value(0).toString());
        else
            tl.append(t.value(0).toString().prepend(QLatin1Char('.')).prepend(schema));
    }
}

class QPSQLResultPrivate
{
public:
    QPSQLResultPrivate(QPSQLResult *qq): q(qq), driver(0), result(0), currentSize(-1) {}

    QPSQLResult *q;
    const QPSQLDriverPrivate *driver;
    PGresult *result;
    int currentSize;

    bool processResults();
};

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type,
                            const QPSQLDriverPrivate *p)
{
    const char *s = PQerrorMessage(p->connection);
    QString msg = p->isUtf8 ? QString::fromUtf8(s) : QString::fromLocal8Bit(s);
    return QSqlError(QLatin1String("QPSQL: ") + err, msg, type);
}

bool QPSQLResultPrivate::processResults()
{
    if (!result)
        return false;

    int status = PQresultStatus(result);
    if (status == PGRES_TUPLES_OK) {
        q->setSelect(true);
        q->setActive(true);
        currentSize = PQntuples(result);
        return true;
    } else if (status == PGRES_COMMAND_OK) {
        q->setSelect(false);
        q->setActive(true);
        currentSize = -1;
        return true;
    }
    q->setLastError(qMakeError(QLatin1String("Unable to create query"), QSqlError::StatementError,
                               driver));
    return false;
}

static QCoreVariant::Type qDecodePSQLType(int t)
{
    QCoreVariant::Type type = QCoreVariant::Invalid;
    switch (t) {
    case BOOLOID        :
        type = QCoreVariant::Bool;
        break;
    case INT8OID        :
        type = QCoreVariant::LongLong;
        break;
    case INT2OID        :
        //    case INT2VECTOROID  : // 7.x
    case INT4OID        :
        type = QCoreVariant::Int;
        break;
    case NUMERICOID     :
    case FLOAT4OID      :
    case FLOAT8OID      :
        type = QCoreVariant::Double;
        break;
    case ABSTIMEOID     :
    case RELTIMEOID     :
    case DATEOID        :
        type = QCoreVariant::Date;
        break;
    case TIMEOID        :
#ifdef TIMETZOID // 7.x
        case TIMETZOID  :
#endif
        type = QCoreVariant::Time;
        break;
    case TIMESTAMPOID   :
#ifdef DATETIMEOID
    // Postgres 6.x datetime workaround.
    // DATETIMEOID == TIMESTAMPOID (only the names have changed)
    case DATETIMEOID    :
#endif
#ifdef TIMESTAMPTZOID
    // Postgres 7.2 workaround
    // TIMESTAMPTZOID == TIMESTAMPOID == DATETIMEOID
    case TIMESTAMPTZOID :
#endif
        type = QCoreVariant::DateTime;
        break;
        //    case ZPBITOID        : // 7.x
        //    case VARBITOID        : // 7.x
    case OIDOID         :
    case BYTEAOID       :
        type = QCoreVariant::ByteArray;
        break;
    case REGPROCOID     :
    case XIDOID         :
    case CIDOID         :
        //    case OIDVECTOROID   : // 7.x
    case UNKNOWNOID     :
        //    case TINTERVALOID   : // 7.x
        type = QCoreVariant::Invalid;
        break;
    default:
    case TIDOID         :
    case CHAROID        :
    case BPCHAROID        :
        //    case LZTEXTOID        : // 7.x
    case VARCHAROID        :
    case TEXTOID        :
    case NAMEOID        :
    case CASHOID        :
    case INETOID        :
    case CIDROID        :
    case CIRCLEOID      :
        type = QCoreVariant::String;
        break;
    }
    return type;
}

QPSQLResult::QPSQLResult(const QPSQLDriver* db, const QPSQLDriverPrivate* p)
    : QSqlResult(db)
{
    d = new QPSQLResultPrivate(this);
    d->driver = p;
}

QPSQLResult::~QPSQLResult()
{
    cleanup();
    delete d;
}

PGresult* QPSQLResult::result()
{
    return d->result;
}

void QPSQLResult::cleanup()
{
    if (d->result)
        PQclear(d->result);
    d->result = 0;
    setAt(QSql::BeforeFirstRow);
    d->currentSize = -1;
    setActive(false);
}

bool QPSQLResult::fetch(int i)
{
    if (!isActive())
        return false;
    if (i < 0)
        return false;
    if (i >= d->currentSize)
        return false;
    if (at() == i)
        return true;
    setAt(i);
    return true;
}

bool QPSQLResult::fetchFirst()
{
    return fetch(0);
}

bool QPSQLResult::fetchLast()
{
    return fetch(PQntuples(d->result) - 1);
}

QCoreVariant QPSQLResult::data(int i)
{
    if (i >= PQnfields(d->result)) {
        qWarning("QPSQLResult::data: column %d out of range", i);
        return QCoreVariant();
    }
    int ptype = PQftype(d->result, i);
    QCoreVariant::Type type = qDecodePSQLType(ptype);
    const char *val = PQgetvalue(d->result, at(), i);
    if (PQgetisnull(d->result, at(), i))
        return QCoreVariant(type);
    switch (type) {
    case QCoreVariant::Bool:
        return QCoreVariant((bool)(val[0] == 't'));
    case QCoreVariant::String:
        return d->driver->isUtf8 ? QString::fromUtf8(val) : QString::fromLocal8Bit(val);
    case QCoreVariant::LongLong:
        if (val[0] == '-')
            return QString::fromLatin1(val).toLongLong();
        else
            return QString::fromLatin1(val).toULongLong();
    case QCoreVariant::Int:
        return atoi(val);
    case QCoreVariant::Double:
        if (ptype == NUMERICOID)
            return QString::fromAscii(val);
        return strtod(val, 0);
    case QCoreVariant::Date:
        if (val[0] == '\0') {
            return QCoreVariant(QDate());
        } else {
            return QCoreVariant(QDate::fromString(QString::fromLatin1(val), Qt::ISODate));
        }
    case QCoreVariant::Time: {
        const QString str = QString::fromLatin1(val);
        if (str.isEmpty())
            return QCoreVariant(QTime());
        if (str.at(str.length() - 3) == QLatin1Char('+'))
            // strip the timezone
            return QCoreVariant(QTime::fromString(str.left(str.length() - 3), Qt::ISODate));
        return QCoreVariant(QTime::fromString(str, Qt::ISODate));
    }
    case QCoreVariant::DateTime: {
        QString dtval = QString::fromLatin1(val);
        if (dtval.length() < 10)
            return QCoreVariant(QDateTime());
        // remove the timezone
        if (dtval.at(dtval.length() - 3) == QLatin1Char('+'))
            dtval.chop(3);
        // milliseconds are sometimes returned with 2 digits only
        if (dtval.at(dtval.length() - 3).isPunct())
            dtval += QLatin1Char('0');
        if (dtval.isEmpty())
            return QCoreVariant(QDateTime());
        else
            return QCoreVariant(QDateTime::fromString(dtval, Qt::ISODate));
    }
    case QCoreVariant::ByteArray: {
        if (ptype == BYTEAOID) {
            int i = 0;
            QByteArray ba(val);
            while (i < ba.length()) {
                if (ba.at(i) == '\\') {
                    if (QChar::fromLatin1(ba.at(i + 1)).isDigit()) {
                        char *v = ba.data() + i + 3;
                        ba[i] = static_cast<char>(strtol(v - 2, &v, 8));
                        ba.remove(i + 1, 3);
                    } else {
                        ba[i] = ba.at(i + 1);
                        ba.remove(i + 1, 1);
                    }
                }
                ++i;
            }
            return QCoreVariant(ba);
        }

        QByteArray ba;
        const_cast<QSqlDriver *>(driver())->beginTransaction();
        Oid oid = atoi(val);
        int fd = lo_open(d->driver->connection, oid, INV_READ);
        if (fd < 0) {
            qWarning("QPSQLResult::data: unable to open large object for read");
            const_cast<QSqlDriver *>(driver())->commitTransaction();
            return QCoreVariant(ba);
        }
        int size = 0;
        int retval = lo_lseek(d->driver->connection, fd, 0L, SEEK_END);
        if (retval >= 0) {
            size = lo_tell(d->driver->connection, fd);
            lo_lseek(d->driver->connection, fd, 0L, SEEK_SET);
        }
        if (size == 0) {
            lo_close(d->driver->connection, fd);
            const_cast<QSqlDriver *>(driver())->commitTransaction();
            return QCoreVariant(ba);
        }
        char * buf = new char[size];

#ifdef Q_OS_WIN32
        // For some reason lo_read() fails if we try to read more than
        // 32760 bytes
        char * p = buf;
        int nread = 0;

        while(size < nread){
                retval = lo_read(d->driver->connection, fd, p, 32760);
                nread += retval;
                p += retval;
        }
#else
        retval = lo_read(d->driver->connection, fd, buf, size);
#endif

        if (retval < 0) {
            qWarning("QPSQLResult::data: unable to read large object");
        } else {
            ba = QByteArray(buf, size);
        }
        delete [] buf;
        lo_close(d->driver->connection, fd);
        const_cast<QSqlDriver *>(driver())->commitTransaction();
        return QCoreVariant(ba);
    }
    default:
    case QCoreVariant::Invalid:
        qWarning("QPSQLResult::data: unknown data type");
    }
    return QCoreVariant();
}

bool QPSQLResult::isNull(int field)
{
    PQgetvalue(d->result, at(), field);
    return PQgetisnull(d->result, at(), field);
}

bool QPSQLResult::reset (const QString& query)
{
    cleanup();
    if (!driver())
        return false;
    if (!driver()->isOpen() || driver()->isOpenError())
        return false;
    d->result = PQexec(d->driver->connection,
                       d->driver->isUtf8 ? query.toUtf8().constData()
                                         : query.toLocal8Bit().constData());
    return d->processResults();
}

int QPSQLResult::size()
{
    return d->currentSize;
}

int QPSQLResult::numRowsAffected()
{
    return QString::fromLatin1(PQcmdTuples(d->result)).toInt();
}

QSqlRecord QPSQLResult::record() const
{
    QSqlRecord info;
    if (!isActive() || !isSelect())
        return info;

    int count = PQnfields(d->result);
    for (int i = 0; i < count; ++i) {
        QSqlField f;
        if (d->driver->isUtf8)
            f.setName(QString::fromUtf8(PQfname(d->result, i)));
        else
            f.setName(QString::fromLocal8Bit(PQfname(d->result, i)));
        f.setType(qDecodePSQLType(PQftype(d->result, i)));
        int len = PQfsize(d->result, i);
        int precision = PQfmod(d->result, i);
        // swap length and precision if length == -1
        if (len == -1 && precision > -1) {
            len = precision - 4;
            precision = -1;
        }
        f.setLength(len);
        f.setPrecision(precision);
        f.setSqlType(PQftype(d->result, i));
        info.append(f);
    }
    return info;
}

///////////////////////////////////////////////////////////////////

static bool setEncodingUtf8(PGconn* connection)
{
    PGresult* result = PQexec(connection, "SET CLIENT_ENCODING TO 'UNICODE'");
    int status = PQresultStatus(result);
    PQclear(result);
    return status == PGRES_COMMAND_OK;
}

static void setDatestyle(PGconn* connection)
{
    PGresult* result = PQexec(connection, "SET DATESTYLE TO 'ISO'");
    int status =  PQresultStatus(result);
    if (status != PGRES_COMMAND_OK)
        qWarning("%s", PQerrorMessage(connection));
    PQclear(result);
}

static QPSQLDriver::Protocol getPSQLVersion(PGconn* connection)
{
    PGresult* result = PQexec(connection, "select version()");
    int status =  PQresultStatus(result);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
        QString val = QString::fromAscii(PQgetvalue(result, 0, 0));
        PQclear(result);
        QRegExp rx(QLatin1String("(\\d+)\\.(\\d+)"));
        rx.setMinimal(true); // enforce non-greedy RegExp
        if (rx.indexIn(val) != -1) {
            int vMaj = rx.cap(1).toInt();
            int vMin = rx.cap(2).toInt();
            if (vMaj < 6) {
                qWarning("This version of PostgreSQL is not supported and may not work.");
                return QPSQLDriver::Version6;
            }
            if (vMaj == 6) {
                return QPSQLDriver::Version6;
            } else if (vMaj == 7) {
                if (vMin < 1)
                    return QPSQLDriver::Version7;
                else if (vMin < 3)
                    return QPSQLDriver::Version71;
            }
            return QPSQLDriver::Version73;
        }
    } else {
        qWarning("This version of PostgreSQL is not supported and may not work.");
    }

    return QPSQLDriver::Version6;
}

QPSQLDriver::QPSQLDriver(QObject *parent)
    : QSqlDriver(parent)
{
    init();
}

QPSQLDriver::QPSQLDriver(PGconn * conn, QObject * parent)
    : QSqlDriver(parent)
{
    init();
    d->connection = conn;
    if (conn) {
        d->pro = getPSQLVersion(d->connection);
        setOpen(true);
        setOpenError(false);
    }
}

void QPSQLDriver::init()
{
    d = new QPSQLDriverPrivate();
}

QPSQLDriver::~QPSQLDriver()
{
    if (d->connection)
        PQfinish(d->connection);
    delete d;
}

PGconn* QPSQLDriver::connection()
{
    return d->connection;
}


bool QPSQLDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
        return true;
    case QuerySize:
        return true;
    case BLOB:
        return d->pro >= QPSQLDriver::Version71;
    case Unicode:
        return d->isUtf8;
    default:
        return false;
    }
}

bool QPSQLDriver::open(const QString & db,
                        const QString & user,
                        const QString & password,
                        const QString & host,
                        int port,
                        const QString& connOpts)
{
    if (isOpen())
        close();
    QString connectString;
    if (host.length())
        connectString.append(QLatin1String("host=")).append(host);
    if (db.length())
        connectString.append(QLatin1String(" dbname=")).append(db);
    if (user.length())
        connectString.append(QLatin1String(" user=")).append(user);
    if (password.length())
        connectString.append(QLatin1String(" password=")).append(password);
    if (port > -1)
        connectString.append(QLatin1String(" port=")).append(QString::number(port));

    // add any connect options - the server will handle error detection
    if (!connOpts.isEmpty()) {
        QString opt = connOpts;
        opt.replace(QLatin1Char(';'), QLatin1Char(' '), Qt::CaseInsensitive);
        connectString.append(QLatin1Char(' ')).append(opt);
    }

    d->connection = PQconnectdb(connectString.toLocal8Bit().constData());
    if (PQstatus(d->connection) == CONNECTION_BAD) {
        setLastError(qMakeError(QLatin1String("Unable to connect"), QSqlError::ConnectionError, d));
        setOpenError(true);
        return false;
    }

    d->pro = getPSQLVersion(d->connection);
    d->isUtf8 = setEncodingUtf8(d->connection);
    setDatestyle(d->connection);

    setOpen(true);
    setOpenError(false);
    return true;
}

void QPSQLDriver::close()
{
    if (isOpen()) {
        if (d->connection)
            PQfinish(d->connection);
        d->connection = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *QPSQLDriver::createResult() const
{
    return new QPSQLResult(this, d);
}

bool QPSQLDriver::beginTransaction()
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::beginTransaction: Database not open");
        return false;
    }
    PGresult* res = PQexec(d->connection, "BEGIN");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        setLastError(qMakeError(QLatin1String("Could not begin transaction"),
                                QSqlError::TransactionError, d));
        return false;
    }
    PQclear(res);
    return true;
}

bool QPSQLDriver::commitTransaction()
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::commitTransaction: Database not open");
        return false;
    }
    PGresult* res = PQexec(d->connection, "COMMIT");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        setLastError(qMakeError(QLatin1String("Could not commit transaction"),
                                QSqlError::TransactionError, d));
        return false;
    }
    PQclear(res);
    return true;
}

bool QPSQLDriver::rollbackTransaction()
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::rollbackTransaction: Database not open");
        return false;
    }
    PGresult* res = PQexec(d->connection, "ROLLBACK");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        setLastError(qMakeError(QLatin1String("Could not rollback transaction"),
                                QSqlError::TransactionError, d));
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

QStringList QPSQLDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;
    QSqlQuery t(createResult());
    t.setForwardOnly(true);

    if (type & QSql::Tables)
        d->appendTables(tl, t, QLatin1Char('r'));
    if (type & QSql::Views)
        d->appendTables(tl, t, QLatin1Char('v'));
    if (type & QSql::SystemTables) {
        t.exec(QLatin1String("select relname from pg_class where (relkind = 'r') "
                "and (relname like 'pg_%') "));
        while (t.next())
            tl.append(t.value(0).toString());
    }

    return tl;
}

static void qSplitTableName(QString &tablename, QString &schema)
{
    int dot = tablename.indexOf(QLatin1Char('.'));
    if (dot == -1)
        return;
    schema = tablename.left(dot);
    tablename = tablename.mid(dot + 1);
}

QSqlIndex QPSQLDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex idx(tablename);
    if (!isOpen())
        return idx;
    QSqlQuery i(createResult());
    QString stmt;

    QString tbl = tablename;
    QString schema;
    qSplitTableName(tbl, schema);

    switch(d->pro) {
    case QPSQLDriver::Version6:
        stmt = QLatin1String("select pg_att1.attname, int(pg_att1.atttypid), pg_cl.relname "
                "from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
                "where lower(pg_cl.relname) = '%1_pkey' "
                "and pg_cl.oid = pg_ind.indexrelid "
                "and pg_att2.attrelid = pg_ind.indexrelid "
                "and pg_att1.attrelid = pg_ind.indrelid "
                "and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
                "order by pg_att2.attnum");
        break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
        stmt = QLatin1String("select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
                "from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
                "where lower(pg_cl.relname) = '%1_pkey' "
                "and pg_cl.oid = pg_ind.indexrelid "
                "and pg_att2.attrelid = pg_ind.indexrelid "
                "and pg_att1.attrelid = pg_ind.indrelid "
                "and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
                "order by pg_att2.attnum");
        break;
    case QPSQLDriver::Version73:
        stmt = QLatin1String("SELECT pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_class.relname "
                "FROM pg_attribute, pg_class "
                "WHERE %1 pg_class.oid = "
                "(SELECT indexrelid FROM pg_index WHERE indisprimary = true AND indrelid = "
                " (SELECT oid FROM pg_class WHERE lower(relname) = '%2')) "
                "AND pg_attribute.attrelid = pg_class.oid "
                "AND pg_attribute.attisdropped = false "
                "ORDER BY pg_attribute.attnum");
        if (schema.isEmpty())
            stmt = stmt.arg(QLatin1String(""));
        else
            stmt = stmt.arg(QString::fromLatin1("pg_class.relnamespace = (select oid from "
                   "pg_namespace where pg_namespace.nspname = '%1') AND ").arg(schema.toLower()));
        break;
    }

    i.exec(stmt.arg(tbl.toLower()));
    while (i.isActive() && i.next()) {
        QSqlField f(i.value(0).toString(), qDecodePSQLType(i.value(1).toInt()));
        idx.append(f);
        idx.setName(i.value(2).toString());
    }
    return idx;
}

QSqlRecord QPSQLDriver::record(const QString& tablename) const
{
    QSqlRecord info;
    if (!isOpen())
        return info;

    QString tbl = tablename;
    QString schema;
    qSplitTableName(tbl, schema);

    QString stmt;
    switch(d->pro) {
    case QPSQLDriver::Version6:
        stmt = QLatin1String("select pg_attribute.attname, int(pg_attribute.atttypid), "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "int(pg_attribute.attrelid), pg_attribute.attnum "
                "from pg_class, pg_attribute "
                "where lower(pg_class.relname) = '%1' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid ");
        break;
    case QPSQLDriver::Version7:
        stmt = QLatin1String("select pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "pg_attribute.attrelid::int, pg_attribute.attnum "
                "from pg_class, pg_attribute "
                "where lower(pg_class.relname) = '%1' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid ");
        break;
    case QPSQLDriver::Version71:
        stmt = QLatin1String("select pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "pg_attrdef.adsrc "
                "from pg_class, pg_attribute "
                "left join pg_attrdef on (pg_attrdef.adrelid = "
                "pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
                "where lower(pg_class.relname) = '%1' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid "
                "order by pg_attribute.attnum ");
        break;
    case QPSQLDriver::Version73:
        stmt = QLatin1String("select pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "pg_attrdef.adsrc "
                "from pg_class, pg_attribute "
                "left join pg_attrdef on (pg_attrdef.adrelid = "
                "pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
                "where %1 "
                "lower(pg_class.relname) = '%2' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid "
                "and pg_attribute.attisdropped = false "
                "order by pg_attribute.attnum ");
        if (schema.isEmpty())
            stmt = stmt.arg(QLatin1String(""));
        else
            stmt = stmt.arg(QString::fromLatin1("pg_class.relnamespace = (select oid from "
                   "pg_namespace where pg_namespace.nspname = '%1') and ").arg(schema.toLower()));
        break;
    }

    QSqlQuery query(createResult());
    query.exec(stmt.arg(tbl.toLower()));
    if (d->pro >= QPSQLDriver::Version71) {
        while (query.next()) {
            int len = query.value(3).toInt();
            int precision = query.value(4).toInt();
            // swap length and precision if length == -1
            if (len == -1 && precision > -1) {
                len = precision - 4;
                precision = -1;
            }
            QString defVal = query.value(5).toString();
            if (!defVal.isEmpty() && defVal.at(0) == QLatin1Char('\''))
                defVal = defVal.mid(1, defVal.length() - 2);
            QSqlField f(query.value(0).toString(), qDecodePSQLType(query.value(1).toInt()));
            f.setRequired(query.value(2).toBool());
            f.setLength(len);
            f.setPrecision(precision);
            f.setDefaultValue(defVal);
            f.setSqlType(query.value(1).toInt());
            info.append(f);
        }
    } else {
        // Postgres < 7.1 cannot handle outer joins
        while (query.next()) {
            QString defVal;
            QString stmt2 = QLatin1String("select pg_attrdef.adsrc from pg_attrdef where "
                            "pg_attrdef.adrelid = %1 and pg_attrdef.adnum = %2 ");
            QSqlQuery query2(createResult());
            query2.exec(stmt2.arg(query.value(5).toInt()).arg(query.value(6).toInt()));
            if (query2.isActive() && query2.next())
                defVal = query2.value(0).toString();
            if (!defVal.isEmpty() && defVal.at(0) == QLatin1Char('\''))
                defVal = defVal.mid(1, defVal.length() - 2);
            int len = query.value(3).toInt();
            int precision = query.value(4).toInt();
            // swap length and precision if length == -1
            if (len == -1 && precision > -1) {
                len = precision - 4;
                precision = -1;
            }
            QSqlField f(query.value(0).toString(), qDecodePSQLType(query.value(1).toInt()));
            f.setRequired(query.value(2).toBool());
            f.setLength(len);
            f.setPrecision(precision);
            f.setDefaultValue(defVal);
            f.setSqlType(query.value(1).toInt());
            info.append(f);
        }
    }

    return info;
}

QString QPSQLDriver::formatValue(const QSqlField &field,
                                  bool) const
{
    QString r;
    if (field.isNull()) {
        r = QLatin1String("NULL");
    } else {
        switch (field.type()) {
        case QCoreVariant::DateTime:
            if (field.value().toDateTime().isValid()) {
                QDate dt = field.value().toDateTime().date();
                QTime tm = field.value().toDateTime().time();
                // msecs need to be right aligned otherwise psql
                // interpretes them wrong
                r = QLatin1String("'") + QString::number(dt.year()) + QLatin1String("-")
                          + QString::number(dt.month()) + QLatin1String("-")
                          + QString::number(dt.day()) + QLatin1String(" ")
                          + tm.toString() + QLatin1String(".")
                          + QString::number(tm.msec()).rightJustified(3, QLatin1Char('0'))
                          + QLatin1String("'");
            } else {
                r = QLatin1String("NULL");
            }
            break;
        case QCoreVariant::Time:
            if (field.value().toTime().isValid()) {
                r = field.value().toTime().toString(Qt::ISODate);
            } else {
                r = QLatin1String("NULL");
            }
        case QCoreVariant::String:
        {
            // Escape '\' characters
            r = QSqlDriver::formatValue(field);
            r.replace(QLatin1String("\\"), QLatin1String("\\\\"));
            break;
        }
        case QCoreVariant::Bool:
            if (field.value().toBool())
                r = QLatin1String("TRUE");
            else
                r = QLatin1String("FALSE");
            break;
        case QCoreVariant::ByteArray: {
            QByteArray ba(field.value().toByteArray());
            QString res;
            r = QLatin1String("'");
            unsigned char uc;
            for (int i = 0; i < ba.size(); ++i) {
                uc = (unsigned char) ba[i];
                if (uc > 40 && uc < 92) {
                    r += QLatin1Char(uc);
                } else {
                    r += QLatin1String("\\\\");
                    r += QString::number((unsigned char) ba[i], 8).rightJustified(3,
                            QLatin1Char('0'), true);
                }
            }
            r += QLatin1String("'");
            break;
        }
        default:
            r = QSqlDriver::formatValue(field);
            break;
        }
    }
    return r;
}

QString QPSQLDriver::escapeIdentifier(const QString &identifier) const
{
    QString res = identifier;
    res.replace(QLatin1Char('"'), QLatin1String("\"\""));
    res.prepend(QLatin1Char('"')).append(QLatin1Char('"'));
    int idx = identifier.indexOf(QLatin1Char('.'));
    if (idx != -1)
        res.replace(QLatin1Char('.'), QLatin1String("\".\""));
    return res;
}

bool QPSQLDriver::isOpen() const
{
    return PQstatus(d->connection) == CONNECTION_OK;
}

QPSQLDriver::Protocol QPSQLDriver::protocol() const
{
    return d->pro;
}

