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
#include <qsqlquery.h>
#include "../connection.h"

bool createConnections();


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv, FALSE );

    int rows = 0;

    if ( createConnections() ) {
	QSqlQuery query( "INSERT INTO staff ( id, forename, surname, salary ) "
		     "VALUES ( 1155, 'Ginger', 'Davis', 50000 )" );
	if ( query.isActive() ) rows += query.numRowsAffected() ;

	query.exec( "UPDATE staff SET salary=60000 WHERE id=1155" );
	if ( query.isActive() ) rows += query.numRowsAffected() ;

	query.exec( "DELETE FROM staff WHERE id=1155" ); 
	if ( query.isActive() ) rows += query.numRowsAffected() ;
    }

    return ( rows == 3 ) ? 0 : 1; 
}



