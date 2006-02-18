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
#include "qwidgetitemdata_p.h"
#include <QtCore/QStack>

#ifndef QT_NO_TREEWIDGET

/*!
  \class QTreeWidgetItemIterator
  \ingroup model-view
  \brief The QTreeWidgetItemIterator class provides a way to iterate over the 
  items in a QTreeWidget instance.

  The iterator will walk the items in a pre-order traversal order, thus visiting the parent
  node \e before it continues to the child nodes.

  It is possible to filter out certain types of node by passing certain
  \l{IteratorFlag}{flags} to the constructor of QTreeWidgetItemIterator.


  \sa QTreeWidget, {Model/View Programming}, QTreeWidgetItem
*/

class QTreeWidgetItemIteratorPrivate {
    friend class QTreeWidgetItemIterator;
public:
    QTreeWidgetItemIteratorPrivate(QTreeModel *model)
        : m_currentIndex(0), m_model(model)
    {

    }

    QTreeWidgetItemIteratorPrivate(const QTreeWidgetItemIteratorPrivate& other)
        : m_currentIndex(other.m_currentIndex), m_model(other.m_model), m_parentIndex(other.m_parentIndex)
    {

    }

    QTreeWidgetItemIteratorPrivate &operator=(const QTreeWidgetItemIteratorPrivate& other)
    {
        m_currentIndex = other.m_currentIndex;
        m_parentIndex = other.m_parentIndex;
        m_model = other.m_model;
        return (*this);
    }

    ~QTreeWidgetItemIteratorPrivate()
    {

    }

    QTreeWidgetItem *next(const QTreeWidgetItem *current);
    QTreeWidgetItem *previous(const QTreeWidgetItem *current);
private:
    int             m_currentIndex;
    QTreeModel     *m_model;    // This class should not have ownership of the model.
    QStack<int>     m_parentIndex;
};

QTreeWidgetItem *QTreeWidgetItemIteratorPrivate::next(const QTreeWidgetItem *current)
{
    if (!current) return 0;

    QTreeWidgetItem *next = 0;
    if (current->childCount()) {
        // walk the child
        m_parentIndex.push(m_currentIndex);
        m_currentIndex = 0;
        next = current->child(0);
    } else {
        // walk the sibling
        QTreeWidgetItem *parent = current->parent();
        next = parent ? parent->child(m_currentIndex + 1) : m_model->tree.value(m_currentIndex + 1);
        while (!next && parent) {
            // if we had no sibling walk up the parent and try the sibling of that
            parent = parent->parent();
            m_currentIndex = m_parentIndex.pop();
            next = parent ? parent->child(m_currentIndex + 1) : m_model->tree.value(m_currentIndex + 1);
        }
        if (next) ++(m_currentIndex);
    }
    return next;
}

QTreeWidgetItem *QTreeWidgetItemIteratorPrivate::previous(const QTreeWidgetItem *current)
{
    if (!current) return 0;

    QTreeWidgetItem *prev = 0;
    // walk the previous sibling
    QTreeWidgetItem *parent = current->parent();
    prev = parent ? parent->child(m_currentIndex - 1) : m_model->tree.value(m_currentIndex - 1);
    if (prev) {
        // Yes, we had a previous sibling but we need go down to the last leafnode.
        --m_currentIndex;
        while (prev && prev->childCount()) {
            m_parentIndex.push(m_currentIndex);
            m_currentIndex = prev->childCount() - 1;
            prev = prev->child(m_currentIndex);
        }
    } else if (parent) {
        m_currentIndex = m_parentIndex.pop();
        prev = parent;
    }
    return prev;
}


/*!
    Constructs an iterator for the same QTreeWidget as \a it. The
    current iterator item is set to point on the current item of \a it.
*/

QTreeWidgetItemIterator::QTreeWidgetItemIterator(const QTreeWidgetItemIterator &it)
    :  d_ptr(new QTreeWidgetItemIteratorPrivate(*(it.d_ptr))),
    current(it.current), flags(it.flags)
{
    Q_D(QTreeWidgetItemIterator);
    Q_ASSERT(d->m_model);
    d->m_model->iterators.append(this);
}

/*!
    Constructs an iterator for the QTreeWidget \a widget. 
    The iterator is set to point to the first toplevel 
    item (QTreeWidgetItem) of \a widget or the next matching item if 
    the toplevel item doesn't match the flags.

    \sa QTreeWidgetItemIterator::IteratorFlag
*/

QTreeWidgetItemIterator::QTreeWidgetItemIterator(QTreeWidget *widget, IteratorFlags flags)
: current(0), flags(flags)
{
    Q_ASSERT(widget);
    QTreeModel *model = qobject_cast<QTreeModel*>(widget->model());
    Q_ASSERT(model);
    d_ptr = new QTreeWidgetItemIteratorPrivate(model);
    model->iterators.append(this);
    current = model->tree.first();
    if (current && !matchesFlags(current))
        ++(*this);
}

/*!
    Constructs an iterator for the QTreeWidget that contains the \a item
    using the flags \a flags. The iterator is set
    to point to \a item or the next matching item if \a item doesn't
    match the flags.

    \sa QTreeWidgetItemIterator::IteratorFlag
*/

QTreeWidgetItemIterator::QTreeWidgetItemIterator(QTreeWidgetItem *item, IteratorFlags flags)
    : d_ptr(new QTreeWidgetItemIteratorPrivate(::qobject_cast<QTreeModel*>(item->view->model()))), current(item), flags(flags)
{
    Q_D(QTreeWidgetItemIterator);
    Q_ASSERT(item);
    QTreeModel *model = ::qobject_cast<QTreeModel*>(item->view->model());
    Q_ASSERT(model);
    model->iterators.append(this);


    // Initialize m_currentIndex and m_parentIndex as it would be if we had traversed from
    // the beginning.
    QTreeWidgetItem *parent = item;
    parent = parent->parent();
    QList<QTreeWidgetItem *> children = parent ? parent->children : d->m_model->tree;
    d->m_currentIndex = children.indexOf(item);

    while (parent) {
        parent = parent->parent();
        QList<QTreeWidgetItem *> children = parent ? parent->children : d->m_model->tree;
        int index = children.indexOf(item);
        d->m_parentIndex.prepend(index);
    }

    if (current && !matchesFlags(current))
        ++(*this);
}

/*!
    Destroys the iterator.
*/

QTreeWidgetItemIterator::~QTreeWidgetItemIterator()
{
    d_func()->m_model->iterators.removeAll(this);
    delete d_ptr;
}

/*!
    Assignment. Makes a copy of \a it and returns a reference to its
    iterator.
*/

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator=(const QTreeWidgetItemIterator &it)
{
    Q_D(QTreeWidgetItemIterator);
    if (d_func()->m_model != it.d_func()->m_model) {
        d_func()->m_model->iterators.removeAll(this);
        it.d_func()->m_model->iterators.append(this);
    }
    current = it.current;
    flags = it.flags;
    d->operator=(*it.d_func());
    return *this;
}

/*!
    The prefix ++ operator (++it) advances the iterator to the next matching item 
    and returns a reference to the resulting iterator.
    Sets the current pointer to 0 if the current item is the last matching item.
*/

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator++()
{
    if (current)
        do {
            current = d_func()->next(current);
        } while (current && !matchesFlags(current));
    return *this;
}

/*!
    The prefix -- operator (--it) advances the iterator to the previous matching item 
    and returns a reference to the resulting iterator.
    Sets the current pointer to 0 if the current item is the first matching item.
*/

QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator--()
{
    if (current)
        do {
            current = d_func()->previous(current);
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
        // ### We only test the check state for column 0
        Qt::CheckState check = item->checkState(0);
        // PartiallyChecked matches as Checked.
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
  \fn const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator++(int)

  The postfix ++ operator (it++) advances the iterator to the next matching item 
  and returns an iterator to the previously current item.
*/

/*!
  \fn QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator+=(int n)

  Makes the iterator go forward by n matching items. (If n is negative, the iterator goes backward.)

  If the current item is beyond the last item, the current item pointer is
  set to 0. Returns the resulting iterator.
*/

/*!
  \fn const QTreeWidgetItemIterator QTreeWidgetItemIterator::operator--(int)

  The postfix -- operator (it--) makes the preceding matching item current and returns an iterator to the previously current item.
*/

/*!
  \fn QTreeWidgetItemIterator &QTreeWidgetItemIterator::operator-=(int n)

  Makes the iterator go back by n matching items. (If n is negative, the iterator goes forward.)

  If the current item is ahead of the last item, the current item pointer is
  set to 0. Returns the resulting iterator.
*/

/*!
  \fn QTreeWidgetItem *QTreeWidgetItemIterator::operator*() const

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
