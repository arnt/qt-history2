#include "resultwindow.h"
#include "datagrid.h"
#include <qstringlist.h>
#include <qlistbox.h>
#include <qsqlview.h>
#include <qsqlfield.h>
#include <qsqlindex.h>


class MyDataGrid : public DataGrid
{
public:
    MyDataGrid ( QSqlDatabase* db, QWidget * parent = 0, const char * name = 0 )
	: DataGrid( parent, name )
    {
	r = new QSqlRowset( db, "sheet1" );
	QSqlIndex sort( "sheet1", "sheetSort" );
	sort.append( r->field("d") );
	r->select( sort );
	take( r, FALSE );
	addColumn( r->field("r") );
	addColumn( r->field("d") );
    }
    ~MyDataGrid()
    {
	delete r;
    }
private:
    QSqlRowset* r;
};


ResultWindow::ResultWindow ( QWidget * parent=0, const char * name=0, WFlags f=0 )
    : SqlBrowseWindowBase(parent, name, f),
      view(0)
{
    db = QSqlConnection::database();
    QStringList fil = db->tables();
    tableList->insertStringList( fil );
    connect( execButton,SIGNAL(clicked()), this, SLOT(slotExec()));
    //    dg = new MyDataGrid( db, this );

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
