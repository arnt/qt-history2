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

#ifndef QTEXTCONTROL_P_P_H
#define QTEXTCONTROL_P_P_H

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

#include "QtGui/qtextdocumentfragment.h"
#include "QtGui/qscrollbar.h"
#include "QtGui/qtextcursor.h"
#include "QtGui/qtextformat.h"
#include "QtGui/qmenu.h"
#include "QtGui/qabstracttextdocumentlayout.h"
#include "QtCore/qbasictimer.h"

class QMimeData;

class QTextControlPrivate
{
    Q_DECLARE_PUBLIC(QTextControl)
public:
    inline QTextControlPrivate()
        : doc(0), cursorOn(false), readOnly(false),
          autoFormatting(QTextControl::AutoNone),
          mousePressed(false), mightStartDrag(false),
          lineWrap(QTextControl::WidgetWidth), lineWrapColumnOrWidth(0),
          lastSelectionState(false), ignoreAutomaticScrollbarAdjustement(false), textFormat(Qt::AutoText),
          preferRichText(false),
          overwriteMode(false),
          acceptRichText(true),
          preeditCursor(0), hideCursor(false)
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
/*
    void startDrag();
*/

    void paste(const QMimeData *source);

    void setCursorPosition(const QPoint &pos);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void repaintContents(const QRectF &contentsRect);
    void repaintCursor();
    inline void repaintSelection()
    { emit q_ptr->viewportUpdateRequest(selectionRect()); }

    inline QPointF mapToContents(const QPointF &point) const
    { return QPointF(point.x() + viewport.x(), point.y() + viewport.y()); }

    void selectionChanged();

    void pageUp(QTextCursor::MoveMode moveMode);
    void pageDown(QTextCursor::MoveMode moveMode);

    void updateCurrentCharFormatAndSelection();

    void setClipboardSelection();
    void ensureVisible(int documentPosition);
    void ensureVisible(const QRectF &rect);

    void ensureViewportLayouted();

    void emitCursorPosChanged(const QTextCursor &someCursor);

    void setBlinkingCursorEnabled(bool enable);

    void extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition);
    void extendLinewiseSelection(int suggestedNewPosition);

    void deleteSelected();
    
    inline void updateViewport()
    { emit q_ptr->viewportUpdateRequest(QRectF(QPointF(0, 0), viewport.size())); }

    void undo();
    void redo();
    void setCursorAfterUndoRedo(int undoPosition, int charsAdded, int charsRemoved);
    
    QRectF rectForPosition(int position) const;
    QRectF selectionRect() const;

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    QTextCharFormat lastCharFormat;

    QTextCursor dndFeedbackCursor;

    bool readOnly; /* ### move to document? */

    QTextControl::AutoFormatting autoFormatting;

    QBasicTimer cursorBlinkTimer;
    QBasicTimer autoScrollTimer;
    QBasicTimer trippleClickTimer;
    QPoint trippleClickPoint;

    bool mousePressed;

    bool mightStartDrag;
    QPoint dragStartPos;
    QBasicTimer dragStartTimer;

    QTextControl::LineWrapMode lineWrap;
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
    
    QRectF viewport;

    QPalette palette;
    
    QTextControl *q_ptr;
};

#endif // QTEXTCONTROL_P_H
