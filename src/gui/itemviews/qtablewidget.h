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

#include <qtableview.h>

class QTableWidget;

class Q_GUI_EXPORT QTableWidgetItem
{
    friend class QTableModel;
    friend class QTableWidget;
public:
    QTableWidgetItem();
    virtual ~QTableWidgetItem();

    inline QAbstractItemModel::ItemFlags flags() const { return itemFlags; }
    inline void setFlags(QAbstractItemModel::ItemFlags flags) { itemFlags = flags; }

    inline QString text() const
        { return data(QAbstractItemModel::DisplayRole).toString(); }
    inline void setText(const QString &text)
        { setData(QAbstractItemModel::DisplayRole, text); }

    inline QIconSet icon() const
        { return data(QAbstractItemModel::DecorationRole).toIcon(); }
    inline void setIcon(const QIconSet &icon)
        { setData(QAbstractItemModel::DecorationRole, icon); }

    inline QString statusTip() const
        { return data(QAbstractItemModel::StatusTipRole).toString(); }
    inline void setStatusTip(const QString &statusTip)
        { setData(QAbstractItemModel::StatusTipRole, statusTip); }

    inline QString toolTip() const
        { return data(QAbstractItemModel::ToolTipRole).toString(); }
    inline void setToolTip(const QString &toolTip)
         { setData(QAbstractItemModel::ToolTipRole, toolTip); }

    inline QString whatsThis() const
        { return data(QAbstractItemModel::WhatsThisRole).toString(); }
    inline void setWhatsThis(const QString &whatsThis)
        { setData(QAbstractItemModel::WhatsThisRole, whatsThis); }

    inline QFont font() const
        { return data(QAbstractItemModel::FontRole).toFont(); }
    inline void setFont(const QFont &font)
        { setData(QAbstractItemModel::FontRole, font); }

    inline QColor backgroundColor() const
        { return data(QAbstractItemModel::BackgroundColorRole).toColor(); }
    virtual void setBackgroundColor(const QColor &color)
        { setData(QAbstractItemModel::BackgroundColorRole, color); }

    inline QColor textColor() const
        { return data(QAbstractItemModel::TextColorRole).toColor(); }
    inline void setTextColor(const QColor &color)
        { setData(QAbstractItemModel::TextColorRole, color); }

    inline int checked() const
        { return data(QAbstractItemModel::CheckStateRole).toInt(); }
    inline void setChecked(const bool checked)
        { setData(QAbstractItemModel::CheckStateRole, checked); }

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);
    virtual bool operator<(const QTableWidgetItem &other) const;

protected:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QVector<Data> values;
    QTableWidget *view;
    QAbstractItemModel::ItemFlags itemFlags;
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
