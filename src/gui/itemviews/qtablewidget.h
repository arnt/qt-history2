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

#ifndef QTABLEWIDGET_H
#define QTABLEWIDGET_H

#ifndef QT_H
#include <qtableview.h>
#include <qwidgetbaseitem.h>
#endif

class QTableWidget;

class Q_GUI_EXPORT QTableWidgetItem : public QWidgetCellItem
{
public:
    QTableWidgetItem(QTableWidget *view);
    ~QTableWidgetItem();
protected:
    QTableWidget *view;
};

class QTableWidgetPrivate;

class Q_GUI_EXPORT QTableWidget : public QTableView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTableWidget)
    friend class QTableWidgetItem;

public:
    QTableWidget(QWidget *parent = 0);
    ~QTableWidget();

    void setRowCount(int rows);
    void setColumnCount(int columns);

    QTableWidgetItem *item(int row, int column) const;
    void setItem(int row, int column, QTableWidgetItem *item);
    QTableWidgetItem *takeItem(int row, int column);

    QTableWidgetItem *verticalHeaderItem(int row) const;
    void setVerticalHeaderItem(int row, QTableWidgetItem *item);

    QTableWidgetItem *horizontalHeaderItem(int column) const;
    void setHorizontalHeaderItem(int column, QTableWidgetItem *item);

protected:
    void removeItem(QTableWidgetItem *item);
};

#endif
