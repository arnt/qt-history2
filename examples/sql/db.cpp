#include "db.h"
#include "databaseapp.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>

void drop_db();

void create_db()
{
    drop_db();

    /* get the default app database */
    QSqlDatabase* db = QSqlDatabase::database();

    /* load sql script and execute each statement */
    QFile sqlScript( "create_db.sql" );
    if ( sqlScript.open( IO_ReadOnly ) ) {
        QTextStream t( &sqlScript );    
	QString stmt;
        while ( !t.eof() ) {    
            stmt += t.readLine();    
	    if ( stmt[ stmt.length()-1 ] == ';' ) {
		db->exec( stmt );
		stmt = QString::null;
	    }
        }
        sqlScript.close();	
    }
    
}

void drop_db()
{
    QSqlDatabase* db = QSqlDatabase::database();
    db->exec("drop table INVOICE;");
    db->exec("drop table PRODUCT;");
    db->exec("drop table CUSTOMER;");
}
