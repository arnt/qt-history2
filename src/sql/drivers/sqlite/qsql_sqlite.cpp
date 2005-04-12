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
        rInf.append(QSqlField(colName.mid(dotIdx == -1 ? 0 : dotIdx + 1),
                qSqliteType(sqlite3_column_type(stmt, i))));
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

    // keep trying while busy, wish I could implement this better.
    while ((res = sqlite3_step(stmt)) == SQLITE_BUSY) {
        // sleep instead requesting result again immidiately.
#if defined Q_OS_WIN
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    switch(res) {
    case SQLITE_ROW:
        // check to see if should fill out columns
        if (rInf.isEmpty())
            // must be first call.
            initColumns();
        if (idx < 0 && !initialFetch)
            return true;
        for (i = 0; i < rInf.count(); ++i)
            // todo - handle other types
            switch (rInf.field(i).type()) {
            case QVariant::ByteArray:
                values[i + idx] = QByteArray(static_cast<const char *>(
                            sqlite3_column_blob(stmt, i)),
                            sqlite3_column_bytes(stmt, i));
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

/*
   Execute \a query.
*/
bool QSQLiteResult::reset (const QString &query)
{
    // this is where we build a query.
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
        return true;
    case QuerySize:
    case PreparedQueries:
    case NamedPlaceholders:
    case PositionalPlaceholders:
        return false;
    }
    return false;
}

/*
   SQLite dbs have no user name, passwords, hosts or ports.
   just file names.
*/
bool QSQLiteDriver::open(const QString & db, const QString &, const QString &, const QString &, int, const QString &)
{
    if (isOpen())
        close();

    if (db.isEmpty())
        return false;

    if (sqlite3_open16(db.constData(), &d->access) == SQLITE_OK) {
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

QSqlIndex QSQLiteDriver::primaryIndex(const QString &tblname) const
{
    QSqlRecord rec(record(tblname)); // expensive :(

    if (!isOpen())
        return QSqlIndex();

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    // first find a UNIQUE INDEX
    q.exec(QLatin1String("PRAGMA index_list('") + tblname + QLatin1String("');"));
    QString indexname;
    while(q.next()) {
        if (q.value(2).toInt()==1) {
            indexname = q.value(1).toString();
            break;
        }
    }
    if (indexname.isEmpty())
        return QSqlIndex();

    q.exec(QLatin1String("PRAGMA index_info('") + indexname + QLatin1String("');"));

    QSqlIndex index(indexname);
    while(q.next()) {
        QSqlField fld = rec.field(q.value(2).toString());
        if (fld.isValid())
            index.append(fld);
    }
    return index;
}

QSqlRecord QSQLiteDriver::record(const QString &tbl) const
{
    if (!isOpen())
        return QSqlRecord();

    QSqlQuery q(createResult());
    q.setForwardOnly(true);
    q.exec(QLatin1String("SELECT * FROM ") + tbl + QLatin1String(" LIMIT 1"));
    return q.record();
}

QVariant QSQLiteDriver::handle() const
{
    return qVariantFromValue(d->access);
}

