#include "qtextedit.h"

#include <qfont.h>
#include <qpainter.h>
#include <qevent.h>
#include <qdebug.h>
#include <qdragobject.h>
#include <qclipboard.h>
#include <qmenu.h>
#include <qstyle.h>
#include <qbasictimer.h>
#include <qtimer.h>
#include <qscrollbar.h>
#include <private/qviewport_p.h>

#include "private/qtextdocumentlayout_p.h"
#include "private/qtextdocument_p.h"
#include "qtextdocument.h"
#include "qtextcursor.h"
#include "qtextdocumentfragment.h"
#include "qtextlist.h"

#include <qdatetime.h>
#include <qapplication.h>
#include <limits.h>
#include <qtexttable.h>

#ifndef QT_NO_ACCEL
#include <qkeysequence.h>
#define ACCEL_KEY(k) "\t" + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

#define d d_func()
#define q q_func()

class QRichTextDrag : public QTextDrag
{
public:
    QRichTextDrag(const QTextDocumentFragment &_fragment, QWidget *dragSource);

    virtual const char *format(int i) const;
    virtual QByteArray encodedData(const char *mime) const;

    static bool decode(const QMimeSource *e, QTextDocumentFragment &fragment);
    static bool canDecode(const QMimeSource* e);

private:
    QTextDocumentFragment fragment;
    mutable bool plainTextSet;
};

QRichTextDrag::QRichTextDrag(const QTextDocumentFragment &_fragment, QWidget *dragSource)
    : QTextDrag(dragSource), fragment(_fragment), plainTextSet(false)
{
}

const char *QRichTextDrag::format(int i) const
{
    const char *fmt = QTextDrag::format(i);
    if (fmt)
        return fmt;
    if (QTextDrag::format(i - 1))
        return "application/x-qt-richtext";
    return 0;
}

QByteArray QRichTextDrag::encodedData(const char *mime) const
{
    if (qstrcmp(mime, "application/x-qt-richtext") == 0) {
        QByteArray binary;
        QDataStream stream(&binary, IO_WriteOnly);
        stream << fragment;
        return binary;
    }

    if (!plainTextSet) {
        const_cast<QRichTextDrag *>(this)->setText(fragment.toPlainText());
        plainTextSet = true;
    }

    return QTextDrag::encodedData(mime);
}

bool QRichTextDrag::decode(const QMimeSource *e, QTextDocumentFragment &fragment)
{
    if (e->provides("application/x-qt-richtext")) {
        QDataStream stream(e->encodedData("application/x-qt-richtext"), IO_ReadOnly);
        stream >> fragment;
        return true;
    } else if (e->provides("application/x-qrichtext")) {
        fragment = QTextDocumentFragment::fromHTML(e->encodedData("application/x-qrichtext"));
        return true;
    }

    QString plainText;
    if (!QTextDrag::decode( e, plainText ))
        return false;

    fragment = QTextDocumentFragment::fromPlainText(plainText);
    return true;
}

bool QRichTextDrag::canDecode(const QMimeSource* e)
{
    if (e->provides("application/x-qt-richtext")
        || e->provides("application/x-qrichtext"))
        return true;
    return QTextDrag::canDecode(e);
}

class QTextEditPrivate : public QViewportPrivate
{
    Q_DECLARE_PUBLIC(QTextEdit)
public:
    inline QTextEditPrivate()
        : doc(0), cursorOn(false), readOnly(false),
          autoFormatting(QTextEdit::AutoAll), tabChangesFocus(false), trippleClickTimerActive(false),
          mousePressed(false), mightStartDrag(false), wordWrap(QTextEdit::WidgetWidth), wrapColumnOrWidth(0),
          lastSelectionState(false), textFormat(Qt::AutoText)
    {}

    bool cursorMoveKeyEvent(QKeyEvent *e);

    void updateCurrentCharFormat();

    void indent();
    void outdent();

    void createAutoBulletList();

    void init(const QTextDocumentFragment &fragment = QTextDocumentFragment(),
              QTextDocument *document = 0);

    void startDrag();

    void paste(const QMimeSource *source);

    inline void trippleClickTimeout()
    { trippleClickTimerActive = false; }

    void setCursorPosition(const QPoint &pos);
    void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    void update(const QRect &contentsRect);

    inline QPoint translateCoordinates(const QPoint &point)
    { return QPoint(point.x() + hbar->value(), point.y() + vbar->value()); }

    void selectionChanged();

    // helper for compat functions
    QTextBlock blockAt(const QPoint &pos, int *documentPosition = 0) const;

    inline int contentsX() const { return hbar->value(); }
    inline int contentsY() const { return vbar->value(); }
    inline int contentsWidth() const { return hbar->maximum() + viewport->width(); }
    inline int contentsHeight() const { return vbar->maximum() + viewport->height(); }

    bool pageUp(QTextCursor::MoveMode moveMode);
    bool pageDown(QTextCursor::MoveMode moveMode);

    void updateCurrentCharFormatAndSelection();

    void adjustScrollbars();

    QTextDocument *doc;
    bool cursorOn;
    QTextCursor cursor;
    QTextCharFormat currentCharFormat;

    bool readOnly; /* ### move to document? */

    QTextEdit::AutoFormatting autoFormatting;
    bool tabChangesFocus;

    QBasicTimer cursorBlinkTimer;

    bool trippleClickTimerActive;
    QPoint trippleClickPoint;

    bool mousePressed;

    bool mightStartDrag;
    QPoint dragStartPos;
    QBasicTimer dragStartTimer;

    QTextEdit::WordWrap wordWrap;
    int wrapColumnOrWidth;

    bool lastSelectionState;

    // Qt3 COMPAT only
    // ### non-compat'ed append needs it, too
    Qt::TextFormat textFormat;
};

bool QTextEditPrivate::cursorMoveKeyEvent(QKeyEvent *e)
{
    QTextCursor::MoveMode mode = e->state() & Qt::ShiftButton
                                   ? QTextCursor::KeepAnchor
                                   : QTextCursor::MoveAnchor;

    QTextCursor::MoveOperation op = QTextCursor::NoMove;
#ifdef Q_WS_MAC
    // There can be only one modifier (+ shift), but we also need to make sure
    // that we have a "move key" pressed before we reject it.
    bool twoModifiers
        = ((e->state() & (Qt::ControlButton | Qt::AltButton))
           == (Qt::ControlButton | Qt::AltButton))
        || ((e->state() & (Qt::ControlButton | Qt::MetaButton))
            == (Qt::ControlButton | Qt::MetaButton))
        || ((e->state() & (Qt::AltButton | Qt::MetaButton)) == (Qt::AltButton | Qt::MetaButton));
#endif
    switch (e->key()) {
#ifndef Q_WS_MAC  // Use the default Windows bindings.
        case Qt::Key_Up:
            op = QTextCursor::Up;
            break;
        case Qt::Key_Down:
            op = QTextCursor::Down;
            break;
        case Qt::Key_Left:
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::WordLeft
                 : QTextCursor::Left;
            break;
        case Qt::Key_Right:
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::WordRight
                 : QTextCursor::Right;
            break;
        case Qt::Key_Home:
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::Start
                 : QTextCursor::StartOfLine;
            break;
        case Qt::Key_End:
            op = e->state() & Qt::ControlButton
                 ? QTextCursor::End
                 : QTextCursor::EndOfLine;
            break;
#else
// Except for pageup and pagedown, Mac OS X has very different behavior, we don't do it all, but
// here's the breakdown:
// Shift still works as an anchor, but only one of the other keys can be down Ctrl (Command),
// Alt (Option), or Meta (Control).
// Command/Control + Left/Right -- Move to left or right of the line
//                 + Up/Down -- Move to top bottom of the file. (Control doesn't move the cursor)
// Option + Left/Right -- Move one word Left/right.
//        + Up/Down  -- Begin/End of Paragraph.
// Home/End Top/Bottom of file. (usually don't move the cursor, but will select)
        case Qt::Key_Up:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::Start;
                else if (e->state() & Qt::AltButton)
                    op = QTextCursor::StartOfBlock;
                else
                    op = QTextCursor::Up;
            }
            break;
        case Qt::Key_Down:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::End;
                else if (e->state() & Qt::AltButton)
                    op = QTextCursor::EndOfBlock;
                else
                    op = QTextCursor::Down;
            }
            break;
        case Qt::Key_Left:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::StartOfLine;
                else if (e->state() & Qt::AltButton)
                    op = QTextCursor::WordLeft;
                else
                    op = QTextCursor::Left;
            }
            break;
        case Qt::Key_Right:
            if (twoModifiers) {
                QApplication::beep();
                return true;
            } else {
                if (e->state() & (Qt::ControlButton | Qt::MetaButton))
                    op = QTextCursor::EndOfLine;
                else if (e->state() & Qt::AltButton)
                    op = QTextCursor::WordRight;
                else
                    op = QTextCursor::Right;
            }
            break;
        case Qt::Key_Home:
            if (e->state() & (Qt::ControlButton | Qt::MetaButton | Qt::AltButton)) {
                QApplication::beep();
                return true;
            } else {
                op = QTextCursor::Start;
            }
            break;
        case Qt::Key_End:
            if (e->state() & (Qt::ControlButton | Qt::MetaButton | Qt::AltButton)) {
                QApplication::beep();
                return true;
            } else {
                op = QTextCursor::End;
            }
            break;
#endif
        case Qt::Key_Next:
            return pageDown(mode);
        case Qt::Key_Prior:
            return pageUp(mode);
    default:
        return false;
    }

    cursor.movePosition(op, mode);
    q->ensureCursorVisible();

    selectionChanged();

    // ####
    viewport->update();

    return true;
}

void QTextEditPrivate::updateCurrentCharFormat()
{
    currentCharFormat = cursor.charFormat();

    emit q->currentCharFormatChanged(currentCharFormat);
    // compat signals
    emit q->currentFontChanged(currentCharFormat.font());
    emit q->currentColorChanged(currentCharFormat.color());
}

void QTextEditPrivate::indent()
{
    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextList *list = cursor.currentList();
    if (!list) {
        QTextBlockFormat modifier;
        modifier.setIndent(blockFmt.indent() + 1);
        cursor.mergeBlockFormat(modifier);
    } else {
        QTextListFormat format = list->format();
        format.setIndent(format.indent() + 1);

        if (list->itemNumber(cursor.block()) == 1)
            list->setFormat(format);
        else
            cursor.createList(format);
    }
}

void QTextEditPrivate::outdent()
{
    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextList *list = cursor.currentList();

    if (!list) {
        QTextBlockFormat modifier;
        modifier.setIndent(blockFmt.indent() - 1);
        cursor.mergeBlockFormat(modifier);
    } else {
        QTextListFormat listFmt = list->format();
        listFmt.setIndent(listFmt.indent() - 1);
        list->setFormat(listFmt);
    }
}

void QTextEditPrivate::createAutoBulletList()
{
    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextListFormat listFmt;
    listFmt.setStyle(QTextListFormat::ListDisc);
    listFmt.setIndent(blockFmt.indent() + 1);

    blockFmt.setIndent(0);
    cursor.setBlockFormat(blockFmt);

    cursor.createList(listFmt);

    cursor.endEditBlock();
}

void QTextEditPrivate::init(const QTextDocumentFragment &fragment, QTextDocument *document)
{
    if (!doc) {
        doc = document;
        if (!doc)
            doc = new QTextDocument(q);

        QObject::connect(doc->documentLayout(), SIGNAL(update(const QRect &)), q, SLOT(update(const QRect &)));
        QObject::connect(doc->documentLayout(), SIGNAL(usedSizeChanged()), q, SLOT(adjustScrollbars()));
        cursor = QTextCursor(doc);

        hbar->setSingleStep(20);
        vbar->setSingleStep(20);

        QObject::connect(doc, SIGNAL(contentsChanged()), q, SLOT(updateCurrentCharFormatAndSelection()));

        // compat signals
        QObject::connect(doc, SIGNAL(contentsChanged()), q, SIGNAL(textChanged()));
        QObject::connect(doc, SIGNAL(undoAvailable(bool)), q, SIGNAL(undoAvailable(bool)));
        QObject::connect(doc, SIGNAL(redoAvailable(bool)), q, SIGNAL(redoAvailable(bool)));

        cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, q);

        viewport->setBackgroundRole(QPalette::Base);
        viewport->setAcceptDrops(true);
        viewport->setFocusProxy(q);
        q->setFocusPolicy(Qt::WheelFocus);
    }

    q->clear();

    doc->setModified(false);

    QTextCharFormat fmt;
    fmt.setFont(q->font());
    fmt.setColor(q->palette().color(QPalette::Text));

    // ###############
//     d->cursor.movePosition(QTextCursor::Start);
//     d->cursor.setBlockFormat(fmt);

    viewport->setCursor(readOnly ? Qt::ArrowCursor : Qt::IbeamCursor);

    if (fragment.isEmpty())
        return;
    cursor.movePosition(QTextCursor::Start);
    doc->setUndoRedoEnabled(false);
    cursor.insertFragment(fragment);
    doc->setUndoRedoEnabled(true);

    cursor.movePosition(QTextCursor::Start);
    updateCurrentCharFormatAndSelection();
}

void QTextEditPrivate::startDrag()
{
    mousePressed = false;
    QRichTextDrag *drag = new QRichTextDrag(cursor, viewport);
    if (readOnly)
        drag->dragCopy();
    else
        drag->drag();
}

void QTextEditPrivate::paste(const QMimeSource *source)
{
    if (readOnly || !source || !QRichTextDrag::canDecode(source))
	return;

    QTextDocumentFragment fragment;
    if (!QRichTextDrag::decode(source, fragment))
	return;

    cursor.insertFragment(fragment);
}

void QTextEditPrivate::setCursorPosition(const QPoint &pos)
{
    const int cursorPos = doc->documentLayout()->hitTest(pos, QText::FuzzyHit);
    if (cursorPos == -1)
        return;
    cursor.setPosition(cursorPos);
}

void QTextEditPrivate::setCursorPosition(int pos, QTextCursor::MoveMode mode)
{
    cursor.setPosition(pos, mode);
    q->ensureCursorVisible();
}

void QTextEditPrivate::update(const QRect &contentsRect)
{
    const int xOffset = hbar->value();
    const int yOffset = vbar->value();
    const QRect visibleRect(xOffset, yOffset, viewport->width(), viewport->height());

    QRect r = contentsRect.intersect(visibleRect);
    if (r.isEmpty())
        return;

    r.moveBy(-xOffset, -yOffset);
    viewport->update(r);
}

void QTextEditPrivate::selectionChanged()
{
    bool current = cursor.hasSelection();
    if (current == lastSelectionState)
        return;

    lastSelectionState = current;
    emit q->copyAvailable(current);
    emit q->selectionChanged();
}

QTextBlock QTextEditPrivate::blockAt(const QPoint &pos, int *documentPosition) const
{
    const int docPos = doc->documentLayout()->hitTest(pos, QText::ExactHit);

    if (docPos == -1) {
        if (documentPosition)
            *documentPosition = -1;
        return QTextBlock();
    }

    if (documentPosition)
        *documentPosition = docPos;

    QTextDocumentPrivate *pt = doc->docHandle();
    return QTextBlock(pt, pt->blockMap().findNode(docPos));
}

bool QTextEditPrivate::pageUp(QTextCursor::MoveMode moveMode)
{
    int targetY = vbar->value() - viewport->height();
    bool moved = false;
    do {
        q->ensureCursorVisible();
        moved = cursor.movePosition(QTextCursor::Up, moveMode);
    } while (moved && vbar->value() > targetY);
    return moved;
}

bool QTextEditPrivate::pageDown(QTextCursor::MoveMode moveMode)
{
    int targetY = vbar->value() + viewport->height();
    bool moved = false;
    do {
        q->ensureCursorVisible();
        moved = cursor.movePosition(QTextCursor::Down, moveMode);
    } while (moved && vbar->value() < targetY);
    return moved;
}

void QTextEditPrivate::updateCurrentCharFormatAndSelection()
{
    updateCurrentCharFormat();
    selectionChanged();
}

void QTextEditPrivate::adjustScrollbars()
{
    QAbstractTextDocumentLayout *layout = doc->documentLayout();

    const QSize viewportSize = viewport->size();
    const QSize docSize = layout->sizeUsed();

    d->hbar->setRange(0, docSize.width() - viewportSize.width());
    d->hbar->setPageStep(viewportSize.width());

    d->vbar->setRange(0, docSize.height() - viewportSize.height());
    d->vbar->setPageStep(viewportSize.height());
}

/*!
    Constructs an empty QTextEdit with parent \a
    parent.
*/
QTextEdit::QTextEdit(QWidget *parent)
    : QViewport(*new QTextEditPrivate, parent)
{
    d->init();
}

/*!
    Constructs a QTextEdit with parent \a parent. The text edit will display
    the text \a text. The text is interpreted as html.
*/
QTextEdit::QTextEdit(const QString &text, QWidget *parent)
    : QViewport(*new QTextEditPrivate, parent)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->init(fragment);
}

#ifdef QT_COMPAT
QTextEdit::QTextEdit(QWidget *parent, const char *name)
    : QViewport(*new QTextEditPrivate, parent)
{
    d->init();
    setObjectName(name);
}
#endif


QTextEdit::~QTextEdit()
{
}

/*!
    Returns the point size of the font of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
float QTextEdit::fontPointSize() const
{
    return d->currentCharFormat.fontPointSize();
}

/*!
    Returns the font family of the current format.

    \sa setFontFamily() setCurrentFont() setFontPointSize()
*/
QString QTextEdit::fontFamily() const
{
    return d->currentCharFormat.fontFamily();
}

/*!
    Returns the font weight of the current format.

    \sa setFontWeight() setCurrentFont() setFontPointSize()
*/
int QTextEdit::fontWeight() const
{
    return d->currentCharFormat.fontWeight();
}

/*!
    Returns true if the font of the current format is underlined; otherwise returns
    false.

    \sa setFontUnderline()
*/
bool QTextEdit::fontUnderline() const
{
    return d->currentCharFormat.fontUnderline();
}

/*!
    Returns true if the font of the current format is italic; otherwise returns
    false.

    \sa setFontItalic()
*/
bool QTextEdit::fontItalic() const
{
    return d->currentCharFormat.fontItalic();
}

/*!
    Returns the color of the current format.

    \sa setColor()
*/
QColor QTextEdit::color() const
{
    return d->currentCharFormat.color();
}

/*!
    Returns the font of the current format.

    \sa setCurrentFont() setFontFamily() setFontPointSize()
*/
QFont QTextEdit::currentFont() const
{
    return d->currentCharFormat.font();
}

/*!
    Sets the alignment of the current paragraph to \a a. Valid
    alignments are \c Qt::AlignLeft, \c Qt::AlignRight,
    \c Qt::AlignJustify and \c Qt::AlignCenter (which centers
    horizontally).
*/
void QTextEdit::setAlignment(Qt::Alignment a)
{
    if (d->readOnly)
	return;
    QTextBlockFormat fmt;
    fmt.setAlignment(a);
    d->cursor.mergeBlockFormat(fmt);
}

/*!
    Returns the alignment of the current paragraph.

    \sa setAlignment()
*/
Qt::Alignment QTextEdit::alignment() const
{
    return d->cursor.blockFormat().alignment();
}

/*!
    Makes \a document the new document of the text editor. 

    The parent QObject of the provided document remains the owner
    of the object. If the current document is a child of the text
    editor, then it is deleted.

    \sa document()
*/
void QTextEdit::setDocument(QTextDocument *document)
{
    d->doc->disconnect(this);
    d->doc->documentLayout()->disconnect(this);

    if (d->doc->parent() == this)
        delete d->doc;

    d->doc = 0;
    d->init(QTextDocumentFragment(), document);
}

/*!
    Returns a pointer to the underlying document.

    \sa setDocument()
*/
QTextDocument *QTextEdit::document() const
{
    return d->doc;
}

/*!
    Sets the visible cursor to \a cursor.
*/
void QTextEdit::setCursor(const QTextCursor &cursor)
{
    d->cursor = cursor;
    d->updateCurrentCharFormatAndSelection();
    ensureCursorVisible();
}

/*!
    Returns a QTextCursor that represents the currently visible cursor.
 */
QTextCursor QTextEdit::cursor() const
{
    return d->cursor;
}

/*!
    Sets the font family of the current format to \a fontFamily.

    \sa fontFamily() setCurrentFont()
*/
void QTextEdit::setFontFamily(const QString &fontFamily)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontFamily(fontFamily);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the point size of the current format to \a s.

    Note that if \a s is zero or negative, the behavior of this
    function is not defined.

    \sa fontPointSize() setCurrentFont() setFontFamily()
*/
void QTextEdit::setFontPointSize(float s)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontPointSize(s);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the font weight of the current format to \a w.

    \sa fontWeight() setCurrentFont() setFontFamily()
*/
void QTextEdit::setFontWeight(int w)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontWeight(w);
    mergeCurrentCharFormat(fmt);
}

/*!
    If \a b is true sets the current format to underline; otherwise
    sets the current format to non-underline.

    \sa fontUnderline()
*/
void QTextEdit::setFontUnderline(bool b)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontUnderline(b);
    mergeCurrentCharFormat(fmt);
}

/*!
    If \a b is true sets the current format to italic; otherwise sets
    the current format to non-italic.

    \sa fontItalic()
*/
void QTextEdit::setFontItalic(bool b)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFontItalic(b);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the color of the current format, i.e. of the text, to \a c.

    \sa color()
*/
void QTextEdit::setColor(const QColor &c)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setColor(c);
    mergeCurrentCharFormat(fmt);
}

/*!
    Sets the font of the current format to \a f.

    \sa currentFont() setFontPointSize() setFontFamily()
*/
void QTextEdit::setCurrentFont(const QFont &f)
{
    if (d->readOnly)
	return;
    QTextCharFormat fmt;
    fmt.setFont(f);
    mergeCurrentCharFormat(fmt);
}

/*!
    \fn QTextEdit::undo()

    Undoes the last operation.

    If there is no operation to undo, i.e. there is no undo step in
    the undo/redo history, nothing happens.

    \sa redo()
*/

/*!
    \fn QTextEdit::redo()

    Redoes the last operation.

    If there is no operation to redo, i.e. there is no redo step in
    the undo/redo history, nothing happens.

    \sa undo()
*/

/*!
    Copies the selected text to the clipboard and deletes it from
    the text edit.

    If there is no selected text nothing happens.

    \sa copy() paste()
*/

void QTextEdit::cut()
{
    if (d->readOnly || !d->cursor.hasSelection())
	return;
    copy();
    d->cursor.removeSelectedText();
}

/*!
    Copies any selected text to the clipboard.

    \sa copyAvailable()
*/

void QTextEdit::copy()
{
    if (!d->cursor.hasSelection())
	return;
    QRichTextDrag *drag = new QRichTextDrag(d->cursor, 0);
    QApplication::clipboard()->setData(drag);
}

/*!
    Pastes the text from the clipboard into the text edit at the
    current cursor position.

    If there is no text in the clipboard nothing happens.

    \sa cut() copy()
*/

void QTextEdit::paste()
{
    d->paste(QApplication::clipboard()->data());
}

/*!
    Deletes all the text in the text edit.

    \sa cut() removeSelectedText() setPlainText() setHtml()
*/
void QTextEdit::clear()
{
    selectAll();
    d->cursor.removeSelectedText();
}


void QTextEdit::selectAll()
{
    d->cursor.movePosition(QTextCursor::Start);
    d->cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    d->selectionChanged();
}

/*! \internal
*/

void QTextEdit::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == d->cursorBlinkTimer.timerId()) {
        d->cursorOn = !d->cursorOn;

        if (d->cursor.hasSelection())
            d->cursorOn &= (style().styleHint(QStyle::SH_BlinkCursorWhenTextSelected) != 0);

        QRect r = d->cursor.block().layout()->rect();
        r.moveBy(-d->hbar->value(), -d->vbar->value());
        d->viewport->update(r);
    } else if (ev->timerId() == d->dragStartTimer.timerId()) {
        d->dragStartTimer.stop();
        d->startDrag();
    }
}

/*!
    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as plain text.

    Note that the undo/redo history is cleared by this function.

    \sa plainText()
*/

void QTextEdit::setPlainText(const QString &text)
{
    if (text.isEmpty())
        return;

    QTextDocumentFragment fragment = QTextDocumentFragment::fromPlainText(text);
    d->init(fragment);
}

/*!
    Returns the text of the text edit as plain text.

    \sa setPlainText
 */
QString QTextEdit::plainText() const
{
    return d->doc->plainText();
}

/*!
    \overload

    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as rich text in html format.

    Note that the undo/redo history is cleared by this function.
*/

void QTextEdit::setHtml(const QString &text)
{
    if (text.isEmpty())
        return;

    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->init(fragment);
}
/*!
    \overload

    Changes the text of the text edit to the string \a text.
    Any previous text is removed.

    \a text is interpreted as rich text in html format. Any encoding
    attribute specified in the header of the html is obeyed.

    Note that the undo/redo history is cleared by this function.
*/

void QTextEdit::setHtml(const QByteArray &text)
{
    if (text.isEmpty())
	return;

    // use QByteArray overload that obeys html content encoding
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->init(fragment);
}

/*!
    Processes the key event, \a e. By default key events are used to
    provide keyboard navigation and text editing.
*/
void QTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->readOnly) {
        QViewport::keyPressEvent(e);
        return;
    }

    bool updateCurrentFormat = true;

    if (d->cursorMoveKeyEvent(e))
        goto accept;

    if (e->state() & Qt::ControlButton) {
        switch( e->key() ) {
        case Qt::Key_Z:
            d->doc->undo();
            break;
        case Qt::Key_Y:
            d->doc->redo();
            break;
        case Qt::Key_X:
        case Qt::Key_F20:  // Cut key on Sun keyboards
            cut();
            break;
        case Qt::Key_C:
        case Qt::Key_F16: // Copy key on Sun keyboards
            copy();
            break;
        case Qt::Key_V:
        case Qt::Key_F18:  // Paste key on Sun keyboards
            paste();
            break;
        case Qt::Key_Backspace:
            d->cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            goto process;
        case Qt::Key_Delete:
            d->cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
            goto process;
        case Qt::Key_K: {
            QTextBlock block = d->cursor.block();
            if (d->cursor.position() == block.position() + block.length() - 2)
                d->cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            else
                d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            d->cursor.deleteChar();
            break;
        }
        default:
            e->ignore();
            return;
        }
        goto accept;
    }

process:
    switch( e->key() ) {
    case Qt::Key_Backspace: {
        QTextList *list = d->cursor.currentList();
        if (list && d->cursor.atBlockStart())
            list->remove(d->cursor.block());
        else
            d->cursor.deletePreviousChar();
        break;
    }
    case Qt::Key_Delete:
        d->cursor.deleteChar();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        d->cursor.insertBlock();
        break;
    default:
        {
            QString text = e->text();

            if (e->key() == Qt::Key_Tab) {
                if (d->tabChangesFocus) {
                    e->ignore();
                    return;
                }
                if (d->cursor.atBlockStart()) {
                    d->indent();
                    break;
                }
            }

            if (e->key() == Qt::Key_Backtab) {
                if (d->tabChangesFocus) {
                    e->ignore();
                    return;
                }
                if (d->cursor.atBlockStart()) {
                    d->outdent();
                    break;
                }
            }

            if (d->cursor.atBlockStart()
                && (d->autoFormatting & AutoBulletList)
                && (!text.isEmpty())
                && (text[0] == '-' || text[0] == '*')
                && (!d->cursor.currentList())) {

                text.remove(0, 1);
                d->createAutoBulletList();
            }

            if (!text.isEmpty()) {
                d->cursor.insertText(text, d->currentCharFormat);
                updateCurrentFormat = false;
            } else {
                QViewport::keyPressEvent(e);
                return;
            }
            break;
        }
    }

 accept:
    e->accept();
    d->cursorOn = true;

    ensureCursorVisible();

    if (updateCurrentFormat)
        d->updateCurrentCharFormat();

}

void QTextEdit::resizeEvent(QResizeEvent *)
{
    QTextDocumentLayout *layout = qt_cast<QTextDocumentLayout *>(d->doc->documentLayout());
    Q_ASSERT(layout);

    if (d->wordWrap == NoWrap)
        layout->setBlockTextFlags(layout->blockTextFlags() | Qt::TextSingleLine);
    else
        layout->setBlockTextFlags(layout->blockTextFlags() & ~Qt::TextSingleLine);

    int width = d->viewport->width();
    if (d->wordWrap == FixedPixelWidth)
        width = d->wrapColumnOrWidth;

    if (d->wordWrap == FixedColumnWidth)
        layout->setFixedColumnWidth(d->wrapColumnOrWidth);
    else
        layout->setFixedColumnWidth(-1);

    d->doc->documentLayout()->setPageSize(QSize(width, INT_MAX));

    d->adjustScrollbars();
}

void QTextEdit::paintEvent(QPaintEvent *ev)
{
    QPainter p(d->viewport);

    const int xOffset = horizontalScrollBar()->value();
    const int yOffset = verticalScrollBar()->value();
    const QRect r = ev->rect();

    p.translate(-xOffset, -yOffset);
    p.setClipRect(xOffset + r.x(), yOffset + r.y(), r.width(), r.height());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.showCursor = d->cursorOn && d->viewport->hasFocus();
    ctx.cursor = d->cursor;
    ctx.palette = palette();

    d->doc->documentLayout()->draw(&p, ctx);
}

void QTextEdit::mousePressEvent(QMouseEvent *ev)
{
    if (!(ev->button() & Qt::LeftButton))
	return;

    QPoint pos = d->translateCoordinates(ev->pos());

    d->mousePressed = true;
    d->mightStartDrag = false;

    if (d->trippleClickTimerActive
        && ((ev->globalPos() - d->trippleClickPoint).manhattanLength() < QApplication::startDragDistance())) {

        d->cursor.movePosition(QTextCursor::StartOfLine);
        d->cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        d->trippleClickTimerActive = false;
    } else {
        int cursorPos = d->doc->documentLayout()->hitTest(pos, QText::FuzzyHit);
        if (cursorPos != -1) {

            if (d->cursor.hasSelection()
                && cursorPos >= d->cursor.selectionStart()
                && cursorPos <= d->cursor.selectionEnd()) {
                d->mightStartDrag = true;
                d->dragStartPos = ev->globalPos();
                d->dragStartTimer.start(QApplication::startDragTime(), this);
                return;
            }

            d->setCursorPosition(cursorPos);
        }

        d->cursor.clearSelection();
    }

    d->updateCurrentCharFormatAndSelection();
    d->viewport->update();
}

void QTextEdit::mouseMoveEvent(QMouseEvent *ev)
{
    int cursorPos = d->doc->documentLayout()->hitTest(d->translateCoordinates(ev->pos()), QText::FuzzyHit);
    if (cursorPos == -1)
	return;

    if (!(ev->state() & Qt::LeftButton))
	return;

    if (!d->mousePressed)
        return;

    if (d->mightStartDrag) {
        d->dragStartTimer.stop();

        if ((ev->globalPos() - d->dragStartPos).manhattanLength() > QApplication::startDragDistance())
            d->startDrag();

        return;
    }

    d->setCursorPosition(cursorPos, QTextCursor::KeepAnchor);
    d->updateCurrentCharFormatAndSelection();
    d->viewport->update();
}

void QTextEdit::mouseReleaseEvent(QMouseEvent *ev)
{
    if (d->mightStartDrag) {
        d->mousePressed = false;
        d->cursor.clearSelection();
        d->selectionChanged();
    }

    if (d->mousePressed) {
        d->mousePressed = false;
        if (d->cursor.hasSelection()) {
            QRichTextDrag *drag = new QRichTextDrag(d->cursor, this);
            QApplication::clipboard()->setData(drag, QClipboard::Selection);
        }
    } else if (ev->button() == Qt::MidButton
               && !d->readOnly
               && QApplication::clipboard()->supportsSelection()) {
        d->setCursorPosition(d->translateCoordinates(ev->pos()));
        d->paste(QApplication::clipboard()->data(QClipboard::Selection));
    }

    d->viewport->update();

    if (d->dragStartTimer.isActive())
        d->dragStartTimer.stop();
}

void QTextEdit::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton) {
        ev->ignore();
        return;
    }

    d->cursor.movePosition(QTextCursor::PreviousWord);
    d->cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    d->selectionChanged();
    d->viewport->update();

    d->trippleClickTimerActive = true;
    d->trippleClickPoint = ev->globalPos();
    QTimer::singleShot(qApp->doubleClickInterval(), this, SLOT(trippleClickTimeout()));
}

bool QTextEdit::focusNextPrevChild(bool next)
{
    Q_UNUSED(next)
// ###
    return d->readOnly;
//    if (d->cursor.atBlockStart())
//        return false;
//    return QScrollView::focusNextPrevChild(next);
}

void QTextEdit::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu *popup = createContextMenu(ev->pos());
    if (!popup)
	return;
    popup->exec(ev->globalPos());
    delete popup;
}

void QTextEdit::dragEnterEvent(QDragEnterEvent *ev)
{
    if (d->readOnly || !QRichTextDrag::canDecode(ev)) {
        ev->ignore();
        return;
    }
    ev->acceptAction();
}

void QTextEdit::dragMoveEvent(QDragMoveEvent *ev)
{
    if (d->readOnly || !QRichTextDrag::canDecode(ev)) {
        ev->ignore();
        return;
    }

    // don't change the cursor position here, as that would
    // destroy/change our visible selection and it would look ugly
    // and inconsistent. In Qt3's textedit the selection is independent
    // from the cursor, but now it's one thing. In Qt3 when dnd'ing
    // the cursor gets placed at where the text can be dropped. We can't
    // do this however, unless we introduce either a temporary selection
    // or a temporary second cursor. (Simon)

    ev->acceptAction();
}

void QTextEdit::dropEvent(QDropEvent *ev)
{
    if (d->readOnly || !QRichTextDrag::canDecode(ev)) {
        return;
    }
    ev->acceptAction();

    d->setCursorPosition(d->translateCoordinates(ev->pos()));
    d->paste(ev);
}

/*!
    This function is called to create a right mouse button popup menu
    at the document position \a pos. If you want to create a custom
    popup menu, reimplement this function and return the created popup
    menu. Ownership of the popup menu is transferred to the caller.
*/
QMenu *QTextEdit::createContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos);

    QMenu *menu = new QMenu(this);
    QAction *a;

    if (!d->readOnly) {
        a = menu->addAction(tr("&Undo") + ACCEL_KEY(Z), d->doc, SLOT(undo()));
        a->setEnabled(d->doc->isUndoAvailable());
        a = menu->addAction(tr("&Redo") + ACCEL_KEY(Y), d->doc, SLOT(redo()));
        a->setEnabled(d->doc->isRedoAvailable());
        menu->addSeparator();

        a = menu->addAction(tr("Cu&t") + ACCEL_KEY(X), this, SLOT(cut()));
        a->setEnabled(d->cursor.hasSelection());
    }

    a = menu->addAction(tr("&Copy") + ACCEL_KEY(C), this, SLOT(copy()));
    a->setEnabled(d->cursor.hasSelection());

    if (!d->readOnly) {
        a = menu->addAction(tr("&Paste") + ACCEL_KEY(V), this, SLOT(paste()));
        a->setEnabled(!QApplication::clipboard()->text().isEmpty());

        a = menu->addAction(tr("Clear"), this, SLOT(clear()));
        a->setEnabled(!d->doc->isEmpty());
    }

    menu->addSeparator();
    a = menu->addAction(tr("Select All")
#if !defined(Q_WS_X11)
                        + ACCEL_KEY(A)
#endif
                        , this, SLOT(selectAll()));

    a->setEnabled(!d->doc->isEmpty());

    return menu;
}

bool QTextEdit::isReadOnly() const
{
    return d->readOnly;
}

void QTextEdit::setReadOnly(bool ro)
{
    if (d->readOnly == ro)
        return;

    d->readOnly = ro;
    d->viewport->setCursor(d->readOnly ? Qt::ArrowCursor : Qt::IbeamCursor);

    if (ro)
        d->cursorBlinkTimer.stop();
    else
        d->cursorBlinkTimer.start(QApplication::cursorFlashTime() / 2, this);
}

/*!
    If the editor has a selection then the properties of \a modifier are
    applied to the selection. In addition they are merged into the current
    char format.

    \sa QTextCursor::mergeCharFormat
 */
void QTextEdit::mergeCurrentCharFormat(const QTextCharFormat &modifier)
{
    if (d->readOnly)
        return;

    if (d->cursor.hasSelection())
	d->cursor.mergeCharFormat(modifier);

    d->currentCharFormat.merge(modifier);
}

/*!
    Sets the char format that is be used when inserting new text to
    \a format .
 */
void QTextEdit::setCurrentCharFormat(const QTextCharFormat &format)
{
    d->currentCharFormat = format;
}

/*!
    Returns the char format that is used when inserting new text.
 */
QTextCharFormat QTextEdit::currentCharFormat() const
{
    return d->currentCharFormat;
}

QTextEdit::AutoFormatting QTextEdit::autoFormatting() const
{
    return d->autoFormatting;
}

/*!
    \property QTextEdit::autoFormatting
    \brief the enabled set of auto formatting features

    The value can be any combination of the values in the \c
    AutoFormattingFlag enum.  The default is \c AutoAll. Choose \c AutoNone
    to disable all automatic formatting.

    Currently, the only automatic formatting feature provided is \c
    AutoBulletList; future versions of Qt may offer more.
*/
void QTextEdit::setAutoFormatting(AutoFormatting features)
{
    d->autoFormatting = features;
}

/*!
    Convenience slot that inserts \a text at the current
    cursor position.

    It is equivalent to

    \code
    edit->cursor().insertText(text);
    \endcode
 */
void QTextEdit::insertPlainText(const QString &text)
{
    d->cursor.insertText(text);
}

/*!
    Convenience slot that inserts \a text which is assumed to be of
    html formatting at the current cursor position.

    It is equivalent to:

    \code
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    edit->cursor().insertFragment(fragment);
    \endcode
 */
void QTextEdit::insertHtml(const QString &text)
{
    QTextDocumentFragment fragment = QTextDocumentFragment::fromHTML(text);
    d->cursor.insertFragment(fragment);
}

bool QTextEdit::tabChangesFocus() const
{
    return d->tabChangesFocus;
}

/*! \property QTextEdit::tabChangesFocus
  \brief whether TAB changes focus or is accepted as input

  In some occasions text edits should not allow the user to input
  tabulators or change indentation using the TAB key, as this breaks
  the focus chain. The default is false.

*/

void QTextEdit::setTabChangesFocus(bool b)
{
    d->tabChangesFocus = b;
}

/*!
    \property QTextEdit::documentTitle
    \brief the title of the document parsed from the text.
*/

/*!
    \property QTextEdit::wordWrap
    \brief the word wrap mode

    The default mode is \c WidgetWidth which causes words to be
    wrapped at the right edge of the text edit. Wrapping occurs at
    whitespace, keeping whole words intact. If you want wrapping to
    occur within words use setWrapPolicy(). If you set a wrap mode of
    \c FixedPixelWidth or \c FixedColumnWidth you should also call
    setWrapColumnOrWidth() with the width you want.

    \sa WordWrap, wrapColumnOrWidth, wrapPolicy,
*/

QTextEdit::WordWrap QTextEdit::wordWrap() const
{
    return d->wordWrap;
}

void QTextEdit::setWordWrap(WordWrap wrap)
{
    if (d->wordWrap == wrap)
        return;
    d->wordWrap = wrap;
    // ####
    resizeEvent(0);
}

/*!
    \property QTextEdit::wrapColumnOrWidth
    \brief the position (in pixels or columns depending on the wrap mode) where text will be wrapped

    If the wrap mode is \c FixedPixelWidth, the value is the number of
    pixels from the left edge of the text edit at which text should be
    wrapped. If the wrap mode is \c FixedColumnWidth, the value is the
    column number (in character columns) from the left edge of the
    text edit at which text should be wrapped.

    \sa wordWrap
*/

int QTextEdit::wrapColumnOrWidth() const
{
    return d->wrapColumnOrWidth;
}

void QTextEdit::setWrapColumnOrWidth(int w)
{
    d->wrapColumnOrWidth = w;
    resizeEvent(0);
}

/*!
    Finds the next occurrence of the string, \a exp, using the given
    \a flags. Returns true if \a exp was found and changes the cursor
    to select the match; otherwise returns false;
*/
bool QTextEdit::find(const QString &exp, QTextDocument::FindFlags options)
{
    QTextCursor search = d->doc->find(exp, d->cursor, options);
    if (search.isNull())
        return false;

    setCursor(search);
    return true;
}

#ifdef QT_COMPAT
void QTextEdit::moveCursor(CursorAction action, QTextCursor::MoveMode mode)
{
    if (action == MovePageUp) {
        d->pageUp(mode);
        return;
    } else if (action == MovePageDown) {
        d->pageDown(mode);
        return;
    }

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
        default: return;
    }
    d->cursor.movePosition(op, mode);
    ensureCursorVisible();
    d->updateCurrentCharFormatAndSelection();
}

void QTextEdit::doKeyboardAction(KeyboardAction action)
{
    switch (action) {
        case ActionBackspace: d->cursor.deletePreviousChar(); break;
        case ActionDelete: d->cursor.deleteChar(); break;
        case ActionReturn: d->cursor.insertBlock(); break;
        case ActionKill: {
                QTextBlock block = d->cursor.block();
                if (d->cursor.position() == block.position() + block.length() - 2)
                    d->cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                else
                    d->cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                d->cursor.deleteChar();
                break;
            }
        case ActionWordBackspace:
            d->cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            d->cursor.deletePreviousChar();
            break;
        case ActionWordDelete:
            d->cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
            d->cursor.deleteChar();
            break;
    }
}

void QTextEdit::setText(const QString &text)
{
    if (d->textFormat == Qt::AutoText)
        d->textFormat = QText::mightBeRichText(text) ? Qt::RichText : Qt::PlainText;
    if (d->textFormat == Qt::RichText)
        setHtml(text);
    else
        setPlainText(text);
}

QString QTextEdit::text() const
{
    // ########## richtext case
    return document()->plainText();
}


void QTextEdit::setTextFormat(Qt::TextFormat f)
{
    d->textFormat = f;
}

Qt::TextFormat QTextEdit::textFormat() const
{
    return d->textFormat;
}

/*
static int blockNr(QTextBlock block)
{
    int nr = -1;

    for (; block.isValid(); block = block.previous())
        ++nr;

    return nr;
}

QTextBlock QTextEdit::blockAt(int blockNr) const
{
    QTextBlock block = d->doc->rootFrame()->begin().currentBlock();
    while (blockNr > 0 && block.isValid()) {
        block = block.next();
        --blockNr;
    }
    return block;
}

void QTextEdit::setCursorPosition(int parag, int index)
{
    QTextCursor c(blockAt(parag));
    c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, index);
    setCursor(c);
}

void QTextEdit::getCursorPosition(int *parag, int *index) const
{
    if (!parag || !index)
        return;

    QTextBlock block = d->cursor.block();

    Q_ASSERT(block.isValid());

    *parag = blockNr(block);
    *index = d->cursor.position() - block.position();
}

int QTextEdit::paragraphs() const
{
    return d->doc->docHandle()->numBlocks();
}

int QTextEdit::lines() const
{
    int l = 0;
    for (QTextBlock block = d->doc->rootFrame()->begin().currentBlock();
         block.isValid(); block = block.next())
        l += block.layout()->numLines();
    return l;
}

void QTextEdit::getSelection(int *paraFrom, int *indexFrom, int *paraTo, int *indexTo) const
{
    if (!paraFrom || !paraTo || !indexFrom || !indexTo)
        return;

    if (!d->cursor.hasSelection()) {
        *paraFrom = *indexFrom = *paraTo = *indexTo = -1;
        return;
    }

    QTextDocumentPrivate *pt = d->doc->docHandle();

    const int selStart = d->cursor.selectionStart();
    const int selEnd = d->cursor.selectionEnd();

    QTextBlock fromBlock(pt, pt->blockMap().findNode(selStart));
    *paraFrom = blockNr(fromBlock);
    *indexFrom = selStart - fromBlock.position();

    QTextBlock toBlock(pt, pt->blockMap().findNode(selEnd));
    *paraTo = blockNr(toBlock);
    *indexTo = selEnd - toBlock.position();
}
*/

/* really have this?
int QTextEdit::lineOfChar(int parag, int index) const
{
    QTextBlock block = d->blockAt(parag);
    if (!block.isValid())
        return -1;

    QTextLine line = block.layout()->findLine(index);
    if (!line.isValid())
        return -1;

    return line.line();
}
*/

/*
int QTextEdit::paragraphAt(const QPoint &pos) const
{
    return blockNr(d->blockAt(pos));
}

int QTextEdit::charAt(const QPoint &pos, int *parag) const
{
    int docPos = 0;
    QTextBlock block = d->blockAt(pos, &docPos);
    if (!block.isValid()) {
        if (parag)
            *parag = -1;
        return -1;
    }

    if (parag)
        *parag = blockNr(block);

    return docPos - block.position();
}

void QTextEdit::setParagraphBackgroundColor(int parag, const QColor &col)
{
    QTextBlock block = blockAt(parag);
    if (!block.isValid())
        return;

    QTextCursor c(block);
    QTextBlockFormat fmt;
    fmt.setBackgroundColor(col);
    c.mergeBlockFormat(fmt);
}
*/
#endif // QT_COMPAT


/*!
    Appends a new paragraph with \a text to the end of the text edit.
*/
void QTextEdit::append(const QString &text)
{
    Qt::TextFormat f = d->textFormat;
    if (f == Qt::AutoText) {
        if (QText::mightBeRichText(text))
            f = Qt::RichText;
        else
            f = Qt::PlainText;
    }

    const bool atBottom = d->contentsY() >= d->contentsHeight() - d->viewport->height();

    QTextCursor cursor(d->doc);
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    if (f == Qt::PlainText) {
        QString txt = text;
        txt.replace('\n', QChar::ParagraphSeparator);
        cursor.insertText(txt);
    } else {
        QTextDocumentFragment frag = QTextDocumentFragment::fromHTML(text);
        cursor.insertFragment(frag);
    }
    cursor.endEditBlock();

    if (atBottom && d->vbar->isVisible())
        d->vbar->setValue(d->vbar->maximum() - d->viewport->height());
}
/*!
    Ensures that the cursor is visible by scrolling the text edit if
    necessary.
*/
void QTextEdit::ensureCursorVisible()
{
    QTextBlock block = d->cursor.block();
    QTextLayout *layout = block.layout();
    QPoint layoutPos = layout->position();
    const int relativePos = d->cursor.position() - block.position();
    QTextLine line = layout->findLine(relativePos);
    if (!line.isValid())
        return;

    const int cursorX = layoutPos.x() + line.cursorToX(relativePos);
    const int cursorY = layoutPos.y() + line.y();
    const int cursorWidth = 1;
    const int cursorHeight = line.ascent() + line.descent();

    const int visibleWidth = d->viewport->width();
    const int visibleHeight = d->viewport->height();

    if (d->hbar->isVisible()) {
        if (cursorX < d->contentsX())
            d->hbar->setValue(cursorX - cursorWidth);
        else if (cursorX + cursorWidth > d->contentsX() + visibleWidth)
            d->hbar->setValue(cursorX + cursorWidth - visibleWidth);
    }

    if (d->vbar->isVisible()) {
        if (cursorY < d->contentsY())
            d->vbar->setValue(cursorY - cursorHeight);
        else if (cursorY + cursorHeight > d->contentsY() + visibleHeight)
            d->vbar->setValue(cursorY + cursorHeight - visibleHeight);
    }
}

#include "moc_qtextedit.cpp"
