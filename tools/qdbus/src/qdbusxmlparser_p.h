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

#ifndef QDBUSXMLPARSER_H
#define QDBUSXMLPARSER_H

#include <QtCore/qmap.h>
#include <QtXml/qdom.h>
#include <QtDBus/qdbusmacros.h>
#include <QtDBus/qdbusintrospection_p.h>

/*!
    \internal
*/
class QDBusXmlParser
{
    QString m_service;
    QString m_path;
    QDomElement m_node;

public:
    QDBusXmlParser(const QString& service, const QString& path,
                   const QString& xmlData);
    QDBusXmlParser(const QString& service, const QString& path,
                   const QDomElement& node);

    QDBusIntrospection::Interfaces interfaces() const;
    QSharedDataPointer<QDBusIntrospection::Object> object() const;
    QSharedDataPointer<QDBusIntrospection::ObjectTree> objectTree() const;
};

#endif
