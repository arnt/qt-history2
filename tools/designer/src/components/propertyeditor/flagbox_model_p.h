#ifndef FLAGBOX_MODEL_P_H
#define FLAGBOX_MODEL_P_H

#include "propertyeditor_global.h"
#include <QAbstractItemModel>
#include <QList>

class QT_PROPERTYEDITOR_EXPORT FlagBoxModelItem
{
public:
    FlagBoxModelItem(const QString &name, int value, bool checked = false)
        : m_name(name), m_value(value), m_checked(checked) {}

    inline int value() const { return m_value; }

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }

    inline bool isChecked() const { return m_checked; }
    inline void setChecked(bool checked) { m_checked = checked; }

private:
    QString m_name;
    int m_value;
    uint m_checked: 1;
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

#endif // FLAGBOX_MODEL_P_H
