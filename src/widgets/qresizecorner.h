/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qresizecorner.h#3 $
**
** Definition of QResizeCorner class
**
** Created : 980316
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QRESIZECORNER_H
#define QRESIZECORNER_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class Q_EXPORT QResizeCorner: public QWidget
{
public:
    QResizeCorner( QWidget *parent, const char* name=0 );

    QSize sizeHint() const;
    
protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
private:
    QPoint p;
    QSize s;
};

#endif
