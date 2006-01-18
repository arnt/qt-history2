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

#ifndef QT_NO_TEXTEDIT

class QMimeData;

class QTextEditPrivate : public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QTextEdit)
public:
    inline QTextEditPrivate()
        : doc(0), cursorOn(false),
	  readOnly(false),
          autoFormatting(QTextEdit::AutoNone), tabChangesFocus(false),
#ifndef QT_NO_DRAGANDDROP
          mousePressed(false), mightStartDrag(false),
#endif
          lineWrap(QTextEdit::WidgetWidth), lineWrapColumnOrWidth(0),
          lastSelectionState(false), ignoreAutomaticScrollbarAdjustement(false), textFormat(Qt::AutoText),
          preferRichText(false),
          overwriteMode(false),
          acceptRichText(true),
          preeditCursor(-1)
    {}

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

    void repaintContents(const QRectF &contentsRect);
    void repaintCursor();
    inline void repaintSelection()
    { viewport->update(selectionRect()); }

    inline QPoint mapToContents(const QPoint &point) const
    { return QPoint(point.x() + hbar->value(), point.y() + vbar->value()); }

    void selectionChanged();

    void pageUp(QTextCursor::MoveMode moveMode);
    void pageDown(QTextCursor::MoveMode moveMode);

    void updateCurrentCharFormatAndSelection();

    void adjustScrollbars();
#ifndef QT_NO_CLIPBOARD
    void setClipboardSelection();
#endif
    void ensureVisible(int documentPosition);
    void ensureVisible(const QRect &rect);

    void ensureViewportLayouted();

    void emitCursorPosChanged(const QTextCursor &someCursor);

    void setBlinkingCursorEnabled(bool enable);

    void extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition);
    void extendLinewiseSelection(int suggestedNewPosition);

    void relayoutDocument();

    void deleteSelected();

    void undo();
    void redo();
    void setCursorAfterUndoRedo(int undoPosition, int charsAdded, int charsRemoved);
    
    QRect rectForPosition(int position) const;
    QRect selectionRect() const;

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

    QVector<QAbstractTextDocumentLayout::Selection> extraSelections;
};
#endif // QT_NO_TEXTEDIT

#ifndef QT_NO_MENU
class QUnicodeControlCharacterMenu : public QMenu
{
    Q_OBJECT
public:
    QUnicodeControlCharacterMenu(QWidget *editWidget, QWidget *parent);

private Q_SLOTS:
    void actionTriggered();

private:
    QWidget *editWidget;
};
#endif // QT_NO_MENU

#endif // QTEXTEDIT_P_H
