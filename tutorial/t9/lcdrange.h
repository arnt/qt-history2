/****************************************************************************
** Definition of LCDRange class, Qt tutorial 9
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef LCDRANGE_H
#define LCDRANGE_H

#include <qscrbar.h>
#include <qlcdnum.h>


class LCDRange : public QWidget
{
    Q_OBJECT
public:
    LCDRange( QWidget *parent=0, const char *name=0 );

    int value() const;
public slots:
    void setValue( int );
    void setRange( int min, int max );
signals:
    void valueChanged( int );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QScrollBar  *sBar;
    QLCDNumber  *lcd;
};

#endif // LCDRANGE_H
