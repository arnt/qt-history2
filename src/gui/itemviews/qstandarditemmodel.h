/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTANDARDITEMMODEL_H
#define QSTANDARDITEMMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qlist.h>
#include <QtGui/qfont.h>
#include <QtGui/qicon.h>
#ifndef QT_NO_DATASTREAM
#include <QtCore/qdatastream.h>
#endif

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_STANDARDITEMMODEL

class QStandardItemModel;

class QStandardItemPrivate;
class Q_GUI_EXPORT QStandardItem
{
public:
    QStandardItem();
    QStandardItem(const QString &text);
    QStandardItem(const QIcon &icon, const QString &text);
    QStandardItem(int rows, int columns = 1);
    QStandardItem(const QList<QStandardItem*> &items);
    virtual ~QStandardItem();

    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);

    inline QString text() const {
        return qvariant_cast<QString>(data(Qt::DisplayRole));
    }
    inline void setText(const QString &text);

    inline QIcon icon() const {
        return qvariant_cast<QIcon>(data(Qt::DecorationRole));
    }
    inline void setIcon(const QIcon &icon);

#ifndef QT_NO_TOOLTIP
    inline QString toolTip() const {
        return qvariant_cast<QString>(data(Qt::ToolTipRole));
    }
    inline void setToolTip(const QString &toolTip);
#endif

    inline QString statusTip() const {
        return qvariant_cast<QString>(data(Qt::StatusTipRole));
    }
    inline void setStatusTip(const QString &statusTip);

#ifndef QT_NO_WHATSTHIS
    inline QString whatsThis() const {
        return qvariant_cast<QString>(data(Qt::WhatsThisRole));
    }
    inline void setWhatsThis(const QString &whatsThis);
#endif

    inline QSize sizeHint() const {
        return qvariant_cast<QSize>(data(Qt::SizeHintRole));
    }
    inline void setSizeHint(const QSize &sizeHint);

    inline QFont font() const {
        return qvariant_cast<QFont>(data(Qt::FontRole));
    }
    inline void setFont(const QFont &font);

    inline Qt::Alignment textAlignment() const {
        return Qt::Alignment(qvariant_cast<int>(data(Qt::TextAlignmentRole)));
    }
    inline void setTextAlignment(Qt::Alignment textAlignment);

    inline QColor backgroundColor() const {
        return qvariant_cast<QColor>(data(Qt::BackgroundColorRole));
    }
    inline void setBackgroundColor(const QColor &backgroundColor);

    inline QColor textColor() const {
        return qvariant_cast<QColor>(data(Qt::TextColorRole));
    }
    inline void setTextColor(const QColor &textColor);

    inline Qt::CheckState checkState() const {
        return Qt::CheckState(qvariant_cast<int>(data(Qt::CheckStateRole)));
    }
    inline void setCheckState(Qt::CheckState checkState);

    inline QString accessibleText() const {
        return qvariant_cast<QString>(data(Qt::AccessibleTextRole));
    }
    inline void setAccessibleText(const QString &accessibleText);

    inline QString accessibleDescription() const {
        return qvariant_cast<QString>(data(Qt::AccessibleDescriptionRole));
    }
    inline void setAccessibleDescription(const QString &accessibleDescription);

    Qt::ItemFlags flags() const;
    void setFlags(Qt::ItemFlags flags);

    inline bool isEnabled() const {
        return (flags() & Qt::ItemIsEnabled) != 0;
    }
    void setEnabled(bool enabled);

    inline bool isEditable() const {
        return (flags() & Qt::ItemIsEditable) != 0;
    }
    void setEditable(bool editable);

    inline bool isSelectable() const {
        return (flags() & Qt::ItemIsSelectable) != 0;
    }
    void setSelectable(bool selectable);

    inline bool isCheckable() const {
        return (flags() & Qt::ItemIsUserCheckable) != 0;
    }
    void setCheckable(bool checkable);

    inline bool isTristate() const {
        return (flags() & Qt::ItemIsTristate) != 0;
    }
    void setTristate(bool tristate);

    inline bool isDragEnabled() const {
        return (flags() & Qt::ItemIsDragEnabled) != 0;
    }
    void setDragEnabled(bool dragEnabled);

    inline bool isDropEnabled() const {
        return (flags() & Qt::ItemIsDropEnabled) != 0;
    }
    void setDropEnabled(bool dropEnabled);

    QStandardItem *parent() const;
    int row() const;
    int column() const;
    QModelIndex index() const;
    QStandardItemModel *model() const;
    bool isTopLevelItem() const;

    int rowCount() const;
    void setRowCount(int rows);
    int columnCount() const;
    void setColumnCount(int columns);

    bool hasChildren() const;
    QStandardItem *child(int row, int column = 0) const;
    void setChild(int row, int column, QStandardItem *item);
    inline void setChild(int row, QStandardItem *item);

    void insertRow(int row, const QList<QStandardItem*> &items);
    void insertColumn(int column, const QList<QStandardItem*> &items);
    void insertRows(int row, int count);
    void insertColumns(int column, int count);

    void removeRow(int row);
    void removeColumn(int column);
    void removeRows(int row, int count);
    void removeColumns(int column, int count);

    inline void appendRow(const QList<QStandardItem*> &items);
    inline void appendColumn(const QList<QStandardItem*> &items);
    inline void insertRow(int row, QStandardItem *item);
    inline void appendRow(QStandardItem *item);

    QStandardItem *takeChild(int row, int column = 0);
    QList<QStandardItem*> takeRow(int row);
    QList<QStandardItem*> takeColumn(int column);

    virtual QStandardItem *clone() const;

    enum ItemType { Type = 0, UserType = 1000 };
    virtual int type() const;

#ifndef QT_NO_DATASTREAM
    virtual void read(QDataStream &in);
    virtual void write(QDataStream &out) const;
#endif
    virtual bool operator<(const QStandardItem &other) const;

protected:
    QStandardItem(const QStandardItem &other);
    QStandardItem(QStandardItemPrivate &dd);
    QStandardItem &operator=(const QStandardItem &other);
    QStandardItemPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QStandardItem)
    friend class QStandardItemModelPrivate;
    friend class QStandardItemModel;
};

inline void QStandardItem::setText(const QString &atext)
{ setData(Qt::DisplayRole, atext); }

inline void QStandardItem::setIcon(const QIcon &aicon)
{ setData(Qt::DecorationRole, aicon); }

inline void QStandardItem::setToolTip(const QString &atoolTip)
{ setData(Qt::ToolTipRole, atoolTip); }

inline void QStandardItem::setStatusTip(const QString &astatusTip)
{ setData(Qt::StatusTipRole, astatusTip); }

inline void QStandardItem::setWhatsThis(const QString &awhatsThis)
{ setData(Qt::WhatsThisRole, awhatsThis); }

inline void QStandardItem::setSizeHint(const QSize &asizeHint)
{ setData(Qt::SizeHintRole, asizeHint); }

inline void QStandardItem::setFont(const QFont &afont)
{ setData(Qt::FontRole, afont); }

inline void QStandardItem::setTextAlignment(Qt::Alignment atextAlignment)
{ setData(Qt::TextAlignmentRole, int(atextAlignment)); }

inline void QStandardItem::setBackgroundColor(const QColor &abackgroundColor)
{ setData(Qt::BackgroundColorRole, abackgroundColor); }

inline void QStandardItem::setTextColor(const QColor &atextColor)
{ setData(Qt::TextColorRole, atextColor); }

inline void QStandardItem::setCheckState(Qt::CheckState acheckState)
{ setData(Qt::CheckStateRole, acheckState); }

inline void QStandardItem::setAccessibleText(const QString &aaccessibleText)
{ setData(Qt::AccessibleTextRole, aaccessibleText); }

inline void QStandardItem::setAccessibleDescription(const QString &aaccessibleDescription)
{ setData(Qt::AccessibleDescriptionRole, aaccessibleDescription); }

inline void QStandardItem::setChild(int arow, QStandardItem *aitem)
{ setChild(arow, 0, aitem); }

inline void QStandardItem::appendRow(const QList<QStandardItem*> &aitems)
{ insertRow(rowCount(), aitems); }

inline void QStandardItem::appendColumn(const QList<QStandardItem*> &aitems)
{ insertColumn(columnCount(), aitems); }

inline void QStandardItem::insertRow(int arow, QStandardItem *aitem)
{ insertRow(arow, QList<QStandardItem*>() << aitem); }

inline void QStandardItem::appendRow(QStandardItem *aitem)
{ insertRow(rowCount(), aitem); }

class QStandardItemModelPrivate;

class Q_GUI_EXPORT QStandardItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QStandardItemModel(QObject *parent = 0);
    QStandardItemModel(int rows, int columns, QObject *parent = 0);
    QStandardItemModel(const QList<QStandardItem*> &items, QObject *parent = 0);
    ~QStandardItemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    void clear();

#ifdef Q_NO_USING_KEYWORD
    inline QObject *parent() const { return QObject::parent(); }
#else
    using QObject::parent;
#endif

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QStandardItem *itemFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromItem(const QStandardItem *item) const;

    QStandardItem *item(int row, int column = 0) const;
    void setItem(int row, int column, QStandardItem *item);
    inline void setItem(int row, QStandardItem *item);
    QStandardItem *topLevelParent() const;

    QStandardItem *horizontalHeaderItem(int column) const;
    void setHorizontalHeaderItem(int column, QStandardItem *item);
    QStandardItem *verticalHeaderItem(int row) const;
    void setVerticalHeaderItem(int row, QStandardItem *item);

    void setHorizontalHeaderLabels(const QStringList &labels);
    void setVerticalHeaderLabels(const QStringList &labels);

    void setRowCount(int rows);
    void setColumnCount(int columns);

    void appendRow(const QList<QStandardItem*> &items);
    void appendColumn(const QList<QStandardItem*> &items);
    inline void appendRow(QStandardItem *item);

    QStandardItem *takeItem(int row, int column = 0);
    QList<QStandardItem*> takeRow(int row);
    QList<QStandardItem*> takeColumn(int column);

    QStandardItem *takeHorizontalHeaderItem(int column);
    QStandardItem *takeVerticalHeaderItem(int row);

    const QStandardItem *itemPrototype() const;
    void setItemPrototype(const QStandardItem *item);

    QList<QStandardItem*> findItems(const QString &text, Qt::MatchFlags flags, int column = 0) const;

protected:
    QStandardItemModel(QStandardItemModelPrivate &dd, QObject *parent = 0);

private:
    friend class QStandardItemPrivate;
    friend class QStandardItem;
    Q_DISABLE_COPY(QStandardItemModel)
    Q_DECLARE_PRIVATE(QStandardItemModel)
};

inline void QStandardItemModel::setItem(int arow, QStandardItem *aitem)
{ setItem(arow, 0, aitem); }

inline void QStandardItemModel::appendRow(QStandardItem *aitem)
{ appendRow(QList<QStandardItem*>() << aitem); }

#ifndef QT_NO_DATASTREAM
QDataStream &operator>>(QDataStream &in, QStandardItem &item);
QDataStream &operator<<(QDataStream &out, const QStandardItem &item);
#endif

#endif // QT_NO_STANDARDITEMMODEL

QT_END_HEADER

#endif //QSTANDARDITEMMODEL_H
