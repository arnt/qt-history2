#include <private/qtools_p.h>
#include <qdebug.h>

#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include <qtextformat.h>
#include <private/qtextformat_p.h>
#include "qtextcursor.h"
#include "qtextimagehandler_p.h"
#include "qtextcursor_p.h"
#include "qtextdocumentlayout_p.h"

#include <stdlib.h>
#include <new>

#define TAG(a,b,c,d) (((Q_UINT32)a) << 24) | (((Q_UINT32)b) << 16) | (((Q_UINT32)c) << 8) | d;

#define PMDEBUG if(0) qDebug

static bool isValidBlockSeparator(const QChar &ch)
{
    return ch == QChar::ParagraphSeparator
        || ch == QTextBeginningOfFrame
        || ch == QTextEndOfFrame
        || ch == QTextTableCellSeparator;
}

#ifndef QT_NO_DEBUG
static bool noBlockInString(const QString &str)
{
    return !str.contains(QChar::ParagraphSeparator)
        && !str.contains(QTextBeginningOfFrame)
        && !str.contains(QTextEndOfFrame)
        && !str.contains(QTextTableCellSeparator);
}
#endif

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

    formats = new QTextFormatCollection(this);
    ++formats->ref;
    if (!layout)
        layout = new QTextDocumentLayout();
    frame = new QTextFrame(this);
    framesDirty = false;

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
    // ##### optimise when only appending to the fragment!
    Q_ASSERT(noBlockInString(text.mid(strPos, length)));

    split(pos);
    uint x = fragments.insert_single(pos, length);
    QTextFragment *X = fragments.fragment(x);
    X->format = format;
    X->stringPosition = strPos;
    uint w = fragments.prev(x);
    if (w)
        unite(w);

    int b = blocks.findNode(pos);
    blocks.setSize(b, blocks.size(b)+length);

    Q_ASSERT(blocks.length() == fragments.length());

    QTextFrame *frame = qt_cast<QTextFrame *>(formats->format(format).group());
    if (frame)
        framesDirty = true;

    adjustDocumentChangesAndCursors(pos, length, op);
}

void QTextPieceTable::insert_block(int pos, uint strPos, int format, int blockFormat, UndoCommand::Operation op, int command)
{
    split(pos);
    uint x = fragments.insert_single(pos, 1);
    QTextFragment *X = fragments.fragment(x);
    X->format = format;
    X->stringPosition = strPos;
    // no need trying to unite, since paragraph separators are always in a fragment of their own

    Q_ASSERT(isValidBlockSeparator(text.at(strPos)));
    Q_ASSERT(blocks.length()+1 == fragments.length());

    int block_pos = pos;
    if (blocks.length() && command == UndoCommand::BlockRemoved)
        ++block_pos;
    int size = 1;
    int n = blocks.findNode(block_pos);
    int key = n ? blocks.position(n) : blocks.length();

    Q_ASSERT(n || (!n && block_pos == blocks.length()));
    if (key != block_pos) {
        Q_ASSERT(key < block_pos);
        int oldSize = blocks.size(n);
        blocks.setSize(n, block_pos-key);
        size += oldSize - (block_pos-key);
    }
    int b = blocks.insert_single(block_pos, size);
    QTextBlock *B = blocks.fragment(b);
    B->format = blockFormat;

    Q_ASSERT(blocks.length() == fragments.length());

    QTextGroup *group = formats->format(blockFormat).group();
    if (group)
        group->insertBlock(QTextBlockIterator(this, b));

    QTextFrame *frame = qt_cast<QTextFrame *>(formats->format(format).group());
    if (frame)
        framesDirty = true;

    adjustDocumentChangesAndCursors(pos, 1, op);
}

void QTextPieceTable::insertBlock(const QChar &blockSeparator,
                                  int pos, int blockFormat, int charFormat, UndoCommand::Operation op)
{
    Q_ASSERT(formats->format(blockFormat).isBlockFormat());
    Q_ASSERT(formats->format(charFormat).isCharFormat());
    Q_ASSERT(pos >= 0 && (pos < fragments.length() || (pos == 0 && fragments.length() == 0)));
    Q_ASSERT(isValidBlockSeparator(blockSeparator));

    beginEditBlock();

    int strPos = text.length();
    text.append(blockSeparator);
    insert_block(pos, strPos, charFormat, blockFormat, op, UndoCommand::BlockRemoved);

    Q_ASSERT(blocks.length() == fragments.length());

    UndoCommand c = { UndoCommand::BlockInserted, true,
                      op, charFormat, strPos, pos, { blockFormat } };

    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());

    endEditBlock();
}

void QTextPieceTable::insertBlock(int pos, int blockFormat, int charFormat, UndoCommand::Operation op)
{
    insertBlock(QChar::ParagraphSeparator, pos, blockFormat, charFormat, op);
}

void QTextPieceTable::insert(int pos, int strPos, int strLength, int format)
{
    if (strLength <= 0)
        return;

    Q_ASSERT(pos >= 0 && pos < fragments.length());
    Q_ASSERT(formats->format(format).isCharFormat());

    QTextFrame *frame = qt_cast<QTextFrame *>(formats->format(format).group());
    if (frame)
        framesDirty = true;

    insert_string(pos, strPos, strLength, format, UndoCommand::MoveCursor);

    beginEditBlock();

    UndoCommand c = { UndoCommand::Inserted, true,
                      UndoCommand::MoveCursor, format, strPos, pos, { strLength } };
    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());

    endEditBlock();
}

void QTextPieceTable::insert(int pos, const QString &str, int format)
{
    if (str.size() == 0)
        return;

    Q_ASSERT(noBlockInString(str));

    int strPos = text.length();
    text.append(str);
    insert(pos, strPos, str.length(), format);
}

int QTextPieceTable::remove_string(int pos, uint length, UndoCommand::Operation op)
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(blocks.length() >= pos+(int)length);

    int b = blocks.findNode(pos);
    uint x = fragments.findNode(pos);

    Q_ASSERT(blocks.size(b) > length);
    Q_ASSERT(x && fragments.position(x) == (uint)pos && fragments.size(x) == length);
    Q_ASSERT(noBlockInString(text.mid(fragments.fragment(x)->stringPosition, length)));

    blocks.setSize(b, blocks.size(b)-length);

    QTextFrame *frame = qt_cast<QTextFrame *>(formats->format(fragments.fragment(x)->format).group());
    if (frame)
        remove_frame(frame);

    const int w = fragments.erase_single(x);

    adjustDocumentChangesAndCursors(pos, -length, op);

    return w;
}

int QTextPieceTable::remove_block(int pos, int *blockFormat, int command, UndoCommand::Operation op)
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(blocks.length() > pos);

    int b = blocks.findNode(pos);
    uint x = fragments.findNode(pos);

    Q_ASSERT(x && (int)fragments.position(x) == pos);
    Q_ASSERT(fragments.size(x) == 1);
    Q_ASSERT(isValidBlockSeparator(text.at(fragments.fragment(x)->stringPosition)));
    Q_ASSERT(b);

    if (blocks.size(b) == 1 && command == UndoCommand::BlockAdded) {
	Q_ASSERT((int)blocks.position(b) == pos);
//  	qDebug("removing empty block");
	// empty block remove the block itself
    } else {
	// non empty block, merge with next one into this block
//  	qDebug("merging block with next");
	int n = blocks.next(b);
	Q_ASSERT((int)blocks.position(n) == pos + 1);
	blocks.setSize(b, blocks.size(b) + blocks.size(n) - 1);
	b = n;
    }
    *blockFormat = blocks.fragment(b)->format;

    QTextGroup *group = formats->format(blocks.fragment(b)->format).group();
    if (group)
        group->removeBlock(QTextBlockIterator(this, b));

    QTextFrame *frame = qt_cast<QTextFrame *>(formats->format(fragments.fragment(x)->format).group());
    if (frame)
        remove_frame(frame);

    blocks.erase_single(b);
    const int w = fragments.erase_single(x);

    adjustDocumentChangesAndCursors(pos, -1, op);

    return w;
}

void QTextPieceTable::remove(int pos, int length, UndoCommand::Operation op)
{
    Q_ASSERT(pos >= 0 && pos+length <= fragments.length());
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(frameAt(pos) == frameAt(pos+length-1));

    beginEditBlock();

    split(pos);
    split(pos+length);

    uint x = fragments.findNode(pos);
    uint end = fragments.findNode(pos+length);

    uint w = 0;
    while (x != end) {
        uint n = fragments.next(x);

        uint key = fragments.position(x);
        uint b = blocks.findNode(key+1);

        QTextFragment *X = fragments.fragment(x);
        UndoCommand c = { UndoCommand::Removed, true,
                          op, X->format, X->stringPosition, key, { X->size } };

        if (key+1 != blocks.position(b)) {
//  	    qDebug("remove_string from %d length %d", key, X->size);
            Q_ASSERT(noBlockInString(text.mid(X->stringPosition, X->size)));
            w = remove_string(key, X->size, op);
        } else {
//  	    qDebug("remove_block at %d", key);
            Q_ASSERT(X->size == 1 && isValidBlockSeparator(text.at(X->stringPosition)));
            b = blocks.prev(b);
            c.command = blocks.size(b) == 1 ? UndoCommand::BlockDeleted : UndoCommand::BlockRemoved;
            w = remove_block(key, &c.blockFormat, UndoCommand::BlockAdded, op);
        }
        appendUndoItem(c);
        x = n;

    }
    if (w)
        unite(w);

    Q_ASSERT(blocks.length() == fragments.length());

    endEditBlock();
}

void QTextPieceTable::setCharFormat(int pos, int length, const QTextCharFormat &newFormat, FormatChangeMode mode)
{
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

        int offset = pos - it.position();
        int length = qMin(endPos - pos, int(fragment->size - offset));
        int oldFormat = fragment->format;

        if (mode == MergeFormat) {
            QTextFormat format = formats->format(fragment->format);
            format.merge(newFormat);
            fragment->format = formats->indexForFormat(format);
        } else {
            fragment->format = newFormatIdx;
        }

        UndoCommand c = { UndoCommand::CharFormatChanged, true, UndoCommand::MoveCursor, oldFormat,
                          0, pos, { length } };
        appendUndoItem(c);

        pos += length;
        Q_ASSERT(pos == (int)(it.position() + fragment->size) || pos >= endPos);
    }

    int n = fragments.findNode(startPos - 1);
    if (n)
        unite(n);

    n = fragments.findNode(endPos);
    if (n)
        unite(n);

    QTextBlockIterator blockIt = blocksFind(startPos);
    QTextBlockIterator endIt = blocksFind(endPos);
    if (!endIt.atEnd())
        ++endIt;
    for (; !blockIt.atEnd() && blockIt != endIt; ++blockIt)
        QTextPieceTable::block(blockIt)->invalidate();

    documentChange(startPos, length);
    emit contentsChanged();

    endEditBlock();
}

void QTextPieceTable::setBlockFormat(const QTextBlockIterator &from, const QTextBlockIterator &to,
				     const QTextBlockFormat &newFormat, FormatChangeMode mode)
{
    beginEditBlock();

    Q_ASSERT(newFormat.isValid());

    int newFormatIdx = -1;
    if (mode == SetFormat)
        newFormatIdx = formats->indexForFormat(newFormat);
    QTextGroup *group = newFormat.group();

    QTextBlockIterator it = from;
    QTextBlockIterator end = to;
    if (!end.atEnd())
	++end;

    for (; it != end; ++it) {
        int oldFormat = block(it)->format;
        QTextBlockFormat format = formats->blockFormat(oldFormat);
        QTextGroup *oldGroup = format.group();
        if (mode == MergeFormat) {
            format.merge(newFormat);
            newFormatIdx = formats->indexForFormat(format);
            group = format.group();
        }
        block(it)->format = newFormatIdx;

        block(it)->invalidate();

        UndoCommand c = { UndoCommand::BlockFormatChanged, true, UndoCommand::MoveCursor, oldFormat,
                              0, it.position(), { 1 } };
        appendUndoItem(c);

        if (group != oldGroup) {
            if (oldGroup)
                oldGroup->removeBlock(it);
            if (group)
                group->insertBlock(it);
        } else if (group) {
	    group->blockFormatChanged(it);
	}
    }

    documentChange(from.position(), to.position() + to.length() - from.position());
    emit contentsChanged();

    endEditBlock();
}


bool QTextPieceTable::split(int pos)
{
    uint x = fragments.findNode(pos);
    if (x) {
        int k = fragments.position(x);
//          qDebug("found fragment with key %d, size_left=%d, size=%d to split at %d",
//                k, (*it)->size_left, (*it)->size, pos);
        if (k != pos) {
            Q_ASSERT(k <= pos);
            // need to resize the first fragment and add a new one
            QTextFragment *X = fragments.fragment(x);
            int oldsize = X->size;
            fragments.setSize(x, pos-k);
            uint n = fragments.insert_single(pos, oldsize-(pos-k));
            X = fragments.fragment(x);
            QTextFragment *N = fragments.fragment(n);
            N->stringPosition = X->stringPosition + pos-k;
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

    if (nf->format == ff->format && (ff->stringPosition + (int)ff->size == nf->stringPosition)) {
        if (isValidBlockSeparator(text.at(ff->stringPosition))
            || isValidBlockSeparator(text.at(nf->stringPosition)))
            return false;

        fragments.setSize(f, ff->size + nf->size);
        fragments.erase_single(n);
        return true;
    }
    return false;
}


void QTextPieceTable::undoRedo(bool undo)
{
    PMDEBUG("%s, undoPosition=%d, undoStack size=%d", undo ? "undo:" : "redo:", undoPosition, undoStack.size());
    if (!undoEnabled || (undo && undoPosition == 0) || (!undo && undoPosition == undoStack.size()))
        return;

    undoEnabled = false;
    beginEditBlock();
    while (1) {
        if (undo)
            --undoPosition;
        UndoCommand &c = undoStack[undoPosition];

	switch(c.command) {
        case UndoCommand::Inserted:
            remove(c.pos, c.length, (UndoCommand::Operation)c.operation);
            PMDEBUG("   erase: from %d, length %d", c.pos, c.length);
            c.command = UndoCommand::Removed;
	    break;
        case UndoCommand::Removed:
            PMDEBUG("   insert: format %d (from %d, length %d, strpos=%d)", c.format, c.pos, c.length, c.strPos);
            insert_string(c.pos, c.strPos, c.length, c.format, (UndoCommand::Operation)c.operation);
            c.command = UndoCommand::Inserted;
	    break;
	case UndoCommand::BlockInserted:
	case UndoCommand::BlockAdded:
            remove_block(c.pos, &c.blockFormat, c.command, (UndoCommand::Operation)c.operation);
            PMDEBUG("   blockremove: from %d", c.pos);
	    if (c.command == UndoCommand::BlockInserted)
		c.command = UndoCommand::BlockRemoved;
	    else
		c.command = UndoCommand::BlockDeleted;
	    break;
	case UndoCommand::BlockRemoved:
	case UndoCommand::BlockDeleted:
            PMDEBUG("   blockinsert: charformat %d blockformat %d (pos %d, strpos=%d)", c.format, c.blockFormat, c.pos, c.strPos);
            insert_block(c.pos, c.strPos, c.format, c.blockFormat, (UndoCommand::Operation)c.operation, c.command);
	    if (c.command == UndoCommand::BlockRemoved)
		c.command = UndoCommand::BlockInserted;
	    else
		c.command = UndoCommand::BlockAdded;
	    break;
	case UndoCommand::CharFormatChanged: {
            PMDEBUG("   charFormat: format %d (from %d, length %d)", c.format, c.pos, c.length);
            FragmentIterator it = find(c.pos);
            Q_ASSERT(!it.atEnd());

            int oldFormat = it.value()->format;
            setCharFormat(c.pos, c.length, formats->charFormat(c.format));
            c.format = oldFormat;
	    break;
	}
	case UndoCommand::BlockFormatChanged: {
            PMDEBUG("   blockformat: format %d pos %d", c.format, c.pos);
            QTextBlockIterator it = blocksFind(c.pos);
            Q_ASSERT(!it.atEnd());

            int oldFormat = block(it)->format;
            block(it)->format = c.format;
            QTextGroup *oldGroup = formats->blockFormat(oldFormat).group();
            QTextGroup *group = formats->blockFormat(c.format).group();
            c.format = oldFormat;
            if (group != oldGroup) {
                if (oldGroup)
                    oldGroup->removeBlock(it);
                if (group)
                    group->insertBlock(it);
            } else if (group) {
                group->blockFormatChanged(it);
            }
            documentChange(it.position(), it.length());
	    break;
	}
	case UndoCommand::GroupFormatChange: {
            QTextGroup *group = c.group;
            int oldFormat = group->d_func()->index;
            group->d_func()->index = c.format;
            changeGroupFormat(group, c.format);
            c.format = oldFormat;
	    break;
	}
	case UndoCommand::Custom:
            if (undo)
                c.custom->undo();
            else
                c.custom->redo();
	    break;
	default:
	    Q_ASSERT(false);
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
    if (undoPosition < undoStack.size())
        truncateUndoStack();

    if (!undoStack.isEmpty()) {
        UndoCommand &last = undoStack[undoPosition - 1];
        if (last.tryMerge(c))
            return;
    }
    undoStack.append(c);
    undoPosition++;
}

void QTextPieceTable::truncateUndoStack() {
    if (undoPosition == undoStack.size())
        return;

    for (int i = undoPosition; i < undoStack.size(); ++i) {
        UndoCommand c = undoStack[i];
        if (c.command & UndoCommand::Removed) {
            // ########
//             QTextFragment *f = c.fragment_list;
//             while (f) {
//                 QTextFragment *n = f->right;
//                 delete f;
//                 f = n;
//             }
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

    if (framesDirty)
        scan_frames(docChangeFrom, docChangeOldLength, docChangeLength);

    if (docChangeFrom >= 0)
        lout->documentChange(docChangeFrom, docChangeOldLength, docChangeLength);
    docChangeFrom = -1;
}

void QTextPieceTable::documentChange(int from, int length)
{
//     qDebug("QTextPieceTable::documentChange: from=%d,length=%d", from, length);
    if (docChangeFrom < 0) {
        docChangeFrom = from;
        docChangeOldLength = length;
        docChangeLength = length;
        return;
    }
    int start = qMin(from, docChangeFrom);
    int end = qMax(from + length, docChangeFrom + docChangeLength);
    int diff = qMax(0, end - from - length);
    docChangeFrom = start;
    docChangeOldLength += diff;
    docChangeLength += diff;
}

void QTextPieceTable::adjustDocumentChangesAndCursors(int from, int addedOrRemoved, UndoCommand::Operation op)
{
    for (int i = 0; i < cursors.size(); ++i)
        cursors.at(i)->adjustPosition(from, addedOrRemoved, op);

//     qDebug("QTextPieceTable::adjustDocumentChanges: from=%d,addedOrRemoved=%d", from, addedOrRemoved);
    if (docChangeFrom < 0) {
        docChangeFrom = from;
        if (addedOrRemoved > 0) {
            docChangeOldLength = 0;
            docChangeLength = addedOrRemoved;
        } else {
            docChangeOldLength = -addedOrRemoved;
            docChangeLength = 0;
        }
//         qDebug("adjustDocumentChanges:");
//         qDebug("    -> %d %d %d", docChangeFrom, docChangeOldLength, docChangeLength);
        emit contentsChanged();
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

    emit contentsChanged();
}


QString QTextPieceTable::plainText() const
{
    QString result;
    for (QTextPieceTable::FragmentIterator it = begin(); it != end(); ++it) {
        const QTextFragment *f = *it;
        result += QConstString(text.unicode() + f->stringPosition, f->size);
    }
    // remove trailing block separator
    result.truncate(result.length()-1);
    return result;
}



int QTextPieceTable::nextCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == length()-1)
        return position;

    QTextBlockIterator it = blocksFind(position);
    int start = it.position();
    int end = start + it.length() - 1;
    if (position == end)
        return end + 1;

    return it.layout()->nextCursorPosition(position-start, mode) + start;
}

int QTextPieceTable::previousCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == 0)
        return position;

    QTextBlockIterator it = blocksFind(position);
    int start = it.position();
    if (position == start)
        return start - 1;

    return it.layout()->previousCursorPosition(position-start, mode) + start;
}

void QTextPieceTable::changeGroupFormat(QTextGroup *group, int format)
{
    beginEditBlock();
    int oldFormatIndex = group->d_func()->index;
    group->d_func()->index = format;

    QList<QTextBlockIterator> blocks = group->blockList();
    for (int i = 0; i < blocks.size(); ++i) {
        // invalidate blocks and tell layout
        const QTextBlockIterator &block = blocks.at(i);
        documentChange(block.position(), block.length());
    }

    UndoCommand c = { UndoCommand::GroupFormatChange, true, UndoCommand::MoveCursor, oldFormatIndex,
                      0, 0, { 1 } };
    c.group = group;
    appendUndoItem(c);

    emit contentsChanged();
    endEditBlock();
}

static QTextFrame *findChildFrame(QTextFrame *f, int pos)
{
    QList<QTextFrame *> children = f->children();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        if (pos >= c->start() && pos < c->end()) {
            return c;
        }
    }
    return 0;
}

QTextFrame *QTextPieceTable::frameAt(int pos) const
{
    QTextFrame *f = frame;

    while (1) {
        QTextFrame *c = findChildFrame(f, pos);
        if (!c)
            return f;
        f = c;
    }
}

#define d d_func()

void QTextPieceTable::clearFrame(QTextFrame *f)
{
    for (int i = 0; i < f->d->childFrames.count(); ++i)
        clearFrame(f->d->childFrames.at(i));
    f->d->childFrames.clear();
    f->d->parentFrame = 0;
}

void QTextPieceTable::scan_frames(int pos, int charsRemoved, int charsAddded)
{
    // ###### optimise
    Q_UNUSED(pos);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAddded);

    QTextFrame *f = frame;
    clearFrame(f);

    for (FragmentIterator it = begin(); it != end(); ++it) {
        QTextFormat fmt = formats->format(it->format);
        QTextFrame *frame = qt_cast<QTextFrame *>(fmt.group());
        if (!frame)
            continue;

        Q_ASSERT(it.size() == 1);
        QChar ch = text.at(it->stringPosition);

        if (ch == QTextBeginningOfFrame) {
            Q_ASSERT(f != frame);
            frame->d->fragment_start = it.n;
            frame->d->parentFrame = f;
            f->d->childFrames.append(frame);
            f = frame;
        } else if (ch == QTextEndOfFrame) {
            Q_ASSERT(f == frame);
            frame->d->fragment_end = it.n;
            f = frame->d->parentFrame;
        } else if (ch == QChar::ObjectReplacementCharacter) {
            Q_ASSERT(f != frame);
            frame->d->fragment_start = it.n;
            frame->d->fragment_end = it.n;
            frame->d->parentFrame = f;
            f->d->childFrames.append(frame);
        } else if (ch == QTextTableCellSeparator) {
            Q_ASSERT(f == frame);
            ; // nothing for now
        } else {
            Q_ASSERT(false);
        }
    }
    Q_ASSERT(f == frame);
}

void QTextPieceTable::insert_frame(QTextFrame *f)
{
    int start = f->start();
    int end = f->end();
    QTextFrame *parent = frameAt(start);
    Q_ASSERT(parent == frameAt(end));

    if (start != end) {
        // iterator over the parent and move all children contained in my frame to myself
        for (int i = 0; i < parent->d->childFrames.size(); ++i) {
            QTextFrame *c = parent->d->childFrames.at(i);
            if (start < c->start () && end > c->end()) {
                parent->d->childFrames.removeAt(i);
                f->d->childFrames.append(c);
                c->d->parentFrame = f;
            }
        }
    }
    // insert at the correct position
    int i = 0;
    for (; i < parent->d->childFrames.size(); ++i) {
        QTextFrame *c = parent->d->childFrames.at(i);
        if (c->start() > end)
            break;
    }
    parent->d->childFrames.insert(i, f);
    f->d->parentFrame = parent;
}

void QTextPieceTable::remove_frame(QTextFrame *f)
{
    QTextFrame *parent = f->d->parentFrame;
    if (!parent)
        return;

    int index = parent->d->childFrames.indexOf(f);

    // iterator over all children and move them to the parent
    for (int i = 0; i < f->d->childFrames.size(); ++i) {
        QTextFrame *c = f->d->childFrames.at(i);
        parent->d->childFrames.insert(index, c);
        c->d->parentFrame = parent;
        ++index;
    }
    Q_ASSERT(parent->d->childFrames.at(index) == f);
    parent->d->childFrames.removeAt(index);

    f->d->childFrames.clear();
    f->d->parentFrame = 0;
}

QTextFrame *QTextPieceTable::insertFrame(int start, int end, const QTextFrameFormat &format)
{
    Q_ASSERT(start >= 0 && start < length());
    Q_ASSERT(end >= 0 && end < length());
    Q_ASSERT(start <= end || end == -1);

    if (start != end && frameAt(start) != frameAt(end))
        return 0;

    beginEditBlock();

    QTextFrame *frame = qt_cast<QTextFrame *>(formats->createGroup(format));
    Q_ASSERT(frame);

    // #### using the default block and char format below might be wrong
    int idx = formats->indexForFormat(QTextBlockFormat());
    QTextCharFormat cfmt;
    cfmt.setGroup(frame);
    int charIdx = formats->indexForFormat(cfmt);

    insertBlock(QTextBeginningOfFrame, start, idx, charIdx, UndoCommand::MoveCursor);
    insertBlock(QTextEndOfFrame, ++end, idx, charIdx, UndoCommand::KeepCursor);

    frame->d_func()->fragment_start = find(start).n;
    frame->d_func()->fragment_end = find(end).n;

    insert_frame(frame);
    framesDirty = false;

    endEditBlock();

    return frame;
}

void QTextPieceTable::removeFrame(QTextFrame *frame)
{
    QTextFrame *parent = frame->d->parentFrame;
    if (!parent)
        return;

    int start = frame->start();
    int end = frame->end();
    Q_ASSERT(end > start);

    beginEditBlock();

    // remove already removes the frames from the tree
    remove(end, 1);
    remove(start, 1);

    endEditBlock();
}
