#ifndef QTEXTOBJECT_H
#define QTEXTOBJECT_H

#include <qobject.h>
#include <qtextformat.h>

class QTextObjectPrivate;
class QTextDocument;
class QTextBlockIterator;
class QTextCursor;
class QTextBlock;

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

    QTextDocument *document() const;

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

    QTextCursor first() const;
    QTextCursor last() const;
    int firstPosition() const;
    int lastPosition() const;

    QTextFrameLayoutData *layoutData() const;
    void setLayoutData(QTextFrameLayoutData *data);

    QList<QTextFrame *> childFrames();
    QTextFrame *parentFrame();

    class iterator {
        const QTextFrame *f;
        const QTextFrame *cf;
        int cb;
        friend class QTextFrame;
        iterator(const QTextFrame *frame, int b);
    public:
        iterator();
        iterator(const QTextFrame *frame);
        iterator(const iterator &o);

        const QTextFrame *parentFrame() const { return f; }

        const QTextFrame *currentFrame() const;
        QTextBlock currentBlock() const;

        inline bool operator==(const iterator &o) const { return f == o.f && cf == o.cf && cb == o.cb; }
        inline bool operator!=(const iterator &o) const { return f != o.f || cf != o.cf || cb != o.cb; }
        iterator operator++();
        inline iterator operator++(int) { iterator tmp = *this; operator++(); return tmp; }
        iterator operator--();
        inline iterator operator--(int) { iterator tmp = *this; operator--(); return tmp; }
    };

    iterator begin() const;
    iterator end() const;

protected:
    QTextFrame(QTextFramePrivate &p, QTextDocument *doc);
};



class QTextBlock
{
    const QTextDocumentPrivate *p;
    int n;
public:
    inline QTextBlock(const QTextDocumentPrivate *priv, int b) : p(priv), n(b) {}
    inline QTextBlock() : p(0), n(0) {}
    inline QTextBlock(const QTextBlock &o) : p(o.p), n(o.n) {}
    inline QTextBlock &operator=(const QTextBlock &o) { p = o.p; n = o.n; return *this; }

    bool isValid() const { return p != 0 && n != 0; }

    int position() const;
    int length() const;
    bool contains(int position) const;

    QTextLayout *layout() const;
    QTextBlockFormat blockFormat() const;

    QString blockText() const;

    const QTextDocument *document() const;
};

Q_DECLARE_TYPEINFO(QTextBlock, Q_PRIMITIVE_TYPE);


#endif
