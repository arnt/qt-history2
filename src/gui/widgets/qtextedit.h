#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#ifndef QT_H
#include <qviewport.h>
#include <qtextdocument.h>
#endif // QT_H

#if defined(QT_COMPAT)
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qtextobject.h>
#include <qtextlayout.h>
#endif

class QTextCharFormat;
class QStyleSheet;
class QMimeSourceFactory;
class QTextDocument;
class QTextCursor;
class QTextBlockFormat;
class QMenu;
class QTextEditPrivate;

class Q_GUI_EXPORT QTextEdit : public QViewport
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextEdit)
    Q_FLAGS(AutoFormattingFlag)
    Q_ENUMS(WordWrap)
    Q_PROPERTY(AutoFormattingFlag autoFormatting READ autoFormatting WRITE setAutoFormatting)
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

    enum AutoFormattingFlag {
        AutoNone = 0,
        AutoBulletList = 0x00000001,
        AutoAll = 0xffffffff
    };

    Q_DECLARE_FLAGS(AutoFormatting, AutoFormattingFlag);

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
        MoveEnd,
        MovePageUp,
        MovePageDown
#if defined(QT_COMPAT)
        ,
        MovePgUp = MovePageUp,
        MovePgDown = MovePageDown
#endif
    };

    QTextEdit(QWidget *parent = 0);
    QTextEdit(const QString &text, QWidget *parent = 0);
    virtual ~QTextEdit();

    void setDocument(QTextDocument *document);
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

    inline QString documentTitle() const
    { return document()->documentTitle(); }

    inline bool isUndoRedoEnabled() const
    { return document()->isUndoRedoEnabled(); }
    inline void setUndoRedoEnabled(bool enable)
    { document()->setUndoRedoEnabled(enable); }

    WordWrap wordWrap() const;
    void setWordWrap(WordWrap wrap);

    int wrapColumnOrWidth() const;
    void setWrapColumnOrWidth(int w);

    bool find(const QString &exp, QTextDocument::FindFlags options);

    QString plainText() const;

    void append(const QString &text);

    void ensureCursorVisible();

#if defined(QT_COMPAT)
    void moveCursor(CursorAction action, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
#endif

public slots:
    void setFontPointSize(float s);
    void setFontFamily(const QString &fontFamily);
    void setFontWeight(int w);
    void setFontUnderline(bool b);
    void setFontItalic(bool b);
    void setColor(const QColor &c);
    void setCurrentFont(const QFont &f);
    void setAlignment(Qt::Alignment a);

    void setPlainText(const QString &text);
    void setHtml(const QString &text);
    void setHtml(const QByteArray &html);

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
    void modificationChanged(bool m);
    void selectionChanged();

protected:
    virtual void timerEvent(QTimerEvent *ev);
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *ev);
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual bool focusNextPrevChild(bool next);
    virtual void contextMenuEvent(QContextMenuEvent *ev);
    virtual void dragEnterEvent(QDragEnterEvent *ev);
    virtual void dragMoveEvent(QDragMoveEvent *ev);
    virtual void dropEvent(QDropEvent *ev);

    virtual QMenu *createContextMenu(const QPoint &pos);

private:
    Q_PRIVATE_SLOT(void update(const QRect &r))
    Q_PRIVATE_SLOT(void updateCurrentCharFormatAndSelection())
    Q_PRIVATE_SLOT(void adjustScrollbars());
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
    QT_COMPAT_CONSTRUCTOR QTextEdit(QWidget *parent, const char *name);
    inline QT_COMPAT bool find(const QString &exp, bool cs, bool wo)
    {
        QTextDocument::FindFlags flags = 0;
        if (cs)
            flags |= QTextDocument::FindCaseSensitively;
        if (wo)
            flags |= QTextDocument::FindWholeWords;
        return find(exp, flags);
    }

    inline QT_COMPAT void sync() {}

    enum KeyboardAction {
        ActionBackspace,
        ActionDelete,
        ActionReturn,
        ActionKill,
        ActionWordBackspace,
        ActionWordDelete
    };

    QT_COMPAT void doKeyboardAction(KeyboardAction action);

    QT_COMPAT void setText(const QString &text);
    QT_COMPAT QString text() const;
    QT_COMPAT void setTextFormat(Qt::TextFormat);
    QT_COMPAT Qt::TextFormat textFormat() const;

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

    // small helper for compat methods
    // ### compat
    /*
    QTextBlock blockAt(int parag) const;

    QT_COMPAT void setCursorPosition(int para, int index);

    QT_COMPAT void getCursorPosition(int *parag, int *index) const;

    inline QT_COMPAT QString text(int parag) const
    // ###  html case
    { return blockAt(parag).text(); }

    QT_COMPAT int paragraphs() const;
    QT_COMPAT int lines() const;
    inline QT_COMPAT int linesOfParagraph(int parag) const
    { return blockAt(parag).layout()->numLines(); }
    QT_COMPAT int paragraphLength(int parag) const
    { return blockAt(parag).length(); }

//    QT_COMPAT int lineOfChar(int parag, int index) const;

    QT_COMPAT void getSelection(int *paraFrom, int *indexFrom, int *paraTo, int *indexTo) const;

    inline QT_COMPAT QRect paragraphRect(int parag) const
    { return blockAt(parag).layout()->rect(); }

    QT_COMPAT int paragraphAt(const QPoint &pos) const;
    QT_COMPAT int charAt(const QPoint &pos, int *parag) const;
    */

    inline QT_COMPAT bool hasSelectedText() const
    { return cursor().hasSelection(); }
    inline QT_COMPAT QString selectedText() const
    { return cursor().selectedText(); }

    inline QT_COMPAT bool isUndoAvailable() const
    { return document()->isUndoAvailable(); }
    inline QT_COMPAT bool isRedoAvailable() const
    { return document()->isRedoAvailable(); }
    /*
    inline QT_COMPAT QColor paragraphBackgroundColor(int parag) const
    { return blockAt(parag).blockFormat().backgroundColor(); }
    QT_COMPAT void setParagraphBackgroundColor(int parag, const QColor &col);
    */

    inline QT_COMPAT void insert(const QString &text)
    { insertPlainText(text); }

    inline QT_COMPAT bool isModified() const
    { return document()->isModified(); }

public slots:
    inline QT_MOC_COMPAT void setModified(bool m = true)
    { document()->setModified(m); }
    inline QT_MOC_COMPAT void undo() const
    { document()->undo(); }
    inline QT_MOC_COMPAT void redo() const
    { document()->redo(); }

#endif
};

#endif // QTEXTEDIT_H
