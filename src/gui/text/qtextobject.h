#ifndef QTEXTOBJECT_H
#define QTEXTOBJECT_H

#include <qobject.h>
#include <qtextformat.h>

class QTextObjectPrivate;
class QTextDocument;
class QTextBlockIterator;
class QTextCursor;

class Q_GUI_EXPORT QTextObject : public QObject
{
    Q_DECLARE_PRIVATE(QTextObject)
    Q_OBJECT
    friend class QTextDocumentPrivate;

protected:
    QTextObject(QTextDocument *doc);
    ~QTextObject();
    QTextObject(QTextObjectPrivate &p, QTextDocument *doc);

public:
    int formatType() const;
    QTextFormat format() const;
    void setFormat(const QTextFormat &format);

    int objectIndex() const;
};

class QTextBlockGroupPrivate;

class QTextBlockGroup : public QTextObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextBlockGroup)
    friend class QTextDocumentPrivate;

protected:
    QTextBlockGroup(QTextDocument *doc);
    QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc);
    ~QTextBlockGroup();

    virtual void insertBlock(const QTextBlockIterator &block);
    virtual void removeBlock(const QTextBlockIterator &block);
    virtual void blockFormatChanged(const QTextBlockIterator &block);

    QList<QTextBlockIterator> blockList() const;
};

class QTextFrameLayoutData {
public:
    virtual ~QTextFrameLayoutData();
};

class QTextFramePrivate;

class QTextFrame : public QTextObject
{
    Q_DECLARE_PRIVATE(QTextFrame)
    Q_OBJECT
    friend class QTextDocumentPrivate;

public:
    QTextFrame(QTextDocument *doc);
    ~QTextFrame();

    void setFormat(const QTextFrameFormat &format) { QTextObject::setFormat(format); }
    QTextFrameFormat format() const { return QTextObject::format().toFrameFormat(); }

    QTextCursor start();
    QTextCursor end();
    int startPosition();
    int endPosition();

    QTextFrameLayoutData *layoutData() const;
    void setLayoutData(QTextFrameLayoutData *data);

    QList<QTextFrame *> children();
    QTextFrame *parent();

protected:
    QTextFrame(QTextFramePrivate &p, QTextDocument *doc);
};

#endif
