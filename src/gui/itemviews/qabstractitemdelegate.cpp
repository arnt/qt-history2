/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractitemdelegate.h"
#include <qabstractitemmodel.h>
#include <qfontmetrics.h>
#include <qstring.h>

#include <private/qabstractitemdelegate_p.h>
#define d d_func()
#define q q_func()

/*!
    \class QAbstractItemDelegate qabstractitemdelegate.h

    \brief The QAbstractItemDelegate class is used to display and edit
    data items from a model.

    \ingroup model-view

    To render an item in a custom way, reimplement paint() and sizeHint().

    To provide custom editing, there are two approaches that can be
    used. The first approach is to create an editor widget and display
    it directly on top of the item. To do this you must reimplement
    editor() and editType() to provide your editor widget to the
    delegate, setModelData() so that the delegate can update the
    item's content from the editor, setEditorData() to populate the
    editor with the item's initial value, or when the item's value is
    changed elsewhere, and releaseEditor() to destroy your editor when
    it is no longer needed. The second approach is to handle user
    events directly. To do this you could reimplement event().

*/

/*!
    \enum QAbstractItemDelegate::EditType

    This enum describes what must be done to create an editor for an
    item.

    \value NoEditType The default if there is no editor().
    \value PersistentWidget The editor() is always in place.
    \value WidgetOnTyping The editor() appears as soon as the user
    begins typing.
    \value WidgetWhenCurrent The editor() appears as soon as the focus
    moves to the item.
    \value NoWidget There is no editor() widget; usually used if
    event() is reimplemented.
*/

/*!
    \enum QAbstractItemDelegate::StartEditAction

    This enum describes how an editor was invoked to edit an item.

    \value NeverEdit The item cannot be edited.
    \value CurrentChanged The focus was moved to the item.
    \value DoubleClicked The item was double-clicked.
    \value SelectedClicked The item was select-clicked (e.g.
    Shift+Click, or Ctrl+Click).
    \value EditKeyPressed An edit key was pressed (often F2).
    \value AnyKeyPressed A key was pressed when the item had the
    focus.
    \value AlwaysEdit The editor is always present.
*/

/*!
    \enum QAbstractItemDelegate::EndEditAction

    \value Accepted The user accepted their edit. (Usually signified
    by pressing Enter.)
    \value Cancelled The user rejected their edit. (Usually signified
    by pressing Esc.)
*/

/*!
    Creates a new abstract item delegate for the given \a model and
    with the parent \a parent.
*/
QAbstractItemDelegate::QAbstractItemDelegate(QAbstractItemModel *model, QObject *parent)
    : QObject(*(new QAbstractItemDelegatePrivate), parent)
{
    d->model = model;
}

/*!
    \internal
*/
QAbstractItemDelegate::QAbstractItemDelegate(QAbstractItemDelegatePrivate &dd,
                                             QAbstractItemModel *model, QObject *parent)
    : QObject(dd, parent)
{
    d->model = model;
}

/*!
    Destroys the abstract item delegate.
*/
QAbstractItemDelegate::~QAbstractItemDelegate()
{

}

/*!
    \fn void QAbstractItemDelegate::paint(QPainter *painter, const QItemOptions &options, const QModelIndex &index) const = 0;

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. The painter to paint on is given by \a
    painter, the options by \a options, and the item by \a index.

    If you reimplement this you must also reimplement sizeHint().
*/

/*!
    \fn QSize QAbstractItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options, const QModelIndex &index) const = 0

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. The font metrics are given by \a
    fontMetrics, the options by \a options, and the item by \a index.

    If you reimplement this you must also reimplement paint().
*/

/*!
    Returns the model that this abstract item delegate is associated
    with.
*/
QAbstractItemModel *QAbstractItemDelegate::model() const
{
    return d->model;
}

/*!
    Returns the \c EditType for the item at \a index.

    If you provide an editor(), you will want to reimplement this
    function to return the \c EditType appropriate to your editor().
*/
QAbstractItemDelegate::EditType QAbstractItemDelegate::editType(const QModelIndex &) const
{
    return QAbstractItemDelegate::NoEditType;
}

/*!
    Returns the editor to be used for editing the data item at index
    \a index. The action that caused the edit is given by \a action;
    see \c StartEditAction. The editor's parent widget is given by \a
    parent, and the item options by \a options. Ownership is kept by
    the delegate. Subsequent calls to this function with the same
    arguments are not guaranteed to return the same editor object.

    Note: When the editor is no longer in use, call releaseEditor().

    The base implementation returns 0. If you want custom editing you
    will need to reimplement this function.

    \sa editType() setModelData() setEditorData() releaseEditor()
*/
QWidget *QAbstractItemDelegate::editor(StartEditAction, QWidget *,
        const QItemOptions&, const QModelIndex &)
{
    return 0;
}

/*!
    Sets the data for the item at the given \a index to the contents
    of the given \a editor.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa editType() editor() setEditorData() releaseEditor()
*/
void QAbstractItemDelegate::setModelData(QWidget *, const QModelIndex &) const
{
    // do nothing
}

/*!
    Sets the contents of the given \a editor to the data for the item
    at the given \a index.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa editType() editor() setModelData() releaseEditor()
*/
void QAbstractItemDelegate::setEditorData(QWidget *, const QModelIndex &) const
{
    // do nothing
}


// ### DOC: can't guess!
void QAbstractItemDelegate::updateEditorGeometry(QWidget *, const QItemOptions &,
                                                 const QModelIndex &) const
{
    // do nothing
}

/*!
    Notifies the delegate that the given \a editor is no longer in use
    for the item at the given \a index. The way the edit was completed
    is given by \a action; see \c EndEditAction. Typically the
    delegate should destroy the editor at this point.

    The base implementation does nothing. If you want custom editing
    you will probably need to reimplement this function.

    \sa editType() editor() setEditorData() setModelData()
*/
void QAbstractItemDelegate::releaseEditor(EndEditAction, QWidget *, const QModelIndex &)
{
    // do nothing
}

/*!
    Whenever an event occurs this function is called with the \a e
    and the model \a index.

    The base implementation returns false (indicating that it has not
    handled the event). If you reimplement this you should reimplement
    editType() to return \c NoWidget.
*/
bool QAbstractItemDelegate::event(QEvent *, const QModelIndex &)
{
    // do nothing
    return false;
}

/*!
    Creates a string with an ellipses ("..."), for example,
    "Trollte..." or "...olltech" depending on the alignment. This is
    used to display items that are too wide to fit. The font metrics
    to be used are given by \a fontMetrics, the available width by \a
    width, the alignment by \a align, and the string by \a org.
*/
QString QAbstractItemDelegate::ellipsisText(const QFontMetrics &fontMetrics, int width, int align,
                                            const QString &org) const
{
    static QString ell("...");
    int ellWidth = fontMetrics.width(ell);
    QString text;
    int i = 0;
    int len = org.length();
    int offset = (align & Qt::AlignRight) ? len - 1 : 0;

    // FIXME: slow !!!
    while (i < len && fontMetrics.width(text + org.at(offset)) + ellWidth < width) {
	if (align & Qt::AlignRight) {
	    text.prepend(org.at(offset));
            offset = (len - 1) - ++i;
	} else {
	    text.append(org.at(offset));
            offset = ++i;
        }
    }
 
    if (align & Qt::AlignRight) {
        if (text.isEmpty())
            text = org.right(1);
        text.prepend(ell);
    } else {
        if (text.isEmpty())
            text = org.left(1);
	text.append(ell);
    }
    return text;
}
