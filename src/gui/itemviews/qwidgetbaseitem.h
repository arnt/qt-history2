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

#ifndef QWIDGETBASEITEM_H
#define QWIDGETBASEITEM_H

#ifndef QT_H
#include <qvector.h>
#include <qvariant.h>
#include <qstring.h>
#include <qiconset.h>
#include <qabstractitemmodel.h>
#include <qitemdelegate.h>
#endif

class QWidgetBaseItem
{
public:
    inline bool isEditable() const { return editable; }
    inline void setEditable(bool enable) { editable = enable; }
    inline bool isSelectable() const { return selectable; }
    inline void setSelectable(bool enable) { selectable = enable; }
    inline bool isCheckable() const { return checkable; }
    inline void setCheckable(bool enable) { checkable = enable; }
    inline bool isEnabled() const { return enabled; }
    inline void setEnabled(bool enable) { enabled = enable; }

    // additional roles used in these items
    enum Role {
        FontRole = QAbstractItemModel::UserRole + 1,
        BackgroundColorRole = QAbstractItemModel::UserRole + 2,
        TextColorRole = QAbstractItemModel::UserRole + 3,
        CheckRole = QAbstractItemModel::UserRole +  4
    };

    enum CheckedState {
        Unchecked = 0,
        PartiallyChecked = 1,
        Checked = 2
    };

protected:
    QWidgetBaseItem() : editable(true), selectable(true), checkable(false), enabled(true) {}
    virtual ~QWidgetBaseItem() {}

    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    uint editable : 1;
    uint selectable : 1;
    uint checkable : 1;
    uint enabled : 1;
};

class Q_GUI_EXPORT QWidgetCellItem : public QWidgetBaseItem
{
public:
    virtual QString text() const;
    virtual void setText(const QString &text);

    virtual QIconSet icon() const;
    virtual void setIcon(const QIconSet &icon);

    virtual QString statusTip() const;
    virtual void setStatusTip(const QString &statusTip);

    virtual QString toolTip() const;
    virtual void setToolTip(const QString &toolTip);

    virtual QString whatsThis() const;
    virtual void setWhatsThis(const QString &whatsThis);

    virtual QFont font() const;
    virtual void setFont(const QFont &font);

    virtual QColor backgroundColor() const;
    virtual void setBackgroundColor(const QColor &color);

    virtual QColor textColor() const;
    virtual void setTextColor(const QColor &color);

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);

protected:
    QWidgetCellItem() {}
    ~QWidgetCellItem() {}

    void store(int role, const QVariant &value);
    QVariant retrieve(int role) const;

private:
    QVector<QWidgetBaseItem::Data> values;
};

class QWidgetBaseItemDelegate : public QItemDelegate
{
public:
    QWidgetBaseItemDelegate(QObject *parent) : QItemDelegate(parent) {}
    ~QWidgetBaseItemDelegate() {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif
