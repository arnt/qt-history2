/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "qtextlist.h"
#include "qtextobject_p.h"
#include "qtextcursor.h"
#include "qtextdocument_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QTextListPrivate : public QTextBlockGroupPrivate
{
};

/*!
    \class QTextList qtextlist.h
    \brief The QTextList class provides a decorated list of items in a QTextDocument.

    \ingroup text

    A list contains a sequence of text blocks, each of which is marked with a
    bullet point or other symbol. Multiple levels of lists can be used, and
    the automatic numbering feature provides support for ordered numeric and
    alphabetical lists.

    Lists are created by using a text cursor to insert an empty list at the
    current position or by moving existing text into a new list.
    The \l{QTextCursor::insertList()} function inserts an empty block into the
    document at the cursor position, and makes it the first item in a list.

    \quotefromfile snippets/textdocument-lists/mainwindow.cpp
    \skipto QTextListFormat listFormat
    \printuntil insertList

    The \l{QTextCursor::createList()} function takes the contents of the
    cursor's current block and turns it into the first item of a new list.

    The cursor's current list is found with \l{QTextCursor::currentList()}.

    The number of items in a list is given by count(). Each item can be
    obtained by its index in the list with the item() function. Similarly,
    the index of a given item can be found with itemNumber(). The text of
    each item can be found with the itemText() function.

    Note that the items in the list may not be adjacent elements in the
    document. For example, the top-level items in a multi-level list will
    be separated by the items in lower levels of the list.

    List items can be deleted by index with the removeItem() function.
    remove() deletes the specified item in the list.

    The list's format is set with setFormat() and read with format().
    The format describes the decoration of the list itself, and not the
    individual items.

    \sa QTextBlock, QTextListFormat, QTextCursor
*/

/*!
    \fn bool QTextList::isEmpty() const
    \obsolete

    Returns true if the list has no items; otherwise returns false.

    \bold{Note:} Empty lists are automatically deleted by the QTextDocument that owns
    them.

    \sa count()
*/

/*! \internal
 */
QTextList::QTextList(QTextDocument *doc)
    : QTextBlockGroup(*new QTextListPrivate, doc)
{
}

/*!
  \internal
*/
QTextList::~QTextList()
{
}

/*!
    Returns the number of items in the list.

    \sa isEmpty()
*/
int QTextList::count() const
{
    Q_D(const QTextList);
    return d->blocks.count();
}

/*!
    Returns the \a{i}-th text block in the list.

    \sa count() itemText()
*/
QTextBlock QTextList::item(int i) const
{
    Q_D(const QTextList);
    if (i < 0 || i >= d->blocks.size())
        return QTextBlock();
    return d->blocks.at(i);
}

/*!
    \fn void QTextList::setFormat(const QTextListFormat &format)

    Sets the list's format to \a format.
*/

/*!
    \fn QTextListFormat QTextList::format() const

    Returns the list's format.
*/

/*!
    \fn int QTextList::itemNumber(const QTextBlock &block) const

    Returns the index of the list item that corresponds to the given \a block.
*/
int QTextList::itemNumber(const QTextBlock &blockIt) const
{
    Q_D(const QTextList);
    return d->blocks.indexOf(blockIt);
}

/*!
    \fn QString QTextList::itemText(const QTextBlock &block) const

    Returns the text of the list item that corresponds to the given \a block.
*/
QString QTextList::itemText(const QTextBlock &blockIt) const
{
    Q_D(const QTextList);
    int item = d->blocks.indexOf(blockIt) + 1;
    if (item <= 0)
        return QString();

    QTextBlock block = d->blocks.at(item-1);
    QTextBlockFormat blockFormat = block.blockFormat();

    QString result;

    const int style = format().style();

    switch (style) {
        case QTextListFormat::ListDecimal:
            result = QString::number(item);
            break;
            // from the old richtext
        case QTextListFormat::ListLowerAlpha:
        case QTextListFormat::ListUpperAlpha:
            {
                const char baseChar = style == QTextListFormat::ListUpperAlpha ? 'A' : 'a';

                int c = item;
                while (c > 0) {
                    c--;
                    result.prepend(QChar(baseChar + (c % 26)));
                    c /= 26;
                }
            }
            break;
        default:
            Q_ASSERT(false);
    }
    if (blockFormat.layoutDirection() == Qt::RightToLeft)
        return result.prepend(QLatin1Char('.'));
    return result + QLatin1Char('.');
}

/*!
    Removes the item at item position \a i from the list. When the last item in the
    list is removed, the list is automatically deleted by the QTextDocument that owns
    it.

    \sa add(), remove()
*/
void QTextList::removeItem(int i)
{
    Q_D(QTextList);
    if (i < 0 || i >= d->blocks.size())
        return;

    QTextBlock block = d->blocks.at(i);
    remove(block);
}


/*!
    Removes the given \a block from the list.

    \sa add(), removeItem()
*/
void QTextList::remove(const QTextBlock &block)
{
    QTextBlockFormat fmt = block.blockFormat();
    fmt.setIndent(fmt.indent() + format().indent());
    fmt.setObjectIndex(-1);
    block.docHandle()->setBlockFormat(block, block, fmt, QTextDocumentPrivate::SetFormat);
}

/*!
    Makes the given \a block part of the list.

    \sa remove(), removeItem()
*/
void QTextList::add(const QTextBlock &block)
{
    QTextBlockFormat fmt = block.blockFormat();
    fmt.setObjectIndex(objectIndex());
    block.docHandle()->setBlockFormat(block, block, fmt, QTextDocumentPrivate::SetFormat);
}

QT_END_NAMESPACE
