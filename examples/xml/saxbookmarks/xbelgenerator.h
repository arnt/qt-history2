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

#ifndef XBELGENERATOR_H
#define XBELGENERATOR_H

#include <QTextStream>

QT_DECLARE_CLASS(QTreeWidget)
QT_DECLARE_CLASS(QTreeWidgetItem)

class XbelGenerator
{
public:
    XbelGenerator(QTreeWidget *treeWidget);

    bool write(QIODevice *device);

private:
    static QString indent(int indentLevel);
    static QString escapedText(const QString &str);
    static QString escapedAttribute(const QString &str);
    void generateItem(QTreeWidgetItem *item, int depth);

    QTreeWidget *treeWidget;
    QTextStream out;
};

#endif
