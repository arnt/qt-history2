
#include "qtextlist.h"
#include "qtextlist_p.h"
#include "qtextlistmanager_p.h"
#include "qtextcursor.h"
#include <private/qtextformat_p.h>
#include <qdebug.h>

QTextFormatReferenceChangeCommand::QTextFormatReferenceChangeCommand(QTextPieceTable *table, int _reference, const QTextFormat &_format)
    : formatCollection(table->formatCollection()), reference(_reference), format(_format)
{
}

void QTextFormatReferenceChangeCommand::undo()
{
    format = formatCollection->updateReferenceIndex(reference, format);
}

void QTextFormatReferenceChangeCommand::redo()
{
    undo();
}

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

QTextList::QTextList(QTextPieceTable *table, QObject *parent)
    : QObject(*(new QTextListPrivate), parent), tbl(table)
{
    Q_ASSERT(table);
}

QTextList::~QTextList()
{
}

int QTextList::count() const
{
    return d->blocks.count();
}

QTextCursor QTextList::item(int i) const
{
    if (i < 0 || i >= d->blocks.count())
	return QTextCursor();

    return QTextCursor(tbl, d->blocks.at(i).key() + 1);
}

void QTextList::setFormat(const QTextListFormat &format)
{
    int ref = d->listFormatIndex();
    if (ref == -1)
	return;

    QTextFormatReferenceChangeCommand *cmd = new QTextFormatReferenceChangeCommand(tbl, ref, format);
    cmd->redo();
    tbl->appendUndoItem(cmd);
}

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
		char baseChar = 'a';
		if (style == QTextListFormat::ListUpperAlpha)
		    baseChar = 'A';

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
    // ### rtl
    if (blockFormat.direction() == QTextBlockFormat::RightToLeft)
	return result.prepend(QString::fromLatin1("."));
    return result + QString::fromLatin1(".");

}



