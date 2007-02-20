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

#ifndef XBELREADER_H
#define XBELREADER_H

#include <QIcon>
#include "../../qxmlstream.h"

class QTreeWidget;
class QTreeWidgetItem;

class XbelReader : public QXmlStreamReader
{
public:
    XbelReader(QTreeWidget *treeWidget);

    bool read(QIODevice *device);

private:
    void readUnknownElement();
    void readXBEL();
    void readTitle(QTreeWidgetItem *item);
    void readSeparator(QTreeWidgetItem *item);
    void readFolder(QTreeWidgetItem *item);
    void readBookmark(QTreeWidgetItem *item);

    QTreeWidgetItem *createChildItem(QTreeWidgetItem *item);

    QTreeWidget *treeWidget;

    QIcon folderIcon;
    QIcon bookmarkIcon;
};

#endif
