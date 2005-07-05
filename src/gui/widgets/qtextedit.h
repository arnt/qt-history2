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

#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#include <QtGui/qabstractscrollarea.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>

#ifndef QT_NO_TEXTEDIT

#ifdef QT3_SUPPORT
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qtextobject.h>
#include <QtGui/qtextlayout.h>
#endif

class QTextCharFormat;
class QStyleSheet;
class QTextDocument;
class QTextCursor;
class QTextBlockFormat;
class QMenu;
class QTextEditPrivate;
class QMimeData;

class Q_GUI_EXPORT QTextEdit : public QAbstractScrollArea
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextEdit)
    Q_FLAGS(AutoFormatting)
    Q_ENUMS(LineWrapMode)
    Q_PROPERTY(AutoFormatting autoFormatting READ autoFormatting WRITE setAutoFormatting)
    Q_PROPERTY(bool tabChangesFocus READ tabChangesFocus WRITE setTabChangesFocus)
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle)
    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled)
    Q_PROPERTY(LineWrapMode lineWrapMode READ lineWrapMode WRITE setLineWrapMode)
    QDOC_PROPERTY(QTextOption::WrapMode wordWrapMode READ wordWrapMode WRITE setWordWrapMode)
    Q_PROPERTY(int lineWrapColumnOrWidth READ lineWrapColumnOrWidth WRITE setLineWrapColumnOrWidth)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(QString html READ toHtml WRITE setHtml)
public:
    enum LineWrapMode {
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

    Q_DECLARE_FLAGS(AutoFormatting, AutoFormattingFlag)

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
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        ,
        MovePgUp = MovePageUp,
        MovePgDown = MovePageDown
#endif
    };

    explicit QTextEdit(QWidget *parent = 0);
    explicit QTextEdit(const QString &text, QWidget *parent = 0);
    virtual ~QTextEdit();

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

    bool isReadOnly() const;
    void setReadOnly(bool ro);

    qreal fontPointSize() const;
    QString fontFamily() const;
    int fontWeight() const;
    bool fontUnderline() const;
    bool fontItalic() const;
    QColor textColor() const;
    QFont currentFont() const;
    Qt::Alignment alignment() const;

    void mergeCurrentCharFormat(const QTextCharFormat &modifier);

    void setCurrentCharFormat(const QTextCharFormat &format);
    QTextCharFormat currentCharFormat() const;

    AutoFormatting autoFormatting() const;
    void setAutoFormatting(AutoFormatting features);

    bool tabChangesFocus() const;
    void setTabChangesFocus(bool b);

    inline void setDocumentTitle(const QString &title)
    { document()->setMetaInformation(QTextDocument::DocumentTitle, title); }
    inline QString documentTitle() const
    { return document()->metaInformation(QTextDocument::DocumentTitle); }

    inline bool isUndoRedoEnabled() const
    { return document()->isUndoRedoEnabled(); }
    inline void setUndoRedoEnabled(bool enable)
    { document()->setUndoRedoEnabled(enable); }

    LineWrapMode lineWrapMode() const;
    void setLineWrapMode(LineWrapMode mode);

    int lineWrapColumnOrWidth() const;
    void setLineWrapColumnOrWidth(int w);

    QTextOption::WrapMode wordWrapMode() const;
    void setWordWrapMode(QTextOption::WrapMode policy);

    bool find(const QString &exp, QTextDocument::FindFlags options = 0);

    inline QString toPlainText() const
    { return document()->toPlainText(); }
    inline QString toHtml() const
    { return document()->toHtml(); }

    void append(const QString &text);

    void ensureCursorVisible();

    virtual QVariant loadResource(int type, const QUrl &name);
#ifndef QT_NO_MENU
    QMenu *createStandardContextMenu();
#endif
    
    QTextCursor cursorForPosition(const QPoint &pos) const;
    QRect cursorRect(const QTextCursor &cursor) const;
    QRect cursorRect() const;

    QString anchorAt(const QPoint& pos) const;

public slots:
    void setFontPointSize(qreal s);
    void setFontFamily(const QString &fontFamily);
    void setFontWeight(int w);
    void setFontUnderline(bool b);
    void setFontItalic(bool b);
    void setTextColor(const QColor &c);
    void setCurrentFont(const QFont &f);
    void setAlignment(Qt::Alignment a);

    void setPlainText(const QString &text);
    void setHtml(const QString &text);

#ifndef QT_NO_CLIPBOARD
    void cut();
    void copy();
    void paste();
#endif
    
    void clear();
    void selectAll();

    void insertPlainText(const QString &text);
    void insertHtml(const QString &text);

    void scrollToAnchor(const QString &name);

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);

signals:
    void textChanged();
    void undoAvailable(bool b);
    void redoAvailable(bool b);
    void currentCharFormatChanged(const QTextCharFormat &format);
    void copyAvailable(bool b);
    void selectionChanged();
    void cursorPositionChanged();

protected:
    virtual void timerEvent(QTimerEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual bool focusNextPrevChild(bool next);
    virtual void contextMenuEvent(QContextMenuEvent *e);
#ifndef QT_NO_DRAGANDDROP
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dropEvent(QDropEvent *e);
#endif
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void showEvent(QShowEvent *);
    virtual void changeEvent(QEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

    virtual QMimeData *createMimeDataFromSelection() const;
    virtual bool canInsertFromMimeData(const QMimeData *source) const;
    virtual void insertFromMimeData(const QMimeData *source);

    virtual void inputMethodEvent(QInputMethodEvent *);
    QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

    QTextEdit(QTextEditPrivate &dd, QWidget *parent);

#ifdef QT3_SUPPORT
signals:
    QT_MOC_COMPAT void currentFontChanged(const QFont &f);
    QT_MOC_COMPAT void currentColorChanged(const QColor &c);

public:
    QT3_SUPPORT_CONSTRUCTOR QTextEdit(QWidget *parent, const char *name);
    inline QT3_SUPPORT bool find(const QString &exp, bool cs, bool wo)
    {
        QTextDocument::FindFlags flags = 0;
        if (cs)
            flags |= QTextDocument::FindCaseSensitively;
        if (wo)
            flags |= QTextDocument::FindWholeWords;
        return find(exp, flags);
    }

    inline QT3_SUPPORT void sync() {}

    QT3_SUPPORT void moveCursor(CursorAction action, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    enum KeyboardAction {
        ActionBackspace,
        ActionDelete,
        ActionReturn,
        ActionKill,
        ActionWordBackspace,
        ActionWordDelete
    };

    QT3_SUPPORT void doKeyboardAction(KeyboardAction action);

    QT3_SUPPORT void setText(const QString &text);
    QT3_SUPPORT QString text() const;
    QT3_SUPPORT void setTextFormat(Qt::TextFormat);
    QT3_SUPPORT Qt::TextFormat textFormat() const;

    inline QT3_SUPPORT void setBold(bool b) { setFontWeight(b ? QFont::Bold : QFont::Normal); }
    inline QT3_SUPPORT void setUnderline(bool b) { setFontUnderline(b); }
    inline QT3_SUPPORT void setItalic(bool i) { setFontItalic(i); }
    inline QT3_SUPPORT void setFamily(const QString &family) { setFontFamily(family); }
    inline QT3_SUPPORT void setPointSize(int size) { setFontPointSize(size); }

    inline QT3_SUPPORT bool italic() const { return fontItalic(); }
    inline QT3_SUPPORT bool bold() const { return fontWeight() >= QFont::Bold; }
    inline QT3_SUPPORT bool underline() const { return fontUnderline(); }
    inline QT3_SUPPORT QString family() const { return fontFamily(); }
    inline QT3_SUPPORT int pointSize() const { return (int)(fontPointSize()+0.5); }

    inline QT3_SUPPORT bool hasSelectedText() const
    { return textCursor().hasSelection(); }
    inline QT3_SUPPORT QString selectedText() const
    { return textCursor().selectedText(); }

    inline QT3_SUPPORT bool isUndoAvailable() const
    { return document()->isUndoAvailable(); }
    inline QT3_SUPPORT bool isRedoAvailable() const
    { return document()->isRedoAvailable(); }

    inline QT3_SUPPORT void insert(const QString &text)
    { insertPlainText(text); }

    inline QT3_SUPPORT bool isModified() const
    { return document()->isModified(); }

    inline QT3_SUPPORT QColor color() const
    { return textColor(); }

public slots:
    inline QT_MOC_COMPAT void setModified(bool m = true)
    { document()->setModified(m); }
    inline QT_MOC_COMPAT void undo() const
    { document()->undo(); }
    inline QT_MOC_COMPAT void redo() const
    { document()->redo(); }

    inline QT_MOC_COMPAT void setColor(const QColor &c)
    { setTextColor(c); }

#endif

private:
    Q_DISABLE_COPY(QTextEdit)
    Q_PRIVATE_SLOT(d_func(), void repaintContents(const QRectF &r))
    Q_PRIVATE_SLOT(d_func(), void updateCurrentCharFormatAndSelection())
    Q_PRIVATE_SLOT(d_func(), void adjustScrollbars())
    Q_PRIVATE_SLOT(d_func(), void emitCursorPosChanged(const QTextCursor &))
};

#endif // QT_NO_TEXTEDIT
#endif // QTEXTEDIT_H
