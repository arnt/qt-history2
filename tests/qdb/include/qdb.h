/*
    Xbase project source code

    This file contains the LocalSQL class

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

#ifndef QDB_H
#define QDB_H

#include "environment.h"

class LocalSQL : public localsql::Environment
{
public:
    LocalSQL();
    virtual ~LocalSQL();

    void setOutput( QTextStream& stream );
    QTextStream& output();
    bool parse( const QString& commands, bool verbose = FALSE );
    bool execute( bool verbose = FALSE );
    void reset();
    void addFileDriver( int id, const QString& fileName );
    int addFileDriver( const QString& fileName );
    void removeFileDriver( int id );
    void addResultSet( int id );
    localsql::FileDriver* fileDriver( int id );
    localsql::Stack* stack();
    localsql::Program* program();
    localsql::ResultSet* resultSet( int id );
    void setLastError( const QString& error );
    QString lastError() const;

private:
    class Private;
    Private* d;
};

#endif
