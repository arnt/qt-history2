#ifndef QTEXTLIST_P_H
#define QTEXTLIST_P_H

#ifndef QT_H
#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
#include "qabstracttextdocumentlayout.h"
#include <qpointer.h>
#include <private/qobject_p.h>
#endif // QT_H

class QTextList;

class QTextListPrivate : public QObjectPrivate
{
public:
    int itemNumber(const QTextPieceTable::BlockIterator &block) const;

    void appendBlock(const QTextPieceTable::BlockIterator &block);

    int listFormatIndex() const;

    void removeAllFormatIndicesFromBlocks();

    typedef QVector<QTextPieceTable::BlockIterator> BlockList;
    BlockList blocks;
};

class QTextListItem
{
public:
    inline QTextListItem() : item(-1) {}
    QTextListItem(const QTextPieceTable::BlockIterator &block);
    QTextListItem(QTextList *_list, int _item);

    QString text() const;

    inline int itemNumber() const { return item; }

private:
    QPointer<QTextList> list;
    int item;
};

template <class Manager>
class QTextFormatReferenceChangeCommand : public QAbstractUndoItem
{
public:
    QTextFormatReferenceChangeCommand(Manager *_manager, int _objectId, int _reference, const QTextFormat &newFormat)
	: manager(_manager), objectId(_objectId), reference(_reference), format(newFormat)
    {}

    virtual void undo()
    {
	if (!manager)
	    return;

	QTextPieceTable *pt = manager->pieceTable();

	format = pt->formatCollection()->updateReferenceIndex(reference, format);

	QAbstractTextDocumentLayout *layout = pt->layout();

	QVector<QTextPieceTable::BlockIterator> affectedBlocks = manager->blocksForObject(objectId);
	for (int i = 0; i < affectedBlocks.size(); ++i) {
	    const QTextPieceTable::BlockIterator &block = affectedBlocks.at(i);
	    int start = block.start();
	    int len = block.end() - start;
	    layout->documentChange(start, len, len);
	}
    }

    virtual void redo()
    { undo(); }

private:
    QPointer<Manager> manager;
    int objectId;
    int reference;
    QTextFormat format;
};

#endif // QTEXTLIST_P_H
