/*
$Id$
*/  

#ifndef PRODUCTLIST_H
#define PRODUCTLIST_H

#include <qtable.h>

class ProductList: public QTable
{
Q_OBJECT

public:
    ProductList();

private slots:
    void processValueChanged( int, int );
    void changeQuantity( int );

private:
    QWidget * createEditor( int, int, bool ) const; 
    void setCellContentFromEditor( int, int );

    QWidget * createMyEditor( int, int ) const; 

    double calcPrice( int );
    double sumUp( int );
};

#endif
