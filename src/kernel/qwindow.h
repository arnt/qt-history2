/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindow.h#9 $
**
** Definition of QWindow class
**
** Created : 931112
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
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

private:	// Disabled copy constructor and operator=
    QWindow( const QWindow & );
    QWindow &operator=( const QWindow & );
};


#endif // QWINDOW_H
