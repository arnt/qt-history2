/*
$Id$
*/

#ifndef CONVERTER_H
#define CONVERTER_H 

#include <qtable.h>

class EuroConverter: public QTable
{
    Q_OBJECT

public:
    EuroConverter();

private slots:
    void processValueChange( int, int );

private:
    double calculate( double );
    int inputcurrency;
    QComboTableItem * currencies;

};

#endif
