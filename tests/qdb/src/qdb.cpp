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

/*! \class QDb

  The main database environment. //## more
*/


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

/*! Parses \a commands and creates a new program.  Any previous program
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
	    return FALSE;
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
