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

#include <localsql.h>
#include <qfile.h>
#include <sqlinterpreter.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qvaluestack.h>

class LocalSQL::Private
{
public:
    Private()
	: stdOut( stdout, IO_WriteOnly ),
	  affectedRows(0)
    {
	out = &stdOut;
    }
    QMap<int,FileDriver> drivers;
    QMap<int,ResultSet> results;
    QMap<int,int> driverAlias;
    Stack stck;
    Program pgm;
    Parser prs;
    QTextStream stdOut;
    QTextStream* out;
    QString err;
    QString path;
    int affectedRows;
};

/*! \class LocalSQL

  The main database environment. //## more
*/


/*!  Constructs an empty database environment

*/

LocalSQL::LocalSQL()
{
    d = new Private();
}


/*! Destroys the object and frees any allocated resources.

*/

LocalSQL::~LocalSQL()
{
    reset();
    delete d;
}

/*! Adds a driver to the environment with the name \a fileName
 identified by \a id.

 \sa fileDriver()
*/

void LocalSQL::addFileDriver( int id, const QString& fileName )
{
    d->drivers[id] = FileDriver( this, fileName );
}

/*! Adds a driver to the environment with the name \a fileName.  A
  unique id is automatically generated and returned.

 \sa fileDriver()
*/

int LocalSQL::addFileDriver( const QString& fileName )
{
    int id = d->drivers.count()+1;
    d->drivers[id] = FileDriver( this, fileName );
    return id;
}

/*! Closes and removes the file driver identified by 'id'.  The driver
 must have been previously added with addfileDriver().
 */

void LocalSQL::removeFileDriver( int id )
{
    d->drivers[id].close();
    d->drivers.remove( d->drivers.find( id ) );
}

/*! Adds a result to the environment with the name \a fileName
 identified by \a id.

 \sa resultSet()
*/

void LocalSQL::addResultSet( int id )
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

LocalSQLFileDriver* LocalSQL::fileDriver( int id )
{
    if ( id < 0 )
	id = d->driverAlias[id];
    return &d->drivers[id];
}


/*! Returns a pointer to the stack.  This pointer is owned by the
environment, so do not delete it.

*/

localsql::Stack* LocalSQL::stack()
{
    return &d->stck;
}


/*! Returns a pointer to the program. This pointer is owned by the
  environment, so do not delete it.

*/

LocalSQLProgram* LocalSQL::program()
{
    return &d->pgm;
}

/*! Parses \a commands and creates a new program.  Any previous program
   is first cleared.  If \a verbose is TRUE, each command processed
   will be echoed.

   \sa setOutput()

*/

bool LocalSQL::parse( const QString& commands, bool verbose )
{
    if ( verbose )
	output() << "parsing..." << endl;
    return Parser().parse( commands, this );
}

/*! Executes the program produced by parse(). If an error is
  encountered and \a verbose is TRUE, the error is echoed.

  \sa parse() setOutput()

*/

bool LocalSQL::execute( bool verbose )
{
    if ( verbose )
	output() << "executing..." << endl;
    LocalSQLOp* op = 0;
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

void LocalSQL::reset()
{
    d->stck.clear();
    d->pgm.clear();
    uint i = 0;
    for( i = 0; i < d->drivers.count(); ++i )
	d->drivers[i].close();
    d->drivers.clear();
    d->results.clear();
    d->affectedRows = -1;
}

/*! Returns the result set identified by \a id.  The result set must
  have been previously added with addresultSet(), otherwise the
  pointer that is returned will have no functionality.  This pointer
  is owned by the environment, so do not delete it.

  \sa addResultSet()

*/

LocalSQLResultSet* LocalSQL::resultSet( int id )
{
    return &d->results[id];
}

/*! Sets all output (debug messages and error text) to the text stream
  \a stream.

  \sa output()

*/

void LocalSQL::setOutput( QTextStream& stream )
{
    d->out = &stream;
}

/*! Returns a reference to the output stream used for debug messages
  and error text.

  \sa setOutput()

*/
QTextStream& LocalSQL::output()
{
    return *d->out;
}

/*! Sets the last error text to \a error.

  \sa lastError()
*/

void LocalSQL::setLastError( const QString& error )
{
    d->err = error;
}

/*! Returns the last error.

  \sa setLastError()
*/

QString LocalSQL::lastError() const
{
    return d->err;
}

bool LocalSQL::addFileDriverAlias( const List& drivers, const QString fieldname, int alias )
{
    if ( alias >= 0 ) {
	setLastError( "Internal error: bad alias:" + QString::number( alias ) );
	return FALSE;
    }
    int aliasedFile = -1;
    for ( uint i = 0; i < drivers.count(); ++i ) {
	LocalSQLFileDriver* drv = fileDriver( drivers[i].toInt() );
	if ( !drv ) {
	    setLastError( "Internal error: unknown file id:" + drivers[i].toString() );
	    return FALSE;
	}
	QStringList names = drv->columnNames();
	if ( names.contains( fieldname ) ) {
	    if ( aliasedFile > -1 ) {
		setLastError( "Ambiguous column name: " + fieldname );
		return FALSE;
	    }
	    aliasedFile = drivers[i].toInt();
	}
    }
    if ( aliasedFile < 0 ) {
	setLastError( "Unknown column:" + fieldname );
	return FALSE;
    }
    d->driverAlias[alias] = aliasedFile;
    return TRUE;
}

void LocalSQL::setPath( const QString& path )
{
    if ( path.length() == 0 )
	return;
    d->path = path;
    if ( d->path[ d->path.length()-1 ] == '/' )
	d->path = d->path.mid( 0, d->path.length()-1 );
}

QString LocalSQL::path() const
{
    return d->path;
}

void LocalSQL::setAffectedRows( int i )
{
    d->affectedRows = i;
}

int LocalSQL::affectedRows() const
{
    return d->affectedRows;
}
