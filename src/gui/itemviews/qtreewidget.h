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
#include <qwidgetbaseitem.h>
#include <qlist.h>
#endif

class QTreeModel;
class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeWidget)

    friend class QTreeWidgetItem;
public:
    QTreeWidget(QWidget *parent = 0);

    int columnCount() const;
    void setColumnCount(int columns);

    QTreeWidgetItem *headerItem();
    void setHeaderItem(QTreeWidgetItem *item);

protected:
    void appendItem(QTreeWidgetItem *item);
    void removeItem(QTreeWidgetItem *item);
    bool isSelected(QTreeWidgetItem *item) const;
    void setModel(QAbstractItemModel *model);
};

class Q_GUI_EXPORT QTreeWidgetItem : public QWidgetBaseItem
{
    friend class QTreeModel;
public:
    QTreeWidgetItem(QTreeWidget *view);
    QTreeWidgetItem(QTreeWidgetItem *parent);
    virtual ~QTreeWidgetItem();

    // these functions are intended to be reimplemented    
    virtual CheckedState checkedState() const;
    virtual void setCheckedState(CheckedState state);

    virtual QString text(int column) const;
    virtual void setText(int column, const QString &text);
    
    virtual QIconSet icon(int column) const;
    virtual void setIcon(int column, const QIconSet &icon);

    virtual QString statusTip(int column) const;
    virtual void setStatusTip(int column, const QString &statusTip);

    virtual QString toolTip(int column) const;
    virtual void setToolTip(int column, const QString &toolTip);

    virtual QString whatsThis(int column) const;
    virtual void setWhatsThis(int column, const QString &whatsThis);

    virtual QFont font(int column) const;
    virtual void setFont(int column, const QFont &font);

    virtual QColor backgroundColor(int column) const;
    virtual void setBackgroundColor(int column, const QColor &color);

    virtual QColor textColor(int column) const;
    virtual void setTextColor(int column, const QColor &color);
    
    virtual QVariant data(int column, int role) const;
    virtual void setData(int column, int role, const QVariant &value);

    // other functions
    inline QTreeWidgetItem *parent() { return par; }
    inline QTreeWidgetItem *child(int index) { return children.at(index); }
    inline int childCount() const { return children.count(); }

    inline bool isSelected() { return view->isSelected(this); }
    
    inline int columnCount() const { return values.count(); }
    inline void setColumnCount(int count) { values.resize(count); }

protected:
    QTreeWidgetItem();

    void store(int column, int role, const QVariant &value);
    QVariant retrieve(int column, int role) const;

private:
    QTreeWidget *view;
    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;
    QVector< QVector<QWidgetBaseItem::Data> > values;
    // One item has a vector of column entries. Each column has a vector of (role, value) pairs.
};

#endif
