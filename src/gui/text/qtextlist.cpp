
#include "qtextlist.h"
#include "qtextlist_p.h"
#include "qtextcursor.h"
#include <private/qtextformat_p.h>
#include <qdebug.h>

#define d d_func()

/*!
    \class QTextList qtextlist.h
    \brief A list in a QTextDocument

    \ingroup text

    QTextList represents a list object in a QTextDocument. Lists can
    be created through QTextCursor::createList and queried with
    QTextCursor::currentList.

*/

/*! \internal
 */
QTextList::QTextList()
    : QTextFormatGroup(*(new QTextListPrivate))
{
}

/*!
  \internal
*/
QTextList::~QTextList()
{
}

/*!
  \returns the number of items in the list.
*/
int QTextList::count() const
{
    return d->blocks.count();
}

/*!
  \returns a QTextCursor positioned at the \a i 'th item in this list.
*/
QTextBlockIterator QTextList::item(int i) const
{
    if (i < 1 || i > d->blocks.size())
        return QTextBlockIterator();
    return d->blocks.at(i-1);
}

/*!
  \fn void QTextList::setFormat(const QTextListFormat &format)

  sets the format of the list to \a format.
*/

/*!
  \fn QTextListFormat QTextList::format() const

  \returns the format of the list.
*/

int QTextList::itemNumber(const QTextBlockIterator &blockIt) const
{
    return d->blocks.indexOf(blockIt) + 1;
}

QString QTextList::itemText(const QTextBlockIterator &blockIt) const
{
    int item = d->blocks.indexOf(blockIt) + 1;
    if (item <= 0)
        return QString();

    QTextBlockIterator block = d->blocks.at(item-1);
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
    if (blockFormat.direction() == QTextBlockFormat::RightToLeft)
        return result.prepend(QChar('.'));
    return result + QChar('.');
}

void QTextList::insertBlock(const QTextBlockIterator &block)
{
    d->blocks.append(block);
    qBubbleSort(d->blocks);
}

void QTextList::removeBlock(const QTextBlockIterator &block)
{
    d->blocks.remove(block);
}
