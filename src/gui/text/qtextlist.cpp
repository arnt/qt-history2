
#include "qtextlist.h"
#include "qtextformat_p.h"
#include "qtextcursor.h"
#include "qtextblockiterator.h"
#include "qtextpiecetable_p.h"
#include <qdebug.h>


class QTextListPrivate : public QTextBlockGroupPrivate
{
};

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
QTextList::QTextList(QObject *parent)
    : QTextBlockGroup(*new QTextListPrivate, parent)
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
*/
int QTextList::count() const
{
    return d->blocks.count();
}

/*!
  Returns a QTextCursor positioned at the \a i 'th item in this list.
*/
QTextBlockIterator QTextList::item(int i) const
{
    if (i < 0 || i >= d->blocks.size())
        return QTextBlockIterator();
    return d->blocks.at(i);
}

/*!
  \fn void QTextList::setFormat(const QTextListFormat &format)

  sets the format of the list to \a format.
*/

/*!
  \fn QTextListFormat QTextList::format() const

  Returns the format of the list.
*/

int QTextList::itemNumber(const QTextBlockIterator &blockIt) const
{
    return d->blocks.indexOf(blockIt);
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

void QTextList::removeItem(int i)
{
    if (i < 0 || i >= d->blocks.size())
        return;

    QTextBlockIterator block = d->blocks.at(i);
    QTextBlockFormat fmt = block.blockFormat();
    fmt.setIndent(fmt.indent() + format().indent());
    fmt.setObject(0);
    const_cast<QTextPieceTable *>(block.pieceTable())->setBlockFormat(block, block, fmt, QTextPieceTable::SetFormat);
}
