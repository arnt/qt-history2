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

#ifndef QHEADERWIDGET_H
#define QHEADERWIDGET_H

#ifndef QT_H
#include <qheaderview.h>
#include <qwidgetbaseitem.h>
#endif

class QHeaderWidget;

class Q_GUI_EXPORT QHeaderWidgetItem : public QWidgetCellItem
{
public:
    QHeaderWidgetItem(QHeaderWidget *view);
    ~QHeaderWidgetItem();
protected:
    QHeaderWidget *view;
};

class QHeaderWidgetPrivate;

class Q_GUI_EXPORT QHeaderWidget : public QHeaderView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHeaderWidget)
    friend class QHeaderWidgetItem;

public:
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QHeaderWidget(QWidget *parent, const char *name);
#endif
    QHeaderWidget(Qt::Orientation orientation, QWidget *parent = 0);
    ~QHeaderWidget();

    void setSectionCount(int sections);

    QHeaderWidgetItem *item(int section) const;
    void setItem(int section, QHeaderWidgetItem *item);
    QHeaderWidgetItem *takeItem(int section);

protected:
    void removeItem(QHeaderWidgetItem *item);
    void setModel(QAbstractItemModel *model);
};

#endif
