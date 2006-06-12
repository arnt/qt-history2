/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "imagedelegate.h"

ImageDelegate::ImageDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *ImageDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem & /* option */,
                                     const QModelIndex &index) const
{
    QComboBox *comboBox = new QComboBox(parent);
    if (index.column() == 1) {
        comboBox->addItem(tr("Normal"));
        comboBox->addItem(tr("Active"));
        comboBox->addItem(tr("Disabled"));
        comboBox->addItem(tr("Selected"));
    } else if (index.column() == 2) {
        comboBox->addItem(tr("Off"));
        comboBox->addItem(tr("On"));
    }

    connect(comboBox, SIGNAL(activated(int)), this, SLOT(emitCommitData()));

    return comboBox;
}

void ImageDelegate::setEditorData(QWidget *editor,
                                  const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox)
        return;

    int pos = comboBox->findText(index.model()->data(index).toString(),
                                 Qt::MatchExactly);
    comboBox->setCurrentIndex(pos);
}

void ImageDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!comboBox)
        return;

    model->setData(index, comboBox->currentText());
}

void ImageDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget *>(sender()));
}
