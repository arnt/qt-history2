#include "qabstractitemdelegate.h"

/*!
  \class QAbstractItemDelegate qabstractitemdelegate.h

  \brief Renders and edits a data item from a model

  This class provides the functionality to render, lay out and offer
  editor functionality for a data item.

  To render an item reimplement paint() and sizeHint().

  An item delegate can offer different ways for editing a data item.

  One way to edit an item is to create a widget on top of the item,
  which allows editing the contents of the item. For that
  createEditor() has to be reimplemented to create the editor widget,
  setContentFromEditor() has to be reimplemented to set the edited
  contents back to the data item and updateEditor() to update the
  editor in case the data of the item changed while being edited.

  The other way is to not create a widget but handle user events
  directly. For that keyPressEvent(), keyReleaseEvent(),
  mousePressEvent(), mouseMoveEvent(), mouseReleaseEvent() and
  mouseDoubleClickEvent() can be reimplemented.
*/

QAbstractItemDelegate::EditType QAbstractItemDelegate::editType(const QModelIndex &item) const
{
    return QAbstractItemDelegate::NoEditType;
}

QWidget *QAbstractItemDelegate::createEditor(StartEditAction, QWidget *,
					     const QItemOptions &, const QModelIndex &) const
{
    return 0;
}

void QAbstractItemDelegate::setContentFromEditor(QWidget *, const QModelIndex &) const
{
    // do nothing
}

void QAbstractItemDelegate::updateEditorContents(QWidget *, const QModelIndex &) const
{
    // do nothing
}

void QAbstractItemDelegate::updateEditorGeometry(QWidget *, const QItemOptions &, const QModelIndex &) const
{
    // do nothing
}
