/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDECORATION_QWS_H
#define QDECORATION_QWS_H

#include "qregion.h"

class QPopupMenu;

/*
 Implements decoration styles
*/
class QDecoration
{
public:
    QDecoration() {}
    virtual ~QDecoration() {}

    enum DecorItem { None = 0, All = 1, Title = 2, Top = 3, Bottom = 4, Left = 5, Right = 6,
                     TopLeft = 7, TopRight = 8, BottomLeft = 9, BottomRight = 10,
                     Close = 11, Minimize = 12, Maximize = 13, Normalize = 14, Menu = 15,
                     Help = 16, Border = 17, LastDecorItem = Border };

    enum DecoreState { Normal = 0x04, Disabled = 0x08, Hover = 0x01, Pressed = 0x02 };

    virtual QRegion region(const QWidget *, const QRect &rect, DecorItem r=All) = 0;

    virtual void close(QWidget *);
    virtual void minimize(QWidget *);
    virtual void maximize(QWidget *);

    virtual void paintItem(QPainter *, const QWidget *, DecorItem item = All,
                           DecoreState state = Normal) = 0;
};

#endif // QDECORATION_QWS_H
