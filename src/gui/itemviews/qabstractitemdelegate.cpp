#include "qabstractitemdelegate.h"
#include <qabstractitemmodel.h>
#include <qfontmetrics.h>
#include <qstring.h>

#include <private/qabstractitemdelegate_p.h>
#define d d_func()
#define q q_func()

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
  directly. For that event() can be reimplemented.
*/

QAbstractItemDelegate::QAbstractItemDelegate(QAbstractItemModel *model, QObject *parent)
    : QObject(*(new QAbstractItemDelegatePrivate), parent)
{
    d->model = model;
}

QAbstractItemDelegate::QAbstractItemDelegate(QAbstractItemDelegatePrivate &dd, QAbstractItemModel *model,
                                             QObject *parent)
    : QObject(dd, parent)
{
    d->model = model;
}

QAbstractItemDelegate::~QAbstractItemDelegate()
{

}

QAbstractItemModel *QAbstractItemDelegate::model() const
{
    return d->model;
}

QAbstractItemDelegate::EditType QAbstractItemDelegate::editType(const QModelIndex &) const
{
    return QAbstractItemDelegate::NoEditType;
}

QWidget *QAbstractItemDelegate::createEditor(StartEditAction, QWidget *, const QItemOptions&,
                                             const QModelIndex &)
{
    return 0;
}

void QAbstractItemDelegate::removeEditor(EndEditAction, QWidget *, const QModelIndex &)
{
    // do nothing
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

/*!
  Creates a string with ... like "Trollte..." or "...olltech" depending on the alignment
*/

QString QAbstractItemDelegate::ellipsisText(const QFontMetrics &fontMetrics, int width, int align,
                                            const QString &org) const
{
    int ellWidth = fontMetrics.width("...");
    QString text = QString::fromLatin1("");
    int i = 0;
    int len = org.length();
    int offset = (align & Qt::AlignRight) ? (len - 1) - i : i;
    while (i < len && fontMetrics.width(text + org.at(offset)) + ellWidth < width) {
	if (align & Qt::AlignRight)
	    text.prepend(org.at(offset));
	else
	    text.append(org.at(offset));
	offset = (align & Qt::AlignRight) ? (len - 1) - ++i : ++i;
    }
    if (text.isEmpty())
	text = (align & Qt::AlignRight) ? org.right(1) : text = org.left(1);
    if (align & Qt::AlignRight)
	text.prepend("...");
    else
	text.append("...");
    return text;
}
