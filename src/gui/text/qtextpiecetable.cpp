#include <private/qtools_p.h>
#include <qdebug.h>

#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include <qtextformat.h>
#include <private/qtextformat_p.h>
#include "qtextlistmanager_p.h"
#include "qtexttablemanager_p.h"
#include "qtextcursor.h"
#include "qtextimagehandler_p.h"
#include "qtextcursor_p.h"
#include "qtextdocumentlayout_p.h"

#include <stdlib.h>
#include <new>

#define TAG(a,b,c,d) (((Q_UINT32)a) << 24) | (((Q_UINT32)b) << 16) | (((Q_UINT32)c) << 8) | d;

#define PMDEBUG if(0) qDebug

bool UndoCommand::tryMerge(const UndoCommand &other)
{
    if (command != other.command)
	return false;

    if (command == Inserted
	&& (pos + length == other.pos)
	&& (strPos + length == other.strPos)
	&& format == other.format) {

	length += other.length;
	return true;
    }

    // removal to the 'right' using 'Delete' key
    if (command == Removed
	&& pos == other.pos
	&& (strPos + length == other.strPos)
	&& format == other.format) {

	length += other.length;
	return true;
    }

    // removal to the 'left' using 'Backspace'
    if (command == Removed
	&& (other.pos + other.length == pos)
	&& (other.strPos + other.length == strPos)
	&& (format == other.format)) {

	int l = length;
	(*this) = other;

	length += l;
	return true;
    }

    return false;
}

QTextPieceTable::QTextPieceTable(QAbstractTextDocumentLayout *layout)
{
    editBlock = 0;
    docChangeFrom = -1;

    undoPosition = 0;

    formats = new QTextFormatCollection;
    ++formats->ref;
    lists = new QTextListManager(this);
    tables = new QTextTableManager(this);
    if (!layout)
	layout = new QTextDocumentLayout();
    lout = layout;
    // take ownership
    lout->setParent(this);

    undoEnabled = false;
    insertBlock(0, formats->indexForFormat(QTextBlockFormat()), formats->indexForFormat(QTextCharFormat()));
    undoEnabled = true;
}

QTextPieceTable::~QTextPieceTable()
{
    undoPosition = 0;
    undoEnabled = true;
    truncateUndoStack();
    if (!--formats->ref)
	delete formats;
}


void QTextPieceTable::insert_string(int pos, uint strPos, uint length, int format, UndoCommand::Operation op)
{
    Q_ASSERT(!text.mid(strPos, length).contains(QTextParagraphSeparator));

    split(pos);
    uint x = fragments.insert_single(pos, length);
    QTextFragment *X = fragments.fragment(x);
    X->format = format;
    X->position = strPos;
    uint w = fragments.prev(x);
    if (w)
	unite(w);

    int b = blocks.findNode(pos-1);
    blocks.setSize(b, blocks.size(b)+length);

    Q_ASSERT(blocks.length() == fragments.length());

    emit textChanged(pos, length);
    for (int i = 0; i < cursors.size(); ++i)
	cursors.at(i)->adjustPosition(pos, length, op);
    emit contentsChanged();

    adjustDocumentChanges(pos, length);
}

void QTextPieceTable::insert_block(int pos, uint strPos, int format, int blockFormat, UndoCommand::Operation op)
{
    split(pos);
    uint x = fragments.insert_single(pos, 1);
    QTextFragment *X = fragments.fragment(x);
    X->format = format;
    X->position = strPos;
    uint w = fragments.prev(x);
    if (w)
	unite(w);

    Q_ASSERT(text.at(strPos) == QTextParagraphSeparator);
    Q_ASSERT(blocks.length()+1 == fragments.length());

    int n = blocks.findNode(pos);
    int size = 1;
    int key = n ? blocks.key(n) : pos;
    if (key != pos) {
	Q_ASSERT(key < pos);
	int oldSize = blocks.size(n);
	blocks.setSize(n, pos-key);
	size += oldSize - (pos-key);
    }

    int b = blocks.insert_single(pos, size);
    QTextBlock *B = blocks.fragment(b);
    B->format = blockFormat;
    Q_ASSERT(blocks.length() == fragments.length());

    emit blockChanged(pos, QText::Insert);

    emit textChanged(pos, 1);
    for (int i = 0; i < cursors.size(); ++i)
	cursors.at(i)->adjustPosition(pos, 1, op);
    emit contentsChanged();

    adjustDocumentChanges(pos, 1);
}

int QTextPieceTable::remove_string(int pos, uint length)
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(blocks.length() >= pos+(int)length);

    split(pos);
    split(pos+length);

    int b = blocks.findNode(pos);
    uint x = fragments.findNode(pos);

    Q_ASSERT(blocks.size(b) > length);
    Q_ASSERT(x && fragments.key(x) == (uint)pos && fragments.size(x) == length);
    Q_ASSERT(!text.mid(fragments.fragment(x)->position, length).contains(QTextParagraphSeparator));

    blocks.setSize(b, blocks.size(b)-length);
    return fragments.erase_single(x);
}

int QTextPieceTable::remove_block(int pos)
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(blocks.length() >= pos);


    split(pos);
    split(pos+1);

    int b = blocks.findNode(pos);
    uint x = fragments.findNode(pos);

    Q_ASSERT(b && (int)blocks.key(b) == pos);
    Q_ASSERT(x && (int)fragments.key(x) == pos);
    Q_ASSERT(text.at(fragments.fragment(x)->position) == QTextParagraphSeparator);

    blocks.erase_single(b);
    return fragments.erase_single(x);
}

void QTextPieceTable::insertBlock(int pos, int blockFormat, int charFormat)
{
    Q_ASSERT(blockFormat == -1 || formats->format(blockFormat).isBlockFormat());
    Q_ASSERT(charFormat == -1 || formats->format(charFormat).isCharFormat());
    Q_ASSERT(pos > 0 || (pos == 0 && fragments.length() == 0));

    beginEditBlock();

    int strPos = text.length();
    text.append(QTextParagraphSeparator);
    insert_block(pos, strPos, charFormat, blockFormat, UndoCommand::MoveCursor);

    Q_ASSERT(blocks.length() == fragments.length());

    truncateUndoStack();

    UndoCommand c = { UndoCommand::Inserted, true,
		      UndoCommand::MoveCursor, blockFormat, strPos, pos, { 1 } };

    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());

    endEditBlock();
}

void QTextPieceTable::insert(int pos, const QString &str, int format)
{
    if (str.size() == 0)
	return;
    Q_ASSERT(pos > 0);
    Q_ASSERT(!str.contains(QTextParagraphSeparator));
    Q_ASSERT(format == -1 || formats->format(format).isCharFormat());
    int strPos = text.length();
    text.append(str);
    insert(pos, strPos, str.length(), format);
}

void QTextPieceTable::insert(int pos, int strPos, int strLength, int format)
{
    if (strLength <= 0)
	return;
    Q_ASSERT(pos > 0);
    Q_ASSERT(format == -1 || formats->format(format).isCharFormat());

    insert_string(pos, strPos, strLength, format, UndoCommand::MoveCursor);

    Q_ASSERT(blocks.length() == fragments.length());

    beginEditBlock();

    truncateUndoStack();
    UndoCommand c = { UndoCommand::Inserted, true,
		      UndoCommand::MoveCursor, format, strPos, pos, { strLength } };
    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());

    endEditBlock();
}

void QTextPieceTable::remove(int pos, int length, UndoCommand::Operation op)
{
    Q_ASSERT(pos >= 0 && pos+length <= fragments.length());
    Q_ASSERT(blocks.length() == fragments.length());

    beginEditBlock();
    truncateUndoStack();

    split(pos);
    split(pos+length);

    uint b = blocks.findNode(pos);
    uint be = blocks.findNode(pos+length-1);
    uint i = b;
    while (i != be) {
	int k = blocks.key(i);
	split(k);
	split(k+1);
	i = blocks.next(i);
    }

    uint x = fragments.findNode(pos);
    uint end = fragments.findNode(pos+length);

    uint w = 0;
    while (x != end) {
	uint n = fragments.next(x);

	uint key = fragments.key(x);
	uint b = blocks.findNode(key);

	QTextFragment *X = fragments.fragment(x);
	UndoCommand c = { UndoCommand::Removed, true,
			  op, X->format, X->position, key, { X->size } };

	if (key != blocks.key(b)) {
	    Q_ASSERT(!text.mid(X->position, X->size).contains(QTextParagraphSeparator));
	    w = remove_string(key, X->size);
	} else {
	    Q_ASSERT(X->size == 1 && text.at(X->position) == QTextParagraphSeparator);
	    c.command = UndoCommand::BlockRemoved;
	    w = remove_block(key);
	}
	appendUndoItem(c);
	x = n;

    }
    if (w)
	unite(w);

    Q_ASSERT(blocks.length() == fragments.length());

    emit textChanged(pos, -length);
    for (int i = 0; i < cursors.size(); ++i)
	cursors.at(i)->adjustPosition(pos, -length, op);
    emit contentsChanged();

    adjustDocumentChanges(pos, -length);

    endEditBlock();
}

void QTextPieceTable::setCharFormat(int pos, int length, const QTextCharFormat &newFormat, FormatChangeMode mode)
{
    truncateUndoStack();

    Q_ASSERT(newFormat.isValid());

    beginEditBlock();

    int newFormatIdx = -1;
    if (mode == SetFormat)
	newFormatIdx = formats->indexForFormat(newFormat);

    const int startPos = pos;
    const int endPos = pos + length;

    split(startPos);
    split(endPos);

    while (pos < endPos) {
	FragmentMap::Iterator it = fragments.find(pos);
	Q_ASSERT(!it.atEnd());

	QTextFragment *fragment = it.value();

	Q_ASSERT(formats->format(fragment->format).type() == QTextFormat::CharFormat);

	int offset = pos - it.key();
	int length = qMin(endPos - pos, int(fragment->size - offset));
	int oldFormat = fragment->format;

	if (mode == MergeFormat) {
	    QTextFormat format = formats->format(fragment->format);
	    format += newFormat;
	    fragment->format = formats->indexForFormat(format);
	} else {
	    fragment->format = newFormatIdx;
	}

	if (undoEnabled) {
	    UndoCommand c = { UndoCommand::CharFormatChanged, true, UndoCommand::MoveCursor, oldFormat,
			      0, pos, { length } };

	    appendUndoItem(c);
	    Q_ASSERT(undoPosition == undoStack.size());
	}

	pos += length;
	Q_ASSERT(pos == (int)(it.key() + fragment->size) || pos >= endPos);
    }

    int n = fragments.findNode(startPos - 1);
    if (n)
	unite(n);

    n = fragments.findNode(endPos);
    if (n)
	unite(n);

    endEditBlock();

    BlockIterator blockIt = blocksFind(startPos);
    BlockIterator endIt = blocksFind(endPos);
    if (!endIt.atEnd())
	++endIt;
    for (; !blockIt.atEnd() && blockIt != endIt; ++blockIt)
	blockIt.value()->invalidate();

    emit formatChanged(startPos, length);
    emit contentsChanged();
}

void QTextPieceTable::setBlockFormat(int pos, int length, const QTextBlockFormat &newFormat, FormatChangeMode mode)
{
    truncateUndoStack();

    beginEditBlock();

    Q_ASSERT(newFormat.isValid());

    int newFormatIdx = -1;
    if (mode == SetFormat)
	newFormatIdx = formats->indexForFormat(newFormat);

    BlockIterator blockIt = blocksFind(pos);
    BlockIterator endIt = blocksFind(pos + length);

    const int startPos = blockIt.start();
    const int endPos = endIt.end();

    if (!endIt.atEnd())
	++endIt;
    for (; !blockIt.atEnd() && blockIt != endIt; ++blockIt) {
	int oldFormat = blockIt.blockFormatIndex();
	if (mode == MergeFormat) {
	    QTextBlockFormat format = formats->blockFormat(oldFormat);
	    format += newFormat;
	    newFormatIdx = formats->indexForFormat(format);
	}
	blockIt.value()->format = newFormatIdx;

	blockIt.value()->invalidate();
	if (undoEnabled) {
	    UndoCommand c = { UndoCommand::BlockFormatChanged, true, UndoCommand::MoveCursor, oldFormat,
			      0, blockIt.key(), { 1 } };

	    appendUndoItem(c);
	    Q_ASSERT(undoPosition == undoStack.size());
	}
    }

    endEditBlock();

    emit formatChanged(startPos, endPos - startPos + 1);
    emit contentsChanged();
}


bool QTextPieceTable::split(int pos)
{
    uint x = fragments.findNode(pos);
    if (x) {
	int k = fragments.key(x);
//  	qDebug("found fragment with key %d, size_left=%d, size=%d to split at %d",
// 	       k, (*it)->size_left, (*it)->size, pos);
	if (k != pos) {
	    Q_ASSERT(k <= pos);
	    // need to resize the first fragment and add a new one
	    QTextFragment *X = fragments.fragment(x);
	    int oldsize = X->size;
	    fragments.setSize(x, pos-k);
	    uint n = fragments.insert_single(pos, oldsize-(pos-k));
	    X = fragments.fragment(x);
	    QTextFragment *N = fragments.fragment(n);
	    N->position = X->position + pos-k;
	    N->format = X->format;
	    return true;
	}
    }
    return false;
}

bool QTextPieceTable::unite(uint f)
{
    uint n = fragments.next(f);
    if (!n)
	return false;

    QTextFragment *ff = fragments.fragment(f);
    QTextFragment *nf = fragments.fragment(n);

    if (nf->format == ff->format && (ff->position + (int)ff->size == nf->position)) {
	fragments.setSize(f, ff->size + nf->size);
	fragments.erase_single(n);
	return true;
    }
    return false;
}


void QTextPieceTable::undoRedo(bool undo)
{
    PMDEBUG("%s, undoPosition=%d, undoStack size=%d", undo ? "undo:" : "redo:", undoPosition, undoStack.size());
    if ((undo && undoPosition == 0) || (!undo && undoPosition == undoStack.size()))
	return;

    undoEnabled = false;
    beginEditBlock();
    while (1) {
	if (undo)
	    --undoPosition;
	UndoCommand &c = undoStack[undoPosition];

	if(c.command == UndoCommand::Inserted) {
	    remove(c.pos, c.length, (UndoCommand::Operation)c.operation);
	    PMDEBUG("   erase: from %d, length %d", c.pos, c.length);
	    c.command = UndoCommand::Removed;
	} else if (c.command == UndoCommand::Removed) {
	    PMDEBUG("   insert: format %d (from %d, length %d, strpos=%d)", c.format, c.pos, c.length, c.strPos);
	    insert_string(c.pos, c.strPos, c.length, c.format, (UndoCommand::Operation)c.operation);
	    c.command = UndoCommand::Inserted;
	} else if (c.command == UndoCommand::CharFormatChanged) {
	    FragmentIterator it = find(c.pos);
	    Q_ASSERT(!it.atEnd());

	    int oldFormat = it.value()->format;
	    setCharFormat(c.pos, c.length, formats->charFormat(c.format));
	    c.format = oldFormat;
	} else if (c.command == UndoCommand::BlockFormatChanged) {
	    BlockIterator it = blocksFind(c.pos);
	    Q_ASSERT(!it.atEnd());

	    int oldFormat = it.value()->format;
	    it.value()->format = c.format;
	    c.format = oldFormat;
	    // ########### emit doc changed signal
	} else if (c.command == UndoCommand::Custom) {
	    if (undo)
		c.custom->undo();
	    else
		c.custom->redo();
	}
	if (undo) {
	    if (undoPosition == 0 || !undoStack[undoPosition-1].block)
		break;
	} else {
	    ++undoPosition;
	    if (undoPosition == undoStack.size() || !undoStack[undoPosition-1].block)
		break;
	}
    }
    endEditBlock();
    undoEnabled = true;
}

/*!
    Appends a custom undo \a item to the undo stack.
*/
void QTextPieceTable::appendUndoItem(QAbstractUndoItem *item)
{
    if (!undoEnabled) {
	delete item;
	return;
    }

    UndoCommand c = { UndoCommand::Custom, (editBlock != 0),
		      UndoCommand::MoveCursor, 0, 0, 0, { 0 } };
    c.custom = item;
    appendUndoItem(c);
}

void QTextPieceTable::appendUndoItem(const UndoCommand &c)
{
    if (!undoEnabled)
	return;

    if (!undoStack.isEmpty()) {
	UndoCommand &last = undoStack[undoPosition - 1];
	if (last.tryMerge(c))
	    return;
    }
    undoStack.append(c);
    undoPosition++;
}

void QTextPieceTable::truncateUndoStack() {
    if (!undoEnabled || undoPosition >= undoStack.size())
	return;

    for (int i = undoPosition; i < undoStack.size(); ++i) {
	UndoCommand c = undoStack[i];
	if (c.command & UndoCommand::Removed) {
	    // ########
// 	    QTextFragment *f = c.fragment_list;
// 	    while (f) {
// 		QTextFragment *n = f->right;
// 		delete f;
// 		f = n;
// 	    }
	} else if (c.command & UndoCommand::Custom) {
	    delete c.custom;
	}
    }
    undoStack.resize(undoPosition);
}

void QTextPieceTable::enableUndoRedo(bool enable)
{
    if (!enable) {
	undoPosition = 0;
	truncateUndoStack();
    }
    undoEnabled = enable;
}

void QTextPieceTable::endEditBlock()
{
    if (--editBlock)
	return;

    if (undoEnabled && undoPosition > 0)
	undoStack[undoPosition - 1].block = false;

    if (docChangeFrom >= 0)
	lout->documentChange(docChangeFrom, docChangeOldLength, docChangeLength);
    docChangeFrom = -1;
}

void QTextPieceTable::adjustDocumentChanges(int from, int addedOrRemoved)
{
    if (docChangeFrom < 0) {
	docChangeFrom = from;
	if (addedOrRemoved > 0) {
	    docChangeOldLength = 0;
	    docChangeLength = addedOrRemoved;
	} else {
	    docChangeOldLength = -addedOrRemoved;
	    docChangeLength = 0;
	}
// 	qDebug("adjustDocumentChanges:");
// 	qDebug("    -> %d %d %d", docChangeFrom, docChangeOldLength, docChangeLength);
	return;
    }

    // have to merge the new change with the already existing one.
    int added = qMax(0, addedOrRemoved);
    int removed = qMax(0, -addedOrRemoved);

    int diff = 0;
    if(from + removed < docChangeFrom)
	diff = docChangeFrom - from - removed;
    else if(from > docChangeFrom + docChangeLength)
	diff = from - (docChangeFrom + docChangeLength);

    int overlap_start = qMax(from, docChangeFrom);
    int overlap_end = qMin(from + removed, docChangeFrom + docChangeLength);
    int removedInside = qMax(0, overlap_end - overlap_start);
    removed -= removedInside;

//     qDebug("adjustDocumentChanges: from=%d, addedOrRemoved=%d, diff=%d, removedInside=%d", from, addedOrRemoved, diff, removedInside);
    docChangeFrom = qMin(docChangeFrom, from);
    docChangeOldLength += removed + diff;
    docChangeLength += added - removedInside + diff;
//     qDebug("    -> %d %d %d", docChangeFrom, docChangeOldLength, docChangeLength);
}


QString QTextPieceTable::plainText() const
{
    QString result;
    for (QTextPieceTable::FragmentIterator it = begin(); it != end(); ++it) {
	const QTextFragment *f = *it;
	result += QConstString(text.unicode() + f->position, f->size);
    }
    return result;
}



int QTextPieceTable::nextCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == length())
	return position;

    BlockIterator it = blocksFind(position-1);
    int start = it.start();
    int end = it.end();
    if (position == end)
	return end + 1;

    return it.layout()->nextCursorPosition(position-start, mode) + start;
}

int QTextPieceTable::previousCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == 1)
	return position;

    BlockIterator it = blocksFind(position-1);
    int start = it.start();
    if (position == start)
	return start - 1;

    return it.layout()->previousCursorPosition(position-start, mode) + start;
}

QTextLayout *QTextPieceTable::BlockIterator::layout() const
{
    const QTextBlock *b = value();
    if (!b->layout) {
	b->layout = new QTextLayout();
	b->layout->setFormatCollection(const_cast<QTextPieceTable *>(pt)->formatCollection());
	b->layout->setDocumentLayout(pt->layout());
    }
    if (b->textDirty) {
	QString text = blockText();
	b->layout->setText(text);
	b->textDirty = false;

	if (!text.isEmpty()) {
	    int lastTextPosition = 0;
	    int textLength = 0;

	    QTextPieceTable::FragmentIterator it = pt->find(start());
	    QTextPieceTable::FragmentIterator e = pt->find(end());
	    int lastFormatIdx = it.value()->format;
	    for (; it != e; ++it) {
		const QTextFragment *fragment = it.value();
		int formatIndex = fragment->format;

		if (formatIndex != lastFormatIdx) {
		    Q_ASSERT(lastFormatIdx != -1);
		    b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);

		    lastFormatIdx = formatIndex;
		    lastTextPosition += textLength;
		    textLength = 0;
		}

		textLength += fragment->size;
	    }

	    Q_ASSERT(lastFormatIdx != -1);
	    b->layout->setFormat(lastTextPosition, textLength, lastFormatIdx);
	}
    }
    return b->layout;
}

QString QTextPieceTable::BlockIterator::blockText() const
{
    if (atEnd())
	return QString::null;

    const QString buffer = pt->buffer();
    QString text;

    QTextPieceTable::FragmentIterator it = pt->find(start());
    QTextPieceTable::FragmentIterator e = pt->find(end());

    for (; it != e; ++it) {
	const QTextFragment *fragment = it.value();

	text += QConstString(buffer.unicode()+fragment->position, fragment->size);
    }

    return text;
}


int QTextPieceTable::BlockIterator::start() const
{
    if (atEnd())
	return -1;
    return key() + 1;
}

int QTextPieceTable::BlockIterator::end() const
{
    if (atEnd())
	return -1;

    QFragmentMap<QTextBlock>::ConstIterator it = *this;

    ++it;

    if (it.atEnd())
	return pt->length();

    return it.key();
}

void QTextPieceTable::BlockIterator::setBlockFormat(const QTextBlockFormat &format)
{
    if (atEnd())
	return;
    const QTextBlock *b = value();
    b->format = const_cast<QTextPieceTable *>(pt)->formatCollection()->indexForFormat(format);
}

QTextBlockFormat QTextPieceTable::BlockIterator::blockFormat() const
{
    const QTextBlock *b = value();
    if (b->format == -1)
	return QTextBlockFormat();

    return pt->formatCollection()->blockFormat(b->format);
}

int QTextPieceTable::BlockIterator::charFormatIndex() const
{
    if (atEnd())
	return -1;

    QTextPieceTable::FragmentIterator it = pt->find(key());
    Q_ASSERT(!it.atEnd());

    return it.value()->format;
}

QTextCharFormat QTextPieceTable::BlockIterator::charFormat() const
{
    int idx = charFormatIndex();
    if (idx == -1)
 	return QTextCharFormat();

    return pt->formatCollection()->charFormat(idx);
}
