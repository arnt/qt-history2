/*
$Id$
*/

#ifndef SPINBOXITEM_H
#define SPINBOXITEM_H

#include <qtable.h>
#include <qstring.h>

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
