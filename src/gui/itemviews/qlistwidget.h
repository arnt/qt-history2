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

#ifndef QLISTWIDGET_H
#define QLISTWIDGET_H

#ifndef QT_H
#include <qlistview.h>
#include <qwidgetbaseitem.h>
#endif

class QListWidget;

class QListWidgetItem : public QWidgetCellItem
{
public:
    QListWidgetItem(QListWidget *view);
    ~QListWidgetItem();
protected:
    QListWidget *view;
};

class QListWidgetPrivate;

class Q_GUI_EXPORT QListWidget : public QListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListWidget)

    friend class QListWidgetItem;
public:
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QListWidget(QWidget *parent, const char* name);
#endif
    QListWidget(QWidget *parent = 0);
    ~QListWidget();

    QListWidgetItem *item(int row) const;
    void insertItem(int row, QListWidgetItem *item);
    void appendItem(QListWidgetItem *item);
    QListWidgetItem *takeItem(int row);

protected:
    void removeItem(QListWidgetItem *item);
};

#endif
