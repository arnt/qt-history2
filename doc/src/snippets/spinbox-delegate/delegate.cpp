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

/*
    delegate.cpp

    A delegate that allows the user to change integer values from the model
    using a spin box widget.
*/

#include <QtGui>

#include "delegate.h"


SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

/*!
    Returns an editor widget (a spin box) that restricts values from the
    model to integers in the range [0, 100]. We call the standard interface
    functions for this class to ensure that the editor is updated in a
    consistent way.

    If editing is never allowed by the model, we return 0 to indicate that
    no editor widget was created.
*/

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem & /* option */,
    const QModelIndex & /* index */) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(0);
    editor->setMaximum(100);
    editor->installEventFilter(const_cast<SpinBoxDelegate*>(this));

    return editor;
}

/*!
    Reads data from the model, and writes it to the editor widget, assuming
    that it is a spin box.
*/

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index,
        QAbstractItemModel::DisplayRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

/*!
    Reads the contents of the spin box, and writes it to the model.
*/

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value);
}

/*!
    Updates the editor widget's geometry using the information supplied in
    the style option. This is the minimum that the delegate must do in this
    case.
*/

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}
