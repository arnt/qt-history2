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

#ifndef QSQLRELATIONALDELEGATE_H
#define QSQLRELATIONALDELEGATE_H

#include "QtGui/qitemdelegate.h"
#include "QtGui/qlistview.h"
#include "QtGui/qcombobox.h"
#include "QtSql/qsqlrelationaltablemodel.h"

class Q_SQL_EXPORT QSqlRelationalDelegate: public QItemDelegate
{
public:

explicit QSqlRelationalDelegate(QObject *parent)
    : QItemDelegate(parent)
{}

~QSqlRelationalDelegate()
{}

QWidget *createEditor(QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    const QSqlRelationalTableModel *sqlModel = qt_cast<const QSqlRelationalTableModel *>(index.model());
    if (!sqlModel || !sqlModel->relationModel(index.column()))
        return QItemDelegate::createEditor(parent, option, index);

    QComboBox *combo = new QComboBox(parent);
//    combo->setModel(sqlModel->relationModel(index.column()));
    combo->installEventFilter(const_cast<QSqlRelationalDelegate *>(this));

    return combo;
}

void setEditorData(QWidget *editor, const QModelIndex &index) const
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
        combo->addItem(val.toString());
        if (currentItem < 0 && val == parentEditValue)
            currentItem = i;
    }
    if (currentItem >= 0)
        combo->setCurrentIndex(currentItem);
}

void setModelData(QWidget *editor, QAbstractItemModel *model,
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

    int currentItem = combo->currentIndex();
    int childColIndex = childModel->fieldIndex(sqlModel->relation(
                            index.column()).displayColumn());
    int childEditIndex = childModel->fieldIndex(sqlModel->relation(
                            index.column()).indexColumn());
    QVariant val;
    val = childModel->data(childModel->index(currentItem, childColIndex),
                           QAbstractItemModel::DisplayRole);
    sqlModel->setData(index, val, QAbstractItemModel::DisplayRole);
    val = childModel->data(childModel->index(currentItem, childEditIndex),
                           QAbstractItemModel::EditRole);
    sqlModel->setData(index, val, QAbstractItemModel::EditRole);
}

};

#endif // QSQLRELATIONALDELEGATE_H
