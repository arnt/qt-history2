/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qalabel.h#1 $
**
** Definition of QAccelLabel widget class
**
** Created : 941215
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QALABEL_H
#define QALABEL_H

#include "qaccel.h"
#include "qlabel.h"


class QAccelLabel : public QLabel
{
    Q_OBJECT
public:
    QAccelLabel( QWidget * buddy, int key, const char * text,
		 QWidget *parent, const char *name=0, WFlags f=0 );
   ~QAccelLabel();

    QWidget * buddy() const { return b; }

    void setAlignment( int );

private slots:
    void focusSlot( int );

private:
    QAccel * a;
    QWidget * b;

private:	// Disabled copy constructor and operator=
    QAccelLabel( const QAccelLabel & ) {}
    QAccelLabel &operator=( const QAccelLabel & ) { return *this; }
};


#endif // QLABEL_H
