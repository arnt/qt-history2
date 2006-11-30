/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpropertyeditor_model_p.h"
#include <QtCore/qdebug.h>

namespace qdesigner_internal {

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

    if (const IProperty *p = privateData(parent)) {
        return (p->kind() == IProperty::Property_Group)
            ? static_cast<const IPropertyGroup*>(p)->propertyCount()
            : 0;
    }

    return (m_initialInput->kind() == IProperty::Property_Group)
        ? static_cast<const IPropertyGroup*>(m_initialInput)->propertyCount()
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

    const IProperty *o = privateData(index);
    switch (index.column()) {  // ### cleanup
        case 0:
            switch (role) {
                case Qt::EditRole:
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
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
                case Qt::ToolTipRole:
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
    
void QPropertyEditorModel::refresh(IProperty *property)
{
    // find parent if it is a fake
    IProperty *parent = property;
    while (parent && parent->isFake())
        parent = parent->parent();
    
    const int parentRow = rowOf(parent);
    if (parentRow == -1)
        return;
    
    const QModelIndex parentIndex0 = createIndex(parentRow, 0, parent);
    const QModelIndex parentIndex1 = createIndex(parentRow, 1, parent);
            
    emit dataChanged(parentIndex0, parentIndex1);
    // refresh children
    if (parent->kind() == IProperty::Property_Group) {
        IPropertyGroup* group =  static_cast<IPropertyGroup*>(parent);
        if (const int numRows = group->propertyCount()) {
            const  QModelIndex leftTopChild = parentIndex0.child(0, 0);
            const  QModelIndex rightBottomChild = parentIndex0.child(numRows - 1, 1);
            emit dataChanged(leftTopChild, rightBottomChild);
        }
    }
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
    Q_ASSERT(index.isValid());

    Qt::ItemFlags foo = QAbstractItemModel::flags(index);

    if (isEditable(index))
        foo |= Qt::ItemIsEditable;

    return foo;
}

int QPropertyEditorModel::rowOf(IProperty *property) const
{
    Q_ASSERT(property);
    if (property == m_initialInput)
        return 0;
    
    IProperty *parent = property->parent();
    
    if (!parent || parent->kind() != IProperty::Property_Group)
        return -1;
    
    return static_cast<const IPropertyGroup*>(parent)->indexOf(property);
}
}
