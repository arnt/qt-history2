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

#ifndef QTEXTEDIT_P_H
#define QTEXTEDIT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractscrollarea_p.h"
#include "QtGui/qtextdocumentfragment.h"
#include "QtGui/qscrollbar.h"
#include "QtGui/qtextcursor.h"
#include "QtGui/qtextformat.h"
#include "QtGui/qmenu.h"
#include "QtGui/qabstracttextdocumentlayout.h"
#include "QtCore/qbasictimer.h"
#include "QtGui/qtextedit.h"

#ifndef QT_NO_TEXTEDIT

class QMimeData;

class QTextEditPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QTextEdit)
public:
    QTextEditPrivate();

    bool cursorMoveKeyEvent(QKeyEvent *e);

    void updateCurrentCharFormat();

    void indent();
    void outdent();

    void gotoNextTableCell();
    void gotoPreviousTableCell();

    void createAutoBulletList();

    void init(const QString &html = QString());
    void setContent(Qt::TextFormat format = Qt::RichText, const QString &text = QString(),
                    QTextDocument *document = 0);
#ifndef QT_NO_DRAGANDDROP
    void startDrag();
#endif

    void paste(const QMimeData *source);
    void paint(QPainter *p, QPaintEvent *e);

    void setCursorPosition(const QPoint &pos);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void _q_repaintContents(const QRectF &contentsRect);
    void repaintCursor();
    inline void repaintSelection()
    { viewport->update(selectionRect()); }

    inline QPoint mapToContents(const QPoint &point) const
    { return QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset()); }

    void selectionChanged();

    void pageUp(QTextCursor::MoveMode moveMode);
    void pageDown(QTextCursor::MoveMode moveMode);

    void _q_updateCurrentCharFormatAndSelection();

    void _q_adjustScrollbars();
#ifndef QT_NO_CLIPBOARD
    void setClipboardSelection();
#endif
    void ensureVisible(int documentPosition);
    void ensureVisible(const QRect &rect);

    void ensureViewportLayouted();

    void _q_emitCursorPosChanged(const QTextCursor &someCursor);

    void setBlinkingCursorEnabled(bool enable);

    void extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition);
    void extendLinewiseSelection(int suggestedNewPosition);

    void relayoutDocument();

    void _q_deleteSelected();

    void undo();
    void redo();
    void _q_setCursorAfterUndoRedo(int undoPosition, int charsAdded, int charsRemoved);
    
    inline int horizontalOffset() const
    { return q_func()->isRightToLeft() ? (hbar->maximum() - hbar->value()) : hbar->value(); }
    inline int verticalOffset() const
    { return vbar->value(); }
    
    QRect rectForPosition(int position) const;
    QRect selectionRect(const QTextCursor &cursor) const;
    inline QRect selectionRect() const
    { return selectionRect(this->cursor); }

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    QTextCharFormat lastCharFormat;

    QTextCursor dndFeedbackCursor;

    bool readOnly; /* ### move to document? */

    QTextEdit::AutoFormatting autoFormatting;
    bool tabChangesFocus;

    QBasicTimer cursorBlinkTimer;
    QBasicTimer autoScrollTimer;
    QBasicTimer trippleClickTimer;
    QPoint trippleClickPoint;

    bool mousePressed;

#ifndef QT_NO_DRAGANDDROP
    bool mightStartDrag;
    QPoint dragStartPos;
    QBasicTimer dragStartTimer;
#endif

    QTextEdit::LineWrapMode lineWrap;
    int lineWrapColumnOrWidth;

    bool lastSelectionState;

    bool ignoreAutomaticScrollbarAdjustement;

    // Qt3 COMPAT only
    // ### non-compat'ed append needs it, too
    Qt::TextFormat textFormat;
    bool preferRichText;

    QString anchorToScrollToWhenVisible;

    QTextCursor selectedWordOnDoubleClick;
    QTextCursor selectedLineOnDoubleClick;

    // for QTextBrowser:
    QTextCursor focusIndicator;

#ifdef QT_KEYPAD_NAVIGATION
    QBasicTimer deleteAllTimer;
#endif

    bool overwriteMode;
    bool acceptRichText;

    int preeditCursor;
    bool hideCursor; // used to hide the cursor in the preedit area

    QVector<QAbstractTextDocumentLayout::Selection> extraSelections;
};
#endif // QT_NO_TEXTEDIT

#ifndef QT_NO_CONTEXTMENU
class QUnicodeControlCharacterMenu : public QMenu
{
    Q_OBJECT
public:
    QUnicodeControlCharacterMenu(QObject *editWidget, QWidget *parent);

private Q_SLOTS:
    void menuActionTriggered();

private:
    QObject *editWidget;
};
#endif // QT_NO_CONTEXTMENU

// also used by QLabel
class QTextEditMimeData : public QMimeData
{
public:
    inline QTextEditMimeData(const QTextDocumentFragment &aFragment) : fragment(aFragment) {}

    virtual QStringList formats() const;
protected:
    virtual QVariant retrieveData(const QString &mimeType, QVariant::Type type) const;
private:
    void setup() const;

    mutable QTextDocumentFragment fragment;
};

#endif // QTEXTEDIT_P_H
