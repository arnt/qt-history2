#include <qdb.h>
#include <qfile.h>

/*!  Constructs an empty environment

*/

QDb::QDb()
    : stdOut( stdout, IO_WriteOnly )
{
    out = &stdOut;
}


/*! Destroys the object and frees any allocated resources.

*/

QDb::~QDb()
{
    reset();
}

void QDb::addDriver( int id, const QString& fileName )
{
    drivers[id] = FileDriver( this, fileName );
}

void QDb::addResult( int id )
{
    results[id] = ResultSet( this );
}


/*! Returns a reference to the file driver identified by \id

  \sa addDriver()

*/

FileDriver& QDb::fileDriver( int id )
{
    return drivers[id];
}


/*! Returns a reference to the stack.

*/

QValueStack<QVariant>& QDb::stack()
{
    return stck;
}


/*! Returns a reference to the program.

*/

Program& QDb::program()
{
    return pgm;
}


bool QDb::parse( const QString& /*commands*/, bool verbose )
{
    //## jasmin todo
    if ( verbose )
	output() << "parsing..." << endl;;
    return TRUE;
}

/*!

*/

bool QDb::execute( bool verbose )
{
    if ( verbose )
	output() << "executing..." << endl;
    qdb::Op* op = 0;
    pgm.resetCounter();
    while( (op = pgm.next() ) ) {
	if ( !op->exec( this ) ) {
	    if ( verbose )
		output() << "[Line " + QString::number(pgm.counter()) + "] " + lastError() << endl;
	    break;
	}
    }
    return TRUE;
}


/*!

*/

void QDb::reset()
{
    stck.clear();
    pgm.clear();
    uint i = 0;
    for( i = 0; i < drivers.count(); ++i )
	drivers[i].close();
    drivers.clear();
    results.clear();
}

/*!

*/

ResultSet& QDb::resultSet( int id )
{
    return results[id];
}

bool QDb::save( QIODevice *dev )
{
    if ( !dev || !dev->isOpen() )
	return FALSE;
    pgm.resetCounter();
    int i = 0;
    QDataStream stream( dev );
    qdb::Op* op = 0;
    while( (op = pgm.next() ) ) {
	stream << i << op->name();
	if ( op->P(0).isValid() )
	     stream << op->P(0);
	if ( op->P(1).isValid() )
	     stream << op->P(1);
	if ( op->P(2).isValid() )
	     stream << op->P(2);
	stream << "\n";
	++i;
    }
    pgm.resetCounter();
    return TRUE;
}

bool QDb::save( const QString& filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    save( &f );
    f.close();
    return TRUE;
}

static QString asListing( QVariant& v )
{
    QString s;
    switch( v.type() ) {
    case QVariant::List: {
	s = "list(";
	QValueList<QVariant> l = v.toList();
	for ( uint i = 0; i < l.count(); ++i )
	    s += asListing( l[i] ) + (i<l.count()-1?QString(","):QString(")"));
	break;
    }
    default:
	s = v.toString();
    }
    if ( s.isNull() )
	s = ".";
    return s;
}

bool QDb::saveListing( QTextStream& stream )
{
    stream << "Program Listing" << endl;
    pgm.resetCounter();
    int i = 0;
    qdb::Op* op = 0;
    while( (op = pgm.next() ) ) {
	stream << QString::number( i ).rightJustify(4) << op->name().rightJustify(15);
	stream << asListing( op->P(0) ).rightJustify(15);
	stream << asListing( op->P(1) ).rightJustify(15);
	stream << asListing( op->P(2) ).rightJustify(15);
	stream << endl;
	++i;
    }
    pgm.resetCounter();
    return TRUE;
}


bool QDb::saveListing( const QString& filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    QTextStream stream( &f );
    saveListing( stream );
    f.close();
    return TRUE;
}
