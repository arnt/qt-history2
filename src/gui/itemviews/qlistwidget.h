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

#ifndef QT_H
#include <qlistview.h>
#include <qabstractitemmodel.h>
#include <qiconset.h>
#include <qstring.h>
#include <qvector.h>
#endif

class Q_GUI_EXPORT QListWidgetItem
{

public:
    QListWidgetItem()  : edit(true), select(true) {}
    ~QListWidgetItem() {}

    inline QString text() const { return data(QAbstractItemModel::DisplayRole).toString(); }
    inline QIconSet iconSet() const { return data(QAbstractItemModel::DecorationRole).toIconSet(); }

    inline bool isEditable() const { return edit; }
    inline bool isSelectable() const { return select; }

    inline void setText(const QString &text) { setData(QAbstractItemModel::DisplayRole, text); }
    inline void setIconSet(const QIconSet &iconSet) { setData(QAbstractItemModel::DisplayRole, iconSet); }

    inline void setEditable(bool editable) { edit = editable; }
    inline void setSelectable(bool selectable) { select = selectable; }

    bool operator ==(const QListWidgetItem &other) const;
    inline bool operator !=(const QListWidgetItem &other) const { return !operator==(other); }

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

private:
    struct Data {
        Data() {}
        Data(int r, QVariant v) {
            role = r;
            value = v;
        }
        int role;
        QVariant value;
    };

    QVector<Data> values;
    uint edit : 1;
    uint select : 1;
};

class QListWidgetPrivate;

class Q_GUI_EXPORT QListWidget : public QListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QListWidget)

public:
#ifdef QT_COMPAT
    QListWidget(QWidget *parent, const char* name);
#endif
    QListWidget(QWidget *parent = 0);
    ~QListWidget();

    void setText(int row, const QString &text);
    void setIconSet(int row, const QIconSet &iconSet);
    QString text(int row) const;
    QIconSet iconSet(int row) const;

    QListWidgetItem item(int row) const;
    void setItem(int row, const QListWidgetItem &item);
    void appendItem(const QListWidgetItem &item);
};

#endif
