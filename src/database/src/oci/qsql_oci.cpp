#include "qsql_oci.h"

#include <oci.h>
#include <qvector.h>
#include <qdatetime.h>

class QOCIPrivate
{
public:
    QOCIPrivate() {}
    OCIEnv           *env;
    OCIError         *err;
    OCISvcCtx        *svc;
    OCIStmt          *sql;
};

QString OraWarn( const QOCIPrivate* d)
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

QSqlError makeError( const QString& err, int type, const QOCIPrivate* p )
{
    return QSqlError("QOCI: " + err, OraWarn(p), type );
}

QSqlFieldInfo makeFieldInfo( const QOCIPrivate* p, ub4 i )
{
    qDebug("making field info for field:" + QString::number(i));
    OCIParam	*param;
    text        *colName;
    ub4         colNameLen(0);
    ub4         colLength(0);
    sb2 	colPrecision(0);
    sb1 	colScale(0);
    ub4		colType(0);
    sb4         paramStatus;
    int		r;
    QVariant::Type type(QVariant::Invalid);
    paramStatus = OCIParamGet(  p->sql,
				OCI_HTYPE_STMT,
				p->err,
				(void**)&param,
				i );
    if ( paramStatus == OCI_SUCCESS ) {

	r = OCIAttrGet( (dvoid*) param,
			OCI_DTYPE_PARAM,
			(dvoid**) &colName,
			(ub4 *) &colNameLen,
			(ub4) OCI_ATTR_NAME,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( OraWarn( p ) );
#endif
        r = OCIAttrGet( (dvoid*) param,
			OCI_DTYPE_PARAM,
	                &colPrecision,
			0,
			OCI_ATTR_PRECISION,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( OraWarn( p ) );
#endif
        r = OCIAttrGet( (dvoid*) param,
			OCI_DTYPE_PARAM,
                	&colScale,
                	0,
			OCI_ATTR_SCALE,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( OraWarn( p ) );
#endif
	r = OCIAttrGet((dvoid*) param,
			OCI_DTYPE_PARAM,
                    	&colLength,
                    	0,
			OCI_ATTR_DATA_SIZE,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( OraWarn( p ) );
#endif
	r = OCIAttrGet( (dvoid*)param,
			OCI_DTYPE_PARAM,
			&colType,
			0,
			OCI_ATTR_DATA_TYPE,
			p->err);
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( OraWarn( p ) );
#endif
	switch ( colType ) {
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
	    colLength = 7;
	    break;
	default:
	    colLength = 0;
	    type = QVariant::Invalid;
	    break;
	}
	QString field((char*)colName);
	field.truncate(colNameLen);
	return QSqlFieldInfo( field, type, colLength, colPrecision );
    }
    qDebug("Returning invalid field");
    return QSqlFieldInfo( "", QVariant::Invalid, 0, 0 );
}

QOCIDriver::QOCIDriver( QObject * parent, const char * name )
: QSqlDriver(parent,"QOCI")
{
    init();
}

void QOCIDriver::init()
{
    d = new QOCIPrivate();
    int r = OCIEnvCreate( &d->env,
			    OCI_DEFAULT,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    NULL);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "Unable to create environment - " + OraWarn( d ) );
#endif
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->err,
			OCI_HTYPE_ERROR,
			(size_t) 0,
			(dvoid **) 0);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "Unable to alloc error handle - " + OraWarn( d ) );
#endif
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->svc,
			OCI_HTYPE_SVCCTX,
			(size_t) 0,
			(dvoid **) 0);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "Unable to alloc service context - " + OraWarn( d ) );
#endif
    if ( r != 0 )
    	setLastError( makeError( "Unable to initialize", QSqlError::Connection, d ) );
}

QOCIDriver::~QOCIDriver()
{
    cleanup();
    delete d;
}

bool QOCIDriver::open( const QString & db,
    			const QString & user,
			const QString & password,
			const QString & host)
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
	setLastError( makeError("Unable to logon", QSqlError::Connection, d ) );
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

QSql QOCIDriver::createResult() const
{
    return QSql( new QOCIResult( this, d ) );
}

bool QOCIDriver::beginTransaction()
{
    return FALSE;
}

bool QOCIDriver::commitTransaction()
{
    return FALSE;
}

bool QOCIDriver::rollbackTransaction()
{
    return FALSE;
}

bool QOCIDriver::endTrans()
{
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////

QOCIResultInfo::QOCIResultInfo( const QOCIPrivate* p )
: QSqlResultInfo()
{
    ub4 numCols = 0;
    int r  = OCIAttrGet( (dvoid*)p->sql,
    				OCI_HTYPE_STMT,
				(dvoid**)&numCols,
            			0,
				OCI_ATTR_PARAM_COUNT,
				p->err);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( OraWarn( p ) );
#endif
    for ( ub4 i = 0; i < numCols; ++i )
	appendField( makeFieldInfo( p, i ) );
    int rowCount;
    OCIAttrGet( p->sql,
    		OCI_HTYPE_STMT,
		&rowCount,
		NULL,
		OCI_ATTR_ROW_COUNT,
		p->err);
    setAffectedRows( rowCount );
    setSize( -1 );
}

QOCIResultInfo::~QOCIResultInfo()
{
}

////////////////////////////////////////////////////////////////////////////

class QOCIResultPrivate
{
public:
    QOCIResultPrivate( int size, QOCIPrivate* d )
    : data( size )
    {
	ub4		dataSize(0);
	OCIDefine 	*dfn;
	int 		r;
	for ( int i=1; i <= size; ++i ) {
	    QSqlFieldInfo f = makeFieldInfo( d, i );
	    dataSize = f.length;
	    qDebug("about to bind field " + QString::number(i) + " with length:" + QString::number(dataSize));
	    if ( f.type == QVariant::DateTime ) {
	    	r = OCIDefineByPos( d->sql,
	 			&dfn,
	 			d->err,
         			i,
	 			create(i-1,dataSize),
	 			dataSize,
         			SQLT_DAT,
	 			(dvoid *) 0,
         			(ub2 *) 0,
	 			(ub2 *) 0,
	 			OCI_DEFAULT);
	    } else {
	    	r = OCIDefineByPos( d->sql,
	 			&dfn,
	 			d->err,
         			i,
	 			create(i-1,dataSize),
	 			dataSize+1,
         			SQLT_STR,
	 			(dvoid *) 0,
         			(ub2 *) 0,
	 			(ub2 *) 0,
	 			OCI_DEFAULT);
	    }
#ifdef CHECK_RANGE
	    if ( r != 0 )
	    	qWarning( OraWarn( d ) );
#endif
	}
    }
    ~QOCIResultPrivate()
    {
    	for ( uint i=0; i < data.size(); ++i ) {
	    char* c = data.at( i );
	    delete [] c;
	}
    }
    char* create( int position, int size )
    {
	char* c = new char[ size+1 ];
	data.insert( position , c );
	return c;
    }
    char* at( int i )
    {
	return data.at( i );
    }
    int size()
    {
	return data.size();
    }
private:
    QVector<char> data;
};

////////////////////////////////////////////////////////////////////////////

QOCIResult::QOCIResult( const QOCIDriver * db, QOCIPrivate* p )
: QSqlResult(db),
  cols(0),
  resultInfo(0)
{
    d = new QOCIPrivate();
    (*d) = (*p);
}

QOCIResult::~QOCIResult()
{
    int r(0);
    if ( d->sql ) {
	r = OCIHandleFree( d->sql,OCI_HTYPE_STMT );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "Unable to free statement handle - " + OraWarn( d ) );
#endif
    }
    delete d;
    if ( resultInfo )
        delete resultInfo;
    if ( cols )
	delete cols;
}

const QSqlResultInfo* QOCIResult::info()
{
    if ( resultInfo )
	delete resultInfo;
    if ( isActive() )
    	resultInfo = new QOCIResultInfo( d );
    return resultInfo;
}

bool QOCIResult::reset ( const QString& query )
{
    int r(0);
    if ( d->sql ) {
	r = OCIHandleFree( d->sql,OCI_HTYPE_STMT );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "Unable to free statement handle - " + OraWarn( d ) );
#endif
    }
    if ( cols ) {
	delete cols;
	cols = 0;
    }
    rowCache.clear();
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->sql,
			OCI_HTYPE_STMT,
			0,
			0);
    if ( r != 0 ) {
#ifdef CHECK_RANGE
	qWarning( "Unable to alloc statement - " + OraWarn( d ) );
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
#ifdef CHECK_RANGE
	qWarning( "Unable to prepare statement - " + OraWarn( d ) );
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
    	ub4 parmCount;
    	r = OCIAttrGet( d->sql,
    			OCI_HTYPE_STMT,
			(dvoid*)&parmCount,
			NULL,
			OCI_ATTR_PARAM_COUNT,
			d->err );
    	if ( r == 0 )
	    cols = new QOCIResultPrivate( parmCount, d );
    } else {
    	r = OCIStmtExecute( d->svc,
				d->sql,
				d->err,
				1,
				0,
				(CONST OCISnapshot *) NULL,
				(OCISnapshot *) NULL,
				OCI_COMMIT_ON_SUCCESS  );
    }
    if ( r != 0 ) {
#ifdef CHECK_RANGE
	qWarning( OraWarn( d ) );
#endif
	setLastError( makeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    setActive( TRUE) ;
    return TRUE;
}

bool QOCIResult::fetchNext()
{
    if ( rowCache.contains( at() + 1 ) ) {
    	setAt( at() + 1 );
    	return TRUE;
    }
    int r = OCIStmtFetch (  d->sql,
			    d->err,
			    1,
			    OCI_FETCH_NEXT,
			    OCI_DEFAULT );
//    if( r==OCI_ERROR)
//	qDebug("next error" + OraWarn(d));
//    if ( r==OCI_SUCCESS_WITH_INFO)
//	qDebug("next success with info" + OraWarn(d));
    if ( r != 0 ) {
	setAt( BeforeFirst );
	return FALSE;
    }
    setAt( at() + 1 );
    for ( int i = 0; i < cols->size(); ++i ) {
	QSqlFieldInfo f = makeFieldInfo( d, i+1 );
	if ( f.type == QVariant::DateTime ) {
	    int century = cols->at(i)[0];
	    int year = (unsigned char)cols->at(i)[1];
	    if ( year > 100 && century > 100 ) {
		 year = ((century-100)*100) + (year-100);
	    	int month = cols->at(i)[2];
	    	int day = cols->at(i)[3];
	    	int hour = cols->at(i)[5];
	    	int min = cols->at(i)[6];
	    	int sec = cols->at(i)[7];
	    	rowCache[at()][i] = QVariant( QDateTime( QDate(year,month,day), QTime(hour,min,sec)));
		qDebug(QVariant( QDateTime( QDate(year,month,day), QTime(hour,min,sec))).toString());
	    } else {
		rowCache[at()][i] = QVariant( QDateTime() );
	    }
	} else {
	    rowCache[at()][i] = QVariant(cols->at(i));
	}
    }
    return TRUE;
}

bool QOCIResult::fetch( int i )
{
    if ( rowCache.contains( i ) ) {
    	setAt( i );
    	return TRUE;
    }
    while ( i > at() ) {
	if ( !fetchNext() )
	    return FALSE;
    }
    setAt( i );
    return TRUE;
}

bool QOCIResult::fetchFirst()
{
    if ( rowCache.contains( 0 ) ) {
    	setAt( 0 );
	return TRUE;
    }
    if ( at() == BeforeFirst )
	return fetchNext();
    setAt( AfterLast );
    return FALSE;
}

bool QOCIResult::fetchLast()
{
    setAt( AfterLast );
    return FALSE;
}

QVariant QOCIResult::data( int field )
{
    return rowCache[at()][field];
}

//QString QOCIResult::string( int field )
//{
//    if ( cols )
//    	return QString(cols->at(field));
//    return QString::null;
//}

//QByteArray QOCIResult::binary( int field )
//{
//    return string( field ).local8Bit();
//}

bool QOCIResult::isNull( int field ) const
{
    return FALSE;
}