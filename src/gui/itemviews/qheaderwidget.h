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
#endif

class QHeaderWidget;

class Q_GUI_EXPORT QHeaderWidgetItem
{
public:
    QHeaderWidgetItem(QHeaderWidget *view);
    virtual ~QHeaderWidgetItem();

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
    virtual bool operator<(const QHeaderWidgetItem &other) const;

protected:
    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QVector<Data> values;
    QAbstractItemModel::ItemFlags itemFlags;
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
