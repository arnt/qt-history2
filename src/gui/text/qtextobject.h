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

#ifndef QTEXTOBJECT_H
#define QTEXTOBJECT_H

#include <qobject.h>
#include <qtextformat.h>

class QTextObjectPrivate;
class QTextDocument;
class QTextDocumentPrivate;
class QTextCursor;
class QTextBlock;
class QTextFragment;
class QTextLayout;

class Q_GUI_EXPORT QTextObject : public QObject
{
    Q_DECLARE_PRIVATE(QTextObject)
    Q_OBJECT
    friend class QTextDocumentPrivate;

protected:
    QTextObject(QTextDocument *doc);
    ~QTextObject();
    QTextObject(QTextObjectPrivate &p, QTextDocument *doc);

    void setFormat(const QTextFormat &format);
public:
    int formatType() const;
    QTextFormat format() const;

    QTextDocument *document() const;

    int objectIndex() const;

    QTextDocumentPrivate *docHandle() const;
};

class QTextBlockGroupPrivate;

class Q_GUI_EXPORT QTextBlockGroup : public QTextObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextBlockGroup)
    friend class QTextDocumentPrivate;

protected:
    QTextBlockGroup(QTextDocument *doc);
    QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc);
    ~QTextBlockGroup();

    virtual void blockInserted(const QTextBlock &block);
    virtual void blockRemoved(const QTextBlock &block);
    virtual void blockFormatChanged(const QTextBlock &block);

    QList<QTextBlock> blockList() const;
};

class Q_GUI_EXPORT QTextFrameLayoutData {
public:
    virtual ~QTextFrameLayoutData();
};

class QTextFramePrivate;

class Q_GUI_EXPORT QTextFrame : public QTextObject
{
    Q_DECLARE_PRIVATE(QTextFrame)
    Q_OBJECT
    friend class QTextDocumentPrivate;

public:
    QTextFrame(QTextDocument *doc);
    ~QTextFrame();

    void setFormat(const QTextFrameFormat &format) { QTextObject::setFormat(format); }
    QTextFrameFormat format() const { return QTextObject::format().toFrameFormat(); }

    QTextCursor firstCursorPosition() const;
    QTextCursor lastCursorPosition() const;
    int firstPosition() const;
    int lastPosition() const;

    QTextFrameLayoutData *layoutData() const;
    void setLayoutData(QTextFrameLayoutData *data);

    QList<QTextFrame *> childFrames();
    QTextFrame *parentFrame();

    class Q_GUI_EXPORT iterator {
        QTextFrame *f;
        int b;
        int e;
        QTextFrame *cf;
        int cb;

        friend class QTextFrame;
        friend class QTextTableCell;
        iterator(QTextFrame *frame, int block, int begin, int end);
    public:
        iterator();
        iterator(const iterator &o);

        QTextFrame *parentFrame() const { return f; }

        QTextFrame *currentFrame() const;
        QTextBlock currentBlock() const;

        bool atEnd() const { return !cf && cb == e; }

        inline bool operator==(const iterator &o) const { return f == o.f && cf == o.cf && cb == o.cb; }
        inline bool operator!=(const iterator &o) const { return f != o.f || cf != o.cf || cb != o.cb; }
        iterator operator++();
        inline iterator operator++(int) { iterator tmp = *this; operator++(); return tmp; }
        iterator operator--();
        inline iterator operator--(int) { iterator tmp = *this; operator--(); return tmp; }
    };

    // more Qt
    typedef iterator Iterator;

    iterator begin() const;
    iterator end() const;

protected:
    QTextFrame(QTextFramePrivate &p, QTextDocument *doc);
};

class Q_GUI_EXPORT QTextBlock
{
    QTextDocumentPrivate *p;
    int n;
    friend class QTextDocumentPrivate;
public:
    inline QTextBlock(QTextDocumentPrivate *priv, int b) : p(priv), n(b) {}
    inline QTextBlock() : p(0), n(0) {}
    inline QTextBlock(const QTextBlock &o) : p(o.p), n(o.n) {}
    inline QTextBlock &operator=(const QTextBlock &o) { p = o.p; n = o.n; return *this; }

    inline bool isValid() const { return p != 0 && n != 0; }

    inline bool operator==(const QTextBlock &o) const { return p == o.p && n == o.n; }
    inline bool operator!=(const QTextBlock &o) const { return p != o.p || n != o.n; }
    inline bool operator<(const QTextBlock &o) const { return position() < o.position(); }

    int position() const;
    int length() const;
    bool contains(int position) const;

    QTextLayout *layout() const;
    QTextBlockFormat blockFormat() const;
    QTextCharFormat charFormat() const;

    QString text() const;

    const QTextDocument *document() const;

    class Q_GUI_EXPORT iterator {
        const QTextDocumentPrivate *p;
        int b;
        int e;
        int n;
        friend class QTextBlock;
        iterator(const QTextDocumentPrivate *priv, int begin, int end, int f) : p(priv), b(begin), e(end), n(f) {}
    public:
        iterator() : p(0), b(0), e(0), n(0) {}
        iterator(const iterator &o) : p(o.p), b(o.b), e(o.e), n(o.n) {}

        QTextFragment fragment() const;

        bool atEnd() const { return n == e; }

        inline bool operator==(const iterator &o) const { return p == o.p && n == o.n; }
        inline bool operator!=(const iterator &o) const { return p != o.p || n != o.n; }
        iterator operator++();
        inline iterator operator++(int) { iterator tmp = *this; operator++(); return tmp; }
        iterator operator--();
        inline iterator operator--(int) { iterator tmp = *this; operator--(); return tmp; }
    };

    // more Qt
    typedef iterator Iterator;

    iterator begin() const;
    iterator end() const;

    QTextBlock next() const;
    QTextBlock previous() const;

    inline QTextDocumentPrivate *docHandle() const { return p; }
};

Q_DECLARE_TYPEINFO(QTextBlock, Q_PRIMITIVE_TYPE);


class Q_GUI_EXPORT QTextFragment
{
    const QTextDocumentPrivate *p;
    int n;
    int ne;

public:
    inline QTextFragment(const QTextDocumentPrivate *priv, int f, int fe) : p(priv), n(f), ne(fe) {}
    inline QTextFragment() : p(0), n(0), ne(0) {}
    inline QTextFragment(const QTextFragment &o) : p(o.p), n(o.n), ne(o.ne) {}
    inline QTextFragment &operator=(const QTextFragment &o) { p = o.p; n = o.n; ne = o.ne; return *this; }

    inline bool isValid() const { return p && n; }

    inline bool operator==(const QTextFragment &o) const { return p == o.p && n == o.n; }
    inline bool operator!=(const QTextFragment &o) const { return p != o.p || n != o.n; }
    inline bool operator<(const QTextFragment &o) const { return position() < o.position(); }

    int position() const;
    int length() const;
    bool contains(int position) const;

    QTextCharFormat charFormat() const;
    QString text() const;
};

Q_DECLARE_TYPEINFO(QTextFragment, Q_PRIMITIVE_TYPE);

#endif
