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

#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <qwidget.h>

class QTreeWidget;
class QTreeWidgetItem;
class QSqlDatabase;

class ConnectionWidget: public QWidget
{
    Q_OBJECT
public:
    ConnectionWidget(QWidget *parent = 0);
    virtual ~ConnectionWidget();

    QSqlDatabase currentDatabase() const;

public slots:
    void refresh();
    void on_tree_doubleClicked(QTreeWidgetItem *item, int column, Qt::ButtonState button);

private:
    void setActive(QTreeWidgetItem *);

    QTreeWidget *tree;
    QString activeDb;
};

#endif

