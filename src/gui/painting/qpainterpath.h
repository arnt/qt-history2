/****************************************************************************
**
** Definition of the QPaintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPAINTERPATH_H
#define QPAINTERPATH_H

#include "qpoint.h"

class QPainterPathPrivate;

class Q_GUI_EXPORT QPainterPath
{
public:
    QPainterPath();
    ~QPainterPath();

    void beginSubpath();
    void closeSubpath();
    
    void addLine(const QPoint &p1, const QPoint &p2);
    inline void addLine(int x1, int y1, int x2, int y2);

private:
    QPainterPathPrivate *d;

    friend class QPainter;
};

inline void QPainterPath::addLine(int x1, int y1, int x2, int y2)
{
    addLine(QPoint(x1, y1), QPoint(x2, y2));    
}

#endif // QPAINTERPATH_H
