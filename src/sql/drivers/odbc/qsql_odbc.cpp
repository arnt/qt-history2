/****************************************************************************
**
** Implementation of ODBC driver classes.
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

#include "qsql_odbc.h"
#include <qsqlrecord.h>

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#include <qapplication.h>
#endif
#include <qcorevariant.h>
#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qstringlist.h>
#include <qvarlengtharray.h>
#include <qvector.h>

// undefine this to prevent initial check of the ODBC driver
#define ODBC_CHECK_DRIVER

#if defined(Q_ODBC_VERSION_2)
//crude hack to get non-unicode capable driver managers to work
# undef UNICODE
# define SQLTCHAR SQLCHAR
# define SQL_C_WCHAR SQL_C_CHAR
#endif

static const int COLNAMESIZE = 255;
//Map Qt parameter types to ODBC types
static const SQLSMALLINT qParamType[4] = { SQL_PARAM_INPUT, SQL_PARAM_INPUT, SQL_PARAM_OUTPUT, SQL_PARAM_INPUT_OUTPUT };

class QODBCDriverPrivate
{
public:
    QODBCDriverPrivate()
    : hEnv(0), hDbc(0), useSchema(false)
    {
        sql_char_type = sql_varchar_type = sql_longvarchar_type = QCoreVariant::CString;
        unicode = false;
    }

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;

    uint unicode :1;
    uint useSchema :1;
    QCoreVariant::Type sql_char_type;
    QCoreVariant::Type sql_varchar_type;
    QCoreVariant::Type sql_longvarchar_type;

    bool checkDriver() const;
    void checkUnicode();
    void checkSchemaUsage();
    bool setConnectionOptions(const QString& connOpts);
    void splitTableQualifier(const QString &qualifier, QString &catalog,
                             QString &schema, QString &table);
};

class QODBCPrivate
{
public:
    QODBCPrivate()
    : hEnv(0), hDbc(0), hStmt(0), useSchema(false)
    {
        sql_char_type = sql_varchar_type = sql_longvarchar_type = QCoreVariant::CString;
        unicode = false;
    }

    inline void clearValues()
    { fieldCache.fill(QCoreVariant()); }

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    SQLHANDLE hStmt;

    uint unicode :1;
    uint useSchema :1;
    QCoreVariant::Type sql_char_type;
    QCoreVariant::Type sql_varchar_type;
    QCoreVariant::Type sql_longvarchar_type;

    QSqlRecord rInf;
    QVector<QCoreVariant> fieldCache;
};

static QString qWarnODBCHandle(int handleType, SQLHANDLE handle)
{
    SQLINTEGER nativeCode_;
    SQLSMALLINT msgLen;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR state_[SQL_SQLSTATE_SIZE+1];
    SQLTCHAR description_[SQL_MAX_MESSAGE_LENGTH];
    r = SQLGetDiagRec(handleType,
                         handle,
                         1,
                         (SQLTCHAR*)state_,
                         &nativeCode_,
                         (SQLTCHAR*)description_,
                         SQL_MAX_MESSAGE_LENGTH-1, /* in bytes, not in characters */
                         &msgLen);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
#ifdef UNICODE
        return QString((const QChar*)description_, (uint)msgLen);
#else
        return QString::fromLocal8Bit((const char*)description_);
#endif
    return QString();
}

static QString qODBCWarn(const QODBCPrivate* odbc)
{
    return (qWarnODBCHandle(SQL_HANDLE_ENV, odbc->hEnv) + " "
             + qWarnODBCHandle(SQL_HANDLE_DBC, odbc->hDbc) + " "
             + qWarnODBCHandle(SQL_HANDLE_STMT, odbc->hStmt));
}

static QString qODBCWarn(const QODBCDriverPrivate* odbc)
{
    return (qWarnODBCHandle(SQL_HANDLE_ENV, odbc->hEnv) + " "
             + qWarnODBCHandle(SQL_HANDLE_DBC, odbc->hDbc));
}

static void qSqlWarning(const QString& message, const QODBCPrivate* odbc)
{
    qWarning("%s\tError: %s", message.local8Bit(), qODBCWarn(odbc).local8Bit());
}

static void qSqlWarning(const QString &message, const QODBCDriverPrivate *odbc)
{
    qWarning("%s\tError: %s", message.local8Bit(), qODBCWarn(odbc).local8Bit());
}

static QSqlError qMakeError(const QString& err, int type, const QODBCPrivate* p)
{
    return QSqlError("QODBC3: " + err, qODBCWarn(p), type);
}

static QSqlError qMakeError(const QString& err, int type, const QODBCDriverPrivate* p)
{
    return QSqlError("QODBC3: " + err, qODBCWarn(p), type);
}

template<class T>
static QCoreVariant::Type qDecodeODBCType(SQLSMALLINT sqltype, const T* p)
{
    QCoreVariant::Type type = QCoreVariant::Invalid;
    switch (sqltype) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
        type = QCoreVariant::Double;
        break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
    case SQL_TINYINT:
        type = QCoreVariant::Int;
        break;
    case SQL_BIGINT:
        type = QCoreVariant::LongLong;
        break;
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        type = QCoreVariant::ByteArray;
        break;
    case SQL_DATE:
    case SQL_TYPE_DATE:
        type = QCoreVariant::Date;
        break;
    case SQL_TIME:
    case SQL_TYPE_TIME:
        type = QCoreVariant::Time;
        break;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
        type = QCoreVariant::DateTime;
        break;
#ifndef Q_ODBC_VERSION_2
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
        type = QCoreVariant::String;
        break;
#endif
    case SQL_CHAR:
        type = p->sql_char_type;
        break;
    case SQL_VARCHAR:
        type = p->sql_varchar_type;
        break;
    case SQL_LONGVARCHAR:
        type = p->sql_longvarchar_type;
        break;
    default:
        type = QCoreVariant::CString;
        break;
    }
    return type;
}

static QString qGetStringData(SQLHANDLE hStmt, int column, int colSize, bool unicode = false)
{
    QString fieldVal;
    SQLRETURN r = SQL_ERROR;
    SQLINTEGER lengthIndicator = 0;

    if (colSize <= 0) {
        colSize = 255;
    } else if (colSize > 65536) { // limit buffer size to 64 KB
        colSize = 65536;
    } else {
        colSize++; // make sure there is room for more than the 0 termination
        if (unicode) {
            colSize *= 2; // a tiny bit faster, since it saves a SQLGetData() call
        }
    }
    char* buf = new char[colSize];
    while (true) {
        r = SQLGetData(hStmt,
                        column+1,
                        unicode ? SQL_C_WCHAR : SQL_C_CHAR,
                        (SQLPOINTER)buf,
                        colSize,
                        &lengthIndicator);
        if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
            if (lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL) {
                fieldVal = QString::null;
                break;
            }
            // if SQL_SUCCESS_WITH_INFO is returned, indicating that
            // more data can be fetched, the length indicator does NOT
            // contain the number of bytes returned - it contains the
            // total number of bytes that CAN be fetched
            // colSize-1: remove 0 termination when there is more data to fetch
            int rSize = (r == SQL_SUCCESS_WITH_INFO) ? (unicode ? colSize-2 : colSize-1) : lengthIndicator;
            if (unicode) {
                fieldVal += QString((QChar*) buf, rSize / 2);
            } else {
                buf[rSize] = 0;
                fieldVal += buf;
            }
            if (lengthIndicator < colSize) {
                // workaround for Drivermanagers that don't return SQL_NO_DATA
                break;
            }
        } else if (r == SQL_NO_DATA) {
            break;
        } else {
            qWarning("qGetStringData: Error while fetching data (%d)", r);
            fieldVal = QString::null;
            break;
        }
    }
    delete[] buf;
    return fieldVal;
}

static QCoreVariant qGetBinaryData(SQLHANDLE hStmt, int column)
{
    QByteArray fieldVal;
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQL_ERROR;

    SQLTCHAR colName[COLNAMESIZE];
    r = SQLDescribeCol(hStmt,
                        column+1,
                        colName,
                        COLNAMESIZE,
                        &colNameLen,
                        &colType,
                        &colSize,
                        &colScale,
                        &nullable);
    if (r != SQL_SUCCESS)
        qWarning("qGetBinaryData: Unable to describe column %d", column);
    // SQLDescribeCol may return 0 if size cannot be determined
    if (!colSize)
        colSize = 255;
    else if (colSize > 65536) // read the field in 64 KB chunks
        colSize = 65536;
    fieldVal.resize(colSize);
    ulong read = 0;
    while (true) {
        r = SQLGetData(hStmt,
                        column+1,
                        SQL_C_BINARY,
                        (SQLPOINTER)(fieldVal.constData() - read),
                        fieldVal.size() - read,
                        &lengthIndicator);
        if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO)
            break;
        if (lengthIndicator == SQL_NULL_DATA)
            return QCoreVariant(QCoreVariant::ByteArray);
        if (lengthIndicator == SQL_NO_TOTAL) {
            read += colSize;
            colSize = 65536;
        } else {
            read += lengthIndicator;
        }
        if (r == SQL_SUCCESS) { // the whole field was read in one chunk
            fieldVal.resize(read);
            break;
        }
        fieldVal.resize(fieldVal.size() + colSize);
    }
    return fieldVal;
}

static QCoreVariant qGetIntData(SQLHANDLE hStmt, int column)
{
    SQLINTEGER intbuf = 0;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData(hStmt,
                              column+1,
                              SQL_C_SLONG,
                              (SQLPOINTER)&intbuf,
                              0,
                              &lengthIndicator);
    if ((r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) || lengthIndicator == SQL_NULL_DATA) {
        return QCoreVariant(QCoreVariant::Int);
    }
    return QCoreVariant((int)intbuf);
}

static QCoreVariant qGetDoubleData(SQLHANDLE hStmt, int column)
{
    SQLDOUBLE dblbuf;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData(hStmt,
                              column+1,
                              SQL_C_DOUBLE,
                              (SQLPOINTER)&dblbuf,
                              0,
                              &lengthIndicator);
    if ((r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) || lengthIndicator == SQL_NULL_DATA)
        return QCoreVariant(QCoreVariant::Double);

    return (double) dblbuf;
}

static QCoreVariant qGetBigIntData(SQLHANDLE hStmt, int column)
{
    SQLBIGINT lngbuf = Q_INT64_C(0);
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData(hStmt,
                              column+1,
                              SQL_C_SBIGINT,
                              (SQLPOINTER) &lngbuf,
                              0,
                              &lengthIndicator);
    if ((r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) || lengthIndicator == SQL_NULL_DATA)
        return QCoreVariant(QCoreVariant::LongLong);

    return QCoreVariant(lngbuf);
}

// creates a QSqlField from a valid hStmt generated
// by SQLColumns. The hStmt has to point to a valid position.
static QSqlField qMakeFieldInfo(const SQLHANDLE hStmt, const QODBCDriverPrivate* p)
{
    QString fname = qGetStringData(hStmt, 3, -1, p->unicode);
    int type = qGetIntData(hStmt, 4).toInt(); // column type
    QSqlField f(qGetStringData(hStmt, 3, -1, p->unicode), qDecodeODBCType(type, p));
    int required = qGetIntData(hStmt, 10).toInt(); // nullable-flag
    // required can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    if (required == SQL_NO_NULLS)
        f.setRequired(true);
    else if (required == SQL_NULLABLE)
        f.setRequired(false);
    // else we don't know
    QCoreVariant var = qGetIntData(hStmt, 6);
    f.setLength(var.isNull() ? -1 : var.toInt()); // column size
    var = qGetIntData(hStmt, 8).toInt();
    f.setPrecision(var.isNull() ? -1 : var.toInt()); // precision
    f.setSqlType(type);
    return f;
}

static QSqlField qMakeFieldInfo(const QODBCPrivate* p, int i )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR colName[COLNAMESIZE];
    r = SQLDescribeCol(p->hStmt,
                        i+1,
                        colName,
                        (SQLSMALLINT)COLNAMESIZE,
                        &colNameLen,
                        &colType,
                        &colSize,
                        &colScale,
                        &nullable);

    if (r != SQL_SUCCESS) {
        qSqlWarning(QString("qMakeField: Unable to describe column %1").arg(i), p);
        return QSqlField();
    }
#ifdef UNICODE
    QString qColName((const QChar*)colName, (uint)colNameLen);
#else
    QString qColName = QString::fromLocal8Bit((const char*)colName);
#endif
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if (nullable == SQL_NO_NULLS) {
        required = 1;
    } else if (nullable == SQL_NULLABLE) {
        required = 0;
    }
    QCoreVariant::Type type = qDecodeODBCType(colType, p);
    QSqlField f(qColName, type);
    f.setSqlType(colType);
    f.setLength(colSize == 0 ? -1 : (int)colSize);
    f.setPrecision(colScale == 0 ? -1 : (int)colSize);
    if (nullable == SQL_NO_NULLS)
        f.setRequired(true);
    else if (nullable == SQL_NULLABLE)
        f.setRequired(false);
    // else we don't know
    return f;
}

bool QODBCDriverPrivate::setConnectionOptions(const QString& connOpts)
{
    // Set any connection attributes
    const QStringList opts(connOpts.split(';', QString::SkipEmptyParts));
    SQLRETURN r = SQL_SUCCESS;
    for (int i = 0; i < opts.count(); ++i) {
        const QString tmp(opts.at(i));
        int idx;
        if ((idx = tmp.indexOf('=')) == -1) {
            qWarning("QODBCDriver::open: Illegal connect option value '%s'", tmp.latin1());
            continue;
        }
        const QString opt(tmp.left(idx));
        const QString val(tmp.mid(idx + 1).simplified());
        SQLUINTEGER v = 0;

        r = SQL_SUCCESS;
        if (opt == "SQL_ATTR_ACCESS_MODE") {
            if (val == "SQL_MODE_READ_ONLY") {
                v = SQL_MODE_READ_ONLY;
            } else if (val == "SQL_MODE_READ_WRITE") {
                v = SQL_MODE_READ_WRITE;
            } else {
                qWarning("QODBCDriver::open: Unknown option value '%s'",
                         val.local8Bit());
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER) v, 0);
        } else if (opt == "SQL_ATTR_CONNECTION_TIMEOUT") {
            v = val.toUInt();
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER) v, 0);
        } else if (opt == "SQL_ATTR_LOGIN_TIMEOUT") {
            v = val.toUInt();
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) v, 0);
        } else if (opt == "SQL_ATTR_CURRENT_CATALOG") {
            val.utf16(); // 0 terminate
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CURRENT_CATALOG,
#ifdef UNICODE
                                    (SQLWCHAR*) val.unicode(),
#else
                                    (SQLCHAR*) val.latin1(),
#endif
                                    SQL_NTS);
        } else if (opt == "SQL_ATTR_METADATA_ID") {
            if (val == "SQL_TRUE") {
                v = SQL_TRUE;
            } else if (val == "SQL_FALSE") {
                v = SQL_FALSE;
            } else {
                qWarning("QODBCDriver::open: Unknown option value '%s'",
                         val.local8Bit());
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) v, 0);
        } else if (opt == "SQL_ATTR_PACKET_SIZE") {
            v = val.toUInt();
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_PACKET_SIZE, (SQLPOINTER) v, 0);
        } else if (opt == "SQL_ATTR_TRACEFILE") {
            val.utf16(); // 0 terminate
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_TRACEFILE,
#ifdef UNICODE
                                    (SQLWCHAR*) val.unicode(),
#else
                                    (SQLCHAR*) val.latin1(),
#endif
                                    SQL_NTS);
        } else if (opt == "SQL_ATTR_TRACE") {
            if (val == "SQL_OPT_TRACE_OFF") {
                v = SQL_OPT_TRACE_OFF;
            } else if (val == "SQL_OPT_TRACE_ON") {
                v = SQL_OPT_TRACE_ON;
            } else {
                qWarning("QODBCDriver::open: Unknown option value '%s'", val.local8Bit());
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_TRACE, (SQLPOINTER) v, 0);
#ifndef Q_ODBC_VERSION_2
        } else if (opt == "SQL_ATTR_CONNECTION_POOLING") {
            if (val == "SQL_CP_OFF")
                v = SQL_CP_OFF;
            else if (val == "SQL_CP_ONE_PER_DRIVER")
                v = SQL_CP_ONE_PER_DRIVER;
            else if (val == "SQL_CP_ONE_PER_HENV")
                v = SQL_CP_ONE_PER_HENV;
            else if (val == "SQL_CP_DEFAULT")
                v = SQL_CP_DEFAULT;
            else {
                qWarning("QODBCDriver::open: Unknown option value '%s'", val.local8Bit());
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)v, 0);
        } else if (opt == "SQL_ATTR_CP_MATCH") {
            if (val == "SQL_CP_STRICT_MATCH")
                v = SQL_CP_STRICT_MATCH;
            else if (val == "SQL_CP_RELAXED_MATCH")
                v = SQL_CP_RELAXED_MATCH;
            else if (val == "SQL_CP_MATCH_DEFAULT")
                v = SQL_CP_MATCH_DEFAULT;
            else {
                qWarning("QODBCDriver::open: Unknown option value '%s'", val.local8Bit());
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CP_MATCH, (SQLPOINTER)v, 0);
#endif
        } else {
                qWarning("QODBCDriver::open: Unknown connection attribute '%s'", opt.local8Bit());
        }
        if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO)
            qSqlWarning(QString("QODBCDriver::open: Unable to set connection attribute'%1'").arg(
                        opt), this);
    }
    return true;
}

void QODBCDriverPrivate::splitTableQualifier(const QString & qualifier, QString &catalog,
                                       QString &schema, QString &table)
{
    if (!useSchema) {
        table = qualifier;
        return;
    }
    QStringList l = qualifier.split('.');
    if (l.count() > 3)
        return; // can't possibly be a valid table qualifier
    int i = 0, n = l.count();
    if (n == 1) {
        table = qualifier;
    } else {
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            if (n == 3) {
                if (i == 0) {
                    catalog = *it;
                } else if (i == 1) {
                    schema = *it;
                } else if (i == 2) {
                    table = *it;
                }
            } else if (n == 2) {
                if (i == 0) {
                    schema = *it;
                } else if (i == 1) {
                    table = *it;
                }
            }
            i++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////

QODBCResult::QODBCResult(const QODBCDriver * db, QODBCDriverPrivate* p)
: QSqlResult(db)
{
    d = new QODBCPrivate();
    d->hEnv = p->hEnv;
    d->hDbc = p->hDbc;
    d->unicode = p->unicode;
    d->useSchema = p->useSchema;
    d->sql_char_type = p->sql_char_type;
    d->sql_varchar_type = p->sql_varchar_type;
    d->sql_longvarchar_type = p->sql_longvarchar_type;
}

QODBCResult::~QODBCResult()
{
    if (d->hStmt && driver()->isOpen()) {
        SQLRETURN r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
        if (r != SQL_SUCCESS)
            qSqlWarning("QODBCDriver: Unable to free statement handle " + QString::number(r), d);
    }

    delete d;
}

bool QODBCResult::reset (const QString& query)
{
    setActive(false);
    setAt(QSql::BeforeFirst);
    SQLRETURN r;

    d->rInf.clear();
    d->fieldCache.clear();
    // Always reallocate the statement handle - the statement attributes
    // are not reset if SQLFreeStmt() is called which causes some problems.
    if (d->hStmt) {
        r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
        if (r != SQL_SUCCESS) {
            qSqlWarning("QODBCResult::reset: Unable to free statement handle", d);
            return false;
        }
    }
    r  = SQLAllocHandle(SQL_HANDLE_STMT,
                         d->hDbc,
                         &d->hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCResult::reset: Unable to allocate statement handle", d);
        return false;
    }

    if (isForwardOnly()) {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                            SQL_IS_UINTEGER);
    } else {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_STATIC,
                            SQL_IS_UINTEGER);
    }
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCResult::reset: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. Please check your ODBC driver configuration", d);
        return false;
    }

#ifdef UNICODE
    r = SQLExecDirect(d->hStmt,
                       (SQLWCHAR*) query.unicode(),
                       (SQLINTEGER) query.length());
#else
    QByteArray query8(query.local8Bit());
    r = SQLExecDirect(d->hStmt,
                       (SQLCHAR*) query8.constData(),
                       (SQLINTEGER) query8.length());
#endif
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        setLastError(qMakeError("Unable to execute statement", QSqlError::Statement, d));
        return false;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols(d->hStmt, &count);
    if (count) {
        setSelect(true);
        for (int i = 0; i < count; ++i) {
            d->rInf.append(qMakeFieldInfo(d, i));
        }
        d->fieldCache.resize(count);
    } else {
        setSelect(false);
    }
    setActive(true);
    return true;
}

bool QODBCResult::fetch(int i)
{
    if (isForwardOnly() && i < at())
        return false;
    if (i == at())
        return true;
    d->clearValues();
    int actualIdx = i + 1;
    if (actualIdx <= 0) {
        setAt(QSql::BeforeFirst);
        return false;
    }
    SQLRETURN r;
    if (isForwardOnly()) {
        bool ok = true;
        while (ok && i > at())
            ok = fetchNext();
        return ok;
    } else {
        r = SQLFetchScroll(d->hStmt,
                            SQL_FETCH_ABSOLUTE,
                            actualIdx);
    }
    if (r != SQL_SUCCESS){
        return false;
    }
    setAt(i);
    return true;
}

bool QODBCResult::fetchNext()
{
    SQLRETURN r;
    d->clearValues();
    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_NEXT,
                       0);
    if (r != SQL_SUCCESS)
        return false;
    setAt(at() + 1);
    return true;
}

bool QODBCResult::fetchFirst()
{
    if (isForwardOnly() && at() != QSql::BeforeFirst)
        return false;
    SQLRETURN r;
    d->clearValues();
    if (isForwardOnly()) {
        return fetchNext();
    }
    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_FIRST,
                       0);
    if (r != SQL_SUCCESS)
        return false;
    setAt(0);
    return true;
}

bool QODBCResult::fetchPrior()
{
    if (isForwardOnly())
        return false;
    SQLRETURN r;
    d->clearValues();
    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_PRIOR,
                       0);
    if (r != SQL_SUCCESS)
        return false;
    setAt(at() - 1);
    return true;
}

bool QODBCResult::fetchLast()
{
    SQLRETURN r;
    d->clearValues();

    if (isForwardOnly()) {
        // cannot seek to last row in forwardOnly mode, so we have to use brute force
        int i = at();
        if (i == QSql::AfterLast)
            return false;
        if (i == QSql::BeforeFirst)
            i = 0;
        while (fetchNext())
            ++i;
        setAt(i);
        return true;
    }

    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_LAST,
                       0);
    if (r != SQL_SUCCESS) {
        return false;
    }
    SQLINTEGER currRow;
    r = SQLGetStmtAttr(d->hStmt,
                        SQL_ROW_NUMBER,
                        &currRow,
                        SQL_IS_INTEGER,
                        0);
    if (r != SQL_SUCCESS)
        return false;
    setAt(currRow-1);
    return true;
}

QCoreVariant QODBCResult::data(int field)
{
    if (field >= d->rInf.count() || field < 0) {
        qWarning("QODBCResult::data: column %d out of range", field);
        return QCoreVariant();
    }
    if (field < d->fieldCache.size() && d->fieldCache.at(field).isValid())
        return d->fieldCache.at(field);
    SQLRETURN r(0);
    SQLINTEGER lengthIndicator = 0;

    const QSqlField info = d->rInf.field(field);
    switch (info.type()) {
        case QCoreVariant::LongLong:
            d->fieldCache[field] = qGetBigIntData(d->hStmt, field);
        break;
        case QCoreVariant::Int:
            d->fieldCache[field] = qGetIntData(d->hStmt, field);
        break;
        case QCoreVariant::Date:
            DATE_STRUCT dbuf;
            r = SQLGetData(d->hStmt,
                            field + 1,
                            SQL_C_DATE,
                            (SQLPOINTER)&dbuf,
                            0,
                            &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
                d->fieldCache[field] = QCoreVariant(QDate(dbuf.year, dbuf.month, dbuf.day));
            else
                d->fieldCache[field] = QCoreVariant(QCoreVariant::Date);
        break;
        case QCoreVariant::Time:
            TIME_STRUCT tbuf;
            r = SQLGetData(d->hStmt,
                            field + 1,
                            SQL_C_TIME,
                            (SQLPOINTER)&tbuf,
                            0,
                            &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
                d->fieldCache[field] = QCoreVariant(QTime(tbuf.hour, tbuf.minute, tbuf.second));
            else
                d->fieldCache[field] = QCoreVariant(QCoreVariant::Time);
        break;
        case QCoreVariant::DateTime:
            TIMESTAMP_STRUCT dtbuf;
            r = SQLGetData(d->hStmt,
                            field + 1,
                            SQL_C_TIMESTAMP,
                            (SQLPOINTER)&dtbuf,
                            0,
                            &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
                d->fieldCache[field] = QCoreVariant(QDateTime(QDate(dtbuf.year, dtbuf.month, dtbuf.day), QTime(dtbuf.hour, dtbuf.minute, dtbuf.second)));
            else
                d->fieldCache[field] = QCoreVariant(QCoreVariant::DateTime);
            break;
        case QCoreVariant::ByteArray:
            d->fieldCache[field] = qGetBinaryData(d->hStmt, field);
            break;
        case QCoreVariant::String:
            d->fieldCache[field] = qGetStringData(d->hStmt, field, info.length(), true);
            break;
        case QCoreVariant::Double:
            if (info.typeID() == SQL_DECIMAL || info.typeID() == SQL_NUMERIC)
                // bind Double values as string to prevent loss of precision
                d->fieldCache[field] = qGetStringData(d->hStmt, field,
                                                       info.length() + 1, false); // length + 1 for the comma
            else
                d->fieldCache[field] = qGetDoubleData(d->hStmt, field);
            break;
        // ###        case QCoreVariant::CString:
        default:
            d->fieldCache[field] = QCoreVariant(qGetStringData(d->hStmt, field,
                                                              info.length(), false));
            break;
    }
    return d->fieldCache[field];
}

bool QODBCResult::isNull(int field)
{
    if (field < 0 || field > d->fieldCache.size())
        return true;
    if (!d->fieldCache.at(field).isValid()) {
        // since there is no good way to find out whether the value is NULL
        // without fetching the field we'll fetch it here.
        // (data() also sets the NULL flag)
        data(field);
    }
    return d->fieldCache.at(field).isNull();
}

int QODBCResult::size()
{
    return -1;
}

int QODBCResult::numRowsAffected()
{
    SQLINTEGER affectedRowCount(0);
    SQLRETURN r = SQLRowCount(d->hStmt, &affectedRowCount);
    if (r == SQL_SUCCESS)
        return affectedRowCount;
    else
        qSqlWarning("QODBCResult::numRowsAffected: Unable to count affected rows", d);
    return -1;
}

bool QODBCResult::prepare(const QString& query)
{
    setActive(false);
    setAt(QSql::BeforeFirst);
    SQLRETURN r;

    d->rInf.clear();
    if (d->hStmt) {
        r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
        if (r != SQL_SUCCESS) {
            qSqlWarning("QODBCResult::prepare: Unable to close statement", d);
            return false;
        }
    }
    r  = SQLAllocHandle(SQL_HANDLE_STMT,
                         d->hDbc,
                         &d->hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCResult::prepare: Unable to allocate statement handle", d);
        return false;
    }

    if (isForwardOnly()) {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                            SQL_IS_UINTEGER);
    } else {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_STATIC,
                            SQL_IS_UINTEGER);
    }
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCResult::prepare: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. Please check your ODBC driver configuration", d);
        return false;
    }

#ifdef UNICODE
    r = SQLPrepare(d->hStmt,
                    (SQLWCHAR*) query.unicode(),
                    (SQLINTEGER) query.length());
#else
    QByteArray query8(query.local8Bit());
    r = SQLPrepare(d->hStmt,
                    (SQLCHAR*) query8.constData(),
                    (SQLINTEGER) query8.length());
#endif

    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCResult::prepare: Unable to prepare statement", d);
        return false;
    }
    return true;
}

bool QODBCResult::exec()
{
    SQLRETURN r;
    QList<QByteArray> tmpStorage; // holds temporary buffers
    QVarLengthArray<SQLINTEGER, 32> indicators(boundValues().count());

    setActive(false);
    setAt(QSql::BeforeFirst);
    d->rInf.clear();

    if (!d->hStmt) {
            qSqlWarning("QODBCResult::exec: No statement handle available", d);
            return false;
    } else {
        r = SQLFreeStmt(d->hStmt, SQL_CLOSE);
        if (r != SQL_SUCCESS) {
            qSqlWarning("QODBCResult::exec: Unable to close statement handle", d);
            return false;
        }
    }

    // bind parameters - only positional binding allowed
    QVector<QCoreVariant>& values = boundValues();
    int i;
    for (i = 0; i < values.count(); ++i) {
        if (bindValueType(i) & QSql::Out)
            values[i].detach();
        QCoreVariant val(values.at(i));
        SQLINTEGER * ind = &indicators[i];
        if (val.isNull())
            *ind = SQL_NULL_DATA;
        switch (val.type()) {
            case QCoreVariant::Date: {
                QByteArray ba;
                ba.resize(sizeof(DATE_STRUCT));
                DATE_STRUCT *dt = (DATE_STRUCT *)ba.constData();
                QDate qdt = val.toDate();
                dt->year = qdt.year();
                dt->month = qdt.month();
                dt->day = qdt.day();
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_DATE,
                                      SQL_DATE,
                                      0,
                                      0,
                                      (void *) dt,
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                tmpStorage.append(ba);
                break; }
            case QCoreVariant::Time: {
                QByteArray ba;
                ba.resize(sizeof(TIME_STRUCT));
                TIME_STRUCT *dt = (TIME_STRUCT *)ba.constData();
                QTime qdt = val.toTime();
                dt->hour = qdt.hour();
                dt->minute = qdt.minute();
                dt->second = qdt.second();
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_TIME,
                                      SQL_TIME,
                                      0,
                                      0,
                                      (void *) dt,
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                tmpStorage.append(ba);
                break; }
            case QCoreVariant::DateTime: {
                QByteArray ba;
                ba.resize(sizeof(TIMESTAMP_STRUCT));
                TIMESTAMP_STRUCT * dt = (TIMESTAMP_STRUCT *)ba.constData();
                QDateTime qdt = val.toDateTime();
                dt->year = qdt.date().year();
                dt->month = qdt.date().month();
                dt->day = qdt.date().day();
                dt->hour = qdt.time().hour();
                dt->minute = qdt.time().minute();
                dt->second = qdt.time().second();
                dt->fraction = 0;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_TIMESTAMP,
                                      SQL_TIMESTAMP,
                                      0,
                                      0,
                                      (void *) dt,
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                tmpStorage.append(ba);
                break; }
            case QCoreVariant::Int:
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_SLONG,
                                      SQL_INTEGER,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                break;
            case QCoreVariant::Double:
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_DOUBLE,
                                      SQL_DOUBLE,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                break;
            case QCoreVariant::ByteArray:
                if (*ind != SQL_NULL_DATA) {
                    *ind = val.toByteArray().size();
                }
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_BINARY,
                                      SQL_LONGVARBINARY,
                                      val.toByteArray().size(),
                                      0,
                                      (void *) val.toByteArray().constData(),
                                      val.toByteArray().size(),
                                      ind);
                break;
#ifndef Q_ODBC_VERSION_2
            case QCoreVariant::String:
                if (d->unicode) {
                    QString str(val.toString());
                    str.utf16();
                    if (bindValueType(i) & QSql::Out) {
                        QByteArray ba((char*)str.constData(), str.capacity() * sizeof(QChar));
                        r = SQLBindParameter(d->hStmt,
                                            i + 1,
                                            qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                            SQL_C_WCHAR,
                                            SQL_WVARCHAR,
                                            0, // god knows... don't change this!
                                            0,
                                            (void *)ba.constData(),
                                            ba.size(),
                                            ind);
                        break;
                    }

                    r = SQLBindParameter(d->hStmt,
                                          i + 1,
                                          qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                          SQL_C_WCHAR,
                                          SQL_WVARCHAR,
                                          0, // god knows... don't change this!
                                          0,
                                          (void *)str.constData(),
                                          str.length() * sizeof(QChar),
                                          ind);
                    break;
                }
#endif
            // fall through
            default: {
                QByteArray ba(val.toString().local8Bit());
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[(QFlag)(bindValueType(i)) & QSql::InOut],
                                      SQL_C_CHAR,
                                      SQL_VARCHAR,
                                      ba.length() + 1,
                                      0,
                                      (void *) ba.constData(),
                                      ba.length() + 1,
                                      ind);
                tmpStorage.append(ba);
                break; }
        }
        if (r != SQL_SUCCESS) {
            qWarning("QODBCResult::exec: unable to bind variable: %s", qODBCWarn(d).local8Bit()
);
            setLastError(qMakeError("Unable to bind variable", QSqlError::Statement, d));
            return false;
        }
    }
    r = SQLExecute(d->hStmt);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qWarning("QODBCResult::exec: Unable to execute statement: %s", qODBCWarn(d).local8Bit()
);
        setLastError(qMakeError("Unable to execute statement", QSqlError::Statement, d));
        return false;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols(d->hStmt, &count);
    if (count) {
        setSelect(true);
        for (int i = 0; i < count; ++i) {
            d->rInf.append(qMakeFieldInfo(d, i));
        }
    } else {
        setSelect(false);
    }
    setActive(true);

    //get out parameters
    if (!hasOutValues())
        return true;

    for (i = 0; i < values.count(); ++i) {
        switch (values.at(i).type()) {
            case QCoreVariant::Date: {
                DATE_STRUCT ds = *((DATE_STRUCT *)tmpStorage.takeFirst().constData());
                values[i] = QCoreVariant(QDate(ds.year, ds.month, ds.day));
                break; }
            case QCoreVariant::Time: {
                TIME_STRUCT dt = *((TIME_STRUCT *)tmpStorage.takeFirst().constData());
                values[i] = QCoreVariant(QTime(dt.hour, dt.minute, dt.second));
                break; }
            case QCoreVariant::DateTime: {
                TIMESTAMP_STRUCT dt = *((TIMESTAMP_STRUCT*)
                                        tmpStorage.takeFirst().constData());
                values[i] = QCoreVariant(QDateTime(QDate(dt.year, dt.month, dt.day),
                                         QTime(dt.hour, dt.minute, dt.second)));
                break; }
            case QCoreVariant::Int:
            case QCoreVariant::Double:
            case QCoreVariant::ByteArray:
                //nothing to do
                break;
            case QCoreVariant::String:
                if (d->unicode) {
                    if (bindValueType(i) & QSql::Out)
                        values[i] = QString::fromUtf16((ushort*)tmpStorage.takeFirst().constData());
                    break;
                }
                // fall through
            default: {
                QByteArray ba = tmpStorage.takeFirst();
                if (bindValueType(i) & QSql::Out)
                    values[i] = QString::fromLocal8Bit(tmpStorage.takeFirst().constData());
                break; }
        }
        if (indicators[i] == SQL_NULL_DATA)
            values[i] = QCoreVariant(values[i].type());
    }
    return true;
}

QSqlRecord QODBCResult::record() const
{
    if (!isActive() || !isSelect())
        return QSqlRecord();
    return d->rInf;
}

////////////////////////////////////////


QODBCDriver::QODBCDriver(QObject *parent)
    : QSqlDriver(parent)
{
    init();
}

QODBCDriver::QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject * parent)
    : QSqlDriver(parent)
{
    init();
    d->hEnv = env;
    d->hDbc = con;
    if (env && con) {
        setOpen(true);
        setOpenError(false);
    }
}

void QODBCDriver::init()
{
    d = new QODBCDriverPrivate();
}

QODBCDriver::~QODBCDriver()
{
    cleanup();
    delete d;
}

bool QODBCDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions: {
        if (!d->hDbc)
            return false;
        SQLUSMALLINT txn;
        SQLSMALLINT t;
        int r = SQLGetInfo(d->hDbc,
                        (SQLUSMALLINT)SQL_TXN_CAPABLE,
                        &txn,
                        sizeof(txn),
                        &t);
        if (r != SQL_SUCCESS || txn == SQL_TC_NONE)
            return false;
        else
            return true;
    }
    case QuerySize:
        return false;
    case BLOB:
        return true;
    case Unicode:
        return d->unicode;
    case PreparedQueries:
        return true;
    case PositionalPlaceholders:
        return true;
    default:
        return false;
    }
}

bool QODBCDriver::open(const QString & db,
                        const QString & user,
                        const QString & password,
                        const QString &,
                        int,
                        const QString& connOpts)
{
    if (isOpen())
      close();
    SQLRETURN r;
    r = SQLAllocHandle(SQL_HANDLE_ENV,
                        SQL_NULL_HANDLE,
                        &d->hEnv);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qSqlWarning("QODBCDriver::open: Unable to allocate environment", d);
        setOpenError(true);
        return false;
    }
    r = SQLSetEnvAttr(d->hEnv,
                       SQL_ATTR_ODBC_VERSION,
                       (SQLPOINTER)SQL_OV_ODBC2,
                       SQL_IS_UINTEGER);
    r = SQLAllocHandle(SQL_HANDLE_DBC,
                        d->hEnv,
                        &d->hDbc);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qSqlWarning("QODBCDriver::open: Unable to allocate connection", d);
        setOpenError(true);
        return false;
    }

    if (!d->setConnectionOptions(connOpts))
        return false;

    // Create the connection string
    QString connQStr;
    // support the "DRIVER={SQL SERVER};SERVER=blah" syntax
    if (db.contains(".dsn"))
        connQStr = "FILEDSN=" + db;
    else if (db.contains("DRIVER") || db.contains("SERVER"))
        connQStr = db;
    else
        connQStr = "DSN=" + db;
    connQStr += ";UID=" + user + ";PWD=" + password;
    SQLSMALLINT cb;
    SQLTCHAR connOut[1024];
    r = SQLDriverConnect(d->hDbc,
                          NULL,
#ifdef UNICODE
                          (SQLWCHAR*)connQStr.unicode(),
#else
                          (SQLCHAR*)connQStr.latin1(),
#endif
                          (SQLSMALLINT)connQStr.length(),
                          connOut,
                          1024,
                          &cb,
                          SQL_DRIVER_NOPROMPT);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        setLastError(qMakeError("Unable to connect", QSqlError::Connection, d));
        setOpenError(true);
        return false;
    }

    if (!d->checkDriver()) {
        setLastError(qMakeError("Unable to connect - Driver doesn't support all needed functionality", QSqlError::Connection, d));
        setOpenError(true);
        return false;
    }

    d->checkUnicode();
    d->checkSchemaUsage();

    setOpen(true);
    setOpenError(false);
    return true;
}

void QODBCDriver::close()
{
    cleanup();
    setOpen(false);
    setOpenError(false);
}

void QODBCDriver::cleanup()
{
    SQLRETURN r;
    if (!d)
        return;

    if(d->hDbc) {
        // Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
        if (isOpen()) {
            r = SQLDisconnect(d->hDbc);
            if (r != SQL_SUCCESS)
                qSqlWarning("QODBCDriver::disconnect: Unable to disconnect datasource", d);
        }

        r = SQLFreeHandle(SQL_HANDLE_DBC, d->hDbc);
        if (r != SQL_SUCCESS)
            qSqlWarning("QODBCDriver::cleanup: Unable to free connection handle", d);
        d->hDbc = 0;
    }

    if (d->hEnv) {
        r = SQLFreeHandle(SQL_HANDLE_ENV, d->hEnv);
        if (r != SQL_SUCCESS)
            qSqlWarning("QODBCDriver::cleanup: Unable to free environment handle", d);
        d->hEnv = 0;
    }
}

// checks whether the server can return char, varchar and longvarchar
// as two byte unicode characters
void QODBCDriverPrivate::checkUnicode()
{
#if defined(Q_WS_WIN)
    if (!qt_winunicode) {
        unicode = false;
        return;
    }
#endif
    SQLRETURN   r;
    SQLUINTEGER fFunc;

    unicode = false;
    r = SQLGetInfo(hDbc,
                    SQL_CONVERT_CHAR,
                    (SQLPOINTER)&fFunc,
                    sizeof(fFunc),
                    NULL);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WCHAR)) {
        sql_char_type = QCoreVariant::String;
        unicode = true;
    }

    r = SQLGetInfo(hDbc,
                    SQL_CONVERT_VARCHAR,
                    (SQLPOINTER)&fFunc,
                    sizeof(fFunc),
                    NULL);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WVARCHAR)) {
        sql_varchar_type = QCoreVariant::String;
        unicode = true;
    }

    r = SQLGetInfo(hDbc,
                    SQL_CONVERT_LONGVARCHAR,
                    (SQLPOINTER)&fFunc,
                    sizeof(fFunc),
                    NULL);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WLONGVARCHAR)) {
        sql_longvarchar_type = QCoreVariant::String;
        unicode = true;
    }
}

bool QODBCDriverPrivate::checkDriver() const
{
#ifdef ODBC_CHECK_DRIVER
    // do not query for SQL_API_SQLFETCHSCROLL because it can't be used at this time
    static const SQLUSMALLINT reqFunc[] = {
                SQL_API_SQLDESCRIBECOL, SQL_API_SQLGETDATA, SQL_API_SQLCOLUMNS,
                SQL_API_SQLGETSTMTATTR, SQL_API_SQLGETDIAGREC, SQL_API_SQLEXECDIRECT,
                SQL_API_SQLGETINFO, SQL_API_SQLTABLES, 0
    };

    // these functions are optional
    static const SQLUSMALLINT optFunc[] = {
        SQL_API_SQLNUMRESULTCOLS, SQL_API_SQLROWCOUNT, 0
    };

    SQLRETURN r;
    SQLUSMALLINT sup;


    int i;
    // check the required functions
    for (i = 0; reqFunc[i] != 0; ++i) {

        r = SQLGetFunctions(hDbc, reqFunc[i], &sup);

        if (r != SQL_SUCCESS) {
            qSqlWarning("QODBCDriver::checkDriver: Cannot get list of supported functions", this);
            return false;
        }
        if (sup == SQL_FALSE) {
            qWarning ("QODBCDriver::open: Warning - Driver doesn't support all needed functionality (%d). "
                       "Please look at the Qt SQL Module Driver documentation for more information.", reqFunc[i]);
            return false;
        }
    }

    // these functions are optional and just generate a warning
    for (i = 0; optFunc[i] != 0; ++i) {

        r = SQLGetFunctions(hDbc, optFunc[i], &sup);

        if (r != SQL_SUCCESS) {
            qSqlWarning("QODBCDriver::checkDriver: Cannot get list of supported functions", this);
            return false;
        }
        if (sup == SQL_FALSE) {
            qWarning("QODBCDriver::checkDriver: Warning - Driver doesn't support some non-critical functions (%d)", optFunc[i]);
            return true;
        }
    }
#endif //ODBC_CHECK_DRIVER

    return true;
}

void QODBCDriverPrivate::checkSchemaUsage()
{
    SQLRETURN   r;
    SQLUINTEGER val;

    r = SQLGetInfo(hDbc,
                   SQL_SCHEMA_USAGE,
                   (SQLPOINTER) &val,
                   sizeof(val),
                   NULL);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
        useSchema = (val != 0);
}

QSqlQuery QODBCDriver::createQuery() const
{
    return QSqlQuery(new QODBCResult(this, d));
}

bool QODBCDriver::beginTransaction()
{
    if (!isOpen()) {
        qWarning(" QODBCDriver::beginTransaction: Database not open");
        return false;
    }
    SQLUINTEGER ac(SQL_AUTOCOMMIT_OFF);
    SQLRETURN r  = SQLSetConnectAttr(d->hDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)ac,
                                      sizeof(ac));
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError("Unable to disable autocommit", QSqlError::Transaction, d));
        return false;
    }
    return true;
}

bool QODBCDriver::commitTransaction()
{
    if (!isOpen()) {
        qWarning(" QODBCDriver::commitTransaction: Database not open");
        return false;
    }
    SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,
                              d->hDbc,
                              SQL_COMMIT);
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError("Unable to commit transaction", QSqlError::Transaction, d));
        return false;
    }
    return endTrans();
}

bool QODBCDriver::rollbackTransaction()
{
    if (!isOpen()) {
        qWarning(" QODBCDriver::rollbackTransaction: Database not open");
        return false;
    }
    SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,
                              d->hDbc,
                              SQL_ROLLBACK);
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError("Unable to rollback transaction", QSqlError::Transaction, d));
        return false;
    }
    return endTrans();
}

bool QODBCDriver::endTrans()
{
    SQLUINTEGER ac(SQL_AUTOCOMMIT_ON);
    SQLRETURN r  = SQLSetConnectAttr(d->hDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)ac,
                                      sizeof(ac));
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError("Unable to enable autocommit", QSqlError::Transaction, d));
        return false;
    }
    return true;
}

QStringList QODBCDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;
    SQLHANDLE hStmt;

    SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  d->hDbc,
                                  &hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCDriver::tables: Unable to allocate handle", d);
        return tl;
    }
    r = SQLSetStmtAttr(hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER);
    QString tableType;
    if (type & QSql::Tables)
        tableType += "TABLE,";
    if (type & QSql::Views)
        tableType += "VIEW,";
    if (type & QSql::SystemTables)
        tableType += "SYSTEM TABLE,";
    if (tableType.isEmpty())
        return tl;
    tableType.truncate(tableType.length() - 1);

    r = SQLTables(hStmt,
                   NULL,
                   0,
                   NULL,
                   0,
                   NULL,
                   0,
#ifdef UNICODE
                   (SQLWCHAR*)tableType.unicode(),
#else
                   (SQLCHAR*)tableType.latin1(),
#endif
                   tableType.length() /* characters, not bytes */);

    if (r != SQL_SUCCESS)
        qSqlWarning("QODBCDriver::tables Unable to execute table list", d);
    r = SQLFetchScroll(hStmt,
                        SQL_FETCH_NEXT,
                        0);
    while (r == SQL_SUCCESS) {
        QString fieldVal = qGetStringData(hStmt, 2, -1, d->unicode);
        tl.append(fieldVal);
        r = SQLFetchScroll(hStmt,
                            SQL_FETCH_NEXT,
                            0);
    }

    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (r!= SQL_SUCCESS)
        qSqlWarning("QODBCDriver: Unable to free statement handle" + QString::number(r), d);
    return tl;
}

QSqlIndex QODBCDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex index(tablename);
    if (!isOpen())
        return index;
    bool usingSpecialColumns = false;
    QSqlRecord rec = record(tablename);

    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  d->hDbc,
                                  &hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCDriver::primaryIndex: Unable to list primary key", d);
        return index;
    }
    QString catalog, schema, table;
    d->splitTableQualifier(tablename, catalog, schema, table);
    r = SQLSetStmtAttr(hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER);
    r = SQLPrimaryKeys(hStmt,
#ifdef UNICODE
                        catalog.length() == 0 ? NULL : (SQLWCHAR*)catalog.unicode(),
#else
                        catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.latin1(),
#endif
                        catalog.length(),
#ifdef UNICODE
                        schema.length() == 0 ? NULL : (SQLWCHAR*)schema.unicode(),
#else
                        schema.length() == 0 ? NULL : (SQLCHAR*)schema.latin1(),
#endif
                        schema.length(),
#ifdef UNICODE
                        (SQLWCHAR*)table.unicode(),
#else
                        (SQLCHAR*)table.latin1(),
#endif
                        table.length() /* in characters, not in bytes */);

    // if the SQLPrimaryKeys() call does not succeed (e.g the driver
    // does not support it) - try an alternative method to get hold of
    // the primary index (e.g MS Access and FoxPro)
    if (r != SQL_SUCCESS) {
            r = SQLSpecialColumns(hStmt,
                                   SQL_BEST_ROWID,
#ifdef UNICODE
                                   catalog.length() == 0 ? NULL : (SQLWCHAR*)catalog.unicode(),
#else
                                   catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.latin1(),
#endif
                                   catalog.length(),
#ifdef UNICODE
                                   schema.length() == 0 ? NULL : (SQLWCHAR*)schema.unicode(),
#else
                                   schema.length() == 0 ? NULL : (SQLCHAR*)schema.latin1(),
#endif
                                   schema.length(),
#ifdef UNICODE
                                   (SQLWCHAR*)table.unicode(),
#else
                                   (SQLCHAR*)table.latin1(),
#endif

                                   table.length(),
                                   SQL_SCOPE_CURROW,
                                   SQL_NULLABLE);

            if (r != SQL_SUCCESS) {
                qSqlWarning("QODBCDriver::primaryIndex: Unable to execute primary key list", d);
            } else {
                usingSpecialColumns = true;
            }
    }
    r = SQLFetchScroll(hStmt,
                        SQL_FETCH_NEXT,
                        0);
    int fakeId = 0;
    QString cName, idxName;
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while (r == SQL_SUCCESS) {
        if (usingSpecialColumns) {
            cName = qGetStringData(hStmt, 1, -1, d->unicode); // column name
            idxName = QString::number(fakeId++); // invent a fake index name
        } else {
            cName = qGetStringData(hStmt, 3, -1, d->unicode); // column name
            idxName = qGetStringData(hStmt, 5, -1, d->unicode); // pk index name
        }
        index.append(rec.field(cName));
        index.setName(idxName);
        r = SQLFetchScroll(hStmt,
                            SQL_FETCH_NEXT,
                            0);
    }
    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (r!= SQL_SUCCESS)
        qSqlWarning("QODBCDriver: Unable to free statement handle" + QString::number(r), d);
    return index;
}

QSqlRecord QODBCDriver::record(const QString& tablename) const
{
    QSqlRecord fil;
    if (!isOpen())
        return fil;

    SQLHANDLE hStmt;
    QString catalog, schema, table;
    d->splitTableQualifier(tablename, catalog, schema, table);
    SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  d->hDbc,
                                  &hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning("QODBCDriver::record: Unable to allocate handle", d);
        return fil;
    }
    r = SQLSetStmtAttr(hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER);
    r =  SQLColumns(hStmt,
#ifdef UNICODE
                     catalog.length() == 0 ? NULL : (SQLWCHAR*)catalog.unicode(),
#else
                     catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.latin1(),
#endif
                     catalog.length(),
#ifdef UNICODE
                     schema.length() == 0 ? NULL : (SQLWCHAR*)schema.unicode(),
#else
                     schema.length() == 0 ? NULL : (SQLCHAR*)schema.latin1(),
#endif
                     schema.length(),
#ifdef UNICODE
                     (SQLWCHAR*)table.unicode(),
#else
                     (SQLCHAR*)table.latin1(),
#endif
                     table.length(),
                     NULL,
                     0);
    if (r != SQL_SUCCESS)
        qSqlWarning("QODBCDriver::record: Unable to execute column list", d);
    r = SQLFetchScroll(hStmt,
                        SQL_FETCH_NEXT,
                        0);
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while (r == SQL_SUCCESS) {

        fil.append(qMakeFieldInfo(hStmt, d));

        r = SQLFetchScroll(hStmt,
                            SQL_FETCH_NEXT,
                            0);
    }

    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (r!= SQL_SUCCESS)
        qSqlWarning("QODBCDriver: Unable to free statement handle " + QString::number(r), d);

    return fil;
}

SQLHANDLE QODBCDriver::environment()
{
    return d->hEnv;
}

SQLHANDLE QODBCDriver::connection()
{
    return d->hDbc;
}

QString QODBCDriver::formatValue(const QSqlField &field,
                                 bool trimStrings) const
{
    QString r;
    if (field.isNull()) {
        r = QLatin1String("NULL");
    } else if (field.type() == QCoreVariant::DateTime) {
        // Use an escape sequence for the datetime fields
        if (field.value().toDateTime().isValid()){
            QDate dt = field.value().toDateTime().date();
            QTime tm = field.value().toDateTime().time();
            // Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
            r = "{ ts '" +
                QString::number(dt.year()) + "-" +
                QString::number(dt.month()).rightJustified(2, '0', true) + "-" +
                QString::number(dt.day()).rightJustified(2, '0', true) + " " +
                tm.toString() +
                "' }";
        } else
            r = QLatin1String("NULL");
    } else if (field.type() == QCoreVariant::ByteArray) {
        QByteArray ba = field.value().toByteArray();
        QString res;
        static const char hexchars[] = "0123456789abcdef";
        for (int i = 0; i < ba.size(); ++i) {
            uchar s = (uchar) ba[i];
            res += hexchars[s >> 4];
            res += hexchars[s & 0x0f];
        }
        r = "0x" + res;
    } else {
        r = QSqlDriver::formatValue(field, trimStrings);
    }
    return r;
}
