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

#ifndef QTREEWIDGET_H
#define QTREEWIDGET_H

#ifndef QT_H
#include <qtreeview.h>
#include <qlist.h>
#include <qvector.h>
#include <qiconset.h>
#include <qstring.h>
#endif

class QTreeWidget;
class QTreeModel;

class Q_GUI_EXPORT QTreeWidgetItem
{
    friend class QTreeModel;
public:
    QTreeWidgetItem(QTreeWidget *view);
    QTreeWidgetItem(QTreeWidgetItem *parent);
    virtual ~QTreeWidgetItem();

    inline const QTreeWidgetItem *parent() const { return par; }
    inline const QTreeWidgetItem *child(int index) const { return children.at(index); }
    inline QTreeWidgetItem *child(int index) { return children.at(index); }
    inline int childCount() const { return children.count(); }

    inline int columnCount() const { return c; }
    inline QString text(int column) const
        { return data(column, QAbstractItemModel::DisplayRole).toString(); }
    inline QIconSet iconSet(int column) const
        { return data(column, QAbstractItemModel::DecorationRole).toIconSet(); }

    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    void setColumnCount(int columns);
    inline void setText(int column, const QString &text)
        { setData(column, QAbstractItemModel::DisplayRole, text); }
    inline void setIconSet(int column, const QIconSet &iconSet)
        { setData(column, QAbstractItemModel::DecorationRole, iconSet); }

    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    inline bool operator ==(const QTreeWidgetItem &other) const
        { return par == other.par && children == other.children; }
    inline bool operator !=(const QTreeWidgetItem &other) const { return !operator==(other); }

    QVariant data(int column, int role) const;
    void setData(int column, int role, const QVariant &value);

private:
    QTreeWidgetItem();

    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;

    struct Data {
        Data() {}
        Data(int r, QVariant v) {
            role = r;
            value = v;
        }
        int role;
        QVariant value;
    };

    QVector< QVector<Data> > values;
    QTreeWidget *view;
    int c;
    uint edit : 1;
    uint select : 1;
};

class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeWidget)

    friend class QTreeWidgetItem;
public:
    QTreeWidget(QWidget *parent = 0);

    void setColumnCount(int columns);
    void setColumnText(int column, const QString &text);
    void setColumnIconSet(int column, const QIconSet &iconSet);
    QString columnText(int column) const;
    QIconSet columnIconSet(int column) const;

protected:
    void append(QTreeWidgetItem *item);
};

#endif
