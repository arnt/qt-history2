/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhbox.h#3 $
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

class QBoxLayout;

class QHBox : public QWidget
{
    Q_OBJECT
public:
    QHBox( QWidget *parent=0, const char *name=0, WFlags f=0 );
    bool event( QEvent * );
protected:
    QHBox( bool horizontal, QWidget *parent=0, const char *name=0, WFlags f=0 );
    virtual void childEvent( QChildEvent * );
private:
    QBoxLayout *lay;
};

#endif //QHBOX_H
