#include "mainwindow.h"
#include <qsqltable.h>
#include <qstringlist.h>

MainWindow::MainWindow ( QWidget * parent, const char * name, WFlags f )
    : MasterChildWindowBase(parent, name, f),
      master( "qsql_master" ),
      child( "qsql_child" )
{
    master.select();
    masterTable->setSorting( TRUE );
    childTable->setSorting( TRUE );
    masterTable->setCursor( &master );
    child.field("masterid")->setVisible( FALSE );
    reloadChildTable( 1 );
    connect( masterTable, SIGNAL( currentChanged(const QSqlRecord*)),
	     SLOT( newMasterSelection(const QSqlRecord*)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::newMasterSelection( const QSqlRecord* fields )
{
    int idx = fields->field( "id" )->value().toInt();
    reloadChildTable( idx );
}

void MainWindow::reloadChildTable( int masterIdx )
{
    child.select( "masterid=" + QString::number(masterIdx) );
    childTable->setCursor( &child );
}

