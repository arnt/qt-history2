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
#include "ui4_p.h"
#include <QDomDocument>

#ifdef QFORMINTERNAL_NAMESPACE
using namespace QFormInternal;
#endif

/*******************************************************************************
** Implementations
*/

void DomUI::clear(bool clear_all)
{
    delete m_widget;
    delete m_layoutDefault;
    delete m_layoutFunction;
    delete m_customWidgets;
    delete m_tabStops;
    delete m_images;
    delete m_includes;
    delete m_resources;
    delete m_connections;

    if (clear_all) {
        m_text = QString();
        m_has_attr_version = false;
        m_has_attr_stdSetDef = false;
        m_attr_stdSetDef = 0;
    }

    m_widget = 0;
    m_layoutDefault = 0;
    m_layoutFunction = 0;
    m_customWidgets = 0;
    m_tabStops = 0;
    m_images = 0;
    m_includes = 0;
    m_resources = 0;
    m_connections = 0;
}

DomUI::DomUI(): bits(0)
{
    m_has_attr_version = false;
    m_has_attr_stdSetDef = false;
    m_attr_stdSetDef = 0;
    m_widget = 0;
    m_layoutDefault = 0;
    m_layoutFunction = 0;
    m_customWidgets = 0;
    m_tabStops = 0;
    m_images = 0;
    m_includes = 0;
    m_resources = 0;
    m_connections = 0;
}

DomUI::~DomUI()
{
    delete m_widget;
    delete m_layoutDefault;
    delete m_layoutFunction;
    delete m_customWidgets;
    delete m_tabStops;
    delete m_images;
    delete m_includes;
    delete m_resources;
    delete m_connections;
}

void DomUI::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("version")))
        setAttributeVersion(node.attribute(QLatin1String("version")));
    if (node.hasAttribute(QLatin1String("stdsetdef")))
        setAttributeStdSetDef(node.attribute(QLatin1String("stdsetdef")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("author")) {
            setElementAuthor(e.text());
            continue;
        }
        if (tag == QLatin1String("comment")) {
            setElementComment(e.text());
            continue;
        }
        if (tag == QLatin1String("exportmacro")) {
            setElementExportMacro(e.text());
            continue;
        }
        if (tag == QLatin1String("class")) {
            setElementClass(e.text());
            continue;
        }
        if (tag == QLatin1String("widget")) {
            DomWidget *v = new DomWidget();
            v->read(e);
            setElementWidget(v);
            continue;
        }
        if (tag == QLatin1String("layoutdefault")) {
            DomLayoutDefault *v = new DomLayoutDefault();
            v->read(e);
            setElementLayoutDefault(v);
            continue;
        }
        if (tag == QLatin1String("layoutfunction")) {
            DomLayoutFunction *v = new DomLayoutFunction();
            v->read(e);
            setElementLayoutFunction(v);
            continue;
        }
        if (tag == QLatin1String("pixmapfunction")) {
            setElementPixmapFunction(e.text());
            continue;
        }
        if (tag == QLatin1String("customwidgets")) {
            DomCustomWidgets *v = new DomCustomWidgets();
            v->read(e);
            setElementCustomWidgets(v);
            continue;
        }
        if (tag == QLatin1String("tabstops")) {
            DomTabStops *v = new DomTabStops();
            v->read(e);
            setElementTabStops(v);
            continue;
        }
        if (tag == QLatin1String("images")) {
            DomImages *v = new DomImages();
            v->read(e);
            setElementImages(v);
            continue;
        }
        if (tag == QLatin1String("includes")) {
            DomIncludes *v = new DomIncludes();
            v->read(e);
            setElementIncludes(v);
            continue;
        }
        if (tag == QLatin1String("resources")) {
            DomResources *v = new DomResources();
            v->read(e);
            setElementResources(v);
            continue;
        }
        if (tag == QLatin1String("connections")) {
            DomConnections *v = new DomConnections();
            v->read(e);
            setElementConnections(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomUI::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("ui") : tagName.toLower());

    QDomElement child;

    if (hasAttributeVersion())
        e.setAttribute(QLatin1String("version"), attributeVersion());

    if (hasAttributeStdSetDef())
        e.setAttribute(QLatin1String("stdsetdef"), attributeStdSetDef());

    if (bit.author) {
        child = doc.createElement(QLatin1String("author"));
        child.appendChild(doc.createTextNode(m_author));
        e.appendChild(child);
    }

    if (bit.comment) {
        child = doc.createElement(QLatin1String("comment"));
        child.appendChild(doc.createTextNode(m_comment));
        e.appendChild(child);
    }

    if (bit.exportMacro) {
        child = doc.createElement(QLatin1String("exportmacro"));
        child.appendChild(doc.createTextNode(m_exportMacro));
        e.appendChild(child);
    }

    if (bit.klass) {
        child = doc.createElement(QLatin1String("class"));
        child.appendChild(doc.createTextNode(m_class));
        e.appendChild(child);
    }

    if (m_widget != 0)
        e.appendChild(m_widget->write(doc, QLatin1String("widget")));

    if (m_layoutDefault != 0)
        e.appendChild(m_layoutDefault->write(doc, QLatin1String("layoutdefault")));

    if (m_layoutFunction != 0)
        e.appendChild(m_layoutFunction->write(doc, QLatin1String("layoutfunction")));

    if (bit.pixmapFunction) {
        child = doc.createElement(QLatin1String("pixmapfunction"));
        child.appendChild(doc.createTextNode(m_pixmapFunction));
        e.appendChild(child);
    }

    if (m_customWidgets != 0)
        e.appendChild(m_customWidgets->write(doc, QLatin1String("customwidgets")));

    if (m_tabStops != 0)
        e.appendChild(m_tabStops->write(doc, QLatin1String("tabstops")));

    if (m_images != 0)
        e.appendChild(m_images->write(doc, QLatin1String("images")));

    if (m_includes != 0)
        e.appendChild(m_includes->write(doc, QLatin1String("includes")));

    if (m_resources != 0)
        e.appendChild(m_resources->write(doc, QLatin1String("resources")));

    if (m_connections != 0)
        e.appendChild(m_connections->write(doc, QLatin1String("connections")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomUI::setElementAuthor(const QString& a)
{
    bit.author = true;
    m_author = a;
}

void DomUI::setElementComment(const QString& a)
{
    bit.comment = true;
    m_comment = a;
}

void DomUI::setElementExportMacro(const QString& a)
{
    bit.exportMacro = true;
    m_exportMacro = a;
}

void DomUI::setElementClass(const QString& a)
{
    bit.klass = true;
    m_class = a;
}

void DomUI::setElementWidget(DomWidget* a)
{
    bit.widget = true;
    delete m_widget;
    m_widget = a;
}

void DomUI::setElementLayoutDefault(DomLayoutDefault* a)
{
    bit.layoutDefault = true;
    delete m_layoutDefault;
    m_layoutDefault = a;
}

void DomUI::setElementLayoutFunction(DomLayoutFunction* a)
{
    bit.layoutFunction = true;
    delete m_layoutFunction;
    m_layoutFunction = a;
}

void DomUI::setElementPixmapFunction(const QString& a)
{
    bit.pixmapFunction = true;
    m_pixmapFunction = a;
}

void DomUI::setElementCustomWidgets(DomCustomWidgets* a)
{
    bit.customWidgets = true;
    delete m_customWidgets;
    m_customWidgets = a;
}

void DomUI::setElementTabStops(DomTabStops* a)
{
    bit.tabStops = true;
    delete m_tabStops;
    m_tabStops = a;
}

void DomUI::setElementImages(DomImages* a)
{
    bit.images = true;
    delete m_images;
    m_images = a;
}

void DomUI::setElementIncludes(DomIncludes* a)
{
    bit.includes = true;
    delete m_includes;
    m_includes = a;
}

void DomUI::setElementResources(DomResources* a)
{
    bit.resources = true;
    delete m_resources;
    m_resources = a;
}

void DomUI::setElementConnections(DomConnections* a)
{
    bit.connections = true;
    delete m_connections;
    m_connections = a;
}

void DomIncludes::clear(bool clear_all)
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomIncludes::DomIncludes(): bits(0)
{
}

DomIncludes::~DomIncludes()
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();
}

void DomIncludes::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("include")) {
            DomInclude *v = new DomInclude();
            v->read(e);
            m_include.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomIncludes::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("includes") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_include.size(); ++i) {
        DomInclude* v = m_include[i];
        QDomNode child = v->write(doc, QLatin1String("include"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomIncludes::setElementInclude(const QList<DomInclude*>& a)
{
    bit.include = true;
    m_include = a;
}

void DomInclude::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_location = false;
        m_has_attr_impldecl = false;
    }

}

DomInclude::DomInclude(): bits(0)
{
    m_has_attr_location = false;
    m_has_attr_impldecl = false;
}

DomInclude::~DomInclude()
{
}

void DomInclude::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("location")))
        setAttributeLocation(node.attribute(QLatin1String("location")));
    if (node.hasAttribute(QLatin1String("impldecl")))
        setAttributeImpldecl(node.attribute(QLatin1String("impldecl")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomInclude::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("include") : tagName.toLower());

    QDomElement child;

    if (hasAttributeLocation())
        e.setAttribute(QLatin1String("location"), attributeLocation());

    if (hasAttributeImpldecl())
        e.setAttribute(QLatin1String("impldecl"), attributeImpldecl());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomResources::clear(bool clear_all)
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
    }

}

DomResources::DomResources(): bits(0)
{
    m_has_attr_name = false;
}

DomResources::~DomResources()
{
    for (int i = 0; i < m_include.size(); ++i)
        delete m_include[i];
    m_include.clear();
}

void DomResources::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("include")) {
            DomResource *v = new DomResource();
            v->read(e);
            m_include.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomResources::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("resources") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_include.size(); ++i) {
        DomResource* v = m_include[i];
        QDomNode child = v->write(doc, QLatin1String("include"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomResources::setElementInclude(const QList<DomResource*>& a)
{
    bit.include = true;
    m_include = a;
}

void DomResource::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_location = false;
    }

}

DomResource::DomResource(): bits(0)
{
    m_has_attr_location = false;
}

DomResource::~DomResource()
{
}

void DomResource::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("location")))
        setAttributeLocation(node.attribute(QLatin1String("location")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomResource::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("resource") : tagName.toLower());

    QDomElement child;

    if (hasAttributeLocation())
        e.setAttribute(QLatin1String("location"), attributeLocation());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomActionGroup::clear(bool clear_all)
{
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
    }

}

DomActionGroup::DomActionGroup(): bits(0)
{
    m_has_attr_name = false;
}

DomActionGroup::~DomActionGroup()
{
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
}

void DomActionGroup::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("action")) {
            DomAction *v = new DomAction();
            v->read(e);
            m_action.append(v);
            continue;
        }
        if (tag == QLatin1String("actiongroup")) {
            DomActionGroup *v = new DomActionGroup();
            v->read(e);
            m_actionGroup.append(v);
            continue;
        }
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomActionGroup::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("actiongroup") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_action.size(); ++i) {
        DomAction* v = m_action[i];
        QDomNode child = v->write(doc, QLatin1String("action"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_actionGroup.size(); ++i) {
        DomActionGroup* v = m_actionGroup[i];
        QDomNode child = v->write(doc, QLatin1String("actiongroup"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomActionGroup::setElementAction(const QList<DomAction*>& a)
{
    bit.action = true;
    m_action = a;
}

void DomActionGroup::setElementActionGroup(const QList<DomActionGroup*>& a)
{
    bit.actionGroup = true;
    m_actionGroup = a;
}

void DomActionGroup::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomActionGroup::setElementAttribute(const QList<DomProperty*>& a)
{
    bit.attribute = true;
    m_attribute = a;
}

void DomAction::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
        m_has_attr_menu = false;
    }

}

DomAction::DomAction(): bits(0)
{
    m_has_attr_name = false;
    m_has_attr_menu = false;
}

DomAction::~DomAction()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
}

void DomAction::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));
    if (node.hasAttribute(QLatin1String("menu")))
        setAttributeMenu(node.attribute(QLatin1String("menu")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomAction::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("action") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (hasAttributeMenu())
        e.setAttribute(QLatin1String("menu"), attributeMenu());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomAction::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomAction::setElementAttribute(const QList<DomProperty*>& a)
{
    bit.attribute = true;
    m_attribute = a;
}

void DomActionRef::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
    }

}

DomActionRef::DomActionRef(): bits(0)
{
    m_has_attr_name = false;
}

DomActionRef::~DomActionRef()
{
}

void DomActionRef::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomActionRef::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("actionref") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomImages::clear(bool clear_all)
{
    for (int i = 0; i < m_image.size(); ++i)
        delete m_image[i];
    m_image.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomImages::DomImages(): bits(0)
{
}

DomImages::~DomImages()
{
    for (int i = 0; i < m_image.size(); ++i)
        delete m_image[i];
    m_image.clear();
}

void DomImages::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("image")) {
            DomImage *v = new DomImage();
            v->read(e);
            m_image.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomImages::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("images") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_image.size(); ++i) {
        DomImage* v = m_image[i];
        QDomNode child = v->write(doc, QLatin1String("image"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomImages::setElementImage(const QList<DomImage*>& a)
{
    bit.image = true;
    m_image = a;
}

void DomImage::clear(bool clear_all)
{
    delete m_data;

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
    }

    m_data = 0;
}

DomImage::DomImage(): bits(0)
{
    m_has_attr_name = false;
    m_data = 0;
}

DomImage::~DomImage()
{
    delete m_data;
}

void DomImage::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("data")) {
            DomImageData *v = new DomImageData();
            v->read(e);
            setElementData(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomImage::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("image") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (m_data != 0)
        e.appendChild(m_data->write(doc, QLatin1String("data")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomImage::setElementData(DomImageData* a)
{
    bit.data = true;
    delete m_data;
    m_data = a;
}

void DomImageData::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_format = false;
        m_has_attr_length = false;
        m_attr_length = 0;
    }

}

DomImageData::DomImageData(): bits(0)
{
    m_has_attr_format = false;
    m_has_attr_length = false;
    m_attr_length = 0;
}

DomImageData::~DomImageData()
{
}

void DomImageData::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("format")))
        setAttributeFormat(node.attribute(QLatin1String("format")));
    if (node.hasAttribute(QLatin1String("length")))
        setAttributeLength(node.attribute(QLatin1String("length")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomImageData::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("imagedata") : tagName.toLower());

    QDomElement child;

    if (hasAttributeFormat())
        e.setAttribute(QLatin1String("format"), attributeFormat());

    if (hasAttributeLength())
        e.setAttribute(QLatin1String("length"), attributeLength());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomCustomWidgets::clear(bool clear_all)
{
    for (int i = 0; i < m_customWidget.size(); ++i)
        delete m_customWidget[i];
    m_customWidget.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomCustomWidgets::DomCustomWidgets(): bits(0)
{
}

DomCustomWidgets::~DomCustomWidgets()
{
    for (int i = 0; i < m_customWidget.size(); ++i)
        delete m_customWidget[i];
    m_customWidget.clear();
}

void DomCustomWidgets::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("customwidget")) {
            DomCustomWidget *v = new DomCustomWidget();
            v->read(e);
            m_customWidget.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomCustomWidgets::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("customwidgets") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_customWidget.size(); ++i) {
        DomCustomWidget* v = m_customWidget[i];
        QDomNode child = v->write(doc, QLatin1String("customwidget"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomCustomWidgets::setElementCustomWidget(const QList<DomCustomWidget*>& a)
{
    bit.customWidget = true;
    m_customWidget = a;
}

void DomHeader::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_location = false;
    }

}

DomHeader::DomHeader(): bits(0)
{
    m_has_attr_location = false;
}

DomHeader::~DomHeader()
{
}

void DomHeader::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("location")))
        setAttributeLocation(node.attribute(QLatin1String("location")));

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomHeader::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("header") : tagName.toLower());

    QDomElement child;

    if (hasAttributeLocation())
        e.setAttribute(QLatin1String("location"), attributeLocation());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomCustomWidget::clear(bool clear_all)
{
    delete m_header;
    delete m_sizeHint;
    delete m_sizePolicy;
    delete m_properties;

    if (clear_all) {
        m_text = QString();
    }

    m_header = 0;
    m_sizeHint = 0;
    m_container = 0;
    m_sizePolicy = 0;
    m_properties = 0;
}

DomCustomWidget::DomCustomWidget(): bits(0)
{
    m_header = 0;
    m_sizeHint = 0;
    m_container = 0;
    m_sizePolicy = 0;
    m_properties = 0;
}

DomCustomWidget::~DomCustomWidget()
{
    delete m_header;
    delete m_sizeHint;
    delete m_sizePolicy;
    delete m_properties;
}

void DomCustomWidget::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("class")) {
            setElementClass(e.text());
            continue;
        }
        if (tag == QLatin1String("extends")) {
            setElementExtends(e.text());
            continue;
        }
        if (tag == QLatin1String("header")) {
            DomHeader *v = new DomHeader();
            v->read(e);
            setElementHeader(v);
            continue;
        }
        if (tag == QLatin1String("sizehint")) {
            DomSize *v = new DomSize();
            v->read(e);
            setElementSizeHint(v);
            continue;
        }
        if (tag == QLatin1String("container")) {
            setElementContainer(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("sizepolicy")) {
            DomSizePolicyData *v = new DomSizePolicyData();
            v->read(e);
            setElementSizePolicy(v);
            continue;
        }
        if (tag == QLatin1String("pixmap")) {
            setElementPixmap(e.text());
            continue;
        }
        if (tag == QLatin1String("properties")) {
            DomProperties *v = new DomProperties();
            v->read(e);
            setElementProperties(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomCustomWidget::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("customwidget") : tagName.toLower());

    QDomElement child;

    if (bit.klass) {
        child = doc.createElement(QLatin1String("class"));
        child.appendChild(doc.createTextNode(m_class));
        e.appendChild(child);
    }

    if (bit.extends) {
        child = doc.createElement(QLatin1String("extends"));
        child.appendChild(doc.createTextNode(m_extends));
        e.appendChild(child);
    }

    if (m_header != 0)
        e.appendChild(m_header->write(doc, QLatin1String("header")));

    if (m_sizeHint != 0)
        e.appendChild(m_sizeHint->write(doc, QLatin1String("sizehint")));

    if (bit.container) {
        child = doc.createElement(QLatin1String("container"));
        child.appendChild(doc.createTextNode(QString::number(m_container)));
        e.appendChild(child);
    }

    if (m_sizePolicy != 0)
        e.appendChild(m_sizePolicy->write(doc, QLatin1String("sizepolicy")));

    if (bit.pixmap) {
        child = doc.createElement(QLatin1String("pixmap"));
        child.appendChild(doc.createTextNode(m_pixmap));
        e.appendChild(child);
    }

    if (m_properties != 0)
        e.appendChild(m_properties->write(doc, QLatin1String("properties")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomCustomWidget::setElementClass(const QString& a)
{
    bit.klass = true;
    m_class = a;
}

void DomCustomWidget::setElementExtends(const QString& a)
{
    bit.extends = true;
    m_extends = a;
}

void DomCustomWidget::setElementHeader(DomHeader* a)
{
    bit.header = true;
    delete m_header;
    m_header = a;
}

void DomCustomWidget::setElementSizeHint(DomSize* a)
{
    bit.sizeHint = true;
    delete m_sizeHint;
    m_sizeHint = a;
}

void DomCustomWidget::setElementContainer(int a)
{
    bit.container = true;
    m_container = a;
}

void DomCustomWidget::setElementSizePolicy(DomSizePolicyData* a)
{
    bit.sizePolicy = true;
    delete m_sizePolicy;
    m_sizePolicy = a;
}

void DomCustomWidget::setElementPixmap(const QString& a)
{
    bit.pixmap = true;
    m_pixmap = a;
}

void DomCustomWidget::setElementProperties(DomProperties* a)
{
    bit.properties = true;
    delete m_properties;
    m_properties = a;
}

void DomProperties::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomProperties::DomProperties(): bits(0)
{
}

DomProperties::~DomProperties()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

void DomProperties::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomPropertyData *v = new DomPropertyData();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomProperties::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("properties") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomPropertyData* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomProperties::setElementProperty(const QList<DomPropertyData*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomPropertyData::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_type = false;
    }

}

DomPropertyData::DomPropertyData(): bits(0)
{
    m_has_attr_type = false;
}

DomPropertyData::~DomPropertyData()
{
}

void DomPropertyData::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("type")))
        setAttributeType(node.attribute(QLatin1String("type")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomPropertyData::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("propertydata") : tagName.toLower());

    QDomElement child;

    if (hasAttributeType())
        e.setAttribute(QLatin1String("type"), attributeType());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomSizePolicyData::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_horData = 0;
    m_verData = 0;
}

DomSizePolicyData::DomSizePolicyData(): bits(0)
{
    m_horData = 0;
    m_verData = 0;
}

DomSizePolicyData::~DomSizePolicyData()
{
}

void DomSizePolicyData::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hordata")) {
            setElementHorData(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("verdata")) {
            setElementVerData(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomSizePolicyData::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("sizepolicydata") : tagName.toLower());

    QDomElement child;

    if (bit.horData) {
        child = doc.createElement(QLatin1String("hordata"));
        child.appendChild(doc.createTextNode(QString::number(m_horData)));
        e.appendChild(child);
    }

    if (bit.verData) {
        child = doc.createElement(QLatin1String("verdata"));
        child.appendChild(doc.createTextNode(QString::number(m_verData)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomSizePolicyData::setElementHorData(int a)
{
    bit.horData = true;
    m_horData = a;
}

void DomSizePolicyData::setElementVerData(int a)
{
    bit.verData = true;
    m_verData = a;
}

void DomLayoutDefault::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_spacing = false;
        m_attr_spacing = 0;
        m_has_attr_margin = false;
        m_attr_margin = 0;
    }

}

DomLayoutDefault::DomLayoutDefault(): bits(0)
{
    m_has_attr_spacing = false;
    m_attr_spacing = 0;
    m_has_attr_margin = false;
    m_attr_margin = 0;
}

DomLayoutDefault::~DomLayoutDefault()
{
}

void DomLayoutDefault::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("spacing")))
        setAttributeSpacing(node.attribute(QLatin1String("spacing")).toInt());
    if (node.hasAttribute(QLatin1String("margin")))
        setAttributeMargin(node.attribute(QLatin1String("margin")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomLayoutDefault::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("layoutdefault") : tagName.toLower());

    QDomElement child;

    if (hasAttributeSpacing())
        e.setAttribute(QLatin1String("spacing"), attributeSpacing());

    if (hasAttributeMargin())
        e.setAttribute(QLatin1String("margin"), attributeMargin());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomLayoutFunction::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_spacing = false;
        m_has_attr_margin = false;
    }

}

DomLayoutFunction::DomLayoutFunction(): bits(0)
{
    m_has_attr_spacing = false;
    m_has_attr_margin = false;
}

DomLayoutFunction::~DomLayoutFunction()
{
}

void DomLayoutFunction::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("spacing")))
        setAttributeSpacing(node.attribute(QLatin1String("spacing")));
    if (node.hasAttribute(QLatin1String("margin")))
        setAttributeMargin(node.attribute(QLatin1String("margin")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomLayoutFunction::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("layoutfunction") : tagName.toLower());

    QDomElement child;

    if (hasAttributeSpacing())
        e.setAttribute(QLatin1String("spacing"), attributeSpacing());

    if (hasAttributeMargin())
        e.setAttribute(QLatin1String("margin"), attributeMargin());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomTabStops::clear(bool clear_all)
{
    m_tabStop.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomTabStops::DomTabStops(): bits(0)
{
}

DomTabStops::~DomTabStops()
{
    m_tabStop.clear();
}

void DomTabStops::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("tabstop")) {
            m_tabStop.append(e.text());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomTabStops::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("tabstops") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_tabStop.size(); ++i) {
        QString v = m_tabStop[i];
	if (bit.tabStop) {
            QDomNode child = doc.createElement(QLatin1String("tabstop"));
            child.appendChild(doc.createTextNode(v));
            e.appendChild(child);
	}
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomTabStops::setElementTabStop(const QStringList& a)
{
    bit.tabStop = true;
    m_tabStop = a;
}

void DomLayout::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_class = false;
    }

}

DomLayout::DomLayout(): bits(0)
{
    m_has_attr_class = false;
}

DomLayout::~DomLayout()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
}

void DomLayout::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("class")))
        setAttributeClass(node.attribute(QLatin1String("class")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
        if (tag == QLatin1String("item")) {
            DomLayoutItem *v = new DomLayoutItem();
            v->read(e);
            m_item.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomLayout::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("layout") : tagName.toLower());

    QDomElement child;

    if (hasAttributeClass())
        e.setAttribute(QLatin1String("class"), attributeClass());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_item.size(); ++i) {
        DomLayoutItem* v = m_item[i];
        QDomNode child = v->write(doc, QLatin1String("item"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomLayout::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomLayout::setElementAttribute(const QList<DomProperty*>& a)
{
    bit.attribute = true;
    m_attribute = a;
}

void DomLayout::setElementItem(const QList<DomLayoutItem*>& a)
{
    bit.item = true;
    m_item = a;
}

void DomLayoutItem::clear(bool clear_all)
{
    delete m_widget;
    delete m_layout;
    delete m_spacer;

    if (clear_all) {
        m_text = QString();
        m_has_attr_row = false;
        m_attr_row = 0;
        m_has_attr_column = false;
        m_attr_column = 0;
        m_has_attr_rowSpan = false;
        m_attr_rowSpan = 0;
        m_has_attr_colSpan = false;
        m_attr_colSpan = 0;
    }

    m_kind = Unknown;

    m_widget = 0;
    m_layout = 0;
    m_spacer = 0;
}

DomLayoutItem::DomLayoutItem()
{
    m_kind = Unknown;

    m_has_attr_row = false;
    m_attr_row = 0;
    m_has_attr_column = false;
    m_attr_column = 0;
    m_has_attr_rowSpan = false;
    m_attr_rowSpan = 0;
    m_has_attr_colSpan = false;
    m_attr_colSpan = 0;
    m_widget = 0;
    m_layout = 0;
    m_spacer = 0;
}

DomLayoutItem::~DomLayoutItem()
{
    delete m_widget;
    delete m_layout;
    delete m_spacer;
}

void DomLayoutItem::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("row")))
        setAttributeRow(node.attribute(QLatin1String("row")).toInt());
    if (node.hasAttribute(QLatin1String("column")))
        setAttributeColumn(node.attribute(QLatin1String("column")).toInt());
    if (node.hasAttribute(QLatin1String("rowspan")))
        setAttributeRowSpan(node.attribute(QLatin1String("rowspan")).toInt());
    if (node.hasAttribute(QLatin1String("colspan")))
        setAttributeColSpan(node.attribute(QLatin1String("colspan")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("widget")) {
            DomWidget *v = new DomWidget();
            v->read(e);
            setElementWidget(v);
            continue;
        }
        if (tag == QLatin1String("layout")) {
            DomLayout *v = new DomLayout();
            v->read(e);
            setElementLayout(v);
            continue;
        }
        if (tag == QLatin1String("spacer")) {
            DomSpacer *v = new DomSpacer();
            v->read(e);
            setElementSpacer(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomLayoutItem::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("layoutitem") : tagName.toLower());

    QDomElement child;

    if (hasAttributeRow())
        e.setAttribute(QLatin1String("row"), attributeRow());

    if (hasAttributeColumn())
        e.setAttribute(QLatin1String("column"), attributeColumn());

    if (hasAttributeRowSpan())
        e.setAttribute(QLatin1String("rowspan"), attributeRowSpan());

    if (hasAttributeColSpan())
        e.setAttribute(QLatin1String("colspan"), attributeColSpan());

    switch(kind()) {
    case Widget: {
        DomWidget* v = elementWidget();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("widget"));
            e.appendChild(child);
        }
        break;
    }
    case Layout: {
        DomLayout* v = elementLayout();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("layout"));
            e.appendChild(child);
        }
        break;
    }
    case Spacer: {
        DomSpacer* v = elementSpacer();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("spacer"));
            e.appendChild(child);
        }
        break;
    }
    default:
        break;
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomLayoutItem::setElementWidget(DomWidget* a)
{
    clear(false);
    m_kind = Widget;
    m_widget = a;
}

void DomLayoutItem::setElementLayout(DomLayout* a)
{
    clear(false);
    m_kind = Layout;
    m_layout = a;
}

void DomLayoutItem::setElementSpacer(DomSpacer* a)
{
    clear(false);
    m_kind = Spacer;
    m_spacer = a;
}

void DomRow::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomRow::DomRow(): bits(0)
{
}

DomRow::~DomRow()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

void DomRow::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomRow::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("row") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomRow::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomColumn::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomColumn::DomColumn(): bits(0)
{
}

DomColumn::~DomColumn()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

void DomColumn::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomColumn::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("column") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomColumn::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomItem::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_row = false;
        m_attr_row = 0;
        m_has_attr_column = false;
        m_attr_column = 0;
    }

}

DomItem::DomItem(): bits(0)
{
    m_has_attr_row = false;
    m_attr_row = 0;
    m_has_attr_column = false;
    m_attr_column = 0;
}

DomItem::~DomItem()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
}

void DomItem::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("row")))
        setAttributeRow(node.attribute(QLatin1String("row")).toInt());
    if (node.hasAttribute(QLatin1String("column")))
        setAttributeColumn(node.attribute(QLatin1String("column")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("item")) {
            DomItem *v = new DomItem();
            v->read(e);
            m_item.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomItem::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("item") : tagName.toLower());

    QDomElement child;

    if (hasAttributeRow())
        e.setAttribute(QLatin1String("row"), attributeRow());

    if (hasAttributeColumn())
        e.setAttribute(QLatin1String("column"), attributeColumn());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_item.size(); ++i) {
        DomItem* v = m_item[i];
        QDomNode child = v->write(doc, QLatin1String("item"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomItem::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomItem::setElementItem(const QList<DomItem*>& a)
{
    bit.item = true;
    m_item = a;
}

void DomWidget::clear(bool clear_all)
{
    m_class.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_row.size(); ++i)
        delete m_row[i];
    m_row.clear();
    for (int i = 0; i < m_column.size(); ++i)
        delete m_column[i];
    m_column.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
    for (int i = 0; i < m_layout.size(); ++i)
        delete m_layout[i];
    m_layout.clear();
    for (int i = 0; i < m_widget.size(); ++i)
        delete m_widget[i];
    m_widget.clear();
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_addAction.size(); ++i)
        delete m_addAction[i];
    m_addAction.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_class = false;
        m_has_attr_name = false;
    }

}

DomWidget::DomWidget(): bits(0)
{
    m_has_attr_class = false;
    m_has_attr_name = false;
}

DomWidget::~DomWidget()
{
    m_class.clear();
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
    for (int i = 0; i < m_attribute.size(); ++i)
        delete m_attribute[i];
    m_attribute.clear();
    for (int i = 0; i < m_row.size(); ++i)
        delete m_row[i];
    m_row.clear();
    for (int i = 0; i < m_column.size(); ++i)
        delete m_column[i];
    m_column.clear();
    for (int i = 0; i < m_item.size(); ++i)
        delete m_item[i];
    m_item.clear();
    for (int i = 0; i < m_layout.size(); ++i)
        delete m_layout[i];
    m_layout.clear();
    for (int i = 0; i < m_widget.size(); ++i)
        delete m_widget[i];
    m_widget.clear();
    for (int i = 0; i < m_action.size(); ++i)
        delete m_action[i];
    m_action.clear();
    for (int i = 0; i < m_actionGroup.size(); ++i)
        delete m_actionGroup[i];
    m_actionGroup.clear();
    for (int i = 0; i < m_addAction.size(); ++i)
        delete m_addAction[i];
    m_addAction.clear();
}

void DomWidget::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("class")))
        setAttributeClass(node.attribute(QLatin1String("class")));
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("class")) {
            m_class.append(e.text());
            continue;
        }
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
        if (tag == QLatin1String("attribute")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_attribute.append(v);
            continue;
        }
        if (tag == QLatin1String("row")) {
            DomRow *v = new DomRow();
            v->read(e);
            m_row.append(v);
            continue;
        }
        if (tag == QLatin1String("column")) {
            DomColumn *v = new DomColumn();
            v->read(e);
            m_column.append(v);
            continue;
        }
        if (tag == QLatin1String("item")) {
            DomItem *v = new DomItem();
            v->read(e);
            m_item.append(v);
            continue;
        }
        if (tag == QLatin1String("layout")) {
            DomLayout *v = new DomLayout();
            v->read(e);
            m_layout.append(v);
            continue;
        }
        if (tag == QLatin1String("widget")) {
            DomWidget *v = new DomWidget();
            v->read(e);
            m_widget.append(v);
            continue;
        }
        if (tag == QLatin1String("action")) {
            DomAction *v = new DomAction();
            v->read(e);
            m_action.append(v);
            continue;
        }
        if (tag == QLatin1String("actiongroup")) {
            DomActionGroup *v = new DomActionGroup();
            v->read(e);
            m_actionGroup.append(v);
            continue;
        }
        if (tag == QLatin1String("addaction")) {
            DomActionRef *v = new DomActionRef();
            v->read(e);
            m_addAction.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomWidget::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("widget") : tagName.toLower());

    QDomElement child;

    if (hasAttributeClass())
        e.setAttribute(QLatin1String("class"), attributeClass());

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_class.size(); ++i) {
        QString v = m_class[i];
	if (bit.klass) {
            QDomNode child = doc.createElement(QLatin1String("class"));
            child.appendChild(doc.createTextNode(v));
            e.appendChild(child);
	}
    }
    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_attribute.size(); ++i) {
        DomProperty* v = m_attribute[i];
        QDomNode child = v->write(doc, QLatin1String("attribute"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_row.size(); ++i) {
        DomRow* v = m_row[i];
        QDomNode child = v->write(doc, QLatin1String("row"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_column.size(); ++i) {
        DomColumn* v = m_column[i];
        QDomNode child = v->write(doc, QLatin1String("column"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_item.size(); ++i) {
        DomItem* v = m_item[i];
        QDomNode child = v->write(doc, QLatin1String("item"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_layout.size(); ++i) {
        DomLayout* v = m_layout[i];
        QDomNode child = v->write(doc, QLatin1String("layout"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_widget.size(); ++i) {
        DomWidget* v = m_widget[i];
        QDomNode child = v->write(doc, QLatin1String("widget"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_action.size(); ++i) {
        DomAction* v = m_action[i];
        QDomNode child = v->write(doc, QLatin1String("action"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_actionGroup.size(); ++i) {
        DomActionGroup* v = m_actionGroup[i];
        QDomNode child = v->write(doc, QLatin1String("actiongroup"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_addAction.size(); ++i) {
        DomActionRef* v = m_addAction[i];
        QDomNode child = v->write(doc, QLatin1String("addaction"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomWidget::setElementClass(const QStringList& a)
{
    bit.klass = true;
    m_class = a;
}

void DomWidget::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomWidget::setElementAttribute(const QList<DomProperty*>& a)
{
    bit.attribute = true;
    m_attribute = a;
}

void DomWidget::setElementRow(const QList<DomRow*>& a)
{
    bit.row = true;
    m_row = a;
}

void DomWidget::setElementColumn(const QList<DomColumn*>& a)
{
    bit.column = true;
    m_column = a;
}

void DomWidget::setElementItem(const QList<DomItem*>& a)
{
    bit.item = true;
    m_item = a;
}

void DomWidget::setElementLayout(const QList<DomLayout*>& a)
{
    bit.layout = true;
    m_layout = a;
}

void DomWidget::setElementWidget(const QList<DomWidget*>& a)
{
    bit.widget = true;
    m_widget = a;
}

void DomWidget::setElementAction(const QList<DomAction*>& a)
{
    bit.action = true;
    m_action = a;
}

void DomWidget::setElementActionGroup(const QList<DomActionGroup*>& a)
{
    bit.actionGroup = true;
    m_actionGroup = a;
}

void DomWidget::setElementAddAction(const QList<DomActionRef*>& a)
{
    bit.addAction = true;
    m_addAction = a;
}

void DomSpacer::clear(bool clear_all)
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
    }

}

DomSpacer::DomSpacer(): bits(0)
{
    m_has_attr_name = false;
}

DomSpacer::~DomSpacer()
{
    for (int i = 0; i < m_property.size(); ++i)
        delete m_property[i];
    m_property.clear();
}

void DomSpacer::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("property")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            m_property.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomSpacer::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("spacer") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    for (int i = 0; i < m_property.size(); ++i) {
        DomProperty* v = m_property[i];
        QDomNode child = v->write(doc, QLatin1String("property"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomSpacer::setElementProperty(const QList<DomProperty*>& a)
{
    bit.property = true;
    m_property = a;
}

void DomColor::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_alpha = false;
        m_attr_alpha = 0;
    }

    m_red = 0;
    m_green = 0;
    m_blue = 0;
}

DomColor::DomColor(): bits(0)
{
    m_has_attr_alpha = false;
    m_attr_alpha = 0;
    m_red = 0;
    m_green = 0;
    m_blue = 0;
}

DomColor::~DomColor()
{
}

void DomColor::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("alpha")))
        setAttributeAlpha(node.attribute(QLatin1String("alpha")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("red")) {
            setElementRed(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("green")) {
            setElementGreen(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("blue")) {
            setElementBlue(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomColor::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("color") : tagName.toLower());

    QDomElement child;

    if (hasAttributeAlpha())
        e.setAttribute(QLatin1String("alpha"), attributeAlpha());

    if (bit.red) {
        child = doc.createElement(QLatin1String("red"));
        child.appendChild(doc.createTextNode(QString::number(m_red)));
        e.appendChild(child);
    }

    if (bit.green) {
        child = doc.createElement(QLatin1String("green"));
        child.appendChild(doc.createTextNode(QString::number(m_green)));
        e.appendChild(child);
    }

    if (bit.blue) {
        child = doc.createElement(QLatin1String("blue"));
        child.appendChild(doc.createTextNode(QString::number(m_blue)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomColor::setElementRed(int a)
{
    bit.red = true;
    m_red = a;
}

void DomColor::setElementGreen(int a)
{
    bit.green = true;
    m_green = a;
}

void DomColor::setElementBlue(int a)
{
    bit.blue = true;
    m_blue = a;
}

void DomGradientStop::clear(bool clear_all)
{
    delete m_color;

    if (clear_all) {
        m_text = QString();
        m_has_attr_position = false;
        m_attr_position = 0.0;
    }

    m_color = 0;
}

DomGradientStop::DomGradientStop(): bits(0)
{
    m_has_attr_position = false;
    m_attr_position = 0.0;
    m_color = 0;
}

DomGradientStop::~DomGradientStop()
{
    delete m_color;
}

void DomGradientStop::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("position")))
        setAttributePosition(node.attribute(QLatin1String("position")).toDouble());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("color")) {
            DomColor *v = new DomColor();
            v->read(e);
            setElementColor(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomGradientStop::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("gradientstop") : tagName.toLower());

    QDomElement child;

    if (hasAttributePosition())
        e.setAttribute(QLatin1String("position"), attributePosition());

    if (m_color != 0)
        e.appendChild(m_color->write(doc, QLatin1String("color")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomGradientStop::setElementColor(DomColor* a)
{
    bit.color = true;
    delete m_color;
    m_color = a;
}

void DomGradient::clear(bool clear_all)
{
    for (int i = 0; i < m_gradientStop.size(); ++i)
        delete m_gradientStop[i];
    m_gradientStop.clear();

    if (clear_all) {
        m_text = QString();
        m_has_attr_startX = false;
        m_attr_startX = 0.0;
        m_has_attr_startY = false;
        m_attr_startY = 0.0;
        m_has_attr_endX = false;
        m_attr_endX = 0.0;
        m_has_attr_endY = false;
        m_attr_endY = 0.0;
        m_has_attr_centralX = false;
        m_attr_centralX = 0.0;
        m_has_attr_centralY = false;
        m_attr_centralY = 0.0;
        m_has_attr_focalX = false;
        m_attr_focalX = 0.0;
        m_has_attr_focalY = false;
        m_attr_focalY = 0.0;
        m_has_attr_radius = false;
        m_attr_radius = 0.0;
        m_has_attr_angle = false;
        m_attr_angle = 0.0;
        m_has_attr_type = false;
        m_has_attr_spread = false;
        m_has_attr_coordinateMode = false;
    }

}

DomGradient::DomGradient(): bits(0)
{
    m_has_attr_startX = false;
    m_attr_startX = 0.0;
    m_has_attr_startY = false;
    m_attr_startY = 0.0;
    m_has_attr_endX = false;
    m_attr_endX = 0.0;
    m_has_attr_endY = false;
    m_attr_endY = 0.0;
    m_has_attr_centralX = false;
    m_attr_centralX = 0.0;
    m_has_attr_centralY = false;
    m_attr_centralY = 0.0;
    m_has_attr_focalX = false;
    m_attr_focalX = 0.0;
    m_has_attr_focalY = false;
    m_attr_focalY = 0.0;
    m_has_attr_radius = false;
    m_attr_radius = 0.0;
    m_has_attr_angle = false;
    m_attr_angle = 0.0;
    m_has_attr_type = false;
    m_has_attr_spread = false;
    m_has_attr_coordinateMode = false;
}

DomGradient::~DomGradient()
{
    for (int i = 0; i < m_gradientStop.size(); ++i)
        delete m_gradientStop[i];
    m_gradientStop.clear();
}

void DomGradient::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("startx")))
        setAttributeStartX(node.attribute(QLatin1String("startx")).toDouble());
    if (node.hasAttribute(QLatin1String("starty")))
        setAttributeStartY(node.attribute(QLatin1String("starty")).toDouble());
    if (node.hasAttribute(QLatin1String("endx")))
        setAttributeEndX(node.attribute(QLatin1String("endx")).toDouble());
    if (node.hasAttribute(QLatin1String("endy")))
        setAttributeEndY(node.attribute(QLatin1String("endy")).toDouble());
    if (node.hasAttribute(QLatin1String("centralx")))
        setAttributeCentralX(node.attribute(QLatin1String("centralx")).toDouble());
    if (node.hasAttribute(QLatin1String("centraly")))
        setAttributeCentralY(node.attribute(QLatin1String("centraly")).toDouble());
    if (node.hasAttribute(QLatin1String("focalx")))
        setAttributeFocalX(node.attribute(QLatin1String("focalx")).toDouble());
    if (node.hasAttribute(QLatin1String("focaly")))
        setAttributeFocalY(node.attribute(QLatin1String("focaly")).toDouble());
    if (node.hasAttribute(QLatin1String("radius")))
        setAttributeRadius(node.attribute(QLatin1String("radius")).toDouble());
    if (node.hasAttribute(QLatin1String("angle")))
        setAttributeAngle(node.attribute(QLatin1String("angle")).toDouble());
    if (node.hasAttribute(QLatin1String("type")))
        setAttributeType(node.attribute(QLatin1String("type")));
    if (node.hasAttribute(QLatin1String("spread")))
        setAttributeSpread(node.attribute(QLatin1String("spread")));
    if (node.hasAttribute(QLatin1String("coordinatemode")))
        setAttributeCoordinateMode(node.attribute(QLatin1String("coordinatemode")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("gradientstop")) {
            DomGradientStop *v = new DomGradientStop();
            v->read(e);
            m_gradientStop.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomGradient::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("gradient") : tagName.toLower());

    QDomElement child;

    if (hasAttributeStartX())
        e.setAttribute(QLatin1String("startx"), attributeStartX());

    if (hasAttributeStartY())
        e.setAttribute(QLatin1String("starty"), attributeStartY());

    if (hasAttributeEndX())
        e.setAttribute(QLatin1String("endx"), attributeEndX());

    if (hasAttributeEndY())
        e.setAttribute(QLatin1String("endy"), attributeEndY());

    if (hasAttributeCentralX())
        e.setAttribute(QLatin1String("centralx"), attributeCentralX());

    if (hasAttributeCentralY())
        e.setAttribute(QLatin1String("centraly"), attributeCentralY());

    if (hasAttributeFocalX())
        e.setAttribute(QLatin1String("focalx"), attributeFocalX());

    if (hasAttributeFocalY())
        e.setAttribute(QLatin1String("focaly"), attributeFocalY());

    if (hasAttributeRadius())
        e.setAttribute(QLatin1String("radius"), attributeRadius());

    if (hasAttributeAngle())
        e.setAttribute(QLatin1String("angle"), attributeAngle());

    if (hasAttributeType())
        e.setAttribute(QLatin1String("type"), attributeType());

    if (hasAttributeSpread())
        e.setAttribute(QLatin1String("spread"), attributeSpread());

    if (hasAttributeCoordinateMode())
        e.setAttribute(QLatin1String("coordinatemode"), attributeCoordinateMode());

    for (int i = 0; i < m_gradientStop.size(); ++i) {
        DomGradientStop* v = m_gradientStop[i];
        QDomNode child = v->write(doc, QLatin1String("gradientstop"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomGradient::setElementGradientStop(const QList<DomGradientStop*>& a)
{
    bit.gradientStop = true;
    m_gradientStop = a;
}

void DomBrush::clear(bool clear_all)
{
    delete m_color;
    delete m_texture;
    delete m_gradient;

    if (clear_all) {
        m_text = QString();
        m_has_attr_brushStyle = false;
    }

    m_kind = Unknown;

    m_color = 0;
    m_texture = 0;
    m_gradient = 0;
}

DomBrush::DomBrush()
{
    m_kind = Unknown;

    m_has_attr_brushStyle = false;
    m_color = 0;
    m_texture = 0;
    m_gradient = 0;
}

DomBrush::~DomBrush()
{
    delete m_color;
    delete m_texture;
    delete m_gradient;
}

void DomBrush::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("brushstyle")))
        setAttributeBrushStyle(node.attribute(QLatin1String("brushstyle")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("color")) {
            DomColor *v = new DomColor();
            v->read(e);
            setElementColor(v);
            continue;
        }
        if (tag == QLatin1String("texture")) {
            DomProperty *v = new DomProperty();
            v->read(e);
            setElementTexture(v);
            continue;
        }
        if (tag == QLatin1String("gradient")) {
            DomGradient *v = new DomGradient();
            v->read(e);
            setElementGradient(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomBrush::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("brush") : tagName.toLower());

    QDomElement child;

    if (hasAttributeBrushStyle())
        e.setAttribute(QLatin1String("brushstyle"), attributeBrushStyle());

    switch(kind()) {
    case Color: {
        DomColor* v = elementColor();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("color"));
            e.appendChild(child);
        }
        break;
    }
    case Texture: {
        DomProperty* v = elementTexture();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("texture"));
            e.appendChild(child);
        }
        break;
    }
    case Gradient: {
        DomGradient* v = elementGradient();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("gradient"));
            e.appendChild(child);
        }
        break;
    }
    default:
        break;
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomBrush::setElementColor(DomColor* a)
{
    clear(false);
    m_kind = Color;
    m_color = a;
}

void DomBrush::setElementTexture(DomProperty* a)
{
    clear(false);
    m_kind = Texture;
    m_texture = a;
}

void DomBrush::setElementGradient(DomGradient* a)
{
    clear(false);
    m_kind = Gradient;
    m_gradient = a;
}

void DomColorRole::clear(bool clear_all)
{
    delete m_brush;

    if (clear_all) {
        m_text = QString();
        m_has_attr_role = false;
    }

    m_brush = 0;
}

DomColorRole::DomColorRole(): bits(0)
{
    m_has_attr_role = false;
    m_brush = 0;
}

DomColorRole::~DomColorRole()
{
    delete m_brush;
}

void DomColorRole::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("role")))
        setAttributeRole(node.attribute(QLatin1String("role")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("brush")) {
            DomBrush *v = new DomBrush();
            v->read(e);
            setElementBrush(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomColorRole::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("colorrole") : tagName.toLower());

    QDomElement child;

    if (hasAttributeRole())
        e.setAttribute(QLatin1String("role"), attributeRole());

    if (m_brush != 0)
        e.appendChild(m_brush->write(doc, QLatin1String("brush")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomColorRole::setElementBrush(DomBrush* a)
{
    bit.brush = true;
    delete m_brush;
    m_brush = a;
}

void DomColorGroup::clear(bool clear_all)
{
    for (int i = 0; i < m_colorRole.size(); ++i)
        delete m_colorRole[i];
    m_colorRole.clear();
    for (int i = 0; i < m_color.size(); ++i)
        delete m_color[i];
    m_color.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomColorGroup::DomColorGroup(): bits(0)
{
}

DomColorGroup::~DomColorGroup()
{
    for (int i = 0; i < m_colorRole.size(); ++i)
        delete m_colorRole[i];
    m_colorRole.clear();
    for (int i = 0; i < m_color.size(); ++i)
        delete m_color[i];
    m_color.clear();
}

void DomColorGroup::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("colorrole")) {
            DomColorRole *v = new DomColorRole();
            v->read(e);
            m_colorRole.append(v);
            continue;
        }
        if (tag == QLatin1String("color")) {
            DomColor *v = new DomColor();
            v->read(e);
            m_color.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomColorGroup::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("colorgroup") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_colorRole.size(); ++i) {
        DomColorRole* v = m_colorRole[i];
        QDomNode child = v->write(doc, QLatin1String("colorrole"));
        e.appendChild(child);
    }
    for (int i = 0; i < m_color.size(); ++i) {
        DomColor* v = m_color[i];
        QDomNode child = v->write(doc, QLatin1String("color"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomColorGroup::setElementColorRole(const QList<DomColorRole*>& a)
{
    bit.colorRole = true;
    m_colorRole = a;
}

void DomColorGroup::setElementColor(const QList<DomColor*>& a)
{
    bit.color = true;
    m_color = a;
}

void DomPalette::clear(bool clear_all)
{
    delete m_active;
    delete m_inactive;
    delete m_disabled;

    if (clear_all) {
        m_text = QString();
    }

    m_active = 0;
    m_inactive = 0;
    m_disabled = 0;
}

DomPalette::DomPalette(): bits(0)
{
    m_active = 0;
    m_inactive = 0;
    m_disabled = 0;
}

DomPalette::~DomPalette()
{
    delete m_active;
    delete m_inactive;
    delete m_disabled;
}

void DomPalette::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("active")) {
            DomColorGroup *v = new DomColorGroup();
            v->read(e);
            setElementActive(v);
            continue;
        }
        if (tag == QLatin1String("inactive")) {
            DomColorGroup *v = new DomColorGroup();
            v->read(e);
            setElementInactive(v);
            continue;
        }
        if (tag == QLatin1String("disabled")) {
            DomColorGroup *v = new DomColorGroup();
            v->read(e);
            setElementDisabled(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomPalette::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("palette") : tagName.toLower());

    QDomElement child;

    if (m_active != 0)
        e.appendChild(m_active->write(doc, QLatin1String("active")));

    if (m_inactive != 0)
        e.appendChild(m_inactive->write(doc, QLatin1String("inactive")));

    if (m_disabled != 0)
        e.appendChild(m_disabled->write(doc, QLatin1String("disabled")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomPalette::setElementActive(DomColorGroup* a)
{
    bit.active = true;
    delete m_active;
    m_active = a;
}

void DomPalette::setElementInactive(DomColorGroup* a)
{
    bit.inactive = true;
    delete m_inactive;
    m_inactive = a;
}

void DomPalette::setElementDisabled(DomColorGroup* a)
{
    bit.disabled = true;
    delete m_disabled;
    m_disabled = a;
}

void DomFont::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_pointSize = 0;
    m_weight = 0;
    m_italic = false;
    m_bold = false;
    m_underline = false;
    m_strikeOut = false;
}

DomFont::DomFont(): bits(0)
{
    m_pointSize = 0;
    m_weight = 0;
    m_italic = false;
    m_bold = false;
    m_underline = false;
    m_strikeOut = false;
}

DomFont::~DomFont()
{
}

void DomFont::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("family")) {
            setElementFamily(e.text());
            continue;
        }
        if (tag == QLatin1String("pointsize")) {
            setElementPointSize(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("weight")) {
            setElementWeight(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("italic")) {
            setElementItalic((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
        if (tag == QLatin1String("bold")) {
            setElementBold((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
        if (tag == QLatin1String("underline")) {
            setElementUnderline((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
        if (tag == QLatin1String("strikeout")) {
            setElementStrikeOut((e.text() == QLatin1String("true") ? true : false));
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomFont::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("font") : tagName.toLower());

    QDomElement child;

    if (bit.family) {
        child = doc.createElement(QLatin1String("family"));
        child.appendChild(doc.createTextNode(m_family));
        e.appendChild(child);
    }

    if (bit.pointSize) {
        child = doc.createElement(QLatin1String("pointsize"));
        child.appendChild(doc.createTextNode(QString::number(m_pointSize)));
        e.appendChild(child);
    }

    if (bit.weight) {
        child = doc.createElement(QLatin1String("weight"));
        child.appendChild(doc.createTextNode(QString::number(m_weight)));
        e.appendChild(child);
    }

    if (bit.italic) {
        child = doc.createElement(QLatin1String("italic"));
        child.appendChild(doc.createTextNode((m_italic ? QLatin1String("true") : QLatin1String("false"))));
        e.appendChild(child);
    }

    if (bit.bold) {
        child = doc.createElement(QLatin1String("bold"));
        child.appendChild(doc.createTextNode((m_bold ? QLatin1String("true") : QLatin1String("false"))));
        e.appendChild(child);
    }

    if (bit.underline) {
        child = doc.createElement(QLatin1String("underline"));
        child.appendChild(doc.createTextNode((m_underline ? QLatin1String("true") : QLatin1String("false"))));
        e.appendChild(child);
    }

    if (bit.strikeOut) {
        child = doc.createElement(QLatin1String("strikeout"));
        child.appendChild(doc.createTextNode((m_strikeOut ? QLatin1String("true") : QLatin1String("false"))));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomFont::setElementFamily(const QString& a)
{
    bit.family = true;
    m_family = a;
}

void DomFont::setElementPointSize(int a)
{
    bit.pointSize = true;
    m_pointSize = a;
}

void DomFont::setElementWeight(int a)
{
    bit.weight = true;
    m_weight = a;
}

void DomFont::setElementItalic(bool a)
{
    bit.italic = true;
    m_italic = a;
}

void DomFont::setElementBold(bool a)
{
    bit.bold = true;
    m_bold = a;
}

void DomFont::setElementUnderline(bool a)
{
    bit.underline = true;
    m_underline = a;
}

void DomFont::setElementStrikeOut(bool a)
{
    bit.strikeOut = true;
    m_strikeOut = a;
}

void DomPoint::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_x = 0;
    m_y = 0;
}

DomPoint::DomPoint(): bits(0)
{
    m_x = 0;
    m_y = 0;
}

DomPoint::~DomPoint()
{
}

void DomPoint::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomPoint::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("point") : tagName.toLower());

    QDomElement child;

    if (bit.x) {
        child = doc.createElement(QLatin1String("x"));
        child.appendChild(doc.createTextNode(QString::number(m_x)));
        e.appendChild(child);
    }

    if (bit.y) {
        child = doc.createElement(QLatin1String("y"));
        child.appendChild(doc.createTextNode(QString::number(m_y)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomPoint::setElementX(int a)
{
    bit.x = true;
    m_x = a;
}

void DomPoint::setElementY(int a)
{
    bit.y = true;
    m_y = a;
}

void DomRect::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
}

DomRect::DomRect(): bits(0)
{
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
}

DomRect::~DomRect()
{
}

void DomRect::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("width")) {
            setElementWidth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("height")) {
            setElementHeight(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomRect::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("rect") : tagName.toLower());

    QDomElement child;

    if (bit.x) {
        child = doc.createElement(QLatin1String("x"));
        child.appendChild(doc.createTextNode(QString::number(m_x)));
        e.appendChild(child);
    }

    if (bit.y) {
        child = doc.createElement(QLatin1String("y"));
        child.appendChild(doc.createTextNode(QString::number(m_y)));
        e.appendChild(child);
    }

    if (bit.width) {
        child = doc.createElement(QLatin1String("width"));
        child.appendChild(doc.createTextNode(QString::number(m_width)));
        e.appendChild(child);
    }

    if (bit.height) {
        child = doc.createElement(QLatin1String("height"));
        child.appendChild(doc.createTextNode(QString::number(m_height)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomRect::setElementX(int a)
{
    bit.x = true;
    m_x = a;
}

void DomRect::setElementY(int a)
{
    bit.y = true;
    m_y = a;
}

void DomRect::setElementWidth(int a)
{
    bit.width = true;
    m_width = a;
}

void DomRect::setElementHeight(int a)
{
    bit.height = true;
    m_height = a;
}

void DomSizePolicy::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_hSizeType = 0;
    m_vSizeType = 0;
    m_horStretch = 0;
    m_verStretch = 0;
}

DomSizePolicy::DomSizePolicy(): bits(0)
{
    m_hSizeType = 0;
    m_vSizeType = 0;
    m_horStretch = 0;
    m_verStretch = 0;
}

DomSizePolicy::~DomSizePolicy()
{
}

void DomSizePolicy::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hsizetype")) {
            setElementHSizeType(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("vsizetype")) {
            setElementVSizeType(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("horstretch")) {
            setElementHorStretch(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("verstretch")) {
            setElementVerStretch(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomSizePolicy::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("sizepolicy") : tagName.toLower());

    QDomElement child;

    if (bit.hSizeType) {
        child = doc.createElement(QLatin1String("hsizetype"));
        child.appendChild(doc.createTextNode(QString::number(m_hSizeType)));
        e.appendChild(child);
    }

    if (bit.vSizeType) {
        child = doc.createElement(QLatin1String("vsizetype"));
        child.appendChild(doc.createTextNode(QString::number(m_vSizeType)));
        e.appendChild(child);
    }

    if (bit.horStretch) {
        child = doc.createElement(QLatin1String("horstretch"));
        child.appendChild(doc.createTextNode(QString::number(m_horStretch)));
        e.appendChild(child);
    }

    if (bit.verStretch) {
        child = doc.createElement(QLatin1String("verstretch"));
        child.appendChild(doc.createTextNode(QString::number(m_verStretch)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomSizePolicy::setElementHSizeType(int a)
{
    bit.hSizeType = true;
    m_hSizeType = a;
}

void DomSizePolicy::setElementVSizeType(int a)
{
    bit.vSizeType = true;
    m_vSizeType = a;
}

void DomSizePolicy::setElementHorStretch(int a)
{
    bit.horStretch = true;
    m_horStretch = a;
}

void DomSizePolicy::setElementVerStretch(int a)
{
    bit.verStretch = true;
    m_verStretch = a;
}

void DomSize::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_width = 0;
    m_height = 0;
}

DomSize::DomSize(): bits(0)
{
    m_width = 0;
    m_height = 0;
}

DomSize::~DomSize()
{
}

void DomSize::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("width")) {
            setElementWidth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("height")) {
            setElementHeight(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomSize::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("size") : tagName.toLower());

    QDomElement child;

    if (bit.width) {
        child = doc.createElement(QLatin1String("width"));
        child.appendChild(doc.createTextNode(QString::number(m_width)));
        e.appendChild(child);
    }

    if (bit.height) {
        child = doc.createElement(QLatin1String("height"));
        child.appendChild(doc.createTextNode(QString::number(m_height)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomSize::setElementWidth(int a)
{
    bit.width = true;
    m_width = a;
}

void DomSize::setElementHeight(int a)
{
    bit.height = true;
    m_height = a;
}

void DomDate::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_year = 0;
    m_month = 0;
    m_day = 0;
}

DomDate::DomDate(): bits(0)
{
    m_year = 0;
    m_month = 0;
    m_day = 0;
}

DomDate::~DomDate()
{
}

void DomDate::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("year")) {
            setElementYear(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("month")) {
            setElementMonth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("day")) {
            setElementDay(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomDate::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("date") : tagName.toLower());

    QDomElement child;

    if (bit.year) {
        child = doc.createElement(QLatin1String("year"));
        child.appendChild(doc.createTextNode(QString::number(m_year)));
        e.appendChild(child);
    }

    if (bit.month) {
        child = doc.createElement(QLatin1String("month"));
        child.appendChild(doc.createTextNode(QString::number(m_month)));
        e.appendChild(child);
    }

    if (bit.day) {
        child = doc.createElement(QLatin1String("day"));
        child.appendChild(doc.createTextNode(QString::number(m_day)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomDate::setElementYear(int a)
{
    bit.year = true;
    m_year = a;
}

void DomDate::setElementMonth(int a)
{
    bit.month = true;
    m_month = a;
}

void DomDate::setElementDay(int a)
{
    bit.day = true;
    m_day = a;
}

void DomTime::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_hour = 0;
    m_minute = 0;
    m_second = 0;
}

DomTime::DomTime(): bits(0)
{
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
}

DomTime::~DomTime()
{
}

void DomTime::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hour")) {
            setElementHour(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("minute")) {
            setElementMinute(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("second")) {
            setElementSecond(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomTime::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("time") : tagName.toLower());

    QDomElement child;

    if (bit.hour) {
        child = doc.createElement(QLatin1String("hour"));
        child.appendChild(doc.createTextNode(QString::number(m_hour)));
        e.appendChild(child);
    }

    if (bit.minute) {
        child = doc.createElement(QLatin1String("minute"));
        child.appendChild(doc.createTextNode(QString::number(m_minute)));
        e.appendChild(child);
    }

    if (bit.second) {
        child = doc.createElement(QLatin1String("second"));
        child.appendChild(doc.createTextNode(QString::number(m_second)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomTime::setElementHour(int a)
{
    bit.hour = true;
    m_hour = a;
}

void DomTime::setElementMinute(int a)
{
    bit.minute = true;
    m_minute = a;
}

void DomTime::setElementSecond(int a)
{
    bit.second = true;
    m_second = a;
}

void DomDateTime::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_hour = 0;
    m_minute = 0;
    m_second = 0;
    m_year = 0;
    m_month = 0;
    m_day = 0;
}

DomDateTime::DomDateTime(): bits(0)
{
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
    m_year = 0;
    m_month = 0;
    m_day = 0;
}

DomDateTime::~DomDateTime()
{
}

void DomDateTime::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hour")) {
            setElementHour(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("minute")) {
            setElementMinute(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("second")) {
            setElementSecond(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("year")) {
            setElementYear(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("month")) {
            setElementMonth(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("day")) {
            setElementDay(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomDateTime::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("datetime") : tagName.toLower());

    QDomElement child;

    if (bit.hour) {
        child = doc.createElement(QLatin1String("hour"));
        child.appendChild(doc.createTextNode(QString::number(m_hour)));
        e.appendChild(child);
    }

    if (bit.minute) {
        child = doc.createElement(QLatin1String("minute"));
        child.appendChild(doc.createTextNode(QString::number(m_minute)));
        e.appendChild(child);
    }

    if (bit.second) {
        child = doc.createElement(QLatin1String("second"));
        child.appendChild(doc.createTextNode(QString::number(m_second)));
        e.appendChild(child);
    }

    if (bit.year) {
        child = doc.createElement(QLatin1String("year"));
        child.appendChild(doc.createTextNode(QString::number(m_year)));
        e.appendChild(child);
    }

    if (bit.month) {
        child = doc.createElement(QLatin1String("month"));
        child.appendChild(doc.createTextNode(QString::number(m_month)));
        e.appendChild(child);
    }

    if (bit.day) {
        child = doc.createElement(QLatin1String("day"));
        child.appendChild(doc.createTextNode(QString::number(m_day)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomDateTime::setElementHour(int a)
{
    bit.hour = true;
    m_hour = a;
}

void DomDateTime::setElementMinute(int a)
{
    bit.minute = true;
    m_minute = a;
}

void DomDateTime::setElementSecond(int a)
{
    bit.second = true;
    m_second = a;
}

void DomDateTime::setElementYear(int a)
{
    bit.year = true;
    m_year = a;
}

void DomDateTime::setElementMonth(int a)
{
    bit.month = true;
    m_month = a;
}

void DomDateTime::setElementDay(int a)
{
    bit.day = true;
    m_day = a;
}

void DomStringList::clear(bool clear_all)
{
    m_string.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomStringList::DomStringList(): bits(0)
{
}

DomStringList::~DomStringList()
{
    m_string.clear();
}

void DomStringList::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("string")) {
            m_string.append(e.text());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomStringList::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("stringlist") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_string.size(); ++i) {
        QString v = m_string[i];
	if (bit.string) {
            QDomNode child = doc.createElement(QLatin1String("string"));
            child.appendChild(doc.createTextNode(v));
            e.appendChild(child);
	}
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomStringList::setElementString(const QStringList& a)
{
    bit.string = true;
    m_string = a;
}

void DomResourcePixmap::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_resource = false;
        m_has_attr_alias = false;
    }

}

DomResourcePixmap::DomResourcePixmap(): bits(0)
{
    m_has_attr_resource = false;
    m_has_attr_alias = false;
}

DomResourcePixmap::~DomResourcePixmap()
{
}

void DomResourcePixmap::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("resource")))
        setAttributeResource(node.attribute(QLatin1String("resource")));
    if (node.hasAttribute(QLatin1String("alias")))
        setAttributeAlias(node.attribute(QLatin1String("alias")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomResourcePixmap::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("resourcepixmap") : tagName.toLower());

    QDomElement child;

    if (hasAttributeResource())
        e.setAttribute(QLatin1String("resource"), attributeResource());

    if (hasAttributeAlias())
        e.setAttribute(QLatin1String("alias"), attributeAlias());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomString::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_notr = false;
        m_has_attr_comment = false;
    }

}

DomString::DomString(): bits(0)
{
    m_has_attr_notr = false;
    m_has_attr_comment = false;
}

DomString::~DomString()
{
}

void DomString::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("notr")))
        setAttributeNotr(node.attribute(QLatin1String("notr")));
    if (node.hasAttribute(QLatin1String("comment")))
        setAttributeComment(node.attribute(QLatin1String("comment")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomString::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("string") : tagName.toLower());

    QDomElement child;

    if (hasAttributeNotr())
        e.setAttribute(QLatin1String("notr"), attributeNotr());

    if (hasAttributeComment())
        e.setAttribute(QLatin1String("comment"), attributeComment());

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomPointF::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_x = 0;
    m_y = 0;
}

DomPointF::DomPointF(): bits(0)
{
    m_x = 0;
    m_y = 0;
}

DomPointF::~DomPointF()
{
}

void DomPointF::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toDouble());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toDouble());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomPointF::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("pointf") : tagName.toLower());

    QDomElement child;

    if (bit.x) {
        child = doc.createElement(QLatin1String("x"));
        child.appendChild(doc.createTextNode(QString::number(m_x)));
        e.appendChild(child);
    }

    if (bit.y) {
        child = doc.createElement(QLatin1String("y"));
        child.appendChild(doc.createTextNode(QString::number(m_y)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomPointF::setElementX(double a)
{
    bit.x = true;
    m_x = a;
}

void DomPointF::setElementY(double a)
{
    bit.y = true;
    m_y = a;
}

void DomRectF::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
}

DomRectF::DomRectF(): bits(0)
{
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
}

DomRectF::~DomRectF()
{
}

void DomRectF::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toDouble());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toDouble());
            continue;
        }
        if (tag == QLatin1String("width")) {
            setElementWidth(e.text().toDouble());
            continue;
        }
        if (tag == QLatin1String("height")) {
            setElementHeight(e.text().toDouble());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomRectF::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("rectf") : tagName.toLower());

    QDomElement child;

    if (bit.x) {
        child = doc.createElement(QLatin1String("x"));
        child.appendChild(doc.createTextNode(QString::number(m_x)));
        e.appendChild(child);
    }

    if (bit.y) {
        child = doc.createElement(QLatin1String("y"));
        child.appendChild(doc.createTextNode(QString::number(m_y)));
        e.appendChild(child);
    }

    if (bit.width) {
        child = doc.createElement(QLatin1String("width"));
        child.appendChild(doc.createTextNode(QString::number(m_width)));
        e.appendChild(child);
    }

    if (bit.height) {
        child = doc.createElement(QLatin1String("height"));
        child.appendChild(doc.createTextNode(QString::number(m_height)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomRectF::setElementX(double a)
{
    bit.x = true;
    m_x = a;
}

void DomRectF::setElementY(double a)
{
    bit.y = true;
    m_y = a;
}

void DomRectF::setElementWidth(double a)
{
    bit.width = true;
    m_width = a;
}

void DomRectF::setElementHeight(double a)
{
    bit.height = true;
    m_height = a;
}

void DomSizeF::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_width = 0;
    m_height = 0;
}

DomSizeF::DomSizeF(): bits(0)
{
    m_width = 0;
    m_height = 0;
}

DomSizeF::~DomSizeF()
{
}

void DomSizeF::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("width")) {
            setElementWidth(e.text().toDouble());
            continue;
        }
        if (tag == QLatin1String("height")) {
            setElementHeight(e.text().toDouble());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomSizeF::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("sizef") : tagName.toLower());

    QDomElement child;

    if (bit.width) {
        child = doc.createElement(QLatin1String("width"));
        child.appendChild(doc.createTextNode(QString::number(m_width)));
        e.appendChild(child);
    }

    if (bit.height) {
        child = doc.createElement(QLatin1String("height"));
        child.appendChild(doc.createTextNode(QString::number(m_height)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomSizeF::setElementWidth(double a)
{
    bit.width = true;
    m_width = a;
}

void DomSizeF::setElementHeight(double a)
{
    bit.height = true;
    m_height = a;
}

void DomChar::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
    }

    m_unicode = 0;
}

DomChar::DomChar(): bits(0)
{
    m_unicode = 0;
}

DomChar::~DomChar()
{
}

void DomChar::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("unicode")) {
            setElementUnicode(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomChar::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("char") : tagName.toLower());

    QDomElement child;

    if (bit.unicode) {
        child = doc.createElement(QLatin1String("unicode"));
        child.appendChild(doc.createTextNode(QString::number(m_unicode)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomChar::setElementUnicode(int a)
{
    bit.unicode = true;
    m_unicode = a;
}

void DomUrl::clear(bool clear_all)
{
    delete m_string;

    if (clear_all) {
        m_text = QString();
    }

    m_string = 0;
}

DomUrl::DomUrl(): bits(0)
{
    m_string = 0;
}

DomUrl::~DomUrl()
{
    delete m_string;
}

void DomUrl::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("string")) {
            DomString *v = new DomString();
            v->read(e);
            setElementString(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomUrl::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("url") : tagName.toLower());

    QDomElement child;

    if (m_string != 0)
        e.appendChild(m_string->write(doc, QLatin1String("string")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomUrl::setElementString(DomString* a)
{
    bit.string = true;
    delete m_string;
    m_string = a;
}

void DomProperty::clear(bool clear_all)
{
    delete m_color;
    delete m_font;
    delete m_iconSet;
    delete m_pixmap;
    delete m_palette;
    delete m_point;
    delete m_rect;
    delete m_sizePolicy;
    delete m_size;
    delete m_string;
    delete m_stringList;
    delete m_date;
    delete m_time;
    delete m_dateTime;
    delete m_pointF;
    delete m_rectF;
    delete m_sizeF;
    delete m_char;
    delete m_url;

    if (clear_all) {
        m_text = QString();
        m_has_attr_name = false;
        m_has_attr_stdset = false;
        m_attr_stdset = 0;
    }

    m_kind = Unknown;

    m_color = 0;
    m_cursor = 0;
    m_font = 0;
    m_iconSet = 0;
    m_pixmap = 0;
    m_palette = 0;
    m_point = 0;
    m_rect = 0;
    m_sizePolicy = 0;
    m_size = 0;
    m_string = 0;
    m_stringList = 0;
    m_number = 0;
    m_float = 0.0;
    m_double = 0;
    m_date = 0;
    m_time = 0;
    m_dateTime = 0;
    m_pointF = 0;
    m_rectF = 0;
    m_sizeF = 0;
    m_longLong = 0;
    m_char = 0;
    m_url = 0;
}

DomProperty::DomProperty()
{
    m_kind = Unknown;

    m_has_attr_name = false;
    m_has_attr_stdset = false;
    m_attr_stdset = 0;
    m_color = 0;
    m_cursor = 0;
    m_font = 0;
    m_iconSet = 0;
    m_pixmap = 0;
    m_palette = 0;
    m_point = 0;
    m_rect = 0;
    m_sizePolicy = 0;
    m_size = 0;
    m_string = 0;
    m_stringList = 0;
    m_number = 0;
    m_float = 0.0;
    m_double = 0;
    m_date = 0;
    m_time = 0;
    m_dateTime = 0;
    m_pointF = 0;
    m_rectF = 0;
    m_sizeF = 0;
    m_longLong = 0;
    m_char = 0;
    m_url = 0;
}

DomProperty::~DomProperty()
{
    delete m_color;
    delete m_font;
    delete m_iconSet;
    delete m_pixmap;
    delete m_palette;
    delete m_point;
    delete m_rect;
    delete m_sizePolicy;
    delete m_size;
    delete m_string;
    delete m_stringList;
    delete m_date;
    delete m_time;
    delete m_dateTime;
    delete m_pointF;
    delete m_rectF;
    delete m_sizeF;
    delete m_char;
    delete m_url;
}

void DomProperty::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("name")))
        setAttributeName(node.attribute(QLatin1String("name")));
    if (node.hasAttribute(QLatin1String("stdset")))
        setAttributeStdset(node.attribute(QLatin1String("stdset")).toInt());

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("bool")) {
            setElementBool(e.text());
            continue;
        }
        if (tag == QLatin1String("color")) {
            DomColor *v = new DomColor();
            v->read(e);
            setElementColor(v);
            continue;
        }
        if (tag == QLatin1String("cstring")) {
            setElementCstring(e.text());
            continue;
        }
        if (tag == QLatin1String("cursor")) {
            setElementCursor(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("enum")) {
            setElementEnum(e.text());
            continue;
        }
        if (tag == QLatin1String("font")) {
            DomFont *v = new DomFont();
            v->read(e);
            setElementFont(v);
            continue;
        }
        if (tag == QLatin1String("iconset")) {
            DomResourcePixmap *v = new DomResourcePixmap();
            v->read(e);
            setElementIconSet(v);
            continue;
        }
        if (tag == QLatin1String("pixmap")) {
            DomResourcePixmap *v = new DomResourcePixmap();
            v->read(e);
            setElementPixmap(v);
            continue;
        }
        if (tag == QLatin1String("palette")) {
            DomPalette *v = new DomPalette();
            v->read(e);
            setElementPalette(v);
            continue;
        }
        if (tag == QLatin1String("point")) {
            DomPoint *v = new DomPoint();
            v->read(e);
            setElementPoint(v);
            continue;
        }
        if (tag == QLatin1String("rect")) {
            DomRect *v = new DomRect();
            v->read(e);
            setElementRect(v);
            continue;
        }
        if (tag == QLatin1String("set")) {
            setElementSet(e.text());
            continue;
        }
        if (tag == QLatin1String("sizepolicy")) {
            DomSizePolicy *v = new DomSizePolicy();
            v->read(e);
            setElementSizePolicy(v);
            continue;
        }
        if (tag == QLatin1String("size")) {
            DomSize *v = new DomSize();
            v->read(e);
            setElementSize(v);
            continue;
        }
        if (tag == QLatin1String("string")) {
            DomString *v = new DomString();
            v->read(e);
            setElementString(v);
            continue;
        }
        if (tag == QLatin1String("stringlist")) {
            DomStringList *v = new DomStringList();
            v->read(e);
            setElementStringList(v);
            continue;
        }
        if (tag == QLatin1String("number")) {
            setElementNumber(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("float")) {
            setElementFloat(e.text().toFloat());
            continue;
        }
        if (tag == QLatin1String("double")) {
            setElementDouble(e.text().toDouble());
            continue;
        }
        if (tag == QLatin1String("date")) {
            DomDate *v = new DomDate();
            v->read(e);
            setElementDate(v);
            continue;
        }
        if (tag == QLatin1String("time")) {
            DomTime *v = new DomTime();
            v->read(e);
            setElementTime(v);
            continue;
        }
        if (tag == QLatin1String("datetime")) {
            DomDateTime *v = new DomDateTime();
            v->read(e);
            setElementDateTime(v);
            continue;
        }
        if (tag == QLatin1String("pointf")) {
            DomPointF *v = new DomPointF();
            v->read(e);
            setElementPointF(v);
            continue;
        }
        if (tag == QLatin1String("rectf")) {
            DomRectF *v = new DomRectF();
            v->read(e);
            setElementRectF(v);
            continue;
        }
        if (tag == QLatin1String("sizef")) {
            DomSizeF *v = new DomSizeF();
            v->read(e);
            setElementSizeF(v);
            continue;
        }
        if (tag == QLatin1String("longlong")) {
            setElementLongLong(e.text().toLongLong());
            continue;
        }
        if (tag == QLatin1String("char")) {
            DomChar *v = new DomChar();
            v->read(e);
            setElementChar(v);
            continue;
        }
        if (tag == QLatin1String("url")) {
            DomUrl *v = new DomUrl();
            v->read(e);
            setElementUrl(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomProperty::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("property") : tagName.toLower());

    QDomElement child;

    if (hasAttributeName())
        e.setAttribute(QLatin1String("name"), attributeName());

    if (hasAttributeStdset())
        e.setAttribute(QLatin1String("stdset"), attributeStdset());

    switch(kind()) {
    case Bool: {
        QDomElement child = doc.createElement(QLatin1String("bool"));
        QDomText text = doc.createTextNode(elementBool());
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Color: {
        DomColor* v = elementColor();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("color"));
            e.appendChild(child);
        }
        break;
    }
    case Cstring: {
        QDomElement child = doc.createElement(QLatin1String("cstring"));
        QDomText text = doc.createTextNode(elementCstring());
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Cursor: {
        QDomElement child = doc.createElement(QLatin1String("cursor"));
        QDomText text = doc.createTextNode(QString::number(elementCursor()));
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Enum: {
        QDomElement child = doc.createElement(QLatin1String("enum"));
        QDomText text = doc.createTextNode(elementEnum());
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Font: {
        DomFont* v = elementFont();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("font"));
            e.appendChild(child);
        }
        break;
    }
    case IconSet: {
        DomResourcePixmap* v = elementIconSet();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("iconset"));
            e.appendChild(child);
        }
        break;
    }
    case Pixmap: {
        DomResourcePixmap* v = elementPixmap();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("pixmap"));
            e.appendChild(child);
        }
        break;
    }
    case Palette: {
        DomPalette* v = elementPalette();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("palette"));
            e.appendChild(child);
        }
        break;
    }
    case Point: {
        DomPoint* v = elementPoint();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("point"));
            e.appendChild(child);
        }
        break;
    }
    case Rect: {
        DomRect* v = elementRect();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("rect"));
            e.appendChild(child);
        }
        break;
    }
    case Set: {
        QDomElement child = doc.createElement(QLatin1String("set"));
        QDomText text = doc.createTextNode(elementSet());
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case SizePolicy: {
        DomSizePolicy* v = elementSizePolicy();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("sizepolicy"));
            e.appendChild(child);
        }
        break;
    }
    case Size: {
        DomSize* v = elementSize();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("size"));
            e.appendChild(child);
        }
        break;
    }
    case String: {
        DomString* v = elementString();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("string"));
            e.appendChild(child);
        }
        break;
    }
    case StringList: {
        DomStringList* v = elementStringList();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("stringlist"));
            e.appendChild(child);
        }
        break;
    }
    case Number: {
        QDomElement child = doc.createElement(QLatin1String("number"));
        QDomText text = doc.createTextNode(QString::number(elementNumber()));
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Float: {
        QDomElement child = doc.createElement(QLatin1String("float"));
        QDomText text = doc.createTextNode(QString::number(elementFloat()));
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Double: {
        QDomElement child = doc.createElement(QLatin1String("double"));
        QDomText text = doc.createTextNode(QString::number(elementDouble()));
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Date: {
        DomDate* v = elementDate();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("date"));
            e.appendChild(child);
        }
        break;
    }
    case Time: {
        DomTime* v = elementTime();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("time"));
            e.appendChild(child);
        }
        break;
    }
    case DateTime: {
        DomDateTime* v = elementDateTime();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("datetime"));
            e.appendChild(child);
        }
        break;
    }
    case PointF: {
        DomPointF* v = elementPointF();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("pointf"));
            e.appendChild(child);
        }
        break;
    }
    case RectF: {
        DomRectF* v = elementRectF();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("rectf"));
            e.appendChild(child);
        }
        break;
    }
    case SizeF: {
        DomSizeF* v = elementSizeF();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("sizef"));
            e.appendChild(child);
        }
        break;
    }
    case LongLong: {
        QDomElement child = doc.createElement(QLatin1String("longLong"));
        QDomText text = doc.createTextNode(QString::number(elementLongLong()));
        child.appendChild(text);
        e.appendChild(child);
        break;
    }
    case Char: {
        DomChar* v = elementChar();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("char"));
            e.appendChild(child);
        }
        break;
    }
    case Url: {
        DomUrl* v = elementUrl();
        if (v != 0) {
            QDomElement child = v->write(doc, QLatin1String("url"));
            e.appendChild(child);
        }
        break;
    }
    default:
        break;
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomProperty::setElementBool(const QString& a)
{
    clear(false);
    m_kind = Bool;
    m_bool = a;
}

void DomProperty::setElementColor(DomColor* a)
{
    clear(false);
    m_kind = Color;
    m_color = a;
}

void DomProperty::setElementCstring(const QString& a)
{
    clear(false);
    m_kind = Cstring;
    m_cstring = a;
}

void DomProperty::setElementCursor(int a)
{
    clear(false);
    m_kind = Cursor;
    m_cursor = a;
}

void DomProperty::setElementEnum(const QString& a)
{
    clear(false);
    m_kind = Enum;
    m_enum = a;
}

void DomProperty::setElementFont(DomFont* a)
{
    clear(false);
    m_kind = Font;
    m_font = a;
}

void DomProperty::setElementIconSet(DomResourcePixmap* a)
{
    clear(false);
    m_kind = IconSet;
    m_iconSet = a;
}

void DomProperty::setElementPixmap(DomResourcePixmap* a)
{
    clear(false);
    m_kind = Pixmap;
    m_pixmap = a;
}

void DomProperty::setElementPalette(DomPalette* a)
{
    clear(false);
    m_kind = Palette;
    m_palette = a;
}

void DomProperty::setElementPoint(DomPoint* a)
{
    clear(false);
    m_kind = Point;
    m_point = a;
}

void DomProperty::setElementRect(DomRect* a)
{
    clear(false);
    m_kind = Rect;
    m_rect = a;
}

void DomProperty::setElementSet(const QString& a)
{
    clear(false);
    m_kind = Set;
    m_set = a;
}

void DomProperty::setElementSizePolicy(DomSizePolicy* a)
{
    clear(false);
    m_kind = SizePolicy;
    m_sizePolicy = a;
}

void DomProperty::setElementSize(DomSize* a)
{
    clear(false);
    m_kind = Size;
    m_size = a;
}

void DomProperty::setElementString(DomString* a)
{
    clear(false);
    m_kind = String;
    m_string = a;
}

void DomProperty::setElementStringList(DomStringList* a)
{
    clear(false);
    m_kind = StringList;
    m_stringList = a;
}

void DomProperty::setElementNumber(int a)
{
    clear(false);
    m_kind = Number;
    m_number = a;
}

void DomProperty::setElementFloat(float a)
{
    clear(false);
    m_kind = Float;
    m_float = a;
}

void DomProperty::setElementDouble(double a)
{
    clear(false);
    m_kind = Double;
    m_double = a;
}

void DomProperty::setElementDate(DomDate* a)
{
    clear(false);
    m_kind = Date;
    m_date = a;
}

void DomProperty::setElementTime(DomTime* a)
{
    clear(false);
    m_kind = Time;
    m_time = a;
}

void DomProperty::setElementDateTime(DomDateTime* a)
{
    clear(false);
    m_kind = DateTime;
    m_dateTime = a;
}

void DomProperty::setElementPointF(DomPointF* a)
{
    clear(false);
    m_kind = PointF;
    m_pointF = a;
}

void DomProperty::setElementRectF(DomRectF* a)
{
    clear(false);
    m_kind = RectF;
    m_rectF = a;
}

void DomProperty::setElementSizeF(DomSizeF* a)
{
    clear(false);
    m_kind = SizeF;
    m_sizeF = a;
}

void DomProperty::setElementLongLong(qlonglong a)
{
    clear(false);
    m_kind = LongLong;
    m_longLong = a;
}

void DomProperty::setElementChar(DomChar* a)
{
    clear(false);
    m_kind = Char;
    m_char = a;
}

void DomProperty::setElementUrl(DomUrl* a)
{
    clear(false);
    m_kind = Url;
    m_url = a;
}

void DomConnections::clear(bool clear_all)
{
    for (int i = 0; i < m_connection.size(); ++i)
        delete m_connection[i];
    m_connection.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomConnections::DomConnections(): bits(0)
{
}

DomConnections::~DomConnections()
{
    for (int i = 0; i < m_connection.size(); ++i)
        delete m_connection[i];
    m_connection.clear();
}

void DomConnections::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("connection")) {
            DomConnection *v = new DomConnection();
            v->read(e);
            m_connection.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomConnections::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("connections") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_connection.size(); ++i) {
        DomConnection* v = m_connection[i];
        QDomNode child = v->write(doc, QLatin1String("connection"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomConnections::setElementConnection(const QList<DomConnection*>& a)
{
    bit.connection = true;
    m_connection = a;
}

void DomConnection::clear(bool clear_all)
{
    delete m_hints;

    if (clear_all) {
        m_text = QString();
    }

    m_hints = 0;
}

DomConnection::DomConnection(): bits(0)
{
    m_hints = 0;
}

DomConnection::~DomConnection()
{
    delete m_hints;
}

void DomConnection::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("sender")) {
            setElementSender(e.text());
            continue;
        }
        if (tag == QLatin1String("signal")) {
            setElementSignal(e.text());
            continue;
        }
        if (tag == QLatin1String("receiver")) {
            setElementReceiver(e.text());
            continue;
        }
        if (tag == QLatin1String("slot")) {
            setElementSlot(e.text());
            continue;
        }
        if (tag == QLatin1String("hints")) {
            DomConnectionHints *v = new DomConnectionHints();
            v->read(e);
            setElementHints(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomConnection::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("connection") : tagName.toLower());

    QDomElement child;

    if (bit.sender) {
        child = doc.createElement(QLatin1String("sender"));
        child.appendChild(doc.createTextNode(m_sender));
        e.appendChild(child);
    }

    if (bit.signal) {
        child = doc.createElement(QLatin1String("signal"));
        child.appendChild(doc.createTextNode(m_signal));
        e.appendChild(child);
    }

    if (bit.receiver) {
        child = doc.createElement(QLatin1String("receiver"));
        child.appendChild(doc.createTextNode(m_receiver));
        e.appendChild(child);
    }

    if (bit.slot) {
        child = doc.createElement(QLatin1String("slot"));
        child.appendChild(doc.createTextNode(m_slot));
        e.appendChild(child);
    }

    if (m_hints != 0)
        e.appendChild(m_hints->write(doc, QLatin1String("hints")));

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomConnection::setElementSender(const QString& a)
{
    bit.sender = true;
    m_sender = a;
}

void DomConnection::setElementSignal(const QString& a)
{
    bit.signal = true;
    m_signal = a;
}

void DomConnection::setElementReceiver(const QString& a)
{
    bit.receiver = true;
    m_receiver = a;
}

void DomConnection::setElementSlot(const QString& a)
{
    bit.slot = true;
    m_slot = a;
}

void DomConnection::setElementHints(DomConnectionHints* a)
{
    bit.hints = true;
    delete m_hints;
    m_hints = a;
}

void DomConnectionHints::clear(bool clear_all)
{
    for (int i = 0; i < m_hint.size(); ++i)
        delete m_hint[i];
    m_hint.clear();

    if (clear_all) {
        m_text = QString();
    }

}

DomConnectionHints::DomConnectionHints(): bits(0)
{
}

DomConnectionHints::~DomConnectionHints()
{
    for (int i = 0; i < m_hint.size(); ++i)
        delete m_hint[i];
    m_hint.clear();
}

void DomConnectionHints::read(const QDomElement &node)
{

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("hint")) {
            DomConnectionHint *v = new DomConnectionHint();
            v->read(e);
            m_hint.append(v);
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomConnectionHints::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("connectionhints") : tagName.toLower());

    QDomElement child;

    for (int i = 0; i < m_hint.size(); ++i) {
        DomConnectionHint* v = m_hint[i];
        QDomNode child = v->write(doc, QLatin1String("hint"));
        e.appendChild(child);
    }
    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomConnectionHints::setElementHint(const QList<DomConnectionHint*>& a)
{
    bit.hint = true;
    m_hint = a;
}

void DomConnectionHint::clear(bool clear_all)
{

    if (clear_all) {
        m_text = QString();
        m_has_attr_type = false;
    }

    m_x = 0;
    m_y = 0;
}

DomConnectionHint::DomConnectionHint(): bits(0)
{
    m_has_attr_type = false;
    m_x = 0;
    m_y = 0;
}

DomConnectionHint::~DomConnectionHint()
{
}

void DomConnectionHint::read(const QDomElement &node)
{
    if (node.hasAttribute(QLatin1String("type")))
        setAttributeType(node.attribute(QLatin1String("type")));

    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        QDomElement e = n.toElement();
        QString tag = e.tagName().toLower();
        if (tag == QLatin1String("x")) {
            setElementX(e.text().toInt());
            continue;
        }
        if (tag == QLatin1String("y")) {
            setElementY(e.text().toInt());
            continue;
        }
    }

    m_text.clear();
    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isText())
            m_text.append(child.nodeValue());
    }
}

QDomElement DomConnectionHint::write(QDomDocument &doc, const QString &tagName)
{
    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("connectionhint") : tagName.toLower());

    QDomElement child;

    if (hasAttributeType())
        e.setAttribute(QLatin1String("type"), attributeType());

    if (bit.x) {
        child = doc.createElement(QLatin1String("x"));
        child.appendChild(doc.createTextNode(QString::number(m_x)));
        e.appendChild(child);
    }

    if (bit.y) {
        child = doc.createElement(QLatin1String("y"));
        child.appendChild(doc.createTextNode(QString::number(m_y)));
        e.appendChild(child);
    }

    if (!m_text.isEmpty())
        e.appendChild(doc.createTextNode(m_text));

    return e;
}

void DomConnectionHint::setElementX(int a)
{
    bit.x = true;
    m_x = a;
}

void DomConnectionHint::setElementY(int a)
{
    bit.y = true;
    m_y = a;
}

