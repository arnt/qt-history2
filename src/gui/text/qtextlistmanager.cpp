
#include "qtextlistmanager_p.h"
#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include "qtextlist.h"
#include <private/qtextformat_p.h>

#include <qdebug.h>

QTextListManager::QTextListManager(QTextPieceTable *_table)
    : QObject(_table), table(_table)
{
    connect(table, SIGNAL(blockChanged(int,QText::ChangeOperation)), this, SLOT(blockChanged(int,QText::ChangeOperation)));
    connect(table, SIGNAL(formatChanged(int,int)), this, SLOT(formatChanged(int,int)));
}


QTextListManager::~QTextListManager()
{
}

QTextList *QTextListManager::list(QTextFormatGroup *group) const
{
    return lists.value(group);
}

QVector<QTextPieceTable::BlockIterator> QTextListManager::blocksForObject(QTextFormatGroup *group) const
{
    QVector<QTextPieceTable::BlockIterator> blocks;
    QTextList *l = list(group);
    if (l)
	blocks = l->d_func()->blocks;
    return blocks;
}

void QTextListManager::blockChanged(int blockPosition, QText::ChangeOperation op)
{
    QTextPieceTable::BlockIterator blockIt = table->blocksFind(blockPosition);
    if (blockIt.atEnd())
	return;

    QTextPieceTable::FragmentIterator fragmentIt = table->find(blockIt.key());
    if (fragmentIt.atEnd())
	return;

    int formatIdx = fragmentIt.value()->format;
    if (formatIdx == -1)
	return;

    Q_ASSERT(table->formatCollection()->format(formatIdx).isBlockFormat());

    QTextBlockFormat blockFmt = table->formatCollection()->blockFormat(formatIdx);
    QTextFormatGroup *group = blockFmt.group();
    if (!group)
	return;

    if (op == QText::Insert)
	addListEntry(group, blockIt);
    else
	removeListEntry(group, blockIt);
}

void QTextListManager::formatChanged(int position, int length)
{
    QTextPieceTable::BlockIterator blockIt = table->blocksFind(position);
    if (blockIt.atEnd())
	return;

    QTextPieceTable::BlockIterator end = table->blocksFind(position + length - 1);
    if (!end.atEnd())
	++end;

    // there's not much we can do except throwing away what we have that is
    // in the area of change and re-scan manually.

    for (; blockIt != end; ++blockIt) {
	// -1 for idx as we don't know the index anymore, as the old block format is
	// already gone
	removeListEntry(0, blockIt);

	blockChanged(blockIt.key(), QText::Insert);
    }
}

void QTextListManager::listDestroyed(QObject *obj)
{
    ListMap::Iterator it = lists.begin();
    while (it != lists.end()) {
	if (*it == obj) {
	    QTextListPrivate *data = (*it)->d_func();

	    it = lists.erase(it);

	    data->removeAllFormatIndicesFromBlocks();
	} else {
	    ++it;
	}
    }
}

void QTextListManager::removeListEntry(QTextFormatGroup *group, const QTextPieceTable::BlockIterator &blockIt)
{
    if (!group) {
	for (ListMap::ConstIterator it = lists.begin(); it != lists.end(); ++it)
	    if (it.value()->d_func()->blocks.contains(blockIt)) {
		group = it.key();
		break;
	    }
	if (!group)
	    return;
    }

    QTextList *list = lists.value(group);
    if (!list)
	return;
    QTextListPrivate *d = list->d_func();
    Q_ASSERT(d->blocks.contains(blockIt));

    int idx = d->blocks.indexOf(blockIt);
    while (idx != -1) {
	d->blocks.remove(idx, 1);
	idx = d->blocks.indexOf(blockIt, idx - 1);
    }

    if (d->blocks.isEmpty()) {
	lists.remove(group);
	delete list;
    }
}

void QTextListManager::addListEntry(QTextFormatGroup *group, const QTextPieceTable::BlockIterator &blockIt)
{
    QTextList *list = lists.value(group);
    if (!list) {
	list = new QTextList(table, this);
	lists.insert(group, list);
	connect(list, SIGNAL(destroyed(QObject*)), this, SLOT(listDestroyed(QObject*)));
    }

    Q_ASSERT(!list->d_func()->blocks.contains(blockIt));

    list->d_func()->appendBlock(blockIt);
}

