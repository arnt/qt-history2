#include "mainwindow.h"
#include <qsqlview.h>
#include <qsqltable.h>

MainWindow::MainWindow ( QWidget * parent, const char * name, WFlags f )
    : MasterChildWindowBase(parent, name, f)
{
    db = QSqlConnection::database();
    QSqlView masterView( "qsql_master" );
    masterView.select();
    masterTable->setView( masterView );
    reloadChildTable( 1 );

    connect( masterTable, SIGNAL( currentChanged(const QSqlFieldList*)),
	     SLOT( newMasterSelection(const QSqlFieldList*)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::newMasterSelection( const QSqlFieldList* fields )
{
    int idx = fields->field("id").value().toInt();
    reloadChildTable( idx );
}

void MainWindow::reloadChildTable( int masterIdx )
{
    QSqlView childView( "qsql_child" );
    childView.field("masterid").setIsVisible( FALSE );
    childView.select( "masterId = " + QString::number( masterIdx ) );
    childTable->setView( childView );
}

