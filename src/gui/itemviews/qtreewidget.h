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

    // additional roles used in these items
    enum Role {
        FontRole = 33,
        BackgroundColorRole = 34,
        TextColorRole = 35,
        CheckRole = 36
    };

    enum CheckedState {
        Unchecked = 0,
        PartiallyChecked = 1,
        Checked = 2
    };

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
    inline const QTreeWidgetItem *parent() const { return par; }
    inline const QTreeWidgetItem *child(int index) const { return children.at(index); }
    inline QTreeWidgetItem *child(int index) { return children.at(index); }
    inline int childCount() const { return children.count(); }

    inline int columnCount() const { return columns; }
    void setColumnCount(int count);

    inline bool isEditable() const { return editable; }
    inline void setEditable(bool enable) { editable = enable; }
    inline bool isSelectable() const { return selectable; }
    inline void setSelectable(bool enable) { selectable = enable; }
    inline bool isCheckable() const { return checkable; }
    inline void setCheckable(bool enable) { checkable = enable; }
    inline bool isEnabled() const { return enabled; }
    inline void setEnabled(bool enable) { enabled = enable; }

    inline bool operator ==(const QTreeWidgetItem &other) const
        { return par == other.par && children == other.children; }
    inline bool operator !=(const QTreeWidgetItem &other) const { return !operator==(other); }

protected:
    QTreeWidgetItem();

    void store(int column, int role, const QVariant &value);
    QVariant retrieve(int column, int role) const;

private:
    QTreeWidgetItem *par;
    QList<QTreeWidgetItem*> children;

    struct Data {
        Data() : role(-1) {}
        Data(int r, QVariant v) : role(r), value(v) {}
        int role;
        QVariant value;
    };
    QVector< QVector<Data> > values; // each column has a vector of role values

    QTreeWidget *view;
    int columns;
    uint editable : 1;
    uint selectable : 1;
    uint checkable : 1;
    uint enabled : 1;
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

    QString columnStatusTip(int column) const;
    void setColumnStatusTip(int column, const QString &statusTip);

    QString columnToolTip(int column) const;
    void setColumnToolTip(int column, const QString &toolTip);

    QString columnWhatsThis(int column) const;
    void setColumnWhatsThis(int column, const QString &whatsThis);

    QFont columnFont(int column) const;
    void setColumnFont(int column, const QFont &font);

    QColor columnBackgroundColor(int column) const;
    void setColumnBackgroundColor(int column, const QColor &color);

    QColor columnTextColor(int column) const;
    void setColumnTextColor(int column, const QColor &color);
    
    QVariant columnData(int column, int role) const;
    void setColumnData(int column, int role, const QVariant &value);

protected:
    void append(QTreeWidgetItem *item);
};

#endif
