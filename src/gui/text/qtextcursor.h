#ifndef QTEXTCURSOR_H
#define QTEXTCURSOR_H

#include <qstring.h>

#include <qtextlayout.h>
#include <qshareddata.h>
#include "qtextlist.h"

class QTextDocument;
class QTextLayout;
class QTextCursorPrivate;
class QTextDocumentFragment;
class QTextCharFormat;
class QTextBlockFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextImageFormat;
class QTextPieceTable;
class QTextTable;
class QTextBlockIterator;

class Q_GUI_EXPORT QTextCursor
{
    friend class QTextDocumentFragmentPrivate;
public:
    QTextCursor();
    QTextCursor(QTextDocument *document);
    QTextCursor(const QTextBlockIterator &block);
    QTextCursor(const QTextPieceTable *pt, int pos);
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
        StartOfLine,
        PreviousBlock,
        PreviousCharacter,
        PreviousWord,
        Up,
        Left,
        WordLeft,

        End,
        EndOfLine,
        NextBlock,
        NextCharacter,
        NextWord,
        Down,
        Right,
        WordRight
    };

    bool moveTo(MoveOperation op, MoveMode = MoveAnchor, int n = 1);

    void deleteChar();
    void deletePreviousChar();

    bool hasSelection() const;
    void removeSelectedText();
    void clearSelection();
    int selectionStart() const;
    int selectionEnd() const;

    QTextBlockIterator block() const;
    void setBlockFormat(const QTextBlockFormat &format);
    QTextBlockFormat blockFormat() const;
    void applyCharFormatModifier(const QTextCharFormat &modifier);
    void applyBlockFormatModifier(const QTextBlockFormat &modifier);

    QTextCharFormat charFormat() const;

    bool atBlockStart() const;
    bool atEnd() const;

    void insertBlock();
    void insertBlock(const QTextBlockFormat &format);

    QTextList *insertList(const QTextListFormat &format);
    QTextList *insertList(int style);

    QTextList *createList(const QTextListFormat &format);
    QTextList *createList(int style);
    QTextList *currentList() const;
    // ### these two are probably temporary....
    int listItemNumber() const;
    QString listItemText() const;

    QTextTable *insertTable(int rows, int cols, const QTextTableFormat &format);
    QTextTable *insertTable(int rows, int cols);
    QTextTable *currentTable() const;

    QTextFrame *insertFrame(const QTextFrameFormat &format);

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
