#ifndef QTEXTEDIT_P_H
#define QTEXTEDIT_P_H

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
	  imstart(0), imend(0), imselstart(0), imselend(0), readOnly(false),
          autoFormatting(QTextEdit::AutoNone), tabChangesFocus(false),
          mousePressed(false), mightStartDrag(false), wordWrap(QTextEdit::WidgetWidth), wrapColumnOrWidth(0),
          lastSelectionState(false), ignoreAutomaticScrollbarAdjustement(false), textFormat(Qt::AutoText)
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

    QRect cursorRect() const;

    void setBlinkingCursorEnabled(bool enable);

    // input methods
    bool composeMode() const { return preeditLength(); }
    bool hasIMSelection() const { return imSelectionLength(); }
    int preeditLength() const { return ( imend - imstart ); }
    int imSelectionLength() const { return ( imselend - imselstart ); }

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    QTextCharFormat lastCharFormat;

    int imstart, imend, imselstart, imselend;

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

    QString anchorToScrollToWhenVisible;
};

#endif // QTEXTEDIT_P_H
