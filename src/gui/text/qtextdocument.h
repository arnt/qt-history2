#ifndef Q_TEXTDOCUMENT_H
#define Q_TEXTDOCUMENT_H

#ifndef QT_H
#include <qobject.h>
#endif // QT_H

class QTextFormatCollection;
class QTextListFormat;
class QTextPieceTable;
class QSize;
class QRect;
class QPainter;
class QAbstractTextDocumentLayout;

namespace QText
{
    enum HitTestAccuracy { ExactHit, FuzzyHit };
    enum ChangeOperation { Insert, Remove };
}

class QAbstractUndoItem
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
    Q_DECLARE_PRIVATE(QTextDocument);
    friend class QTextEditor; // ####
    friend class QTextCursor;
public:
    QTextDocument(QObject *parent = 0);
    QTextDocument(const QString &text, QObject *parent = 0);
    QTextDocument(QAbstractTextDocumentLayout *documentLayout, QObject *parent = 0);
    ~QTextDocument();

    QString plainText() const;

    bool isEmpty() const;

    void enableUndoRedo(bool enable);
    bool isUndoRedoEnabled() const;

    bool isUndoRedoAvailable() const;

    QAbstractTextDocumentLayout *documentLayout() const;

//     // ###
//     inline QTextPieceTablePointer &table() { return pieceTable; }

    QString documentTitle() const;

    void setPageSize(const QSize &s);
    QSize pageSize() const;

    int numPages() const;

    void draw(int page, QPainter *p, const QRect &rect);

signals:
    void contentsChanged();

public slots:
    void undo();
    void redo();
    void appendUndoItem(QAbstractUndoItem *);

private:
    void undoRedo(bool undo);

#if defined(Q_DISABLE_COPY)
    QTextDocument(const QTextDocument &);
    QTextDocument &operator=(const QTextDocument &);
#endif
};

inline void QTextDocument::undo() { undoRedo(true); }
inline void QTextDocument::redo() { undoRedo(false); }

#endif
