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

using namespace qdesigner::components::propertyeditor;

QPropertyEditorModel::QPropertyEditorModel(QObject *parent)
    : QAbstractItemModel(parent), m_initialInput(0)
{
}

QPropertyEditorModel::~QPropertyEditorModel()
{
}

void QPropertyEditorModel::setInitialInput(IProperty *initialInput)
{
    Q_ASSERT(initialInput);

    m_initialInput = initialInput;
    reset();
}

QModelIndex QPropertyEditorModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, m_initialInput);

    return createIndex(row, column, childAt(privateData(parent), row));
}

QModelIndex QPropertyEditorModel::parent(const QModelIndex &index) const
{
    if (!index.isValid() || privateData(index) == m_initialInput)
        return QModelIndex();

    Q_ASSERT(privateData(index));

    return indexOf(parentOf(privateData(index)));
}

int QPropertyEditorModel::rowCount(const QModelIndex &parent) const
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

int QPropertyEditorModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
}

bool QPropertyEditorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (IProperty *property = privateData(index)) {
        if (role == Qt::EditRole) {
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

QVariant QPropertyEditorModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!privateData(index))
        return QVariant();

    IProperty *o = privateData(index);
    switch (index.column()) {  // ### cleanup
        case 0:
            switch (role) {
                case Qt::EditRole:
                case Qt::DisplayRole:
                    return o->propertyName().isEmpty()
                        ? QLatin1String("<noname>")
                        : o->propertyName();
                default:
                    break;
            }
            break;

        case 1: {
            switch (role) {
                case Qt::EditRole:
                    return o->value();
                case Qt::DisplayRole:
                    return o->toString();
                case Qt::DecorationRole:
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

QString QPropertyEditorModel::columnText(int col) const
{
    switch (col) {
        case 0: return QLatin1String("Property");
        case 1: return QLatin1String("Value");
        default: return QString();
    }
}



void QPropertyEditorModel::refreshHelper(IProperty *property)
{
    QModelIndex index0 = indexOf(property, 0);
    QModelIndex index1 = indexOf(property, 1);
    emit dataChanged(index0, index1);
}

void QPropertyEditorModel::refresh(IProperty *property)
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

void QPropertyEditorModel::propertyAdded(IProperty *property)
{
    QModelIndex p = parent(indexOf(property));
    int r = rowCount(p);

    emit rowsInserted(p, r - 1, r);
}

void QPropertyEditorModel::propertyRemoved(const QModelIndex &index)
{
    emit rowsAboutToBeRemoved(parent(index), index.row(), index.row() + 1);
}

void QPropertyEditorModel::refresh()
{
    refresh(m_initialInput);
}

bool QPropertyEditorModel::isEditable(const QModelIndex &index) const
{
    return index.column() == 1 && privateData(index)->hasEditor();
}

QModelIndex QPropertyEditorModel::buddy(const QModelIndex &index) const
{
    if (index.column() == 0)
        return createIndex(index.row(), 1, index.internalPointer());
    return index;
}

QVariant QPropertyEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();
        return columnText(section);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags QPropertyEditorModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags foo = QAbstractItemModel::flags(index);
    if (isEditable(index))
        foo |= Qt::ItemIsEditable;
    return foo;
}
