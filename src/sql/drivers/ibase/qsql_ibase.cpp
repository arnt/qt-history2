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

#include "qsql_ibase.h"

#include <qdatetime.h>
#include <qcorevariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qstringlist.h>

#include <qdebug.h>

#include <ibase.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

static bool getIBaseError(QString& msg, ISC_STATUS* status, long &sqlcode)
{
    if (status[0] != 1 || status[1] <= 0)
        return false;

    sqlcode = isc_sqlcode(status);
    char buf[512];
    isc_sql_interprete(sqlcode, buf, 512);
    msg = QString::fromUtf8(buf);
    return true;
}

static void createDA(XSQLDA *&sqlda)
{
    sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(1));
    sqlda->sqln = 1;
    sqlda->sqld = 0;
    sqlda->version = SQLDA_VERSION1;
    sqlda->sqlvar[0].sqlind = 0;
    sqlda->sqlvar[0].sqldata = 0;
}

static void enlargeDA(XSQLDA *&sqlda, int n)
{
    free(sqlda);
    sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(n));
    sqlda->sqln = n;
    sqlda->version = SQLDA_VERSION1;
}

static void initDA(XSQLDA *sqlda)
{
    for (int i = 0; i < sqlda->sqld; ++i) {
        switch (sqlda->sqlvar[i].sqltype & ~1) {
        case SQL_INT64:
        case SQL_LONG:
        case SQL_SHORT:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIME:
        case SQL_TEXT:
        case SQL_BLOB:
//        case SQL_ARRAY:
            sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen);
            break;
        case SQL_ARRAY:
            qDebug("init: Array");
            sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen);
            break;
        case SQL_VARYING:
            sqlda->sqlvar[i].sqldata = (char*)malloc(sqlda->sqlvar[i].sqllen + sizeof(short));
            break;
        default:
            // not supported - do not bind.
            sqlda->sqlvar[i].sqldata = 0;
            break;
        }
        if (sqlda->sqlvar[i].sqltype & 1) {
            sqlda->sqlvar[i].sqlind = (short*)malloc(sizeof(short));
            *(sqlda->sqlvar[i].sqlind) = 0;
        } else {
            sqlda->sqlvar[i].sqlind = 0;
        }
    }
}

static void delDA(XSQLDA *&sqlda)
{
    if (!sqlda)
        return;
    for (int i = 0; i < sqlda->sqld; ++i) {
        free(sqlda->sqlvar[i].sqlind);
        free(sqlda->sqlvar[i].sqldata);
    }
    free(sqlda);
    sqlda = 0;
}

static QCoreVariant::Type qIBaseTypeName(int iType)
{
    switch (iType) {
    case blr_varying:
    case blr_varying2:
    case blr_text:
    case blr_cstring:
    case blr_cstring2:
        return QCoreVariant::String;
    case blr_sql_time:
        return QCoreVariant::Time;
    case blr_sql_date:
    case blr_timestamp:
        return QCoreVariant::DateTime;
    case blr_blob:
        return QCoreVariant::ByteArray;
    case blr_quad:
    case blr_short:
    case blr_long:
        return QCoreVariant::Int;
    case blr_int64:
        return QCoreVariant::LongLong;
    case blr_float:
    case blr_d_float:
    case blr_double:
        return QCoreVariant::Double;
    }
    qWarning("qIBaseTypeName: unknown datatype: %d", iType);
    return QCoreVariant::Invalid;
}

static QCoreVariant::Type qIBaseTypeName2(int iType)
{
    switch(iType & ~1) {
    case SQL_VARYING:
    case SQL_TEXT:
        return QCoreVariant::String;
    case SQL_LONG:
    case SQL_SHORT:
        return QCoreVariant::Int;
    case SQL_INT64:
        return QCoreVariant::LongLong;
    case SQL_FLOAT:
    case SQL_DOUBLE:
        return QCoreVariant::Double;
    case SQL_TIMESTAMP:
        return QCoreVariant::DateTime;
    case SQL_TYPE_TIME:
        return QCoreVariant::Time;
    case SQL_ARRAY:
        return QCoreVariant::List;
    default:
        return QCoreVariant::Invalid;
    }
}

static int qIBaseTypeLength(int iType, int scale)
{
    switch(iType & ~1) {
    case SQL_VARYING:
    case SQL_TEXT:
        return qAbs(scale);
    case SQL_LONG:
        return sizeof(long);
    case SQL_SHORT:
        return sizeof(short);
    case SQL_INT64:
        return sizeof(Q_LONGLONG);
    case SQL_FLOAT:
        return sizeof(float);
    case SQL_DOUBLE:
        return sizeof(double);
    case SQL_TIMESTAMP:
        return sizeof(ISC_TIMESTAMP);
    case SQL_TYPE_TIME:
        return sizeof(ISC_TIME);
    default:
        return -1;
    }
}

static ISC_TIMESTAMP toTimeStamp(const QDateTime &dt)
{
    static const QTime midnight(0, 0, 0, 0);
    static const QDate basedate(1858, 11, 17);
    ISC_TIMESTAMP ts;
    ts.timestamp_time = midnight.msecsTo(dt.time()) * 10;
    ts.timestamp_date = basedate.daysTo(dt.date());
    return ts;
}

static ISC_TIME toTime(const QTime &t)
{
    static const QTime midnight(0, 0, 0, 0);
    return (ISC_TIME)midnight.msecsTo(t) * 10;
}

class QIBaseDriverPrivate
{
public:
    QIBaseDriverPrivate(QIBaseDriver *d) : q(d)
    {
        ibase = 0;
        trans = 0;
    }

    bool isError(const QString &msg = QString::null,
                 QSqlError::ErrorType typ = QSqlError::UnknownError)
    {
        QString imsg;
        long sqlcode;
        if (!getIBaseError(imsg, status, sqlcode))
            return false;

        q->setLastError(QSqlError(msg, imsg, typ, int(sqlcode)));
        return true;
    }

public:
    QIBaseDriver* q;
    isc_db_handle ibase;
    isc_tr_handle trans;
    ISC_STATUS status[20];
};

class QIBaseResultPrivate
{
public:
    QIBaseResultPrivate(QIBaseResult *d, const QIBaseDriver *ddb);
    ~QIBaseResultPrivate() { cleanup(); }

    void cleanup();
    bool isError(const QString &msg = QString::null,
                 QSqlError::ErrorType typ = QSqlError::UnknownError)
    {
        QString imsg;
        long sqlcode;
        if (!getIBaseError(imsg, status, sqlcode))
            return false;

        q->setLastError(QSqlError(msg, imsg, typ, int(sqlcode)));
        return true;
    }

    bool transaction();
    bool commit();

    bool isSelect();
    QCoreVariant fetchBlob(ISC_QUAD *bId);
    void writeBlob(int i, const QByteArray &ba);
    QCoreVariant fetchArray(int pos, ISC_QUAD *arr);
    void writeArray(int i, const QList<QCoreVariant> &list);

public:
    QIBaseResult *q;
    const QIBaseDriver *db;
    ISC_STATUS status[20];
    isc_tr_handle trans;
    //indicator whether we have a local transaction or a transaction on driver level
    bool localTransaction;
    isc_stmt_handle stmt;
    isc_db_handle ibase;
    XSQLDA *sqlda; // output sqlda
    XSQLDA *inda; // input parameters
    int queryType;
};

QIBaseResultPrivate::QIBaseResultPrivate(QIBaseResult *d, const QIBaseDriver *ddb):
    q(d), db(ddb), trans(0), stmt(0), ibase(ddb->d->ibase), sqlda(0), inda(0), queryType(-1)
{
    localTransaction = (ddb->d->ibase == 0);
}

void QIBaseResultPrivate::cleanup()
{
    if (stmt) {
        isc_dsql_free_statement(status, &stmt, DSQL_drop);
        stmt = 0;
    }

    commit();
    if (!localTransaction)
        trans = 0;

    delDA(sqlda);
    delDA(inda);

    queryType = -1;
    q->cleanup();
}

void QIBaseResultPrivate::writeBlob(int i, const QByteArray &ba)
{
    isc_blob_handle handle = 0;
    ISC_QUAD *bId = (ISC_QUAD*)inda->sqlvar[i].sqldata;
    isc_create_blob2(status, &ibase, &trans, &handle, bId, 0, 0);
    if (!isError(QLatin1String("Unable to create BLOB"), QSqlError::StatementError)) {
        int i = 0;
        while (i < ba.size()) {
            isc_put_segment(status, &handle, qMin(ba.size() - i, SHRT_MAX), const_cast<char*>(ba.data()));
            if (isError(QLatin1String("Unable to write BLOB")))
                break;
            i += SHRT_MAX;
        }
    }
    isc_close_blob(status, &handle);
}

QCoreVariant QIBaseResultPrivate::fetchBlob(ISC_QUAD *bId)
{
    isc_blob_handle handle = 0;

    isc_open_blob2(status, &ibase, &trans, &handle, bId, 0, 0);
    if (isError(QLatin1String("Unable to open BLOB"), QSqlError::StatementError))
        return QCoreVariant();

    unsigned short len = 0;
    QByteArray ba;
    ba.resize(255);
    ISC_STATUS stat = isc_get_segment(status, &handle, &len, ba.size(), ba.data());
    while (status[1] == isc_segment) {
        uint osize = ba.size();
        // double the amount of data fetched on each iteration
        ba.resize(qMin(ba.size() * 2, SHRT_MAX));
        stat = isc_get_segment(status, &handle, &len, osize, ba.data() + osize);
    }
    bool isErr = isError(QLatin1String("Unable to read BLOB"), QSqlError::StatementError);
    isc_close_blob(status, &handle);
    if (isErr)
        return QCoreVariant();

    if (ba.size() > 255)
        ba.resize(ba.size() / 2 + len);
    else
        ba.resize(len);

    return ba;
}

template<typename T>
static QList<QCoreVariant> toList(char* buf, int count)
{
    qDebug("toList");
    QList<QCoreVariant> res;
    for (int i = 0; i < count; ++i)
        res.append(*(T*)(buf + sizeof(T) * i));
    return res;
}

template<>
static QList<QCoreVariant> toList<long>(char* buf, int count)
{
    qDebug("toLongList");
    QList<QCoreVariant> res;
    for (int i = 0; i < count; ++i) {
        if (sizeof(int) == sizeof(long))
            res.append(int((*(long*)(buf + sizeof(long) * i))));
        else
            res.append((Q_LONGLONG)(*(long*)(buf + sizeof(long) * i)));
    }
    return res;
}

QCoreVariant QIBaseResultPrivate::fetchArray(int pos, ISC_QUAD *arr)
{
    QCoreVariant res;
    if (!arr)
        return res;

    QByteArray relname(sqlda->sqlvar[pos].relname, sqlda->sqlvar[pos].relname_length);
    QByteArray sqlname(sqlda->sqlvar[pos].sqlname, sqlda->sqlvar[pos].sqlname_length);
    qDebug() << relname << "." << sqlname;

    ISC_ARRAY_DESC desc;

    isc_array_lookup_bounds(status, &ibase, &trans, relname.data(), sqlname.data(), &desc);
    if (isError(QLatin1String("Could not find array at position ") + QString::number(pos),
                QSqlError::StatementError))
        return res;

//    qDebug() << "dtype" << desc.array_desc_dtype << "scale" << (int)desc.array_desc_scale << "length" << desc.array_desc_length << "dimensions" << desc.array_desc_dimensions;

    long buflen = qIBaseTypeLength(desc.array_desc_dtype, desc.array_desc_scale) * desc.array_desc_length;
    QByteArray ba;
    ba.resize(int(buflen));
    isc_array_get_slice(status, &ibase, &trans, arr, &desc, ba.data(), &buflen);
    if (isError(QLatin1String("Could not get array data at position ") + QString::number(pos),
                QSqlError::StatementError))
        return res;

    qDebug("getting...");
    switch(desc.array_desc_dtype & ~1) {
    case SQL_VARYING:
    case SQL_TEXT:
        break; // TODO
    case SQL_LONG:
        return toList<long>(ba.data(), desc.array_desc_length);
    case SQL_SHORT:
        return toList<short>(ba.data(), desc.array_desc_length);
    case SQL_INT64:
        return toList<Q_LONGLONG>(ba.data(), desc.array_desc_length);
    case SQL_FLOAT:
        return toList<float>(ba.data(), desc.array_desc_length);
    case SQL_DOUBLE:
        return toList<double>(ba.data(), desc.array_desc_length);
    case SQL_TIMESTAMP:
        break; //TODO
    case SQL_TYPE_TIME:
        break; //TODO
    }

    return res;
}

template<typename T>
static void fillList(QByteArray &arr, const QList<QCoreVariant> &list, QCoreVariant::Type typ)
{
    for (int i = 0; i < list.length(); ++i) {
        T val(QVariant_to<T>(list.at(i)));
    }
}

void QIBaseResultPrivate::writeArray(int i, const QList<QCoreVariant> &list)
{ // TODO: NULL values, size mismatch
    ISC_QUAD *bId = (ISC_QUAD*)inda->sqlvar[i].sqldata;
//    *(ISC_QUAD*)inda->sqlvar[i].sqldata = 0;

    QByteArray relname(inda->sqlvar[i].relname, inda->sqlvar[i].relname_length);
    QByteArray sqlname(inda->sqlvar[i].sqlname, inda->sqlvar[i].sqlname_length);

    ISC_ARRAY_DESC desc;

    isc_array_lookup_bounds(status, &ibase, &trans, relname.data(), sqlname.data(), &desc);
    if (isError(QLatin1String("Could not find array at position ") + QString::number(i),
                QSqlError::StatementError))
        return;

    qDebug() << bId << relname << "." << sqlname;

    long buflen = qIBaseTypeLength(desc.array_desc_dtype, desc.array_desc_scale) * desc.array_desc_length;
    QByteArray ba;
    ba.resize(int(buflen));

    isc_array_put_slice(status, &ibase, &trans, bId, &desc, ba.data(), &buflen);
}


bool QIBaseResultPrivate::isSelect()
{
    char acBuffer[9];
    char qType = isc_info_sql_stmt_type;
    isc_dsql_sql_info(status, &stmt, 1, &qType, sizeof(acBuffer), acBuffer);
    if (isError(QLatin1String("Could not get query info"), QSqlError::StatementError))
        return false;
    int iLength = isc_vax_integer(&acBuffer[1], 2);
    queryType = isc_vax_integer(&acBuffer[3], iLength);
    return (queryType == isc_info_sql_stmt_select);
}

bool QIBaseResultPrivate::transaction()
{
    if (trans)
        return true;
    if (db->d->trans) {
        localTransaction = false;
        trans = db->d->trans;
        return true;
    }
    localTransaction = true;

    isc_start_transaction(status, &trans, 1, &ibase, 0, NULL);
    if (isError(QLatin1String("Could not start transaction"), QSqlError::StatementError))
        return false;

    return true;
}

// does nothing if the transaction is on the
// driver level
bool QIBaseResultPrivate::commit()
{
    if (!trans)
        return false;
    // don't commit driver's transaction, the driver will do it for us
    if (!localTransaction)
        return true;

    isc_commit_transaction(status, &trans);
    trans = 0;
    return !isError(QLatin1String("Unable to commit transaction"), QSqlError::StatementError);
}

//////////

QIBaseResult::QIBaseResult(const QIBaseDriver* db):
    QSqlCachedResult(db)
{
    d = new QIBaseResultPrivate(this, db);
}

QIBaseResult::~QIBaseResult()
{
    delete d;
}

bool QIBaseResult::prepare(const QString& query)
{
    //qDebug("prepare: %s", query.ascii());
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;
    d->cleanup();
    setActive(false);
    setAt(QSql::BeforeFirst);

    createDA(d->sqlda);
    createDA(d->inda);

    if (!d->transaction())
        return false;

    isc_dsql_allocate_statement(d->status, &d->ibase, &d->stmt);
    if (d->isError(QLatin1String("Could not allocate statement"), QSqlError::StatementError))
        return false;
    isc_dsql_prepare(d->status, &d->trans, &d->stmt, 0, const_cast<char*>(query.utf8()), 3, d->sqlda);
    if (d->isError(QLatin1String("Could not prepare statement"), QSqlError::StatementError))
        return false;

    isc_dsql_describe_bind(d->status, &d->stmt, 1, d->inda);
    if (d->isError(QLatin1String("Could not describe input statement"), QSqlError::StatementError))
        return false;
    if (d->inda->sqld > d->inda->sqln) {
        enlargeDA(d->inda, d->inda->sqld);

        isc_dsql_describe_bind(d->status, &d->stmt, 1, d->inda);
        if (d->isError(QLatin1String("Could not describe input statement"), QSqlError::StatementError))
            return false;
    }
    initDA(d->inda);
    if (d->sqlda->sqld > d->sqlda->sqln) {
        // need more field descriptors
        enlargeDA(d->sqlda, d->sqlda->sqld);

        isc_dsql_describe(d->status, &d->stmt, 1, d->sqlda);
        if (d->isError(QLatin1String("Could not describe statement"), QSqlError::StatementError))
            return false;
    }
    initDA(d->sqlda);

    setSelect(d->isSelect());
    if (!isSelect()) {
        free(d->sqlda);
        d->sqlda = 0;
    }

    return true;
}

bool QIBaseResult::exec()
{
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;
    setActive(false);
    setAt(QSql::BeforeFirst);

    if (d->inda) {
        QVector<QCoreVariant>& values = boundValues();
        int i;
        if (values.count() > d->inda->sqld) {
            qWarning("QIBaseResult::exec: Parameter mismatch, expected %d, got %d parameters", d->inda->sqld, values.count());
            return false;
        }
        int para = 0;
        for (i = 0; i < values.count(); ++i) {
            if (!d->inda->sqlvar[para].sqldata)
                // skip unknown datatypes
                continue;
            const QCoreVariant val(values[i]);
            if (d->inda->sqlvar[para].sqltype & 1) {
                if (val.isNull()) {
                    // set null indicator
                    *(d->inda->sqlvar[para].sqlind) = 1;
                    // and set the value to 0, otherwise it would count as empty string.
                    *((short*)d->inda->sqlvar[para].sqldata) = 0;
                    continue;
                }
                // a value of 0 means non-null.
                *(d->inda->sqlvar[para].sqlind) = 0;
            }
            switch(d->inda->sqlvar[para].sqltype & ~1) {
            case SQL_INT64:
                if (d->inda->sqlvar[para].sqlscale < 0)
                    *((Q_LONGLONG*)d->inda->sqlvar[para].sqldata) =
                        Q_LONGLONG(val.toDouble() * pow(10, d->inda->sqlvar[para].sqlscale * -1));
                else
                    *((Q_LONGLONG*)d->inda->sqlvar[para].sqldata) = val.toLongLong();
                break;
            case SQL_LONG:
                *((long*)d->inda->sqlvar[para].sqldata) = (long)val.toLongLong();
                break;
            case SQL_SHORT:
                *((short*)d->inda->sqlvar[para].sqldata) = (short)val.toInt();
                break;
            case SQL_FLOAT:
                *((float*)d->inda->sqlvar[para].sqldata) = (float)val.toDouble();
                break;
            case SQL_DOUBLE:
                *((double*)d->inda->sqlvar[para].sqldata) = val.toDouble();
                break;
            case SQL_TIMESTAMP:
                *((ISC_TIMESTAMP*)d->inda->sqlvar[para].sqldata) = toTimeStamp(val.toDateTime());
                break;
            case SQL_TYPE_TIME:
                *((ISC_TIME*)d->inda->sqlvar[para].sqldata) = toTime(val.toTime());
                break;
            case SQL_VARYING: {
                QByteArray str(val.toString().utf8()); // keep a copy of the string alive in this scope
                short buflen = d->inda->sqlvar[para].sqllen;
                if (str.length() < buflen)
                    buflen = str.length();
                *(short*)d->inda->sqlvar[para].sqldata = buflen; // first two bytes is the length
                memcpy(d->inda->sqlvar[para].sqldata + sizeof(short), str.data(), buflen);
                break; }
            case SQL_TEXT: {
                QByteArray str(val.toString().toUtf8());
                str = str.leftJustified(d->inda->sqlvar[para].sqllen, ' ', true);
                memcpy(d->inda->sqlvar[para].sqldata, str.data(), d->inda->sqlvar[para].sqllen);
                break; }
        case SQL_BLOB:
                d->writeBlob(para, val.toByteArray());
                break;
        case SQL_ARRAY:
                d->writeArray(para, val.toList());
                break;
        default:
                break;
            }
        }
    }

    if (colCount()) {
        isc_dsql_free_statement(d->status, &d->stmt, DSQL_close);
        if (d->isError(QLatin1String("Unable to close statement")))
            return false;
        cleanup();
    }
    if (d->sqlda)
        init(d->sqlda->sqld);
    isc_dsql_execute2(d->status, &d->trans, &d->stmt, 1, d->inda, 0);
    if (d->isError(QLatin1String("Unable to execute query")))
        return false;

    setActive(true);
    return true;
}

bool QIBaseResult::reset (const QString& query)
{
    qDebug("reset: %s", query.ascii());
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;
    d->cleanup();
    setActive(false);
    setAt(QSql::BeforeFirst);

    createDA(d->sqlda);

    if (!d->transaction())
        return false;

    isc_dsql_allocate_statement(d->status, &d->ibase, &d->stmt);
    if (d->isError(QLatin1String("Could not allocate statement"), QSqlError::StatementError))
        return false;
    isc_dsql_prepare(d->status, &d->trans, &d->stmt, 0, const_cast<char*>(query.utf8()), 3, d->sqlda);
    if (d->isError(QLatin1String("Could not prepare statement"), QSqlError::StatementError))
        return false;

    if (d->sqlda->sqld > d->sqlda->sqln) {
        // need more field descriptors
        int n = d->sqlda->sqld;
        free(d->sqlda);
        d->sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(n));
        d->sqlda->sqln = n;
        d->sqlda->version = SQLDA_VERSION1;

        isc_dsql_describe(d->status, &d->stmt, 1, d->sqlda);
        if (d->isError(QLatin1String("Could not describe statement"), QSqlError::StatementError))
            return false;
    }

    initDA(d->sqlda);

    setSelect(d->isSelect());
    if (isSelect()) {
        init(d->sqlda->sqld);
    } else {
        free(d->sqlda);
        d->sqlda = 0;
    }

    isc_dsql_execute(d->status, &d->trans, &d->stmt, 1, 0);
    if (d->isError(QLatin1String("Unable to execute query")))
        return false;

    // commit non-select queries (if they are local)
    if (!isSelect() && !d->commit())
        return false;

    setActive(true);
    return true;
}

bool QIBaseResult::gotoNext(QSqlCachedResult::ValueCache& row, int rowIdx)
{
    ISC_STATUS stat = isc_dsql_fetch(d->status, &d->stmt, 1, d->sqlda);

    if (stat == 100) {
        // no more rows
        setAt(QSql::AfterLast);
        return false;
    }
    if (d->isError(QLatin1String("Could not fetch next item"), QSqlError::StatementError))
        return false;
    if (rowIdx < 0) // not interested in actual values
        return true;

    for (int i = 0; i < d->sqlda->sqld; ++i) {
        int idx = rowIdx + i;
        char *buf = d->sqlda->sqlvar[i].sqldata;
        int size = d->sqlda->sqlvar[i].sqllen;
        Q_ASSERT(buf);

        if ((d->sqlda->sqlvar[i].sqltype & 1) && *d->sqlda->sqlvar[i].sqlind) {
            // null value
            QCoreVariant v;
            v.cast(qIBaseTypeName2(d->sqlda->sqlvar[i].sqltype));
            row[idx] = v;
            continue;
        }

        qDebug() << "sqld" << d->sqlda->sqld << "type:" << (d->sqlda->sqlvar[i].sqltype & ~1);

        switch(d->sqlda->sqlvar[i].sqltype & ~1) {
        case SQL_VARYING:
            // pascal strings - a short with a length information followed by the data
            row[idx] = QString::fromUtf8(buf + sizeof(short), *(short*)buf);
            break;
        case SQL_INT64:
            if (d->sqlda->sqlvar[i].sqlscale < 0)
                row[idx] = *(Q_LONGLONG*)buf * pow(10, d->sqlda->sqlvar[i].sqlscale);
            else
                row[idx] = QCoreVariant(*(Q_LONGLONG*)buf);
            break;
        case SQL_LONG:
            if (sizeof(int) == sizeof(long)) //dear compiler: please optimize me out.
                row[idx] = QCoreVariant(int((*(long*)buf)));
            else
                row[idx] = QCoreVariant(Q_LONGLONG((*(long*)buf)));
            break;
        case SQL_SHORT:
            row[idx] = QCoreVariant(int((*(short*)buf)));
            break;
        case SQL_FLOAT:
            row[idx] = QCoreVariant(double((*(float*)buf)));
            break;
        case SQL_DOUBLE:
            row[idx] = QCoreVariant(*(double*)buf);
            break;
        case SQL_TIMESTAMP: {
            // have to demangle the structure ourselves because isc_decode_timestamp
            // strips the msecs
            static const QDate bd(1858, 11, 17);
            QTime t;
            t = t.addMSecs(int(((ISC_TIMESTAMP*)buf)->timestamp_time / 10));
            QDate d = bd.addDays(int(((ISC_TIMESTAMP*)buf)->timestamp_date));
            row[idx] = QDateTime(d, t);
        }
        break;
        case SQL_TYPE_TIME: {
            // have to demangle the structure ourselves because isc_decode_time
            // strips the msecs
            QTime t;
            t = t.addMSecs(int((*(ISC_TIME*)buf) / 10));
            row[idx] = t;
        }
        break;
        case SQL_TEXT:
            row[idx] = QString::fromUtf8(buf, size);
            break;
        case SQL_BLOB:
            row[idx] = d->fetchBlob((ISC_QUAD*)buf);
            break;
        case SQL_ARRAY:
            qDebug("SQL_ARRAY");
            row[idx] = d->fetchArray(i, (ISC_QUAD*)buf);
            break;
        default:
            // unknown type - don't even try to fetch
            row[idx] = QCoreVariant();
            break;
        }
    }

    return true;
}

int QIBaseResult::size()
{
    static char sizeInfo[] = {isc_info_sql_records};
    char buf[33];

    if (!isActive() || !isSelect())
        return -1;

    isc_dsql_sql_info(d->status, &d->stmt, sizeof(sizeInfo), sizeInfo, sizeof(buf), buf);
    for (char* c = buf + 3; *c != isc_info_end; /*nothing*/) {
        char ct = *c++;
        short len = isc_vax_integer(c, 2);
        c += 2;
        int val = isc_vax_integer(c, len);
        c += len;
        if (ct == isc_info_req_select_count)
            return val;
    }
    return -1;
}

int QIBaseResult::numRowsAffected()
{
    static char acCountInfo[] = {isc_info_sql_records};
    char cCountType;

    switch (d->queryType) {
    case isc_info_sql_stmt_select:
        cCountType = isc_info_req_select_count;
        break;
    case isc_info_sql_stmt_update:
        cCountType = isc_info_req_update_count;
        break;
    case isc_info_sql_stmt_delete:
        cCountType = isc_info_req_delete_count;
        break;
    case isc_info_sql_stmt_insert:
        cCountType = isc_info_req_insert_count;
        break;
    }

    char acBuffer[33];
    int iResult = -1;
    isc_dsql_sql_info(d->status, &d->stmt, sizeof(acCountInfo), acCountInfo, sizeof(acBuffer), acBuffer);
    if (d->isError(QLatin1String("Could not get statement info"), QSqlError::StatementError))
        return -1;
    for (char *pcBuf = acBuffer + 3; *pcBuf != isc_info_end; /*nothing*/) {
        char cType = *pcBuf++;
        short sLength = isc_vax_integer (pcBuf, 2);
        pcBuf += 2;
        int iValue = isc_vax_integer (pcBuf, sLength);
        pcBuf += sLength;

        if (cType == cCountType) {
            iResult = iValue;
            break;
        }
    }
    return iResult;
}

QSqlRecord QIBaseResult::record() const
{
    QSqlRecord rec;
    if (!isActive() || !d->sqlda)
        return rec;

    XSQLVAR v;
    for (int i = 0; i < d->sqlda->sqld; ++i) {
        v = d->sqlda->sqlvar[i];
        QSqlField f(QString::fromLatin1(v.sqlname, v.sqlname_length).simplified(),
                    qIBaseTypeName2(d->sqlda->sqlvar[i].sqltype));
        f.setLength(v.sqllen);
        f.setPrecision(v.sqlscale);
        f.setSqlType(v.sqltype);
        rec.append(f);
    }
    return rec;
}

/*********************************/

QIBaseDriver::QIBaseDriver(QObject * parent)
    : QSqlDriver(parent)
{
    d = new QIBaseDriverPrivate(this);
}

QIBaseDriver::QIBaseDriver(void *connection, QObject *parent)
    : QSqlDriver(parent)
{
    d = new QIBaseDriverPrivate(this);
    d->ibase = (isc_db_handle)connection;
    setOpen(true);
    setOpenError(false);
}

QIBaseDriver::~QIBaseDriver()
{
    delete d;
}

bool QIBaseDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
    case QuerySize:
    case PreparedQueries:
    case PositionalPlaceholders:
    case Unicode:
    case BLOB:
        return true;
    default:
        return false;
    }
}

bool QIBaseDriver::open(const QString & db,
          const QString & user,
          const QString & password,
          const QString & host,
          int /*port*/,
          const QString & /* connOpts */)
{
    if (isOpen())
        close();

    static const char enc[8] = "UTF_FSS";
    QByteArray usr = user.toLocal8Bit();
    QByteArray pass = password.toLocal8Bit();
    usr.truncate(255);
    pass.truncate(255);

    QByteArray ba;
    ba.resize(usr.length() + pass.length() + sizeof(enc) + 6);
    int i = -1;
    ba[++i] = isc_dpb_version1;
    ba[++i] = isc_dpb_user_name;
    ba[++i] = usr.length();
    memcpy(ba.data() + ++i, usr.data(), usr.length());
    i += usr.length();
    ba[i] = isc_dpb_password;
    ba[++i] = pass.length();
    memcpy(ba.data() + ++i, pass.data(), pass.length());
    i += pass.length();
    ba[i] = isc_dpb_lc_ctype;
    ba[++i] = sizeof(enc) - 1;
    memcpy(ba.data() + ++i, enc, sizeof(enc) - 1);
    i += sizeof(enc) - 1;

    QString ldb;
    if (!host.isEmpty())
        ldb += host + QLatin1Char(':');
    ldb += db;
    isc_attach_database(d->status, 0, (char*)ldb.latin1(), &d->ibase, i, ba.data());
    if (d->isError(QLatin1String("Error opening database"), QSqlError::ConnectionError)) {
        setOpenError(true);
        return false;
    }

    setOpen(true);
    return true;
}

void QIBaseDriver::close()
{
    if (isOpen()) {
        isc_detach_database(d->status, &d->ibase);
        d->ibase = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *QIBaseDriver::createResult() const
{
    return new QIBaseResult(this);
}

bool QIBaseDriver::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    if (d->trans)
        return false;

    isc_start_transaction(d->status, &d->trans, 1, &d->ibase, 0, NULL);
    return !d->isError(QLatin1String("Could not start transaction"), QSqlError::TransactionError);
}

bool QIBaseDriver::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    if (!d->trans)
        return false;

    isc_commit_transaction(d->status, &d->trans);
    d->trans = 0;
    return !d->isError(QLatin1String("Unable to commit transaction"), QSqlError::TransactionError);
}

bool QIBaseDriver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return false;
    if (!d->trans)
        return false;

    isc_rollback_transaction(d->status, &d->trans);
    d->trans = 0;
    return !d->isError(QLatin1String("Unable to rollback transaction"), QSqlError::TransactionError);
}

QStringList QIBaseDriver::tables(QSql::TableType type) const
{
    QStringList res;
    if (!isOpen())
        return res;

    QString typeFilter;

    if (type == QSql::SystemTables) {
        typeFilter += QLatin1String("RDB$SYSTEM_FLAG != 0");
    } else if (type == (QSql::SystemTables | QSql::Views)) {
        typeFilter += QLatin1String("RDB$SYSTEM_FLAG != 0 OR RDB$VIEW_BLR NOT NULL");
    } else {
        if (!(type & QSql::SystemTables))
            typeFilter += QLatin1String("RDB$SYSTEM_FLAG = 0 AND ");
        if (!(type & QSql::Views))
            typeFilter += QLatin1String("RDB$VIEW_BLR IS NULL AND ");
        if (!(type & QSql::Tables))
            typeFilter += QLatin1String("RDB$VIEW_BLR IS NOT NULL AND ");
        if (!typeFilter.isEmpty())
            typeFilter.chop(5);
    }
    if (!typeFilter.isEmpty())
        typeFilter.prepend(QLatin1String("where "));

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    if (!q.exec(QLatin1String("select rdb$relation_name from rdb$relations ") + typeFilter))
        return res;
    while(q.next())
            res << q.value(0).toString().simplified();

    return res;
}

QSqlRecord QIBaseDriver::record(const QString& tablename) const
{
    QSqlRecord rec;
    if (!isOpen())
        return rec;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);

    q.exec(QLatin1String("SELECT a.RDB$FIELD_NAME, b.RDB$FIELD_TYPE, b.RDB$FIELD_LENGTH, "
           "b.RDB$FIELD_SCALE, b.RDB$FIELD_PRECISION, a.RDB$NULL_FLAG "
           "FROM RDB$RELATION_FIELDS a, RDB$FIELDS b "
           "WHERE b.RDB$FIELD_NAME = a.RDB$FIELD_SOURCE "
           "AND a.RDB$RELATION_NAME = '") + tablename.toUpper() + QLatin1String("' "
           "ORDER BY a.RDB$FIELD_POSITION"));

    while (q.next()) {
        int type = q.value(1).toInt();
        QSqlField f(q.value(0).toString().simplified(), qIBaseTypeName(type));
        f.setLength(q.value(5).toInt());
        f.setPrecision(q.value(2).toInt());
        f.setRequired(q.value(4).toInt() > 0 ? true : false);
        f.setSqlType(type);

        rec.append(f);
    }

    return rec;
}

QSqlIndex QIBaseDriver::primaryIndex(const QString &table) const
{
    QSqlIndex index(table);
    if (!isOpen())
        return index;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    q.exec(QLatin1String("SELECT a.RDB$INDEX_NAME, b.RDB$FIELD_NAME, d.RDB$FIELD_TYPE "
           "FROM RDB$RELATION_CONSTRAINTS a, RDB$INDEX_SEGMENTS b, RDB$RELATION_FIELDS c, RDB$FIELDS d "
           "WHERE a.RDB$CONSTRAINT_TYPE = 'PRIMARY KEY' "
           "AND a.RDB$RELATION_NAME = '") + table.toUpper() +
           QLatin1String(" 'AND a.RDB$INDEX_NAME = b.RDB$INDEX_NAME "
           "AND c.RDB$RELATION_NAME = a.RDB$RELATION_NAME "
           "AND c.RDB$FIELD_NAME = b.RDB$FIELD_NAME "
           "AND d.RDB$FIELD_NAME = c.RDB$FIELD_SOURCE "
           "ORDER BY b.RDB$FIELD_POSITION"));

    while (q.next()) {
        QSqlField field(q.value(1).toString().simplified(), qIBaseTypeName(q.value(2).toInt()));
        index.append(field); //TODO: asc? desc?
        index.setName(q.value(0).toString());
    }

    return index;
}

QString QIBaseDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
    switch (field.type()) {
    case QCoreVariant::DateTime: {
        QDateTime datetime = field.value().toDateTime();
        if (datetime.isValid())
            return QLatin1Char('\'') + QString::number(datetime.date().year()) + QLatin1Char('-') +
                QString::number(datetime.date().month()) + QLatin1Char('-') +
                QString::number(datetime.date().day()) + QLatin1Char(' ') +
                QString::number(datetime.time().hour()) + QLatin1Char(':') +
                QString::number(datetime.time().minute()) + QLatin1Char(':') +
                QString::number(datetime.time().second()) + QLatin1Char('.') +
                QString::number(datetime.time().msec()).rightJustified(3, QLatin1Char('0'), true) +
		QLatin1Char('\'');
        else
            return QLatin1String("NULL");
    }
    case QCoreVariant::Time: {
        QTime time = field.value().toTime();
        if (time.isValid())
            return QLatin1Char('\'') + QString::number(time.hour()) + QLatin1Char(':') +
                QString::number(time.minute()) + QLatin1Char(':') +
                QString::number(time.second()) + QLatin1Char('.') +
                QString::number(time.msec()).rightJustified(3, QLatin1Char('0'), true) +
                QLatin1Char('\'');
        else
            return QLatin1String("NULL");
    }
    case QCoreVariant::Date: {
        QDate date = field.value().toDate();
        if (date.isValid())
            return QLatin1Char('\'') + QString::number(date.year()) + QLatin1Char('-') +
                QString::number(date.month()) + QLatin1Char('-') +
                QString::number(date.day()) + QLatin1Char('\'');
            else
                return QLatin1String("NULL");
    }
    default:
        return QSqlDriver::formatValue(field, trimStrings);
    }
}
