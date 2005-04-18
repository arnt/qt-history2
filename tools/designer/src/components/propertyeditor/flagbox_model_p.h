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

#ifndef FLAGBOX_MODEL_P_H
#define FLAGBOX_MODEL_P_H

#include "propertyeditor_global.h"
#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>

namespace qdesigner { namespace components { namespace propertyeditor {

class QT_PROPERTYEDITOR_EXPORT FlagBoxModelItem
{
public:
    FlagBoxModelItem(const QString &name, unsigned int value, bool checked = false)
        : m_name(name), m_value(value), m_checked(checked) {}

    inline unsigned int value() const { return m_value; }

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }

    inline bool isChecked() const { return m_checked; }
    inline void setChecked(bool checked) { m_checked = checked; }

private:
    QString m_name;
    unsigned int m_value;
    bool m_checked;
};

class QT_PROPERTYEDITOR_EXPORT FlagBoxModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    FlagBoxModel(QObject *parent = 0);
    virtual ~FlagBoxModel();

    inline FlagBoxModelItem itemAt(int index) const
    { return m_items.at(index); }

    inline FlagBoxModelItem &item(int index)
    { return m_items[index]; }

    inline QList<FlagBoxModelItem> items() const { return m_items; }
    void setItems(const QList<FlagBoxModelItem> &items);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    QList<FlagBoxModelItem> m_items;
};

} } } // namespace qdesigner::components::propertyeditor

#endif // FLAGBOX_MODEL_P_H
