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

#include "qlocale.h"
#include "qdebug.h"

QSvgG::QSvgG(QSvgNode *parent)
    : QSvgStructureNode(parent)
{

}

QSvgStructureNode::~QSvgStructureNode()
{
    qDeleteAll(m_renderers);
}

void QSvgG::draw(QPainter *p)
{
    QList<QSvgNode*>::iterator itr = m_renderers.begin();
    applyStyle(p);
    while (itr != m_renderers.end()) {
        QSvgNode *node = *itr;
        if (node->isVisible())
            node->draw(p);
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
        QList<QSvgStructureNode*>::const_iterator itr = group->m_linkedScopes.constBegin();
        while (itr != group->m_linkedScopes.constEnd()) {
            prop = (*itr)->styleProperty(id);
            if (prop)
                return prop;
            ++itr;
        }
        group = static_cast<QSvgStructureNode*>(group->parent());
    }
    return 0;
}

QHash<QString, bool> QSvgSwitch::m_features;
QHash<QString, bool> QSvgSwitch::m_extensions;

QSvgSwitch::QSvgSwitch(QSvgNode *parent)
    : QSvgStructureNode(parent)
{
    init();
}

void QSvgSwitch::draw(QPainter *p)
{
    QList<QSvgNode*>::iterator itr = m_renderers.begin();
    applyStyle(p);
    while (itr != m_renderers.end()) {
        QSvgNode *node = *itr;
        if (node->isVisible()) {
            const QStringList &features  = node->requiredFeatures();
            const QStringList &extensions = node->requiredExtensions();
            const QStringList &languages = node->requiredLanguages();
            const QStringList &formats = node->requiredFormats();
            const QStringList &fonts = node->requiredFonts();

            bool okToRender = true;
            if (!features.isEmpty()) {
                QStringList::const_iterator sitr = features.constBegin();
                for (; sitr != features.constEnd(); ++sitr) {
                    if (!m_features.contains(*sitr)) {
                        okToRender = false;
                        break;
                    }
                }
            }

            if (okToRender && !extensions.isEmpty()) {
                QStringList::const_iterator sitr = extensions.constBegin();
                for (; sitr != extensions.constEnd(); ++sitr) {
                    if (!m_extensions.contains(*sitr)) {
                        okToRender = false;
                        break;
                    }
                }
            }

            if (okToRender && !languages.isEmpty()) {
                QStringList::const_iterator sitr = languages.constBegin();
                okToRender = false;
                for (; sitr != languages.constEnd(); ++sitr) {
                    if ((*sitr).startsWith(m_systemLanguagePrefix)) {
                        okToRender = true;
                        break;
                    }
                }
            }

            if (okToRender && !formats.isEmpty()) {
                okToRender = false;
            }

            if (okToRender && !fonts.isEmpty()) {
                okToRender = false;
            }

            if (okToRender) {
                node->draw(p);
                break;
            }
        }
        ++itr;
    }
    revertStyle(p);
}

QSvgNode::Type QSvgSwitch::type() const
{
    return SWITCH;
}

void QSvgSwitch::init()
{
    if (m_features.isEmpty()) {
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#SVG", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#SVG-static", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#CoreAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Structure", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#ConditionalProcessing", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#ConditionalProcessingAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Image", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Prefetch", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Shape", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Text", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#PaintAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#OpacityAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#GraphicsAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Gradient", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#SolidColor", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#XlinkAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#ExternalResourcesRequiredAttribute", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Font", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Hyperlinking", true);
        m_features.insert("http://www.w3.org/Graphics/SVG/feature/1.2/#Extensibility", true);
    }

    QLocale locale;
    m_systemLanguage = locale.name().replace("_", "-");
    int idx = m_systemLanguage.indexOf('-');
    m_systemLanguagePrefix = m_systemLanguage.mid(0, idx);
}
