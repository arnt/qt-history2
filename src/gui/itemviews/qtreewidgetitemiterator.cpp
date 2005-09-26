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

#include "qtreewidgetitemiterator.h"
#include "qtreewidget.h"
#include "qtreewidget_p.h"

#ifndef QT_NO_TREEWIDGET

/*!
    Constructs an iterator for the same QTreeWidget as \a it. The
    current iterator item is set to point on the current item of \a it.
*/

QTreeWidgetItemIterator::QTreeWidgetItemIterator(const QTreeWidgetItemIterator &it)
    : model(it.model), current(it.current), flags(it.flags)
{
    Q_ASSERT(model);
    model->iterators.append(this);
}

/*!
    Constructs an iterator for the QTreeWidget \a widget. The current
    iterator item is set to point on the first toplevel item (QTreeWidgetItem)
    of \a widget.
*/

QTreeWidgetItemIterator::QTreeWidgetItemIterator(QTreeWidget *widget, IteratorFlags flags)
    : model(0), current(0), flags(flags)
{
    Q_ASSERT(widget);
    model = qobject_cast<QTreeModel*>(widget->model());
    Q_ASSERT(model);
    model->iterators.append(this);
    current = model->tree.first();
    if (current && !matchesFlags(current))
        ++(*this);
}

/*!
    Constructs an iterator for the QTreeWidget that contains the \a item
    using the flags \a flags. The current iterator item is set
    to point to \a item or the next matching item if \a item doesn't
    match the flags.

    \sa QTreeWidgetItemIterator::IteratorFlag
*/

QTreeWidgetItemIterator::QTreeWidgetItemIterator(QTreeWidgetItem *item, IteratorFlags flags)
    : model(0), current(item), flags(flags)
{
    Q_ASSERT(item);
    model = item->model;
    Q_ASSERT(model);
    model->iterators.append(this);
    if (current && !matchesFlags(current))
        ++(*this);
}

/*!
    Destroys the iterator.
*/

QTreeWidgetItemIterator::~QTreeWidgetItemIterator()
{
    model->iterators.removeAll(this);
}

/*!
    Assignment. Makes a copy of \a it and returns a reference to its
    iterator.
*/

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator=(const QTreeWidgetItemIterator &it)
{
    if (model != it.model) {
        model->iterators.removeAll(this);
        it.model->iterators.append(this);
    }
    current = it.current;
    flags = it.flags;
    model = it.model;
    return *this;
}

/*!
    Preincrement. Makes the next item the new current item and returns
    a reference to the iterator.
    Sets the current pointer to 0 if the current item is the last item.
*/

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator++()
{
    if (current)
        do {
            current = QTreeModel::walk(current);
        } while (current && !matchesFlags(current));
    return *this;
}

/*!
    Predecrement. Makes the previous item the new current item and
    returns a reference to the iterator.
    Sets the current pointer to 0 if the current item is the first item.
*/

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator--()
{
    if (current)
        do {
            QTreeWidgetItem *item = QTreeModel::previousSibling(current);
            if (item) {
                while (!item->children.isEmpty())
                    item = item->children.last();
                current = item;
            } else {
                current = current->parent();
            }
        } while (current && !matchesFlags(current));
    return *this;
}

/*!
  \internal
*/
bool QTreeWidgetItemIterator::matchesFlags(const QTreeWidgetItem *item) const
{
    if (!item)
        return false;

    if (flags == All)
        return true;

    {
        Qt::ItemFlags itemFlags = item->flags();
        if ((flags & Selectable) && !(itemFlags & Qt::ItemIsSelectable))
            return false;
        if ((flags & NotSelectable) && (itemFlags & Qt::ItemIsSelectable))
            return false;
        if ((flags & DragEnabled) && !(itemFlags & Qt::ItemIsDragEnabled))
            return false;
        if ((flags & DragDisabled) && (itemFlags & Qt::ItemIsDragEnabled))
            return false;
        if ((flags & DropEnabled) && !(itemFlags & Qt::ItemIsDropEnabled))
            return false;
        if ((flags & DropDisabled) && (itemFlags & Qt::ItemIsDropEnabled))
            return false;
        if ((flags & Enabled) && !(itemFlags & Qt::ItemIsEnabled))
            return false;
        if ((flags & Disabled) && (itemFlags & Qt::ItemIsEnabled))
            return false;
        if ((flags & Editable) && !(itemFlags & Qt::ItemIsEditable))
            return false;
        if ((flags & NotEditable) && (itemFlags & Qt::ItemIsEditable))
            return false;
    }

    {
        Qt::CheckState check = item->checkState(0); //###FIXME
        // Not sure why the FIXME is here,
        // but we (jasaethe && mariusbm) decided that Qt::PartiallyChecked should match as Checked.
        if ((flags & Checked) && (check == Qt::Unchecked))
            return false;
        if ((flags & NotChecked) && (check != Qt::Unchecked))
            return false;
    }

    if ((flags & HasChildren) && !item->childCount())
        return false;
    if ((flags & NoChildren) && item->childCount())
        return false;

    {
        QTreeWidget *widget = item->view;
        Q_ASSERT(widget);

        bool hidden = widget->isItemHidden(item);
        if ((flags & Hidden) && !hidden)
            return false;
        if ((flags & NotHidden) && hidden)
            return false;

        bool selected = widget->isItemSelected(item);
        if ((flags & Selected) && !selected)
            return false;
        if ((flags & Unselected) && selected)
            return false;
    }

    return true;
}

/*!
  \fn const QTreeWidgetItemIterator operator++(int)

  Postincrement. Makes the next item the new current item and returns
  the iterator for the item that \e was the current item.
*/

/*!
  \fn QTreeWidgetItemIterator &operator+=(int n)

  Sets the current item to the item \a n positions after the current
  item. If that item is beyond the last item, the current item pointer is
  set to 0. Returns the iterator for the current item.
*/

/*!
  \fn const QTreeWidgetItemIterator operator--(int)

  Postdecrement. Makes the previous item the new current item and
  returns the iterator for the item that \e was the current item.
*/

/*!
  \fn QTreeWidgetItemIterator &operator-=(int n)

  Sets the current item to the item \a n positions before the
  current item. If that item is before the first item, the current
  item is set to 0. Returns the a reference to the iterator.
*/

/*!
  \fn QTreeWidgetItem *operator*() const

  Dereference operator. Returns a pointer to the current item.
*/


/*!
    \enum QTreeWidgetItemIterator::IteratorFlag

    These flags can be passed to a QTreeWidgetItemIterator constructor
    (OR-ed together if more than one is used), so that the iterator
    will only iterate over items that match the given flags.

    \value All
    \value Hidden
    \value NotHidden
    \value Selected
    \value Unselected
    \value Selectable
    \value NotSelectable
    \value DragEnabled
    \value DragDisabled
    \value DropEnabled
    \value DropDisabled
    \value HasChildren
    \value NoChildren
    \value Checked
    \value NotChecked
    \value Enabled
    \value Disabled
    \value Editable
    \value NotEditable
    \value UserFlag
*/
#endif // QT_NO_TREEWIDGET
