/****************************************************************************
** Definition of LCDRange class, Qt tutorial 12
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef LCDRANGE_H
#define LCDRANGE_H

#include <qscrbar.h>
#include <qlcdnum.h>
#include <qlabel.h>


class LCDRange : public QWidget
{
    Q_OBJECT
public:
    LCDRange( QWidget *parent=0, const char *name=0 );
    LCDRange( const char *s, QWidget *parent=0, const char *name=0 );

    int         value() const;
    const char *text()  const;
public slots:
    void setValue( int );
    void setRange( int min, int max );
    void setText( const char * );
signals:
    void valueChanged( int );
protected:
    void resizeEvent( QResizeEvent * );
private:
    void init();

    QScrollBar  *sBar;
    QLCDNumber  *lcd;
    QLabel      *label;
};

#endif // LCDRANGE_H
