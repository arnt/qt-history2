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
#ifndef QSVGNODE_H
#define QSVGNODE_H

#include "qsvgstyle_p.h"

#include "QtCore/qstring.h"
#include "QtCore/qhash.h"

class QPainter;

class QSvgNode
{
public:
    enum Type
    {
        DOC,
        G,
        DEFS,
        ANIMATION,
        ARC,
        CIRCLE,
        ELLIPSE,
        IMAGE,
        LINE,
        PATH,
        POLYGON,
        POLYLINE,
        RECT,
        TEXT,
        TEXTAREA,
        USE,
        VIDEO
    };
public:
    QSvgNode(QSvgNode *parent=0);
    virtual ~QSvgNode();
    virtual void draw(QPainter *p) =0;

    QSvgNode *parent() const;

    void appendStyleProperty(QSvgStyleProperty *prop, const QString &id,
                             bool justLink=false);
    void applyStyle(QPainter *p);
    void revertStyle(QPainter *p);
    QSvgStyleProperty *styleProperty(QSvgStyleProperty::Type type) const;
    QSvgStyleProperty *styleProperty(const QString &id) const;

    virtual Type type() const =0;
    virtual QRect bounds() const;
protected:
    QSvgNode   *m_parent;
    QSvgStyle   m_style;
    QHash<QString, QSvgStyleProperty*> m_styles;
};

inline QSvgNode *QSvgNode::parent() const
{
    return m_parent;
}


#endif
