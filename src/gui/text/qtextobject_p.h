#ifndef QTEXTOBJECT_P_H
#define QTEXTOBJECT_P_H

#include "qtextobject.h"
#include <private/qobject_p.h>

class QTextDocumentPrivate;

class QTextObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextObject)
public:
    QTextDocumentPrivate *pieceTable;
    int objectIndex;
};

class QTextBlockGroupPrivate : public QTextObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextBlockGroup)
public:

    typedef QList<QTextBlockIterator> BlockList;
    BlockList blocks;
};

class QTextFrameLayoutData;

class QTextFramePrivate : public QTextObjectPrivate
{
    friend class QTextDocumentPrivate;
    Q_DECLARE_PUBLIC(QTextFrame)
public:

    virtual void fragmentAdded(const QChar &type, uint fragment);
    virtual void fragmentRemoved(const QChar &type, uint fragment);
    void remove_me();

    uint fragment_start;
    uint fragment_end;

    QTextFrame *parentFrame;
    QList<QTextFrame *> childFrames;
    QTextFrameLayoutData *layoutData;
};


#endif
