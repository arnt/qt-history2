#ifndef Q_TEXTPIECETABLE_H
#define Q_TEXTPIECETABLE_H

#ifndef QT_H
#include "qglobal.h"
#include <qstring.h>
#include <qvector.h>
#include <qlist.h>
#include <qobject.h>
#include "qfragmentmap_p.h"
#include <qtextlayout.h>
#include <private/qtextformat_p.h>
#include <qtextdocument.h>
#include <qshareddatapointer.h>

#include "qtextglobal_p.h"
#include <qtextblockiterator.h>
#endif // QT_H
// #define QT_QMAP_DEBUG

#ifdef QT_QMAP_DEBUG
#include <iostream>
#endif

class QTextFormatCollection;
class QTextFormat;
class QTextListManager;
class QTextTableManager;
class QTextBlockFormat;
class QTextCursorPrivate;
class QAbstractTextDocumentLayout;

class QTextFragment : public QFragment
{
public:
    inline void initialize() {}
    inline void invalidate() const {}
    inline void free() {}
    int position;
    int format;
};

class QTextBlock : public QFragment
{
public:
    inline void initialize()
    { layout = 0; layoutDirty = true; textDirty = true; rect = QRect(); }
    inline void invalidate() const
    { layoutDirty = true; textDirty = true; }
    inline void free()
    { if (layoutDirty) delete layout; }

    // ##### probably store a QTextEngine * here!
    mutable QTextLayout *layout;
    mutable bool layoutDirty;
    mutable bool textDirty;
    mutable QRect rect;
    mutable int format;
};


class QAbstractUndoItem;

class UndoCommand
{
public:
    enum Command {
	Inserted = 0,
	Removed = 1,
	CharFormatChanged = 2,
	BlockFormatChanged = 3,
	BlockInserted = 4,
	BlockRemoved = 5,
	Custom = 8
    };
    enum Operation {
	KeepCursor = 0,
	MoveCursor = 1
    };
    Q_UINT16 command : 16;
    Q_UINT16 block : 1;
    Q_UINT16 operation : 2;
    int format;
    Q_UINT32 strPos;
    Q_UINT32 pos;
    union {
	int blockFormat;
	Q_UINT32 length;
	QAbstractUndoItem *custom;
    };

    bool tryMerge(const UndoCommand &other);
};
Q_DECLARE_TYPEINFO(UndoCommand, Q_PRIMITIVE_TYPE);


class QTextPieceTable : public QObject, public QSharedObject
{
    Q_OBJECT
public:
    typedef QFragmentMap<QTextFragment> FragmentMap;
    typedef FragmentMap::ConstIterator FragmentIterator;
    typedef QFragmentMap<QTextBlock> BlockMap;

    QTextPieceTable(QAbstractTextDocumentLayout *layout);
    ~QTextPieceTable();

    void insert(int pos, const QString &text, int format = -1);
    void insert(int pos, int strPos, int strLength, int format = -1);
    void insertBlock(int pos, int blockFormat, int charFormat);
    void remove(int pos, int length, UndoCommand::Operation = UndoCommand::MoveCursor);

    enum FormatChangeMode { MergeFormat, SetFormat };

    void setCharFormat(int pos, int length, const QTextCharFormat &newFormat, FormatChangeMode mode = SetFormat);
    void setBlockFormat(int pos, int length, const QTextBlockFormat &newFormat, FormatChangeMode mode = SetFormat);

    void undoRedo(bool undo);
    inline void undo() { undoRedo(true); }
    inline void redo() { undoRedo(false); }
    void appendUndoItem(QAbstractUndoItem *);
    void truncateUndoStack();
    inline void beginEditBlock() { editBlock++; }
    void endEditBlock();
    void enableUndoRedo(bool enable);
    inline bool isUndoRedoEnabled() const { return undoEnabled; }

    inline bool isUndoAvailable() const { return undoEnabled && undoPosition > 0; }
    inline bool isRedoAvailable() const { return undoEnabled && undoPosition < undoStack.size(); }

    inline QString buffer() const { return text; }
    QString plainText() const;
    inline int length() const { return fragments.length(); }

    inline QTextFormatCollection *formatCollection() { return formats; }
    inline const QTextFormatCollection *formatCollection() const { return formats; }
    inline QTextListManager *listManager() const { return lists; }
    inline QTextTableManager *tableManager() const { return tables; }
    inline QAbstractTextDocumentLayout *layout() const { return lout; }

    inline FragmentIterator find(int pos) const { return fragments.find(pos); }
    inline FragmentIterator begin() const { return fragments.begin(); }
    inline FragmentIterator end() const { return fragments.end(); }

    inline QTextBlockIterator blocksBegin() const { return QTextBlockIterator(this, blocks.firstNode()); }
    inline QTextBlockIterator blocksEnd() const { return QTextBlockIterator(this, 0); }
    inline QTextBlockIterator blocksFind(int pos) const { return QTextBlockIterator(this, blocks.findNode(pos)); }
    inline int numBlocks() const { return blocks.numNodes(); }

    const BlockMap &blockMap() const { return blocks; }
    const FragmentMap &fragmentMap() const { return fragments; }

    static const QTextBlock *block(const QTextBlockIterator &it) { return it.pt->blocks.fragment(it.n); }
    static void setBlockFormat(const QTextBlockIterator &it, const QTextBlockFormat &format);

    inline QTextDocumentConfig *config() { return &docConfig; }
    inline const QTextDocumentConfig *config() const { return &docConfig; }

    int nextCursorPosition(int position, QTextLayout::CursorMode mode) const;
    int previousCursorPosition(int position, QTextLayout::CursorMode mode) const;

signals:
    void contentsChanged();
    void textChanged(int positionOfChange, int charsAddedOrRemoved);
    void formatChanged(int position, int length);
    void blockChanged(int blockPosition, QText::ChangeOperation);

private:
    bool split(int pos);
    bool unite(uint f);

    void removeBlocks(int pos, int length);

    void insert_string(int pos, uint strPos, uint length, int format, UndoCommand::Operation op);
    void insert_block(int pos, uint strPos, int format, int blockformat, UndoCommand::Operation op);
    int remove_string(int pos, uint length);
    int remove_block(int pos);

    void adjustDocumentChanges(int from, int addedOrRemoved);

public:
    inline void addCursor(QTextCursorPrivate *c) { cursors.append(c); }
    inline void removeCursor(QTextCursorPrivate *c) { cursors.remove(c); }

private:
    QTextPieceTable(const QTextPieceTable& m);
    QTextPieceTable& operator= (const QTextPieceTable& m);

    void appendUndoItem(const UndoCommand &c);

    QString text;

    QVector<UndoCommand> undoStack;
    bool undoEnabled;
    int undoPosition;

    int editBlock;
    int docChangeFrom;
    int docChangeOldLength;
    int docChangeLength;

    QTextFormatCollection *formats;
    QTextListManager *lists;
    QTextTableManager *tables;
    QAbstractTextDocumentLayout *lout;
    FragmentMap fragments;
    BlockMap blocks;

    QList<QTextCursorPrivate*> cursors;

    QTextDocumentConfig docConfig;
};

#endif // QPIECEMAP_H
