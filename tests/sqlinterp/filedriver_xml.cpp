#include "sqlinterpreter.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qdom.h>
#include <qsql.h>

class FileDriver::Private
{
public:
    QDomDocument doc;
    QSqlRecord header;
    QSqlRecord buffer;
    QValueList<int> marked;
    int records;
    bool setAt( int i ) {
	QDomNodeList nl = doc.elementsByTagName( "record" );
	if ( !nl.count() || i > (int)nl.count()-1 )
	    return FALSE;
	if ( !buffer.count() )
	    return FALSE;
	/* fill record buffer */
	QDomNode item = nl.item( i ).firstChild();
	for ( int j = 0; j < (int)buffer.count(); ++j ) {
	    QDomNode n = item;
	    int child = 0;
	    while ( child < j ) {
		n = n.nextSibling();
		++child;
	    }
	    QVariant v = n.toElement().text();
	    v.cast( buffer.field(j)->type() );
	    buffer.setValue( j, v );
	}
	return TRUE;
    }
};

FileDriver::FileDriver( const QString& name = QString::null )
    : nm( name )
{
    d = new Private();
}

FileDriver::~FileDriver()
{
    delete d;
}

FileDriver::FileDriver( const FileDriver& other )
    : Interpreter::FileDriver(), nm( other.nm )
{
    *d = *other.d;
}

FileDriver& FileDriver::operator=( const FileDriver& other )
{

    nm = other.nm;
    *d = *other.d;
    return *this;
}

bool FileDriver::create( const QSqlRecord* record )
{
    QFileInfo fi( name() );
    if ( fi.exists() )
	return FALSE;
    if ( !record || !record->count() )
	return FALSE;

    qDebug("before creating doc");
    d->doc = d->doc.implementation().createDocument( "xxx" , "table",
						     d->doc.doctype() );
    qDebug("after creating doc");

//     QString emptydoc("<!DOCTYPE table><table></table>");
//     d->doc.setContent( emptydoc );
    QDomElement header = d->doc.createElement( "header" );
    for ( uint i = 0; i < record->count(); ++i ) {
	QDomElement fieldInfo = d->doc.createElement( "field" );
	QDomElement fieldName = d->doc.createElement( "name" );
	QDomElement fieldType = d->doc.createElement( "type" );
	fieldName.appendChild( d->doc.createTextNode( record->field(i)->name() ) );
	fieldType.appendChild( d->doc.createTextNode(QString::number( record->field(i)->type()) ) );
	fieldInfo.appendChild( fieldName );
	fieldInfo.appendChild( fieldType );
	header.appendChild( fieldInfo );
    }
    d->doc.documentElement().appendChild( header );
    QFile f( name() );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    QTextStream ts( &f );
    d->doc.save( ts, 0 );
    f.close();
    return TRUE;
}

bool FileDriver::open()
{
    if ( !name() )
	return FALSE;
    QFile f( name() );
    if ( !f.open( IO_ReadOnly ) )
	return FALSE;
    if ( !d->doc.setContent( &f ) ) {
	f.close();
	return FALSE;
    }
    f.close();
    /* read the table header */
    QDomElement header = d->doc.firstChild().firstChild().toElement();
    if ( header.isNull() || header.tagName() != "header" )
	return FALSE;
    QDomNodeList nl = header.elementsByTagName( "field" );
    for ( uint i = 0; i < nl.count(); ++i ) {
	QString fieldName = QString::null;
	QVariant::Type fieldType = QVariant::Invalid;
	QDomElement n = nl.item(i).toElement();
	if ( n.firstChild().toElement().tagName() == "name" )
	    fieldName = n.firstChild().firstChild().toText().data();
	if ( n.firstChild().nextSibling().toElement().tagName() == "type" )
	    fieldType = (QVariant::Type)n.firstChild().nextSibling().firstChild().toText().data().toInt();
	if ( fieldName != QString::null && fieldType != QVariant::Invalid ) {
	    d->header.append( QSqlField( fieldName, fieldType ) );
	}
    }
    d->buffer = d->header; /* 'current record' buffer */
    /* get a record count */
    QDomNodeList recCount = d->doc.documentElement().elementsByTagName( "record" );
    d->records = recCount.count();
    setAt( QSql::BeforeFirst );
    setIsOpen( TRUE );
    return TRUE;
}

bool FileDriver::close()
{
    if ( !isOpen() )
	return TRUE;
    if ( commit() ) {
	setIsOpen( FALSE );
	return TRUE;
    }
    return FALSE;
}

bool FileDriver::insert( const QSqlRecord* record )
{
    if ( !isOpen() )
	return FALSE;
    if ( record->count() != d->header.count() )
	return FALSE;
    uint i = 0;
    for ( i = 0; i < record->count(); ++i ) {
	if ( record->field(i)->type() != d->header.field(i)->type() )
	    return FALSE;
    }
    QDomElement newrec = d->doc.createElement( "record" );
    for ( i = 0; i < record->count(); ++i ) {
	QDomElement field = d->doc.createElement( "field" );
	field.appendChild( d->doc.createTextNode( record->field(i)->value().toString() ) );
	newrec.appendChild( field );
    }
    d->doc.documentElement().appendChild( newrec );
    return TRUE;
}

bool FileDriver::next()
{
    if ( !isOpen() )
	return FALSE;
    int next = at() + 1;
    if ( next > d->records-1 ) {
	setAt( QSql::BeforeFirst );
	return FALSE;
    }
    if ( !d->setAt( next ) )
	return FALSE;
    setAt( next );
    return TRUE;
}

bool FileDriver::mark()
{
    if ( !isOpen() )
	return FALSE;
    if ( at() == QSql::BeforeFirst )
	return FALSE;
    d->marked.append( at() );
    return TRUE;
}

bool FileDriver::deleteMarked()
{
    if ( !isOpen() )
	return FALSE;
    if ( !d->marked.count() )
	return FALSE;
    QDomNodeList nl = d->doc.elementsByTagName( "record" );
    if ( !nl.count() )
	return FALSE;
    for ( uint i = 0; i < d->marked.count(); ++i ) {
	QDomNode n = nl.item( d->marked[i] );
	if ( d->doc.documentElement().removeChild( n ).isNull() )
	    return FALSE;
    }
    d->marked.clear();
    return TRUE;
}

bool FileDriver::commit()
{
    QFile f( name() );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    QTextStream ts( &f );
    d->doc.save( ts, 0 );
    f.close();
    return TRUE;
}

bool FileDriver::field( uint i, QVariant& v )
{
    if ( at() == QSql::BeforeFirst )
	return FALSE;
    if ( i > d->buffer.count() )
	return FALSE;
    if ( !isOpen() )
	return FALSE;
    v = d->buffer.value( i );
    return TRUE;
}

bool FileDriver::updateMarked( const QSqlRecord* record )
{
    if ( !isOpen() )
	return FALSE;
    if ( !d->marked.count() )
	return FALSE;
    uint j = 0;
    for ( j = 0; j < record->count(); ++j ) {
	int pos = d->header.position( record->field(j)->name() );
	if ( pos == -1 )
	    return FALSE;
	if ( record->field(j)->type() != d->header.field( pos )->type() )
	    return FALSE;
    }
    QDomNodeList nl = d->doc.elementsByTagName( "record" );
    if ( !nl.count() )
	return FALSE;
    for ( uint i = 0; i < d->marked.count(); ++i ) {
	QDomNode n = nl.item( d->marked[i] ).firstChild();
	for ( j = 0; j < record->count(); ++j ) {
	    int child = 0;
	    int pos = d->header.position( record->field(j)->name() );
	    while ( child < pos ) {
		n = n.nextSibling();
		++child;
	    }
	    if ( n.isNull() )
		return FALSE;
	    n.removeChild( n.firstChild() );
	    n.appendChild( d->doc.createTextNode( record->field(j)->value().toString() ) );
	}
    }
    d->marked.clear();
    return TRUE;
}

bool FileDriver::rewindMarked()
{
    setMarkedAt( -1 );
    return TRUE;
}

bool FileDriver::nextMarked()
{
    int next = markedAt() + 1;
    if ( next > (int)d->marked.count()-1 )
	return FALSE;
    if ( !d->setAt( next ) )
	return FALSE;
    setMarkedAt( next );
    return TRUE;
}

bool FileDriver::update( const QSqlRecord* record )
{
    if ( !isOpen() )
	return FALSE;
    if ( at() == QSql::BeforeFirst )
	return FALSE;
    QDomNodeList nl = d->doc.elementsByTagName( "record" );
    if ( !nl.count() )
	return FALSE;
    uint j = 0;
    for ( j = 0; j < record->count(); ++j ) {
	if ( record->field(j)->type() != d->buffer.field(j)->type() )
	    return FALSE;
    }
    QDomNode rec = nl.item( at() );
    uint i = 0;
    for ( i = 0; i < record->count(); ++i ) {
	QDomNode n = rec.firstChild();
	int child = 0;
	while ( child < (int)i ) {
	    n = n.nextSibling();
	    ++child;
	}
	n.replaceChild( d->doc.createTextNode( record->field(i)->value().toString() ), n.firstChild() );
    }
    return TRUE;
}
