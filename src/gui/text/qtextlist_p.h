#ifndef QTEXTLIST_P_H
#define QTEXTLIST_P_H

#ifndef QT_H
#include "qtextpiecetable_p.h"
#include "qtextdocument.h"
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

    typedef QList<QTextPieceTable::BlockIterator> BlockList;
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

class QTextFormatReferenceChangeCommand : public QAbstractUndoItem
{
public:
    QTextFormatReferenceChangeCommand(QTextPieceTable *table, int reference, const QTextFormat &format);

    virtual void undo();
    virtual void redo();

private:
    QTextFormatCollection *formatCollection;
    int reference;
    QTextFormat format;
};

#endif // QTEXTLIST_P_H
