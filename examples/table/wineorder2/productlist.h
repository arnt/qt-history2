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

private:
    double calcPrice( int );
    double sumUp( int );
};

#endif
