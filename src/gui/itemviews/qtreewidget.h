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

    inline int columnCount() const { return columns; }
    void setColumnCount(int count);

    inline bool isEditable() const { return editable; }
    inline bool isSelectable() const { return selectable; }
    inline void setEditable(bool enable) { editable = enable; }
    inline void setSelectable(bool enable) { selectable = enable; }

    inline bool operator ==(const QTreeWidgetItem &other) const
        { return par == other.par && children == other.children; }
    inline bool operator !=(const QTreeWidgetItem &other) const { return !operator==(other); }

    // these functions are intended to be reimplemented
    virtual QString text(int column) const;
    virtual QIconSet icon(int column) const;
    virtual void setText(int column, const QString &text);
    virtual void setIcon(int column, const QIconSet &icon);
    
    virtual QVariant data(int column, int role) const;
    virtual void setData(int column, int role, const QVariant &value);

private:
    QTreeWidgetItem();

    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;

    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };

    QVector< QVector<Data> > values;
    QTreeWidget *view;
    int columns;
    uint editable : 1;
    uint selectable : 1;
};

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

    QString columnText(int column) const;
    void setColumnText(int column, const QString &text);

    QIconSet columnIcon(int column) const;
    void setColumnIcon(int column, const QIconSet &icon);

    QVariant columnData(int column, int role) const;
    void setColumnData(int column, int role, const QVariant &value);

protected:
    void append(QTreeWidgetItem *item);
};

#endif
