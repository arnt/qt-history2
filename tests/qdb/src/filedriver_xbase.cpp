/*
  LocalSQL

  Copyright (C) 2001 Trolltech AS

  Contact:
	 Dave Berton (db@trolltech.com)
	 Jasmin Blanchette (jasmin@trolltech.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "sqlinterpreter.h"

#include <xbase.h>
#include <xbexcept.h>

#include <qarray.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qvector.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#define DEBUG_XBASE
// #define VERBOSE_DEBUG_XBASE

class XbaseSys
{
public:
    XbaseSys( const QString& path )
	: dbPath( path )
    {
	checkSystemDir();
    }
    bool setNullInfo( const QString& tablename, const QValueList<uint>& cols )
    {
	if ( !cols.count() )
	    return TRUE;
	xbShort rc = XB_NO_ERROR;
	xbDbf notnullfile( &x );
	QString nnFileName = sysDirPath() + "/" + tablename + "nn.dbf";
	if ( !QFile::exists( nnFileName ) ) {
	    xbSchema notnullrec[] =
	    {
		{ "FIELDNUM", XB_NUMERIC_FLD,     9, 0 },
		{ "",0,0,0 }
	    };
	    notnullfile.SetVersion( 4 );   /* dbase IV files */
	    rc = notnullfile.CreateDatabase( nnFileName, notnullrec, XB_OVERLAY );
	    if ( rc != XB_NO_ERROR )
		return FALSE;
	    notnullfile.CloseDatabase();
	}
	rc = notnullfile.OpenDatabase( nnFileName );
	if ( rc != XB_NO_ERROR )
	    return FALSE;
	for ( uint i = 0; i < cols.count(); ++i ) {
	    notnullfile.BlankRecord();
	    notnullfile.PutField( (short)0, QString::number(cols[i]).latin1() );
	    rc = notnullfile.AppendRecord();
	    if ( rc != XB_NO_ERROR ) {
		notnullfile.CloseDatabase();
		return FALSE;
	    }
	}
	notnullfile.CloseDatabase();
	return TRUE;
    }
    bool nullInfo( const QString& tablename, QValueList<uint>& cols )
    {
	if ( QFile::exists( sysDirPath() + "/" + tablename + "nn.dbf" ) ) {
	    xbDbf notnullfile( &x );
	    xbShort rc = notnullfile.OpenDatabase( sysDirPath() + "/" + tablename + "nn.dbf" );
	    if ( rc != XB_NO_ERROR )
		return FALSE;
	    char buf[10];
	    rc = notnullfile.GetFirstRecord();
	    while( rc == XB_NO_ERROR ) {
		notnullfile.GetField( 1, buf );
		cols.append( QString(buf).toInt() );
		rc = notnullfile.GetNextRecord();
	    }
	    notnullfile.CloseDatabase();
	}
	return TRUE;
    }

private:
    QString sysDirPath()
    {
	return dbPath + "/sys";
    }
    void checkSystemDir()
    {
	QDir dir( dbPath );
	QDir sysdir( sysDirPath() );
	if ( !sysdir.exists() ) {
	    if ( !dir.mkdir( "sys" ) )
		qWarning( "Unable to create database system dir" );
	}
    }
    QString dbPath;
    xbXBase x;
};

static bool canConvert( QVariant::Type t1, QVariant::Type t2 )
{
   switch ( t1 ) {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::String:
    case QVariant::CString:
    case QVariant::Double:
    case QVariant::Bool:
	return TRUE;
    case QVariant::Date:
	switch ( t2 ) {
	case QVariant::Int:
	case QVariant::UInt:
	case QVariant::Double:
	case QVariant::Bool:
	    return FALSE;
	case QVariant::String:
	case QVariant::CString:
	case QVariant::Date:
	    return TRUE;
	default:
	    return FALSE;
	}
    default:
	return 0;
    }
}

static QVariant::Type xbaseTypeToVariant( char type )
{
    switch ( type ) {
    case 'N': /* numeric */
	return QVariant::Int;
    case 'F': /* float */
	return QVariant::Double;
    case 'D': /* date */
	return QVariant::Date;
    case 'L': /* logical */
	return QVariant::Bool;
    case 'C': /* character */
    default:
	return QVariant::String;
    }
}

static char variantToXbaseType( QVariant::Type type )
{
    switch ( type ) {
    case QVariant::Int:
    case QVariant::UInt:
	return XB_NUMERIC_FLD;
    case QVariant::Bool:
	return XB_LOGICAL_FLD;
    case QVariant::String:
    case QVariant::CString:
	return XB_CHAR_FLD;
    case QVariant::Date:
	return XB_DATE_FLD;
    case QVariant::Double:
	return XB_FLOAT_FLD;
    default:
	return 0;
    }
}

#ifdef DEBUG_XBASE
#define ERROR_RETURN(err) do { \
			   env->setLastError( err ); \
			   close(); \
			   env->output() << endl; \
			   return FALSE; \
			  } while(0)
#else
#define ERROR_RETURN(err) do { \
			   env->setLastError( err ); \
			   close(); \
			   return FALSE; \
			  } while(0)
#endif



#define UNKNOWN_FIELD_NAME    QString("Field does not exist: ")
#define UNKNOWN_FIELD_NUM     QString("Field number does not exist: ")
#define UNKNOWN_TABLE         QString("Table does not exist: ")

class FileDriver::Private
{
public:
    Private()
	: file(&x), dopack( FALSE )
    {
	indexes.setAutoDelete( TRUE );
    }
    Private( const Private& )
	: file(&x), dopack( FALSE )
    {
	indexes.setAutoDelete( TRUE );
    }
    Private& operator=( const Private& )
    {
	indexes.setAutoDelete( TRUE );
	dopack = FALSE;
	return *this;
    }
    ~Private()
    {
    }
    xbShort putField( int i, const QVariant& v )
    {
	xbShort rc;
	bool nullify = FALSE;
	if ( v.type() == QVariant::Invalid ) {
	    nullify = TRUE;
	}
	switch ( xbaseTypeToVariant( file.GetFieldType(i) ) ) {
	case QVariant::String:
	case QVariant::CString: {
	    QCString data;
	    if ( nullify )
		data = "\0";
	    else
		data = v.toString().utf8();
	    rc = file.PutField( i, data.data() );
	    break;
	}
	case QVariant::Date: {
	    QDate d = v.toDate();
	    QString val;
	    if ( nullify )
		val = "17520913"; /* an invalid QDate */
	    else
		val = QString( QString::number( d.year() ) +
				   QString::number( d.month() ).rightJustify( 2, '0' ) +
				   QString::number( d.day() ).rightJustify( 2, '0' ) );
	    rc = file.PutField( i, val.latin1() );
	    break;
	}
	case QVariant::Bool: {
	    QString val;
	    if ( nullify )
		val = "?";
	    else
		val = QString::number(v.toBool());
	    rc = file.PutField( i, val.latin1() );
	    break;
	}
	default: {
	    QCString data;
	    if ( nullify )
		data = "\0";
	    else
		data = v.toString().utf8();
	    rc = file.PutField( i, data.data() );
	    break;
	}
	}
	return rc;
    }
    int getField( int i, QVariant& v )
    {
	int len = file.GetFieldLen( i );
	char buf[ len ];
	int r = file.GetField( i, buf );
	switch ( file.GetFieldType(i) ) {
	case 'N': { /* numeric */
	    QString d( buf );
	    if ( d.simplifyWhiteSpace().length() == 0 ) /* null? */
		v = LOCALSQL_NULL;
	    else
		v = d.toInt();
	    break;
	}
	case 'F': { /* float */
	    QString d( buf );
	    if ( d.simplifyWhiteSpace().length() == 0 ) /* null? */
		v = LOCALSQL_NULL;
	    else
		v = d.toDouble();
	    break;
	}
	case 'D': { /* date */
	    QString date( buf );
	    int y = date.mid( 0, 4 ).toInt();
	    int m = date.mid( 4, 2 ).toInt();
	    int d = date.mid( 6, 2 ).toInt();
	    if ( !QDate::isValid( y, m, d ) ) /* null? */
		v = LOCALSQL_NULL;
	    else
		v = QDate( y, m, d );
	    break;
	}
	case 'L': { /* logical */
	    QString d(buf);
	    if ( d == "?" ) /* null? */
		v = LOCALSQL_NULL;
	    else
		v = QVariant( QString(buf).toInt(), 1 );
	    break;
	}
	case 'C': /* character */
	default:
	    QString utf8 = QString::fromUtf8( buf ).simplifyWhiteSpace();
	    v = utf8;
	    break;
	}
	return r;
    }
    xbXBase x;		/* initialize xbase  */
    xbDbf file;		/* class for table   */
    QValueList<int> marked;
    QVector<xbNdx> indexes;
    QValueList<uint> notnulls; /* not null columns */
    bool dopack;
};

FileDriver::FileDriver( LocalSQLEnvironment* environment, const QString& name = QString::null )
    : nm( name ), env( environment )
{
    d = new Private();
    setIsOpen( FALSE );
}

FileDriver::~FileDriver()
{
    delete d;
}

FileDriver::FileDriver( const FileDriver& other )
    : LocalSQLFileDriver(), nm( other.nm ), env( other.env )
{
    *d = *other.d;
    setIsOpen( FALSE );
}

FileDriver& FileDriver::operator=( const FileDriver& other )
{
    env = other.env;
    nm = other.nm;
    *d = *other.d;
    setIsOpen( FALSE );
    return *this;
}

bool FileDriver::create( const List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::create..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "Unable to create table, no table name specified" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Unable to create table, no fields defined" );
    }
    if ( QFile::exists( env->path() + "/" + name() ) ||
	 QFile::exists( env->path() + "/" + name() + ".dbf" ) ) {
	env->output() << "Table already exists: " << name() << endl;
	return TRUE;
    }
    d->notnulls.clear();
    QArray<xbSchema> xbrec( data.count()+1 ); /* one extra for null entry */
    xbSchema x;
    uint i = 0;
    for ( i = 0; i < data.count(); ++i ) {
	List fieldDescription = data[i].toList();
	if ( fieldDescription.count() != 5 ) {
	    ERROR_RETURN( "Internal error: Unable to create table, expected 5 field descriptors, got " +
			  QString::number( fieldDescription.count() ) );
	}
	QString name = fieldDescription[0].toString();
	int namelen = QMAX( name.length(), 11 );
	QVariant::Type type = (QVariant::Type)fieldDescription[1].toInt();
	if ( type == QVariant::DateTime )
	    type = QVariant::Date;
	int len = fieldDescription[2].toInt();
	int prec = fieldDescription[3].toInt();
	qstrncpy( x.FieldName, name.latin1(), namelen );
	x.FieldName[namelen] = 0;
	x.Type = variantToXbaseType( type );
	switch( x.Type ) {
	case  XB_DATE_FLD:
	    x.FieldLen = 8;
	    x.NoOfDecs = 0;
	    break;
	case XB_LOGICAL_FLD:
	    x.FieldLen = 1;
	    x.NoOfDecs = 0;
	    break;
	default:
	    x.FieldLen = len;
	    x.NoOfDecs = prec;
	    break;
	}
	if ( !x.Type ) {
	    ERROR_RETURN( "Internal error: Unable to create table, unknown type for field:" + name );
	}
	xbrec[i] = x;
	if ( fieldDescription[4].toBool() == TRUE )
	    d->notnulls.append( i );
    }
    memset( &x, 0, sizeof(xbSchema) );
    xbrec[ data.count() ] = x;
    d->file.SetVersion( 4 );   /* dbase IV files */
    xbShort rc = d->file.CreateDatabase( env->path() + "/" + name().latin1(), xbrec.data(), XB_OVERLAY );
    if ( rc != XB_NO_ERROR ) {
	ERROR_RETURN( "Unable to create table '" + name() + "': " + QString( xbStrError( rc ) ) );
    }
    d->file.CloseDatabase();   /* Close database and associated indexes */
    /* not null specification */
    if ( d->notnulls.count() ) {
	XbaseSys sys( env->path() );
	if ( !sys.setNullInfo( name(), d->notnulls ) ) {
	    ERROR_RETURN( "Unable to store NOT NULL information" );
	}
    }
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::open()
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::open..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "Unable to open table, no table name specified" );
    }
    xbShort rc = d->file.OpenDatabase( env->path() + "/" + name().latin1() );
    if ( rc != XB_NO_ERROR ) {
	ERROR_RETURN( UNKNOWN_TABLE + "'" + name() + "' [" + QString ( xbStrError( rc ) ) + "]" );
    }
#ifdef DEBUG_XBASE
    env->output() << env->path() + "/" + name().latin1() << " opened..." << flush;
#endif
    setIsOpen( TRUE );
    /* open all associated indexes */
    QFileInfo fi( env->path() + "/" + name() );
    QString basename = fi.baseName();
    QDir dir( env->path() );
    QStringList indexList = dir.entryList(  basename + "*.ndx", QDir::Files );
    d->indexes.resize( indexList.count() );
    for ( uint i = 0; i < indexList.count(); ++i ) {
	xbNdx* idx = new xbNdx( &d->file );
	rc = idx->OpenIndex( env->path() + "/" + indexList[i].latin1() );
	if ( rc != XB_NO_ERROR ) {
	    delete idx;
	    ERROR_RETURN( "Unable to open table index: " + QString( xbStrError( rc ) ) );
	}
#ifdef DEBUG_XBASE
	env->output() << env->path() + "/" + indexList[i].latin1() << " index opened..." << flush;
	if ( idx->UniqueIndex() )
	    env->output() << "(unique)" << flush;
#endif
	d->indexes.insert( i, idx );
    }
    /* determine NOT NULL columns */
    d->notnulls.clear();
    XbaseSys sys( env->path() );
    if ( !sys.nullInfo( name(), d->notnulls ) ) {
	ERROR_RETURN( "Unable to load null info: " + QString ( xbStrError( rc ) ) );
    }
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

uint FileDriver::size() const
{
    return d->file.NoOfRecords();
}

QValueList<uint> FileDriver::columnSizes() const
{
    QValueList<uint> sizes;
    for ( uint i = 0; i < count(); ++i )
	sizes += d->file.GetFieldLen( i );
    return sizes;
}

QValueList<uint> FileDriver::columnPrecs() const
{
    QValueList<uint> precs;
    for ( uint i = 0; i < count(); ++i )
	precs += d->file.GetFieldDecimal( i );
    return precs;
}

QValueList<QVariant::Type> FileDriver::columnTypes() const
{
    QValueList<QVariant::Type> types;
    for ( uint i = 0; i < count(); ++i )
	types += xbaseTypeToVariant( d->file.GetFieldType( i ) );
    return types;
}

QStringList FileDriver::columnNames() const
{
    QStringList l;
    for ( uint i = 0; i < count(); ++i )
	l += QString( d->file.GetFieldName( i ) ).simplifyWhiteSpace();
    return l;
}

QStringList FileDriver::notNullColumnNames() const
{
    QStringList names = columnNames();
    QStringList nullnames;
    for ( uint i = 0; i < d->notnulls.count(); ++i )
	nullnames += names[d->notnulls[i]];
    return nullnames;
}

uint FileDriver::count() const
{
    return d->file.FieldCount();
}

QStringList FileDriver::primaryIndex()
{
    QStringList idx;
    for ( uint i = 0; i < d->indexes.count(); ++i ) {
	if ( d->indexes[i]->UniqueIndex() ) { /* first unique index */
	    char buf[XB_MAX_NDX_NODE_SIZE];
	    d->indexes[i]->GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    idx = QStringList::split( "+", QString( buf ) );
	    break;
	}
    }
    return idx;

}

QStringList FileDriver::indexNames()
{
    QStringList idx;
    for ( uint i = 0; i < d->indexes.count(); ++i ) {
	char buf[XB_MAX_NDX_NODE_SIZE];
	d->indexes[i]->GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	idx += QString(buf);
    }
    return idx;
}

bool FileDriver::close()
{
    if ( !isOpen() )
	return TRUE;
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::close..." << flush;
#endif
    if ( !commit() )
	return FALSE;
    d->file.CloseDatabase();   /* Close database and associated indexes */
    setIsOpen( FALSE );
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::insert( const List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::insert..." << flush;
#endif
    env->setAffectedRows( -1 );
    if ( !isOpen() )
	return FALSE;
    if ( !data.count() ) {
	ERROR_RETURN( "Unable to insert, no values specified" );
    }
    if ( (int)data.count() > d->file.FieldCount() ) {
	ERROR_RETURN( "Unable to insert, too many values" );
    }
    d->file.BlankRecord();
    uint i = 0;
    for ( ; i < data.count(); ++i ) {
	List insertData = data[i].toList();
	if ( !insertData.count() ) {
	    ERROR_RETURN( "Internal error: Unable to insert, no insert data" );
	}
	QString name;
	int pos;
	if ( insertData[0].type() == QVariant::String || insertData[0].type() == QVariant::CString ) {
	    name = insertData[0].toString();
	    pos = d->file.GetFieldNo( name );
	} else {
	    pos = insertData[0].toInt();
	    name = QString( d->file.GetFieldName( pos ) ).simplifyWhiteSpace();
	}
	if ( pos == -1 ) {
	    ERROR_RETURN( UNKNOWN_FIELD_NAME + "'" + name + "'" );
	}
	if ( !name.length() ) {
	    ERROR_RETURN( UNKNOWN_FIELD_NUM + QString::number(pos) );
	}
	QVariant val = insertData[1];
	if ( d->notnulls.contains( pos ) && val.type() == QVariant::Invalid ) {
	    ERROR_RETURN("Unable to insert NULL value in NOT NULL field: " + name );
	}
	if ( xbaseTypeToVariant( d->file.GetFieldType( pos ) ) == QVariant::Date ) {
	    if ( val.type() != QVariant::Invalid ) { /* which would indicate a NULL date */
		QDate d = val.toDate();
		if ( !d.isValid() ) {
		    ERROR_RETURN( "Unable to insert, invalid date '" + val.toString() + "'" );
		}
	    }
	}
	xbShort rc = d->putField( pos, val );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to insert, unable to put field data: " + QString( xbStrError( rc ) ) +
			  " for field: " + name );
	}
    }
    xbShort rc = d->file.AppendRecord();
    switch( rc ) {
    case XB_NO_ERROR:
	break;
    default:
	ERROR_RETURN( "Unable to insert: " + QString( xbStrError( rc ) ) );
	break;
    }
    env->setAffectedRows( 1 );
    d->dopack = TRUE;
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::next()
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::next..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: Unable to get next record, file not open" );
    }
    xbShort rc = XB_NO_ERROR;
    if ( d->file.GetCurRecNo() == 0 ) {
	rc = d->file.GetFirstRecord();
    } else {
	rc = d->file.GetNextRecord();
    }
    if ( rc != XB_NO_ERROR )
	return FALSE;
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::mark()
{
    if ( !isOpen() )
	return FALSE;
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::mark: marking record " << d->file.GetCurRecNo() << endl;
#endif
    d->marked.append( d->file.GetCurRecNo() );
    return TRUE;
}

bool FileDriver::unmark()
{
    if ( !isOpen() )
	return FALSE;
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::unmark: unmarking record " << d->file.GetCurRecNo() << endl;
#endif
    if ( d->marked.find( d->file.GetCurRecNo() ) == d->marked.end() )
	return TRUE;
    d->marked.remove( d->marked.find( d->file.GetCurRecNo() ) );
    setMarkedAt( markedAt() - 1 );
    return TRUE;
}

bool FileDriver::deleteMarked()
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::deleteMarked..." << flush;
#endif
    env->setAffectedRows( -1 );
    if ( !isOpen() )
	return FALSE;
    if ( !d->marked.count() ) {
	env->setAffectedRows( 0 );
	return TRUE;
    }
    for ( uint i = 0; i < d->marked.count(); ++i ) {
	xbShort rc = d->file.GetRecord( d->marked[i] );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to get record for delete: " + QString( xbStrError( rc  ) ) );
	}
	rc = d->file.DeleteRecord();
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to delete record: " + QString( xbStrError( rc  ) ) );
	}
    }
    env->setAffectedRows( d->marked.count() );
    d->dopack = TRUE;
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::commit()
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::commit..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: unable to commit, file not open" );
    }
    if ( d->dopack ) {
	d->file.PackDatabase( F_SETLKW );
	d->dopack = FALSE;
    }
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::field( const QString& name, QVariant& v )
{
    if ( !isOpen() )
	return FALSE;
    int i = d->file.GetFieldNo( name.latin1() );
    if ( i == -1 ) {
	ERROR_RETURN( "Field not found:" + name );
    }
    return field( i, v );
}

bool FileDriver::field( uint i, QVariant& v )
{
    if ( !isOpen() )
	return FALSE;
    if ( d->file.GetCurRecNo() < 0 ) {
	ERROR_RETURN( "Internal error: Not on valid record" );
    }
    if ( (int)i > d->file.FieldCount()-1 ) {
	ERROR_RETURN( UNKNOWN_FIELD_NUM + QString::number(i) );
    }
    return d->getField( i, v );
}

bool FileDriver::fieldType( const QString& name, QVariant& v )
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::fieldTypeInfo(string)..." << flush;
#endif
    if ( !isOpen() )
	return FALSE;
    int i = d->file.GetFieldNo( name.latin1() );
    if ( i == -1 ) {
	ERROR_RETURN( UNKNOWN_FIELD_NAME + "'" + name + "'" );
    }
    return fieldType( i, v );
}

bool FileDriver::fieldType( int i, QVariant& v )
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::fieldTypeInfo(int)..." << flush;
#endif
    if ( !isOpen() )
	return FALSE;
    if ( i == -1 || i > d->file.FieldCount()-1 ) {
	ERROR_RETURN( UNKNOWN_FIELD_NUM + QString::number(i));
    }
    QVariant::Type type = xbaseTypeToVariant( d->file.GetFieldType( i ) );
    v = (int) type;
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::updateMarked( const List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::updateMarked..." << flush;
#endif
    env->setAffectedRows( -1 );
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: unable to update marked, file not open" );
    }
    if ( !d->marked.count() ) {
	env->setAffectedRows( 0 );
	return TRUE;
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Internal error: Unable to update marked, no fields defined" );
    }
    if ( (int)data.count() > d->file.FieldCount() ) {
	ERROR_RETURN( "Internal error: Unable to update marked, too many fields" );
    }
    uint i = 0;
    for ( ; i < data.count(); ++i ) {
	List updateData = data[i].toList();
	if ( !updateData.count() ) {
	    ERROR_RETURN( "Internal error: Unable to update marked, no update data" );
	}
	xbShort fieldnum = d->file.GetFieldNo( updateData[0].toString().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "Internal error: Unable to update marked. field not found:" + updateData[0].toString() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( updateData[1].type() ) ) {
	    ERROR_RETURN( "Internal error: Unable to update marked, bad type:" + updateData[0].toString() );
	}
    }
    for ( i = 0; i < d->marked.count(); ++i ) {
	xbShort rc = d->file.GetRecord( d->marked[i] );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to get record for update: " + QString( xbStrError( rc ) ) );
	}
	uint j = 0;
	for ( j = 0; j < data.count(); ++j ) {
	    List updateData = data[i].toList();
	    xbShort fieldnum = d->file.GetFieldNo( updateData[0].toString().latin1() );
	    xbShort rc = d->putField( fieldnum, updateData[1] );
	    if ( rc != XB_NO_ERROR ) {
		ERROR_RETURN( "Unable to put record field data: " + QString( xbStrError( rc )  ) );
	    }
	}
	rc = d->file.PutRecord();
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to update record: " + QString( xbStrError( rc ) ) );
	}
    }
    env->setAffectedRows( d->marked.count() );
    d->dopack = TRUE;
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::rewindMarked()
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::rewindMarked..." << flush;
#endif
    setMarkedAt( -1 );
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::nextMarked()
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::nextMarked..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: Unable to get next marked record, file not open" );
    }
    int next = markedAt() + 1;
    if ( next > (int)d->marked.count()-1 )
	return FALSE;
    if ( d->file.GetRecord( d->marked[next] ) != XB_NO_ERROR )
	return FALSE;

    setMarkedAt( next );
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::update( const List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::update..." << flush;
#endif
    env->setAffectedRows( -1 );
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: Unable to update, file not open" );
    }
    if ( d->file.GetCurRecNo() == 0 ) {
	ERROR_RETURN( "Internal error: Unable to update, not positioned on valid record" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Internal error: Unable to update, no update data");
    }
    xbShort rc;
    uint i = 0;
    for ( ;  i < data.count(); ++i ) {
	List updateData = data[i].toList();
	if ( updateData.count() != 2 ) {
	    ERROR_RETURN( "Internal error: Unable to update, expected 2 field descriptors, got " +
			  QString::number( updateData.count() ) );
	}
	QString name = updateData[0].toString();
	xbShort pos = d->file.GetFieldNo( name.latin1() );
	if ( pos == -1 ) {
	    ERROR_RETURN( "Unable to update, field not found:" + name );
	}
	if ( !name.length() ) {
	    ERROR_RETURN( "Internal error: Unable to update, " + UNKNOWN_FIELD_NUM + QString::number(pos) );
	}
	if ( !canConvert( updateData[1].type(), xbaseTypeToVariant( d->file.GetFieldType( pos ) ) ) ) {
	    QVariant v; v.cast( xbaseTypeToVariant( d->file.GetFieldType( pos ) ) );
	    ERROR_RETURN( "Unable to update, invalid field type:" + QString(updateData[1].typeName()) +
			  ", expected:" + QString( v.typeName() ) );
	}
	rc = d->putField( pos, updateData[1] );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to update field data: " + QString( xbStrError( rc ) ) );
	}
    }
    rc = d->file.PutRecord();
    if ( rc != XB_NO_ERROR ) {
	ERROR_RETURN( "Unable to update: " + QString( xbStrError( rc ) ) );
    }
    env->setAffectedRows( 1 );
    d->dopack = TRUE;
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::rangeAction( const List* data, const List* cols,
			      LocalSQLResultSet* result )
{
    if ( !data ) {
	ERROR_RETURN( "Internal error: Unable to perform range action, no data" );
    }
    bool dosave = cols && result;
    bool domark = !dosave;
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::rangeAction...";
    if ( domark )
	env->output() << "marking...";
    if ( dosave )
	env->output() << "saving...";
    env->output() << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: File not open" );
    }
    if ( dosave && !cols->count() ) {
	ERROR_RETURN( "Internal error: Unable to perform range action, no result columns defined" );
    }
    if ( domark ) {
	clearMarked();
    }
    bool forceScan = TRUE;
    xbShort rc = XB_NO_ERROR;
    QString indexDesc;
    uint i = 0;
    /* metadata check and build index string */
    for ( i = 0; i < data->count(); ++i ) {
	List rangeMarkFieldData = (*data)[i].toList();
	if ( rangeMarkFieldData.count() != 2 ) {
	    ERROR_RETURN( "Internal error: Bad range data");
	}
	QString name = rangeMarkFieldData[0].toString();
	QVariant value = rangeMarkFieldData[1];
	xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "Internal error: Field not found:" + name );
	}
	if ( !canConvert( xbaseTypeToVariant( d->file.GetFieldType( fieldnum ) ),
					      value.type() ) ) {
	    QVariant v; v.cast( xbaseTypeToVariant( d->file.GetFieldType( fieldnum ) ) );
	    ERROR_RETURN( "Incompatible types: '" + QString( value.typeName() ) + "' and '" +
			  QString( v.typeName() ) + "'" );
	}
	indexDesc += QString(( indexDesc.length()>0 ? QString("+") : QString::null ) ) +
		     name;
    }
    /* check if index already exists */
    char buf[XB_MAX_NDX_NODE_SIZE];
    if ( d->indexes.count() ) {
	for ( i = 0; i < d->indexes.count(); ++i ) {
	    d->indexes[i]->GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    if ( QString(buf) == indexDesc ) {
#ifdef DEBUG_XBASE
		env->output() << "using index scan on:" << buf << "..." << flush;
#endif
		forceScan = FALSE;
		/* create search value */
		QString searchValue;
		bool numeric = TRUE;
		for ( uint j = 0; j < data->count(); ++j ) {
		    QVariant val = (*data)[j].toList()[1];
		    searchValue += val.toString();
		    if ( val.type() == QVariant::String ||
			 val.type() == QVariant::CString )
			numeric = FALSE;
		}
		if ( numeric ) {
#ifdef DEBUG_XBASE
		    env->output() << "numeric value:'" << searchValue.latin1() << "'..." << flush;
#endif
		    rc = d->indexes[i]->FindKey( searchValue.toDouble() );
		} else {
#ifdef DEBUG_XBASE
		    env->output() << "string value:'" << searchValue.latin1() << "'..." << flush;
#endif
		    rc = d->indexes[i]->FindKey( searchValue.utf8() );
		}
		if ( rc == XB_FOUND ) {
#ifdef DEBUG_XBASE
		    env->output() << "key found, scanning forward..." << flush;
#endif
		    rc = XB_NO_ERROR;
		    /* found a key, now scan until we hit a new key value */
		    for ( ; rc == XB_NO_ERROR ; ) {
			bool actionOK = TRUE;
			for ( uint k = 0; k < data->count(); ++k ) {
			    List rangeMarkFieldData = (*data)[k].toList();
			    QString name = rangeMarkFieldData[0].toString();
			    QVariant value = rangeMarkFieldData[1];
			    xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
			    QVariant v;
			    field( fieldnum, v );
			    if ( v != value ) {
				actionOK = FALSE;
				break;
			    }
			}
			if ( actionOK ) {
			    if ( domark )
				mark();
			    if ( dosave ) {
				if ( !saveResult( cols, result ) )
				    return FALSE;
			    }
			} else
			    break;
			rc = d->indexes[i]->GetNextKey();
		    }
		}
		break;
	    }
	}
    }
    if ( forceScan ) {
	/* use brute force */
#ifdef DEBUG_XBASE
	env->output() << "using full table scan..." << flush;
#endif
	rc = d->file.GetFirstRecord();
	while ( rc == XB_NO_ERROR ) {
	    bool actionOK = TRUE;
	    for ( i = 0; i < data->count(); ++i ) {
		List rangeMarkFieldData = (*data)[i].toList();
		QString name = rangeMarkFieldData[0].toString();
		QVariant value = rangeMarkFieldData[1];
		xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
		QVariant v;
		field( fieldnum, v );
		if ( v != value ) {
		    actionOK = FALSE;
		    break;
		}
	    }
	    if ( actionOK ) {
		if ( domark )
		    mark();
		if ( dosave ) {
		    if ( !saveResult( cols, result ) )
			return FALSE;
		}
	    }
	    rc = d->file.GetNextRecord();
	}
    }
#ifdef DEBUG_XBASE
    if ( domark ) {
	env->output() << "recs marked:" << d->marked.count() << " " << flush;
	rewindMarked();
    }
     if ( dosave )
	 env->output() << "recs saved:" << result->size() << " " << flush;
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::saveResult( const List* cols, LocalSQLResultSet* result )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::saveResult..." << flush;
#endif
    if ( !cols || !result ) {
	ERROR_RETURN( "Internal error: Unable to save result, no columns or no result" );
    }
    /* build a Record from cols, which may be a field desc
       or a literal value
     */
    Record rec;
    for ( uint i = 0; i < cols->count(); ++i ) {
	QVariant v;
	if ( (*cols)[i].type() == QVariant::List ) { /* field desc */
	    if ( !field( (*cols)[i].toList()[0].toString(), v ) ) {
		ERROR_RETURN( UNKNOWN_FIELD_NAME + "'" + (*cols)[i].toString() + "'" );
	    }
	} else { /* literal value */
	    v = (*cols)[i];
	}
	rec.append( v );
    }
    if ( !result->append( rec ) ) {
	return FALSE;
    }
#ifdef DEBUG_XBASE
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::rangeSave( const List& data, const List& cols, LocalSQLResultSet* result )
{
    return rangeAction( &data, &cols, result );
}

bool FileDriver::rangeMark( const List& data )
{
    return rangeAction( &data, 0 , 0 );
}

bool FileDriver::markAll()
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::markAll..." << flush;
#endif
    int currentAt = d->file.GetCurRecNo();
    xbShort rc = d->file.GetFirstRecord();
    clearMarked();
    while( rc == XB_NO_ERROR ) {
	d->marked.append( d->file.GetCurRecNo() );
	rc = d->file.GetNextRecord();
    }
    d->file.GetRecord( currentAt );
#ifdef DEBUG_XBASE
    env->output() << d->marked.count() << " recs marked...success" << endl;
#endif
     rewindMarked();
     return TRUE;
}

bool FileDriver::createIndex( const List& data, bool unique, bool notnull )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::createIndex..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: Unable to create index, file not open" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Internal error: Unable to create index, no fields defined");
    }
    QValueList<uint> notnulls;
    uint i = 0;
    xbShort rc;
    QString indexDesc;
    QVariant::Type indexType = QVariant::Invalid;
    for ( ; i < data.count(); ++i ) {
	QString name = data[i].toString();
	/* create the index description string */
	xbShort fieldnum = d->file.GetFieldNo( name );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "Internal error: Unable to create index, field not found:" + name );
	}
	if ( indexType == QVariant::Invalid ) /* save type of first indexed field */
	    indexType = xbaseTypeToVariant( d->file.GetFieldType( fieldnum ) );
	if ( !canConvert( xbaseTypeToVariant( d->file.GetFieldType( fieldnum ) ),
			  indexType ) ) {
	    QVariant v1; v1.cast( indexType );
	    QVariant v2; v2.cast( xbaseTypeToVariant( d->file.GetFieldType( fieldnum ) ) );
	    ERROR_RETURN( "Unable to create index, incompatible index field types: '" +
			  QString( v1.typeName() ) + "' and '" +
			  QString( v2.typeName() ) + "'" );
	}
	indexDesc += QString(( indexDesc.length()>0 ? QString("+") : QString::null ) ) + name;
	if ( notnull && !d->notnulls.contains( fieldnum ) )
	    notnulls.append( fieldnum );
    }
    /* check if index already exists */
    bool forceCreate = TRUE;
    char buf[XB_MAX_NDX_NODE_SIZE];
    if ( d->indexes.count() ) {
	for ( i = 0; i < d->indexes.count(); ++i ) {
	    d->indexes[i]->GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    if ( QString(buf) == indexDesc ) {
		forceCreate = FALSE;
		env->output() << "Unable to create index, index already exists: " <<
		    env->path() + "/" + name() << "." << buf << endl;
		break;
	    }
	}
    }
    QString indexName;
    if ( forceCreate ) {
	/* create the index on disk */
#ifdef DEBUG_XBASE
	env->output() << "creating index..." << flush;
	if ( unique )
	    env->output() << "(unique)" << flush;
#endif
	xbNdx* idx = new xbNdx( &d->file );
	d->indexes.resize( d->indexes.size()+1 );
	d->indexes.insert( d->indexes.size()-1, idx );
	/* get unique index name based on file name */
	QFileInfo fi( env->path() + "/" + name() );
	QString basename = fi.baseName();
	indexName = basename;
	i = 1;
	fi.setFile( env->path() + "/" + indexName + QString::number(i) + ".ndx" );
	while ( fi.exists() ) {
	    i++;
	    fi.setFile( env->path() + "/" + indexName + QString::number(i) + ".ndx" );
	}
	indexName = fi.fileName();
	rc = idx->CreateIndex( env->path() + "/" + indexName.latin1(), indexDesc.latin1(),
			       unique ? XB_UNIQUE : XB_NOT_UNIQUE, XB_OVERLAY );
	if ( rc != XB_NO_ERROR ) {
	    QFile::remove( env->path() + "/" + indexName );
	    ERROR_RETURN( "Unable to create index: " + QString( xbStrError( rc ) ) );
	}
	/* build index */
	if ( ( rc = idx->ReIndex() ) != XB_NO_ERROR ) {
	    QFile::remove( env->path() + "/" + indexName );
	    ERROR_RETURN( "Unable to create index, unable to build index: " + QString( xbStrError( rc ) ) );
	}
	/* save not null info */
	if ( notnulls.count() ) {
	    XbaseSys sys( env->path() );
	    if ( !sys.setNullInfo( name(), notnulls ) ) {
		ERROR_RETURN( "Unable to store NOT NULL information creating index: " +
			      QString( xbStrError( rc ) ) );
	    }
	}
    }
#ifdef DEBUG_XBASE
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::drop()
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::drop..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "Internal error: unable to drop table, no table name" );
    }
    if ( isOpen() )
	close();
    QFileInfo fi( env->path() + "/" + name() );
    QString basename = fi.baseName();
    QDir dir( env->path() + "/" );
    dir.setNameFilter( basename + "*.ndx; " + basename + "*.dbf " );
    QStringList indexList = dir.entryList( QDir::Files );
    if ( indexList.count() == 0 ) {
	env->output() <<  "Warning: " << name() << " does not exist" << endl;
    }
#ifdef DEBUG_XBASE
    env->output() << "Removing: " << indexList.join( "," ) << endl;
#endif
    for ( uint i = 0; i < indexList.count(); ++i )
	QFile::remove( indexList[i] );
    QFile::remove( env->path() + "/" + basename + "nn.dbf" );
#ifdef DEBUG_XBASE
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::clearMarked()
{
    d->marked.clear();
    return TRUE;
}

bool FileDriver::star( QVariant& v )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::star..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: Unable to get star values, file not open" );
    }
    List list;
    for ( uint i = 0; i < count(); ++i ) {
	QVariant val;
	if ( !field( i, val ) )
	    return FALSE;
	list.append( val );
    }
    v = list;
#ifdef DEBUG_XBASE
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::starDescription( QVariant& v )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::starDescription..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: Unable to get star description, file not open" );
    }
    List list;
    QStringList names = columnNames();
    QValueList<QVariant::Type> types = columnTypes();
    for ( uint i = 0; i < names.count(); ++i ) {
	List fieldDescription;
	fieldDescription.append( names[i] );
	fieldDescription.append( (int)types[i] );
	list.append( fieldDescription );
    }
    v = list;
#ifdef DEBUG_XBASE
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::isNull( uint i, bool& v )
{
    QVariant var;
    if ( !field( i, var ) )
	return FALSE;
    v = ( var.type() == QVariant::Invalid );
    return TRUE;
}

bool FileDriver::isNull( const QString& name, bool& v )
{
    QVariant var;
    if ( !field( name, var ) )
	return FALSE;
    v = ( var.type() == QVariant::Invalid );
    return TRUE;
}
