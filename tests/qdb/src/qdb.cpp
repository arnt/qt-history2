#include <qdb.h>
#include <qfile.h>
#include <sqlinterpreter.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qvaluestack.h>

class QDb::Private
{
public:
    Private() : stdOut( stdout, IO_WriteOnly )
    {
	out = &stdOut;
    }
    QMap<int,FileDriver> drivers;
    QMap<int,ResultSet> results;
    qdb::Stack stck;
    Program pgm;
    Parser prs;
    QTextStream stdOut;
    QTextStream* out;
    QString err;
};


/*!  Constructs an empty database environment

*/

QDb::QDb()
{
    d = new Private();
}


/*! Destroys the object and frees any allocated resources.

*/

QDb::~QDb()
{
    reset();
    delete d;
}

/*! Adds a driver to the environment with the name \a fileName
 identified by \a id.

 \sa fileFriver()
*/

void QDb::addFileDriver( int id, const QString& fileName )
{
    d->drivers[id] = FileDriver( this, fileName );
}

/*! Adds a result to the environment with the name \a fileName
 identified by \a id.

 \sa resultSet()
*/

void QDb::addResultSet( int id )
{
    d->results[id] = ResultSet( this );
}


/*! Returns a reference to the file driver identified by \id

  \sa addDriver()

*/

qdb::FileDriver* QDb::fileDriver( int id )
{
    return &d->drivers[id];
}


/*! Returns a reference to the stack.

*/

qdb::Stack* QDb::stack()
{
    return &d->stck;
}


/*! Returns a reference to the program.

*/

qdb::Program* QDb::program()
{
    return &d->pgm;
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
    d->pgm.resetCounter();
    while( (op = d->pgm.next() ) ) {
	if ( !op->exec( this ) ) {
	    if ( verbose )
		output() << "[Line " + QString::number(d->pgm.counter()) + "] " + lastError() << endl;
	    break;
	}
    }
    return TRUE;
}


/*!

*/

void QDb::reset()
{
    d->stck.clear();
    d->pgm.clear();
    uint i = 0;
    for( i = 0; i < d->drivers.count(); ++i )
	d->drivers[i].close();
    d->drivers.clear();
    d->results.clear();
}

/*!

*/

qdb::ResultSet* QDb::resultSet( int id )
{
    return &d->results[id];
}

bool QDb::save( QIODevice *dev )
{
    if ( !dev || !dev->isOpen() )
	return FALSE;
    d->pgm.resetCounter();
    int i = 0;
    QDataStream stream( dev );
    qdb::Op* op = 0;
    while( (op = d->pgm.next() ) ) {
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
    d->pgm.resetCounter();
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
    d->pgm.resetCounter();
    int i = 0;
    qdb::Op* op = 0;
    while( (op = d->pgm.next() ) ) {
	stream << QString::number( i ).rightJustify(4) << op->name().rightJustify(15);
	stream << asListing( op->P(0) ).rightJustify(15);
	stream << asListing( op->P(1) ).rightJustify(15);
	stream << asListing( op->P(2) ).rightJustify(15);
	stream << endl;
	++i;
    }
    d->pgm.resetCounter();
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

void QDb::setOutput( QTextStream& stream )
{
    d->out = &stream;
}

QTextStream& QDb::output()
{
    return *d->out;
}

void QDb::setLastError( const QString& error )
{
    d->err = error;
}

QString QDb::lastError() const
{
    return d->err;
}
