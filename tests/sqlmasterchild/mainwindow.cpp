#include "mainwindow.h"
#include <qsqltable.h>
#include <qstringlist.h>

MainWindow::MainWindow ( QWidget * parent, const char * name, WFlags f )
    : MasterChildWindowBase(parent, name, f),
      master( "qsql_master" ),
      child( "qsql_child" )
{
    masterTable->setCursor( &master, TRUE );
    childTable->setCursor( &child, TRUE );
    childTable->removeColumn( 1 );
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
    childTable->setFilter( "masterid=" + QString::number(masterIdx) );
    childTable->refresh();
}
