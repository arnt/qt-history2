/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGRAPHICSITEM_P_H
#define QGRAPHICSITEM_P_H

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

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class Q_AUTOTEST_EXPORT QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsItem)
public:
    enum Extra {
        ExtraTransform,
        ExtraToolTip,
        ExtraCursor
    };

    enum AncestorFlag {
        NoFlag = 0,
        AncestorHandlesChildEvents = 0x1,
        AncestorClipsChildren = 0x2,
        AncestorIgnoresTransformations = 0x4
    };

    inline QGraphicsItemPrivate()
        : z(0), scene(0), parent(0), index(-1), q_ptr(0)
    {
        acceptedMouseButtons = 0x1f;
        visible = 1;
        explicitlyHidden = 0;
        enabled = 1;
        explicitlyDisabled = 0;
        selected = 0;
        acceptsHover = 0;
        acceptDrops = 0;
        isMemberOfGroup = 0;
        handlesChildEvents = 0;
        itemDiscovered = 0;
        hasTransform = 0;
        hasCursor = 0;
        ancestorFlags = 0;
        flags = 0;
        pad = 0;
    }

    inline virtual ~QGraphicsItemPrivate()
    { }

    void updateAncestorFlag(QGraphicsItem::GraphicsItemFlag childFlag,
                            AncestorFlag flag = NoFlag, bool enabled = false, bool root = true);
    void setIsMemberOfGroup(bool enabled);
    void remapItemPos(QEvent *event, QGraphicsItem *item);
    QTransform sceneTransform(const QTransform &worldTransform) const;
    QPointF genericMapFromScene(const QPointF &pos, const QWidget *viewport) const;
    bool itemIsUntransformable() const;

    void setVisibleHelper(bool newVisible, bool explicitly, bool update = true);
    void setEnabledHelper(bool newEnabled, bool explicitly, bool update = true);

    inline QVariant extra(Extra type) const
    {
        for (int i = 0; i < extras.size(); ++i) {
            const ExtraStruct &extra = extras.at(i);
            if (extra.type == type)
                return extra.value;
        }
        return QVariant();
    }

    inline void setExtra(Extra type, const QVariant &value)
    {
        int index = -1;
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            extras << ExtraStruct(type, value);
        } else {
            extras[index].value = value;
        }
    }

    inline void unsetExtra(Extra type)
    {
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                extras.removeAt(i);
                return;
            }
        }
    }

    struct ExtraStruct {
        ExtraStruct(Extra type, QVariant value)
            : type(type), value(value)
        { }

        Extra type;
        QVariant value;

        bool operator<(Extra extra) const
        { return type < extra; }
    };
    QList<ExtraStruct> extras;

    QPointF pos;
    qreal z;
    QGraphicsScene *scene;
    QGraphicsItem *parent;
    QList<QGraphicsItem *> children;
    int index;
    quint32 acceptedMouseButtons : 5;
    quint32 visible : 1;
    quint32 explicitlyHidden : 1;
    quint32 enabled : 1;
    quint32 explicitlyDisabled : 1;
    quint32 selected : 1;
    quint32 acceptsHover : 1;
    quint32 acceptDrops : 1;
    quint32 isMemberOfGroup : 1;
    quint32 handlesChildEvents : 1;
    quint32 itemDiscovered : 1;
    quint32 hasTransform : 1;
    quint32 hasCursor : 1;
    quint32 ancestorFlags : 3;
    quint32 flags : 11;
    quint32 pad : 1;

    QGraphicsItem *q_ptr;
};

#endif // QT_NO_GRAPHICSVIEW

#endif
