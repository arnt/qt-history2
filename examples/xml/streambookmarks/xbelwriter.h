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

#ifndef XBELWRITER_H
#define XBELWRITER_H

#include <QXmlStreamWriter>

class QTreeWidget;
class QTreeWidgetItem;

class XbelWriter : public QXmlStreamWriter
{
public:
    XbelWriter(QTreeWidget *treeWidget);
    bool writeFile(QIODevice *device);

private:
    void writeItem(QTreeWidgetItem *item);
    QTreeWidget *treeWidget;
};

#endif
