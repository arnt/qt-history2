/****************************************************************************
**
** Implementation of SQLite driver classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_sqlite.h"

#include <qdatetime.h>
#include <qmap.h>
#include <qregexp.h>

#if (QT_VERSION-0 < 0x030000)
#include <qlist.h>
#include <qvector.h>
#include <unistd.h>
#include "../../../3rdparty/libraries/sqlite/sqlite.h"
#else
#include <qptrlist.h>
#include <qptrvector.h>
#include <unistd.h>
#include <sqlite.h>
#endif

typedef struct sqlite_vm sqlite_vm;

#define QSQLite_DRIVER_NAME "QSQLITE1"

static QSqlVariant::Type nameToType(const QString& typeName)
{
    QString tName = typeName.upper();
    if (tName.startsWith("INT"))
        return QSqlVariant::Int;
    if (tName.startsWith("FLOAT") || tName.startsWith("NUMERIC"))
        return QSqlVariant::Double;
    if (tName.startsWith("BOOL"))
        return QSqlVariant::Bool;
    // SQLite is typeless - consider everything else as string
    return QSqlVariant::String;
}

class QSQLiteDriverPrivate
{
public:
    QSQLiteDriverPrivate() : access(0) {}
    sqlite *access;
    bool utf8;
};

class QSQLiteResultPrivate
{
public:
    QSQLiteResultPrivate(QSQLiteResult* res);
    bool fetchNext();
    bool isSelect();
    // initializes the recordInfo and the cache
    void init(const char **cnames, int numCols);
    void finalize();

    QSQLiteResult* p;
    sqlite *access;

    // and we have too keep our own struct for the data (sqlite works via 
    // callback.
    const char *currentTail;
    sqlite_vm *currentMachine;

#if (QT_VERSION-0 < 0x030000)
    typedef QVector<QSqlVariant> RowCache;
    typedef QVector<RowCache> RowsetCache;
#else
    typedef QPtrVector<QSqlVariant> RowCache;
    typedef QPtrVector<RowCache> RowsetCache;
#endif
    RowsetCache rowCache;
    int rowCacheEnd;
    int rowCacheCurrent;
    
    uint skipFetch: 1; // skip the next fetchNext() ??
    uint skippedStatus: 1; // the status of the fetchNext() that's skipped ??
    uint forwardOnly: 1; // if true, then don't expand cache, loop. Otherwise expand cache.
    uint utf8: 1;
    
    QSqlRecordInfo rInf;
};

static const uint initial_cache_size = 128;

QSQLiteResultPrivate::QSQLiteResultPrivate(QSQLiteResult* res) : p(res), access(0), currentTail(0),
    currentMachine(0), rowCacheEnd(0), rowCacheCurrent(-1), skipFetch(FALSE), skippedStatus(FALSE), forwardOnly(FALSE), utf8(FALSE)
{
    rowCache.setAutoDelete(TRUE);
    rowCache.resize(initial_cache_size);
}

/* BEWARE!! Call that only once after reset! */
bool QSQLiteResultPrivate::isSelect()
{
    // XXX should be a bool to protect this.
    skippedStatus = fetchNext();
    skipFetch = TRUE;
    return !rInf.isEmpty();
}

void QSQLiteResultPrivate::finalize()
{
    if (!currentMachine)
        return;

    char* err = 0;
    int res = sqlite_finalize(currentMachine, &err);
    if (err) {
        p->setLastError(QSqlError("Unable to fetch results", err, QSqlError::Statement, res));
        sqlite_freemem(err);
    }
    currentMachine = 0;
}

void QSQLiteResultPrivate::init(const char **cnames, int numCols)
{
    if (!cnames)
        return;

    rInf.clear();
    if (numCols <= 0)
        return;

    for (int i = 0; i < numCols; ++i) {
        const char* lastDot = strrchr(cnames[i], '.');
        const char* fieldName = lastDot ? lastDot + 1 : cnames[i];
        rInf.append(QSqlFieldInfo(fieldName, nameToType(cnames[i+numCols])));
    }
    /* XXX this seems wrong, why add an empty record to the cache?
    if (forwardOnly) {
	RowCache *r = new RowCache(numCols);
	r->setAutoDelete(TRUE);
	if (rowCacheEnd == rowCache.size())
	    rowCache.resize(rowCache.size() << 1);
	rowCache.insert(rowCacheEnd++, r); // why?
    }
    */
}

bool QSQLiteResultPrivate::fetchNext()
{
    // may be caching.
    const char **fvals;
    const char **cnames;
    int colNum;
    int res;
    int i;

    if (skipFetch) {
        // already fetched
        skipFetch = FALSE;
        return skippedStatus;
    }
 
    if (!currentMachine)
        return FALSE;

    // keep trying while busy, wish I could implement this better.
    while ((res = sqlite_step(currentMachine, &colNum, &fvals, &cnames)) == SQLITE_BUSY) {
    }

    switch(res) {
        case SQLITE_ROW:
            // check to see if should fill out columns
            if (rInf.isEmpty())
                // must be first call.
                init(cnames, colNum);
            if (fvals) {
                RowCache *values = new RowCache(colNum);
		values->setAutoDelete(TRUE);
                for (i = 0; i < colNum; ++i) {
                    if (utf8)
                        values->insert(i, new QSqlVariant(QString::fromUtf8(fvals[i])));
                    else
                        values->insert(i, new QSqlVariant(QString(fvals[i])));
                }
                if (forwardOnly) {
		    // e.g. not caching
		    if (rowCacheCurrent != -1)
			rowCache.remove(0);
                    rowCache.insert(0, values);
		    rowCacheCurrent = 0;
		} else {
		    if (rowCacheEnd == rowCache.size())
			rowCache.resize(rowCache.size() << 1);
		    rowCache.insert(rowCacheEnd++, values);
		}
                return TRUE;
            }
            break;
        case SQLITE_DONE:
            if (rInf.isEmpty())
                // must be first call.
                init(cnames, colNum);
            return FALSE;
        case SQLITE_ERROR:
        case SQLITE_MISUSE:
        default:
            // something wrong, don't get col info, but still return false
            finalize(); // finalize to get the error message.
            return FALSE;
    }
    return FALSE;
}

QSQLiteResult::QSQLiteResult(const QSQLiteDriver* db)
: QSqlResult(db)
{
    d = new QSQLiteResultPrivate(this);
    d->access = db->d->access;
    d->utf8 = db->d->utf8;
}

QSQLiteResult::~QSQLiteResult()
{
    cleanup();
    delete d;
}

void QSQLiteResult::cleanup()
{
    d->finalize();
    d->rowCache.clear();
    d->rowCache.resize(initial_cache_size);
    d->rowCacheEnd = 0;
    d->rowCacheCurrent = -1;
    d->rInf.clear();
    d->currentTail = 0;
    d->currentMachine = 0;
    d->skipFetch = FALSE;
    d->skippedStatus = FALSE;
    setAt(QSql::BeforeFirst);
    setActive(FALSE);
}

bool QSQLiteResult::fetch(int i)
{
    if (i == at())
        return TRUE;
    if (i < 0)
        return FALSE;
    if (d->forwardOnly) {
        if (at() > i || at() == QSql::AfterLast)
            return FALSE;
    } else {
        if (i < (int)d->rowCacheEnd) {
            setAt(i);
            return TRUE;
        } else {
            setAt(d->rowCacheEnd - 1);
        }
    }

    // need to move to that result.
    int current = at();
    while(fetchNext()) {
        current++;
        if (current == i)
            break;
    }
    return isValid();
}

bool QSQLiteResult::fetchNext()
{
    if (at() == QSql::AfterLast)
        return FALSE;    
    if (!d->forwardOnly && ((d->rowCacheEnd-1) >=  at() + 1)) {
        setAt(at() + 1);
        return TRUE;
    }
    
    if (!d->fetchNext()) {
        setAt(QSql::AfterLast);
        return FALSE;
    }
    setAt(at() + 1);
    return TRUE;
}

bool QSQLiteResult::fetchLast()
{
    if (!d->forwardOnly && at() == QSql::AfterLast && d->rowCacheEnd > 0) {
        setAt(d->rowCacheEnd - 1);
        return TRUE;
    }
    if (at() >= QSql::BeforeFirst) {
        int i = at();
        while (fetchNext())
            i++; /* brute force */
        if (d->forwardOnly && at() == QSql::AfterLast) {
            setAt(i);
            return TRUE;
        } else {
            return fetch(d->rowCacheEnd - 1);
        }
    }
    return FALSE;
}

bool QSQLiteResult::fetchFirst()
{
    if (at() != QSql::BeforeFirst) {
        if (d->forwardOnly)
            return FALSE;
        setAt(0);
        return TRUE;
    }

    if (d->fetchNext()) {
        setAt(0);
        return TRUE;
    }
        
    return FALSE;
}

QSqlVariant QSQLiteResult::data(int field)
{
    if (!isSelect() || !isValid())
        return QSqlVariant();
   
    if (d->forwardOnly)
        return *(d->rowCache.at(0)->at(field));
    else
        return *(d->rowCache.at(at())->at(field));
}

bool QSQLiteResult::isNull(int field)
{
    if (!isSelect() || !isValid())
        return FALSE;
    
    if (d->forwardOnly)
        return d->rowCache.at(0)->at(field)->isNull();
    else
        return d->rowCache.at(at())->at(field)->isNull();
}

/*
   Execute \a query.
*/
bool QSQLiteResult::reset (const QString& query)
{
    // this is where we build a query.
    if (!driver())
        return FALSE;
    if (!driver()-> isOpen() || driver()->isOpenError())
        return FALSE;

    cleanup();

    // Um, ok.  callback based so.... pass private static function for this.
    setSelect(FALSE);
    d->forwardOnly = isForwardOnly();
    char *err = 0;
    int res = sqlite_compile(d->access, 
                                d->utf8 ? query.utf8() : query.local8Bit(),
                                &(d->currentTail),
                                &(d->currentMachine),
                                &err);
    if (res != SQLITE_OK || err) {
        setLastError(QSqlError("Unable to execute statement", err, QSqlError::Statement, res));
        qDebug("err %s", err);
        sqlite_freemem(err);
    }
    //if (*d->currentTail != '\000' then there is more sql to eval
    if (d->currentMachine != 0) {
        setSelect(d->isSelect());
        setActive(TRUE);
        return TRUE;
    } else {
        setActive(FALSE);
        return FALSE;
    }
}

int QSQLiteResult::size()
{
    return -1;
}

int QSQLiteResult::numRowsAffected()
{
    return sqlite_changes(d->access);
}

/////////////////////////////////////////////////////////

QSQLiteDriver::QSQLiteDriver(QObject * parent, const char * name)
: QSqlDriver(parent, name ? name : QSQLite_DRIVER_NAME)
{
    d = new QSQLiteDriverPrivate();
    d->access = 0;
    d->utf8 = (qstrcmp(sqlite_encoding, "UTF-8") == 0);
}

QSQLiteDriver::~QSQLiteDriver()
{
    delete d;
}

bool QSQLiteDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
        return TRUE;
#if (QT_VERSION-0 >= 0x030000)
    case Unicode:
        return d->utf8;
#endif
//   case BLOB:
    default:
        return FALSE;
    }
}

/*
   SQLite dbs have no user name, passwords, hosts or ports.
   just file names.
*/
bool QSQLiteDriver::open(const QString & db, const QString &, const QString &, const QString &, int, const QString &)
{
    if (isOpen())
        close();

    char* err = 0;
    d->access = sqlite_open(db.latin1(), 0, &err);
    if (err) {
        setLastError(QSqlError("Error to open database", err, QSqlError::Connection));
        sqlite_freemem(err);
        err = 0;
    }

    if (d->access) {
        setOpen(TRUE);
        return TRUE;
    }
    setOpenError(TRUE);
    return FALSE;
}

void QSQLiteDriver::close()
{
    if (isOpen()) {
        sqlite_close(d->access);
        d->access = 0;
        setOpen(FALSE);
        setOpenError(FALSE);
    }
}

QSqlQuery QSQLiteDriver::createQuery() const
{
    return QSqlQuery(new QSQLiteResult(this));
}

bool QSQLiteDriver::beginTransaction()
{
    if (!isOpen() || isOpenError())
        return FALSE;

    char* err;
    int res = sqlite_exec(d->access, "BEGIN", 0, this, &err);

    if (res == SQLITE_OK)
        return TRUE;

    setLastError(QSqlError("Unable to begin transaction", err, QSqlError::Transaction, res));
    sqlite_freemem(err);
    return FALSE;
}

bool QSQLiteDriver::commitTransaction()
{
    if (!isOpen() || isOpenError())
        return FALSE;

    char* err;
    int res = sqlite_exec(d->access, "COMMIT", 0, this, &err);

    if (res == SQLITE_OK)
        return TRUE;

    setLastError(QSqlError("Unable to commit transaction", err, QSqlError::Transaction, res));
    sqlite_freemem(err);
    return FALSE;
}

bool QSQLiteDriver::rollbackTransaction()
{
    if (!isOpen() || isOpenError())
        return FALSE;

    char* err;
    int res = sqlite_exec(d->access, "ROLLBACK", 0, this, &err);

    if (res == SQLITE_OK)
        return TRUE;

    setLastError(QSqlError("Unable to rollback Transaction", err, QSqlError::Transaction, res));
    sqlite_freemem(err);
    return FALSE;
}

QStringList QSQLiteDriver::tables(const QString &typeName) const
{
    QStringList res;
    if (!isOpen())
        return res;
    int type = typeName.toInt();

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
#if (QT_VERSION-0 >= 0x030000)
    if ((type & (int)QSql::Tables) && (type & (int)QSql::Views))
        q.exec("SELECT name FROM sqlite_master WHERE type='table' OR type='view'");
    else if (typeName.isEmpty() || (type & (int)QSql::Tables))
        q.exec("SELECT name FROM sqlite_master WHERE type='table'");
    else if (type & (int)QSql::Views)
        q.exec("SELECT name FROM sqlite_master WHERE type='view'");
#else
        q.exec("SELECT name FROM sqlite_master WHERE type='table' OR type='view'");
#endif


    if (q.isActive()) {
        while(q.next())
            res.append(q.value(0).toString());
    }

#if (QT_VERSION-0 >= 0x030000)
    if (type & (int)QSql::SystemTables) {
        // there are no internal tables beside this one:
        res.append("sqlite_master");
    }
#endif

    return res;
}

QSqlIndex QSQLiteDriver::primaryIndex(const QString &tblname) const
{
    QSqlIndex index(tblname);
    QSqlRecord rec(record(tblname)); // expensive :(

    if (!isOpen())
        return index;

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    q.exec("SELECT sql FROM sqlite_master WHERE name='" + tblname + "'");
    if (!q.next())
        return index;

    QString sqlForTable(q.value(0).toString().simplifyWhiteSpace().upper());
    
    // primary index will have name like "(tblname autoindex 1)"
//    qDebug("working with string %s", sqlForTable.latin1());
    QString sqlName;
    QString sqlType;
    
    //sqlForTable is the sql statment used to create the table.
    // looking for.
    // field1, field2, field3, PRIMARY KEY(field1, field2)    
    // "(name TYPE PRIMARY KEY"
    // "(name PRIMARY KEY"
    // ", name TYPE PRIMARY KEY"
    // ", name PRIMARY KEY"
    int a = sqlForTable.find(",");
    int b = sqlForTable.find("PRIMARY KEY");
    if (b > 0) {
        int c = 0;
        static QChar popen('(');
        if (sqlForTable.at(b + 12) == popen)
            c = b + 13;
        else if (sqlForTable.at(b + 13) == popen)
            c = b + 14;
        int endP = sqlForTable.find(")", c);
        if (c && endP) {
            QString idxStr = sqlForTable.mid(c, endP - c).stripWhiteSpace();
//            qDebug("*** got IdxString: " + idxStr);
            QStringList fNames = QStringList::split(",", idxStr);
            for (QStringList::Iterator it = fNames.begin(); it != fNames.end(); ++it) {
                QSqlVariant::Type type = QSqlVariant::Invalid;
                if (rec.contains(sqlName))
                    type = rec.field(sqlName)->type();
                index.append((*it).stripWhiteSpace(), type);
            }
            return index;
        }
    }
    while (a > 0 && b > 0 && b > a) {
//        qDebug("loop, %d, %d", a, b);
        a = sqlForTable.find(",");
        b = sqlForTable.find("PRIMARY KEY");
        sqlForTable=sqlForTable.mid(a+1);
    }
//    qDebug("out, %d, %d", a, b);
    if (b > 0) 
    {
        // have a primary key
        sqlForTable = sqlForTable.left(b);
//        qDebug("working with string %s", sqlForTable.latin1());
        a = sqlForTable.findRev(",");
        b = sqlForTable.findRev("(");
//        qDebug("out, %d, %d", a, b);
        if (a > b) {
            sqlForTable = sqlForTable.mid(a);
        } else if (b > 0) {
            sqlForTable = sqlForTable.mid(b+1);
        }
//        qDebug("working with string %s", sqlForTable.latin1());

        a = sqlForTable.find(" ");
        if (a > 0) {
            sqlName = sqlForTable.left(a);
            sqlType = sqlForTable.mid(a+1);
        } else {
            sqlName = sqlForTable;
        }
    }

    if (!sqlName.isEmpty()) {
//        qDebug("sqlprimary %s, %s", sqlName.latin1() , sqlType.isEmpty() ? "(none)": sqlType.latin1());
    } else {
//        qDebug("no primary index found");
        return index;
    }
    QSqlVariant::Type type = QSqlVariant::Invalid;
    if (rec.contains(sqlName))
        type = rec.field(sqlName)->type();
    index.append(sqlName.lower(), type);
    index.setName(sqlName.lower());
    return index;
}

QSqlRecordInfo QSQLiteDriver::recordInfo(const QString &tbl) const
{
    if (!isOpen())
        return QSqlRecordInfo();

    QSqlQuery q = createQuery();
    q.setForwardOnly(TRUE);
    q.exec("SELECT * FROM " + tbl + " LIMIT 1");
    return recordInfo(q);
}

QSqlRecord QSQLiteDriver::record(const QString &tblname) const
{
    if (!isOpen())
        return QSqlRecord();

    return recordInfo(tblname).toRecord();
}

QSqlRecord QSQLiteDriver::record(const QSqlQuery& query) const
{
    if (query.isActive() && query.driver() == this) {
        QSQLiteResult* result = (QSQLiteResult*)query.result();
        return result->d->rInf.toRecord();    
    }
    return QSqlRecord();
}

QSqlRecordInfo QSQLiteDriver::recordInfo(const QSqlQuery& query) const
{
    if (query.isActive() && query.driver() == this) {
        QSQLiteResult* result = (QSQLiteResult*)query.result();
        return result->d->rInf;    
    }
    return QSqlRecordInfo();
}
