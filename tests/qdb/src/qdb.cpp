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


/*! Returns a pointer to the file driver identified by \id.  A
  previous call to addFileDriver() must have been made, otherwise the
  pointer that is returned will point to a driver that has no
  functionality.  This pointer is owned by the environment, so do not
  delete it.

  \sa addFileDriver()

*/

qdb::FileDriver* QDb::fileDriver( int id )
{
    return &d->drivers[id];
}


/*! Returns a pointer to the stack.  This pointer is owned by the
environment, so do not delete it.

*/

qdb::Stack* QDb::stack()
{
    return &d->stck;
}


/*! Returns a pointer to the program. This pointer is owned by the
  environment, so do not delete it.

*/

qdb::Program* QDb::program()
{
    return &d->pgm;
}

/* Parses \a commands and creates a new program.  Any previous program
   is first cleared.  If \a verbose is TRUE, each command processed
   will be echoed.

   \sa setOutput()

*/

bool QDb::parse( const QString& /*commands*/, bool verbose )
{
    //## jasmin todo
    if ( verbose )
	output() << "parsing..." << endl;;
    return TRUE;
}

/*! Executes the program produced by parse(). If an error is
  encountered and \a verbose is TRUE, the error is echoed.

  \sa parse() setOutput()

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


/*! Resets internal state (empties the stack, clears the program,
closes and removed any file drivers, etc).

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

/*! Returns the result set identified by \a id.  The result set must
  have been previously added with addresultSet(), otherwise the
  pointer that is returned will have no functionality.  This pointer
  is owned by the environment, so do not delete it.

  \sa addResultSet()

*/

qdb::ResultSet* QDb::resultSet( int id )
{
    return &d->results[id];
}

/*! Saves the contents of the program as a binary stream to the device
\a dev.
*/

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

/*! Saves the contents of the program as a binary stream to the file
named \a filename.
*/

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

/*! Saves the contents of the program as human readable text to the
  text stream \a stream..
*/

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

/*! Saves the contents of the program as human readable text to the
  file named \a filename..
*/

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

/*! Sets all output (debug messages and error text) to the text stream
  \a stream.

  \sa output()

*/

void QDb::setOutput( QTextStream& stream )
{
    d->out = &stream;
}

/*! Returns a reference to the output stream used for debug messages
  and error text.

  \sa setOutput()

*/
QTextStream& QDb::output()
{
    return *d->out;
}

/*! Sets the last error text to \a error.

  \sa lastError()
*/

void QDb::setLastError( const QString& error )
{
    d->err = error;
}

/*! Returns the last error.

  \sa setLastError()
*/

QString QDb::lastError() const
{
    return d->err;
}
