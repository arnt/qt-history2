#include "resultwindow.h"
#include <qstringlist.h>
#include <qlistbox.h>
#include <qsql.h>
#include <qsqlview.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qsqltable.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>

// class MyDataGrid : public QSqlTable
// {
// public:
//     MyDataGrid ( QWidget * parent = 0, const char * name = 0 )
// 	: QSqlGrid( parent, name )
//     {
// 	QSqlConnection::addDatabase( "QMYSQL", "troll", "db", "", "lupinella", "mysql" );
// 	QSql r( "select * from Customer order by 2;", "mysql" );
// 	setQuery( r, FALSE );
// 	addColumn( r.fields().field( 2 ) );
// 	addColumn( r.fields().field( 4 ) );
//     }
//     ~MyDataGrid()
//     {
//     }
// };


ResultWindow::ResultWindow ( QWidget * parent, const char * name, WFlags f )
    : SqlBrowseWindowBase(parent, name, f)
{
    db = QSqlConnection::database();
    QStringList fil = db->tables();
    tableList->insertStringList( fil );
    connect( execButton,SIGNAL(clicked()), this, SLOT(slotExec()));
    connect( dataGrid, SIGNAL( currentChanged(const QSqlFieldList*)),
	     SLOT( newSelection(const QSqlFieldList*)));
    browseType->setButton( 0 );
    //    grid->addWidget( new MyDataGrid( this ), 5, 5 );
}

ResultWindow::~ResultWindow()
{

}

void ResultWindow::slotExec()
{
    if ( db->isOpen() ) {
	QButton* b = browseType->selected();
	if ( b ) {
	    switch ( browseType->id( b ) ) {
	    case 0: // SQL
		dataGrid->setQuery( "select * from " + tableList->currentText() + ";" );
		break;
	    case 1: // Rowset
		dataGrid->setRowset( tableList->currentText() );
		break;
	    case 2: // View
		dataGrid->setView( tableList->currentText() );
		break;
	    }
	}
    }
}


void ResultWindow::newSelection( const QSqlFieldList* fields )
{
    QString cap ("Selection ");
    for ( uint i = 0; i < fields->count(); ++i ) {
	QSqlField f = fields->field(i);	
	cap += " - " + f.value().toString();
    }
    setCaption( cap );
}
