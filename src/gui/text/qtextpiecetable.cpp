#include <private/qtools_p.h>
#include <qdebug.h>

#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include <qtextformat.h>
#include <private/qtextformat_p.h>
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
    // ##### optimise when only appending to the fragment!
    Q_ASSERT(!text.mid(strPos, length).contains(QTextParagraphSeparator));

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

    emit textChanged(pos, length);
    for (int i = 0; i < cursors.size(); ++i)
        cursors.at(i)->adjustPosition(pos, length, op);
    emit contentsChanged();

    adjustDocumentChanges(pos, length);
}

void QTextPieceTable::insert_block(int pos, uint strPos, int format, int blockFormat, UndoCommand::Operation op)
{
    // ##### optimise when only appending to the fragment!
    split(pos);
    uint x = fragments.insert_single(pos, 1);
    QTextFragment *X = fragments.fragment(x);
    X->format = format;
    X->stringPosition = strPos;
    uint w = fragments.prev(x);
    if (w)
        unite(w);

    Q_ASSERT(text.at(strPos) == QTextParagraphSeparator);
    Q_ASSERT(blocks.length()+1 == fragments.length());

    int n = blocks.findNode(pos);
    Q_ASSERT(n || (!n && !pos));
    int size = 1;
    int key = n ? blocks.position(n) : 0;
    if (key != pos) {
        Q_ASSERT(key < pos);
        int oldSize = blocks.size(n);
        blocks.setSize(n, pos-key+1);
        size += oldSize - (pos-key+1);
    }

    int b = blocks.insert_single(pos+1, size);
    QTextBlock *B = blocks.fragment(b);
    B->format = blockFormat;

    Q_ASSERT(blocks.length() == fragments.length());

    QTextFormatGroup *group = formats->format(blockFormat).group();
    if (group)
        group->insertBlock(QTextBlockIterator(this, b));
    emit blockChanged(pos+1, QText::Insert);

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
    Q_ASSERT(x && fragments.position(x) == (uint)pos && fragments.size(x) == length);
    Q_ASSERT(!text.mid(fragments.fragment(x)->stringPosition, length).contains(QTextParagraphSeparator));

    blocks.setSize(b, blocks.size(b)-length);
    return fragments.erase_single(x);
}

int QTextPieceTable::remove_block(int pos)
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(blocks.length() == fragments.length());
    Q_ASSERT(blocks.length() > pos);

    split(pos);
    split(pos+1);

    int b = blocks.findNode(pos+1);
    uint x = fragments.findNode(pos);

    Q_ASSERT(b && (int)blocks.position(b) == pos+1);
    Q_ASSERT(x && (int)fragments.position(x) == pos);
    Q_ASSERT(text.at(fragments.fragment(x)->stringPosition) == QTextParagraphSeparator);

    QTextFormatGroup *group = formats->format(blocks.fragment(b)->format).group();
    if (group)
        group->removeBlock(QTextBlockIterator(this, b));

    int size = blocks.size(b);
    int p = blocks.prev(b);
    Q_ASSERT(p);
    blocks.setSize(p, blocks.size(p) + size - 1);
    blocks.erase_single(b);

    return fragments.erase_single(x);
}

void QTextPieceTable::insertBlock(int pos, int blockFormat, int charFormat)
{
    Q_ASSERT(blockFormat == -1 || formats->format(blockFormat).isBlockFormat());
    Q_ASSERT(charFormat == -1 || formats->format(charFormat).isCharFormat());
    Q_ASSERT(pos >= 0 && (pos < fragments.length() || (pos == 0 && fragments.length() == 0)));

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

    Q_ASSERT(!str.contains(QTextParagraphSeparator));

    int strPos = text.length();
    text.append(str);
    insert(pos, strPos, str.length(), format);
}

void QTextPieceTable::insert(int pos, int strPos, int strLength, int format)
{
    if (strLength <= 0)
        return;

    Q_ASSERT(pos >= 0 && pos < fragments.length());
    Q_ASSERT(format == -1 || formats->format(format).isCharFormat());

    insert_string(pos, strPos, strLength, format, UndoCommand::MoveCursor);

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

    uint b = blocks.findNode(pos+1);
    uint be = blocks.findNode(pos+length);
    while (b != be) {
        int k = blocks.position(b);
        split(k);
        split(k+1);
        b = blocks.next(b);
    }

    uint x = fragments.findNode(pos);
    uint end = fragments.findNode(pos+length);

    uint w = 0;
    while (x != end) {
        uint n = fragments.next(x);

        uint key = fragments.position(x);
        b = blocks.findNode(key+1);

        QTextFragment *X = fragments.fragment(x);
        UndoCommand c = { UndoCommand::Removed, true,
                          op, X->format, X->stringPosition, key, { X->size } };

        if (key+1 != blocks.position(b)) {
            Q_ASSERT(!text.mid(X->stringPosition, X->size).contains(QTextParagraphSeparator));
            w = remove_string(key, X->size);
        } else {
            Q_ASSERT(X->size == 1 && text.at(X->stringPosition) == QTextParagraphSeparator);
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

        int offset = pos - it.position();
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
        Q_ASSERT(pos == (int)(it.position() + fragment->size) || pos >= endPos);
    }

    int n = fragments.findNode(startPos - 1);
    if (n)
        unite(n);

    n = fragments.findNode(endPos);
    if (n)
        unite(n);

    endEditBlock();

    QTextBlockIterator blockIt = blocksFind(startPos);
    QTextBlockIterator endIt = blocksFind(endPos);
    if (!endIt.atEnd())
        ++endIt;
    for (; !blockIt.atEnd() && blockIt != endIt; ++blockIt)
        QTextPieceTable::block(blockIt)->invalidate();

    emit formatChanged(startPos, length);
    emit contentsChanged();
}

void QTextPieceTable::setBlockFormat(int pos, int length, const QTextBlockFormat &newFormat, FormatChangeMode mode)
{
    truncateUndoStack();

    beginEditBlock();

    Q_ASSERT(newFormat.isValid());

    /* if we have only one block then QTextCursor calls us with
     * pos == length(), which is okay as-is, but also means we have
     * to pass pos - 1 to blocksFind
     */
    if (pos >= this->length()) {
        pos = this->length() - 1;
        length = 1;
    }

    int newFormatIdx = -1;
    if (mode == SetFormat)
        newFormatIdx = formats->indexForFormat(newFormat);
    QTextFormatGroup *group = newFormat.group();

    QTextBlockIterator blockIt = blocksFind(pos);
    QTextBlockIterator endIt = blocksFind(pos + length);

    const int startPos = blockIt.position();
    const int endPos = endIt.position() + endIt.length() - 1;

    if (!endIt.atEnd())
        ++endIt;
    for (; !blockIt.atEnd() && blockIt != endIt; ++blockIt) {
        int oldFormat = block(blockIt)->format;
        QTextBlockFormat format = formats->blockFormat(oldFormat);
        QTextFormatGroup *oldGroup = format.group();
        if (mode == MergeFormat) {
            format += newFormat;
            newFormatIdx = formats->indexForFormat(format);
            group = format.group();
        }
        block(blockIt)->format = newFormatIdx;

        block(blockIt)->invalidate();
        if (undoEnabled) {
            UndoCommand c = { UndoCommand::BlockFormatChanged, true, UndoCommand::MoveCursor, oldFormat,
                              0, blockIt.position(), { 1 } };

            appendUndoItem(c);
            Q_ASSERT(undoPosition == undoStack.size());
        }
        if (group != oldGroup) {
            if (oldGroup)
                oldGroup->removeBlock(blockIt);
            if (group)
                group->insertBlock(blockIt);
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
            QTextBlockIterator it = blocksFind(c.pos);
            Q_ASSERT(!it.atEnd());

            int oldFormat = block(it)->format;
            block(it)->format = c.format;
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
//         qDebug("adjustDocumentChanges:");
//         qDebug("    -> %d %d %d", docChangeFrom, docChangeOldLength, docChangeLength);
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
        result += QConstString(text.unicode() + f->stringPosition, f->size);
    }
    // remove trailing block separator
    result.truncate(result.length()-1);
    return result;
}



int QTextPieceTable::nextCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == length())
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
    if (position == 1)
        return position;

    QTextBlockIterator it = blocksFind(position);
    int start = it.position();
    if (position == start)
        return start - 1;

    return it.layout()->previousCursorPosition(position-start, mode) + start;
}


void QTextPieceTable::setBlockFormat(const QTextBlockIterator &it, const QTextBlockFormat &format)
{
    if (it.atEnd())
        return;
    const QTextBlock *b = it.pt->blocks.fragment(it.n);
    b->format = const_cast<QTextPieceTable *>(it.pt)->formatCollection()->indexForFormat(format);
}
