#include "qtsimplexml.h"

#include <q3ptrlist.h>
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

    if (index >= (int) siblArray.count()) {
        int orgCount= (int) siblArray.count();
        for (int i = index; i >= orgCount; --i) {
            QtSimpleXml *item = new QtSimpleXml("item");
	    item->parent = this;
	    siblArray.append(item);
	}
    }

    return *siblArray.at((int) index);
}

QtSimpleXml &QtSimpleXml::operator [](const QString &key)
{
    if (!siblStruct.find(key)) {
	QtSimpleXml *item = new QtSimpleXml(key);
	item->parent = this;
	siblStruct.insert(key, item);
    }

    return *siblStruct.find(key);
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
        Q3DictIterator<QtSimpleXml> it(siblStruct);
        while (it.current()) {
            QtSimpleXml *item = it.current();
            if (item->valid) {
                QDomNode node = it.current()->toDomElement(&doc);
                doc.appendChild(node);
            }
            ++it;
        }
    }

    {
        Q3PtrListIterator<QtSimpleXml> it(siblArray);
        while (it.current()) {
            QtSimpleXml *item = it.current();
            if (item->valid) {
                QDomElement node = item->toDomElement(&doc);
                doc.appendChild(node);
            }
            ++it;
        }
    }
    return doc;
}

QDomElement QtSimpleXml::toDomElement(QDomDocument *doc) const
{
    QDomElement elem = doc->createElement(n);
    Q3DictIterator<QString> ita(attr);
    while (ita.current()) {
	elem.setAttribute(ita.currentKey(), *ita.current());
	++ita;
    }

    if(!s.isEmpty())
        elem.appendChild(doc->createTextNode(s));

    {
        Q3DictIterator<QtSimpleXml> it(siblStruct);
        while (it.current()) {
		QtSimpleXml *item = it.current();
		if (item->valid) {
		    QDomNode node = it.current()->toDomElement(doc);
		    elem.appendChild(node);
		}
		++it;
	    }
    }

    {
        Q3PtrListIterator<QtSimpleXml> it(siblArray);
        while (it.current()) {
            QtSimpleXml *item = it.current();
            if (item->valid) {
                QDomNode node = it.current()->toDomElement(doc);
                elem.appendChild(node);
                }
                ++it;
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

bool QtSimpleXml::isValid() const
{
    return valid;
}

void QtSimpleXml::setAttribute(const QString &key, const QString &value)
{
    attr.insert(key, new QString(value));
}

QString QtSimpleXml::attribute(const QString &key)
{
    QString *str = attr[key];
    return str ? *str : QString();
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
        siblStruct.insert(xmlNode->name(), xmlNode);
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
        attr.insert(attribute.name(), new QString(attribute.value()));
    }

    if (element.firstChild().isText()) {
  //      printf("Got text %s\n", element.text().stripWhiteSpace().latin1());
        s = element.text().stripWhiteSpace();
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
            if(xmlNode->name()=="item")
                siblArray.append(xmlNode);
            else
                siblStruct.insert(xmlNode->name(), xmlNode);


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
