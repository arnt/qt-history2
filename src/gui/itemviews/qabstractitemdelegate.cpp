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

/*!
    \class QAbstractItemDelegate qabstractitemdelegate.h

    \brief The QAbstractItemDelegate class is used to display and edit
    data items from a model.

    \ingroup model-view

    To render an item in a custom way, reimplement paint() and sizeHint().

    To provide custom editing, there are two approaches that can be
    used. The first approach is to create an editor widget and display
    it directly on top of the item. To do this you must reimplement
    editor() and editorType() to provide your editor widget to the
    delegate, setModelData() so that the delegate can update the
    item's content from the editor, setEditorData() to populate the
    editor with the item's initial value, or when the item's value is
    changed elsewhere, and releaseEditor() to destroy your editor when
    it is no longer needed. The second approach is to handle user
    events directly. To do this you could reimplement event().

    \sa \link model-view-programming.html Model/View Programming\endlink.
*/

/*!
    \enum QAbstractItemDelegate::EditorType

    This enum describes what must be done to create an editor for an
    item.

    \value Widget The editor() is a widget.
    \value Events There is no editor() widget; used if event() is
    reimplemented.
*/

/*!
    \enum QAbstractItemDelegate::BeginEditAction

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
QAbstractItemDelegate::QAbstractItemDelegate(QObject *parent)
    : QObject(parent)
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
    provide custom rendering. The painter to paint on is given by \a
    painter, the options by \a option, and the item by \a index.

    If you reimplement this you must also reimplement sizeHint().
*/

/*!
    \fn QSize QAbstractItemDelegate::sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0

    This pure abstract function must be reimplemented if you want to
    provide custom rendering. The font metrics are given by \a
    fontMetrics, the options by \a option, and the item by \a index.

    If you reimplement this you must also reimplement paint().
*/

/*!
    Returns the \c EditorType for the item at \a index.

    If you provide an editor(), you will want to reimplement this
    function to return the \c EditorType appropriate to your editor().
*/
QAbstractItemDelegate::EditorType QAbstractItemDelegate::editorType(const QAbstractItemModel *,
                                                                    const QModelIndex &) const
{
    return QAbstractItemDelegate::Events;
}

/*!
    Returns the editor to be used for editing the data item at index
    \a index in the model \a model. The action that caused the edit is
    given by \a action; see \c BeginEditAction. The editor's parent
    widget is given by \a parent, and the item options by \a option.
    Ownership is kept by the delegate. Subsequent calls to this
    function with the same arguments are not guaranteed to return
    the same editor object.

    Note: When the editor is no longer in use, call releaseEditor().

    The base implementation returns 0. If you want custom editing you
    will need to reimplement this function.

    \sa editorType() setModelData() setEditorData() releaseEditor()
*/
QWidget *QAbstractItemDelegate::editor(BeginEditAction, QWidget *, const QStyleOptionViewItem &,
                                       const QAbstractItemModel *, const QModelIndex &)
{
    return 0;
}

/*!
    Notifies the delegate that the given \a editor is no longer in use
    for the item at the given \a index in the model \a model.
    The way the edit was completed is given by \a action;
    see \c EndEditAction. Typically the delegate should destroy the
    editor at this point.

    The base implementation does nothing. If you want custom editing
    you will probably need to reimplement this function.

    \sa editorType() editor() setEditorData() setModelData()
*/
void QAbstractItemDelegate::releaseEditor(EndEditAction, QWidget *,
                                          QAbstractItemModel *, const QModelIndex &)
{
    // do nothing
}

/*!
    Sets the contents of the given \a editor to the data for the item
    at the given \a index, in the model \a model.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa editorType() editor() setModelData() releaseEditor()
*/
void QAbstractItemDelegate::setEditorData(QWidget *,
                                          const QAbstractItemModel *,
                                          const QModelIndex &) const
{
    // do nothing
}

/*!
    Sets the data for the item at the given \a index in model \a model
    to the contents of the given \a editor.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa editorType() editor() setEditorData() releaseEditor()
*/
void QAbstractItemDelegate::setModelData(QWidget *,
                                         QAbstractItemModel *,
                                         const QModelIndex &) const
{
    // do nothing
}

/*!
  Updates the editor geometry of the ginen \a editor for the item at the
  given \a index in the \a model, according to the rectangle specified in the \a option.
  If the item has an internal layout, the editor will be layed out accordingly.

  The base implementation dowes nothing. If you want custom editing
    you will need to reimplement this function.
  
  \sa editorType() editor() releaseEditor()
 */
void QAbstractItemDelegate::updateEditorGeometry(QWidget *,
                                                 const QStyleOptionViewItem &,
                                                 const QAbstractItemModel *,
                                                 const QModelIndex &) const
{
    // do nothing
}

/*!
    Whenever an event occurs this function is called with the \a e
    and the model \a index in the \a model.

    The base implementation returns false (indicating that it has not
    handled the event). If you reimplement this you should reimplement
    editorType() to return \c NoWidget.
*/
bool QAbstractItemDelegate::event(QEvent *, QAbstractItemModel *, const QModelIndex &)
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
