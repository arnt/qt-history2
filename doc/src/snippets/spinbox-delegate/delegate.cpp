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
    spinBox = 0;
}

/*!
    Returns an editor widget (a spin box) that restricts values from the
    model to integers in the range [0, 100]. We call the standard interface
    functions for this class to ensure that the editor is updated in a
    consistent way.

    If editing is never allowed by the model, we return 0 to indicate that
    no editor widget was created.
*/

QWidget *SpinBoxDelegate::editor(QWidget *parent,
    const QStyleOptionViewItem & /* option */,
    const QAbstractItemModel * /* model */,
    const QModelIndex & /* index */)
{
    if (spinBox)
        cancelEditing(spinBox);

    spinBox = new QSpinBox(parent);
    spinBox->setMinimum(0);
    spinBox->setMaximum(100);
    spinBox->installEventFilter(this);

    return spinBox;
}

/*!
    Releases the editor. This involves writing the contents of the editor
    widget to the model before destroying it.
*/

void SpinBoxDelegate::releaseEditor(QWidget *editor)
{
    delete editor;
}

/*!
    Reads data from the model, and writes it to the editor widget, assuming
    that it is a spin box.
*/

void SpinBoxDelegate::setEditorData(QWidget *editor,
    const QAbstractItemModel *model, const QModelIndex &index) const
{
    int value = model->data(index, QAbstractItemModel::DisplayRole).toInt();

    static_cast<QSpinBox *>(editor)->setValue(value);
}

/*!
    Reads the contents of the spin box, and writes it to the model.
*/

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    int value = static_cast<QSpinBox *>(editor)->value();

// ### Remove this before TP2:
    value = static_cast<QSpinBox *>(editor)->cleanText().toInt();

    model->setData(index, QAbstractItemModel::EditRole, value);
}

/*!
    Updates the editor widget's geometry using the information supplied in
    the style option. This is the minimum that the delegate must do in this
    case.
*/

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QAbstractItemModel* /* model */,
    const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}

void SpinBoxDelegate::acceptEditing(QWidget *editor)
{
    emit commitData(editor);
    emit doneEditing(editor);
}

void SpinBoxDelegate::cancelEditing(QWidget *editor)
{
    emit doneEditing(editor);
}

bool SpinBoxDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (object == spinBox) {
    
        if (event->type() == QEvent::KeyPress) {

            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            switch (keyEvent->key()) {
                case Qt::Key_Return:
                    acceptEditing(spinBox);
                    return true;

                case Qt::Key_Escape:
                    cancelEditing(spinBox);
                    return true;

                default:
                    return false;
            }
        }
        else
            return false;
    }

    return QItemDelegate::eventFilter(object, event);
}
