#include <private/qtools_p.h>
#include <qdebug.h>

#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include <qtextformat.h>
#include <private/qtextformat_p.h>
#include "qtextlistmanager_p.h"
#include "qtexttablemanager_p.h"
#include "qtextcursor.h"
#include "qtextobjectmanager_p.h"
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

QTextPieceTable::QTextPieceTable()
{
    undoBlock = 0;
    undoPosition = 0;
    undoEnabled = false;

    insertBlockSeparator(0, formats.indexForFormat(QTextBlockFormat()));
    undoEnabled = true;

    lists = new QTextListManager(this);
    tables = new QTextTableManager(this);
    objects = new QTextObjectManager(this);
    lout = new QTextDocumentLayout(this);
    // ### temporary
    objects->registerHandler(QTextFormat::ImageFormat, new QTextImageHandler(this));
}

QTextPieceTable::~QTextPieceTable()
{
    undoPosition = 0;
    truncateUndoStack();
}


void QTextPieceTable::insertWithoutUndo(int pos, uint strPos, uint length, int format, UndoCommand::Operation op)
{
    split(pos);
    uint x = fragments.insert_single(pos, length);
    QTextFragment *X = fragments.fragment(x);
    X->format = format;
    X->position = strPos;
    uint w = fragments.prev(x);
    if (w)
	unite(w);

    if (length == 1 && text.at(strPos) == QTextParagraphSeparator) {
	insertBlock(pos);
    } else {
	int b = blocks.findNode(pos-1);
	if (!b)
	    b = blocks.prev(b);
	blocks.setSize(b, blocks.size(b)+length);
    }
    Q_ASSERT(blocks.length() == fragments.length());

    emit textChanged(pos, length, op);
    for (int i = 0; i < cursors.size(); ++i)
	cursors.at(i)->adjustPosition(pos, length, op);
    emit contentsChanged();
}

void QTextPieceTable::insertBlock(int pos)
{
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
    uint x = blocks.insert_single(pos, size);
    Q_ASSERT(blocks.length() == fragments.length());
    emit blockChanged(blocks.key(x), true /*added*/);
}

void QTextPieceTable::removeBlocks(int pos, int length)
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(blocks.length() >= pos);

    BlockMap::ConstIterator end = blocks.find(pos + length - 1);
    if (!end.atEnd())
	++end;

    for (BlockMap::ConstIterator bit = blocks.find(pos); bit != end; ++bit) {
	int key = bit.key();
	if (key >= pos)
	    emit blockChanged(key, false /*removed*/);
    }

    int b = blocks.findNode(pos);
    Q_ASSERT(b != 0);

    while (length > 0) {
	Q_ASSERT(b != 0);
	int key = blocks.key(b);
	int n = blocks.next(b);
	int diff;
	if (key != pos) {
	    Q_ASSERT(key < pos);
	    int nk = n ? blocks.key(n) : blocks.length();
	    diff = nk - pos;
	    blocks.setSize(b, blocks.size(b) - diff);
	} else {
	    diff = blocks.size(b);
	    blocks.erase_single(b);
	}
	length -= diff;
	b = n;
    }
    if (length < 0) {
	b = blocks.findNode(pos-1);
	Q_ASSERT((int)blocks.key(b) < pos);
	blocks.setSize(b, blocks.size(b) - length);
    }
}

void QTextPieceTable::insertBlockSeparator(int pos, int blockFormat)
{
    Q_ASSERT(blockFormat == -1 || formats.format(blockFormat).isBlockFormat());
    Q_ASSERT(pos > 0 || (pos == 0 && fragments.length() == 0));

    int strPos = text.length();
    text.append(QTextParagraphSeparator);
    insertWithoutUndo(pos, strPos, 1, blockFormat, UndoCommand::MoveCursor);

    Q_ASSERT(blocks.length() == fragments.length());

    if (!undoEnabled)
	return;

    truncateUndoStack();

    UndoCommand c = { UndoCommand::Inserted, (undoBlock != 0),
		      UndoCommand::MoveCursor, blockFormat, strPos, pos, { 1 } };

    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());
}

void QTextPieceTable::insert(int pos, const QString &str, int format)
{
    if (!str)
	return;
    Q_ASSERT(pos > 0);
    Q_ASSERT(!str.contains(QTextParagraphSeparator));
    Q_ASSERT(format == -1 || formats.format(format).isCharFormat());
    int strPos = text.length();
    text.append(str);
    insert(pos, strPos, str.length(), format);
}

void QTextPieceTable::insert(int pos, int strPos, int strLength, int format)
{
    if (strLength <= 0)
	return;
    Q_ASSERT(pos > 0);
    Q_ASSERT(!text.mid(strPos, strLength).contains(QTextParagraphSeparator));
    Q_ASSERT(format == -1 || formats.format(format).isCharFormat());
    insertWithoutUndo(pos, strPos, strLength, format, UndoCommand::MoveCursor);

    Q_ASSERT(blocks.length() == fragments.length());

    if (!undoEnabled)
	return;

    truncateUndoStack();

    UndoCommand c = { UndoCommand::Inserted, (undoBlock != 0),
		      UndoCommand::MoveCursor, format, strPos, pos, { strLength } };

    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());
}

void QTextPieceTable::remove(int pos, int length, UndoCommand::Operation op)
{
    Q_ASSERT(pos >= 0 && pos+length <= fragments.length());
    Q_ASSERT(blocks.length() == fragments.length());

    removeBlocks(pos, length);

    split(pos);
    split(pos+length);

    uint x = fragments.findNode(pos);
    uint end = fragments.findNode(pos+length);

    uint w = 0;
    if (undoEnabled) {
	truncateUndoStack();

	while (x != end) {
	    uint n = fragments.next(x);

	    QTextFragment *X = fragments.fragment(x);
	    UndoCommand c = { UndoCommand::Removed, ((n != end) | (undoBlock != 0)),
			      op, X->format, X->position, fragments.key(x), { X->size } };
	    appendUndoItem(c);

	    w = fragments.erase_single(x);
	    x = n;
	}
    } else {
	while (x != end) {
	    uint n = fragments.next(x);
	    w = fragments.erase_single(x);
	    x = n;
	}
    }

    if (w)
	unite(w);

    Q_ASSERT(blocks.length() == fragments.length());

    emit textChanged(pos, -length, op);
    for (int i = 0; i < cursors.size(); ++i)
	cursors.at(i)->adjustPosition(pos, -length, op);
    emit contentsChanged();
}

void QTextPieceTable::setFormat(int pos, int length, const QTextFormat &newFormat, FormatChangeMode mode)
{
    if (undoEnabled) {
	truncateUndoStack();
	beginUndoBlock();
    }

    int newFormatIdx = -1;
    if (mode == SetFormat)
	newFormatIdx = formats.indexForFormat(newFormat);

    const int startPos = pos;
    const int endPos = pos + length;

    split(startPos);
    split(endPos);

    while (pos < endPos) {
	FragmentMap::Iterator it = fragments.find(pos);
	Q_ASSERT(!it.atEnd());

	QTextFragment *fragment = it.value();

	if (!formats.format(fragment->format).inheritsFormatType(newFormat.type())) {
	    pos = it.key() + fragment->size;
	    continue;
	}

	int offset = pos - it.key();
	int length = qMin(endPos - pos, int(fragment->size - offset));
	int oldFormat = fragment->format;

	it = fragments.find(pos);
	fragment = it.value();

	if (mode == MergeFormat) {
	    QTextFormat format = formats.format(fragment->format);
	    format += newFormat;
	    fragment->format = formats.indexForFormat(format);
	} else
	    fragment->format = newFormatIdx;

	if (undoEnabled) {
	    UndoCommand c = { UndoCommand::FormatChanged, true /*undoBlock != 0*/, UndoCommand::MoveCursor, oldFormat,
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

    if (undoEnabled)
	endUndoBlock();

    BlockIterator blockIt = blocksFind(startPos);
    BlockIterator endIt = blocksFind(endPos);
    if (!endIt.atEnd())
	++endIt;
    for (; !blockIt.atEnd() && blockIt != endIt; ++blockIt)
	blockIt.value()->invalidate();

    emit formatChanged(startPos, length);
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
    if ((undo && undoPosition == 0) || (!undo && undoPosition == undoStack.size()))
	return;

    PMDEBUG("%s", undo ? "undo:" : "redo:");
    undoEnabled = false;
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
	    insertWithoutUndo(c.pos, c.strPos, c.length, c.format, (UndoCommand::Operation)c.operation);
	    c.command = UndoCommand::Inserted;
	} else if (c.command == UndoCommand::FormatChanged) {
	    FragmentIterator it = find(c.pos);
	    Q_ASSERT(!it.atEnd());

	    int oldFormat = it.value()->format;

	    setFormat(c.pos, c.length, formats.format(c.format));

	    c.format = oldFormat;
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
	    if (undoPosition == undoStack.size() || !undoStack[undoPosition].block)
		break;
	}
    }
    undoEnabled = true;
}

/*!
  appends a custom undo item to the undo stack.
*/
void QTextPieceTable::appendUndoItem(QAbstractUndoItem *item)
{
    UndoCommand c = { UndoCommand::Custom, (undoBlock != 0),
		      UndoCommand::MoveCursor, 0, 0, 0, { 0 } };
    c.custom = item;
    appendUndoItem(c);
}

void QTextPieceTable::appendUndoItem(const UndoCommand &c)
{
    if (!undoStack.isEmpty()) {
	UndoCommand &last = undoStack[undoPosition - 1];
	if (last.tryMerge(c))
	    return;
    }
    undoStack.append(c);
    undoPosition++;
}

void QTextPieceTable::truncateUndoStack() {
    if (undoPosition >= undoStack.size())
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
    undoEnabled = enable;
    if (!enable) {
	undoPosition = 0;
	truncateUndoStack();
    }
}

void QTextPieceTable::endUndoBlock()
{
    if (--undoBlock || undoPosition <= 0)
	return;

    undoStack[undoPosition - 1].block = false;
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
	b->layout->setInlineObjectInterface(pt->objectManager());
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
    const_cast<QTextPieceTable *>(pt)->setFormat(key(), 1, format);
}

QTextBlockFormat QTextPieceTable::BlockIterator::blockFormat() const
{
    int idx = blockFormatIndex();
    if (idx == -1)
	return QTextBlockFormat();

    return pt->formatCollection()->blockFormat(idx);
}

int QTextPieceTable::BlockIterator::blockFormatIndex() const
{
    if (atEnd())
	return -1;

    QTextPieceTable::FragmentIterator it = pt->find(key());
    Q_ASSERT(!it.atEnd());

    return it.value()->format;
}
