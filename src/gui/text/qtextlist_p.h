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
    int itemNumber(const QTextBlockIterator &block) const;

    void appendBlock(const QTextBlockIterator &block);

    QTextFormatGroup *group() const;

    void removeAllFormatIndicesFromBlocks();

    typedef QVector<QTextBlockIterator> BlockList;
    BlockList blocks;
};

class QTextListItem
{
public:
    inline QTextListItem() : item(-1) {}
    QTextListItem(const QTextBlockIterator &block);
    QTextListItem(QTextList *_list, int _item);

    QString text() const;

    inline int itemNumber() const { return item; }

private:
    QPointer<QTextList> list;
    int item;
};

template <class Manager>
class QTextFormatGroupChangeCommand : public QAbstractUndoItem
{
public:
    QTextFormatGroupChangeCommand(Manager *_manager, QTextFormatGroup *g, const QTextFormat &newFormat)
	: manager(_manager), group(g), format(newFormat)
    {}

    virtual void undo()
    {
	if (!manager)
	    return;

	QTextPieceTable *pt = manager->pieceTable();

	QTextFormat oldFormat = group->commonFormat();
	group->setCommonFormat(format);
	format = oldFormat;

	QAbstractTextDocumentLayout *layout = pt->layout();

	QVector<QTextBlockIterator> affectedBlocks = manager->blocksForObject(group);
	for (int i = 0; i < affectedBlocks.size(); ++i) {
	    const QTextBlockIterator &block = affectedBlocks.at(i);
	    int start = block.position();
	    int len = block.length() - 1;
	    layout->documentChange(start, len, len);
	}
    }

    virtual void redo()
    { undo(); }

private:
    QPointer<Manager> manager;
    int objectId;
    QTextFormatGroup *group;
    QTextFormat format;
};

#endif // QTEXTLIST_P_H
