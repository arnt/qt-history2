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

#define DEBUG_XBASE 1
//#define VERBOSE_DEBUG_XBASE

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



class FileDriver::Private
{
public:
    Private() : file(&x) { indexes.setAutoDelete( TRUE ); }
    Private( const Private& ) : file(&x) { indexes.setAutoDelete( TRUE ); }
    Private& operator=( const Private& )
    {
	return *this;
    }
    xbXBase x;		/* initialize xbase  */
    xbDbf file;		/* class for table   */
    QValueList<int> marked;
    QVector<xbNdx> indexes;
};

FileDriver::FileDriver( qdb::Environment* environment, const QString& name = QString::null )
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
    : qdb::FileDriver(), nm( other.nm ), env( other.env )
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

bool FileDriver::create( const qdb::List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::create..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "internal error:FileDriver::create: no name specified" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "internal error:FileDriver::create: no fields defined" );
    }
    QArray<xbSchema> xbrec( data.count()+1 ); /* one extra for null entry */
    xbSchema x;
    uint i = 0;
    for ( i = 0; i < data.count(); ++i ) {
	qdb::List fieldDescription = data[i].toList();
	if ( fieldDescription.count() != 4 ) {
	    ERROR_RETURN( "internal error:FileDriver::create: bad field description" );
	}
	QString name = fieldDescription[0].toString();
	int namelen = QMAX( name.length(), 11 );
	QVariant::Type type = (QVariant::Type)fieldDescription[1].toInt();
	int len = fieldDescription[2].toInt();
	int prec = fieldDescription[3].toInt();
	qstrncpy( x.FieldName, name.latin1(), namelen );
	x.FieldName[namelen] = 0;
	x.Type = variantToXbaseType( type );
	x.FieldLen = len;
	x.NoOfDecs = prec;
	xbrec[i] = x;
    }
    memset( &x, 0, sizeof(xbSchema) );
    xbrec[ data.count() ] = x;
    d->file.SetVersion( 4 );   /* create dbase IV style files */
    xbShort rc;
    if ( ( rc = d->file.CreateDatabase( name().latin1(), xbrec.data(), XB_OVERLAY ) )
	 != XB_NO_ERROR ) {
	ERROR_RETURN( "internal error:FileDriver::create: error creating database" );
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
	ERROR_RETURN( "internal error:FileDriver::open: no file name" );
    }
    if ( d->file.OpenDatabase( name().latin1() ) != XB_NO_ERROR ) {
	ERROR_RETURN( "internal error:FileDriver::open: unable to open " + name() );
    }
#ifdef DEBUG_XBASE
    env->output() << name().latin1() << " opened..." << flush;
#endif
    setIsOpen( TRUE );
    /* open all associated indexes */
    QFileInfo fi( name() );
    QString basename = fi.baseName();
    QDir dir;
    QStringList indexList = dir.entryList( basename + "*.ndx", QDir::Files );
    d->indexes.resize( indexList.count() );
    for ( uint i = 0; i < indexList.count(); ++i ) {
	xbNdx* idx = new xbNdx( &d->file );
	if ( idx->OpenIndex( indexList[i].latin1() ) != XB_NO_ERROR ) {
	    delete idx;
	    ERROR_RETURN( "internal error:FileDriver::open: unable to open index " + indexList[i] );
	}
#ifdef DEBUG_XBASE
	env->output() << indexList[i].latin1() << " index opened..." << flush;
#endif
	d->indexes.insert( i, idx );
    }
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

uint FileDriver::fieldCount() const
{
    return d->file.FieldCount();
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

bool FileDriver::insert( const qdb::List& data )
{
    if ( !isOpen() )
	return FALSE;
    if ( !data.count() ) {
	ERROR_RETURN( "internal error:FileDriver::insert: no values");
    }
    if ( (int)data.count() > d->file.FieldCount() ) {
	ERROR_RETURN( "internal error:FileDriver::insert: too many values");
    }
    d->file.BlankRecord();
    uint i = 0;
    for ( ; i < data.count(); ++i ) {
	qdb::List insertData = data[i].toList();
	if ( !insertData.count() ) {
	    ERROR_RETURN( "internal error:FileDriver::insert: no insert data" );
	}
	QString name;
	int pos;
	if ( insertData[0].type() == QVariant::String || insertData[0].type() == QVariant::CString ) {
	    name = insertData[0].toString();
	    pos = d->file.GetFieldNo( name );
	} else {
	    pos = insertData[0].toInt();
	    name = d->file.GetFieldName( pos );
	}
	if ( pos == -1 ) {
	    ERROR_RETURN( "internal error:FileDriver::insert: unknown field: " + name );
	}
	if ( !name.length() ) {
	    ERROR_RETURN( "internal error:FileDriver::insert: unknown field number:" + QString::number(pos) );
	}
	QVariant val = insertData[1];
	if ( d->file.PutField( pos, val.toString().latin1() ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "internal error:FileDriver::insert: invalid field number or data");
	}
    }
    if ( d->file.AppendRecord() != XB_NO_ERROR ) {
	ERROR_RETURN( "internal error:FileDriver::insert: unable to append record" );
    }
    return TRUE;
}

bool FileDriver::next()
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::next..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "internal error:FileDriver::next: file not open" );
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

bool FileDriver::deleteMarked()
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::deleteMarked..." << flush;
#endif
    if ( !isOpen() )
	return FALSE;
    if ( !d->marked.count() )
	return TRUE;
    for ( uint i = 0; i < d->marked.count(); ++i ) {
	if ( d->file.GetRecord( d->marked[i] ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "internal error:FileDriver::deleteMarked: unable to retrieve marked record" );
	}
	if ( d->file.DeleteRecord() != XB_NO_ERROR ) {
	    ERROR_RETURN( "internal error:FileDriver::deleteMarked: unable to delete marked record" );
	}
    }
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
	ERROR_RETURN( "internal error:FileDriver::commit: file not open" );
    }
    d->file.PackDatabase( F_SETLKW );
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::field( uint i, QVariant& v )
{
    if ( !isOpen() )
	return FALSE;
    if ( d->file.GetCurRecNo() < 0 ) {
	ERROR_RETURN( "internal error:FileDriver::field: not on valid record" );
    }
    if ( (int)i > d->file.FieldCount()-1 ) {
	ERROR_RETURN( "internal error:FileDriver::field: field does not exist" );
    }
    int len = d->file.GetFieldLen( i );
    char buf[ len ];
    d->file.GetField( i, buf );
    switch ( d->file.GetFieldType(i) ) {
    case 'N': /* numeric */
	v = QString(buf).toInt();
	break;
    case 'F': /* float */
	v = QString(buf).toDouble();
	break;
    case 'D': /* date */
	v = QDate::fromString(QString(buf));
	break;
    case 'L': /* logical */
	v = QVariant( QString(buf).toInt(), 1 );
	break;
    case 'C': /* character */
    default:
	v = QString(buf).simplifyWhiteSpace();
	break;
    }
    return TRUE;
}

bool FileDriver::fieldDescription( const QString& name, QVariant& v )
{
    if ( !isOpen() )
	return FALSE;
    int i = d->file.GetFieldNo( name.latin1() );
    return fieldDescription( i, v );
}

bool FileDriver::fieldDescription( int i, QVariant& v )
{
    if ( !isOpen() )
	return FALSE;
    if ( i == -1 || i > d->file.FieldCount()-1 ) {
	ERROR_RETURN( "internal error:FileDriver::fieldDescription: field does not exist" );
    }
    qdb::List field;
    QString name = d->file.GetFieldName( i );
    QVariant::Type type = xbaseTypeToVariant( d->file.GetFieldType( i ) );
    int len = d->file.GetFieldLen( i );
    int prec = d->file.GetFieldDecimal( i );
    field.append( name );
    field.append( type );
    field.append( len );
    field.append( prec );
    v = field;
    return TRUE;
}

bool FileDriver::updateMarked( const qdb::List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::updateMarked..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "internal error:FileDriver::updateMarked: file not open" );
    }
    if ( !d->marked.count() )
	return TRUE;
    if ( !data.count() ) {
	ERROR_RETURN( "internal error:FileDriver::updateMarked: no fields defined" );
    }
    if ( (int)data.count() > d->file.FieldCount() ) {
	ERROR_RETURN( "internal error:FileDriver::updateMarked: too many fields" );
    }
    uint i = 0;
    for ( ; i < data.count(); ++i ) {
	qdb::List updateData = data[i].toList();
	if ( !updateData.count() ) {
	    ERROR_RETURN( "internal error:FileDriver::updateMarked: no update data" );
	}
	xbShort fieldnum = d->file.GetFieldNo( updateData[0].toString().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "internal error:FileDriver::updateMarked: field not found:" + updateData[0].toString() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( updateData[1].type() ) ) {
	    ERROR_RETURN( "internal error:FileDriver::updateMarked: bad type:" + updateData[0].toString() );
	}
    }
    for ( i = 0; i < d->marked.count(); ++i ) {
	if ( d->file.GetRecord( d->marked[i] ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "internal error:FileDriver::updateMarked: unable to retrieve marked record" );
	}
	uint j = 0;
	for ( j = 0; j < data.count(); ++j ) {
	    qdb::List updateData = data[i].toList();
	    xbShort fieldnum = d->file.GetFieldNo( updateData[0].toString().latin1() );
	    if ( d->file.PutField( fieldnum, updateData[1].toString().latin1() ) != XB_NO_ERROR ) {
		ERROR_RETURN( "internal error:FileDriver::updateMarked: invalid field number or data");
	    }
	}
	if ( d->file.PutRecord() != XB_NO_ERROR ) {
	    ERROR_RETURN( "internal error:FileDriver::updateMarked: unable to put record" );
	}
    }
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::rewindMarked()
{
    setMarkedAt( -1 );
    return TRUE;
}

bool FileDriver::nextMarked()
{
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "FileDriver::nextMarked..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "internal error:FileDriver::nextMarked: file not open" );
    }
    int next = markedAt() + 1;
    if ( next > (int)d->marked.count() )
	return FALSE;
    if ( d->file.GetRecord( d->marked[next] ) != XB_NO_ERROR )
	return FALSE;
    setMarkedAt( next );
#ifdef VERBOSE_DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::update( const qdb::List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::update..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "internal error:FileDriver::update: file not open" );
    }
    if ( d->file.GetCurRecNo() == 0 ) {
	ERROR_RETURN( "internal error:FileDriver::update: not positioned on valid record" );
    }
    if ( (int)data.count() != d->file.FieldCount() ) {
	ERROR_RETURN( "internal error:FileDriver::update: incorrect number of fields" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "internal error:FileDriver::update: no update data");
    }
    uint i = 0;
    for ( ;  i < data.count(); ++i ) {
	QString name;
	int pos;
	if ( data[0].type() == QVariant::String || data[0].type() == QVariant::CString ) {
	    name = data[0].toString();
	    pos = d->file.GetFieldNo( name );
	} else {
	    pos = data[0].toInt();
	    name = d->file.GetFieldName( pos );
	}
	if ( pos == -1 ) {
	    ERROR_RETURN( "internal error:FileDriver::update: field not found:" + name );
	}
	if ( !name.length() ) {
	    ERROR_RETURN( "internal error:FileDriver::update: unknown field number:" + QString::number(pos) );
	}
	if ( variantToXbaseType( data[1].type() ) != d->file.GetFieldType( pos ) ) {
	    ERROR_RETURN( "internal error:FileDriver::update: invalid field type:" +
			  QString(data[1].typeName()) );
	}
	if ( d->file.PutField( pos, data[1].toString().latin1() ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "internal error:FileDriver::update: invalid field number or data:" + name);
	}

    }
    if ( d->file.PutRecord() != XB_NO_ERROR ) {
	ERROR_RETURN( "internal error:FileDriver::update: unable to put record" );
    }
#ifdef DEBUG_XBASE
    env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::rangeScan( const qdb::List& data )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::rangeScan..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "internal error:FileDriver::rangeScan: file not open" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "internal error:FileDriver::rangeScan: no fields defined");
    }
    d->marked.clear();
    bool forceScan = TRUE;
    xbShort rc = XB_NO_ERROR;
    QString indexDesc;
    uint i = 0;
    for ( i = 0; i < data.count(); ++i ) {
	qdb::List rangeScanFieldData = data[i].toList();
	if ( rangeScanFieldData.count() != 4 ) {
	    ERROR_RETURN( "internal error:FileDriver::rangeScanFieldData: bad field description");
	}
	QString name = rangeScanFieldData[0].toString();
	QVariant value = data[++i];
	xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "internal error:FileDriver::rangeScan: field not found:" + name );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( value.type() ) ) {
	    ERROR_RETURN( "internal error:FileDriver::rangeScan: bad field type:" +
			  QString( value.typeName() ) );
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
		env->output() << "using index scan..." << flush;
#endif
		forceScan = FALSE;
		/* create search value */
		QString searchValue;
		bool numeric = TRUE;
		for ( uint j = 0; j < data.count(); ++j ) {
		    QVariant val = data[i].toList()[1];
		    searchValue += val.toString();
		    if ( val.type() == QVariant::String ||
			 val.type() == QVariant::CString )
			numeric = FALSE;
		}
#ifdef DEBUG_XBASE
		env->output() << "search value:'" << searchValue.latin1() << "'..." << flush;
#endif
		if ( numeric )
		    rc = d->indexes[i]->FindKey( searchValue.toDouble() );
		else
		    rc = d->indexes[i]->FindKey( searchValue.latin1() );
		if ( rc == XB_FOUND ) {
		    rc = XB_NO_ERROR;
		    /* found a key, now scan until we hit a new key value */
		    for ( ; rc == XB_NO_ERROR ; ) {
			bool markOK = TRUE;
			for ( uint k = 0; k < data.count(); ++k ) {
			    qdb::List rangeScanFieldData = data[i].toList();
			    QString name = rangeScanFieldData[0].toString();
			    QVariant value = rangeScanFieldData[1];
			    xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
			    QVariant v;
			    field( fieldnum, v );
			    if ( v != value ) {
				markOK = FALSE;
				break;
			    }
			}
			if ( markOK )
			    mark();
			else
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
	    bool markRecord = FALSE;
	    for ( i = 0; i < data.count(); ++i ) {
		qdb::List rangeScanFieldData = data[i].toList();
		QString name = rangeScanFieldData[0].toString();
		QVariant value = data[++i];
		xbShort fieldnum = d->file.GetFieldNo( name.latin1() );
		QVariant v;
		field( fieldnum, v );
		if ( v == value )
		    markRecord = TRUE;
		else {
		    markRecord = FALSE;
		    break;
		}
	    }
	    if ( markRecord )
		mark();
	    rc = d->file.GetNextRecord();
	}
    }
#ifdef DEBUG_XBASE
     env->output() << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::createIndex( const qdb::List& data, bool unique )
{
#ifdef DEBUG_XBASE
    env->output() << "FileDriver::createIndex..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "internal error:FileDriver::createIndex: file not open" );
    }
    if ( !data.count() ) {
	ERROR_RETURN( "internal error:FileDriver::createIndex: no fields defined");
    }
    uint i = 0;
    xbShort rc;
    QString indexDesc;
    for ( ; i < data.count(); ++i ) {
	qdb::List createIndexData = data[i].toList();
	if ( createIndexData.count() != 4 ) {
	    ERROR_RETURN( "internal error:FileDriver::createIndex: bad index data");
	}
	QString name = createIndexData[0].toString();
	QVariant::Type type = (QVariant::Type)createIndexData[1].toInt();
	/* create the index description string */
	xbShort fieldnum = d->file.GetFieldNo( name );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "internal error:FileDriver::createIndex: field not found:" + name );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( type ) ) {
	    ERROR_RETURN( "internal error:FileDriver::createIndex: bad field type:" + name );
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
#ifdef DEBUG_XBASE
		env->output() << "index already exists..." << flush;
#endif
		//## make this an error instead?
		break;
	    }
	}
    }
    QString indexName;
    if ( forceCreate ) {
	/* create the index on disk */
#ifdef DEBUG_XBASE
	env->output() << "creating index..." << flush;
#endif
	xbNdx* idx = new xbNdx( &d->file );
	d->indexes.resize( d->indexes.size()+1 );
	d->indexes.insert( d->indexes.size()-1, idx );
	/* get unique index name based on file name */
	QFileInfo fi( name() );
	indexName = fi.baseName();
	i = 1;
	fi.setFile( indexName + QString::number(i) + ".ndx" );
	while ( fi.exists() ) {
	    i++;
	    fi.setFile( indexName + QString::number(i) + ".ndx" );
	}
	indexName = fi.fileName();
	if ( ( rc = idx->CreateIndex( indexName.latin1(), indexDesc.latin1(),
				      unique ? XB_UNIQUE : XB_NOT_UNIQUE, XB_OVERLAY ) ) != XB_NO_ERROR ) {
	    QFile::remove( indexName );
	    ERROR_RETURN( "internal error:FileDriver::createIndex: could not create index:" + indexName + ":" + indexDesc );
	}
	/* build index */
	if ( ( rc = idx->ReIndex() ) != XB_NO_ERROR ) {
	    QFile::remove( indexName );
	    ERROR_RETURN( "internal error:FileDriver::createIndex: could not build index:" + indexName + ":" + indexDesc );
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
	ERROR_RETURN( "internal error:FileDriver::drop: no file name" );
    }
    if ( isOpen() )
	close();
    QFileInfo fi( name() );
    QString basename = fi.baseName();
    QDir dir;
    QStringList indexList = dir.entryList( basename + "*.*", QDir::Files );
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
