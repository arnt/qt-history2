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

#include "qsqlrelationaldelegate.h"

#include "qlistview.h"
#include "qcombobox.h"
#include "qsqlrelationaltablemodel.h"

QSqlRelationalDelegate::QSqlRelationalDelegate(QObject *parent)
    : QItemDelegate(parent)
{

}

QSqlRelationalDelegate::~QSqlRelationalDelegate()
{
}

QWidget *QSqlRelationalDelegate::editor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index)
{
    const QSqlRelationalTableModel *sqlModel = qt_cast<const QSqlRelationalTableModel *>(index.model());
    if (!sqlModel || !sqlModel->relationModel(index.column()))
        return QItemDelegate::editor(parent, option, index);

    QComboBox *combo = new QComboBox(parent);
//    combo->setModel(sqlModel->relationModel(index.column()));
    combo->installEventFilter(this);

    return combo;
}

void QSqlRelationalDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    const QSqlRelationalTableModel *sqlModel = qt_cast<const QSqlRelationalTableModel *>(index.model());
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    QComboBox *combo = qt_cast<QComboBox *>(editor);
    if (!sqlModel || !childModel || !combo) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }

    int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
    QVariant parentEditValue = sqlModel->data(index, QAbstractItemModel::EditRole);

    combo->clear();
    int currentItem = -1;
    for (int i = 0; i < childModel->rowCount(); ++i) {
        QVariant val = childModel->data(childModel->index(i, childColIndex));
        combo->insertItem(val.toString());
        if (currentItem < 0 && val == parentEditValue)
            currentItem = i;
    }
    if (currentItem >= 0)
        combo->setCurrentItem(currentItem);
}

void QSqlRelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                          const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QSqlRelationalTableModel *sqlModel = qt_cast<QSqlRelationalTableModel *>(model);
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    QComboBox *combo = qt_cast<QComboBox *>(editor);
    if (!sqlModel || !childModel || !combo) {
        QItemDelegate::setModelData(editor, model, index);
        return;
    }

    int currentItem = combo->currentItem();
    int childColIndex = childModel->fieldIndex(sqlModel->relation(
                            index.column()).displayColumn());
    int childEditIndex = childModel->fieldIndex(sqlModel->relation(
                            index.column()).indexColumn());
    QVariant val;
    val = childModel->data(childModel->index(currentItem, childColIndex),
                           QAbstractItemModel::DisplayRole);
    sqlModel->setData(index, QAbstractItemModel::DisplayRole, val);
    val = childModel->data(childModel->index(currentItem, childEditIndex),
                           QAbstractItemModel::EditRole);
    sqlModel->setData(index, QAbstractItemModel::EditRole, val);
}

