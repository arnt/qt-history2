/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtable.h>
#include <qapplication.h>
#include <qintdict.h>

class MyTable : public QTable
{
    Q_OBJECT
public:
    MyTable() : QTable( 0, 0 ), items( 1000 ), theWidgets( 1000 ) { setNumRows( 6500 ); setNumCols( 6500 ); theWidgets.setAutoDelete( TRUE ); }
    virtual void setItem( int row, int col, QTableItem *item ) { items.insert( indexOf( row, col ), item ); item->setRow( row ); item->setCol( col ); }
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
    v.setSorting( FALSE );

    a.setMainWidget( &v );
    v.show();
    return a.exec();
}

#include "main.moc"
