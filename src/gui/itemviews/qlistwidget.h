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

#include <qlistview.h>

class QListWidget;
class QListModel;

class Q_GUI_EXPORT QListWidgetItem
{
    friend class QListModel;
public:
    QListWidgetItem(QListWidget *view = 0);
    virtual ~QListWidgetItem();

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
    virtual bool operator<(const QListWidgetItem &other) const;

    void openPersistentEditor();
    void closePersistentEditor();

    bool isSelected() const;
    void setSelected(bool select);

protected:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QVector<Data> values;

private:
    QAbstractItemModel::ItemFlags itemFlags;
    QListModel *model;
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
    int row(const QListWidgetItem *item) const;
    void insertItem(int row, QListWidgetItem *item);
    void insertItems(const QStringList &labels, int row = -1);
    void appendItem(QListWidgetItem *item);
    QListWidgetItem *takeItem(int row);
    int count() const;
    void sort(Qt::SortOrder order);

    void openPersistentEditor(QListWidgetItem *item);
    void closePersistentEditor(QListWidgetItem *item);

    bool isSelected(const QListWidgetItem *item) const;
    void setSelected(const QListWidgetItem *item, bool select);

signals:
    void clicked(QListWidgetItem *item, int button);
    void doubleClicked(QListWidgetItem *item, int button);
    void returnPressed(QListWidgetItem *item);
    void spacePressed(QListWidgetItem *item);

protected:
    void removeItem(QListWidgetItem *item);
    void setModel(QAbstractItemModel *model);

private:
    Q_PRIVATE_SLOT(d, void emitClicked(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitDoubleClicked(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitReturnPressed(const QModelIndex &index));
    Q_PRIVATE_SLOT(d, void emitSpacePressed(const QModelIndex &index));
};

#endif
