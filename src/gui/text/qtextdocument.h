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

#ifndef QTEXTDOCUMENT_H
#define QTEXTDOCUMENT_H

#include <qobject.h>

class QTextFormatCollection;
class QTextListFormat;
class QSize;
class QRect;
class QPainter;
class QPrinter;
class QAbstractTextDocumentLayout;
class QPoint;
class QTextCursor;
class QTextObject;
class QTextFormat;
class QTextFrame;
class QTextBlock;
class QTextCodec;

namespace Qt
{
    enum HitTestAccuracy { ExactHit, FuzzyHit };
    enum WhiteSpaceMode {
        WhiteSpaceNormal,
        WhiteSpacePre,
        WhiteSpaceNoWrap,
        WhiteSpaceModeUndefined = -1
    };

    Q_GUI_EXPORT bool mightBeRichText(const QString&);
    Q_GUI_EXPORT QString escape(const QString& plain);
    Q_GUI_EXPORT QString convertFromPlainText(const QString &plain, WhiteSpaceMode mode = WhiteSpacePre);

    Q_GUI_EXPORT QTextCodec *codecForHtml(const QByteArray &ba);
}

class Q_GUI_EXPORT QAbstractUndoItem
{
public:
    virtual ~QAbstractUndoItem() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
};

inline QAbstractUndoItem::~QAbstractUndoItem()
{
}

class QTextDocumentPrivate;

class Q_GUI_EXPORT QTextDocument : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled)
    Q_PROPERTY(bool modified READ isModified WRITE setModified DESIGNABLE false)

public:
    QTextDocument(QObject *parent = 0);
    QTextDocument(const QString &text, QObject *parent = 0);
    ~QTextDocument();

    bool isEmpty() const;

    void setUndoRedoEnabled(bool enable);
    bool isUndoRedoEnabled() const;

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    void setDocumentLayout(QAbstractTextDocumentLayout *layout);
    QAbstractTextDocumentLayout *documentLayout() const;

    QString documentTitle() const;

    QString toHtml(const QString &encoding = QString()) const;
    void setHtml(const QString &html);

    QString toPlainText() const;
    void setPlainText(const QString &text);

    enum FindFlag
    {
        FindCaseSensitively = 0x00001,
        FindWholeWords = 0x00002
        // ### more
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    enum FindDirection
    {
        FindForward,
        FindBackward
    };

    QTextCursor find(const QString &expr, int from = 0, FindFlags options = 0, FindDirection direction = FindForward) const;
    QTextCursor find(const QString &expr, const QTextCursor &from, FindFlags options = 0, FindDirection direction = FindForward) const;

    QTextFrame *frameAt(int pos) const;
    QTextFrame *rootFrame() const;

    QTextObject *object(int objectIndex) const;
    QTextObject *objectForFormat(const QTextFormat &) const;

    QTextBlock findBlock(int pos) const;
    QTextBlock begin() const;
    QTextBlock end() const;

    bool isModified() const;

    void print(QPrinter *printer) const;

signals:
    void contentsChanged();
    void undoAvailable(bool);
    void redoAvailable(bool);
    void modificationChanged(bool m);
    void cursorPositionChanged(const QTextCursor &cursor);

public slots:
    void undo();
    void redo();
    void appendUndoItem(QAbstractUndoItem *);
    void setModified(bool m = true);

protected:
    virtual QTextObject *createObject(const QTextFormat &f);

public:
    QTextDocumentPrivate *docHandle() const;
private:
    Q_DISABLE_COPY(QTextDocument)
    Q_DECLARE_PRIVATE(QTextDocument)
};

#endif
