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
#include <qabstractitemmodel.h>
#include <qvector.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class Q_GUI_EXPORT QTableWidgetItem
{

public:
    QTableWidgetItem() : edit(true), select(true) {}
    ~QTableWidgetItem() {}

    inline QString text() const { return data(QAbstractItemModel::DisplayRole).toString(); }
    inline QIconSet icon() const { return data(QAbstractItemModel::DecorationRole).toIcon(); }
    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { setData(QAbstractItemModel::DisplayRole, text); }
    inline void setIcon(const QIconSet &icon) { setData(QAbstractItemModel::DisplayRole, icon); }
    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    bool operator ==(const QTableWidgetItem &other) const;
    inline bool operator !=(const QTableWidgetItem &other) const { return !operator==(other); }

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

private:
    struct Data {
        Data() {}
        Data(int r, QVariant v) {
            role = r;
            value = v;
        }
        int role;
        QVariant value;
    };

    QVector<Data> values;
    uint edit : 1;
    uint select : 1;
};

class QTableWidgetPrivate;

class Q_GUI_EXPORT QTableWidget : public QTableView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTableWidget)

public:
    QTableWidget(QWidget *parent = 0);
    ~QTableWidget();

    void setRowCount(int rows);
    void setColumnCount(int columns);

    QTableWidgetItem item(int row, int column) const;
    void setItem(int row, int column, const QTableWidgetItem &item);

    void setText(int row, int column, const QString &text);
    void setIcon(int row, int column, const QIconSet &icon);
    QString text(int row, int column) const;
    QIconSet icon(int row, int column) const;

    void setRowText(int row, const QString &text);
    void setRowIconSet(int row, const QIconSet &icon);
    QString rowText(int row) const;
    QIconSet rowIconSet(int row) const;

    void setColumnText(int column, const QString &text);
    void setColumnIconSet(int column, const QIconSet &icon);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;
};

#endif
