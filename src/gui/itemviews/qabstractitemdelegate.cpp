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

#include "qabstractitemdelegate.h"
#include <qabstractitemmodel.h>
#include <qfontmetrics.h>
#include <qstring.h>

/*!
    \class QAbstractItemDelegate qabstractitemdelegate.h

    \brief The QAbstractItemDelegate class is used to display and edit
    data items from a model.

    \ingroup model-view

    A QAbstractItemDelegate provides the interface and common functionality
    for delegates in the model/view architecture. Delegates display
    individual items in views, and handle the editing of model data.

    To render an item in a custom way, you must implement paint() and
    sizeHint(). The QItemDelegate class provides default implementations for
    these functions; if you do not need custom rendering, subclass that
    class instead.

    To provide custom editing, there are two approaches that can be
    used. The first approach is to create an editor widget and display
    it directly on top of the item. To do this you must reimplement
    editor() to provide an editor widget, setEditorData() to populate the
    editor with the data from the model setModelData() so that the delegate
    can update the model with data from the editor, and releaseEditor() to
    destroy your editor when it is no longer needed. The second approach is to
    handle user events directly. To do this you could reimplement editorEvent().

    \sa \link model-view-programming.html Model/View Programming\endlink QItemDelegate
*/

/*!
    \enum QAbstractItemDelegate::EndEditHint

    This enum describes the different hints that the delegate can give to the
    model and view components to make editing data in a model a comfortable
    experience for the user.

    \value NoHint           There is no recommended action to be performed.

    These hints let the delegate influence the behavior of the view:

    \value EditNextItem     The view should use the delegate to open an
                            editor on the next item in the view.
    \value EditPreviousItem The view should use the delegate to open an
                            editor on the previous item in the view.

    Note that custom views may interpret the concepts of next and previous
    differently.

    The following hints are most useful when models are used that cache
    data, such as those that manipulate date locally in order to increase
    performance or conserve network bandwidth.

    \value SubmitModelCache If the model caches data, it should write out
                            cached data to the underlying data store.
    \value RevertModelCache If the model caches data, it should discard
                            cached data and replace it with data from the
                            underlying data store.

    Although models and views should respond to these hints in appropriate
    ways, custom components may ignore any or all of them if they are not
    relevant.
*/

/*!
  \fn void QAbstractItemDelegate::commitData(QWidget *editor)

  This signal is emitted when the \a editor widget has completed editing the
  data, and wants to write it back into the model.
  ... ### FIXME ###
*/

/*!
    \fn void QAbstractItemDelegate::closeEditor(QWidget *editor, EndEditHint hint)

    This signal is emitted when the user has finished editing an item using
    the specified \a editor.

    The \a hint provides a way for the delegate to influence how the model and
    view behave after editing is completed. It indicates to these components
    what action should be performed next to provide a comfortable editing
    experience for the user. For example, if \c EditNextItem is specified,
    the view should use a delegate to open an editor on the next item in the
    model.

    \sa EndEditHint
*/

/*!
    Creates a new abstract item delegate with the given \a parent.
*/
QAbstractItemDelegate::QAbstractItemDelegate(QObject *parent)
    : QObject(parent)
{

}

/*!
    \internal

    Creates a new abstract item delegate with the given \a parent.
*/
QAbstractItemDelegate::QAbstractItemDelegate(QObjectPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{

}

/*!
    Destroys the abstract item delegate.
*/
QAbstractItemDelegate::~QAbstractItemDelegate()
{

}

/*!
    \fn void QAbstractItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. Use the \a painter and style \a option to
    render the item specified by the item \a index.

    If you reimplement this you must also reimplement sizeHint().
*/

/*!
    \fn QSize QAbstractItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const = 0

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. The options are specified by \a option
    and the model item by \a index.

    If you reimplement this you must also reimplement paint().
*/

/*!
    Returns the editor to be used for editing the data item at the
    given \a index in the \a model. The editor's parent widget is specified by
    \a parent, and the item options by \a option.
    Ownership is kept by the delegate. Subsequent calls to this
    function with the same arguments are not guaranteed to return
    the same editor object.

    Note: When the editor is no longer in use, call releaseEditor().

    The base implementation returns 0. If you want custom editing you
    will need to reimplement this function.

    \sa setModelData() setEditorData() releaseEditor()
*/
QWidget *QAbstractItemDelegate::editor(QWidget *,
                                       const QStyleOptionViewItem &,
                                       const QModelIndex &)
{
    return 0;
}

/*!
    Notifies the delegate that the given \a editor used to edit \a index
    is no longer in use.
    Typically the delegate should destroy the editor at this point.

    The base implementation does nothing. If you want custom editing
    you will probably need to reimplement this function.

    \sa editor() setEditorData() setModelData()
*/
void QAbstractItemDelegate::releaseEditor(QWidget *,
                                          const QModelIndex &)
{
    // do nothing
}

/*!
    Sets the contents of the given \a editor to the data for the item
    at the given \a index, in the model \a model.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa editor() setModelData() releaseEditor()
*/
void QAbstractItemDelegate::setEditorData(QWidget *,
                                          const QModelIndex &) const
{
    // do nothing
}

/*!
    Sets the data for the item at the given \a index in the \a model
    to the contents of the given \a editor.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa editor() setEditorData() releaseEditor()
*/
void QAbstractItemDelegate::setModelData(QWidget *,
                                         QAbstractItemModel *,
                                         const QModelIndex &) const
{
    // do nothing
}

/*!
    Updates the geometry of the \a editor for the item in the \a model
    with the given \a index, according to the rectangle specified in the \a
    option. If the item has an internal layout, the editor will be
    laid out accordingly.

    The base implementation does nothing. If you want custom editing
    you must reimplement this function.

    \sa editor() releaseEditor()
 */
void QAbstractItemDelegate::updateEditorGeometry(QWidget *,
                                                 const QStyleOptionViewItem &,
                                                 const QModelIndex &) const
{
    // do nothing
}

/*!
    Whenever an event occurs, this function is called with the \a e
    \a option and the model \a index in the \a model.

    The base implementation returns false (indicating that it has not
    handled the event).
*/
bool QAbstractItemDelegate::editorEvent(QEvent *,
                                        const QStyleOptionViewItem &,
                                        const QModelIndex &)
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
