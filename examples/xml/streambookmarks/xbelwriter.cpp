/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "xbelwriter.h"

XbelWriter::XbelWriter(QTreeWidget *treeWidget)
    : treeWidget(treeWidget)
{
    setAutoFormatting(true);
}

bool XbelWriter::writeFile(QIODevice *device)
{
    setDevice(device);

    writeStartDocument();
    writeDTD("<!DOCTYPE xbel>");
    writeStartElement("xbel");
    writeAttribute("version", "1.0");
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        writeItem(treeWidget->topLevelItem(i));

    writeEndDocument();
    return true;
}

void XbelWriter::writeItem(QTreeWidgetItem *item)
{
    QString tagName = item->data(0, Qt::UserRole).toString();
    if (tagName == "folder") {
        bool folded = !treeWidget->isItemExpanded(item);
        writeStartElement(tagName);
        writeAttribute("folded", folded ? "yes" : "no");
        writeTextElement("title", item->text(0));
        for (int i = 0; i < item->childCount(); ++i)
            writeItem(item->child(i));
        writeEndElement();
    } else if (tagName == "bookmark") {
        writeStartElement(tagName);
        if (!item->text(1).isEmpty())
            writeAttribute("href", item->text(1));
        writeTextElement("title", item->text(0));
        writeEndElement();
    } else if (tagName == "separator") {
        writeEmptyElement(tagName);
    }
}
