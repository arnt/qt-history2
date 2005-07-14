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

#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtGui/qfont.h>

QT_MODULE(Gui)

class QTextFormatCollection;
class QTextListFormat;
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
class QUrl;
class QVariant;

template<typename T> class QVector;

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

#ifndef QT_NO_TEXTCODEC
    Q_GUI_EXPORT QTextCodec *codecForHtml(const QByteArray &ba);
#endif
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
    Q_PROPERTY(QSizeF pageSize READ pageSize WRITE setPageSize)
    Q_PROPERTY(QFont defaultFont READ defaultFont WRITE setDefaultFont)

public:
    explicit QTextDocument(QObject *parent = 0);
    explicit QTextDocument(const QString &text, QObject *parent = 0);
    ~QTextDocument();

    QTextDocument *clone(QObject *parent = 0) const;

    bool isEmpty() const;
    virtual void clear();

    void setUndoRedoEnabled(bool enable);
    bool isUndoRedoEnabled() const;

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    void setDocumentLayout(QAbstractTextDocumentLayout *layout);
    QAbstractTextDocumentLayout *documentLayout() const;

    enum MetaInformation {
        DocumentTitle
    };
    void setMetaInformation(MetaInformation info, const QString &);
    QString metaInformation(MetaInformation info) const;

    QString toHtml(const QByteArray &encoding = QByteArray()) const;
    void setHtml(const QString &html);

    QString toPlainText() const;
    void setPlainText(const QString &text);

    enum FindFlag
    {
        FindBackward        = 0x00001,
        FindCaseSensitively = 0x00002,
        FindWholeWords      = 0x00004
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    QTextCursor find(const QString &expr, int from = 0, FindFlags options = 0) const;
    QTextCursor find(const QString &expr, const QTextCursor &from, FindFlags options = 0) const;

    QTextFrame *frameAt(int pos) const;
    QTextFrame *rootFrame() const;

    QTextObject *object(int objectIndex) const;
    QTextObject *objectForFormat(const QTextFormat &) const;

    QTextBlock findBlock(int pos) const;
    QTextBlock begin() const;
    QTextBlock end() const;

    void setPageSize(const QSizeF &size);
    QSizeF pageSize() const;

    void setDefaultFont(const QFont &font);
    QFont defaultFont() const;

    int pageCount() const;

    bool isModified() const;

#ifndef QT_NO_PRINTER
    void print(QPrinter *printer) const;
#endif
    
    enum ResourceType {
        HtmlResource  = 1,
        ImageResource = 2,

        UserResource  = 100
    };

    QVariant resource(int type, const QUrl &name) const;
    void addResource(int type, const QUrl &name, const QVariant &resource);

    QVector<QTextFormat> allFormats() const;

    void markContentsDirty(int from, int length);

signals:
    void contentsChange(int from, int charsRemoves, int charsAdded);
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
    virtual QVariant loadResource(int type, const QUrl &name);

public:
    QTextDocumentPrivate *docHandle() const;
private:
    Q_DISABLE_COPY(QTextDocument)
    Q_DECLARE_PRIVATE(QTextDocument)
};

#endif // QTEXTDOCUMENT_H
