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

#include <qarray.h>
#include <qdatetime.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qvector.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <xdb/xbase.h>
#include <xdb/xbexcept.h>

// #define DEBUG_XBASE
// #define VERBOSE_DEBUG_XBASE

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
	switch ( xbaseTypeToVariant( file.GetFieldType(i) ) ) {
	case QVariant::String:
	case QVariant::CString: {
	    QCString data = v.toString().utf8();
	    rc = file.PutField( i, data.data() );
	    break;
	}
	case QVariant::Date: {
	    QDate d = v.toDate();
	    QString val = QString( QString::number( d.year() ) +
				   QString::number( d.month() ).rightJustify( 2, '0' ) +
				   QString::number( d.day() ).rightJustify( 2, '0' ) );
	    rc = file.PutField( i, val.latin1() );
	    break;
	}
	default:
	    rc = file.PutField( i, v.toString().latin1() );
	    break;
	}
	return rc;
    }
    int getField( int i, QVariant& v )
    {
	int len = file.GetFieldLen( i );
	char buf[ len ];
	int r = file.GetField( i, buf );
	switch ( file.GetFieldType(i) ) {
	case 'N': /* numeric */
	    v = QString(buf).toInt();
	    break;
	case 'F': /* float */
	    v = QString(buf).toDouble();
	    break;
	case 'D': { /* date */
	    QString date( buf );
	    int y = date.mid( 0, 4 ).toInt();
	    int m = date.mid( 4, 2 ).toInt();
	    int d = date.mid( 6, 2 ).toInt();
	    v = QDate( y, m, d );
	    break;
	}
	case 'L': /* logical */
	    v = QVariant( QString(buf).toInt(), 1 );
	    break;
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
	ERROR_RETURN( "No table name specified" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "No fields defined, nothing to create" );
    }
    if ( QFile::exists( env->path() + "/" + name() ) ||
	 QFile::exists( env->path() + "/" + name() + ".dbf" ) ) {
	env->output() << "Table already exists: " << name() << endl;
	return TRUE;
    }
    QArray<xbSchema> xbrec( data.count()+1 ); /* one extra for null entry */
    xbSchema x;
    uint i = 0;
    for ( i = 0; i < data.count(); ++i ) {
	List fieldDescription = data[i].toList();
	if ( fieldDescription.count() != 4 ) {
	    ERROR_RETURN( "Internal error: Bad field description" );
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
	    ERROR_RETURN( "Internal error: Unknown type for field:" + name );
	}
	xbrec[i] = x;
    }
    memset( &x, 0, sizeof(xbSchema) );
    xbrec[ data.count() ] = x;
    d->file.SetVersion( 4 );   /* create dbase IV style files */
    xbShort rc = d->file.CreateDatabase( env->path() + "/" + name().latin1(), xbrec.data(), XB_OVERLAY );
    if ( rc != XB_NO_ERROR ) {
	ERROR_RETURN( "Unable to  create table '" + name() + "': " + QString( xbStrError( rc ) ) );
    }

    d->file.CloseDatabase();   /* Close database and associated indexes */
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
	ERROR_RETURN( "No table name specified" );
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
	    ERROR_RETURN( "Unable to open index: " + QString( xbStrError( rc ) ) );
	}
#ifdef DEBUG_XBASE
	env->output() << env->path() + "/" + indexList[i].latin1() << " index opened..." << flush;
	if ( idx->UniqueIndex() )
	    env->output() << "(unique)" << flush;
#endif
	d->indexes.insert( i, idx );
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
	ERROR_RETURN( "No values to insert" );
    }
    if ( (int)data.count() > d->file.FieldCount() ) {
	ERROR_RETURN( "Too many values" );
    }
    d->file.BlankRecord();
    uint i = 0;
    for ( ; i < data.count(); ++i ) {
	List insertData = data[i].toList();
	if ( !insertData.count() ) {
	    ERROR_RETURN( "Internal error: No insert data" );
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
	if ( xbaseTypeToVariant( d->file.GetFieldType( pos ) ) == QVariant::Date ) {
	    QDate d = val.toDate();
	    if ( !d.isValid() ) {
		ERROR_RETURN( "Invalid date '" + val.toString() + "'" );
	    }
	}
	xbShort rc = d->putField( pos, val );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to put field data: " + QString( xbStrError( rc ) ) );
	}
    }
    xbShort rc = d->file.AppendRecord();
    switch( rc ) {
    case XB_NO_ERROR:
	break;
    default:
	ERROR_RETURN( "Unable to insert record: " + QString( xbStrError( rc ) ) );
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
	ERROR_RETURN( "Internal error: File not open" );
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
	    ERROR_RETURN( "Unable to get record: " + QString( xbStrError( rc  ) ) );
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
	ERROR_RETURN( "Internal error: File not open" );
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

bool FileDriver::fieldDescription( const QString& name, QVariant& v )
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::fieldDescription(string)..." << flush;
#endif
    if ( !isOpen() )
	return FALSE;
    int i = d->file.GetFieldNo( name.latin1() );
    if ( i == -1 ) {
	ERROR_RETURN( UNKNOWN_FIELD_NAME + "'" + name + "'" );
    }
    return fieldDescription( i, v );
}

bool FileDriver::fieldDescription( int i, QVariant& v )
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::fieldDescription(int)..." << flush;
#endif
    if ( !isOpen() )
	return FALSE;
    if ( i == -1 || i > d->file.FieldCount()-1 ) {
	ERROR_RETURN( UNKNOWN_FIELD_NUM + QString::number(i));
    }
    List field;
    QString name = QString( d->file.GetFieldName( i ) ).simplifyWhiteSpace();
    QVariant::Type type = xbaseTypeToVariant( d->file.GetFieldType( i ) );
    int len = d->file.GetFieldLen( i );
    int prec = d->file.GetFieldDecimal( i );
    field.append( name );
    field.append( (int) type );
    field.append( len );
    field.append( prec );
    v = field;
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
	ERROR_RETURN( "Internal error: File not open" );
    }
    if ( !d->marked.count() ) {
	env->setAffectedRows( 0 );
	return TRUE;
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Internal error: No fields defined" );
    }
    if ( (int)data.count() > d->file.FieldCount() ) {
	ERROR_RETURN( "Internal error: Too many fields" );
    }
    uint i = 0;
    for ( ; i < data.count(); ++i ) {
	List updateData = data[i].toList();
	if ( !updateData.count() ) {
	    ERROR_RETURN( "Internal error: No update data" );
	}
	xbShort fieldnum = d->file.GetFieldNo( updateData[0].toString().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "Internal error: Field not found:" + updateData[0].toString() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( updateData[1].type() ) ) {
	    ERROR_RETURN( "Internal error: Bad type:" + updateData[0].toString() );
	}
    }
    for ( i = 0; i < d->marked.count(); ++i ) {
	xbShort rc = d->file.GetRecord( d->marked[i] );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to get record: " + QString( xbStrError( rc ) ) );
	}
	uint j = 0;
	for ( j = 0; j < data.count(); ++j ) {
	    List updateData = data[i].toList();
	    xbShort fieldnum = d->file.GetFieldNo( updateData[0].toString().latin1() );
	    xbShort rc = d->putField( fieldnum, updateData[1] );
	    if ( rc != XB_NO_ERROR ) {
		ERROR_RETURN( "Unable to put field data: " + QString( xbStrError( rc )  ) );
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
	ERROR_RETURN( "Internal error: File not open" );
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
	ERROR_RETURN( "Internal error: File not open" );
    }
    if ( d->file.GetCurRecNo() == 0 ) {
	ERROR_RETURN( "Internal error: Not positioned on valid record" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Internal error: No update data");
    }
    xbShort rc;
    uint i = 0;
    for ( ;  i < data.count(); ++i ) {
	List updateData = data[i].toList();
	if ( updateData.count() != 2 ) {
	    ERROR_RETURN( "Internal error: Bad field description" );
	}
	List fieldDesc = updateData[0].toList();
	QString name = fieldDesc[0].toString();
	xbShort pos = d->file.GetFieldNo( name.latin1() );
	if ( pos == -1 ) {
	    ERROR_RETURN( "Field not found:" + name );
	}
	if ( !name.length() ) {
	    ERROR_RETURN( "Internal error: " + UNKNOWN_FIELD_NUM + QString::number(pos) );
	}
	if ( !canConvert( updateData[1].type(), xbaseTypeToVariant( d->file.GetFieldType( pos ) ) ) ) {
	    QVariant v; v.cast( xbaseTypeToVariant( d->file.GetFieldType( pos ) ) );
	    ERROR_RETURN( "Invalid field type:" + QString(updateData[1].typeName()) +
			  ", expected:" + QString( v.typeName() ) );
	}
	rc = d->putField( pos, updateData[1] );
	if ( rc != XB_NO_ERROR ) {
	    ERROR_RETURN( "Unable to put field data: " + QString( xbStrError( rc ) ) );
	}
    }
    rc = d->file.PutRecord();
    if ( rc != XB_NO_ERROR ) {
	ERROR_RETURN( "Unable to update record: " + QString( xbStrError( rc ) ) );
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
	ERROR_RETURN( "Internal error: no data" );
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
	ERROR_RETURN( "Internal error: No result columns defined" );
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
	List rangeMarkFieldDesc = rangeMarkFieldData[0].toList();
	if ( rangeMarkFieldDesc.count() != 4 ) {
	    ERROR_RETURN( "Internal error: Bad field description");
	}
	QString name = rangeMarkFieldDesc[0].toString();
	QVariant value = rangeMarkFieldData[1];
	xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "Internal error: Field not found:" + name );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( value.type() ) ) {
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
			    List rangeMarkFieldDesc = rangeMarkFieldData[0].toList();
			    QString name = rangeMarkFieldDesc[0].toString();
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
			    if ( dosave )
				saveResult( cols, result );
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
		List rangeMarkFieldDesc = rangeMarkFieldData[0].toList();
		QString name = rangeMarkFieldDesc[0].toString();
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
		if ( dosave )
		    saveResult( cols, result );
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
	ERROR_RETURN( "Internal error: no cols or result" );
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

bool FileDriver::createIndex( const List& data, bool unique )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::createIndex..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "Internal error: File not open" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "Internal error: No fields defined");
    }
    uint i = 0;
    xbShort rc;
    QString indexDesc;
    QVariant::Type indexType = QVariant::Invalid;
    for ( ; i < data.count(); ++i ) {
	List createIndexData = data[i].toList();
	if ( createIndexData.count() != 4 ) {
	    ERROR_RETURN( "Internal error: Bad index data");
	}
	QString name = createIndexData[0].toString();
	QVariant::Type type = (QVariant::Type)createIndexData[1].toInt();
	if ( indexType == QVariant::Invalid )
	    indexType = type;
	if ( type != indexType ) {
	    QVariant v; v.cast( indexType );
	    QVariant b; b.cast( type );
	    ERROR_RETURN( "Unable to create index with conflicting "
			  "types: '" + QString( v.typeName() ) + "', '" + QString( b.typeName() ) + "'" );
	}
	/* create the index description string */
	xbShort fieldnum = d->file.GetFieldNo( name );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "Internal error: Field not found:" + name );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( type ) ) {
	    QVariant v1; v1.cast( type );
	    QVariant v2; v2.cast( xbaseTypeToVariant( d->file.GetFieldType( fieldnum ) ) );
	    ERROR_RETURN( "Incompatible types: '" + QString( v1.typeName() ) + "' and '" +
			  QString( v2.typeName() ) + "'" );
	}
	indexDesc += QString(( indexDesc.length()>0 ? QString("+") : QString::null ) ) + name;
    }
    /* check if index already exists */
    bool forceCreate = TRUE;
    char buf[XB_MAX_NDX_NODE_SIZE];
    if ( d->indexes.count() ) {
	for ( i = 0; i < d->indexes.count(); ++i ) {
	    d->indexes[i]->GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    if ( QString(buf) == indexDesc ) {
		forceCreate = FALSE;
		env->output() << "Index already exists: " << env->path() + "/" + name() << "." << buf << endl;
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
	indexName = fi.baseName();
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
	    ERROR_RETURN( "Unable to build index: " + QString( xbStrError( rc ) ) );
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
	ERROR_RETURN( "Internal error: No file name" );
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
