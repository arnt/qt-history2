#ifndef QTEXTCURSOR_H
#define QTEXTCURSOR_H

#include <qstring.h>
#include <qshareddata.h>

class QTextDocument;
class QTextCursorPrivate;
class QTextDocumentFragment;
class QTextCharFormat;
class QTextBlockFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextFrameFormat;
class QTextImageFormat;
class QTextDocumentPrivate;
class QTextList;
class QTextTable;
class QTextFrame;
class QTextBlock;

class Q_GUI_EXPORT QTextCursor
{
    friend class QTextDocumentFragmentPrivate;
public:
    QTextCursor();
    QTextCursor(QTextDocument *document);
    QTextCursor(QTextDocumentPrivate *p, int pos);
    QTextCursor(QTextFrame *frame);
    QTextCursor(const QTextBlock &block);
    QTextCursor(const QTextCursor &cursor);
    QTextCursor &operator=(const QTextCursor &other);
    ~QTextCursor();

    bool isNull() const; // ### or isValid() instead?

    enum MoveMode {
        MoveAnchor,
        KeepAnchor
    };

    void setPosition(int pos, MoveMode mode = MoveAnchor);
    int position() const;

    int anchor() const;

    void insertText(const QString &text);
    void insertText(const QString &text, const QTextCharFormat &format);

    enum MoveOperation {
        NoMove,

        Start,
        Up,
        StartOfLine,
        StartOfBlock,
        StartOfWord,
        PreviousBlock,
        PreviousCharacter,
        PreviousWord,
        Left,
        WordLeft,

        End,
        Down,
        EndOfLine,
        EndOfWord,
        EndOfBlock,
        NextBlock,
        NextCharacter,
        NextWord,
        Right,
        WordRight
    };

    bool movePosition(MoveOperation op, MoveMode = MoveAnchor, int n = 1);

    void deleteChar();
    void deletePreviousChar();

    bool hasSelection() const;
    bool hasComplexSelection() const;
    void removeSelectedText();
    void clearSelection();
    int selectionStart() const;
    int selectionEnd() const;

    QString selectedText() const;
    QTextDocumentFragment selection() const;
    void selectedTableCells(int *firstRow, int *numRows, int *firstColumn, int *numColumns) const;

    QTextBlock block() const;

    QTextCharFormat charFormat() const;
    void setCharFormat(const QTextCharFormat &format);
    void mergeCharFormat(const QTextCharFormat &modifier);

    QTextBlockFormat blockFormat() const;
    void setBlockFormat(const QTextBlockFormat &format);
    void mergeBlockFormat(const QTextBlockFormat &modifier);


    bool atBlockStart() const;
    bool atEnd() const;

    void insertBlock();
    void insertBlock(const QTextBlockFormat &format);

    QTextList *insertList(const QTextListFormat &format);
    QTextList *insertList(int style);

    QTextList *createList(const QTextListFormat &format);
    QTextList *createList(int style);
    QTextList *currentList() const;

    QTextTable *insertTable(int rows, int cols, const QTextTableFormat &format);
    QTextTable *insertTable(int rows, int cols);
    QTextTable *currentTable() const;

    QTextFrame *insertFrame(const QTextFrameFormat &format);
    QTextFrame *currentFrame() const;

    void insertFragment(const QTextDocumentFragment &fragment);

    void insertImage(const QTextImageFormat &format);

    void beginEditBlock();
    void endEditBlock();

    bool operator!=(const QTextCursor &rhs) const;
    bool operator<(const QTextCursor &rhs) const;
    bool operator<=(const QTextCursor &rhs) const;
    bool operator==(const QTextCursor &rhs) const;
    bool operator>=(const QTextCursor &rhs) const;
    bool operator>(const QTextCursor &rhs) const;

private:
    QSharedDataPointer<QTextCursorPrivate> d;
};

#endif // QTEXTCURSOR_H
