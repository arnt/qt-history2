/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindow.h#2 $
**
** Definition of QWindow class
**
** Author  : Haavard Nord
** Created : 931112
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWINDOW_H
#define QWINDOW_H

#include "qwidget.h"


class QWindow : public QWidget			// window widget class
{
    Q_OBJECT
public:
    QWindow( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QWindow();
};


#endif // QWINDOW_H
