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

#include "qpropertyeditor_model_p.h"
#include <QtCore/qdebug.h>

using namespace QPropertyEditor;

Model::Model(QObject *parent)
    : QAbstractItemModel(parent), m_initialInput(0)
{
}

Model::~Model()
{
}

void Model::setInitialInput(IProperty *initialInput)
{
    Q_ASSERT(initialInput);

    m_initialInput = initialInput;
    emit reset();
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, m_initialInput);

    return createIndex(row, column, childAt(privateData(parent), row));
}

QModelIndex Model::parent(const QModelIndex &index) const
{
    if (!index.isValid() || privateData(index) == m_initialInput)
        return QModelIndex();

    Q_ASSERT(privateData(index));

    return indexOf(parentOf(privateData(index)));
}

int Model::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;

    if (IProperty *p = privateData(parent)) {
        return (p->kind() == IProperty::Property_Group)
            ? static_cast<IPropertyGroup*>(p)->propertyCount()
            : 0;
    }

    return (m_initialInput->kind() == IProperty::Property_Group)
        ? static_cast<IPropertyGroup*>(m_initialInput)->propertyCount()
        : 0;
}

int Model::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
}

bool Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (IProperty *property = privateData(index)) {
        if (role == EditRole) {
            property->setValue(value);
            refresh(property);

            IProperty *nonfake = property;
            while (nonfake != 0 && nonfake->isFake())
                nonfake = nonfake->parent();
            if (nonfake != 0 && nonfake->dirty()) {
                nonfake->setDirty(false);
                emit propertyChanged(nonfake);
            }
        }

        return true;
    }

    return false;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!privateData(index))
        return QVariant();

    IProperty *o = privateData(index);
    switch (index.column()) {  // ### cleanup
        case 0:
            switch (role) {
                case EditRole:
                case DisplayRole:
                    return o->propertyName().isEmpty()
                        ? QLatin1String("<noname>")
                        : o->propertyName();
                default:
                    break;
            }
            break;

        case 1: {
            switch (role) {
                case EditRole:
                    return o->value();
                case DisplayRole:
                    return o->toString();
                case DecorationRole:
                    return o->decoration();
                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return QVariant();
}

QString Model::columnText(int col) const
{
    switch (col) {
        case 0: return QLatin1String("Property");
        case 1: return QLatin1String("Value");
        default: return QString();
    }
}



void Model::refreshHelper(IProperty *property)
{
    QModelIndex index0 = indexOf(property, 0);
    QModelIndex index1 = indexOf(property, 1);
    emit dataChanged(index0, index1);
}

void Model::refresh(IProperty *property)
{
    refreshHelper(property);

    // Refresh everyone up to the root

    IProperty *prop = property;
    while (prop != 0) {
        refreshHelper(prop);
        prop = prop->parent();
    }

    // Refresh all children

    if (property->kind() == IProperty::Property_Group) {
        IPropertyGroup *prop_group = static_cast<IPropertyGroup*>(property);
        for (int i = 0; i < prop_group->propertyCount(); ++i) {
            IProperty *child_prop = prop_group->propertyAt(i);
            refreshHelper(child_prop);
        }
    }
}

void Model::propertyAdded(IProperty *property)
{
    QModelIndex p = parent(indexOf(property));
    int r = rowCount(p);

    emit rowsInserted(p, r - 1, r);
}

void Model::propertyRemoved(const QModelIndex &index)
{
    emit rowsAboutToBeRemoved(parent(index), index.row(), index.row() + 1);
}

void Model::refresh()
{
    refresh(m_initialInput);
}

bool Model::isEditable(const QModelIndex &index) const
{
    return index.column() == 1 && privateData(index)->hasEditor();
}

QModelIndex Model::buddy(const QModelIndex &index) const
{
    if (index.column() == 0)
        return createIndex(index.row(), 1, index.data());
    return index;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role != DisplayRole)
            return QVariant();
        return columnText(section);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QAbstractItemModel::ItemFlags Model::flags(const QModelIndex &index) const
{
    ItemFlags foo = QAbstractItemModel::flags(index);
    if (isEditable(index))
        foo |= ItemIsEditable;
    return foo;
}
