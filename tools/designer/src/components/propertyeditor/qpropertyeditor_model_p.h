#ifndef QPROPERTYEDITOR_MODEL_P_H
#define QPROPERTYEDITOR_MODEL_P_H

#include "qpropertyeditor_items_p.h"
#include <qabstractitemmodel.h>

namespace QPropertyEditor
{

class Model: public QAbstractItemModel
{
    Q_OBJECT
public:
    Model(QObject *parent = 0);
    ~Model();

    inline I::Property *initialInput() const
    { return m_initialInput; }

    inline QModelIndex indexOf(I::Property *property, int column = 0) const
    {
        if (property == m_initialInput)
            return createIndex(0, column, m_initialInput);

        I::Property *parent = parentOf(property);
        if (!parent || parent->kind() != I::Property_Group)
            return QModelIndex();

        int row = static_cast<I::PropertyGroup*>(parent)->indexOf(property);
        return createIndex(row, column, property);
    }

    inline I::Property *privateData(const QModelIndex &index) const
    { return static_cast<I::Property*>(index.data()); }

    ItemFlags flags(const QModelIndex &index) const;

signals:
    void propertyChanged(I::Property *property);

public slots:
    void setInitialInput(I::Property *initialInput);
    void propertyAdded(I::Property *property);
    void propertyRemoved(const QModelIndex &index);
    void refresh(I::Property *property);
    void refresh();

public:
//
// QAbstractItemModel interface
//
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const
    { return rowCount(parent) > 0; }

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    virtual bool isEditable(const QModelIndex &index) const;
    virtual QModelIndex buddy(const QModelIndex &index) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected:
    QString columnText(int column) const;

    inline I::Property *childAt(I::Property *parent, int pos) const
    {
        if (parent && parent->kind() == I::Property_Group)
            return static_cast<I::PropertyGroup*>(parent)->propertyAt(pos);

        return 0;
    }

    inline I::Property *parentOf(I::Property *property) const
    { return property ? property->parent() : 0; }

    static I::PropertyGroup *toPropertyGroup(I::Property *property)
    {
        if (!property || property->kind() != I::Property_Group)
            return 0;

        return static_cast<I::PropertyGroup*>(property);
    }

private:
    I::Property *m_initialInput;
};

} // namespace QPropertyEditor

#endif // QPROPERTYEDITOR_MODEL_P_H
