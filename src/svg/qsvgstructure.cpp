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

#include "qsvgstructure_p.h"

#include "qsvgnode_p.h"
#include "qsvgstyle_p.h"

#include "qdebug.h"

QSvgG::QSvgG(QSvgNode *parent)
    : QSvgStructureNode(parent)
{

}

void QSvgG::draw(QPainter *p)
{
    QList<QSvgNode*>::iterator itr = m_renderers.begin();
    applyStyle(p);
    while (itr != m_renderers.end()) {
        (*itr)->draw(p);
        ++itr;
    }
    revertStyle(p);
}

QSvgNode::Type QSvgG::type() const
{
    return G;
}

QSvgStructureNode::QSvgStructureNode(QSvgNode *parent)
    :QSvgNode(parent)
{

}

QSvgNode * QSvgStructureNode::scopeNode(const QString &id) const
{
    const QSvgStructureNode *group = this;
    while (group && group->type() != QSvgNode::DOC) {
        group = static_cast<QSvgStructureNode*>(group->parent());
    }
    if (group)
        return group->m_scope[id];
    return 0;
}

void QSvgStructureNode::addChild(QSvgNode *child, const QString &id, bool def )
{
    if (!def)
        m_renderers.append(child);

     if (child->type() == QSvgNode::DEFS) {
         QSvgDefs *defs =
             static_cast<QSvgDefs*>(child);
         m_linkedScopes.append(defs);
     }

    if (id.isEmpty())
        return; //we can't add it to scope without id

    QSvgStructureNode *group = this;
    while (group && group->type() != QSvgNode::DOC) {
        group = static_cast<QSvgStructureNode*>(group->parent());
    }
    if (group)
        group->m_scope.insert(id, child);
}

QSvgDefs::QSvgDefs(QSvgNode *parent)
    : QSvgStructureNode(parent)
{
}

void QSvgDefs::draw(QPainter *)
{
    //noop
}

QSvgNode::Type QSvgDefs::type() const
{
    return DEFS;
}

QSvgStyleProperty * QSvgStructureNode::scopeStyle(const QString &id) const
{
    const QSvgStructureNode *group = this;
    while (group) {
        QSvgStyleProperty *prop = group->styleProperty(id);
        if (prop)
            return prop;
        QList<QSvgStructureNode*>::const_iterator itr = group->m_linkedScopes.begin();
        while (itr != group->m_linkedScopes.end()) {
            prop = (*itr)->styleProperty(id);
            if (prop)
                return prop;
            ++itr;
        }
        group = static_cast<QSvgStructureNode*>(group->parent());
    }
    return 0;
}
