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

#include "qtsimplexml.h"
#include <QDomDocument>

QtSimpleXml::QtSimpleXml(const QString &name)
{
    valid = false;
    n = name;
    parent = 0;
}

QtSimpleXml &QtSimpleXml::operator [](int index)
{
    if (index < 0)
	return *this;

    if (index > children.size()) {
        static QtSimpleXml NIL;
        qWarning("QtSimpleXml::operator[], Out of range access: size is %i, index is %i",
                 children.size(), index);
        return NIL;
    }

    if (index == children.size()) {
        QtSimpleXml *item = new QtSimpleXml("item");
        item->parent = this;
        children.insert(item->name(), item);
        return *item;
    }

    QMultiMap<QString, QtSimpleXml *>::Iterator it = children.begin();
    while (index--) ++it;
    return *it.value();
}

QtSimpleXml &QtSimpleXml::operator [](const QString &key)
{
    if (!children.contains(key)) {
	QtSimpleXml *item = new QtSimpleXml(key);
	item->parent = this;
	children.insert(item->name(), item);
        return *item;
    }

    return *children.find(key).value();
}

QtSimpleXml &QtSimpleXml::operator =(const QString &text)
{
    valid = true;
    QtSimpleXml *p = parent;
    while (p && !p->valid) {
	p->valid = true;
	p = p->parent;
    }

    s = text;
    return *this;
}

QDomDocument QtSimpleXml::toDomDocument() const
{
    QDomDocument doc;
    QString data = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", data));

    if (!valid)
	return doc;

    if(!s.isEmpty())
        doc.appendChild(doc.createTextNode(s));

    {
        QMultiMap<QString, QtSimpleXml *>::ConstIterator it = children.constBegin();
        for (; it != children.end(); ++it) {
            QtSimpleXml *item = it.value();
            if (item->valid) {
                QDomNode node = item->toDomElement(&doc);
                doc.appendChild(node);
            }
        }
    }

    return doc;
}

QDomElement QtSimpleXml::toDomElement(QDomDocument *doc) const
{
    QDomElement elem = doc->createElement(n);
    QMap<QString, QString>::ConstIterator ita = attr.constBegin();
    for (; ita != attr.constEnd(); ++ita)
	elem.setAttribute(ita.key(), ita.value());

    if(!s.isEmpty())
        elem.appendChild(doc->createTextNode(s));

    {
        QMultiMap<QString, QtSimpleXml *>::ConstIterator it = children.constBegin();
        for (; it != children.constEnd(); ++it) {
            QtSimpleXml *item = it.value();
            if (item->valid) {
                QDomNode node = item->toDomElement(doc);
                elem.appendChild(node);
            }
        }
    }

    return elem;
}

QString QtSimpleXml::name() const
{
    return n;
}

QString QtSimpleXml::text() const
{
    return s;
}

int QtSimpleXml::numChildren() const
{
    return children.count();
}

bool QtSimpleXml::isValid() const
{
    return valid;
}

void QtSimpleXml::setAttribute(const QString &key, const QString &value)
{
    attr.insert(key, QString(value));
}

QString QtSimpleXml::attribute(const QString &key)
{
    return attr[key];
}

bool QtSimpleXml::setContent(const QString &content)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;

    if (!doc.setContent(content, false, &errorMsg, &errorLine, &errorColumn)) {
        errorStr = errorMsg;
        errorStr += " at " + QString::number(errorLine) + ":" + QString::number(errorColumn);
        return false;
    }

    parse(doc);
    return true;
}

bool QtSimpleXml::setContent(QIODevice *device)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(device, false, &errorMsg, &errorLine, &errorColumn)) {
        errorStr = errorMsg;
        errorStr += " at " + QString::number(errorLine) + ":" + QString::number(errorColumn);
        return false;
    }

    QDomNode child = doc.firstChild();
    while (!child.isNull() && !child.isElement())
        child = child.nextSibling();

    while (!child.isNull()) {
        QtSimpleXml *xmlNode = new QtSimpleXml;
        xmlNode->parse(child);
        xmlNode->parent=this;
        children.insert(xmlNode->name(), xmlNode);
        do {
            child = child.nextSibling();
        } while (!child.isNull() && !child.isElement());
    }

    return true;
}


void QtSimpleXml::parse(QDomNode node)
{
 //   puts("parse");
    if (node.isNull())
        return;

    valid = true;
    n = node.nodeName();
    QDomElement element = node.toElement();

    QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < (int) attrs.count(); ++i) {
        QDomAttr attribute = attrs.item(i).toAttr();
        attr.insert(attribute.name(), attribute.value());
    }

    if (element.firstChild().isText()) {
  //      printf("Got text %s\n", element.text().stripWhiteSpace().latin1());
        s = element.text().trimmed();
        return;
    }

    if (node.hasChildNodes()) {

        // Skip to first element child
        QDomNode child = node.firstChild();
        while (!child.isNull() && !child.isElement())
            child = child.nextSibling();

        while (!child.isNull()) {
            QtSimpleXml *xmlNode = new QtSimpleXml;
            xmlNode->parse(child);
            children.insert(xmlNode->name(), xmlNode);

            node = node.nextSibling();

            do {
                child = child.nextSibling();
            } while (!child.isNull() && !child.isElement());
        }
    }
}

QString QtSimpleXml::errorString() const
{
    return errorStr;
}
