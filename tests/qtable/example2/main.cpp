/****************************************************************************
** $Id: //depot/qt/main/examples/table/main.cpp#2 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "../qtable.h"
#include <qapplication.h>
#include <qintdict.h>

class MyTable : public QTable
{
    Q_OBJECT
public:
    MyTable() : QTable( 65535, 65535, 0, 0 ), items( 1000 ), theWidgets( 1000 ) { theWidgets.setAutoDelete( TRUE ); }
    virtual void setItem( int row, int col, QTableItem *item ) { items.insert( indexOf( row, col ), item ); }
    QTableItem *item( int row, int col ) const { return items[ indexOf( row, col ) ]; }
    virtual void clearCell( int row, int col ) { items.remove( indexOf( row, col ) ); };

    virtual void insertWidget( int row, int col, QWidget *e ) { theWidgets.insert( indexOf( row, col ), e); }
    QWidget *cellWidget( int row, int col ) const { return theWidgets[ indexOf( row, col ) ]; }
    virtual void clearCellWidget( int row, int col ) { theWidgets.remove( indexOf( row, col ) ); }
    virtual void resizeData( int ) {}

private:
    QIntDict<QTableItem> items;
    QIntDict<QWidget> theWidgets;
};

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    MyTable v;

    a.setMainWidget( &v );
    v.show();
    return a.exec();
}

#include "main.moc"
