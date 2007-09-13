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

#include "resourcemimedata_p.h"

#include <QtCore/QStringList>
#include <QtCore/QMimeData>
#include <QtXml/QDomDocument>

QT_BEGIN_NAMESPACE

static const char *elementResourceData = "resource";
static const char *typeAttribute = "type";
static const char *typeImage = "image";
static const char *typeFile = "file";
static const char *qrcAttribute = "qrc";
static const char *fileAttribute = "file";

static bool readResourceMimeData(const QMimeData *md,
                                 qdesigner_internal::ResourceMimeData::Type *t = 0,
                                 QString *qrc = 0, QString *file = 0)
{
    if (!md->hasText())
        return false;

    const QString docElementName = QLatin1String(elementResourceData);
    static const QString docElementString = QLatin1Char('<') + docElementName;

    const QString text = md->text();
    if (text.isEmpty() || text.indexOf(docElementString) == -1)
        return false;

    QDomDocument doc;
    if (!doc.setContent(text))
        return false;

    const QDomElement domElement = doc.documentElement();
    if (domElement.tagName() != docElementName)
        return false;

    if (t) {
        const QString typeAttr = QLatin1String(typeAttribute);
        if (domElement.hasAttribute (typeAttr)) {
            const QString fileTypeValue = QLatin1String(typeFile);
            *t = domElement.attribute(typeAttr, fileTypeValue) == fileTypeValue ?
            qdesigner_internal::ResourceMimeData::File : qdesigner_internal::ResourceMimeData::Image;
        } else {
            *t = qdesigner_internal::ResourceMimeData::File;
        }
    }

    if (qrc) {
        const QString qrcAttr = QLatin1String(qrcAttribute);
        if (domElement.hasAttribute(qrcAttr)) {
            *qrc = domElement.attribute(qrcAttr, QString());
        } else {
            qrc->clear();
        }
    }

    if (file) {
        const QString fileAttr = QLatin1String(fileAttribute);
        if (domElement.hasAttribute(fileAttr)) {
            *file = domElement.attribute(fileAttr, QString());
        } else {
            file->clear();
        }
    }

    return true;
}

namespace qdesigner_internal {

ResourceMimeData::ResourceMimeData(Type t) :
    m_type(t)
{
}

QMimeData *ResourceMimeData::toMimeData() const
{
    QDomDocument doc;
    QDomElement elem = doc.createElement(QLatin1String(elementResourceData));
    elem.setAttribute(QLatin1String(typeAttribute), QLatin1String(m_type == Image ? typeImage : typeFile));
    if (!m_qrcPath.isEmpty())
        elem.setAttribute(QLatin1String(qrcAttribute), m_qrcPath);
    if (!m_filePath.isEmpty())
        elem.setAttribute(QLatin1String(fileAttribute), m_filePath);

    doc.appendChild(elem);

    QMimeData *rc = new QMimeData;
    rc->setText(doc.toString());
    return rc;
}

bool ResourceMimeData::isResourceMimeData(const QMimeData *md)
{
    return readResourceMimeData(md);
}

bool  ResourceMimeData::isResourceMimeData(const QMimeData *md, Type desiredType)
{
    Type t;
    return readResourceMimeData(md, &t) && t == desiredType;
}

bool  ResourceMimeData::fromMimeData(const QMimeData *md)
{
    Type type;
    QString file;
    QString qrc;
    if (!readResourceMimeData(md, &type, &qrc, &file))
        return false;
    m_type = type;
    m_qrcPath = qrc;
    m_filePath = file;
    return true;
}

}

QT_END_NAMESPACE
