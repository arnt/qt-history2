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
    friend class QTableWidget;
public:
    QTableWidgetItem(QTableWidget *view = 0);
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
    int rowCount() const;
    void setColumnCount(int columns);
    int columnCount() const;

    void insertRow(int row);
    void insertColumn(int column);

    int row(const QTableWidgetItem *item) const;
    int column(const QTableWidgetItem *item) const;

    QTableWidgetItem *item(int row, int column) const;
    void setItem(int row, int column, QTableWidgetItem *item);
    QTableWidgetItem *takeItem(int row, int column);

    QTableWidgetItem *verticalHeaderItem(int row) const;
    void setVerticalHeaderItem(int row, QTableWidgetItem *item);

    QTableWidgetItem *horizontalHeaderItem(int column) const;
    void setHorizontalHeaderItem(int column, QTableWidgetItem *item);

    void setVerticalHeaderLabels(const QStringList &labels);
    void setHorizontalHeaderLabels(const QStringList &labels);

protected:
    void removeItem(QTableWidgetItem *item);
    void setModel(QAbstractItemModel *model);
};

#endif
