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
#include <qlist.h>

class QListWidget;
class QListModel;

class Q_GUI_EXPORT QListWidgetItem
{
    friend class QListModel;
public:
    QListWidgetItem(QListWidget *view = 0);
    QListWidgetItem(const QString &text, QListWidget *view = 0);
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

    inline int textAlignment() const
        { return data(QAbstractItemModel::TextAlignmentRole).toInt(); }
    inline void setTextAlignment(int alignment)
        { setData(QAbstractItemModel::TextAlignmentRole, alignment); }

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

//     inline bool isHidden() const { return hidden; }
//     inline void setHidden(bool hide) { hidden = hide; }
//     inline void hide() { setHidden(true); }
//     inline void show() { setHidden(false); }

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);
    virtual bool operator<(const QListWidgetItem &other) const;

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
//    bool hidden;
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
    void insertItem(int row, const QString &label);
    void insertItems(int row, const QStringList &labels);
    inline void appendItem(QListWidgetItem *item) { insertItem(count(), item); }
    inline void appendItem(const QString &label) { insertItems(count(), QStringList(label)); }
    inline void appendItems(const QStringList &labels) { insertItems(count(), labels); }
    QListWidgetItem *takeItem(int row);
    int count() const;
    void sort(Qt::SortOrder order);

    QListWidgetItem *currentItem() const;
    void setCurrentItem(QListWidgetItem *item);

    void openPersistentEditor(QListWidgetItem *item);
    void closePersistentEditor(QListWidgetItem *item);

    bool isSelected(const QListWidgetItem *item) const;
    void setSelected(const QListWidgetItem *item, bool select);
    QList<QListWidgetItem*> selectedItems() const;

    QList<QListWidgetItem*> findItems(const QString &text,
                                      QAbstractItemModel::MatchFlags flags
                                      = QAbstractItemModel::MatchDefault) const;

    bool isVisible(const QListWidgetItem *item) const;

public slots:
    void ensureItemVisible(const QListWidgetItem *item);
    void clear();

signals:
    void pressed(QListWidgetItem *item, int button);
    void clicked(QListWidgetItem *item, int button);
    void doubleClicked(QListWidgetItem *item, int button);
    void keyPressed(QListWidgetItem *item, Qt::Key, Qt::ButtonState state);
    void returnPressed(QListWidgetItem *item);
    void currentChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void selectionChanged();

protected:
    void removeItem(QListWidgetItem *item);
    void setModel(QAbstractItemModel *model);
    void setup();

private:
    Q_PRIVATE_SLOT(d, void emitPressed(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitClicked(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitDoubleClicked(const QModelIndex &index, int button));
    Q_PRIVATE_SLOT(d, void emitKeyPressed(const QModelIndex &index, Qt::Key key, Qt::ButtonState state));
    Q_PRIVATE_SLOT(d, void emitReturnPressed(const QModelIndex &index));
    Q_PRIVATE_SLOT(d, void emitCurrentChanged(const QModelIndex &previous, const QModelIndex &current));
};

#endif
