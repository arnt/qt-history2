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
		    //dataGrid->setQuery( "select * from " + tableList->currentText() + ";" );
		    sql.setName( tableList->currentText() );
		    sql.select( sql.primaryIndex() );
		    dataGrid->setView( &sql );		    
		    break;
		case 1: // Rowset
		    // dataGrid->setRowset( tableList->currentText() );
		    break;
		case 2: // View
		    sql.setName( tableList->currentText() );
		    sql.select( sql.primaryIndex() );
		    dataGrid->setView( &sql );
		    break;
		}
	    }
	    break;
	case 1:
//	    sql.setView( queryEdit->text().simplifyWhiteSpace() );
	//    dataGrid->setView( &sql );
	    break;
	}
    }
}

void ResultWindow::newSelection( const QSqlFieldList* fields )
{
    QString cap;
    for ( uint i = 0; i < fields->count(); ++i ) {
	const QSqlField * f  = fields->field(i);
	cap += f->displayLabel().leftJustify(20) + ":" + 
	       f->value().toString().rightJustify(30) + "\n";
    }
    currentRecordEdit->setText( cap );
}
