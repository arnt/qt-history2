/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsizegrip.h#2 $
**
** Definition of QSizeGrip class
**
** Created : 980316
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SIZEGRIP

class Q_EXPORT QSizeGrip: public QWidget
{
    Q_OBJECT
public:
    QSizeGrip( QWidget *parent, const char* name=0 );
    ~QSizeGrip();

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
private:
    QPoint p;
    QSize s;
    int d;
};

#endif //QT_NO_SIZEGRIP
#endif
