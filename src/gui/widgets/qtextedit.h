#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#ifndef QT_H
#include <qscrollview.h>
#include <qtextdocument.h>
#endif // QT_H

#if defined(QT_COMPAT)
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qtextobject.h>
#endif

class QTextCharFormat;
class QStyleSheet;
class QMimeSourceFactory;
class QTextDocument;
class QTextCursor;
class QTextBlockFormat;
class QMenu;
class QTextEditPrivate;

class QTextEdit : public QScrollView
{
    Q_OBJECT
    Q_FLAGS(AutoFormattingFlags)
    Q_ENUMS(WordWrap)
    Q_PROPERTY(AutoFormattingFlags autoFormatting READ autoFormatting WRITE setAutoFormatting)
    Q_PROPERTY(bool tabChangesFocus READ tabChangesFocus WRITE setTabChangesFocus)
    Q_PROPERTY(QString documentTitle READ documentTitle)
    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled)
    Q_PROPERTY(WordWrap wordWrap READ wordWrap WRITE setWordWrap)
    Q_PROPERTY(int wrapColumnOrWidth READ wrapColumnOrWidth WRITE setWrapColumnOrWidth)
public:
    enum WordWrap {
        NoWrap,
        WidgetWidth,
        FixedPixelWidth,
        FixedColumnWidth
    };

    enum AutoFormattingFlags {
        AutoNone = 0,
        AutoBulletList = 0x00000001,
        AutoAll = 0xffffffff
    };

    Q_DECLARE_FLAGS(AutoFormatting, AutoFormattingFlags);

    QTextEdit(QWidget *parent, const char *name = 0);
    virtual ~QTextEdit();

    QTextDocument *document() const;

    void setCursor(const QTextCursor &cursor);
    QTextCursor cursor() const;

    bool isReadOnly() const;
    void setReadOnly(bool ro);

    float fontPointSize() const;
    QString fontFamily() const;
    int fontWeight() const;
    bool fontUnderline() const;
    bool fontItalic() const;
    QColor color() const;
    QFont currentFont() const;
    Qt::Alignment alignment() const;

    void mergeCurrentCharFormat(const QTextCharFormat &modifier);

    void setCurrentCharFormat(const QTextCharFormat &format);
    QTextCharFormat currentCharFormat() const;

    AutoFormatting autoFormatting() const;
    void setAutoFormatting(AutoFormatting features);

    bool tabChangesFocus() const;
    void setTabChangesFocus(bool b);

    QString documentTitle() const;

    bool isUndoRedoEnabled();
    void setUndoRedoEnabled(bool enable);

    WordWrap wordWrap() const;
    void setWordWrap(WordWrap wrap);

    int wrapColumnOrWidth() const;
    void setWrapColumnOrWidth(int w);

    bool find(const QString &exp, StringComparison flags);

    QString plainText() const;

    void append(const QString &text);

    void ensureCursorVisible();

public slots:
    void setFontPointSize(float size);
    void setFontFamily(const QString &family);
    void setFontWeight(int weight);
    void setFontUnderline(bool underline);
    void setFontItalic(bool italic);
    void setColor(const QColor &color);
    void setCurrentFont(const QFont &f);
    void setAlignment(Qt::Alignment a);

    void setPlainText(const QString &text);
    void setHtml(const QString &text);
    void setHtml(const QByteArray &html);


    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

    void clear();
    void selectAll();

    void insertPlainText(const QString &text);
    void insertHtml(const QString &text);

signals:
    void currentCharFormatChanged(const QTextCharFormat &format);
    void copyAvailable(bool b);

protected:
    virtual void timerEvent(QTimerEvent *ev);
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void viewportResizeEvent(QResizeEvent *);
    virtual void drawContents(QPainter *painter, int clipX, int clipY, int clipWidth, int clipHeight);
    virtual void contentsMousePressEvent(QMouseEvent *ev);
    virtual void contentsMouseMoveEvent(QMouseEvent *ev);
    virtual void contentsMouseReleaseEvent(QMouseEvent *ev);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *ev);
    virtual bool focusNextPrevChild(bool next);
    virtual void contentsContextMenuEvent(QContextMenuEvent *ev);
    virtual void contentsDragEnterEvent(QDragEnterEvent *ev);
    virtual void contentsDragMoveEvent(QDragMoveEvent *ev);
    virtual void contentsDropEvent(QDropEvent *ev);

    virtual QMenu *createContextMenu(const QPoint &pos);

private:
    friend class QTextEditPrivate;
    QTextEditPrivate *d;

    Q_PRIVATE_SLOT(void trippleClickTimeout());
#if defined(Q_DISABLE_COPY)
    QTextEdit(const QTextEdit &);
    QTextEdit &operator=(const QTextEdit &);
#endif

signals:
    QT_MOC_COMPAT void textChanged();
    QT_MOC_COMPAT void undoAvailable(bool b);
    QT_MOC_COMPAT void redoAvailable(bool b);
    QT_MOC_COMPAT void currentFontChanged(const QFont &f);
    QT_MOC_COMPAT void currentColorChanged(const QColor &c);

#if defined(QT_COMPAT)
public:
    inline QT_COMPAT bool find(const QString &exp, bool cs, bool wo)
    {
        StringComparison flags = 0;
        if (cs)
            flags |= CaseSensitive;
        if (wo)
            flags |= ExactMatch;
        else
            flags |= Contains;
        return find(exp, flags);
    }

    enum CursorAction {
        MoveBackward,
        MoveForward,
        MoveWordBackward,
        MoveWordForward,
        MoveUp,
        MoveDown,
        MoveLineStart,
        MoveLineEnd,
        MoveHome,
        MoveEnd /*,
        MovePgUp,
        MovePgDown
        */
    };

    inline QT_COMPAT void moveCursor(CursorAction action, bool select)
    {
        QTextCursor::MoveOperation op = QTextCursor::NoMove;
        switch (action) {
            case MoveBackward: op = QTextCursor::Left; break;
            case MoveForward: op = QTextCursor::Right; break;
            case MoveWordBackward: op = QTextCursor::WordLeft; break;
            case MoveWordForward: op = QTextCursor::WordRight; break;
            case MoveUp: op = QTextCursor::Up; break;
            case MoveDown: op = QTextCursor::Down; break;
            case MoveLineStart: op = QTextCursor::StartOfLine; break;
            case MoveLineEnd: op = QTextCursor::EndOfLine; break;
            case MoveHome: op = QTextCursor::Start; break;
            case MoveEnd: op = QTextCursor::End; break;
        }
        QTextCursor::MoveMode mode = (select ? QTextCursor::MoveAnchor : QTextCursor::KeepAnchor);
        QTextCursor curs = cursor();
        curs.movePosition(op, mode);
        setCursor(curs);
    }

    inline QT_COMPAT void sync() {}

    enum KeyboardAction {
        ActionBackspace,
        ActionDelete,
        ActionReturn // ,
        // #### ActionKill,
        // #### ActionWordBackspace,
        // #### ActionWordDelete
    };

    inline QT_COMPAT void doKeyboardAction(KeyboardAction action)
    {
        switch (action) {
            case ActionBackspace: cursor().deletePreviousChar(); break;
            case ActionDelete: cursor().deleteChar(); break;
            case ActionReturn: cursor().insertBlock(); break;
        }
    }

    QT_COMPAT void setText(const QString &text);
    QT_COMPAT QString text() const;
    QT_COMPAT void setTextFormat(TextFormat);
    QT_COMPAT TextFormat textFormat() const;

    inline QT_COMPAT void setBold(bool b) { setFontWeight(b ? QFont::Bold : QFont::Normal); }
    inline QT_COMPAT void setUnderline(bool b) { setFontUnderline(b); }
    inline QT_COMPAT void setItalic(bool i) { setFontItalic(i); }
    inline QT_COMPAT void setFamily(const QString &family) { setFontFamily(family); }
    inline QT_COMPAT void setPointSize(int size) { setFontPointSize(size); }

    inline QT_COMPAT bool italic() const { return fontItalic(); }
    inline QT_COMPAT bool bold() const { return fontWeight() >= QFont::Bold; }
    inline QT_COMPAT bool underline() const { return fontUnderline(); }
    inline QT_COMPAT QString family() const { return fontFamily(); }
    inline QT_COMPAT int pointSize() const { return (int)(fontPointSize()+0.5); }

    QT_COMPAT void setCursorPosition(int para, int index);

    QT_COMPAT void getCursorPosition(int *parag, int *index) const;

    QT_COMPAT QString text(int parag) const;

    QT_COMPAT int paragraphs() const;

    QT_COMPAT int linesOfParagraph(int parag) const;

    QT_COMPAT int paragraphLength(int parag) const;

    QT_COMPAT void getSelection(int *paraFrom, int *indexFrom, int *paraTo, int *indexTo) const;

#endif
};

#endif // QTEXTEDIT_H
