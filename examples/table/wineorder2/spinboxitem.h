/*
$Id$
*/

#ifndef SPIXBOXITEM_H
#define SPIXBOXITEM_H

#include <qtable.h>

class QString;

class SpinBoxItem: public QTableItem
{

public:
    SpinBoxItem( QTable *, const int, const QString &);

private:
    QWidget * createEditor() const;
    void setContentFromEditor( QWidget * );

    int getValue() const; 

    QTable * table;
    QString suffix;
};

#endif
