/****************************************************************************
** $Id: //depot/qt/main/util/quick/qdesigner.h#1 $
**
** Shared functionality used in Quick designer
**
** Created : 970801
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/


#ifndef DESIGNER_H
#define DESIGNER_H

#include <qrect.h>
#include <qwidget.h>

class QDesignerPrivate {
public:
    QDesignerPrivate( QWidget* owner );
    static QRect dluToRect(int x, int y, int w, int h);

    // other shared operations...
};

#endif
