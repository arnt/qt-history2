/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qtools_p.h>
#include <qdebug.h>

#include "qtextdocument_p.h"
#include "qtextdocument.h"
#include <qtextformat.h>
#include "qtextformat_p.h"
#include "qtextobject_p.h"
#include "qtextcursor.h"
#include "qtextimagehandler_p.h"
#include "qtextcursor_p.h"
#include "qtextdocumentlayout_p.h"
#include "qtexttable.h"

#include <stdlib.h>
#include <new>

#define PMDEBUG if(0) qDebug

/*
  Structure of a document:

  DOCUMENT :== FRAME_CONTENTS
  FRAME :== START_OF_FRAME  FRAME_CONTENTS END_OF_FRAME
  FRAME_CONTENTS = LIST_OF_BLOCKS ((FRAME | TABLE) LIST_OF_BLOCKS)*
  TABLE :== (START_OF_FRAME TABLE_CELL)+ END_OF_FRAME
  TABLE_CELL = FRAME_CONTENTS
  LIST_OF_BLOCKS :== (BLOCK END_OF_PARA)* BLOCK
  BLOCK :== (FRAGMENT)*
  FRAGMENT :== String of characters

  END_OF_PARA :== 0x2029 # Paragraph separator in Unicode
  START_OF_FRAME :== 0xfdd0
  END_OF_FRAME := 0xfdd1

  Note also that LIST_OF_BLOCKS can be empty. Nevertheless, there is
  at least one valid cursor position there where you could start
  typing. The block format is in this case determined by the last
  END_OF_PARA/START_OF_FRAME/END_OF_FRAME (see below).

  Lists are not in here, as they are treated specially. A list is just
  a collection of (not neccessarily connected) blocks, that share the
  same objectIndex() in the format that refers to the list format and
  object.

  The above does not clearly note where formats are. Here's
  how it looks currently:

  FRAGMENT: one charFormat associated

  END_OF_PARA: one charFormat, and a blockFormat for the _next_ block.

  START_OF_FRAME: one char format, and a blockFormat (for the next
  block). The format associated with the objectIndex() of the
  charFormat decides whether this is a frame or table and it's
  properties

  END_OF_FRAME: one charFormat and a blockFormat (for the next
  block). The object() of the charFormat is the same as for the
  corresponding START_OF_BLOCK.


  The document is independent of the layout with certain restrictions:

  * Cursor movement (esp. up and down) depend on the layout.
  * You cannot have more than one layout, as the layout data of QTextObjects
    is stored in the text object itself.

*/

#define d d_func()
#define q q_func()

static bool isValidBlockSeparator(const QChar &ch)
{
    return ch == QChar::ParagraphSeparator
        || ch == QTextBeginningOfFrame
        || ch == QTextEndOfFrame;
}

#ifndef QT_NO_DEBUG
static bool noBlockInString(const QString &str)
{
    return !str.contains(QChar::ParagraphSeparator)
        && !str.contains(QTextBeginningOfFrame)
        && !str.contains(QTextEndOfFrame);
}
#endif

bool QTextUndoCommand::tryMerge(const QTextUndoCommand &other)
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

QTextDocumentPrivate::QTextDocumentPrivate()
{
    editBlock = 0;
    docChangeFrom = -1;

    undoPosition = 0;
    framesDirty = false;

    lout = 0;

    modified = false;
    lastUnmodifiedUndoStackPos = 0;
}

void QTextDocumentPrivate::init()
{
    frame = qt_cast<QTextFrame *>(createObject(QTextFrameFormat()));
    framesDirty = false;

    undoEnabled = false;
    insertBlock(0, formats.indexForFormat(QTextBlockFormat()), formats.indexForFormat(QTextCharFormat()));
    undoEnabled = true;
    modified = false;
    lastUnmodifiedUndoStackPos = 0;
}

QTextDocumentPrivate::~QTextDocumentPrivate()
{
    for (int i = 0; i < cursors.count(); ++i)
        cursors.at(i)->priv = 0;
    cursors.clear();
    undoPosition = 0;
    undoEnabled = true;
    truncateUndoStack();
}

void QTextDocumentPrivate::setLayout(QAbstractTextDocumentLayout *layout)
{
    if (lout) {
        qDebug("deleting old layout!!!");
        delete lout;
    }
    lout = layout;
    lout->documentChange(0, 0, length());
}


void QTextDocumentPrivate::insert_string(int pos, uint strPos, uint length, int format, QTextUndoCommand::Operation op)
{
    // ##### optimise when only appending to the fragment!
    Q_ASSERT(noBlockInString(text.mid(strPos, length)));

    split(pos);
    uint x = fragments.insert_single(pos, length);
    QTextFragmentData *X = fragments.fragment(x);
    X->format = format;
    X->stringPosition = strPos;
    uint w = fragments.previous(x);
    if (w)
        unite(w);

    int b = blocks.findNode(pos);
    blocks.setSize(b, blocks.size(b)+length);

    Q_ASSERT(blocks.length() == fragments.length());

    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(format));
    if (frame) {
        frame->d_func()->fragmentAdded(text.at(strPos), x);
        framesDirty = true;
    }

    adjustDocumentChangesAndCursors(pos, length, op);
}

void QTextDocumentPrivate::insert_block(int pos, uint strPos, int format, int blockFormat, QTextUndoCommand::Operation op, int command)
{
    split(pos);
    uint x = fragments.insert_single(pos, 1);
    QTextFragmentData *X = fragments.fragment(x);
    X->format = format;
    X->stringPosition = strPos;
    // no need trying to unite, since paragraph separators are always in a fragment of their own

    Q_ASSERT(isValidBlockSeparator(text.at(strPos)));
    Q_ASSERT(blocks.length()+1 == fragments.length());

    int block_pos = pos;
    if (blocks.length() && command == QTextUndoCommand::BlockRemoved)
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
    QTextBlockData *B = blocks.fragment(b);
    B->format = blockFormat;

    Q_ASSERT(blocks.length() == fragments.length());

    QTextBlockGroup *group = qt_cast<QTextBlockGroup *>(objectForFormat(blockFormat));
    if (group)
        group->blockInserted(QTextBlock(this, b));

    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(formats.format(format)));
    if (frame) {
        frame->d_func()->fragmentAdded(text.at(strPos), x);
        framesDirty = true;
    }

    adjustDocumentChangesAndCursors(pos, 1, op);
}

void QTextDocumentPrivate::insertBlock(const QChar &blockSeparator,
                                  int pos, int blockFormat, int charFormat, QTextUndoCommand::Operation op)
{
    Q_ASSERT(formats.format(blockFormat).isBlockFormat());
    Q_ASSERT(formats.format(charFormat).isCharFormat());
    Q_ASSERT(pos >= 0 && (pos < fragments.length() || (pos == 0 && fragments.length() == 0)));
    Q_ASSERT(isValidBlockSeparator(blockSeparator));

    beginEditBlock();

    int strPos = text.length();
    text.append(blockSeparator);
    insert_block(pos, strPos, charFormat, blockFormat, op, QTextUndoCommand::BlockRemoved);

    Q_ASSERT(blocks.length() == fragments.length());

    QTextUndoCommand c = { QTextUndoCommand::BlockInserted, true,
                      op, charFormat, strPos, pos, { blockFormat } };

    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());

    endEditBlock();
}

void QTextDocumentPrivate::insertBlock(int pos, int blockFormat, int charFormat, QTextUndoCommand::Operation op)
{
    insertBlock(QChar::ParagraphSeparator, pos, blockFormat, charFormat, op);
}

void QTextDocumentPrivate::insert(int pos, int strPos, int strLength, int format)
{
    if (strLength <= 0)
        return;

    Q_ASSERT(pos >= 0 && pos < fragments.length());
    Q_ASSERT(formats.format(format).isCharFormat());

    insert_string(pos, strPos, strLength, format, QTextUndoCommand::MoveCursor);

    beginEditBlock();

    QTextUndoCommand c = { QTextUndoCommand::Inserted, true,
                      QTextUndoCommand::MoveCursor, format, strPos, pos, { strLength } };
    appendUndoItem(c);
    Q_ASSERT(undoPosition == undoStack.size());

    endEditBlock();
}

void QTextDocumentPrivate::insert(int pos, const QString &str, int format)
{
    if (str.size() == 0)
        return;

    Q_ASSERT(noBlockInString(str));

    int strPos = text.length();
    text.append(str);
    insert(pos, strPos, str.length(), format);
}

int QTextDocumentPrivate::remove_string(int pos, uint length, QTextUndoCommand::Operation op)
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

    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(fragments.fragment(x)->format));
    if (frame) {
        frame->d_func()->fragmentRemoved(text.at(fragments.fragment(x)->stringPosition), x);
        framesDirty = true;
    }

    const int w = fragments.erase_single(x);

    adjustDocumentChangesAndCursors(pos, -int(length), op);

    return w;
}

int QTextDocumentPrivate::remove_block(int pos, int *blockFormat, int command, QTextUndoCommand::Operation op)
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

    if (blocks.size(b) == 1 && command == QTextUndoCommand::BlockAdded) {
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

    QTextBlockGroup *group = qt_cast<QTextBlockGroup *>(objectForFormat(blocks.fragment(b)->format));
    if (group)
        group->blockRemoved(QTextBlock(this, b));

    QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(fragments.fragment(x)->format));
    if (frame) {
        frame->d_func()->fragmentRemoved(text.at(fragments.fragment(x)->stringPosition), x);
        framesDirty = true;
    }

    blocks.erase_single(b);
    const int w = fragments.erase_single(x);

    adjustDocumentChangesAndCursors(pos, -1, op);

    return w;
}

#if !defined(QT_NO_DEBUG)
static bool isAncestorFrame(QTextFrame *possibleAncestor, QTextFrame *child)
{
    while (child) {
        if (child == possibleAncestor)
            return true;
        child = child->parentFrame();
    }
    return false;
}
#endif

void QTextDocumentPrivate::remove(int pos, int length, QTextUndoCommand::Operation op)
{
    Q_ASSERT(pos >= 0 && pos+length <= fragments.length());
    Q_ASSERT(blocks.length() == fragments.length());

#if !defined(QT_NO_DEBUG)
    const bool startAndEndInSameFrame = (frameAt(pos) == frameAt(pos + length - 1));

    const bool endIsEndOfChildFrame = (isAncestorFrame(frameAt(pos), frameAt(pos + length - 1))
                                       && text.at(find(pos + length - 1)->stringPosition) == QTextEndOfFrame);

    const bool startIsStartOfFrameAndEndIsEndOfFrameWithCommonParent
               = (text.at(find(pos)->stringPosition) == QTextBeginningOfFrame
                  && text.at(find(pos + length - 1)->stringPosition) == QTextEndOfFrame
                  && frameAt(pos)->parentFrame() == frameAt(pos + length - 1)->parentFrame());

    Q_ASSERT(startAndEndInSameFrame || endIsEndOfChildFrame || startIsStartOfFrameAndEndIsEndOfFrameWithCommonParent);
#endif

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

        QTextFragmentData *X = fragments.fragment(x);
        QTextUndoCommand c = { QTextUndoCommand::Removed, true,
                          op, X->format, X->stringPosition, key, { X->size } };

        if (key+1 != blocks.position(b)) {
//	    qDebug("remove_string from %d length %d", key, X->size);
            Q_ASSERT(noBlockInString(text.mid(X->stringPosition, X->size)));
            w = remove_string(key, X->size, op);
        } else {
//	    qDebug("remove_block at %d", key);
            Q_ASSERT(X->size == 1 && isValidBlockSeparator(text.at(X->stringPosition)));
            b = blocks.previous(b);
            c.command = blocks.size(b) == 1 ? QTextUndoCommand::BlockDeleted : QTextUndoCommand::BlockRemoved;
            w = remove_block(key, &c.blockFormat, QTextUndoCommand::BlockAdded, op);
        }
        appendUndoItem(c);
        x = n;

    }
    if (w)
        unite(w);

    Q_ASSERT(blocks.length() == fragments.length());

    endEditBlock();
}

void QTextDocumentPrivate::setCharFormat(int pos, int length, const QTextCharFormat &newFormat, FormatChangeMode mode)
{
    Q_ASSERT(newFormat.isValid());

    beginEditBlock();

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

        QTextFragmentData *fragment = it.value();

        Q_ASSERT(formats.format(fragment->format).type() == QTextFormat::CharFormat);

        int offset = pos - it.position();
        int length = qMin(endPos - pos, int(fragment->size - offset));
        int oldFormat = fragment->format;

        if (mode == MergeFormat) {
            QTextFormat format = formats.format(fragment->format);
            format.merge(newFormat);
            fragment->format = formats.indexForFormat(format);
        } else {
            fragment->format = newFormatIdx;
        }

        QTextUndoCommand c = { QTextUndoCommand::CharFormatChanged, true, QTextUndoCommand::MoveCursor, oldFormat,
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

    QTextBlock blockIt = blocksFind(startPos);
    QTextBlock endIt = blocksFind(endPos);
    if (endIt.isValid())
        endIt = endIt.next();
    for (; blockIt.isValid() && blockIt != endIt; blockIt = blockIt.next())
        QTextDocumentPrivate::block(blockIt)->invalidate();

    documentChange(startPos, length);

    endEditBlock();
}

void QTextDocumentPrivate::setBlockFormat(const QTextBlock &from, const QTextBlock &to,
				     const QTextBlockFormat &newFormat, FormatChangeMode mode)
{
    beginEditBlock();

    Q_ASSERT(newFormat.isValid());

    int newFormatIdx = -1;
    if (mode == SetFormat)
        newFormatIdx = formats.indexForFormat(newFormat);
    QTextBlockGroup *group = qt_cast<QTextBlockGroup *>(objectForFormat(newFormat));

    QTextBlock it = from;
    QTextBlock end = to;
    if (end.isValid())
	end = end.next();

    for (; it != end; it = it.next()) {
        int oldFormat = block(it)->format;
        QTextBlockFormat format = formats.blockFormat(oldFormat);
        QTextBlockGroup *oldGroup = qt_cast<QTextBlockGroup *>(objectForFormat(format));
        if (mode == MergeFormat) {
            format.merge(newFormat);
            newFormatIdx = formats.indexForFormat(format);
            group = qt_cast<QTextBlockGroup *>(objectForFormat(format));
        }
        block(it)->format = newFormatIdx;

        block(it)->invalidate();

        QTextUndoCommand c = { QTextUndoCommand::BlockFormatChanged, true, QTextUndoCommand::MoveCursor, oldFormat,
                              0, it.position(), { 1 } };
        appendUndoItem(c);

        if (group != oldGroup) {
            if (oldGroup)
                oldGroup->blockRemoved(it);
            if (group)
                group->blockInserted(it);
        } else if (group) {
	    group->blockFormatChanged(it);
	}
    }

    documentChange(from.position(), to.position() + to.length() - from.position());

    endEditBlock();
}


bool QTextDocumentPrivate::split(int pos)
{
    uint x = fragments.findNode(pos);
    if (x) {
        int k = fragments.position(x);
//          qDebug("found fragment with key %d, size_left=%d, size=%d to split at %d",
//                k, (*it)->size_left, (*it)->size, pos);
        if (k != pos) {
            Q_ASSERT(k <= pos);
            // need to resize the first fragment and add a new one
            QTextFragmentData *X = fragments.fragment(x);
            int oldsize = X->size;
            fragments.setSize(x, pos-k);
            uint n = fragments.insert_single(pos, oldsize-(pos-k));
            X = fragments.fragment(x);
            QTextFragmentData *N = fragments.fragment(n);
            N->stringPosition = X->stringPosition + pos-k;
            N->format = X->format;
            return true;
        }
    }
    return false;
}

bool QTextDocumentPrivate::unite(uint f)
{
    uint n = fragments.next(f);
    if (!n)
        return false;

    QTextFragmentData *ff = fragments.fragment(f);
    QTextFragmentData *nf = fragments.fragment(n);

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


void QTextDocumentPrivate::undoRedo(bool undo)
{
    PMDEBUG("%s, undoPosition=%d, undoStack size=%d", undo ? "undo:" : "redo:", undoPosition, undoStack.size());
    if (!undoEnabled || (undo && undoPosition == 0) || (!undo && undoPosition == undoStack.size()))
        return;

    undoEnabled = false;
    beginEditBlock();
    while (1) {
        if (undo)
            --undoPosition;
        QTextUndoCommand &c = undoStack[undoPosition];

	switch(c.command) {
        case QTextUndoCommand::Inserted:
            remove(c.pos, c.length, (QTextUndoCommand::Operation)c.operation);
            PMDEBUG("   erase: from %d, length %d", c.pos, c.length);
            c.command = QTextUndoCommand::Removed;
	    break;
        case QTextUndoCommand::Removed:
            PMDEBUG("   insert: format %d (from %d, length %d, strpos=%d)", c.format, c.pos, c.length, c.strPos);
            insert_string(c.pos, c.strPos, c.length, c.format, (QTextUndoCommand::Operation)c.operation);
            c.command = QTextUndoCommand::Inserted;
	    break;
	case QTextUndoCommand::BlockInserted:
	case QTextUndoCommand::BlockAdded:
            remove_block(c.pos, &c.blockFormat, c.command, (QTextUndoCommand::Operation)c.operation);
            PMDEBUG("   blockremove: from %d", c.pos);
	    if (c.command == QTextUndoCommand::BlockInserted)
		c.command = QTextUndoCommand::BlockRemoved;
	    else
		c.command = QTextUndoCommand::BlockDeleted;
	    break;
	case QTextUndoCommand::BlockRemoved:
	case QTextUndoCommand::BlockDeleted:
            PMDEBUG("   blockinsert: charformat %d blockformat %d (pos %d, strpos=%d)", c.format, c.blockFormat, c.pos, c.strPos);
            insert_block(c.pos, c.strPos, c.format, c.blockFormat, (QTextUndoCommand::Operation)c.operation, c.command);
	    if (c.command == QTextUndoCommand::BlockRemoved)
		c.command = QTextUndoCommand::BlockInserted;
	    else
		c.command = QTextUndoCommand::BlockAdded;
	    break;
	case QTextUndoCommand::CharFormatChanged: {
            PMDEBUG("   charFormat: format %d (from %d, length %d)", c.format, c.pos, c.length);
            FragmentIterator it = find(c.pos);
            Q_ASSERT(!it.atEnd());

            int oldFormat = it.value()->format;
            setCharFormat(c.pos, c.length, formats.charFormat(c.format));
            c.format = oldFormat;
	    break;
	}
	case QTextUndoCommand::BlockFormatChanged: {
            PMDEBUG("   blockformat: format %d pos %d", c.format, c.pos);
            QTextBlock it = blocksFind(c.pos);
            Q_ASSERT(it.isValid());

            int oldFormat = block(it)->format;
            block(it)->format = c.format;
            QTextBlockGroup *oldGroup = qt_cast<QTextBlockGroup *>(objectForFormat(formats.blockFormat(oldFormat)));
            QTextBlockGroup *group = qt_cast<QTextBlockGroup *>(objectForFormat(formats.blockFormat(c.format)));
            c.format = oldFormat;
            if (group != oldGroup) {
                if (oldGroup)
                    oldGroup->blockRemoved(it);
                if (group)
                    group->blockInserted(it);
            } else if (group) {
                group->blockFormatChanged(it);
            }
            documentChange(it.position(), it.length());
	    break;
	}
	case QTextUndoCommand::GroupFormatChange: {
            PMDEBUG("   group format change");
            QTextObject *object = c.object;
            int oldFormat = formats.objectFormatIndex(object->objectIndex());
            changeObjectFormat(object, c.format);
            c.format = oldFormat;
	    break;
	}
	case QTextUndoCommand::Custom:
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
    emit q->undoAvailable(isUndoAvailable());
    emit q->redoAvailable(isRedoAvailable());
}

/*!
    Appends a custom undo \a item to the undo stack.
*/
void QTextDocumentPrivate::appendUndoItem(QAbstractUndoItem *item)
{
    if (!undoEnabled) {
        delete item;
        return;
    }

    QTextUndoCommand c;
    c.command = QTextUndoCommand::Custom;
    c.block = editBlock != 0;
    c.operation = QTextUndoCommand::MoveCursor;
    c.format = 0;
    c.strPos = 0;
    c.pos = 0;
    c.blockFormat = 0;

    c.custom = item;
    appendUndoItem(c);
}

void QTextDocumentPrivate::appendUndoItem(const QTextUndoCommand &c)
{
    PMDEBUG("appendUndoItem, command=%d enabled=%d", c.command, undoEnabled);
    if (!undoEnabled)
        return;
    if (undoPosition < undoStack.size())
        truncateUndoStack();

    if (!undoStack.isEmpty()) {
        QTextUndoCommand &last = undoStack[undoPosition - 1];
        if (last.tryMerge(c))
            return;
    }
    undoStack.append(c);
    undoPosition++;
    emit q->undoAvailable(true);
    emit q->redoAvailable(false);
}

void QTextDocumentPrivate::truncateUndoStack() {
    if (undoPosition == undoStack.size())
        return;

    for (int i = undoPosition; i < undoStack.size(); ++i) {
        QTextUndoCommand c = undoStack[i];
        if (c.command & QTextUndoCommand::Removed) {
            // ########
//             QTextFragment *f = c.fragment_list;
//             while (f) {
//                 QTextFragment *n = f->right;
//                 delete f;
//                 f = n;
//             }
        } else if (c.command & QTextUndoCommand::Custom) {
            delete c.custom;
        }
    }
    undoStack.resize(undoPosition);
}

void QTextDocumentPrivate::enableUndoRedo(bool enable)
{
    if (!enable) {
        undoPosition = 0;
        truncateUndoStack();

        lastUnmodifiedUndoStackPos = -1;
        emit q->undoAvailable(false);
        emit q->redoAvailable(false);
        setModified(false);
    }
    undoEnabled = enable;
}

void QTextDocumentPrivate::joinPreviousEditBlock()
{
    beginEditBlock();

    if (undoEnabled && undoPosition)
        undoStack[undoPosition - 1].block = true;
}

void QTextDocumentPrivate::endEditBlock()
{
    if (--editBlock)
        return;

    if (undoEnabled && undoPosition > 0)
        undoStack[undoPosition - 1].block = false;

    if (framesDirty)
        scan_frames(docChangeFrom, docChangeOldLength, docChangeLength);

    if (lout && docChangeFrom >= 0)
        lout->documentChange(docChangeFrom, docChangeOldLength, docChangeLength);

    docChangeFrom = -1;

    while (!changedCursors.isEmpty()) {
        QTextCursorPrivate *curs = changedCursors.takeFirst();
        emit q->cursorPositionChanged(QTextCursor(curs));
    }

    contentsChanged();
}

void QTextDocumentPrivate::documentChange(int from, int length)
{
//     qDebug("QTextDocumentPrivate::documentChange: from=%d,length=%d", from, length);
    if (docChangeFrom < 0) {
        docChangeFrom = from;
        docChangeOldLength = length;
        docChangeLength = length;
        return;
    }
    int start = qMin(from, docChangeFrom);
    int end = qMax(from + length, docChangeFrom + docChangeLength);
    int diff = qMax(0, end - from - docChangeLength);
    docChangeFrom = start;
    docChangeOldLength += diff;
    docChangeLength += diff;
}

void QTextDocumentPrivate::adjustDocumentChangesAndCursors(int from, int addedOrRemoved, QTextUndoCommand::Operation op)
{
    for (int i = 0; i < cursors.size(); ++i) {
        QTextCursorPrivate *curs = cursors.at(i);
        if (curs->adjustPosition(from, addedOrRemoved, op) == QTextCursorPrivate::CursorMoved) {
            if (editBlock)
                changedCursors.append(curs);
            else
                emit q->cursorPositionChanged(QTextCursor(curs));
        }
    }

//     qDebug("QTextDocumentPrivate::adjustDocumentChanges: from=%d,addedOrRemoved=%d", from, addedOrRemoved);
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
        contentsChanged();
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

    contentsChanged();
}


QString QTextDocumentPrivate::plainText() const
{
    QString result;
    for (QTextDocumentPrivate::FragmentIterator it = begin(); it != end(); ++it) {
        const QTextFragmentData *f = *it;
        result += QString(text.unicode() + f->stringPosition, f->size);
    }
    // remove trailing block separator
    result.chop(1);
    return result;
}



int QTextDocumentPrivate::nextCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == length()-1)
        return position;

    QTextBlock it = blocksFind(position);
    int start = it.position();
    int end = start + it.length() - 1;
    if (position == end)
        return end + 1;

    return it.layout()->nextCursorPosition(position-start, mode) + start;
}

int QTextDocumentPrivate::previousCursorPosition(int position, QTextLayout::CursorMode mode) const
{
    if (position == 0)
        return position;

    QTextBlock it = blocksFind(position);
    int start = it.position();
    if (position == start)
        return start - 1;

    return it.layout()->previousCursorPosition(position-start, mode) + start;
}

void QTextDocumentPrivate::changeObjectFormat(QTextObject *obj, int format)
{
    beginEditBlock();
    int objectIndex = obj->objectIndex();
    int oldFormatIndex = formats.objectFormatIndex(objectIndex);
    formats.setObjectFormatIndex(objectIndex, format);

    QTextBlockGroup *b = qt_cast<QTextBlockGroup *>(obj);
    if (b) {
        QList<QTextBlock> blocks = b->blockList();
        for (int i = 0; i < blocks.size(); ++i) {
            // invalidate blocks and tell layout
            const QTextBlock &block = blocks.at(i);
            documentChange(block.position(), block.length());
        }
    }
    QTextFrame *f = qt_cast<QTextFrame *>(obj);
    if (f)
        documentChange(f->firstPosition(), f->lastPosition() - f->firstPosition());

    QTextUndoCommand c = { QTextUndoCommand::GroupFormatChange, true, QTextUndoCommand::MoveCursor, oldFormatIndex,
                      0, 0, { 1 } };
    c.object = obj;
    appendUndoItem(c);

    endEditBlock();
}

static QTextFrame *findChildFrame(QTextFrame *f, int pos)
{
    // ##### use binary search
    QList<QTextFrame *> children = f->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        if (pos >= c->firstPosition() && pos <= c->lastPosition())
            return c;
    }
    return 0;
}

QTextFrame *QTextDocumentPrivate::frameAt(int pos) const
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

void QTextDocumentPrivate::clearFrame(QTextFrame *f)
{
    for (int i = 0; i < f->d->childFrames.count(); ++i)
        clearFrame(f->d->childFrames.at(i));
    f->d->childFrames.clear();
    f->d->parentFrame = 0;
}

void QTextDocumentPrivate::scan_frames(int pos, int charsRemoved, int charsAddded)
{
    // ###### optimise
    Q_UNUSED(pos);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAddded);

    QTextFrame *f = frame;
    clearFrame(f);

    for (FragmentIterator it = begin(); it != end(); ++it) {
        // QTextFormat fmt = formats.format(it->format);
        QTextFrame *frame = qt_cast<QTextFrame *>(objectForFormat(it->format));
        if (!frame)
            continue;

        Q_ASSERT(it.size() == 1);
        QChar ch = text.at(it->stringPosition);

        if (ch == QTextBeginningOfFrame) {
            if (f != frame) {
                // f == frame happens for tables
                Q_ASSERT(frame->d->fragment_start == it.n || frame->d->fragment_start == 0);
                frame->d->parentFrame = f;
                f->d->childFrames.append(frame);
                f = frame;
            }
        } else if (ch == QTextEndOfFrame) {
            Q_ASSERT(f == frame);
            Q_ASSERT(frame->d->fragment_end == it.n || frame->d->fragment_end == 0);
            f = frame->d->parentFrame;
        } else if (ch == QChar::ObjectReplacementCharacter) {
            Q_ASSERT(f != frame);
            Q_ASSERT(frame->d->fragment_start == it.n || frame->d->fragment_start == 0);
            Q_ASSERT(frame->d->fragment_end == it.n || frame->d->fragment_end == 0);
            frame->d->parentFrame = f;
            f->d->childFrames.append(frame);
        } else {
            Q_ASSERT(false);
        }
    }
    Q_ASSERT(f == frame);
}

void QTextDocumentPrivate::insert_frame(QTextFrame *f)
{
    int start = f->firstPosition();
    int end = f->lastPosition();
    QTextFrame *parent = frameAt(start-1);
    Q_ASSERT(parent == frameAt(end+1));

    if (start != end) {
        // iterator over the parent and move all children contained in my frame to myself
        for (int i = 0; i < parent->d->childFrames.size(); ++i) {
            QTextFrame *c = parent->d->childFrames.at(i);
            if (start < c->firstPosition() && end > c->lastPosition()) {
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
        if (c->firstPosition() > end)
            break;
    }
    parent->d->childFrames.insert(i, f);
    f->d->parentFrame = parent;
}

QTextFrame *QTextDocumentPrivate::insertFrame(int start, int end, const QTextFrameFormat &format)
{
    Q_ASSERT(start >= 0 && start < length());
    Q_ASSERT(end >= 0 && end < length());
    Q_ASSERT(start <= end || end == -1);

    if (start != end && frameAt(start) != frameAt(end))
        return 0;

    beginEditBlock();

    QTextFrame *frame = qt_cast<QTextFrame *>(createObject(format));
    Q_ASSERT(frame);

    // #### using the default block and char format below might be wrong
    int idx = formats.indexForFormat(QTextBlockFormat());
    QTextCharFormat cfmt;
    cfmt.setObjectIndex(frame->objectIndex());
    int charIdx = formats.indexForFormat(cfmt);

    insertBlock(QTextBeginningOfFrame, start, idx, charIdx, QTextUndoCommand::MoveCursor);
    insertBlock(QTextEndOfFrame, ++end, idx, charIdx, QTextUndoCommand::KeepCursor);

    frame->d_func()->fragment_start = find(start).n;
    frame->d_func()->fragment_end = find(end).n;

    insert_frame(frame);
    framesDirty = false;

    endEditBlock();

    return frame;
}

void QTextDocumentPrivate::removeFrame(QTextFrame *frame)
{
    QTextFrame *parent = frame->d->parentFrame;
    if (!parent)
        return;

    int start = frame->firstPosition();
    int end = frame->lastPosition();
    Q_ASSERT(end >= start);

    beginEditBlock();

    // remove already removes the frames from the tree
    remove(end, 1);
    remove(start-1, 1);

    endEditBlock();
}

QTextObject *QTextDocumentPrivate::objectForIndex(int objectIndex) const
{
    if (objectIndex < 0)
        return 0;

    QTextObject *object = objects.value(objectIndex, 0);
    if (!object) {
        QTextDocumentPrivate *that = const_cast<QTextDocumentPrivate *>(this);
        QTextFormat fmt = formats.objectFormat(objectIndex);
        object = that->createObject(fmt, objectIndex);
    }
    return object;
}

QTextObject *QTextDocumentPrivate::objectForFormat(int formatIndex) const
{
    int objectIndex = formats.format(formatIndex).objectIndex();
    return objectForIndex(objectIndex);
}

QTextObject *QTextDocumentPrivate::objectForFormat(const QTextFormat &f) const
{
    return objectForIndex(f.objectIndex());
}

QTextObject *QTextDocumentPrivate::createObject(const QTextFormat &f, int objectIndex)
{
    QTextObject *obj = document()->createObject(f);

    if (obj) {
        obj->d_func()->pieceTable = this;
        obj->d_func()->objectIndex = objectIndex == -1 ? formats.createObjectIndex(f) : objectIndex;
        objects[obj->d_func()->objectIndex] = obj;
    }

    return obj;
}

void QTextDocumentPrivate::contentsChanged()
{
    if (editBlock)
        return;

    if (lastUnmodifiedUndoStackPos != -1
        && lastUnmodifiedUndoStackPos == undoPosition) {
        lastUnmodifiedUndoStackPos = undoPosition;
        setModified(false);
    } else {
        setModified(true);
    }

    emit q->contentsChanged();
}

void QTextDocumentPrivate::setModified(bool m)
{
    if (m == modified)
        return;

    modified = m;

    emit q->modificationChanged(modified);
}

