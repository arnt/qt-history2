/****************************************************************************
**
** Implementation of OCI driver classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_oci.h"

#include <qdatetime.h>
#include <qvector.h>
#include <qmemarray.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qcorevariant.h>
#include <qvarlengtharray.h>
#include <stdlib.h>

#define QOCI_DYNAMIC_CHUNK_SIZE  255
static const ub2 CSID_UTF8 = 871; // UTF-8 not defined in Oracle 8 libraries
static const ub1 CSID_NCHAR = SQLCS_NCHAR;

#ifdef OCI_UTF16ID
static const ub2 CSID_UTF16 = OCI_UTF16ID;
#else
static const ub2 CSID_UTF16 = 0;
#endif

typedef QVarLengthArray<sb2, 32> IndicatorArray;

QByteArray qMakeOraDate(const QDateTime& dt);
QDateTime qMakeDate(const char* oraDate);
QString qOraWarn(const QOCIPrivate* d);
void qOraWarning(const char* msg, const QOCIPrivate* d);

class QOCIPrivate
{
public:
    QOCIPrivate();
    ~QOCIPrivate();

    QSqlResult *q;
    OCIEnv *env;
    OCIError *err;
    OCISvcCtx *svc;
    OCIStmt *sql;
    bool transaction;
    int serverVersion;
    bool utf8;  // db charset
    bool nutf8; // national charset
    bool utf16bind;
    QString user;

    text* oraText(const QString& str) const;
    sb4 oraTextLength(const QString& str) const;
    sb4 oraByteLength(const QString& str) const;
    void setCharset(OCIBind* hbnd);
    int bindValues(QVector<QCoreVariant> &values, IndicatorArray &indicators,
                   QList<QByteArray> &tmpStorage);
    void outValues(QVector<QCoreVariant> &values, IndicatorArray &indicators,
                   QList<QByteArray> &tmpStorage);
    bool isOutValue(int i) const;
};

QOCIPrivate::QOCIPrivate(): q(0), env(0), err(0),
        svc(0), sql(0), transaction(false), serverVersion(-1),
        utf8(false), nutf8(false), utf16bind(false)
{
}

QOCIPrivate::~QOCIPrivate()
{
}

text* QOCIPrivate::oraText(const QString& str) const
{
    if (utf16bind)
        return (text*)str.ucs2();
    return (text*)str.ascii();
}

sb4 QOCIPrivate::oraTextLength(const QString& str) const
{
    if (utf16bind)
        return (sb4)((str.length() + 1) * sizeof(QChar));
    return (sb4)(str.length() + 1);
}

sb4 QOCIPrivate::oraByteLength(const QString& str) const
{
    if (utf16bind)
        return (sb4)((str.length()) * sizeof(QChar));
    return (sb4)str.length();
}

void QOCIPrivate::setCharset(OCIBind* hbnd)
{
    int r = 0;

    Q_ASSERT(hbnd);
#ifdef QOCI_USES_VERSION_9
    if (serverVersion > 8 && !CSID_UTF16) {

        r = OCIAttrSet((void*)hbnd,
                        OCI_HTYPE_BIND,
                        (void*) &CSID_NCHAR,
                        (ub4) 0,
                        (ub4) OCI_ATTR_CHARSET_FORM,
                        err);

#ifdef QT_CHECK_RANGE
        if (r != 0)
            qOraWarning("QOCIPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_FORM: ", this);
#endif
    }
#endif //QOCI_USES_VERSION_9

    const ub2* csid = utf16bind ? &CSID_UTF16 : &CSID_UTF8;
    r = OCIAttrSet((void*)hbnd,
                    OCI_HTYPE_BIND,
                    (void*) &csid,
                    (ub4) 0,
                    (ub4) OCI_ATTR_CHARSET_ID,
                    err);
#ifdef QT_CHECK_RANGE
    if (r != 0)
        qOraWarning("QOCIPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_ID: ", this);
#endif
}

bool QOCIPrivate::isOutValue(int i) const
{
#ifdef QOCI_USES_VERSION_9
    if (serverVersion >= 9) {
        if (((QOCI9Result*)q)->bindValueType(i) & QSql::Out)
            return true;
    } else {
        if (((QOCIResult*)q)->bindValueType(i) & QSql::Out)
            return true;
    }
#else
    if (((QOCIResult*)q)->bindValueType(i) & QSql::Out)
        return true;
#endif
    return false;
}

int QOCIPrivate::bindValues(QVector<QCoreVariant> &values, IndicatorArray &indicators,
                            QList<QByteArray> &tmpStorage)
{
    int r = OCI_SUCCESS;
    for (int i = 0; i < values.count(); ++i) {
        if (isOutValue(i))
            values[i].detach();
        QVariant val = values.at(i);
        void *data = (void*)val.constData();

        //qDebug("binding values: %d, %s", i, values.at(i).toString().ascii());
        OCIBind * hbnd = 0; // Oracle handles these automatically
        sb2 *indPtr = &indicators[i];
        *indPtr = val.isNull() ? -1 : 0;
        //            qDebug("Binding: type: %s utf16: %d holder: %i value: %s",
        // QCoreVariant::typeToName(val.type()), utf16bind, i, val.toString().ascii());
        switch (val.type()) {
            case QCoreVariant::ByteArray:
                r = OCIBindByPos(sql, &hbnd, err,
                                  i + 1,
                                  (dvoid *) ((QByteArray*)data)->constData(),
                                  ((QByteArray*)data)->size(),
                                  SQLT_BIN, (dvoid *) indPtr, (ub2 *) 0, (ub2*) 0,
                                  (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
            break;
            case QCoreVariant::Time:
            case QCoreVariant::Date:
            case QCoreVariant::DateTime: {
                QByteArray ba = qMakeOraDate(values.at(i).toDateTime());
                r = OCIBindByPos(sql, &hbnd, err,
                                  i + 1,
                                  (dvoid *) ba.constData(),
                                  ba.size(),
                                  SQLT_DAT, (dvoid *) indPtr, (ub2 *) 0, (ub2*) 0,
                                  (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
                tmpStorage.append(ba);
                break; }
            case QCoreVariant::Int:
                r = OCIBindByPos(sql, &hbnd, err,
                                  i + 1,
                                  (dvoid *) data,
                                  sizeof(int),
                                  SQLT_INT, (dvoid *) indPtr, (ub2 *) 0, (ub2*) 0,
                                  (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
            break;
            case QCoreVariant::Double:
                r = OCIBindByPos(sql, &hbnd, err,
                                  i + 1,
                                  (dvoid *) data,
                                  sizeof(double),
                                  SQLT_FLT, (dvoid *) indPtr, (ub2 *) 0, (ub2*) 0,
                                  (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
            break;
            case QCoreVariant::String:
            default: {
                QString s = values.at(i).toString();
                if (isOutValue(i)) {
                    QByteArray ba((char*)s.ucs2(), s.capacity() * sizeof(QChar));
                    r = OCIBindByPos(sql, &hbnd, err,
                                    i + 1,
                                    (dvoid *)ba.constData(),
                                    ba.size(),
                                    SQLT_STR, (dvoid *) indPtr, (ub2 *) 0, (ub2*) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
                    tmpStorage.append(ba);
                } else {
                    s.ucs2(); // append 0
                    r = OCIBindByPos(sql, &hbnd, err,
                                    i + 1,
                                    //yes, we cast away the const.
                                    // But Oracle should'nt touch IN values
                                    (dvoid *)s.constData(),
                                    (s.length() + 1) * sizeof(QChar),
                                    SQLT_STR, (dvoid *) indPtr, (ub2 *) 0, (ub2*) 0,
                                    (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
                }
                setCharset(hbnd);
                break; }
        }
        if (r != OCI_SUCCESS)
            qOraWarning("QOCIPrivate::bindValues:", this);
    }
    return r;
}

void QOCIPrivate::outValues(QVector<QCoreVariant> &values, IndicatorArray &indicators,
                            QList<QByteArray> &tmpStorage)
{
    for (int i = 0; i < values.count(); ++i) {

        if (!isOutValue(i))
            continue;

        QCoreVariant::Type typ = values.at(i).type();

        switch(typ) {
            case QCoreVariant::Time:
                values[i] = qMakeDate(tmpStorage.takeFirst()).time();
                break;
            case QCoreVariant::Date:
                values[i] = qMakeDate(tmpStorage.takeFirst()).date();
                break;
            case QCoreVariant::DateTime:
                values[i] = qMakeDate(tmpStorage.takeFirst()).time();
                break;
            case QCoreVariant::String:
                values[i] = QString::fromUcs2((ushort*)tmpStorage.takeFirst().constData());
                break;
            default:
                break; //nothing
        }
        if (indicators[i] == -1) // NULL
            values[i] = QCoreVariant(typ);
    }
}

struct OraFieldInfo
{
    QString           name;
    QCoreVariant::Type type;
    ub1                   oraIsNull;
    ub4                   oraType;
    sb1                   oraScale;
    ub4                   oraLength; // size in bytes
    ub4                   oraFieldLength; // amount of characters
    sb2                   oraPrecision;
};

QString qOraWarn(const QOCIPrivate* d)
{
    sb4 errcode;
    text errbuf[1024];
    errbuf[0] = 0;
    errbuf[1] = 0;

    OCIErrorGet((dvoid *)d->err,
                (ub4) 1,
                (text *)NULL,
                &errcode,
                errbuf,
                (ub4)(sizeof(errbuf)),
                OCI_HTYPE_ERROR);
    if (d->utf16bind)
        return QString::fromUcs2((const unsigned short*)errbuf);
    return QString::fromLocal8Bit((const char*)errbuf);
}

void qOraWarning(const char* msg, const QOCIPrivate* d)
{
    qWarning("%s %s", msg, qOraWarn(d).local8Bit());
}

int qOraErrorNumber(const QOCIPrivate* d)
{
    sb4 errcode;
    OCIErrorGet((dvoid *)d->err,
                (ub4) 1,
                (text *) NULL,
                &errcode,
                NULL,
                0,
                OCI_HTYPE_ERROR);
    return errcode;
}

QSqlError qMakeError(const QString& err, int type, const QOCIPrivate* p)
{
    return QSqlError("QOCI: " + err, qOraWarn(p), type);
}

QCoreVariant::Type qDecodeOCIType(const QString& ocitype, int ocilen, int ociprec, int ociscale)
{
    QCoreVariant::Type type = QCoreVariant::Invalid;
    if (ocitype == "VARCHAR2" || ocitype == "VARCHAR" || ocitype.startsWith("INTERVAL") ||
         ocitype == "CHAR" || ocitype == "NVARCHAR2" || ocitype == "NCHAR")
        type = QCoreVariant::String;
    else if (ocitype == "NUMBER")
        type = QCoreVariant::Int;
    else if (ocitype == "FLOAT")
        type = QCoreVariant::Double;
//    else if (ocitype == "LONG" || ocitype == "NCLOB" || ocitype == "CLOB")
//        type = QCoreVariant::CString;
    else if (ocitype == "RAW" || ocitype == "LONG RAW" || ocitype == "ROWID"
              || ocitype == "CFILE" || ocitype == "BFILE" || ocitype == "BLOB")
        type = QCoreVariant::ByteArray;
    else if (ocitype == "DATE" ||  ocitype.startsWith("TIME"))
        type = QCoreVariant::DateTime;
    else if (ocitype == "UNDEFINED")
        type = QCoreVariant::Invalid;
    if (type == QCoreVariant::Int) {
        if (ocilen == 22 && ociprec == 0 && ociscale == 0)
            type = QCoreVariant::Double;
        if (ociscale > 0)
            type = QCoreVariant::Double;
    }
    if (type == QCoreVariant::Invalid)
        qWarning("qDecodeOCIType: unknown type: %s", ocitype.local8Bit());
    return type;
}

QCoreVariant::Type qDecodeOCIType(int ocitype)
{
    QCoreVariant::Type type = QCoreVariant::Invalid;
    switch (ocitype) {
    case SQLT_STR:
    case SQLT_VST:
    case SQLT_CHR:
    case SQLT_AFC:
    case SQLT_VCS:
    case SQLT_AVC:
    case SQLT_RDD:
    case SQLT_LNG: //???
#ifdef SQLT_INTERVAL_YM
    case SQLT_INTERVAL_YM:
#endif
#ifdef SQLT_INTERVAL_DS
    case SQLT_INTERVAL_DS:
#endif
        type = QCoreVariant::String;
        break;
    case SQLT_INT:
        type = QCoreVariant::Int;
        break;
    case SQLT_FLT:
    case SQLT_NUM:
    case SQLT_VNU:
    case SQLT_UIN:
        type = QCoreVariant::Double;
        break;
    case SQLT_CLOB:
//    case SQLT_LNG:
//        type = QCoreVariant::CString;
//        break;
    case SQLT_VBI:
    case SQLT_BIN:
    case SQLT_LBI:
    case SQLT_LVC:
    case SQLT_LVB:
    case SQLT_BLOB:
    case SQLT_FILE:
    case SQLT_NTY:
    case SQLT_REF:
    case SQLT_RID:
        type = QCoreVariant::ByteArray;
        break;
    case SQLT_DAT:
    case SQLT_ODT:
#ifdef SQLT_TIMESTAMP
    case SQLT_TIMESTAMP:
    case SQLT_TIMESTAMP_TZ:
    case SQLT_TIMESTAMP_LTZ:
#endif
        type = QCoreVariant::DateTime;
        break;
    default:
        type = QCoreVariant::Invalid;
        qWarning("qDecodeOCIType: unknown OCI datatype: %d", ocitype);
        break;
    }
        return type;
}

OraFieldInfo qMakeOraField(const QOCIPrivate* p, OCIParam* param)
{
    OraFieldInfo ofi;
    ub2                colType(0);
    text                *colName = 0;
    ub4                colNameLen(0);
    sb1                colScale(0);
    ub2                colLength(0);
    ub2                colFieldLength(0);
    sb2                colPrecision(0);
    ub1                colIsNull(0);
    int                r(0);
    QCoreVariant::Type type(QCoreVariant::Invalid);

    r = OCIAttrGet((dvoid*)param,
                    OCI_DTYPE_PARAM,
                    &colType,
                    0,
                    OCI_ATTR_DATA_TYPE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    (dvoid**) &colName,
                    (ub4 *) &colNameLen,
                    (ub4) OCI_ATTR_NAME,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    r = OCIAttrGet((dvoid*) param,
    OCI_DTYPE_PARAM,
                    &colLength,
                    0,
                    OCI_ATTR_DATA_SIZE, /* in bytes */
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

#ifdef OCI_ATTR_CHAR_SIZE
    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    &colFieldLength,
                    0,
                    OCI_ATTR_CHAR_SIZE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);
#else
    // for Oracle8.
    colFieldLength = colLength;
#endif

    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    &colPrecision,
                    0,
                    OCI_ATTR_PRECISION,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    r = OCIAttrGet((dvoid*) param,
                    OCI_DTYPE_PARAM,
                    &colScale,
                    0,
                    OCI_ATTR_SCALE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);
    r = OCIAttrGet((dvoid*)param,
                    OCI_DTYPE_PARAM,
                    (dvoid*)&colType,
                    0,
                    OCI_ATTR_DATA_TYPE,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);
    r = OCIAttrGet((dvoid*)param,
                    OCI_DTYPE_PARAM,
                    (dvoid*)&colIsNull,
                    0,
                    OCI_ATTR_IS_NULL,
                    p->err);
    if (r != 0)
        qOraWarning("qMakeOraField:", p);

    type = qDecodeOCIType(colType);
    if (type == QCoreVariant::Int) {
        if (colLength == 22 && colPrecision == 0 && colScale == 0)
            type = QCoreVariant::Double;
        if (colScale > 0)
            type = QCoreVariant::Double;
    }
    if (colType == SQLT_BLOB)
        colLength = 0;

    // colNameLen is length in bytes!!!!
    ofi.name = QString((const QChar*)colName, colNameLen / 2);
    ofi.type = type;
    ofi.oraType = colType;
    ofi.oraFieldLength = colFieldLength;
    ofi.oraLength = colLength;
    ofi.oraScale = colScale;
    ofi.oraPrecision = colPrecision;
    ofi.oraIsNull = colIsNull;

    return ofi;
}


/*! \internal Convert QDateTime to the internal Oracle DATE format
  NB! It does not handle BCE dates.
*/
QByteArray qMakeOraDate(const QDateTime& dt)
{
    QByteArray ba;
    ba.resize(7);
    int year = dt.date().year();
    ba[0]= (year / 100) + 100; // century
    ba[1]= (year % 100) + 100; // year
    ba[2]= dt.date().month();
    ba[3]= dt.date().day();
    ba[4]= dt.time().hour() + 1;
    ba[5]= dt.time().minute() + 1;
    ba[6]= dt.time().second() + 1;
    return ba;
}

QDateTime qMakeDate(const char* oraDate)
{
    int century = oraDate[0];
    if(century >= 100){
        int year    = (unsigned char)oraDate[1];
        year = ((century-100)*100) + (year-100);
        int month = oraDate[2];
        int day   = oraDate[3];
        int hour  = oraDate[4] - 1;
        int min   = oraDate[5] - 1;
        int sec   = oraDate[6] - 1;
        return QDateTime(QDate(year,month,day), QTime(hour,min,sec));
    }
    return QDateTime();
}

class QOCIResultPrivate
{
public:
    QOCIResultPrivate(int size, QOCIPrivate* dp);
    ~QOCIResultPrivate();
    void setCharset(OCIDefine* dfn);
    int readPiecewise(QVector<QCoreVariant> &values, int index = 0);
    int readLOBs(QVector<QCoreVariant> &values, int index = 0);
    void getOraFields(QSqlRecord &rinf);
    char* at(int i);
    int size();
    bool isNull(int i);
    QCoreVariant::Type type(int i);
    int fieldFromDefine(OCIDefine* d);
    OCILobLocator* lobLocator(int i);
    int length(int i);
    QCoreVariant value(int i);

    QVector<QCoreVariant> fs;

private:
    char* create(int position, int size);
    OCILobLocator** createLobLocator(int position, OCIEnv* env);

    class OraFieldInf
    {
    public:
        OraFieldInf(): data(0), len(0), ind(0), typ(QCoreVariant::Invalid), def(0), lob(0) {}
        ~OraFieldInf();
        char *data;
        int len;
        sb2 ind;
        QCoreVariant::Type typ;
        OCIDefine *def;
        OCILobLocator **lob;
    };

    QVector<OraFieldInf> fieldInf;
    QOCIPrivate* d;
};

QOCIResultPrivate::OraFieldInf::~OraFieldInf()
{
    delete [] data;
    if (lob) {
        int r = OCIDescriptorFree((dvoid *)*lob, (ub4) OCI_DTYPE_LOB);
        if (r != 0)
            qWarning("QOCIResultPrivate: Cannot free LOB descriptor");
    }
}

QOCIResultPrivate::QOCIResultPrivate(int size, QOCIPrivate* dp)
    : fs(size), fieldInf(size), d(dp)
{
    ub4 dataSize(0);
    OCIDefine* dfn = 0;
    int r;

    OCIParam* param = 0;
    sb4 parmStatus = 0;
    ub4 count = 1;
    int idx = 0;
    parmStatus = OCIParamGet(d->sql,
                              OCI_HTYPE_STMT,
                              d->err,
                              (void**)&param,
                              count);

    while (parmStatus == OCI_SUCCESS) {
        OraFieldInfo ofi = qMakeOraField(d, param);
        if (ofi.oraType == SQLT_RDD)
            dataSize = 50;
#ifdef SQLT_INTERVAL_YM
#ifdef SQLT_INTERVAL_DS
        else if (ofi.oraType == SQLT_INTERVAL_YM || ofi.oraType == SQLT_INTERVAL_DS)
            // since we are binding interval datatype as string,
            // we are not interested in the number of bytes but characters.
            dataSize = 50;  // magic number
#endif //SQLT_INTERVAL_DS
#endif //SQLT_INTERVAL_YM
        else
            dataSize = ofi.oraLength;
        QCoreVariant::Type type = ofi.type;
        fieldInf[count-1].typ = type;
        switch (type) {
            case QCoreVariant::DateTime:
                r = OCIDefineByPos(d->sql,
                                    &dfn,
                                    d->err,
                                    count,
                                    create(idx, dataSize+1),
                                    dataSize+1,
                                    SQLT_DAT,
                                    (dvoid *) &(fieldInf[idx].ind),
                                    0, 0, OCI_DEFAULT);
            break;
            case QCoreVariant::ByteArray:
                // RAW and LONG RAW fields can't be bound to LOB locators
                if (ofi.oraType == SQLT_BIN) {
                //                    qDebug("binding SQLT_BIN");
                r = OCIDefineByPos(d->sql,
                                    &dfn,
                                    d->err,
                                    count,
                                    create(idx, dataSize),
                                    dataSize,
                                    SQLT_BIN,
                                    (dvoid *) &(fieldInf[idx].ind),
                                    0, 0, OCI_DYNAMIC_FETCH);
            } else if (ofi.oraType == SQLT_LBI) {
                //                    qDebug("binding SQLT_LBI");
                r = OCIDefineByPos(d->sql,
                                    &dfn,
                                    d->err,
                                    count,
                                    0,
                                    SB4MAXVAL,
                                    SQLT_LBI,
                                    (dvoid *) &(fieldInf[idx].ind),
                                    0, 0, OCI_DYNAMIC_FETCH);
            } else {
                //                    qDebug("binding SQLT_BLOB");
                r = OCIDefineByPos(d->sql,
                                    &dfn,
                                    d->err,
                                    count,
                                    createLobLocator(idx, d->env),
                                    (sb4)-1,
                                    SQLT_BLOB,
                                    (dvoid *) &(fieldInf[idx].ind),
                                    0, 0, OCI_DEFAULT);
            }
            break;
            case QCoreVariant::String:
                dataSize += dataSize + sizeof(QChar);
            //qDebug("OCIDefineByPosStr: %d", dataSize);
            r = OCIDefineByPos(d->sql,
                                &dfn,
                                d->err,
                                count,
                                create(idx, dataSize),
                                dataSize,
                                SQLT_STR,
                                (dvoid *) &(fieldInf[idx].ind),
                                0, 0, OCI_DEFAULT);
            if (r == 0)
                setCharset(dfn);
            break;
            default:
            dataSize += ++dataSize; // REMOVE ME
            //qDebug("OCIDefineByPosDef: %d", dataSize);
            r = OCIDefineByPos(d->sql,
                                &dfn,
                                d->err,
                                count,
                                create(idx, dataSize+1),
                                dataSize+1,
                                SQLT_STR,
                                (dvoid *) &(fieldInf[idx].ind),
                                0, 0, OCI_DEFAULT);
            break;
        }
        if (r != 0)
            qOraWarning("QOCIResultPrivate::bind:", d);
        fieldInf[idx].def = dfn;
        ++count;
        ++idx;
        parmStatus = OCIParamGet(d->sql,
                                  OCI_HTYPE_STMT,
                                  d->err,
                                  (void**)&param,
                                  count);
    }
}

QOCIResultPrivate::~QOCIResultPrivate()
{
}

char* QOCIResultPrivate::create(int position, int size)
{
    char* c = new char[size+1];
    // Oracle may not fill fixed width fields
    memset(c, 0, size+1);
    fieldInf[position].data = c;
    fieldInf[position].len = size;
    return c;
}

OCILobLocator** QOCIResultPrivate::createLobLocator(int position, OCIEnv* env)
{
    OCILobLocator** lob = new OCILobLocator*;
    int r = OCIDescriptorAlloc((dvoid *)env,
                                (dvoid **)lob,
                                (ub4)OCI_DTYPE_LOB,
                                (size_t) 0,
                                (dvoid **) 0);
    if (r != 0)
        qWarning("QOCIResultPrivate: Cannot create LOB locator");
    fieldInf[position].lob = lob;
    return lob;
}

void QOCIResultPrivate::setCharset(OCIDefine* dfn)
{
    int r = 0;

    Q_ASSERT(dfn);
#ifdef QOCI_USES_VERSION_9
    if (d->serverVersion > 8 && !CSID_UTF16) {

        r = OCIAttrSet((void*)dfn,
                        OCI_HTYPE_DEFINE,
                        (void*) &CSID_NCHAR,
                        (ub4) 0,
                        (ub4) OCI_ATTR_CHARSET_FORM,
                        d->err);

        if (r != 0)
            qOraWarning("QOCIResultPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_FORM: ", d);
    }
#endif //QOCI_USES_VERSION_9

    if (d->serverVersion > 8) {
        const ub2* csid = d->utf16bind ? &CSID_UTF16 : &CSID_UTF8;
        r = OCIAttrSet((void*)dfn,
                        OCI_HTYPE_DEFINE,
                        (void*) &csid,
                        (ub4) 0,
                        (ub4) OCI_ATTR_CHARSET_ID,
                        d->err);
//        if (r != 0)
//            qOraWarning("QOCIResultPrivate::setCharset: Couldn't set OCI_ATTR_CHARSET_ID: ", d);
    }
}

int QOCIResultPrivate::readPiecewise(QVector<QCoreVariant> &values, int index)
{
    OCIDefine*     dfn;
    ub4            typep;
    ub1            in_outp;
    ub4            iterp;
    ub4            idxp;
    ub1            piecep;
    sword          status;
    text           col [QOCI_DYNAMIC_CHUNK_SIZE+1];
    int            fieldNum = -1;
    int            r = 0;
    bool           nullField;
    for (; ;) {
        r = OCIStmtGetPieceInfo(d->sql, d->err, (dvoid**) &dfn, &typep,
                                 &in_outp, &iterp, &idxp, &piecep);
        if (r != OCI_SUCCESS)
            qOraWarning("OCIResultPrivate::readPiecewise: unable to get piece info:", d);
        fieldNum = fieldFromDefine(dfn);
        int chunkSize = QOCI_DYNAMIC_CHUNK_SIZE;
        nullField = false;
        r  = OCIStmtSetPieceInfo(dfn, OCI_HTYPE_DEFINE,
                                  d->err, (void *)col,
                                  (ub4 *)&chunkSize, piecep, NULL, NULL);
        if (r != OCI_SUCCESS)
            qOraWarning("OCIResultPrivate::readPiecewise: unable to set piece info:", d);
        status = OCIStmtFetch (d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
        if (status == -1) {
            sb4 errcode;
            OCIErrorGet((dvoid *)d->err, (ub4) 1, (text *) NULL,
                        &errcode, NULL, 0,OCI_HTYPE_ERROR);
            switch (errcode) {
                case 1405: /* NULL */
                nullField = true;
                break;
                default:
                qOraWarning("OCIResultPrivate::readPiecewise: unable to fetch next:", d);
                break;
            }
        }
        if (status == OCI_NO_DATA) {
            break;
        }
        if (nullField || !chunkSize) {
            values[fieldNum + index] = QCoreVariant(QCoreVariant::ByteArray);
            fieldInf[fieldNum].ind = -1;
        } else {
            QByteArray ba = values.at(fieldNum + index).toByteArray();
            int sz = ba.size();
            ba.resize(sz + chunkSize);
            memcpy(ba.data() + sz, (char*)col, chunkSize);
            values[fieldNum + index] = ba;
            fieldInf[fieldNum].ind = 0;
        }
        if (status == OCI_SUCCESS_WITH_INFO ||
             status == OCI_NEED_DATA) {
        } else
            break;
    }
    return r;
}

int QOCIResultPrivate::readLOBs(QVector<QCoreVariant> &values, int index)
{
    int r = 0;
    OCILobLocator* lob;
    ub4 amount;
    for (int i = 0; i < size(); ++i) {
        lob = lobLocator(i);
        if (!lob || isNull(i))
            continue;
        r = OCILobGetLength(d->svc, d->err, lob, &amount);
        if (r != 0) {
            qOraWarning("OCIResultPrivate::readLOBs: Can't get size of LOB:", d);
            amount = 0;
        }
        if (amount > 0) {
            QByteArray buf;
            buf.resize(amount);
            // get lob charset ID and tell oracle to transform it
            ub1 csfrm = 0;
            r = OCILobCharSetForm(d->env, d->err, lob, &csfrm);
            if (r != 0) {
                qOraWarning("OCIResultPrivate::readLOBs: Can't get encoding of LOB: ", d);
                csfrm = 0;
            }

            r = OCILobRead(d->svc,
                            d->err,
                            lob,
                            &amount,
                            1,
                            (void*) buf.constData(),
                            (ub4) buf.size(),
                            0, 0,
                            0,
                            csfrm);
            if (r != 0)
                qOraWarning("OCIResultPrivate::readLOBs: Cannot read LOB:", d);
            else
                values[i + index] = buf;
        }
        if (r != 0 || !amount) {
            values[i + index] = QByteArray();
            r = 0; // non-fatal error
        }
    }
    return r;
}

void QOCIResultPrivate::getOraFields(QSqlRecord &rinf)
{
    OCIParam* param = 0;
    ub4 count = 1;
    sb4 parmStatus = OCIParamGet(d->sql,
                                  OCI_HTYPE_STMT,
                                  d->err,
                                  (void**)&param,
                                  count);

    while (parmStatus == OCI_SUCCESS) {
        OraFieldInfo ofi = qMakeOraField(d, param);
        QSqlField inf(ofi.name, ofi.type, (int)ofi.oraIsNull == 0 ? 1 : 0, (int)ofi.oraFieldLength,
                           (int)ofi.oraPrecision, QCoreVariant(), (int)ofi.oraType);
        rinf.append(inf);
        count++;
        parmStatus = OCIParamGet(d->sql,
                                  OCI_HTYPE_STMT,
                                  d->err,
                                  (void**)&param,
                                  count);
    }
}

inline char* QOCIResultPrivate::at(int i)
{
    return fieldInf.at(i).data;
}
inline int QOCIResultPrivate::size()
{
    return fieldInf.size();
}
inline bool QOCIResultPrivate::isNull(int i)
{
//    qDebug("ISNULL %d %d", i, fieldInf.at(i).ind);
    return (fieldInf.at(i).ind == -1);
}
inline QCoreVariant::Type QOCIResultPrivate::type(int i)
{
    return fieldInf.at(i).typ;
}
inline int QOCIResultPrivate::fieldFromDefine(OCIDefine* d)
{
    for (int i = 0; i < fieldInf.count(); ++i) {
        if (fieldInf.at(i).def == d)
            return i;
    }
    return -1;
}
OCILobLocator* QOCIResultPrivate::lobLocator(int i)
{
    OCILobLocator** lob = fieldInf.at(i).lob;
    if (!lob)
        return 0;
    return *lob;
}
inline int QOCIResultPrivate::length(int i)
{
    return fieldInf.at(i).len;
}
QCoreVariant QOCIResultPrivate::value(int i)
{
    QCoreVariant v;
    switch (type(i)) {
        case QCoreVariant::DateTime:
            v = QCoreVariant(qMakeDate(at(i)));
        break;
        case QCoreVariant::String:
        case QCoreVariant::Double: // when converted to strings
        case QCoreVariant::Int:    // keep these as strings so that we do not lose precision
            if (d->utf16bind)
                v = QCoreVariant(QString::fromUcs2((const short unsigned int*)at(i)));
            else
                v = QCoreVariant(QString::fromUtf8(at(i)));
        break;
        case QCoreVariant::ByteArray: {
            int len = length(i);
            if (len > 0)
                return QByteArray(at(i), len);
            return QCoreVariant(QByteArray());
        break;
        }
        default:
        qWarning("QOCIResultPrivate::value: unknown data type");
        break;
    }
    return v;
}


////////////////////////////////////////////////////////////////////////////

QOCIResult::QOCIResult(const QOCIDriver * db, QOCIPrivate* p)
: QtSqlCachedResult(db),
  cols(0)
{
    d = new QOCIPrivate();
    (*d) = (*p);
    d->q = this;
}

QOCIResult::~QOCIResult()
{
    if (d->sql) {
        int r = OCIHandleFree(d->sql, OCI_HTYPE_STMT);
        if (r != 0)
            qOraWarning("~QOCIResult: unable to free statement handle:", d);
    }
    delete d;
    delete cols;
}

OCIStmt* QOCIResult::statement()
{
    return d->sql;
}

bool QOCIResult::reset (const QString& query)
{
    if (!prepare(query))
        return false;
    return exec();
}

bool QOCIResult::gotoNext(QtSqlCachedResult::ValueCache &values, int index)
{
    if (at() == QSql::AfterLast)
        return false;
    int r = 0;
    r = OCIStmtFetch (d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);

    if (r == OCI_SUCCESS_WITH_INFO) {
        qOraWarning("QOCIResult::gotoNext: ", d);
        r = 0; //ignore it
    } else if (r == OCI_NEED_DATA) { /* piecewise */
        if (index < 0)
            r = cols->readPiecewise(cols->fs);
        else
            r = cols->readPiecewise(values, index);
    }
    if(r == OCI_ERROR) {
        switch (qOraErrorNumber(d)) {
        case 1406:
            qWarning("QOCI Warning: data truncated for %s", lastQuery().local8Bit());
            r = 0; /* ignore it */
            break;
        default:
            qOraWarning("QOCIResult::gotoNext: ", d);
        }
    }
    if (index < 0) //not interested in values
        return r == 0;
    // fetch LOBs
    if (r == 0)
        r = cols->readLOBs(values, index);
    if (r == 0) {
        for (int i = 0; i < cols->size(); ++i) {
            if (cols->isNull(i))
                values[i + index] = QCoreVariant(cols->type(i));
            else if (values.at(i + index) == QVariant::Invalid)
                values[i + index] = cols->value(i);
        }
    } else {
        setAt(QSql::AfterLast);
    }
    return r == 0;
}

int QOCIResult::size()
{
    return -1;
}

int QOCIResult::numRowsAffected()
{
    int rowCount;
    OCIAttrGet(d->sql,
                OCI_HTYPE_STMT,
                &rowCount,
                NULL,
                OCI_ATTR_ROW_COUNT,
                d->err);
    return rowCount;
}

bool QOCIResult::prepare(const QString& query)
{
    int r = 0;

    delete cols;
    cols = 0;
    QtSqlCachedResult::cleanup();

    if (d->sql) {
        r = OCIHandleFree(d->sql, OCI_HTYPE_STMT);
        if (r != 0)
            qOraWarning("QOCIResult::prepare: unable to free statement handle:", d);
    }
    if (query.isEmpty())
        return false;
    r = OCIHandleAlloc((dvoid *) d->env,
                        (dvoid **) &d->sql,
                        OCI_HTYPE_STMT,
                        0,
                        0);
    if (r != 0) {
        qOraWarning("QOCIResult::prepare: unable to alloc statement:", d);
        setLastError(qMakeError("Unable to alloc statement", QSqlError::Statement, d));
        return false;
    }
    r = OCIStmtPrepare(d->sql,
                        d->err,
                        (OraText*)query.unicode(),
                        query.length() * sizeof(QChar),
                        OCI_NTV_SYNTAX,
                        OCI_DEFAULT);
    if (r != 0) {
        qOraWarning("QOCIResult::prepare: unable to prepare statement:", d);
        setLastError(qMakeError("Unable to prepare statement", QSqlError::Statement, d));
        return false;
    }
    // do something with the placeholders? into a map?
    return true;
}

bool QOCIResult::exec()
{
    int r = 0;
    ub2 stmtType;
    QList<QByteArray> tmpStorage;
    IndicatorArray indicators(boundValueCount());

//    QtSqlCachedResult::clear();

    // bind placeholders
    if (boundValueCount() > 0
         && d->bindValues(boundValues(), indicators, tmpStorage) != OCI_SUCCESS) {
        qOraWarning("QOCIResult::exec: unable to bind value: ", d);
        setLastError(qMakeError("Unable to bind value", QSqlError::Statement, d));
        return false;
    }

    r = OCIAttrGet(d->sql,
                    OCI_HTYPE_STMT,
                    (dvoid*)&stmtType,
                    NULL,
                    OCI_ATTR_STMT_TYPE,
                    d->err);
    // execute
    if (stmtType == OCI_STMT_SELECT)
    {
        r = OCIStmtExecute(d->svc,
                            d->sql,
                            d->err,
                            0,
                            0,
                            (CONST OCISnapshot *) NULL,
                            (OCISnapshot *) NULL,
                            OCI_DEFAULT);
        if (r != 0) {
            qOraWarning("QOCIResult::exec: unable to execute select statement:", d);
            setLastError(qMakeError("Unable to execute select statement", QSqlError::Statement, d));
            return false;
        }
        ub4 parmCount = 0;
        int r = OCIAttrGet(d->sql, OCI_HTYPE_STMT, (dvoid*)&parmCount, NULL, OCI_ATTR_PARAM_COUNT, d->err);
        if (r == 0 && !cols)
            cols = new QOCIResultPrivate(parmCount, d);
        setSelect(true);
        QtSqlCachedResult::init(parmCount);
    } else { /* non-SELECT */
        r = OCIStmtExecute(d->svc, d->sql, d->err, 1,0,
                                (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL,
                                d->transaction ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS );
        if (r != 0) {
            qOraWarning("QOCIResult::exec: unable to execute statement:", d);
            setLastError(qMakeError("Unable to execute statement", QSqlError::Statement, d));
            return false;
        }
        setSelect(false);
    }
    setAt(QSql::BeforeFirst);
    setActive(true);

    if (hasOutValues())
        d->outValues(boundValues(), indicators, tmpStorage);

    return true;
}

QSqlRecord QOCIResult::record() const
{
    QSqlRecord inf;
    if (!isActive() || !isSelect() || !cols)
        return inf;
    cols->getOraFields(inf);
    return inf;
}

////////////////////////////////////////////////////////////////////////////

#ifdef QOCI_USES_VERSION_9
QOCI9Result::QOCI9Result(const QOCIDriver * db, QOCIPrivate* p)
: QSqlResult(db),
  cols(0)
{
    d = new QOCIPrivate();
    (*d) = (*p);
    d->q = this;
}

QOCI9Result::~QOCI9Result()
{
    if (d->sql) {
        int r = OCIHandleFree(d->sql,OCI_HTYPE_STMT);
        if (r != 0)
            qOraWarning("~QOCI9Result: unable to free statement handle: ", d);
    }
    delete d;
    delete cols;
}

OCIStmt* QOCI9Result::statement()
{
    return d->sql;
}

bool QOCI9Result::reset (const QString& query)
{
    if (!prepare(query))
        return false;
    return exec();
}

bool QOCI9Result::cacheNext(int r)
{
    cols->fs.fill(QCoreVariant());
    if (r == OCI_SUCCESS_WITH_INFO) {
        qOraWarning("QOCI9Result::cacheNext:", d);
        r = 0; //ignore it
    } else if (r == OCI_NEED_DATA) { /* piecewise */
        r = cols->readPiecewise(cols->fs);
    }
    if(r == OCI_ERROR) {
        switch (qOraErrorNumber(d)) {
        case 1406:
            qWarning("QOCI Warning: data truncated for %s", lastQuery().local8Bit());
            r = 0; /* ignore it */
            break;
        default:
            qOraWarning("QOCI9Result::cacheNext: ", d);
        }
    }
    // fetch LOBs
    if (r == 0)
        r = cols->readLOBs(cols->fs);
    if (r == 0) {
        for (int i = 0; i < cols->size(); ++i) {
            if (cols->fs.at(i).type() == QCoreVariant::Invalid && !cols->isNull(i))
                cols->fs[i] = cols->value(i);
        }
    } else {
        setAt(QSql::AfterLast);
    }
    return r == 0;
}

bool QOCI9Result::fetchNext()
{
    int r;
    if (!isForwardOnly()) {
        r = OCIStmtFetch2 (d->sql, d->err, 1, OCI_FETCH_NEXT, (sb4) 1, OCI_DEFAULT);
    } else {
        r = OCIStmtFetch (d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT);
    }
    if (cacheNext(r)) {
        setAt(at() + 1);
        return true;
    }
    return false;
}

bool QOCI9Result::fetch(int i)
{
    if (!isForwardOnly()) {
        int r = OCIStmtFetch2(d->sql, d->err, 1, OCI_FETCH_ABSOLUTE, (sb4) i + 1, OCI_DEFAULT);
        if (cacheNext(r)) {
            setAt(i);
            return true;
        }
        return false;
    } else {
        while (at() < i) {
            if (!fetchNext())
                return false;
        }
        if (at() == i) {
            return true;
        }
    }
    return false;
}

bool QOCI9Result::fetchFirst()
{
    if (isForwardOnly()) {
        if (at() == QSql::BeforeFirst)
            return fetchNext();
    } else {
        int r = OCIStmtFetch2(d->sql, d->err, 1, OCI_FETCH_FIRST, (sb4) 1, OCI_DEFAULT);
        if (cacheNext(r)) {
            setAt(0);
            return true;
        }
    }
    return false;
}

bool QOCI9Result::fetchLast()
{
    if (isForwardOnly()) {
        int i = at();
        while (fetchNext())
            i++; /* brute force */
        if (at() == QSql::AfterLast) {
            setAt(i);
            return true;
        }
    } else {
        int r = OCIStmtFetch2(d->sql, d->err, 1, OCI_FETCH_LAST, (sb4) 0, OCI_DEFAULT);
        if (cacheNext(r)) {
            ub4 currentPos;
            ub4 sz = sizeof(currentPos);
            r = OCIAttrGet((CONST void *) d->sql,
                            OCI_HTYPE_STMT,
                            (void *) &currentPos,
                            (ub4 *) &sz,
                            OCI_ATTR_CURRENT_POSITION,
                            d->err);
            if (r != 0) {
                qWarning("QOCI9Result::fetchLast(): Cannot get current position");
                setAt(QSql::AfterLast);
                return false;
            }
            setAt(currentPos - 1);
            return true;
        }
    }
    return false;
}

bool QOCI9Result::fetchPrev()
{
    if (!isForwardOnly()) {
        int r = OCIStmtFetch2 (d->sql, d->err, 1, OCI_FETCH_PRIOR, (sb4) 1, OCI_DEFAULT);
        if (cacheNext(r)) {
            setAt(at() - 1);
            return true;
        }
    }
    return false;
}

QCoreVariant QOCI9Result::data(int field)
{
    if (field < cols->fs.count())
        return cols->fs.at(field);
    qWarning("QOCI9Result::data: column %d out of range", field);
    return QCoreVariant();
}

bool QOCI9Result::isNull(int field)
{
    if (field < cols->fs.count())
        return cols->isNull(field);
    qWarning("QOCI9Result::isNull: column %d out of range", field);
    return true;
}

int QOCI9Result::size()
{
    return -1;
}

int QOCI9Result::numRowsAffected()
{
    int rowCount;
    OCIAttrGet(d->sql,
                OCI_HTYPE_STMT,
                &rowCount,
                NULL,
                OCI_ATTR_ROW_COUNT,
                d->err);
    return rowCount;
}

bool QOCI9Result::prepare(const QString& query)
{
//    qDebug("QOCI9Result::prepare: %s", query.ascii());

    int r = 0;
    delete cols;
    cols = 0;

    if (d->sql) {
        r = OCIHandleFree(d->sql, OCI_HTYPE_STMT);
        if (r != 0)
            qOraWarning("QOCI9Result::reset: unable to free statement handle: ", d);
    }
    if (query.isNull() || query.length() == 0)
        return false;
    r = OCIHandleAlloc((dvoid *) d->env,
                        (dvoid **) &d->sql,
                        OCI_HTYPE_STMT,
                        0,
                        0);
    if (r != 0) {
        qOraWarning("QOCI9Result::reset: unable to alloc statement: ", d);
        return false;
    }
    r = OCIStmtPrepare(d->sql,
                        d->err,
                        (const OraText*)query.unicode(),
                        query.length() * sizeof(QChar),
                        OCI_NTV_SYNTAX,
                        OCI_DEFAULT);
    if (r != 0) {
        qOraWarning("QOCI9Result::reset: unable to prepare statement: ", d);
        return false;
    }
    return true;
}

bool QOCI9Result::exec()
{
    int r = 0;
    ub2 stmtType;
    QList<QByteArray> tmpStorage;
    IndicatorArray indicators(boundValueCount());

//    qDebug("QOCI9Result::exec: %s", executedQuery().ascii());

    // bind placeholders
    if (boundValueCount() > 0
        && d->bindValues(boundValues(), indicators, tmpStorage) != OCI_SUCCESS) {
        qOraWarning("QOCIResult::exec: unable to bind value: ", d);
        setLastError(qMakeError("Unable to bind value", QSqlError::Statement, d));
        return false;
    }

    r = OCIAttrGet(d->sql,
                    OCI_HTYPE_STMT,
                    (dvoid*)&stmtType,
                    NULL,
                    OCI_ATTR_STMT_TYPE,
                    d->err);
    if (stmtType == OCI_STMT_SELECT)
    {
        ub4 mode = OCI_STMT_SCROLLABLE_READONLY;
        if (isForwardOnly()) {
            mode = OCI_DEFAULT;
        }
        r = OCIStmtExecute(d->svc,
                            d->sql,
                            d->err,
                            0,
                            0,
                            (CONST OCISnapshot *) NULL,
                            (OCISnapshot *) NULL,
                            mode);
        if (r != 0) {
            qOraWarning("QOCI9Result::reset: unable to execute select statement: ", d);
            setLastError(qMakeError("Unable to execute select statement", QSqlError::Statement, d));
            return false;
        }
        ub4 parmCount = 0;
        int r = OCIAttrGet(d->sql, OCI_HTYPE_STMT, (dvoid*)&parmCount, NULL, OCI_ATTR_PARAM_COUNT, d->err);
        if (r == 0 && !cols)
            cols = new QOCIResultPrivate(parmCount, d);
        setSelect(true);
    } else { /* non-SELECT */
        r = OCIStmtExecute(d->svc, d->sql, d->err, 1, 0,
                                (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL,
                                d->transaction ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS);
        if (r != 0) {
            qOraWarning("QOCI9Result::reset: unable to execute statement: ", d);
            setLastError(qMakeError("Unable to execute statement", QSqlError::Statement, d));
            return false;
        }
        setSelect(false);
    }
    setAt(QSql::BeforeFirst);
    setActive(true);

    if (hasOutValues())
        d->outValues(boundValues(), indicators, tmpStorage);

    return true;
}

QSqlRecord QOCI9Result::record() const
{
    QSqlRecord inf;
    if (!isActive() || !isSelect() || !cols)
        return inf;
    cols->getOraFields(inf);
    return inf;
}

#endif //QOCI_USES_VERSION_9
////////////////////////////////////////////////////////////////////////////


QOCIDriver::QOCIDriver(QObject* parent)
    : QSqlDriver(parent)
{
    init();
}

QOCIDriver::QOCIDriver(OCIEnv* env, OCIError* err, OCISvcCtx* ctx, QObject* parent)
    : QSqlDriver(parent)
{
    d = new QOCIPrivate();
    d->env = env;
    d->err = err;
    d->svc = ctx;
    if (env && err && ctx) {
        setOpen(true);
        setOpenError(false);
    }
}

void QOCIDriver::init()
{
    d = new QOCIPrivate();
#ifdef QOCI_USES_VERSION_9
    int r = OCIEnvCreate(&d->env,
                            OCI_UTF16 | OCI_OBJECT,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            0,
                            NULL);
    d->utf16bind = true;
#else
    // this call is deprecated in Oracle >= 8.1.x, but still works
    int r = OCIInitialize(OCI_DEFAULT | OCI_OBJECT,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (r != 0)
        qOraWarning("QOCIDriver: unable to initialize environment:", d);
    r = OCIEnvInit(&d->env,
                    OCI_DEFAULT,
                    0,
                    NULL);
    d->utf16bind = false;
#endif  //QOCI_USES_VERSION_9
    if (r != 0)
        qOraWarning("QOCIDriver: unable to create environment:", d);
    r = OCIHandleAlloc((dvoid *) d->env,
                        (dvoid **) &d->err,
                        OCI_HTYPE_ERROR,
                        (size_t) 0,
                        (dvoid **) 0);
    if (r != 0)
        qOraWarning("QOCIDriver: unable to alloc error handle:", d);
    r = OCIHandleAlloc((dvoid *) d->env,
                        (dvoid **) &d->svc,
                        OCI_HTYPE_SVCCTX,
                        (size_t) 0,
                        (dvoid **) 0);
    if (r != 0)
        qOraWarning("QOCIDriver: unable to alloc service context:", d);
    if (r != 0)
        setLastError(qMakeError("Unable to initialize", QSqlError::Connection, d));
}

QOCIDriver::~QOCIDriver()
{
    cleanup();
    delete d;
}

bool QOCIDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
        return true;
    case QuerySize:
        return false;
    case BLOB:
        return true;
    case Unicode:
        return d->serverVersion >= 9;
    case PreparedQueries:
        return true;
    case NamedPlaceholders:
        return true;
    default:
        return false;
    }
}

bool QOCIDriver::open(const QString & db,
                       const QString & user,
                       const QString & password,
                       const QString & ,
                       int,
                       const QString &)
{
    if (isOpen())
        close();
    int r = OCILogon(       d->env,
                        d->err,
                        &d->svc,
                        (OraText*) user.unicode(),
                        user.length() * sizeof(QChar),
                        (OraText*)password.unicode(),
                        password.length() * sizeof(QChar),
                        (OraText*)db.unicode(),
                        db.length() * sizeof(QChar));
    if (r != 0) {
        setLastError(qMakeError("Unable to logon", QSqlError::Connection, d));
        setOpenError(true);
        return false;
    }

    // get server version
    text vertxt[512];
    r = OCIServerVersion(d->svc,
                          d->err,
                          vertxt,
                          sizeof(vertxt),
                          OCI_HTYPE_SVCCTX);
    if (r != 0) {
#ifdef QT_CHECKRANGE
        qWarning("QOCIDriver::open: could not get Oracle server version.");
#endif
    } else {
        QString versionStr = d->utf16bind ? QString::fromUcs2((unsigned short*)vertxt)
                            : QString::fromUtf8((char*)vertxt, sizeof(vertxt));
        QRegExp vers("([0-9]+)\\.[0-9\\.]+[0-9]");
        if (vers.search(versionStr) >= 0)
            d->serverVersion = vers.cap(1).toInt();
        if (d->serverVersion == 0)
            d->serverVersion = -1;
}
    setOpen(true);
    setOpenError(false);
    d->user = user.toUpper();

    QSqlQuery q = createQuery();
    q.setForwardOnly(true);
    if (q.exec("select parameter, value from nls_database_parameters "
                 "where parameter = 'NLS_CHARACTERSET' "
                 "or parameter = 'NLS_NCHAR_CHARACTERSET'")) {
        while (q.next()) {
            // qDebug("NLS: " + q.value(0).toString()); // ###
            if (q.value(0).toString() == "NLS_CHARACTERSET" &&
                 q.value(1).toString().toUpper().startsWith("UTF8")) {
                d->utf8 = true;
            } else if (q.value(0).toString() == "NLS_NCHAR_CHARACTERSET" &&
                 q.value(1).toString().toUpper().startsWith("UTF8")) {
                d->nutf8 = true;
            }
        }
    } else {
#ifdef QT_CHECKRANGE
        qWarning("QOCIDriver::open: could not get Oracle server character set.");
#endif
    }

    return true;
}

void QOCIDriver::close()
{
    OCILogoff(d->svc, d->err);
    setOpen(false);
    setOpenError(false);
}

void QOCIDriver::cleanup()
{
    if (isOpen()) {
        OCILogoff(d->svc, d->err);
    }
    OCIHandleFree((dvoid *) d->svc, OCI_HTYPE_SVCCTX);
    OCIHandleFree((dvoid *) d->err, OCI_HTYPE_ERROR);
    OCIHandleFree((dvoid *) d->env, OCI_HTYPE_ENV);
}

QSqlQuery QOCIDriver::createQuery() const
{
#ifdef QOCI_USES_VERSION_9
    if (d->serverVersion >= 9)
        return QSqlQuery(new QOCI9Result(this, d));
#endif
    return QSqlQuery(new QOCIResult(this, d));
}

bool QOCIDriver::beginTransaction()
{
    if (!isOpen()) {
        qWarning("QOCIDriver::beginTransaction: Database not open");
        return false;
    }
    d->transaction = true;
    int r = OCITransStart (d->svc,
                            d->err,
                            2,
                            OCI_TRANS_READWRITE);
    if (r == OCI_ERROR) {
        qOraWarning("QOCIDriver::beginTransaction: ", d);
        return false;
    }
    return true;
}

bool QOCIDriver::commitTransaction()
{
    if (!isOpen()) {
        qWarning("QOCIDriver::commitTransaction: Database not open");
        return false;
    }
    d->transaction = false;
    int r = OCITransCommit (d->svc,
                             d->err,
                             0);
    if (r == OCI_ERROR) {
        qOraWarning("QOCIDriver::commitTransaction:", d);
        return false;
    }
    return true;
}

bool QOCIDriver::rollbackTransaction()
{
    if (!isOpen()) {
        qWarning("QOCIDriver::rollbackTransaction: Database not open");
        return false;
    }
    d->transaction = false;
    int r = OCITransRollback (d->svc,
                               d->err,
                               0);
    if (r == OCI_ERROR) {
        qOraWarning("QOCIDriver::rollbackTransaction:", d);
        return false;
    }
    return true;
}

QStringList QOCIDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;

    QSqlQuery t = createQuery();
    t.setForwardOnly(true);
    if (type & QSql::Tables) {
        t.exec("select owner, table_name from all_tables "
                "where owner != 'MDSYS' "
                "and owner != 'LBACSYS' "
                "and owner != 'SYS' "
                "and owner != 'SYSTEM' "
                "and owner != 'WKSYS'"
                "and owner != 'CTXSYS'"
                "and owner != 'WMSYS'");
        while (t.next()) {
            if (t.value(0).toString() != d->user)
                tl.append(t.value(0).toString() + "." + t.value(1).toString());
            else
                tl.append(t.value(1).toString());
        }
    }
    if (type & QSql::Views) {
        t.exec("select owner, view_name from all_views "
                "where owner != 'MDSYS' "
                "and owner != 'LBACSYS' "
                "and owner != 'SYS' "
                "and owner != 'SYSTEM' "
                "and owner != 'WKSYS'"
                "and owner != 'CTXSYS'"
                "and owner != 'WMSYS'");
        while (t.next()) {
            if (t.value(0).toString() != d->user)
                tl.append(t.value(0).toString() + "." + t.value(1).toString());
            else
                tl.append(t.value(1).toString());
        }
    }
    if (type & QSql::SystemTables) {
        t.exec("select table_name from dictionary");
        while (t.next()) {
            tl.append(t.value(0).toString());
        }
    }
    return tl;
}

void qSplitTableAndOwner(const QString & tname, QString * tbl,
                          QString * owner)
{
    int i = tname.indexOf('.'); // prefixed with owner?
    if (i != -1) {
        *tbl = tname.right(tname.length() - i - 1).toUpper();
        *owner = tname.left(i).toUpper();
    } else {
        *tbl = tname.toUpper();
    }
}

QSqlRecord QOCIDriver::record(const QString& tablename) const
{
//    qDebug("*** recordInfo QString");
    QSqlRecord fil;
    if (!isOpen())
        return fil;

    QSqlQuery t = createQuery();
    // using two separate queries for this is A LOT faster than using
    // eg. a sub-query on the sys.synonyms table
    QString stmt("select column_name, data_type, data_length, "
                  "data_precision, data_scale, nullable, data_default%1"
                  "from all_tab_columns "
                  "where table_name=%2");
    if (d->serverVersion >= 9)
        stmt = stmt.arg(", char_length ");
    else
        stmt = stmt.arg(" ");
    bool buildRecordInfo = false;
    QString table, owner, tmpStmt;
    qSplitTableAndOwner(tablename, &table, &owner);
    tmpStmt = stmt.arg("'" + table + "'");
    if (owner.isEmpty()) {
        owner = d->user;
    }
    tmpStmt += " and owner='" + owner + "'";
    t.setForwardOnly(true);
    t.exec(tmpStmt);
    if (!t.next()) { // try and see if the tablename is a synonym
        stmt= stmt.arg("(select tname from sys.synonyms where sname='"
                        + table + "' and creator=owner)");
        t.setForwardOnly(true);
        t.exec(stmt);
        if (t.next())
            buildRecordInfo = true;
    } else {
        buildRecordInfo = true;
    }
    if (buildRecordInfo) {
        do {
            QCoreVariant::Type ty = qDecodeOCIType(t.value(1).toString(), t.value(2).toInt(),
                            t.value(3).toInt(), t.value(4).toInt());
            bool required = t.value(5).toString() == "N";
            int prec = -1;
            if (!t.isNull(3)) {
                prec = t.value(3).toInt();
            }
            int size = t.value(2).toInt();
            if (d->serverVersion >= 9 && (ty == QCoreVariant::String)) {
                // Oracle9: data_length == size in bytes, char_length == amount of characters
                size = t.value(7).toInt();
            }
            QSqlField f(t.value(0).toString(), ty, required, size, prec, t.value(6));
            fil.append(f);
        } while (t.next());
    }
    return fil;
}

QSqlIndex QOCIDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex idx(tablename);
    if (!isOpen())
        return idx;
    QSqlQuery t = createQuery();
    QString stmt("select b.column_name, b.index_name, a.table_name, a.owner "
                  "from all_constraints a, all_ind_columns b "
                  "where a.constraint_type='P' "
                  "and b.index_name = a.constraint_name "
                  "and b.index_owner = a.owner");

    bool buildIndex = false;
    QString table, owner, tmpStmt;
    qSplitTableAndOwner(tablename, &table, &owner);
    tmpStmt = stmt + " and a.table_name='" + table + "'";
    if (owner.isEmpty()) {
        owner = d->user;
    }
    tmpStmt += " and a.owner='" + owner + "'";
    t.setForwardOnly(true);
    t.exec(tmpStmt);

    if (!t.next()) {
        stmt += " and a.table_name=(select tname from sys.synonyms "
                "where sname='" + table + "' and creator=a.owner)";
        t.setForwardOnly(true);
        t.exec(stmt);
        if (t.next()) {
            owner = t.value(3).toString();
            buildIndex = true;
        }
    } else {
        buildIndex = true;
    }
    if (buildIndex) {
        QSqlQuery tt = createQuery();
        tt.setForwardOnly(true);
        idx.setName(t.value(1).toString());
        do {
            tt.exec("select data_type from all_tab_columns where table_name='" +
                     t.value(2).toString() + "' and column_name='" +
                     t.value(0).toString() + "' and owner='" + owner + "'");
            if (!tt.next()) {
                return QSqlIndex();
            }
            QSqlField f(t.value(0).toString(), qDecodeOCIType(tt.value(0).toString(), 0, 0, 0));
            idx.append(f);
        } while (t.next());
        return idx;
    }
    return QSqlIndex();
}

QString QOCIDriver::formatValue(const QSqlField* field, bool) const
{
    switch (field->type()) {
    case QCoreVariant::String: {
        if (d->serverVersion >= 9) {
            QString encStr = "UNISTR('";
            const QString srcStr = field->value().toString();
            for (int i = 0; i < srcStr.length(); ++i) {
                encStr += '\\' + QString::number(srcStr.at(i).unicode(), 16).rightJustified(4, '0');
            }
            encStr += "')";
            return encStr;
        } else {
            return QSqlDriver::formatValue(field);
        }
        break;
    }
    case QCoreVariant::DateTime: {
        QDateTime datetime = field->value().toDateTime();
        QString datestring;
        if (datetime.isValid()) {
            datestring = "TO_DATE('" + QString::number(datetime.date().year()) + "-" + \
                                 QString::number(datetime.date().month()) + "-" + \
                                 QString::number(datetime.date().day()) + " " + \
                                 QString::number(datetime.time().hour()) + ":" + \
                                 QString::number(datetime.time().minute()) + ":" + \
                                 QString::number(datetime.time().second()) + "',"
                                 "'YYYY-MM-DD HH24:MI:SS')";
        } else {
            datestring = "NULL";
        }
        return datestring;
        break;
    }
    case QCoreVariant::Date: {
        QDate date = field->value().toDate();
        QString datestring;
        if (date.isValid()) {
            datestring = "TO_DATE('" + QString::number(date.year()) + "-" + \
                                 QString::number(date.month()) + "-" + \
                                 QString::number(date.day()) + "',"
                                 "'YYYY-MM-DD')";
        } else {
            datestring = "NULL";
        }
        return datestring;
        break;
    }
    default:
        break;
    }
    return QSqlDriver::formatValue(field);
}

OCIEnv* QOCIDriver::environment()
{
    return d->env;
}

OCISvcCtx* QOCIDriver::serviceContext()
{
    return d->svc;
}
