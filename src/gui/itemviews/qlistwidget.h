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

typedef QWidgetCellItem QListWidgetItem;

class QListWidgetPrivate;

class Q_GUI_EXPORT QListWidget : public QListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListWidget)

public:
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QListWidget(QWidget *parent, const char* name);
#endif
    QListWidget(QWidget *parent = 0);
    ~QListWidget();

    QListWidgetItem *itemAt(int row) const;
    void insertItem(int row, QListWidgetItem *item);
    void appendItem(QListWidgetItem *item);
    void removeItem(int row);
};

#endif
