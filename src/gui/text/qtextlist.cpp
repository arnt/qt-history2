
#include "qtextlist.h"
#include "qtextlist_p.h"
#include "qtextlistmanager_p.h"
#include "qtextcursor.h"
#include <private/qtextformat_p.h>
#include <qdebug.h>

#define d d_func()

void QTextListPrivate::appendBlock(const QTextPieceTable::BlockIterator &block)
{
    blocks.append(block);
    qBubbleSort(blocks);
}

int QTextListPrivate::itemNumber(const QTextPieceTable::BlockIterator &block) const
{
    int res = blocks.indexOf(block);
    if (res == -1)
	return res;
    return res + 1;
}

int QTextListPrivate::listFormatIndex() const
{
    if (blocks.isEmpty())
	return -1;

    QTextPieceTable::BlockIterator block = blocks.first();
    Q_ASSERT(!block.atEnd());
    return block.blockFormat().listFormatIndex();
}

void QTextListPrivate::removeAllFormatIndicesFromBlocks()
{
    if (blocks.isEmpty())
	return;

    BlockList allBlocks = blocks;
    blocks.clear();

    QTextPieceTable *table = const_cast<QTextPieceTable *>(allBlocks.first().pieceTable());
    Q_ASSERT(table);

    table->beginEditBlock();

    Q_FOREACH(QTextPieceTable::BlockIterator it, allBlocks) {
	Q_ASSERT(!it.atEnd());

	QTextBlockFormat fmt = it.blockFormat();
	fmt.setListFormatIndex(-1);
	it.setBlockFormat(fmt);
    }

    table->endEditBlock();
}

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
QTextList::QTextList(QTextPieceTable *table, QObject *parent)
    : QObject(*(new QTextListPrivate), parent), tbl(table)
{
    Q_ASSERT(table);
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
QTextCursor QTextList::item(int i) const
{
    if (i < 0 || i >= d->blocks.count())
	return QTextCursor();

    return QTextCursor(tbl, d->blocks.at(i).key() + 1);
}

/*!
  sets the format of the list to \a format.
*/
void QTextList::setFormat(const QTextListFormat &format)
{
    int ref = d->listFormatIndex();
    if (ref == -1)
	return;

    QAbstractUndoItem *cmd = new QTextFormatReferenceChangeCommand<QTextListManager>(tbl->listManager(), d->listFormatIndex(), ref, format);
    cmd->redo();
    tbl->appendUndoItem(cmd);
}

/*!
  \returns the format of the list.
*/
QTextListFormat QTextList::format() const
{
    return tbl->formatCollection()->listFormat(d->listFormatIndex());
}

QTextListItem::QTextListItem(QTextList *_list, int _item)
	: list(_list), item(_item)
{
}

QTextListItem::QTextListItem(const QTextPieceTable::BlockIterator &block)
    : item(-1)
{
    if (block.atEnd())
	return;

    const QTextPieceTable *table = block.pieceTable();
    Q_ASSERT(table);

    int listIdx = block.blockFormat().listFormatIndex();
    if (listIdx == -1)
	return;

    list = table->listManager()->list(listIdx);
    Q_ASSERT(list);
    item = list->d->itemNumber(block);
}

QString QTextListItem::text() const
{
    if (!list || item < 0)
	return QString::null;

    QTextPieceTable::BlockIterator block = list->d->blocks.at(item - 1);
    if (block.atEnd())
	return QString::null;

    QTextBlockFormat blockFormat = block.blockFormat();
    int listIdx = blockFormat.listFormatIndex();
    if (listIdx == -1)
	return QString::null;

    const QTextPieceTable *table = block.pieceTable();
    Q_ASSERT(table);

    QTextListFormat listFmt = table->formatCollection()->listFormat(listIdx);

    QString result;

    const int style = listFmt.style();

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

