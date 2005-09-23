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
#include "qsvgnode_p.h"

#include "QtCore/qdebug.h"

QSvgNode::QSvgNode(QSvgNode *parent)
    : m_parent(parent)
{
}

QSvgNode::~QSvgNode()
{

}

void QSvgNode::appendStyleProperty(QSvgStyleProperty *prop, const QString &id,
                                   bool justLink)
{
    //qDebug()<<"appending "<<prop->type()<< " ("<< id <<") "<<"to "<<this<<this->type();
    if (!justLink) {
        switch (prop->type()) {
        case QSvgStyleProperty::QUALITY:
            delete m_style.quality;
            m_style.quality = static_cast<QSvgQualityStyle*>(prop);
            break;
        case QSvgStyleProperty::FILL:
            delete m_style.fill;
            m_style.fill = static_cast<QSvgFillStyle*>(prop);
            break;
        case QSvgStyleProperty::VIEWPORT_FILL:
            delete m_style.viewportFill;
            m_style.viewportFill = static_cast<QSvgViewportFillStyle*>(prop);
            break;
        case QSvgStyleProperty::FONT:
            delete m_style.font;
            m_style.font = static_cast<QSvgFontStyle*>(prop);
            break;
        case QSvgStyleProperty::STROKE:
            delete m_style.stroke;
            m_style.stroke = static_cast<QSvgStrokeStyle*>(prop);
            break;
        case QSvgStyleProperty::SOLID_COLOR:
            delete m_style.solidColor;
            m_style.solidColor = static_cast<QSvgSolidColorStyle*>(prop);
            break;
        case QSvgStyleProperty::GRADIENT:
            delete m_style.gradient;
            m_style.gradient = static_cast<QSvgGradientStyle*>(prop);
            break;
        case QSvgStyleProperty::TRANSFORM:
            delete m_style.transform;
            m_style.transform = static_cast<QSvgTransformStyle*>(prop);
            break;
        default:
            break;
        }
    }
    if (!id.isEmpty()) {
        m_styles.insert(id, prop);
    }
}

void QSvgNode::applyStyle(QPainter *p)
{
    m_style.apply(p, bounds(), this);
}

void QSvgNode::revertStyle(QPainter *p)
{
    m_style.revert(p);
}

QSvgStyleProperty * QSvgNode::styleProperty(QSvgStyleProperty::Type type) const
{
    const QSvgNode *node = this;
    while (node) {
        switch (type) {
        case QSvgStyleProperty::QUALITY:
            if (node->m_style.quality)
                return node->m_style.quality;
            break;
        case QSvgStyleProperty::FILL:
            if (node->m_style.fill)
                return node->m_style.fill;
            break;
        case QSvgStyleProperty::VIEWPORT_FILL:
            if (m_style.viewportFill)
                return node->m_style.viewportFill;
            break;
        case QSvgStyleProperty::FONT:
            if (node->m_style.font)
                return node->m_style.font;
            break;
        case QSvgStyleProperty::STROKE:
            if (node->m_style.stroke)
                return node->m_style.stroke;
            break;
        case QSvgStyleProperty::SOLID_COLOR:
            if (node->m_style.solidColor)
                return node->m_style.solidColor;
            break;
        case QSvgStyleProperty::GRADIENT:
            if (node->m_style.gradient)
                return node->m_style.gradient;
            break;
        case QSvgStyleProperty::TRANSFORM:
            if (node->m_style.transform)
                return node->m_style.transform;
            break;
        default:
            break;
        }
        node = node->parent();
    }

    return 0;
}

QSvgStyleProperty * QSvgNode::styleProperty(const QString &id) const
{
    QString rid = id;
    if (rid.startsWith("#"))
        rid.remove(0, 1);
    const QSvgNode *node = this;
    while (node) {
        QSvgStyleProperty *style = node->m_styles[rid];
        if (style)
            return style;
        node = node->parent();
    }

    return 0;
}

QRect QSvgNode::bounds() const
{
    return QRect(0, 0, 0, 0);
}
