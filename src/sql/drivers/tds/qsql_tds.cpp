/****************************************************************************
**
** Implementation of TDS driver classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qglobal.h>
#ifdef Q_OS_WIN32    // We assume that MS SQL Server is used. Set Q_USE_SYBASE to force Sybase.
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#include <windows.h>
#else
#define Q_USE_SYBASE
#endif

#include "qsql_tds.h"

#include <qapplication.h>
#include <qhash.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qregexp.h>

#ifdef DBNTWIN32
#define QMSGHANDLE DBMSGHANDLE_PROC
#define QERRHANDLE DBERRHANDLE_PROC
#define QTDSCHAR SQLCHAR
#define QTDSDATETIME4 SQLDATETIM4
#define QTDSDATETIME SQLDATETIME
#define QTDSDATETIME_N SQLDATETIMN
#define QTDSDECIMAL SQLDECIMAL
#define QTDSFLT4 SQLFLT4
#define QTDSFLT8 SQLFLT8
#define QTDSFLT8_N SQLFLTN
#define QTDSINT1 SQLINT1
#define QTDSINT2 SQLINT2
#define QTDSINT4 SQLINT4
#define QTDSINT4_N SQLINTN
#define QTDSMONEY4 SQLMONEY4
#define QTDSMONEY SQLMONEY
#define QTDSMONEY_N SQLMONEYN
#define QTDSNUMERIC SQLNUMERIC
#define QTDSTEXT SQLTEXT
#define QTDSVARCHAR SQLVARCHAR
#define QTDSBIT SQLBIT
#define QTDSBINARY SQLBINARY
#define QTDSVARBINARY SQLVARBINARY
#define QTDSIMAGE SQLIMAGE
#else
#define QMSGHANDLE MHANDLEFUNC
#define QERRHANDLE EHANDLEFUNC
#define QTDSCHAR SYBCHAR
#define QTDSDATETIME4 SYBDATETIME4
#define QTDSDATETIME SYBDATETIME
#define QTDSDATETIME_N SYBDATETIMN
#define QTDSDECIMAL SYBDECIMAL
#define QTDSFLT8 SYBFLT8
#define QTDSFLT8_N SYBFLTN
#define QTDSFLT4 SYBREAL
#define QTDSINT1 SYBINT1
#define QTDSINT2 SYBINT2
#define QTDSINT4 SYBINT4
#define QTDSINT4_N SYBINTN
#define QTDSMONEY4 SYBMONEY4
#define QTDSMONEY SYBMONEY
#define QTDSMONEY_N SYBMONEYN
#define QTDSNUMERIC SYBNUMERIC
#define QTDSTEXT SYBTEXT
#define QTDSVARCHAR SYBVARCHAR
#define QTDSBIT SYBBIT
#define QTDSBINARY SYBBINARY
#define QTDSVARBINARY SYBVARBINARY
#define QTDSIMAGE SYBIMAGE
// magic numbers not defined anywhere in Sybase headers
#define QTDSDECIMAL_2 55
#define QTDSNUMERIC_2 63
#endif  //DBNTWIN32

#define TDS_CURSOR_SIZE 50

// workaround for FreeTDS
#ifndef CS_PUBLIC
#define CS_PUBLIC
#endif

QSqlError qMakeError( const QString& err, int type, int errNo = -1 )
{
    return QSqlError( "QTDS: " + err, QString(), type, errNo );
}

class QTDSDriverPrivate
{
public:
    QTDSDriverPrivate(): login(0) {}
    LOGINREC* login;  // login information
    QString hostName;
    QString db;
};


class QTDSResultPrivate
{
public:
    QTDSResultPrivate():login(0), dbproc(0) {}
    LOGINREC* login;  // login information
    DBPROCESS* dbproc; // connection from app to server
    QSqlError lastError;
    void addErrorMsg( QString& errMsg ) { errorMsgs.append( errMsg ); }
    QString getErrorMsgs() { return errorMsgs.join("\n"); }
    void clearErrorMsgs() { errorMsgs.clear(); }
    QVector<void *> buffer;
    QSqlRecord rec;
    
private:
    QStringList errorMsgs;
};

static QHash<DBPROCESS*, QTDSResultPrivate*> errs;

extern "C" {
static int CS_PUBLIC qTdsMsgHandler ( DBPROCESS* dbproc,
			    DBINT /*msgno*/,
			    int msgstate,
			    int severity,
			    char* msgtext,
			    char* srvname,
			    char* /*procname*/,
			    int /*line*/)
{
    QTDSResultPrivate* p = errs.value( dbproc );

    if ( !p ) {
//	### umm... temporary disabled since this throws shitloads of warnings...
//	qWarning( "QTDSDriver warning (%d): [%s] from server [%s]", msgstate, msgtext, srvname );
	return INT_CANCEL;
    }

    if ( severity > 0 ) {
	QString errMsg = QString( "%1 (%2)" ).arg( msgtext ).arg( msgstate );
	p->addErrorMsg( errMsg );
    }

    return INT_CANCEL;
}

static int CS_PUBLIC qTdsErrHandler( DBPROCESS* dbproc,
				int /*severity*/,
				int dberr,
				int /*oserr*/,
				char* dberrstr,
				char* oserrstr)
{
    QTDSResultPrivate* p = errs.value( dbproc );
    if ( !p ) {
	qWarning( "QTDSDriver error (%d): [%s] [%s]", dberr, dberrstr, oserrstr );
	return INT_CANCEL;
    }
    /*
     * If the process is dead or NULL and
     * we are not in the middle of logging in...
     */
    if( (dbproc == NULL || DBDEAD( dbproc )) ) {
	qWarning( "QTDSDriver error (%d): [%s] [%s]", dberr, dberrstr, oserrstr );
	return INT_CANCEL;
    }


    QString errMsg = QString( "%1 %2\n" ).arg( dberrstr ).arg( oserrstr );
    errMsg += p->getErrorMsgs();
    p->lastError = qMakeError( errMsg, QSqlError::Unknown, dberr );
    p->clearErrorMsgs();

    return INT_CANCEL ;
}

} //extern "C"


QVariant::Type qDecodeTDSType( int type )
{
    QVariant::Type t = QVariant::Invalid;
    switch ( type ) {
    case QTDSCHAR:
    case QTDSTEXT:
    case QTDSVARCHAR:
	t = QVariant::String;
	break;
    case QTDSINT1:
    case QTDSINT2:
    case QTDSINT4:
    case QTDSINT4_N:
    case QTDSBIT:
	t = QVariant::Int;
	break;
    case QTDSFLT4:
    case QTDSFLT8:
    case QTDSFLT8_N:
    case QTDSMONEY4:
    case QTDSMONEY:
    case QTDSDECIMAL:
    case QTDSNUMERIC:
#ifdef QTDSNUMERIC_2
    case QTDSNUMERIC_2:
#endif
#ifdef QTDSDECIMAL_2
    case QTDSDECIMAL_2:
#endif
    case QTDSMONEY_N:
	t = QVariant::Double;
	break;
    case QTDSDATETIME4:
    case QTDSDATETIME:
    case QTDSDATETIME_N:
	t = QVariant::DateTime;
	break;
    case QTDSBINARY:
    case QTDSVARBINARY:
    case QTDSIMAGE:
	t = QVariant::ByteArray;
	break;
    default:
	t = QVariant::Invalid;
	break;
    }
    return t;
}

QVariant::Type qFieldType( QTDSResultPrivate* d, int i )
{
    QVariant::Type type = qDecodeTDSType( dbcoltype( d->dbproc, i+1 ) );
    return type;
}


QTDSResult::QTDSResult(const QTDSDriver* db)
    : QtSqlCachedResult(db)
{
    d = new QTDSResultPrivate();
    d->login = db->d->login;
    
    d->dbproc = dbopen(d->login, const_cast<char*>(db->d->hostName.latin1()));
    if (!d->dbproc)
	return;
    if (dbuse(d->dbproc, const_cast<char*>(db->d->db.latin1())) == FAIL)
	return;

    // insert d in error handler dict
    errs.insert(d->dbproc, d);
}

QTDSResult::~QTDSResult()
{
    cleanup();
    dbclose( d->dbproc );
    errs.remove( d->dbproc );
    delete d;
}

void QTDSResult::cleanup()
{
    d->clearErrorMsgs();
    d->rec.clear();
    for (int i = 0; i < d->buffer.size() / 2; ++i)
	free(d->buffer.at(i * 2));
    d->buffer.clear();
    // "can" stands for "cancel"... very clever.
    dbcanquery( d->dbproc );
    dbfreebuf( d->dbproc );

    QtSqlCachedResult::cleanup();
}

DBPROCESS *QTDSResult::dbprocess() const
{
    return d->dbproc;
}

bool QTDSResult::gotoNext(QtSqlCachedResult::ValueCache &values, int index)
{
    STATUS stat = dbnextrow( d->dbproc );
    if ( stat == NO_MORE_ROWS ) {
	setAt( QSql::AfterLast );
	return FALSE;
    }
    if ( ( stat == FAIL ) || ( stat == BUF_FULL ) ) {
	setLastError( d->lastError );
	return FALSE;
    }

    if (index < 0)
	return TRUE;
  
    for (int i = 0; i < d->rec.count(); ++i) {
	int idx = index + i;
	switch (d->rec.field(i)->type()) {
	    case QVariant::DateTime:
		if ((DBINT)d->buffer.at(i * 2 + 1) == -1) {
		    values[idx] = QVariant(QVariant::DateTime);
		} else {
		    DBDATETIME *bdt = (DBDATETIME*) d->buffer.at(i * 2);
		    QDate date = QDate::fromString("1900-01-01", Qt::ISODate);
		    QTime time = QTime::fromString("00:00:00", Qt::ISODate);
		    values[idx] = QDateTime( date.addDays( bdt->dtdays ), time.addMSecs( int( bdt->dttime / 0.3 ) ) );		
		    break;
		}
	    case QVariant::Int:
		if ((DBINT)d->buffer.at(i * 2 + 1) == -1)
		    values[idx] = QVariant(QVariant::Int);
		else
		    values[idx] = *((int*)d->buffer.at(i * 2));
		break;
	    case QVariant::Double:
		if ((DBINT)d->buffer.at(i * 2 + 1) == -1)
		    values[idx] = QVariant(QVariant::Double);
		else
		    values[idx] = *((double*)d->buffer.at(i * 2));
		break;	    
	    case QVariant::String:
		if ((DBINT)d->buffer.at(i * 2 + 1) == -1)
		    values[idx] = QVariant(QVariant::String);
		else
		    values[idx] = QString::fromLocal8Bit((const char*)d->buffer.at(i * 2));
		break;
	    case QVariant::ByteArray: {
		if ((DBINT)d->buffer.at(i * 2 + 1) == -1)
		    values[idx] = QVariant(QVariant::ByteArray);
		else
		    values[idx] = QByteArray((const char*)d->buffer.at(i * 2));
		break;
	    }
	    default:
		// should never happen, and we already fired
		// a warning while binding.
		values[idx] = QVariant();
		break;
	}	
    }

    return TRUE;
}

bool QTDSResult::reset ( const QString& query )
{
    cleanup();
    if (!driver() || !driver()-> isOpen() || driver()->isOpenError())
	return FALSE;
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    if ( dbcmd( d->dbproc, const_cast<char*>(query.local8Bit()) ) == FAIL ) {
	setLastError( d->lastError );
	return FALSE;
    }

    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbresults( d->dbproc ) != SUCCEED ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }

    setSelect( (DBCMDROW( d->dbproc ) == SUCCEED) ); // decide whether or not we are dealing with a SELECT query
    int numCols = dbnumcols(d->dbproc);
    if (numCols > 0) {
	d->buffer.resize(numCols * 2);
	init(numCols);
    }
    for ( int i = 0; i < numCols; ++i ) {
	QVariant::Type vType = qDecodeTDSType( dbcoltype( d->dbproc, i+1 ) );
	d->rec.append(QSqlField(dbcolname(d->dbproc, i+1), vType, -1, dbcollen( d->dbproc, i+1 )));
	
	RETCODE ret = -1;
	void* p = 0;
	switch ( vType ) {
	case QVariant::Int:	
	    p = malloc(4);
	    ret = dbbind(d->dbproc, i+1, INTBIND, (DBINT) 4, (unsigned char *)p);
	    break;
	case QVariant::Double:
	    // use string binding to prevent loss of precision
	    p = malloc(50);
	    ret = dbbind(d->dbproc, i+1, STRINGBIND, 50, (unsigned char *)p);
	    break;
	case QVariant::String:
	    p = malloc(dbcollen(d->dbproc, i+1) + 1);
	    ret = dbbind(d->dbproc, i+1, STRINGBIND, DBINT(dbcollen( d->dbproc, i+1 ) + 1), (unsigned char *)p);
	    break;
	case QVariant::DateTime:
	    p = malloc(8);
	    ret = dbbind(d->dbproc, i+1, DATETIMEBIND, (DBINT) 8, (unsigned char *)p);
	    break;
	case QVariant::ByteArray:
	    p = malloc(dbcollen( d->dbproc, i+1 ) + 1);
	    ret = dbbind( d->dbproc, i+1, BINARYBIND, DBINT(dbcollen( d->dbproc, i+1 ) + 1), (unsigned char *)p );
	    break;
	default: //don't bind the field since we do not support it
#ifdef QT_CHECK_RANGE
	    qWarning( "QTDSResult::reset: Unsupported type for field \"%s\"", dbcolname( d->dbproc, i+1 ) );
#endif
	    break;
	}
	if ( ret == SUCCEED ) {
	    d->buffer[i * 2] = p;
	    ret = dbnullbind(d->dbproc, i+1, (DBINT*)(&d->buffer[i * 2 + 1]));
	} else {
	    d->buffer[i * 2] = 0;
	    d->buffer[i * 2 + 1] = 0;
	    free(p);
	}
	if ( ( ret != SUCCEED ) && ( ret != -1 ) ) {
	    setLastError( d->lastError );
	    return FALSE;
	}
    }

    setActive( TRUE );
    return TRUE;
}

int QTDSResult::size()
{
    return -1;
}

int QTDSResult::numRowsAffected()
{
#ifdef DBNTWIN32
    if ( dbiscount( d->dbproc ) ) {
	return DBCOUNT( d->dbproc );
    }
    return -1;
#else
    return DBCOUNT( d->dbproc );
#endif
}

QSqlRecord QTDSResult::record() const
{
    return d->rec;
}

///////////////////////////////////////////////////////////////////

QTDSDriver::QTDSDriver(QObject* parent)
    : QSqlDriver(parent)
{
    init();
}

QTDSDriver::QTDSDriver(LOGINREC* rec, const QString& host, const QString &db, QObject* parent)
    : QSqlDriver(parent)
{
    init();    
    d->login = rec;
    d->hostName = host;
    d->db = db;
    if ( rec ) {
	setOpen( TRUE );
	setOpenError( FALSE );
    }
}

LOGINREC* QTDSDriver::loginrec() const
{
    return d->login;
}

void QTDSDriver::init()
{
    d = new QTDSDriverPrivate();
    // the following two code-lines will fail compilation on some FreeTDS versions
    // just comment them out if you have FreeTDS (you won't get any errors and warnings then)
    dberrhandle( (QERRHANDLE)qTdsErrHandler );
    dbmsghandle( (QMSGHANDLE)qTdsMsgHandler );
}

QTDSDriver::~QTDSDriver()
{
    dberrhandle( 0 );
    dbmsghandle( 0 );
    // dbexit also calls dbclose if neccessary
    dbexit();
    delete d;
}

bool QTDSDriver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
    case Transactions:
	return FALSE;
    case QuerySize:
	return FALSE;
    case BLOB:
	return TRUE;
    case Unicode:
	return FALSE;
    default:
	return FALSE;
    }
}

bool QTDSDriver::open( const QString & db,
		       const QString & user,
		       const QString & password,
		       const QString & host,
		       int /*port*/,
		       const QString& /*connOpts*/ )
{
    if ( isOpen() )
	close();
    if ( !dbinit() ) {
	setOpenError( TRUE );
	return FALSE;
    }
    d->login = dblogin();
    if ( !d->login ) {
	setOpenError( TRUE );
	return FALSE;
    }
    DBSETLPWD(d->login, const_cast<char*>(password.local8Bit()));
    DBSETLUSER(d->login, const_cast<char*>(user.local8Bit()));

    setOpen( TRUE );
    setOpenError( FALSE );
    d->hostName = host;
    d->db = db;
    return TRUE;
}

void QTDSDriver::close()
{
    if ( isOpen() ) {
#ifdef Q_USE_SYBASE
	dbloginfree( d->login );
#else
	dbfreelogin( d->login );
#endif
	d->login = 0;
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSqlQuery QTDSDriver::createQuery() const
{
    return QSqlQuery(new QTDSResult(this));
}

bool QTDSDriver::beginTransaction()
{
    return FALSE;
/*
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QTDSDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( dbcmd( d->dbproc, "BEGIN TRANSACTION" ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    while( dbresults( d->dbproc ) == NO_MORE_RESULTS ) {}
    dbfreebuf( d->dbproc );
    inTransaction = TRUE;
    return TRUE;
*/
}

bool QTDSDriver::commitTransaction()
{
    return FALSE;
/*
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QTDSDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( dbcmd( d->dbproc, "COMMIT TRANSACTION" ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    while( dbresults( d->dbproc ) == NO_MORE_RESULTS ) {}
    dbfreebuf( d->dbproc );
    inTransaction = FALSE;
    return TRUE;
*/
}

bool QTDSDriver::rollbackTransaction()
{
    return FALSE;
/*
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QTDSDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( dbcmd( d->dbproc, "ROLLBACK TRANSACTION" ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    while( dbresults( d->dbproc ) == NO_MORE_RESULTS ) {}
    dbfreebuf( d->dbproc );
    inTransaction = FALSE;
    return TRUE;
*/
}

QSqlRecord QTDSDriver::record(const QString& tablename) const
{
    QSqlRecord info;
    if ( !isOpen() )
	return info;
    QSqlQuery t = createQuery();
    t.setForwardOnly( TRUE );
    QString stmt ( "select name, type, length, prec from syscolumns "
		   "where id = (select id from sysobjects where name = '%1')" );
    t.exec( stmt.arg( tablename ) );
    while ( t.next() ) {
	info.append( QSqlField( t.value(0).toString().simplified(),
				qDecodeTDSType( t.value(1).toInt() ),
				-1,
				t.value(2).toInt(),
				t.value(3).toInt(),
				QVariant(),
				t.value(1).toInt() ) );
    }
    return info;
}

QStringList QTDSDriver::tables( const QString& typeName ) const
{
    QStringList list;

    if ( !isOpen() )
	return list;

    int type = typeName.toInt();
    QString typeFilter;

    if ( typeName.isEmpty() || ((type & (int)QSql::Tables) == (int)QSql::Tables) )
	typeFilter += "type='U' or ";
    if ( (type & (int)QSql::SystemTables) == (int)QSql::SystemTables )
	typeFilter += "type='S' or ";
    if ( (type & (int)QSql::Views) == (int)QSql::Views )
	typeFilter += "type='V' or ";

    if ( typeFilter.isEmpty() )
	return list;
    typeFilter.truncate( typeFilter.length() - 4 );

    QSqlQuery t = createQuery();
    t.setForwardOnly( TRUE );
    t.exec( "select name from sysobjects where " + typeFilter );
    while ( t.next() )
	list.append( t.value(0).toString().simplified() );

    return list;
}

QString QTDSDriver::formatValue( const QSqlField* field,
				  bool ) const
{
    QString r;
    if ( field->isNull() )
	r = nullText();
    else if ( field->type() == QVariant::DateTime ) {
	if ( field->value().toDateTime().isValid() ){
	    r = field->value().toDateTime().toString( "'yyyyMMdd hh:mm:ss'" );
	} else
	    r = nullText();
    } else if ( field->type() == QVariant::ByteArray ) {
	QByteArray ba = field->value().toByteArray();
	QString res;
	static const char hexchars[] = "0123456789abcdef";
	for ( int i = 0; i < ba.size(); ++i ) {
	    uchar s = (uchar) ba[i];
	    res += hexchars[s >> 4];
	    res += hexchars[s & 0x0f];
	}
	r = "0x" + res;
    } else {
	r = QSqlDriver::formatValue( field );
    }
    return r;
}

QSqlIndex QTDSDriver::primaryIndex( const QString& tablename ) const
{
    QSqlRecord rec = record( tablename );

    QSqlIndex idx( tablename );
    if ( ( !isOpen() ) || ( tablename.isEmpty() ) )
	return QSqlIndex();

    QSqlQuery t = createQuery();
    t.setForwardOnly( TRUE );
    t.exec( QString( "sp_helpindex '%1'" ).arg( tablename ) );
    if ( t.next() ) {
	QStringList fNames = t.value(2).toString().simplified().split(',');
	QRegExp regx("\\s*(\\S+)(?:\\s+(DESC|desc))?\\s*");
	for( QStringList::Iterator it = fNames.begin(); it != fNames.end(); ++it ) {
	    regx.search( *it );
	    QSqlField f( regx.cap( 1 ), rec.field( regx.cap( 1 ) )->type() );
	    if ( regx.cap( 2 ).toLower() == "desc" ) {
		idx.append( f, TRUE );
	    } else {
		idx.append( f, FALSE );
	    }
	}
	idx.setName( t.value(0).toString().simplified() );
    }
    return idx;
}
