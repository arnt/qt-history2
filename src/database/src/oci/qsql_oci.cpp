#include "qsql_oci.h"

#define OCIEXTP_ORACLE // not needed
#define OCI8DP_ORACLE // not needed
#define OCIEXTP_ORACLE // not needed
#include <oci.h>
#include <qvector.h>
#include <qdatetime.h>

class QOCIPrivate
{
public:
    QOCIPrivate()
	: env(0), err(0), svc(0), sql(0)
    {}
    OCIEnv           *env;
    OCIError         *err;
    OCISvcCtx        *svc;
    OCIStmt          *sql;
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

QSqlField qMakeField( const QOCIPrivate* p, ub4 i )
{
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
	    qWarning( "qMakeField: " + qOraWarn( p ) );
#endif
        r = OCIAttrGet( (dvoid*) param,
			OCI_DTYPE_PARAM,
	                &colPrecision,
			0,
			OCI_ATTR_PRECISION,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "qMakeField: " + qOraWarn( p ) );
#endif
        r = OCIAttrGet( (dvoid*) param,
			OCI_DTYPE_PARAM,
                	&colScale,
                	0,
			OCI_ATTR_SCALE,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "qMakeField: " + qOraWarn( p ) );
#endif
	r = OCIAttrGet((dvoid*) param,
			OCI_DTYPE_PARAM,
                    	&colLength,
                    	0,
			OCI_ATTR_DATA_SIZE,
			p->err );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "qMakeField: " + qOraWarn( p ) );
#endif
	r = OCIAttrGet( (dvoid*)param,
			OCI_DTYPE_PARAM,
			&colType,
			0,
			OCI_ATTR_DATA_TYPE,
			p->err);
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "qMakeField: " + qOraWarn( p ) );
#endif
	type = qDecodeOCIType( colType );
	if ( type == QVariant::DateTime )
	    colLength = 7;
	if ( type == QVariant::Invalid )
	    colLength = 0;
	QString field((char*)colName);
	field.truncate(colNameLen);
	return QSqlField( field, i, type );
    }
    return QSqlField();
}

QOCIDriver::QOCIDriver( QObject * parent, const char * name )
: QSqlDriver(parent, (name ? name : "QOCI"))
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
			    (void**)NULL);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIDriver: Unable to create environment: " + qOraWarn( d ) );
#endif
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->err,
			OCI_HTYPE_ERROR,
			(size_t) 0,
			(dvoid **) 0);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIDriver: Unable to alloc error handle: " + qOraWarn( d ) );
#endif
    r = OCIHandleAlloc( (dvoid *) d->env,
			(dvoid **) &d->svc,
			OCI_HTYPE_SVCCTX,
			(size_t) 0,
			(dvoid **) 0);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIDriver: Unable to alloc service context: " + qOraWarn( d ) );
#endif
    if ( r != 0 )
    	setLastError( qMakeError( "Unable to initialize", QSqlError::Connection, d ) );
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
	setLastError( qMakeError("Unable to logon", QSqlError::Connection, d ) );
	return FALSE;
    }
    setOpen( TRUE );
    return TRUE;
    Q_CONST_UNUSED( host );
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

QStringList QOCIDriver::tables( const QString& user ) const
{
    QSql t = createResult();
    t << "select table_name from user_tables;";
    QStringList tl;
    while ( t.next() )
	tl.append( t[0].toString() );
    return tl;
    Q_CONST_UNUSED( user );
}

QSqlFieldList QOCIDriver::fields( const QString& tablename ) const
{
    QSql t = createResult();
    QString stmt ("select column_name, data_type " //, data_length, data_precision  "
		  "from user_tab_columns "
		  "where table_name='%1';" );
    t << stmt.arg( tablename );
    QSqlFieldList fil;
    while ( t.next() )
	fil.append( QSqlField( t[0].toString(), t.at(), qDecodeOCIType(t[1].toInt()) ));
    return fil;
}

QSqlIndex QOCIDriver::primaryIndex( const QString& tablename ) const
{
    QSql t = createResult();
    QString stmt ("select b.column_name, b.data_type " //, b.data_length, b.data_precision "
		  "from user_constraints a, user_tab_columns b, user_ind_columns c "
		  "where a.constraint_type='P' "
		  "and a.table_name='%1' "
		  "and c.index_name = a.constraint_name "
		  "and b.column_name = c.column_name "
		  "and b.table_name = a.table_name;" );
    t << stmt.arg( tablename );
    QSqlIndex idx( tablename );
    if ( t.next() )
	idx.append( QSqlField( t[0].toString(), t.at(), qDecodeOCIType(t[1].toInt()) ));
    return idx;
    return QSqlIndex();
}

////////////////////////////////////////////////////////////////////////////

class QOCIResultPrivate
{
public:
    QOCIResultPrivate( int size, QOCIPrivate* d )
	: data( size ), ind( size )
    {
	ind.setAutoDelete( TRUE );
	ub4		dataSize(0);
	OCIDefine 	*dfn;
	int 		r;
	for ( int i=1; i <= size; ++i ) {
	    QSqlField f = qMakeField( d, i );
	    //	    dataSize = f.length; // ### must fix this!
	    dataSize = 255; // temporary hack
	    if ( f.type() == QVariant::DateTime ) {
	    	r = OCIDefineByPos( d->sql,
	 			&dfn,
	 			d->err,
         			i,
	 			create(i-1,dataSize),
	 			dataSize,
         			SQLT_DAT,
	 			(dvoid *) createInd( i-1 ),
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
	 			(dvoid *) createInd( i-1 ),
         			(ub2 *) 0,
	 			(ub2 *) 0,
	 			OCI_DEFAULT);
	    }
#ifdef CHECK_RANGE
	    if ( r != 0 )
	    	qWarning( "QOCIResultPrivate: " +  qOraWarn( d ) );
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

private:
    char* create( int position, int size )
    {
	char* c = new char[ size+1 ];
	data.insert( position , c );
	return c;
    }
    sb2* createInd( int position )
    {
	sb2* n = new sb2(0);
	ind.insert( position, n );
	return n;
    }
    QVector<char> data;
    QVector<sb2> ind;
};

////////////////////////////////////////////////////////////////////////////

QOCIResult::QOCIResult( const QOCIDriver * db, QOCIPrivate* p )
: QSqlResult(db),
  cols(0)
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
	    qWarning( "QOCIResult: Unable to free statement handle: " + qOraWarn( d ) );
#endif
    }
    delete d;
    if ( cols )
	delete cols;
}

bool QOCIResult::reset ( const QString& query )
{
    int r(0);
    if ( d->sql ) {
	r = OCIHandleFree( d->sql,OCI_HTYPE_STMT );
#ifdef CHECK_RANGE
	if ( r != 0 )
	    qWarning( "QOCIResult::reset: Unable to free statement handle: " + qOraWarn( d ) );
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
	qWarning( "QOCIResult::reset: Unable to alloc statement: " + qOraWarn( d ) );
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
	qWarning( "QOCIResult::reset: Unable to prepare statement: " + qOraWarn( d ) );
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
#ifdef CHECK_RANGE
	    qWarning( qOraWarn( d ) );
#endif
	    setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	    return FALSE;
	}
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
	qWarning( "QOCIResult::reset: " + qOraWarn( d ) );
#endif
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
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
//	qDebug("next error" + qOraWarn(d));
//    if ( r==OCI_SUCCESS_WITH_INFO)
//	qDebug("next success with info" + qOraWarn(d));
    if ( r != 0 ) {
	setAt( BeforeFirst );
	return FALSE;
    }
    setAt( at() + 1 );
    for ( int i = 0; i < cols->size(); ++i ) {
	QSqlField f = qMakeField( d, i+1 );
	if ( f.type() == QVariant::DateTime ) {
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

bool QOCIResult::isNull( int field ) const
{
    return cols->isNull( field );
}

QSqlFieldList QOCIResult::fields()
{
    QSqlFieldList fil;
    ub4 numCols = 0;
    int r  = OCIAttrGet( (dvoid*)d->sql,
    				OCI_HTYPE_STMT,
				(dvoid**)&numCols,
            			0,
				OCI_ATTR_PARAM_COUNT,
				d->err);
#ifdef CHECK_RANGE
    if ( r != 0 )
	qWarning( "QOCIResult::fields: " + qOraWarn( d ) );
#endif
    for ( ub4 i = 0; i < numCols; ++i ) {
	QSqlField fi = qMakeField( d, i );
	if ( isActive() )
	    fi.value() = data( i );
	fil.append( fi );
    }
    return fil;
}

int QOCIResult::size() const
{
    return -1; // not supported
}

int QOCIResult::affectedRows() const
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

