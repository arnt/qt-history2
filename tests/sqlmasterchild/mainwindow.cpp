#include "mainwindow.h"
#include <qsqltable.h>
#include <qstringlist.h>

MainWindow::MainWindow ( QWidget * parent, const char * name, WFlags f )
    : MasterChildWindowBase(parent, name, f)
{
    //    childTable->removeColumn( 1 );
    connect( masterTable, SIGNAL( currentChanged(const QSqlRecord*)),
	     SLOT( newMasterSelection(const QSqlRecord*)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::newMasterSelection( const QSqlRecord* fields )
{
    int idx = fields->field( "id" )->value().toInt();
    childTable->setFilter( "masterid=" + QString::number(idx) );
    childTable->refresh();
}
