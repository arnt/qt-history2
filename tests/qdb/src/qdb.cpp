/*
    Xbase project source code

    This file contains XBase SQL environment implementations

    Copyright (C) 2000 Dave Berton (db@trolltech.com)
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

    Contact:

      Mail:

	Technology Associates, Inc.
	XBase Project
	1455 Deming Way #11
	Sparks, NV 89434
	USA

      Email:

	xbase@techass.com

      See our website at:

	xdb.sourceforge.net
*/

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

 \sa fileDriver()
*/

void QDb::addFileDriver( int id, const QString& fileName )
{
    d->drivers[id] = FileDriver( this, fileName );
}

/*! Adds a driver to the environment with the name \a fileName.  A
  unique id is automatically generated and returned.

 \sa fileDriver()
*/

int QDb::addFileDriver( const QString& fileName )
{
    int id = d->drivers.count()+1;
    d->drivers[id] = FileDriver( this, fileName );
    return id;
}

/*! Closes and removes the file driver identified by 'id'.  The driver
 must have been previously added with addfileDriver().
 */

void QDb::removeFileDriver( int id )
{
    d->drivers[id].close();
    d->drivers.remove( d->drivers.find( id ) );
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

bool QDb::parse( const QString& commands, bool verbose )
{
    if ( verbose )
	output() << "parsing..." << endl;
    return Parser().parse( commands, this );
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
//	    if ( verbose )
//		output() << "[Line " + QString::number(d->pgm.counter()) + "] " + lastError() << endl;
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
