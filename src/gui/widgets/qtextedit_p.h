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

#include <private/qviewport_p.h>
#include <qtextdocumentfragment.h>
#include <qscrollbar.h>
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qbasictimer.h>

class QMimeData;

class QTextEditPrivate : public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QTextEdit)
public:
    inline QTextEditPrivate()
        : doc(0), cursorOn(false),
	  readOnly(false),
          autoFormatting(QTextEdit::AutoNone), tabChangesFocus(false),
          mousePressed(false), mightStartDrag(false), wordWrap(QTextEdit::WidgetWidth), wrapColumnOrWidth(0),
          lastSelectionState(false), ignoreAutomaticScrollbarAdjustement(false), textFormat(Qt::AutoText),
          preferRichText(false)
    {}

    bool cursorMoveKeyEvent(QKeyEvent *e);

    void updateCurrentCharFormat();

    void indent();
    void outdent();

    void gotoNextTableCell();
    void gotoPreviousTableCell();

    void createAutoBulletList();

    void init(const QTextDocumentFragment &fragment = QTextDocumentFragment(),
              QTextDocument *document = 0);

    void startDrag();

    void paste(const QMimeData *source);

    void setCursorPosition(const QPoint &pos);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void update(const QRect &contentsRect);

    inline QPoint translateCoordinates(const QPoint &point)
    { return QPoint(point.x() + hbar->value(), point.y() + vbar->value()); }

    void selectionChanged();

    inline int contentsX() const { return hbar->value(); }
    inline int contentsY() const { return vbar->value(); }
    inline int contentsWidth() const { return hbar->maximum() + viewport->width(); }
    inline int contentsHeight() const { return vbar->maximum() + viewport->height(); }

    bool pageUp(QTextCursor::MoveMode moveMode);
    bool pageDown(QTextCursor::MoveMode moveMode);

    void updateCurrentCharFormatAndSelection();

    void adjustScrollbars();

    void setClipboardSelection();

    void ensureVisible(int documentPosition);

    void emitCursorPosChanged(const QTextCursor &someCursor);

    QRect cursorRect() const;

    void setBlinkingCursorEnabled(bool enable);

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    QTextCharFormat lastCharFormat;

    bool readOnly; /* ### move to document? */

    QTextEdit::AutoFormatting autoFormatting;
    bool tabChangesFocus;

    QBasicTimer cursorBlinkTimer;

    QBasicTimer trippleClickTimer;
    QPoint trippleClickPoint;

    bool mousePressed;

    bool mightStartDrag;
    QPoint dragStartPos;
    QBasicTimer dragStartTimer;

    QTextEdit::WordWrap wordWrap;
    int wrapColumnOrWidth;

    bool lastSelectionState;

    bool ignoreAutomaticScrollbarAdjustement;

    // Qt3 COMPAT only
    // ### non-compat'ed append needs it, too
    Qt::TextFormat textFormat;
    bool preferRichText;

    QString anchorToScrollToWhenVisible;

    QTextCursor focusIndicator;
    QTextCursor cursorOnDoubleClick;
};

#endif // QTEXTEDIT_P_H
