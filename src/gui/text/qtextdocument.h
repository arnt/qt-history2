#ifndef Q_RTDOCUMENT_H
#define Q_RTDOCUMENT_H

#include <qobject.h>
#include <qsharedpointer.h>

class QTextPieceTable;
class QTextFormatCollection;
class QTextListManager;
class QTextTableManager;
class QTextListFormat;
class QSize;
class QRect;
class QPainter;

typedef QExplicitSharedPointer<QTextPieceTable> QTextPieceTablePointer;

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

class QTextDocument : public QObject
{
    Q_OBJECT
    friend class QTextEditor; // ####
    friend class QTextCursor;
public:
    QTextDocument(QObject *parent = 0);
    QTextDocument(const QString &text, QObject *parent = 0);
    ~QTextDocument();

    QString plainText() const;

    void enableUndoRedo(bool enable);
    bool isUndoRedoEnabled() const;

    // ###
    QTextPieceTablePointer &table() { return pieceTable; }

    QString documentTitle() const;

    void setPageSize(const QSize &s);
    QSize pageSize() const;

    int numPages() const;
    QRect pageRect(int page) const;

    void draw(int page, QPainter *p, const QRect &rect);

signals:
    void contentsChanged();

public slots:
    void undo();
    void redo();
    void appendUndoItem(QAbstractUndoItem *);

private:
    void undoRedo(bool undo);
    void init();

    QTextPieceTablePointer pieceTable;

#if defined(Q_DISABLE_COPY)
    QTextDocument(const QTextDocument &);
    QTextDocument &operator=(const QTextDocument &);
#endif
};

inline void QTextDocument::undo() { undoRedo(true); }
inline void QTextDocument::redo() { undoRedo(false); }

#endif
