#include "resultwindow.h"
#include "datagrid.h"
#include <qstringlist.h>
#include <qlistbox.h>
#include <qsqlview.h>
#include <qsqlfield.h>

ResultWindow::ResultWindow ( QSqlDatabase* database, QWidget * parent=0, const char * name=0, WFlags f=0 )
    : SqlBrowseWindowBase(parent, name, f),
      db(database),
      view(0)
{
    QStringList fil = db->tables();
    tableList->insertStringList( fil );
    connect( execButton,SIGNAL(clicked()), this, SLOT(slotExec()));
}
ResultWindow::~ResultWindow()
{
    if ( view )
	delete view;
}
void ResultWindow::slotExec()
{
    if ( db->isOpen() ) {
	view = new QSqlView( db, tableList->currentText() );
	view->select(); // all records
	dataGrid->take( view );
    }
}
