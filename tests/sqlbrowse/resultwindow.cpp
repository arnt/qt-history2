#include "resultwindow.h"
#include "datagrid.h"
#include <qstringlist.h>
#include <qlistbox.h>
#include <qsqlrowset.h>
#include <qsqlfield.h>

ResultWindow::ResultWindow ( QSqlDatabase* database, QWidget * parent=0, const char * name=0, WFlags f=0 )
    : SqlBrowseWindowBase(parent, name, f),
      db(database)
{
    QStringList fil = db->tables();
    //    for ( uint i = 0; i < fil.count(); ++i )
    tableList->insertStringList( fil );
    //    execButton->setEnabled( FALSE );
    connect( execButton,SIGNAL(clicked()), this, SLOT(slotExec()));
}
ResultWindow::~ResultWindow()
{
}
void ResultWindow::slotExec()
{
    if ( db->isOpen() ) {
	QSqlRowset rset( db, tableList->currentText() );
	rset.select(); // all records
	dataGrid->take( rset );
    }

// 	if ( db && sqlQuery->text().length() && db->isOpen() ) {
// 	    QString query( sqlQuery->text().simplifyWhiteSpace() );
// 	    QSql res =  db->query( query );
// 	    if ( res.isActive() ) {
// 		sqlBrowse->take(res);
// 	    } else {
// 		QSqlError err = res.lastError();
// 		QMessageBox::information(0,"SQL Error",err.driverText() + err.databaseText());
// 	    }
// 	    sqlQuery->setFocus();
// 	}
}
