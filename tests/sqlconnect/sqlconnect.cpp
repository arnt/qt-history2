#include <qapplication.h>
#include <qsqldatabase.h>

int main( int argc, char** argv )
{
    QApplication a( argc, argv, FALSE );
    qDebug("Qt SQL Connection Test");
    qDebug("++++++++++++++");

    qDebug("Creating connection...");
    QSqlDatabase* db = QSqlDatabase::addDatabase( qApp->argv()[1] );
    db->setDatabaseName( qApp->argv()[2] );
    db->setUserName( qApp->argv()[3] );
    db->setPassword( qApp->argv()[4] );
    db->setHostName( qApp->argv()[5] );

    db->open();
    if ( db->isOpen() )
	qDebug(" Database opened.");
    else
	qDebug(" ERROR: could not open database:" + db->lastError().databaseText());

    db->close();
    if ( !db->isOpen() )
	qDebug(" Database closed.");
    else
	qDebug(" ERROR: could not close database:" + db->lastError().databaseText());

    qDebug("Done.");
    return 0;
};
