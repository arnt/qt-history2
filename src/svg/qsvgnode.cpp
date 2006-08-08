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

#include "qsvgnode_p.h"
#include "qsvgtinydocument_p.h"

#include "qdebug.h"

QSvgNode::QSvgNode(QSvgNode *parent)
    : m_parent(parent),
      m_visible(true)
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
            m_style.quality = static_cast<QSvgQualityStyle*>(prop);
            break;
        case QSvgStyleProperty::FILL:
            m_style.fill = static_cast<QSvgFillStyle*>(prop);
            break;
        case QSvgStyleProperty::VIEWPORT_FILL:
            m_style.viewportFill = static_cast<QSvgViewportFillStyle*>(prop);
            break;
        case QSvgStyleProperty::FONT:
            m_style.font = static_cast<QSvgFontStyle*>(prop);
            break;
        case QSvgStyleProperty::STROKE:
            m_style.stroke = static_cast<QSvgStrokeStyle*>(prop);
            break;
        case QSvgStyleProperty::SOLID_COLOR:
            m_style.solidColor = static_cast<QSvgSolidColorStyle*>(prop);
            break;
        case QSvgStyleProperty::GRADIENT:
            m_style.gradient = static_cast<QSvgGradientStyle*>(prop);
            break;
        case QSvgStyleProperty::TRANSFORM:
            m_style.transform = static_cast<QSvgTransformStyle*>(prop);
            break;
        case QSvgStyleProperty::ANIMATE_COLOR:
            m_style.animateColor = static_cast<QSvgAnimateColor*>(prop);
            break;
        case QSvgStyleProperty::ANIMATE_TRANSFORM:
            m_style.animateTransforms.append(
                static_cast<QSvgAnimateTransform*>(prop));
            break;
        case QSvgStyleProperty::OPACITY:
            m_style.opacity = static_cast<QSvgOpacityStyle*>(prop);
            break;
        default:
            qDebug("QSvgNode: Trying to append unknown property!");
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
        case QSvgStyleProperty::ANIMATE_COLOR:
            if (node->m_style.animateColor)
                return node->m_style.animateColor;
            break;
        case QSvgStyleProperty::ANIMATE_TRANSFORM:
            if (!node->m_style.animateTransforms.isEmpty())
                return node->m_style.animateTransforms.first();
            break;
        case QSvgStyleProperty::OPACITY:
            if (!node->m_style.opacity)
                return node->m_style.opacity;
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
    if (rid.startsWith(QLatin1Char('#')))
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

QRectF QSvgNode::bounds() const
{
    return QRectF(0, 0, 0, 0);
}

QSvgTinyDocument * QSvgNode::document() const
{
    QSvgTinyDocument *doc = 0;
    QSvgNode *node = const_cast<QSvgNode*>(this);
    while (node && node->type() != QSvgNode::DOC) {
        node = node->parent();
    }
    doc = static_cast<QSvgTinyDocument*>(node);

    return doc;
}

void QSvgNode::setRequiredFeatures(const QStringList &lst)
{
    m_requiredFeatures = lst;
}

const QStringList & QSvgNode::requiredFeatures() const
{
    return m_requiredFeatures;
}

void QSvgNode::setRequiredExtensions(const QStringList &lst)
{
    m_requiredExtensions = lst;
}

const QStringList & QSvgNode::requiredExtensions() const
{
    return m_requiredExtensions;
}

void QSvgNode::setRequiredLanguages(const QStringList &lst)
{
    m_requiredLanguages = lst;
}

const QStringList & QSvgNode::requiredLanguages() const
{
    return m_requiredLanguages;
}

void QSvgNode::setRequiredFormats(const QStringList &lst)
{
    m_requiredFormats = lst;
}

const QStringList & QSvgNode::requiredFormats() const
{
    return m_requiredFormats;
}

void QSvgNode::setRequiredFonts(const QStringList &lst)
{
    m_requiredFonts = lst;
}

const QStringList & QSvgNode::requiredFonts() const
{
    return m_requiredFonts;
}

void QSvgNode::setVisible(bool visible)
{
    //propagate visibility change of true to the parent
    //not propagating false is just a small performance
    //degradation since we'll iterate over children without
    //drawing any of them
    if (m_parent && visible && !m_parent->isVisible())
        m_parent->setVisible(true);

    m_visible = visible;
}

QRectF QSvgNode::transformedBounds(const QMatrix &mat) const
{
    QMatrix m = mat;

    QSvgTransformStyle *trans = m_style.transform;
    if (trans) {
        m = trans->qmatrix() * m;
    }

    QRectF rect = bounds();

    rect = m.mapRect(rect);
    
    return rect;
}

void QSvgNode::setNodeId(const QString &i)
{
    m_id = i;
}

void QSvgNode::setXmlClass(const QString &str)
{
    m_class = str;
}
