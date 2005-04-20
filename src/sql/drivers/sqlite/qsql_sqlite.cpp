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

#include "qsql_sqlite.h"

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqlquery.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qdebug.h>

#if defined Q_OS_WIN
# include <qt_windows.h>
#else
# include <unistd.h>
#endif

#include <sqlite3.h>

Q_DECLARE_METATYPE(sqlite3*)
Q_DECLARE_METATYPE(sqlite3_stmt*)

static QVariant::Type qSqliteType(int tp)
{
    switch (tp) {
    case SQLITE_INTEGER:
        return QVariant::Int;
    case SQLITE_FLOAT:
        return QVariant::Double;
    case SQLITE_BLOB:
        return QVariant::ByteArray;
    case SQLITE_TEXT:
    default:
        return QVariant::String;
    }
}

static QSqlError qMakeError(sqlite3 *access, const QString &descr, QSqlError::ErrorType type,
                            int errorCode = -1)
{
    return QSqlError(descr,
                     QString::fromUtf16(static_cast<const ushort *>(sqlite3_errmsg16(access))),
                     type, errorCode);
}

class QSQLiteDriverPrivate
{
public:
    inline QSQLiteDriverPrivate() : access(0) {}
    sqlite3 *access;
};


class QSQLiteResultPrivate
{
public:
    QSQLiteResultPrivate(QSQLiteResult *res);
    void cleanup();
    bool fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch);
    bool isSelect();
    // initializes the recordInfo and the cache
    void initColumns();
    void finalize();

    QSQLiteResult* q;
    sqlite3 *access;

    sqlite3_stmt *stmt;

    uint skippedStatus: 1; // the status of the fetchNext() that's skipped
    uint skipRow: 1; // skip the next fetchNext()?
    uint utf8: 1;
    QSqlRecord rInf;
};

static const uint initial_cache_size = 128;

QSQLiteResultPrivate::QSQLiteResultPrivate(QSQLiteResult* res) : q(res), access(0),
    stmt(0), skippedStatus(false), skipRow(false), utf8(false)
{
}

void QSQLiteResultPrivate::cleanup()
{
    finalize();
    rInf.clear();
    skippedStatus = false;
    skipRow = false;
    q->setAt(QSql::BeforeFirstRow);
    q->setActive(false);
    q->cleanup();
}

void QSQLiteResultPrivate::finalize()
{
    if (!stmt)
        return;

    sqlite3_finalize(stmt);
    stmt = 0;
}

void QSQLiteResultPrivate::initColumns()
{
    rInf.clear();

    int nCols = sqlite3_column_count(stmt);
    if (nCols <= 0)
        return;

    q->init(nCols);

    for (int i = 0; i < nCols; ++i) {
        QString colName = QString::fromUtf16(
                    static_cast<const ushort *>(sqlite3_column_name16(stmt, i)));

        int dotIdx = colName.lastIndexOf(QLatin1Char('.'));
        int stp = sqlite3_column_type(stmt, i);
        QSqlField fld(colName.mid(dotIdx == -1 ? 0 : dotIdx + 1), qSqliteType(stp));
        fld.setSqlType(stp);
        rInf.append(fld);
    }
}

bool QSQLiteResultPrivate::fetchNext(QSqlCachedResult::ValueCache &values, int idx, bool initialFetch)
{
    int res;
    int i;

    if (skipRow) {
        // already fetched
        Q_ASSERT(!initialFetch);
        skipRow = false;
        return skippedStatus;
    }
    skipRow = initialFetch;

    if (!stmt)
        return false;

    res = sqlite3_step(stmt);

    switch(res) {
    case SQLITE_ROW:
        // check to see if should fill out columns
        if (rInf.isEmpty())
            // must be first call.
            initColumns();
        if (idx < 0 && !initialFetch)
            return true;
        for (i = 0; i < rInf.count(); ++i)
            switch (rInf.field(i).typeID()) {
            case SQLITE_BLOB:
                values[i + idx] = QByteArray(static_cast<const char *>(
                            sqlite3_column_blob(stmt, i)),
                            sqlite3_column_bytes(stmt, i));
                break;
            case SQLITE_INTEGER:
                values[i + idx] = sqlite3_column_int64(stmt, i);
                break;
            case SQLITE_FLOAT:
                values[i + idx] = sqlite3_column_double(stmt, i);
                break;
            case SQLITE_NULL:
                values[i + idx] = QVariant(QVariant::String);
                break;
            default:
                values[i + idx] = QString::fromUtf16(static_cast<const ushort *>(
                            sqlite3_column_text16(stmt, i)),
                            sqlite3_column_bytes16(stmt, i) / sizeof(ushort));
                break;
        }
        return true;
    case SQLITE_DONE:
        if (rInf.isEmpty())
            // must be first call.
            initColumns();
        q->setAt(QSql::AfterLastRow);
        return false;
    case SQLITE_ERROR:
    case SQLITE_MISUSE:
    case SQLITE_BUSY:
    default:
        // something wrong, don't get col info, but still return false
        q->setLastError(qMakeError(access, QCoreApplication::translate("QSQLiteResult",
                        "Unable to fetch row"), QSqlError::ConnectionError, res));
        finalize();
        q->setAt(QSql::AfterLastRow);
        return false;
    }
    return false;
}

QSQLiteResult::QSQLiteResult(const QSQLiteDriver* db)
    : QSqlCachedResult(db)
{
    d = new QSQLiteResultPrivate(this);
    d->access = db->d->access;
}

QSQLiteResult::~QSQLiteResult()
{
    d->cleanup();
    delete d;
}

bool QSQLiteResult::reset(const QString &query)
{
    if (!prepare(query))
        return false;
    return exec();
}

bool QSQLiteResult::prepare(const QString &query)
{
    if (!driver() || !driver()->isOpen() || driver()->isOpenError())
        return false;

    d->cleanup();

    setSelect(false);

    int res = sqlite3_prepare16(d->access, query.constData(), (query.size() + 1) * sizeof(QChar),
                                &d->stmt, 0);

    if (res != SQLITE_OK) {
        setLastError(qMakeError(d->access, QCoreApplication::translate("QSQLiteResult",
                     "Unable to execute statement"), QSqlError::StatementError, res));
        d->finalize();
        return false;
    }
    return true;
}

bool QSQLiteResult::exec()
{
    const QVector<QVariant> values = boundValues();

    d->skippedStatus = false;
    d->skipRow = false;
    clearValues();

    sqlite3_reset(d->stmt);

    int paramCount = sqlite3_bind_parameter_count(d->stmt);
    if (paramCount == values.count()) {
        for (int i = 0; i < paramCount; ++i) {
            int res = SQLITE_OK;
            const QVariant value = values.at(i);

            if (value.isNull()) {
                res = sqlite3_bind_null(d->stmt, i + 1);
            } else {
                switch (value.type()) {
                case QVariant::ByteArray: {
                    const QByteArray *ba = static_cast<const QByteArray*>(value.constData());
                    res = sqlite3_bind_blob(d->stmt, i + 1, ba->constData(),
                                            ba->size(), SQLITE_STATIC);
                    break; }
                case QVariant::Int:
                    res = sqlite3_bind_int(d->stmt, i + 1, value.toInt());
                    break;
                case QVariant::Double:
                    res = sqlite3_bind_double(d->stmt, i + 1, value.toDouble());
                    break;
                case QVariant::LongLong:
                    res = sqlite3_bind_int64(d->stmt, i + 1, value.toLongLong());
                    break;
                case QVariant::String: {
                    // lifetime of string == lifetime of its qvariant
                    const QString *str = static_cast<const QString*>(value.constData());
                    res = sqlite3_bind_text16(d->stmt, i + 1, str->utf16(),
                                              (str->size()) * sizeof(QChar), SQLITE_STATIC);
                    break; }
                default: {
                    QString str = value.toString();
                    // SQLITE_TRANSIENT makes sure that sqlite buffers the data
                    res = sqlite3_bind_text16(d->stmt, i + 1, str.utf16(),
                                              (str.size()) * sizeof(QChar), SQLITE_TRANSIENT);
                    break; }
                }
            }
            if (res != SQLITE_OK) {
                setLastError(qMakeError(d->access, QCoreApplication::translate("QSQLiteResult",
                             "Unable to bind parameters"), QSqlError::StatementError, res));
                d->finalize();
                return false;
            }
        }
    }
    d->skippedStatus = d->fetchNext(cache(), 0, true);
    setSelect(!d->rInf.isEmpty());
    setActive(true);
    return true;
}

bool QSQLiteResult::gotoNext(QSqlCachedResult::ValueCache& row, int idx)
{
    return d->fetchNext(row, idx, false);
}

int QSQLiteResult::size()
{
    return -1;
}

int QSQLiteResult::numRowsAffected()
{
    return sqlite3_changes(d->access);
}

QVariant QSQLiteResult::lastInsertId() const
{
    if (isActive()) {
        qint64 id = sqlite3_last_insert_rowid(d->access);
        if (id)
            return id;
    }
    return QVariant();
}

QSqlRecord QSQLiteResult::record() const
{
    if (!isActive() || !isSelect())
        return QSqlRecord();
    return d->rInf;
}

QVariant QSQLiteResult::handle() const
{
    return qVariantFromValue(d->stmt);
}

/////////////////////////////////////////////////////////

QSQLiteDriver::QSQLiteDriver(QObject * parent)
    : QSqlDriver(parent)
{
    d = new QSQLiteDriverPrivate();
}

QSQLiteDriver::QSQLiteDriver(sqlite3 *connection, QObject *parent)
    : QSqlDriver(parent)
{
    d = new QSQLiteDriverPrivate();
    d->access = connection;
    setOpen(true);
    setOpenError(false);
}


QSQLiteDriver::~QSQLiteDriver()
{
    delete d;
}

bool QSQLiteDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case BLOB:
    case Transactions:
    case Unicode:
    case LastInsertId:
    case PreparedQueries:
    case PositionalPlaceholders:
        return true;
    case QuerySize:
    case NamedPlaceholders:
        return false;
    }
    return false;
}

static int qGetSqliteTimeout(QString opts)
{
    enum { DefaultTimeout = 5000 };

    opts.remove(QLatin1Char(' '));
    if (opts.startsWith(QLatin1String("QSQLITE_BUSY_TIMEOUT="))) {
        bool ok;
        int nt = opts.mid(21).toInt(&ok);
        if (ok)
            return nt;
    }
    return DefaultTimeout;
}

/*
   SQLite dbs have no user name, passwords, hosts or ports.
   just file names.
*/
bool QSQLiteDriver::open(const QString & db, const QString &, const QString &, const QString &, int, const QString &conOpts)
{
    if (isOpen())
        close();

    if (db.isEmpty())
        return false;

    if (sqlite3_open16(db.constData(), &d->access) == SQLITE_OK) {
        sqlite3_busy_timeout(d->access, qGetSqliteTimeout(conOpts));
        setOpen(true);
        setOpenError(false);
        return true;
    } else {
        setLastError(qMakeError(d->access, tr("Error opening database"),
                     QSqlError::ConnectionError));
        setOpenError(true);
        return false;
    }
}

void QSQLiteDriver::close()
{
    if (isOpen()) {
        if (sqlite3_close(d->access) != SQLITE_OK)
            setLastError(qMakeError(d->access, tr("Error closing database"),
                                    QSqlError::ConnectionError));
        d->access = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *QSQLiteDriver::createResult() const
{
    return new QSQLiteResult(this);
}

bool QSQLiteDriver::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createResult());
    if (!q.exec(QLatin1String("BEGIN"))) {
        setLastError(QSqlError(tr("Unable to begin transaction"),
                               q.lastError().databaseText(), QSqlError::TransactionError));
        return false;
    }

    return true;
}

bool QSQLiteDriver::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createResult());
    if (!q.exec(QLatin1String("COMMIT"))) {
        setLastError(QSqlError(tr("Unable to commit transaction"),
                               q.lastError().databaseText(), QSqlError::TransactionError));
        return false;
    }

    return true;
}

bool QSQLiteDriver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return false;

    QSqlQuery q(createResult());
    if (!q.exec(QLatin1String("ROLLBACK"))) {
        setLastError(QSqlError(tr("Unable to roll back transaction"),
                               q.lastError().databaseText(), QSqlError::TransactionError));
        return false;
    }

    return true;
}

QStringList QSQLiteDriver::tables(QSql::TableType type) const
{
    QStringList res;
    if (!isOpen())
        return res;

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    if ((type & QSql::Tables) && (type & QSql::Views))
        q.exec(QLatin1String("SELECT name FROM sqlite_master WHERE type='table' OR type='view'"));
    else if (type & QSql::Tables)
        q.exec(QLatin1String("SELECT name FROM sqlite_master WHERE type='table'"));
    else if (type & QSql::Views)
        q.exec(QLatin1String("SELECT name FROM sqlite_master WHERE type='view'"));

    if (q.isActive()) {
        while(q.next())
            res.append(q.value(0).toString());
    }

    if (type & QSql::SystemTables) {
        // there are no internal tables beside this one:
        res.append(QLatin1String("sqlite_master"));
    }

    return res;
}

static QSqlIndex qGetTableInfo(QSqlQuery &q, const QString &tableName, bool onlyPIndex = false)
{
    QSqlIndex ind;
    q.exec(QLatin1String("PRAGMA table_info ('") + tableName + QLatin1String("')"));
    while (q.next()) {
        bool isPk = q.value(5).toInt();
        if (onlyPIndex && !isPk)
            continue;
        QSqlField fld(q.value(1).toString(), QVariant::String);
        if (q.value(2).toString().toLower() == QLatin1String("integer")) {
            // integer primary key fields are auto-generated in sqlite
            fld.setType(QVariant::Int);
            fld.setAutoGenerated(isPk);
        }
        fld.setRequired(q.value(3).toInt() != 0);
        fld.setDefaultValue(q.value(4));
        ind.append(fld);
    }
    return ind;
}

QSqlIndex QSQLiteDriver::primaryIndex(const QString &tblname) const
{
    if (!isOpen())
        return QSqlIndex();

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    return qGetTableInfo(q, tblname, true);
}

QSqlRecord QSQLiteDriver::record(const QString &tbl) const
{
    if (!isOpen())
        return QSqlRecord();

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    return qGetTableInfo(q, tbl);
}

QVariant QSQLiteDriver::handle() const
{
    return qVariantFromValue(d->access);
}

