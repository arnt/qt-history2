#ifndef Q_TEXTDOCUMENT_H
#define Q_TEXTDOCUMENT_H

#ifndef QT_H
#include <qobject.h>
#endif // QT_H

class QTextFormatCollection;
class QTextListFormat;
class QSize;
class QRect;
class QPainter;
class QAbstractTextDocumentLayout;
class QPoint;
class QTextCursor;
class QTextObject;
class QTextFormat;
class QTextFrame;
class QTextBlock;

namespace QText
{
    enum HitTestAccuracy { ExactHit, FuzzyHit };
    enum ChangeOperation { Insert, Remove };

    bool mightBeRichText(const QString&);

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
    Q_DECLARE_PRIVATE(QTextDocument)
    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled)
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

    QString html() const;
    void setHtml(const QString &html);

    QString plainText() const;
    void setPlainText(const QString &text);

    enum FindFlag
    {
        SearchCaseSensitive = 0x00001,
        SearchFullWordsOnly = 0x00002
        // ### more
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag);

    QTextCursor find(const QString &exp, int from = 0, FindFlags options = 0) const;
    QTextCursor find(const QString &exp, const QTextCursor &from, FindFlags options = 0) const;

    QTextFrame *frameAt(int pos) const;
    QTextFrame *rootFrame() const;

    QTextObject *object(int objectIndex) const;
    QTextObject *objectForFormat(const QTextFormat &) const;

    QTextBlock findBlock(int pos) const;
    QTextBlock begin() const;
    QTextBlock end() const;

signals:
    void contentsChanged();
    void undoAvailable(bool);
    void redoAvailable(bool);

public slots:
    void undo();
    void redo();
    void appendUndoItem(QAbstractUndoItem *);

public:
    QTextDocumentPrivate *docHandle() const;
protected:
    virtual QTextObject *createObject(const QTextFormat &f);

#if defined(Q_DISABLE_COPY)
    QTextDocument(const QTextDocument &);
    QTextDocument &operator=(const QTextDocument &);
#endif
};

#endif
