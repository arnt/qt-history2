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

namespace QText
{
    enum HitTestAccuracy { ExactHit, FuzzyHit };
    enum ChangeOperation { Insert, Remove };
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
    friend class QTextEditor; // ####
    friend class QTextCursor;
    friend class QAbstractTextDocumentLayout;
public:
    QTextDocument(QObject *parent = 0);
    QTextDocument(const QString &text, QObject *parent = 0);
    ~QTextDocument();

    QString plainText() const;

    bool isEmpty() const;

    void setUndoRedoEnabled(bool enable);
    bool isUndoRedoEnabled() const;

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    void setDocumentLayout(QAbstractTextDocumentLayout *layout);
    QAbstractTextDocumentLayout *documentLayout() const;

    QString documentTitle() const;

    void setHtml(const QString &html);

    QTextCursor find(const QString &exp, int from = 0, StringComparison flags = (CaseSensitive | Contains)) const;
    QTextCursor find(const QString &exp, const QTextCursor &from, StringComparison flags = (CaseSensitive | Contains)) const;

    QTextFrame *frameAt(int pos) const;
    QTextFrame *rootFrame() const;

signals:
    void contentsChanged();
    void undoAvailable(bool);
    void redoAvailable(bool);

public slots:
    void undo();
    void redo();
    void appendUndoItem(QAbstractUndoItem *);


protected:
    virtual QTextObject *createObject(const QTextFormat &f);

#if defined(Q_DISABLE_COPY)
    QTextDocument(const QTextDocument &);
    QTextDocument &operator=(const QTextDocument &);
#endif
};

#endif
