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

#ifndef QGRAPHICSITEM_P_H
#define QGRAPHICSITEM_P_H

#include <QtGui/qmatrix.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qgraphicsitem.h"

class QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsItem)
public:
    inline QGraphicsItemPrivate()
        : z(0), scene(0), parent(0), matr(0), index(-1), q_ptr(0)
    {
        visible = 1;
        enabled = 1;
        selected = 0;
        rotating = 0;
        scaling = 0;
        acceptsMouse = 1;
        acceptsHover = 0;
        flags =  QGraphicsItem::ItemIsSelectable;
        pad = 0;
    }

    inline ~QGraphicsItemPrivate()
    { delete matr; }

    inline QMatrix matrix() const
    { return matr ? *matr : QMatrix(); }

    inline void setMatrix(const QMatrix &m)
    {
        if (m.isIdentity()) {
            delete matr;
            matr = 0;
            return;
        }
        if (!matr)
            matr = new QMatrix;
        *matr = m;
    }

    QPointF pos;
    qreal z;
    QGraphicsScene *scene;
    QGraphicsItem *parent;
    QMatrix *matr;
    QList<QGraphicsItem *> children;
    int index;
    quint32 visible : 1;
    quint32 enabled : 1;
    quint32 selected : 1;
    quint32 rotating : 1;
    quint32 scaling : 1;
    quint32 acceptsMouse : 1;
    quint32 acceptsHover : 1;
    quint32 resizeHandle : 2;
    quint32 flags : 11;
    quint32 pad : 12;

    QGraphicsItem *q_ptr;
};

#endif
