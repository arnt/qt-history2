/****************************************************************************
**
** Implementation of OCI driver classes
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsql_oci.h"

#include <qdatetime.h>
#include <qvector.h>
#include <qarray.h>
#include <qstringlist.h>
#include <stdlib.h>

#define QOCI_DYNAMIC_CHUNK_SIZE  255
#define QOCI_BYTEARRAY_SIZE      255

class QOCIPrivate
{
public:
    QOCIPrivate()
	: env(0), err(0), svc(0), sql(0), transaction( FALSE )
    {}
    OCIEnv           *env;
    OCIError         *err;
    OCISvcCtx        *svc;
    OCIStmt          *sql;
    bool             transaction;
};

struct OraFieldInfo
{
    QString        name;
    QVariant::Type type;
    ub4            oraType;
    sb1	   oraScale;
    ub4            oraLength;
    sb2	   oraPrecision;
};

QString qOraWarn( const QOCIPrivate* d)
{
    unsigned char   errbuf[100];
    int             errcode;
    OCIErrorGet((dvoid *)d->err,
		(ub4) 1,
		(text *) NULL,
		&errcode,
		errbuf,
		(ub4) sizeof(errbuf),
		OCI_HTYPE_ERROR);
    return QString( (char*)errbuf );
}

QSqlError qMakeError( const QString& err, int type, const QOCIPrivate* p )
{
    return QSqlError("QOCI: " + err, qOraWarn(p), type );
}

QVariant::Type qDecodeOCIType( const QString& ocitype, int ocilen, int ociprec, int ociscale )
{
    QVariant::Type type = QVariant::Invalid;
    if ( ocitype == "VARCHAR2" || ocitype == "VARCHAR" || ocitype == "CHAR" || ocitype == "NVARCHAR2" )
	type = QVariant::String;
    else if ( ocitype == "NUMBER" )
	type = QVariant::Int;
    else if ( ocitype == "FLOAT" )
	type = QVariant::Double;
    else if ( ocitype == "LONG" )
	type = QVariant::CString;
    else if ( ocitype == "RAW" || ocitype == "LONG RAW" || ocitype == "ROWID" || ocitype == "NCLOB" || ocitype == "CLOB" || ocitype == "CFILE" || ocitype == "BFILE" || ocitype == "BLOB" )
	type = QVariant::ByteArray;
    else if ( ocitype == "DATE" )
	type = QVariant::DateTime;
    else if ( ocitype.mid(0,4) == "TIME" )
	type = QVariant::Time;
    else if ( ocitype == "UNDEFINED" )
	type = QVariant::Invalid;
    if ( type == QVariant::Int ) {
	if ( ocilen == 22 && ociprec == 0 && ociscale == 0 )
	    type = QVariant::Double;
	if ( ociscale > 0 )
	    type = QVariant::Double;
    }
    if ( type == QVariant::Invalid )
	qWarning("qDecodeOCIType: unknown type:" + ocitype );
    return type;
}

QVariant::Type qDecodeOCIType( int ocitype )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( ocitype ) {
    case SQLT_STR:
    case SQLT_VST:
    case SQLT_CHR:
    case SQLT_AFC:
    case SQLT_VCS:
    case SQLT_AVC:
	type = QVariant::String;
	break;
    case SQLT_INT:
	type = QVariant::Int;
	break;
    case SQLT_FLT:
    case SQLT_NUM:
    case SQLT_VNU:
    case SQLT_UIN:
	type = QVariant::Double;
	break;
    case SQLT_LNG:
	type = QVariant::CString;
	break;
    case SQLT_VBI:
    case SQLT_BIN:
    case SQLT_LBI:
    case SQLT_LVC:
    case SQLT_LVB:
    case SQLT_BLOB:
    case SQLT_CLOB:
    case SQLT_FILE:
    case SQLT_RDD:
    case SQLT_NTY:
    case SQLT_REF:
    case SQLT_RID:
	type = QVariant::ByteArray;
	break;
    case SQLT_DAT:
    case SQLT_ODT:
	type = QVariant::DateTime;
	break;
    default:
		type = QVariant::Invalid;
	break;
    }
	return type;
}

OraFieldInfo qMakeOraField( const QOCIPrivate* p, OCIParam* param )
{
    OraFieldInfo ofi;
    ub4		colType(0);
    text        *colName = 0;
    ub4         colNameLen(0);
    sb1	colScale(0);
    ub4         colLength(0);
    sb2	colPrecision(0);
    int		r(0);
    QVariant::Type type( QVariant::Invalid );

    r = OCIAttrGet( (dvoid*)param,
		    OCI_DTYPE_PARAM,
		    &colType,
		    0,
		    OCI_ATTR_DATA_TYPE,
		    p->err);
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "qMakeOraField: " + qOraWarn( p ) );
#endif

    r = OCIAttrGet( (dvoid*) param,
		    OCI_DTYPE_PARAM,
		    (dvoid**) &colName,
		    (ub4 *) &colNameLen,
		    (ub4) OCI_ATTR_NAME,
		    p->err );
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "qMakeOraField: " + qOraWarn( p ) );
#endif

    r = OCIAttrGet((dvoid*) param,
		   OCI_DTYPE_PARAM,
		   &colLength,
		   0,
		   OCI_ATTR_DATA_SIZE, /* in bytes */
		   p->err );
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "qMakeOraField: " + qOraWarn( p ) );
#endif

    r = OCIAttrGet( (dvoid*) param,
		    OCI_DTYPE_PARAM,
		    &colPrecision,
		    0,
		    OCI_ATTR_PRECISION,
		    p->err );
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "qMakeOraField: " + qOraWarn( p ) );
#endif

    r = OCIAttrGet( (dvoid*) param,
		    OCI_DTYPE_PARAM,
		    &colScale,
		    0,
		    OCI_ATTR_SCALE,
		    p->err );
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "qMakeOraField: " + qOraWarn( p ) );
#endif
    r = OCIAttrGet( (dvoid*)param,
		    OCI_DTYPE_PARAM,
		    &colType,
		    0,
		    OCI_ATTR_DATA_TYPE,
		    p->err);
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "qMakeOraField: " + qOraWarn( p ) );
#endif

    type = qDecodeOCIType( colType );
    if ( type == QVariant::Int ) {
	if ( colLength == 22 && colPrecision == 0 && colScale == 0 )
	    type = QVariant::Double;
	if ( colScale > 0 )
	    type = QVariant::Double;
    }
    if ( type == QVariant::ByteArray )
	 colLength = QOCI_BYTEARRAY_SIZE;

    ofi.name = QString((char*)colName);
    ofi.name.truncate(colNameLen);
    ofi.type = type;
    ofi.oraType = colType;
    ofi.oraLength = colLength;
    ofi.oraScale = colScale;
    ofi.oraPrecision = colPrecision;

//     qDebug("field name:" + ofi.name);
//     qDebug("field type:" + QString::number(ofi.type));
//     qDebug("field oratype:" + QString::number(ofi.oraType));
//     qDebug("field length:" + QString::number(colLength));
//     qDebug("field scale:" + QString::number(colScale));
//     qDebug("field prec:" + QString::number(colPrecision));
//     qDebug("---------------");

    return ofi;
}

class QOCIResultPrivate
{
public:
    QOCIResultPrivate( int size, QOCIPrivate* d )
	: data( size ), len( size ), ind( size ), typ( size ), def( size )
    {
	len.setAutoDelete( TRUE );
	ind.setAutoDelete( TRUE );
	typ.setAutoDelete( TRUE );

	ub4 dataSize(0);
	OCIDefine* dfn = 0;
	int r;

	OCIParam* param = 0;
	sb4 parmStatus = 0;
	ub4 count = 1;
	parmStatus = OCIParamGet( d->sql,
				  OCI_HTYPE_STMT,
				  d->err,
				  (void**)&param,
				  count );

	while ( parmStatus == OCI_SUCCESS ) {
	    OraFieldInfo ofi = qMakeOraField( d, param );
	    dataSize = ofi.oraLength + 1;
	    QVariant::Type type = ofi.type;
	    createType( count-1, type );
	    switch ( type ) {
	    case QVariant::DateTime:
		r = OCIDefineByPos( d->sql,
				&dfn,
				d->err,
				count,
				create(count-1, dataSize) ,
				dataSize,
				SQLT_DAT,
				(dvoid *) createInd( count-1 ),
				0, 0, OCI_DEFAULT);
		break;
	    case QVariant::CString:
		r = OCIDefineByPos( d->sql,
				    &dfn,
				    d->err,
				    count,
				    0,
				    SB4MAXVAL,/* really big */
				    SQLT_STR,
				    (dvoid *) createInd( count-1 ),
				    (ub2 *) 0,
				    (ub2 *) 0,
				    OCI_DYNAMIC_FETCH ); /* piecewise */
		break;
	    default:
	    case QVariant::ByteArray:
		r = OCIDefineByPos( d->sql,
				    &dfn,
				    d->err,
				    count,
				    create(count-1,dataSize),
				    dataSize,
				    SQLT_STR,
				    (dvoid *) createInd( count-1 ),
				    0, 0, OCI_DEFAULT);
		break;
	    }
	    def[(int)(count-1)] = dfn;
	    count++;
	    parmStatus = OCIParamGet( d->sql,
				      OCI_HTYPE_STMT,
				      d->err,
				      (void**)&param,
				      count );
	}

#ifdef QT_CHECK_RANGE
	    if ( r != 0 )
		qWarning( "QOCIResultPrivate::bind field: " + QString::number(r) + " " + qOraWarn( d ) );
#endif
    }
    ~QOCIResultPrivate()
    {
	for ( uint i=0; i < data.size(); ++i ) {
	    char* c = data.at( i );
	    delete [] c;
	}
    }
    char* at( int i )
    {
	return data.at( i );
    }
    int size()
    {
	return data.size();
    }
    bool isNull( int i )
    {
	return ( *ind.at( i ) == -1 );
    }
    QVariant::Type type( int i )
    {
	return *typ.at( i );
    }
    int fieldFromDefine( OCIDefine* d )
    {
	return def.find( d );
    }
    int length( int i )
    {
	return *len[i];
    }
    QVariant value( int i )
    {
	QVariant v;
	switch ( type(i) ) {
	case QVariant::DateTime: {
	    int century = at(i)[0];
		if( century >= 100 ){
	    int year    = (unsigned char)at(i)[1];
		year = ((century-100)*100) + (year-100);
		int month = at(i)[2];
		int day   = at(i)[3];
		int hour  = at(i)[4] - 1;
		int min   = at(i)[5] - 1;
		int sec   = at(i)[6] - 1;
		v = QVariant( QDateTime( QDate(year,month,day), QTime(hour,min,sec)));
		} else {
		// ### Handle BCE dates here
		v = QVariant( QDateTime() );
	    }
	    break;
	}
	case QVariant::String:
	    v = QVariant( QString( at(i) ) );
	    break;
	case QVariant::CString:
	    v = QVariant( QCString( at(i), length(i)+1 ) );
	    break;
	case QVariant::Int:
	    v = QVariant( QString( at(i) ).toInt() );
	    break;
	case QVariant::Double:
	    v = QVariant( QString( at(i) ).toDouble() );
	    break;
	case QVariant::ByteArray: {
	    QByteArray ba;
	    ba.duplicate( at(i), length(i) );
	    return QVariant( ba );
	    break;
	}
	default:
#ifdef QT_CHECK_RANGE
	    qWarning( "QSqlResultPrivate::value: unknown data type" );
#endif
	    break;
	}
	return v;
    }
private:
    char* create( int position, int size )
    {
	char* c = new char[ size+1 ];
	data.insert( position , c );
	int* l = new int();
	*l = size;
	len.insert( position, l );
	return c;
    }
    sb2* createInd( int position )
    {
	sb2* n = new sb2(0);
	ind.insert( position, n );
	return n;
    }
    void createType( int position, QVariant::Type type )
    {
	typ.insert( position, new QVariant::Type(type) );
    }

    QVector<char> data;
    QVector<int> len;
    QVector<sb2> ind;
    QVector<QVariant::Type> typ;
    QArray< OCIDefine* > def;
};


////////////////////////////////////////////////////////////////////////////

QOCIResult::QOCIResult( const QOCIDriver * db, QOCIPrivate* p )
: QSqlResult(db),
  cols(0),
  cached(FALSE),
  forwardOnly(FALSE)
{
    d = new QOCIPrivate();
    (*d) = (*p);
}

QOCIResult::~QOCIResult()
{
    if ( d->sql ) {
	int r = OCIHandleFree( d->sql,OCI_HTYPE_STMT );
#ifdef QT_CHECK_RANGE
	if ( r != 0 )
	    qWarning( "QOCIResult::reset: unable to free statement handle: " + qOraWarn( d ) );
#endif
    }
    delete d;
    if ( cols )
	delete cols;
}

OCIStmt* QOCIResult::statement()
{
    return d->sql;
}

bool QOCIResult::reset ( const QString& query )
{
    int r(0);
    if ( cols ) {
	delete cols;
	cols = 0;
    }
    rowCache.clear();
    fs.clear();
    if ( d->sql ) {
	r = OCIHandleFree( d->sql,OCI_HTYPE_STMT );
#ifdef QT_CHECK_RANGE
	if ( r != 0 )
	    qWarning( "QOCIResult::reset: unable to free statement handle: " + qOraWarn( d ) );
#endif
    }
    cached = FALSE;
    if ( query.isNull() || query.length() == 0 )
	return FALSE;
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->sql,
			OCI_HTYPE_STMT,
			0,
			0);
    if ( r != 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QOCIResult::reset: unable to alloc statement: " + qOraWarn( d ) );
#endif
	return FALSE;
    }
    QString cleanQuery ( query );
    int delim = cleanQuery.findRev( ";" );
    int len = cleanQuery.length()-1;
    if ( delim > -1 && delim == len )
	cleanQuery.replace( cleanQuery.length()-1, 1, "" );
    r = OCIStmtPrepare( d->sql,
			d->err,
			(unsigned char*)cleanQuery.local8Bit().data(),
			cleanQuery.length(),
			OCI_NTV_SYNTAX,
			OCI_DEFAULT );
    if ( r != 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QOCIResult::reset: unable to prepare statement: " + qOraWarn( d ) );
#endif
	return FALSE;
    }
    ub2 stmtType;
    r = OCIAttrGet( d->sql,
			OCI_HTYPE_STMT,
			(dvoid*)&stmtType,
			NULL,
			OCI_ATTR_STMT_TYPE,
			d->err );
    if ( stmtType == OCI_STMT_SELECT )
    {
	r = OCIStmtExecute( d->svc,
				d->sql,
				d->err,
				0,
				0,
				(CONST OCISnapshot *) NULL,
				(OCISnapshot *) NULL,
				OCI_DEFAULT );
	if ( r != 0 ) {
#ifdef QT_CHECK_RANGE
	    qWarning( "OCIResult::reset: unable to execute statement: " + qOraWarn( d ) );
#endif
	    setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	    return FALSE;
	}
	ub4 parmCount = 0;
	int r = OCIAttrGet( d->sql, OCI_HTYPE_STMT, (dvoid*)&parmCount, NULL, OCI_ATTR_PARAM_COUNT, d->err );
	if ( r == 0 )
	    cols = new QOCIResultPrivate( parmCount, d );
	OCIParam* param = 0;
	sb4 parmStatus = 0;
	ub4 count = 1;
	parmStatus = OCIParamGet( d->sql,
				  OCI_HTYPE_STMT,
				  d->err,
				  (void**)&param,
				  count );
	while ( parmStatus == OCI_SUCCESS ) {
	    OraFieldInfo ofi = qMakeOraField( d, param );
	    QSqlField fi( ofi.name, ofi.type );
	    fs.append( fi );
	    count++;
	    parmStatus = OCIParamGet( d->sql,
				      OCI_HTYPE_STMT,
				      d->err,
				      (void**)&param,
				      count );
	}
	setSelect( TRUE );
    } else { /* non-SELECT */
	r = OCIStmtExecute( d->svc, d->sql, d->err, 1,0,
				(CONST OCISnapshot *) NULL,
				(OCISnapshot *) NULL,
				d->transaction ? OCI_DEFAULT : OCI_COMMIT_ON_SUCCESS  );
	if ( r != 0 ) {
#ifdef QT_CHECK_RANGE
	    qWarning( "OCIResult::reset: unable to execute statement: " + qOraWarn( d ) );
#endif
	    setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	    return FALSE;
	}
	setSelect( FALSE );
    }
    setAt( QSql::BeforeFirst );
    setActive( TRUE);
    return TRUE;
}

bool QOCIResult::cacheNext()
{
    if ( cached )
	return FALSE;
    QMap< int, QCString > binMap;
    int currentRecord = at() + 1;
    int r = 0;
    r = OCIStmtFetch (  d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT );
    if ( r == OCI_NEED_DATA ) { /* piecewise */
	QCString       bin;
	OCIDefine*     dfn;
	ub4            typep;
	ub1            in_outp;
	ub4            iterp;
	ub4            idxp;
	ub1            piecep;
	sword          status;
	text           col [QOCI_DYNAMIC_CHUNK_SIZE+1];
	int            fieldNum = -1;
	bool           nullField;
	for ( ; ; ) {
	    r = OCIStmtGetPieceInfo( d->sql, d->err, (dvoid**) &dfn, &typep,
				     &in_outp, &iterp, &idxp, &piecep );
	    if ( r != OCI_SUCCESS )
		qWarning( "QOCIResult::cacheNext: unable to get piece info: " + qOraWarn(d) );
	    fieldNum = cols->fieldFromDefine( dfn );
	    int chunkSize = QOCI_DYNAMIC_CHUNK_SIZE;
	    nullField = FALSE;
	    r  = OCIStmtSetPieceInfo( dfn, OCI_HTYPE_DEFINE,
				      d->err, (void *)col,
				      (ub4 *)&chunkSize, piecep, NULL, NULL);
	    if ( r != OCI_SUCCESS )
		qWarning( "QOCIResult::cacheNext: unable to set piece info: " + qOraWarn(d) );

	    status = OCIStmtFetch (  d->sql, d->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT );
	    if ( status == -1 ) {
		int errcode;
		OCIErrorGet((dvoid *)d->err, (ub4) 1, (text *) NULL, &errcode, NULL, 0, OCI_HTYPE_ERROR);
		switch ( errcode ) {
		case 1405: /* NULL */
		    nullField = TRUE;
		    break;
		default:
		    qWarning( "QOCIResult::cacheNext: unable to fetch next: " + qOraWarn(d) );
		    break;
		}
	    }
	    if ( status == OCI_NO_DATA ) {
		break;
	    }
	    if ( nullField ) {
		binMap[ fieldNum ] = QCString();
	    }
	    else {
		if ( chunkSize ) {
		    QCString tmp( (char*)col, chunkSize+1 );
		    binMap[ fieldNum ] += tmp;
		} else {
		    binMap[ fieldNum ] = QCString();
		}
	    }
	    if ( status == OCI_SUCCESS_WITH_INFO ||
		 status == OCI_NEED_DATA ) {
	    } else
		break;
	}
    }
    if( r == OCI_ERROR ) {
	int errcode;
	OCIErrorGet((dvoid *)d->err,
		    (ub4) 1,
		    (text *) NULL,
		    &errcode,
		    NULL,
		    0,
		    OCI_HTYPE_ERROR);
	switch ( errcode ) {
	case 1406:
	    qWarning("QOCI Warning: data truncated for " + lastQuery());
	    r = 0; /* ignore it */
	    break;
	default:
	    qWarning( "QOCIResult::cacheNext: " + qOraWarn(d) );
	}
    }
    if ( r == 0 ) {
	for ( int i = 0; i < cols->size(); ++i ) {
	    QVariant v;
	    if ( binMap.contains( i ) ) {
		v = QVariant( binMap[i] );
	    } else {
		v = QVariant( cols->value( i ) );
	    }
	    if ( forwardOnly ) {
		fs.setValue( i, v );
		fs.field( i )->setNull( cols->isNull(i) );
	    } else {
		QSqlField f( QString::null, v.type() );
		f.setValue( v );
		f.setNull( cols->isNull(i) );
		rowCache[currentRecord][i] = f;
	    }
	}
    } else {
	cached = TRUE;
	setAt( QSql::AfterLast );
    }
    return r == 0;
}

bool QOCIResult::fetchNext()
{
    if ( !forwardOnly && rowCache.contains( at() + 1 ) ) {
	setAt( at() + 1 );
	return TRUE;
    }
    if ( cacheNext() ) {
	setAt( at() + 1 );
	return TRUE;
    }
    return FALSE;
}

bool QOCIResult::fetch( int i )
{
    if ( !forwardOnly && rowCache.contains( i ) ) {
	setAt( i );
	return TRUE;
    }
    if ( forwardOnly && at() > i )
	return FALSE;
    while ( at() < i ) {
	if ( !cacheNext() )
	    return FALSE;
	setAt( at() + 1 );
    }
    if ( at() == i ) {
	return TRUE;
    }
    return FALSE;
}

bool QOCIResult::fetchFirst()
{
    if ( forwardOnly && at() != QSql::BeforeFirst )
	return FALSE;
    if ( !forwardOnly && rowCache.contains( 0 ) ) {
	setAt( 0 );
	return TRUE;
    }
    if ( cacheNext() ) {
	setAt( 0 );
	return TRUE;
    }
    return FALSE;
}

bool QOCIResult::fetchLast()
{
    if ( !forwardOnly && at() == QSql::AfterLast && rowCache.count() > 0 ) {
	setAt( rowCache.count() - 1 );
	return TRUE;
    }
    if ( at() >= QSql::BeforeFirst ) {
	while ( fetchNext() )
	    ; /* brute force */
	if ( forwardOnly && at() == QSql::AfterLast ) {
	    setAt( at() - 1 );
	    return TRUE;
	} else
	    return fetch( rowCache.count() - 1 );
    }
    return FALSE;
}

QVariant QOCIResult::data( int field )
{
    if ( forwardOnly )
	return fs.value( field );
    else
	return rowCache[at()][field].value();
}

bool QOCIResult::isNull( int field )
{
    if ( forwardOnly )
	return fs.field( field )->isNull();
    else
	return rowCache[at()][field].isNull();
}

int QOCIResult::size()
{
    return -1;
}

int QOCIResult::numRowsAffected()
{
    int rowCount;
    OCIAttrGet( d->sql,
		OCI_HTYPE_STMT,
		&rowCount,
		NULL,
		OCI_ATTR_ROW_COUNT,
		d->err);
    return rowCount;
}

////////////////////////////////////////////////////////////////////////////

QOCIDriver::QOCIDriver( QObject * parent, const char * name )
: QSqlDriver(parent, (name ? name : "QOCI"))
{
    init();
}

void QOCIDriver::init()
{
    d = new QOCIPrivate();
    int r = OCIEnvCreate( &d->env,
			    OCI_DEFAULT | OCI_OBJECT,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    0,
			    NULL);
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIDriver: unable to create environment: " + qOraWarn( d ) );
#endif
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->err,
			OCI_HTYPE_ERROR,
			(size_t) 0,
			(dvoid **) 0);
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIDriver: unable to alloc error handle: " + qOraWarn( d ) );
#endif
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->svc,
			OCI_HTYPE_SVCCTX,
			(size_t) 0,
			(dvoid **) 0);
#ifdef QT_CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIDriver: unable to alloc service context: " + qOraWarn( d ) );
#endif
    if ( r != 0 )
	setLastError( qMakeError( "Unable to initialize", QSqlError::Connection, d ) );
}

QOCIDriver::~QOCIDriver()
{
    cleanup();
    delete d;
}

bool QOCIDriver::hasTransactionSupport() const
{
    return TRUE;
}

bool QOCIDriver::hasQuerySizeSupport() const
{
    return FALSE;
}

bool QOCIDriver::canEditBinaryFields() const
{
    return TRUE;
}

bool QOCIDriver::open( const QString & db,
			const QString & user,
			const QString & password,
			const QString & )
{
    if ( isOpen() )
	close();
    int r = OCILogon(	d->env,
			d->err,
			&d->svc,
			(unsigned char*)user.local8Bit().data(),
			user.length(),
			(unsigned char*)password.local8Bit().data(),
			password.length(),
			(unsigned char*)db.local8Bit().data(),
			db.length() );
    if ( r != 0 ) {
	setLastError( qMakeError("Unable to logon", QSqlError::Connection, d ) );
	return FALSE;
    }
    setOpen( TRUE );
    return TRUE;
}

void QOCIDriver::close()
{
    cleanup();
    setOpen( FALSE );
    setOpenError( FALSE );
}

void QOCIDriver::cleanup()
{
    if ( isOpen() ) {
	int r(0);
	r = OCILogoff( d->svc, d->err );
	r = OCIHandleFree( (dvoid *) d->svc, OCI_HTYPE_SVCCTX );
	r = OCIHandleFree( (dvoid *) d->err, OCI_HTYPE_ERROR );
    }
}

QSqlQuery QOCIDriver::createQuery() const
{
    return QSqlQuery( new QOCIResult( this, d ) );
}

bool QOCIDriver::beginTransaction()
{
    d->transaction = TRUE;
    int r = OCITransStart ( d->svc,
			    d->err,
			    2,
			    OCI_TRANS_READWRITE );
    if ( r == OCI_ERROR ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QOCIDriver::beginTransaction: " + QString::number(r) + qOraWarn( d ) );
#endif
	return FALSE;
    }
    return TRUE;
}

bool QOCIDriver::commitTransaction()
{
    d->transaction = FALSE;
    int r = OCITransCommit ( d->svc,
			     d->err,
			     0 );
    if ( r == OCI_ERROR ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QOCIDriver::commitTransaction: " + qOraWarn( d ) );
#endif
	return FALSE;
    }
    return TRUE;
}

bool QOCIDriver::rollbackTransaction()
{
    d->transaction = FALSE;
    int r = OCITransRollback ( d->svc,
			       d->err,
			       0 );
    if ( r == OCI_ERROR ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QOCIDriver::rollbackTransaction: " + qOraWarn( d ) );
#endif
	return FALSE;
    }
    return TRUE;
}

QStringList QOCIDriver::tables( const QString& ) const
{
    QSqlQuery t = createQuery();
    t.exec( "select table_name from user_tables;" );
    QStringList tl;
    while ( t.next() ) {
	tl.append( t.value(0).toString() );
    }
    return tl;
}

QSqlRecord QOCIDriver::record( const QString& tablename ) const
{
    QSqlQuery t = createQuery();
    QString stmt ("select column_name, data_type, data_length, data_precision, data_scale "
		  "from user_tab_columns "
		  "where table_name='%1';" );
    t.exec( stmt.arg( tablename.upper() ) );
    QSqlRecord fil;
    while ( t.next() ) {
	QString dt = t.value(1).toString();
	QVariant::Type ty = qDecodeOCIType( dt, t.value(1).toInt(), t.value(2).toInt(), t.value(3).toInt() );
	QSqlField f( t.value(0).toString(), ty );
	fil.append( f );
    }
    return fil;
}

QSqlRecord QOCIDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( !query.isActive() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QOCIResult* result = (QOCIResult*)query.result();
	fil = result->fs;
    }
    return fil;
}

QSqlIndex QOCIDriver::primaryIndex( const QString& tablename ) const
{
    QSqlQuery t = createQuery();
    QString stmt ("select b.column_name, b.data_type, c.index_name "
		  "from user_constraints a, user_tab_columns b, user_ind_columns c "
		  "where a.constraint_type='P' "
		  "and a.table_name = '%1' "
		  "and c.index_name = a.constraint_name "
		  "and b.column_name = c.column_name "
		  "and b.table_name = a.table_name;" );
    t.exec( stmt.arg( tablename.upper() ) );
    QSqlIndex idx( tablename );
    if ( t.next() ) {
	// ### This seems a bit fishy - may need the ocilen, ociprec and ociscale params
	QSqlField f( t.value(0).toString(), qDecodeOCIType(t.value(1).toString(), 0, 0, 0) );
	idx.append( f );
	idx.setName( t.value(2).toString() );
    }
    return idx;
    return QSqlIndex();
}

QString QOCIDriver::formatValue( const QSqlField* field, bool ) const
{
    switch ( field->type() ) {
    case QVariant::DateTime: {
	QDateTime datetime = field->value().toDateTime();
	QString datestring;
	if ( datetime.isValid() ) {
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
	// ### what about the Time only type??
    case QVariant::Date: {
	QDate date = field->value().toDate();
	QString datestring;
	if ( date.isValid() ) {
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
    case QVariant::ByteArray: {
	QString hex = QSqlDriver::formatValue( field );
	return hex;
	return QString( "HEXTORAW(" + hex + ")" );
    }
    default:
	break;
    }
    return QSqlDriver::formatValue( field );
}

OCIEnv* QOCIDriver::environment()
{
    return d->env;
}

OCISvcCtx* QOCIDriver::serviceContext()
{
    return d->svc;
}
