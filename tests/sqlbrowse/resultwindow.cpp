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
#include <qtabwidget.h>

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
    QButton* b;
    if ( db->isOpen() ) {
	switch ( tabs->currentPageIndex() ) {
	case 0:
	    b = browseType->selected();
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
	    break;
	case 1:
	    dataGrid->setQuery( queryEdit->text().simplifyWhiteSpace() );
	    break;
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
