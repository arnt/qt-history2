/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef XBELHANDLER_H
#define XBELHANDLER_H

#include <QIcon>
#include <QXmlDefaultHandler>

class QTreeWidget;
class QTreeWidgetItem;

class XbelHandler : public QXmlDefaultHandler
{
public:
    XbelHandler(QTreeWidget *treeWidget);

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;

private:
    QTreeWidgetItem *createChildItem(const QString &tagName);

    QTreeWidget *treeWidget;
    QTreeWidgetItem *item;
    QString currentText;
    QString errorStr;
    bool metXbelTag;

    QIcon folderIcon;
    QIcon bookmarkIcon;
};

#endif
