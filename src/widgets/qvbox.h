/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.h#1 $
**
** Definition of vbox layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QVBOX_H
#define QVBOX_H

#include "qwidget.h"

class QVBoxLayout;

class QVBox : public QWidget
{
    Q_OBJECT
public:
    QVBox( QWidget *parent=0, const char *name=0 );
    bool event( QEvent * );
protected:
    virtual void childEvent( QChildEvent * );
private:
    QVBoxLayout *lay;
};

#endif //QVBOX_H
