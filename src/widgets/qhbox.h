/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.h#1 $
**
** Definition of hbox layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QHBOX_H
#define QHBOX_H

#include "qwidget.h"

class QHBoxLayout;

class QHBox : public QWidget
{
    Q_OBJECT
public:
    QHBox( QWidget *parent=0, const char *name=0 );
    bool event( QEvent * );
protected:
    virtual void childEvent( QChildEvent * );
private:
    QHBoxLayout *lay;
};

#endif //QHBOX_H
