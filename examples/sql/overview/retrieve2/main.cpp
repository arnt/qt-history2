/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include "../connection.h"

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( createConnections() ) {
	QSqlCursor cur( "staff" ); // Specify the table/view name
	cur.select(); // We'll retrieve every record
	while ( cur.next() ) {
	    qDebug( cur.value( "id" ).toString() + ": " +
		    cur.value( "surname" ).toString() + " " +
		    cur.value( "salary" ).toString() );
	}
    }

    return 0;
}
