#ifndef Q_TEXTPIECETABLE_H
#define Q_TEXTPIECETABLE_H

#ifndef QT_H
#include "qglobal.h"
#include <qstring.h>
#include <qvector.h>
#include <qlist.h>
#include <qobject.h>
#include "qfragmentmap_p.h"
#include <private/qtextlayout_p.h>
#include <private/qtextformat_p.h>
#include <qshareddatapointer.h>

#include "qtextglobal_p.h"
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
class QTextObjectManager;
class QTextCursorPrivate;
class QTextDocumentLayout;

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
};


class QAbstractUndoItem;

class UndoCommand
{
public:
    enum Command {
	Inserted = 0,
	Removed = 1,
	FormatChanged = 2,
	Custom = 4
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

    class BlockIterator : public QFragmentMap<QTextBlock>::ConstIterator
    {
	const QTextPieceTable *pt;
    public:
	inline BlockIterator(const QFragmentMap<QTextBlock>::ConstIterator i, const QTextPieceTable *p)
	    : QFragmentMap<QTextBlock>::ConstIterator(i),  pt(p) {}

	inline const QTextPieceTable *pieceTable() const { return pt; }
	QTextLayout *layout() const;

	QString blockText() const;
	int start() const;
	int end() const;

	void setBlockFormat(const QTextBlockFormat &format);
	QTextBlockFormat blockFormat() const;
	int blockFormatIndex() const;
	inline bool contains(int position) const
	    { return position >= start() && position <= end(); }
    };


    QTextPieceTable();
    ~QTextPieceTable();

    void insert(int pos, const QString &text, int format = -1);
    void insert(int pos, int strPos, int strLength, int format = -1);
    void insertBlockSeparator(int pos, int blockFormat);
    void remove(int pos, int length, UndoCommand::Operation = UndoCommand::MoveCursor);

    enum FormatChangeMode { MergeFormat, SetFormat };

    void setFormat(int pos, int length, const QTextFormat &newFormat, FormatChangeMode mode = SetFormat);

    void undoRedo(bool undo);
    inline void undo() { undoRedo(true); }
    inline void redo() { undoRedo(false); }
    void appendUndoItem(QAbstractUndoItem *);
    void truncateUndoStack();
    inline void beginUndoBlock() { undoBlock++; }
    void endUndoBlock();
    void enableUndoRedo(bool enable);
    inline bool isUndoRedoEnabled() const { return undoEnabled; }

    inline QString buffer() const { return text; }
    QString plainText() const;
    inline int length() const { return fragments.length(); }

    inline QTextFormatCollection *formatCollection() { return &formats; }
    inline const QTextFormatCollection *formatCollection() const { return &formats; }
    inline QTextListManager *listManager() const { return lists; }
    inline QTextTableManager *tableManager() const { return tables; }
    inline QTextObjectManager *objectManager() const { return objects; }
    inline QTextDocumentLayout *layout() const { return lout; }

    inline FragmentIterator find(int pos) const { return fragments.find(pos); }
    inline FragmentIterator begin() const { return fragments.begin(); }
    inline FragmentIterator end() const { return fragments.end(); }

    inline BlockIterator blocksBegin() const { return BlockIterator(blocks.begin(), this); }
    inline BlockIterator blocksEnd() const { return BlockIterator(blocks.end(), this); }
    inline BlockIterator blocksFind(int pos) const { return BlockIterator(blocks.find(pos), this); }
    inline int numBlocks() const { return blocks.numNodes(); }

    inline QTextDocumentConfig *config() { return &docConfig; }
    inline const QTextDocumentConfig *config() const { return &docConfig; }

    int nextCursorPosition(int position, QTextLayout::CursorMode mode) const;
    int previousCursorPosition(int position, QTextLayout::CursorMode mode) const;

signals:
    void contentsChanged();
    void textChanged(int positionOfChange, int charsAddedOrRemoved, UndoCommand::Operation op);
    void formatChanged(int position, int length);
    void blockChanged(int blockPosition, bool blockAdded);

private:
    bool split(int pos);
    bool unite(uint f);

    void insertBlock(int pos);
    void removeBlocks(int pos, int length);

    void insertWithoutUndo(int pos, uint strPos, uint length, int format, UndoCommand::Operation op);

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
    int undoBlock;
    int undoPosition;

    QTextFormatCollection formats;
    QTextListManager *lists;
    QTextTableManager *tables;
    QTextObjectManager *objects;
    QTextDocumentLayout *lout;
    FragmentMap fragments;
    BlockMap blocks;

    QList<QTextCursorPrivate*> cursors;

    QTextDocumentConfig docConfig;
};

#endif // QPIECEMAP_H
