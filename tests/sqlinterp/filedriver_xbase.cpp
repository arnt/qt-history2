#include "sqlinterpreter.h"

#include <qsql.h>
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


#ifdef DEBUG_XBASE
#include <iostream>
using namespace std;
#define ERROR_RETURN(err) do { \
			   setLastError( err ); \
			   close(); \
			   cout << endl; \
			   return FALSE; \
			  } while(0)
#else
#define ERROR_RETURN(err) do { \
			   setLastError( err ); \
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

FileDriver::FileDriver( const QString& name = QString::null )
    : nm( name )
{
    d = new Private();
    setIsOpen( FALSE );
}

FileDriver::~FileDriver()
{
    delete d;
}

FileDriver::FileDriver( const FileDriver& other )
    : Interpreter::FileDriver(), nm( other.nm )
{
    *d = *other.d;
    setIsOpen( FALSE );
}

FileDriver& FileDriver::operator=( const FileDriver& other )
{
    nm = other.nm;
    *d = *other.d;
    setIsOpen( FALSE );
    return *this;
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

bool FileDriver::create( const QSqlRecord* record )
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::create..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "FileDriver::create: no file name" );
    }
    if ( !record->count() ) {
	ERROR_RETURN( "FileDriver::create: no fields defined" );
    }
    QArray<xbSchema> xbrec( record->count()+1 );
    xbSchema x;
    for ( uint i = 0; i < record->count(); ++i ) {
	qstrncpy( x.FieldName, record->field(i)->name().latin1(),
		  QMAX( 10, record->field(i)->name().length() )  );
	x.FieldName[11] = 0;
	x.Type = variantToXbaseType( record->field(i)->type() );
	x.FieldLen = 10; //### fix
	x.NoOfDecs = 0;//### fix
	xbrec[i] = x;
    }
    memset( &x, 0, sizeof(xbSchema) );
    xbrec[ record->count() ] = x;
    d->file.SetVersion( 4 );   /* create dbase IV style files */
    xbShort rc;
    if ( ( rc = d->file.CreateDatabase( name().latin1(), xbrec.data(), XB_OVERLAY ) )
	 != XB_NO_ERROR ) {
	ERROR_RETURN( "FileDriver::create: error creating database" );
    }
    d->file.CloseDatabase();   /* Close database and associated indexes */
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::open()
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::open..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "FileDriver::open: no file name" );
    }
    if ( d->file.OpenDatabase( name().latin1() ) != XB_NO_ERROR ) {
	ERROR_RETURN( "FileDriver::open: unable to open " + name() );
    }
#ifdef DEBUG_XBASE
    cout << name().latin1() << " opened..." << flush;
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
	    ERROR_RETURN( "FileDriver::open: unable to open index " + indexList[i] );
	}
#ifdef DEBUG_XBASE
	cout << indexList[i].latin1() << " index opened..." << flush;
#endif
	d->indexes.insert( i, idx );
    }
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::close()
{
    if ( !isOpen() )
	return TRUE;
    if ( !commit() )
	return FALSE;
    d->file.CloseDatabase();   /* Close database and associated indexes */
    setIsOpen( FALSE );
    return TRUE;
}

bool FileDriver::insert( const QSqlRecord* record )
{
    if ( !isOpen() )
	return FALSE;
    if ( (int)record->count() != d->file.FieldCount() ) {
	ERROR_RETURN( "FileDriver::insert: wrong number of fields" );
    }
    uint i = 0;
    for ( i = 0; i < record->count(); ++i ) {
	if ( variantToXbaseType(record->field(i)->type()) != d->file.GetFieldType(i) ) {
	    ERROR_RETURN( "FileDriver::insert: invalid field type" );
	}
    }
    d->file.BlankRecord();
    for ( i = 0; i < record->count(); ++i ) {
	if ( d->file.PutField( i, record->value(i).toString().latin1() ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "FileDriver::insert: invalid field number or data");
	}
    }
    if ( d->file.AppendRecord() != XB_NO_ERROR ) {
	ERROR_RETURN( "FileDriver::insert: unable to append record" );
    }
    return TRUE;
}

bool FileDriver::next()
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::next..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::next: file not open" );
    }
    xbShort rc = XB_NO_ERROR;
    if ( d->file.GetCurRecNo() == 0 ) {
	rc = d->file.GetFirstRecord();
    } else {
	rc = d->file.GetNextRecord();
    }
    if ( rc != XB_NO_ERROR )
	return FALSE;
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::mark()
{
    if ( !isOpen() )
	return FALSE;
#ifdef DEBUG_XBASE
    cout << "FileDriver::mark: marking record " << d->file.GetCurRecNo() << endl;
#endif
    d->marked.append( d->file.GetCurRecNo() );
    return TRUE;
}

bool FileDriver::deleteMarked()
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::deleteMarked..." << flush;
#endif
    if ( !isOpen() )
	return FALSE;
    if ( !d->marked.count() )
	return TRUE;
    for ( uint i = 0; i < d->marked.count(); ++i ) {
	if ( d->file.GetRecord( d->marked[i] ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "FileDriver::deleteMarked: unable to retrieve marked record" );
	}
	if ( d->file.DeleteRecord() != XB_NO_ERROR ) {
	    ERROR_RETURN( "FileDriver::deleteMarked: unable to delete marked record" );
	}
    }
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::commit()
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::commit..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::commit: file not open" );
    }
    d->file.PackDatabase( F_SETLKW );
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::field( uint i, QVariant& v )
{
    if ( !isOpen() )
	return FALSE;
    if ( d->file.GetCurRecNo() < 0 ) {
	ERROR_RETURN( "FileDriver::field: not on valid record" );
    }
    if ( (int)i > d->file.FieldCount()-1 ) {
	ERROR_RETURN( "FileDriver::field: field does not exist" );
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

bool FileDriver::updateMarked( const QSqlRecord* record )
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::updateMarked..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::updateMarked: file not open" );
    }
    if ( !d->marked.count() )
	return TRUE;
    uint i = 0;
    for ( i = 0; i < record->count(); ++i ) {
	xbShort fieldnum = d->file.GetFieldNo( record->field(i)->name().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "FileDriver::updateMarked: field not found:" + record->field(i)->name() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( record->field(i)->type() ) ) {
	    ERROR_RETURN( "FileDriver::updateMarked: bad field type:" + record->field(i)->name() );
	}
    }
    for ( i = 0; i < d->marked.count(); ++i ) {
	if ( d->file.GetRecord( d->marked[i] ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "FileDriver::updateMarked: unable to retrieve marked record" );
	}
	uint j = 0;
	for ( j = 0; j < record->count(); ++j ) {
	    xbShort fieldnum = d->file.GetFieldNo( record->field(j)->name().latin1() );
	    if ( d->file.PutField( fieldnum, record->value(j).toString().latin1() ) != XB_NO_ERROR ) {
		ERROR_RETURN( "FileDriver::updateMarked: invalid field number or data");
	    }
	}
	if ( d->file.PutRecord() != XB_NO_ERROR ) {
	    ERROR_RETURN( "FileDriver::updateMarked: unable to put record" );
	}
    }
#ifdef DEBUG_XBASE
    cout << "success" << endl;
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
#ifdef DEBUG_XBASE
    cout << "FileDriver::nextMarked..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::nextMarked: file not open" );
    }
    int next = markedAt() + 1;
    if ( next > (int)d->marked.count() )
	return FALSE;
    if ( d->file.GetRecord( d->marked[next] ) != XB_NO_ERROR )
	return FALSE;
    setMarkedAt( next );
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::update( const QSqlRecord* record )
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::update..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::update: file not open" );
    }
    if ( d->file.GetCurRecNo() == 0 ) {
	ERROR_RETURN( "FileDriver::update: not positioned on valid record" );
    }
    if ( (int)record->count() != d->file.FieldCount() ) {
	ERROR_RETURN( "FileDriver::update: incorrect number of fields" );
    }
    uint i = 0;
    for ( i = 0; i < record->count(); ++i ) {
	if ( variantToXbaseType(record->field(i)->type()) != d->file.GetFieldType(i) ) {
	    ERROR_RETURN( "FileDriver::update: invalid field type" );
	}
    }
    for ( i = 0; i < record->count(); ++i ) {
	if ( d->file.PutField( i, record->value(i).toString().latin1() ) != XB_NO_ERROR ) {
	    ERROR_RETURN( "FileDriver::update: invalid field number or data");
	}
    }
    if ( d->file.PutRecord() != XB_NO_ERROR ) {
	ERROR_RETURN( "FileDriver::update: unable to put record" );
    }
#ifdef DEBUG_XBASE
    cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::rangeScan( const QSqlRecord* index )
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::rangeScan..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::rangeScan: file not open" );
    }
    uint i = 0;
    for ( i = 0; i < index->count(); ++i ) {
	xbShort fieldnum = d->file.GetFieldNo( index->field(i)->name().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "FileDriver::rangeScan: field not found:" + index->field(i)->name() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( index->field(i)->type() ) ) {
	    ERROR_RETURN( "FileDriver::rangeScan: bad field type:" + index->field(i)->name() );
	}
    }
    d->marked.clear();
    bool forceScan = TRUE;
    xbShort rc = XB_NO_ERROR;
    /* do we have a matching idx? */
    QString indexDesc;
    for ( i = 0; i < index->count(); ++i ) {
	xbShort fieldnum = d->file.GetFieldNo( index->field(i)->name().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "FileDriver::rangeScan: field not found:" + index->field(i)->name() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( index->field(i)->type() ) ) {
	    ERROR_RETURN( "FileDriver::rangeScan: bad field type:" + index->field(i)->name() );
	}
	indexDesc += QString(( indexDesc.length()>0 ? QString("+") : QString::null ) ) +
		     index->field(i)->name();
    }
    /* check if index already exists */
    char buf[XB_MAX_NDX_NODE_SIZE];
    if ( d->indexes.count() ) {
	for ( i = 0; i < d->indexes.count(); ++i ) {
	    d->indexes[i]->GetExpression( buf,XB_MAX_NDX_NODE_SIZE  );
	    if ( QString(buf) == indexDesc ) {
#ifdef DEBUG_XBASE
		cout << "using index scan..." << flush;
#endif
		forceScan = FALSE;
		/* create search value */
		QString searchValue;
		bool numeric = TRUE;
		for ( uint j = 0; j < index->count(); ++j ) {
		    searchValue += index->value(j).toString();
		    if ( index->field(j)->type() == QVariant::String ||
			 index->field(j)->type() == QVariant::CString )
			numeric = FALSE;
		}
#ifdef DEBUG_XBASE
		cout << "search value:'" << searchValue.latin1() << "'..." << flush;
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
			for ( uint k = 0; k < index->count(); ++k ) {
			    xbShort fieldnum = d->file.GetFieldNo( index->field(k)->name().latin1() );
			    QVariant v;
			    field( fieldnum, v );
			    if ( v != index->field( k )->value() ) {
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
	rc = d->file.GetFirstRecord();
	while ( rc == XB_NO_ERROR ) {
	    bool markRecord = FALSE;
	    for ( i = 0; i < index->count(); ++i ) {
		xbShort fieldnum = d->file.GetFieldNo( index->field(i)->name().latin1() );
		QVariant v;
		field( fieldnum, v );
		if ( v == index->field(i)->value() )
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
     cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::createIndex( const QSqlRecord* index, bool unique )
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::createIndex..." << flush;
#endif
    if ( !isOpen() ) {
	ERROR_RETURN( "FileDriver::createIndex: file not open" );
    }
    uint i = 0;
    xbShort rc;
    /* create the index description string */
    QString indexDesc;
    for ( i = 0; i < index->count(); ++i ) {
	xbShort fieldnum = d->file.GetFieldNo( index->field(i)->name().latin1() );
	if (  fieldnum == -1 ) {
	    ERROR_RETURN( "FileDriver::createIndex: field not found:" + index->field(i)->name() );
	}
	if ( d->file.GetFieldType( fieldnum ) != variantToXbaseType( index->field(i)->type() ) ) {
	    ERROR_RETURN( "FileDriver::createIndex: bad field type:" + index->field(i)->name() );
	}
	indexDesc += QString(( indexDesc.length()>0 ? QString("+") : QString::null ) ) +
		     index->field(i)->name();
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
		cout << "index already exists..." << flush;
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
	cout << "creating index..." << flush;
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
	    ERROR_RETURN( "FileDriver::createIndex: could not create index:" + indexName + ":" + indexDesc );
	}
	/* build index */
	if ( ( rc = idx->ReIndex() ) != XB_NO_ERROR ) {
	    QFile::remove( indexName );
	    ERROR_RETURN( "FileDriver::createIndex: could not build index:" + indexName + ":" + indexDesc );
	}

    }
#ifdef DEBUG_XBASE
     cout << "success" << endl;
#endif
    return TRUE;
}

bool FileDriver::drop()
{
#ifdef DEBUG_XBASE
    cout << "FileDriver::dropp..." << flush;
#endif
    if ( !name() ) {
	ERROR_RETURN( "FileDriver::drop: no file name" );
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
     cout << "success" << endl;
#endif
    return TRUE;
}
