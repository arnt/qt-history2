#include "mainwindow.h"
#include <qsqlview.h>
#include <qsqltable.h>

MainWindow::MainWindow ( QWidget * parent, const char * name, WFlags f )
    : MasterChildWindowBase(parent, name, f),
      masterView( "qsql_master" ),
      childView( "qsql_child" )
{
    masterView.select();
    masterTable->setSorting( TRUE );
    childTable->setSorting( TRUE );
    masterTable->setView( &masterView );
    childView.field("masterid")->setIsVisible( FALSE );    
    reloadChildTable( 1 );
    connect( masterTable, SIGNAL( currentChanged(const QSqlFieldList*)),
	     SLOT( newMasterSelection(const QSqlFieldList*)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::newMasterSelection( const QSqlFieldList* fields )
{
    int idx = fields->field( "id" )->value().toInt();
    reloadChildTable( idx );
}

void MainWindow::reloadChildTable( int masterIdx )
{
    childView.select( "masterId = " + QString::number( masterIdx ) );
    childTable->setView( &childView );
}

