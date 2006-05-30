/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlabel.h"
#include "qpainter.h"
#include "qevent.h"
#include "qdrawutil.h"
#include "qmovie.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qpicture.h"
#include "qapplication.h"
#include "qtextdocument.h"
#include "qabstractbutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qframe_p.h"
#include <limits.h>
#include "qtextdocumentfragment.h"
#include "qaction.h"
#include "qmenu.h"
#include "qclipboard.h"
#include "../text/qtextdocumentlayout_p.h"
#include "qtextedit_p.h"
#include <qdebug.h>

class QLabelPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QLabel)
public:
    QLabelPrivate() {}

    void init();
    void clearContents();
    void updateLabel();
    QSize sizeForWidth(int w) const;

    mutable QSize sh;
    mutable QSize msh;
    mutable bool valid_hints;
    int margin;
    QString text;
    QPixmap  *pixmap;
    QPixmap *scaledpixmap;
    QImage *cachedimage;
#ifndef QT_NO_PICTURE
    QPicture *picture;
#endif
#ifndef QT_NO_MOVIE
    QMovie *movie;
    void _q_movieUpdated(const QRect&);
    void _q_movieResized(const QSize&);
#endif
#ifndef QT_NO_SHORTCUT
    void updateShortcut();
#endif
    bool isRichText() const {
        return textformat == Qt::RichText
               || (textformat == Qt::AutoText && Qt::mightBeRichText(text));
    }
#ifndef QT_NO_SHORTCUT
    QPointer<QWidget> buddy;
    int shortcutId;
#endif
    ushort align;
    short indent;
    uint scaledcontents :1;
    Qt::TextFormat textformat;
    QTextDocument* doc;

    QRect layoutRect() const;
    QRect documentRect() const;
    QPoint layoutPoint(const QPoint& p) const;
#ifndef QT_NO_MENU
    QMenu *createStandardContextMenu();
#endif
    void _q_copy();
    void _q_copyLink();
    void _q_selectAll();

    QTextCursor selection;
    bool selectable;
    bool hasCustomCursor;
#ifndef QT_NO_CURSOR
    QCursor cursor;
#endif
    QPoint mousePressPos;
    bool mightBeDrag;
    QString linkToCopy; // for copy link
    QString highlightedAnchor;
    QString anchorWhenMousePressed;
};

/*!
    \class QLabel
    \brief The QLabel widget provides a text or image display.

    \ingroup basic
    \ingroup text
    \mainclass

    QLabel is used for displaying text or an image. No user
    interaction functionality is provided. The visual appearance of
    the label can be configured in various ways, and it can be used
    for specifying a focus mnemonic key for another widget.

    A QLabel can contain any of the following content types:

    \table
    \header \o Content \o Setting
    \row \o Plain text
         \o Pass a QString to setText().
    \row \o Rich text
         \o Pass a QString that contains rich text to setText().
    \row \o A pixmap
         \o Pass a QPixmap to setPixmap().
    \row \o A movie
         \o Pass a QMovie to setMovie().
    \row \o A number
         \o Pass an \e int or a \e double to setNum(), which converts
            the number to plain text.
    \row \o Nothing
         \o The same as an empty plain text. This is the default. Set
            by clear().
    \endtable

    When the content is changed using any of these functions, any
    previous content is cleared.

    The look of a QLabel can be tuned in several ways. All the
    settings of QFrame are available for specifying a widget frame.
    The positioning of the content within the QLabel widget area can
    be tuned with setAlignment() and setIndent(). Text content can
    also wrap lines along word bounderies with setWordWrap(). For
    example, this code sets up a sunken panel with a two-line text in
    the bottom right corner (both lines being flush with the right
    side of the label):

    \code
    QLabel *label = new QLabel(this);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label->setText("first line\nsecond line");
    label->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    \endcode

    A QLabel is often used as a label for an interactive widget. For
    this use QLabel provides a useful mechanism for adding an
    mnemonic (see QKeysequence) that will set the keyboard focus to
    the other widget (called the QLabel's "buddy"). For example:
    \code
    QLineEdit* phoneEdit = new QLineEdit(this);
    QLabel* phoneLabel = new QLabel("&Phone:", this);
    phoneLabel->setBuddy(phoneEdit);
    \endcode

    In this example, keyboard focus is transferred to the label's
    buddy (the QLineEdit) when the user presses Alt+P. If the buddy
    was a button (inheriting from QAbstractButton), triggering the
    mnemonic would emulate a button click.

    \table 100%
    \row
    \o \inlineimage macintosh-label.png Screenshot of a Macintosh style label
    \o A label shown in the \l{Macintosh Style Widget Gallery}{Macintosh widget style}.
    \row
    \o \inlineimage plastique-label.png Screenshot of a Plastique style label
    \o A label shown in the \l{Plastique Style Widget Gallery}{Plastique widget style}.
    \row
    \o \inlineimage windowsxp-label.png Screenshot of a Windows XP style label
    \o A label shown in the \l{Windows XP Style Widget Gallery}{Windows XP widget style}.
    \endtable

    \sa QLineEdit, QTextEdit, QPixmap, QMovie,
        {fowler}{GUI Design Handbook: Label}
*/

#ifndef QT_NO_PICTURE
/*!
    Returns the label's picture or 0 if the label doesn't have a
    picture.
*/

const QPicture *QLabel::picture() const
{
    Q_D(const QLabel);
    return d->picture;
}
#endif


/*!
    Constructs an empty label.

    The \a parent and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setAlignment(), setFrameStyle(), setIndent()
*/
QLabel::QLabel(QWidget *parent, Qt::WFlags f)
    : QFrame(*new QLabelPrivate(), parent, f)
{
    Q_D(QLabel);
    d->init();
}

/*!
    Constructs a label that displays the text, \a text.

    The \a parent and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setAlignment(), setFrameStyle(), setIndent()
*/
QLabel::QLabel(const QString &text, QWidget *parent, Qt::WFlags f)
        : QFrame(*new QLabelPrivate(), parent, f)
{
    Q_D(QLabel);
    d->init();
    setText(text);
}


#ifdef QT3_SUPPORT
/*! \obsolete
    Constructs an empty label.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setAlignment(), setFrameStyle(), setIndent()
*/

QLabel::QLabel(QWidget *parent, const char *name, Qt::WFlags f)
    : QFrame(*new QLabelPrivate(), parent, f)
{
    Q_D(QLabel);
    if (name)
        setObjectName(QString::fromAscii(name));
    d->init();
}


/*! \obsolete
    Constructs a label that displays the text, \a text.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setAlignment(), setFrameStyle(), setIndent()
*/

QLabel::QLabel(const QString &text, QWidget *parent, const char *name,
                Qt::WFlags f)
        : QFrame(*new QLabelPrivate(), parent, f)
{
    Q_D(QLabel);
    if (name)
        setObjectName(QString::fromAscii(name));
    d->init();
    setText(text);
}


/*! \obsolete
    Constructs a label that displays the text \a text. The label has a
    buddy widget, \a buddy.

    If the \a text contains an underlined letter (a letter preceded by
    an ampersand, \&), and the text is in plain text format, when the
    user presses Alt+ the underlined letter, focus is passed to the
    buddy widget.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setBuddy(), setAlignment(), setFrameStyle(),
    setIndent()
*/
QLabel::QLabel(QWidget *buddy, const QString &text,
                QWidget *parent, const char *name, Qt::WFlags f)
    : QFrame(*new QLabelPrivate(), parent, f)
{
    Q_D(QLabel);
    if (name)
        setObjectName(QString::fromAscii(name));
    d->init();
#ifndef QT_NO_SHORTCUT
    setBuddy(buddy);
#endif
    setText(text);
}
#endif //QT3_SUPPORT

/*!
    Destroys the label.
*/

QLabel::~QLabel()
{
    Q_D(QLabel);
    d->clearContents();
}

void QLabelPrivate::init()
{
    Q_Q(QLabel);

    valid_hints = false;
    margin = 0;
#ifndef QT_NO_MOVIE
    movie = 0;
#endif
#ifndef QT_NO_SHORTCUT
    shortcutId = -1;
#endif
    pixmap = 0;
    scaledpixmap = 0;
    cachedimage = 0;
#ifndef QT_NO_PICTURE
    picture = 0;
#endif
    align = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs;
    indent = -1;
    scaledcontents = false;
    textformat = Qt::AutoText;
    doc = 0;

    q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    selectable = false;
    hasCustomCursor = false;
    mightBeDrag = false;
}


/*!
    \property QLabel::text
    \brief the label's text

    If no text has been set this will return an empty string. Setting
    the text clears any previous content.

    The text will be interpreted either as plain text or as rich
    text, depending on the text format setting; see setTextFormat().
    The default setting is Qt::AutoText; i.e. QLabel will try to
    auto-detect the format of the text set.

    If the text is interpreted as a plain text and a buddy has been
    set, the buddy mnemonic key is updated from the new text.

    The label resizes itself if auto-resizing is enabled.

    Note that QLabel is well-suited to display small rich text
    documents, such as small documents that get their document
    specific settings (font, text color, link color) from the label's
    palette and font properties. For large documents, use QTextEdit
    in read-only mode instead. QTextEdit can also provide a scrollbar
    when necessary.

    \sa setTextFormat(), setBuddy(), alignment
*/

void QLabel::setText(const QString &text)
{
    Q_D(QLabel);
    if (d->text == text)
        return;
    d->clearContents();
    d->text = text;
    if (!d->doc) {
        d->doc = new QTextDocument(this);
        d->doc->setUndoRedoEnabled(false);
        d->doc->setDefaultFont(font());
    }

    if (d->isRichText()) {
        d->doc->setHtml(text);
        setMouseTracking(true);
    } else {
        d->doc->setPlainText(text);
        setMouseTracking(false);
#ifndef QT_NO_SHORTCUT
        if (d->buddy)
            d->updateShortcut();
#endif
    }

    d->updateLabel();
}

QString QLabel::text() const
{
    Q_D(const QLabel);
    return d->text;
}

/*!
    Clears any label contents.
*/

void QLabel::clear()
{
    Q_D(QLabel);
    d->clearContents();
    d->updateLabel();
}

/*!
    \property QLabel::pixmap
    \brief the label's pixmap

    If no pixmap has been set this will return an invalid pixmap.

    Setting the pixmap clears any previous content. The buddy
    shortcut, if any, is disabled.
*/
void QLabel::setPixmap(const QPixmap &pixmap)
{
    Q_D(QLabel);
    if (!d->pixmap || d->pixmap->serialNumber() != pixmap.serialNumber()) {
        d->clearContents();
        d->pixmap = new QPixmap(pixmap);
    }

    if (d->pixmap->depth() == 1 && !d->pixmap->mask())
        d->pixmap->setMask(*((QBitmap *)d->pixmap));

    d->updateLabel();
}

const QPixmap *QLabel::pixmap() const
{
    Q_D(const QLabel);
    return d->pixmap;
}

#ifndef QT_NO_PICTURE
/*!
    Sets the label contents to \a picture. Any previous content is
    cleared.

    The buddy shortcut, if any, is disabled.

    \sa picture(), setBuddy()
*/

void QLabel::setPicture(const QPicture &picture)
{
    Q_D(QLabel);
    d->clearContents();
    d->picture = new QPicture(picture);

    d->updateLabel();
}
#endif // QT_NO_PICTURE

/*!
    Sets the label contents to plain text containing the textual
    representation of integer \a num. Any previous content is cleared.
    Does nothing if the integer's string representation is the same as
    the current contents of the label.

    The buddy shortcut, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa setText(), QString::setNum(), setBuddy()
*/

void QLabel::setNum(int num)
{
    QString str;
    str.setNum(num);
    setText(str);
}

/*!
    \overload

    Sets the label contents to plain text containing the textual
    representation of double \a num. Any previous content is cleared.
    Does nothing if the double's string representation is the same as
    the current contents of the label.

    The buddy shortcut, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa setText(), QString::setNum(), setBuddy()
*/

void QLabel::setNum(double num)
{
    QString str;
    str.setNum(num);
    setText(str);
}

/*!
    \property QLabel::alignment
    \brief the alignment of the label's contents

    \sa text
*/

void QLabel::setAlignment(Qt::Alignment alignment)
{
    Q_D(QLabel);
    if (alignment == (d->align & (Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask)))
        return;
    d->align = (d->align & ~(Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask))
               | (alignment & (Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask));

    d->updateLabel();
}

#ifdef QT3_SUPPORT
/*!
    Use setAlignment(Qt::Alignment) instead.

    If \a alignment specifies text flags as well, use setTextFormat()
    to set those.
*/
void QLabel::setAlignment(int alignment)
{
    Q_D(QLabel);
    d->align = alignment & ~(Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask|Qt::TextWordWrap);
    setAlignment(Qt::Alignment(QFlag(alignment)));
}
#endif

Qt::Alignment QLabel::alignment() const
{
    Q_D(const QLabel);
    return QFlag(d->align & (Qt::AlignVertical_Mask|Qt::AlignHorizontal_Mask));
}


/*!
    \property QLabel::wordWrap
    \brief the label's word-wrapping policy

    If this property is true then label text is wrapped where
    necessary at word-breaks; otherwise it is not wrapped at all.
*/
void QLabel::setWordWrap(bool on)
{
    Q_D(QLabel);
    if (on)
        d->align |= Qt::TextWordWrap;
    else
        d->align &= ~Qt::TextWordWrap;

    d->updateLabel();
}

bool QLabel::wordWrap() const
{
    Q_D(const QLabel);
    return d->align & Qt::TextWordWrap;
}

/*!
    \property QLabel::indent
    \brief the label's text indent in pixels

    If a label displays text, the indent applies to the left edge if
    alignment() is Qt::AlignLeft, to the right edge if alignment() is
    Qt::AlignRight, to the top edge if alignment() is Qt::AlignTop, and
    to to the bottom edge if alignment() is Qt::AlignBottom.

    If indent is negative, or if no indent has been set, the label
    computes the effective indent as follows: If frameWidth() is 0,
    the effective indent becomes 0. If frameWidth() is greater than 0,
    the effective indent becomes half the width of the "x" character
    of the widget's current font().

    \sa alignment, margin, frameWidth(), font()
*/

void QLabel::setIndent(int indent)
{
    Q_D(QLabel);
    d->indent = indent;
    d->updateLabel();
}

int QLabel::indent() const
{
    Q_D(const QLabel);
    return d->indent;
}


/*!
    \property QLabel::margin
    \brief the width of the margin

    The margin is the distance between the innermost pixel of the
    frame and the outermost pixel of contents.

    The default margin is 0.

    \sa indent
*/
int QLabel::margin() const
{
    Q_D(const QLabel);
    return d->margin;
}

void QLabel::setMargin(int margin)
{
    Q_D(QLabel);
    if (d->margin == margin)
        return;
    d->margin = margin;
    d->updateLabel();
}


/*!
    \property QLabel::textSelectable
    \brief the text is selectable using the mouse

    If the label displays text, this property enables or disables the selection of the text
    using the mouse.

    By default, the text is not selectable.
*/
bool QLabel::isTextSelectable() const
{
    Q_D(const QLabel);
    return d->selectable;
}

void QLabel::setTextSelectable(bool selectable)
{
    Q_D(QLabel);
    d->selectable = selectable;
}

/*!
    Returns the size that will be used if the width of the label is \a
    w. If \a w is -1, the sizeHint() is returned. If \a w is 0 minimumSizeHint() is returned
*/
QSize QLabelPrivate::sizeForWidth(int w) const
{
    Q_Q(const QLabel);
    QSize contentsMargin(leftmargin + rightmargin, topmargin + bottommargin);

    QRect br;

    int hextra = 2 * margin;
    int vextra = hextra;

    if (pixmap)
        br = pixmap->rect();
#ifndef QT_NO_PICTURE
    else if (picture)
        br = picture->boundingRect();
#endif
#ifndef QT_NO_MOVIE
    else if (movie)
        br = movie->currentPixmap().rect();
#endif
    else if (doc) {
        // Add indentation
        QFontMetrics fm(q->fontMetrics());
        int m = indent;

        if (m < 0 && q->frameWidth()) // no indent, but we do have a frame
            m = fm.width('x') - margin*2;
        if (m > 0) {
            int align = QStyle::visualAlignment(q->layoutDirection(), QFlag(this->align));
            if ((align & Qt::AlignLeft) || (align & Qt::AlignRight))
                hextra += m;
            if ((align & Qt::AlignTop) || (align & Qt::AlignBottom))
                vextra += m;
        }

        // Calculate the length of document if w is the width
        QAbstractTextDocumentLayout *layout = doc->documentLayout();
        if (align & Qt::TextWordWrap) {
            if (w >= 0) {
                w = qMax(w-hextra-contentsMargin.width(), 0); // strip margin and indent
                doc->setPageSize(QSize(w, INT_MAX));
            } else {
                QTextDocumentLayout *l = qobject_cast<QTextDocumentLayout *>(doc->documentLayout());
                Q_ASSERT(l != 0);
                l->adjustSize();
            }
        } else {
            doc->setPageSize(QSize(0, INT_MAX));
        }
        br = QRect(QPoint(0, 0), layout->documentSize().toSize());
    }

    const QSize contentsSize(br.width() + hextra, br.height() + vextra);
    return contentsSize + contentsMargin;
}


/*!
  \reimp
*/

int QLabel::heightForWidth(int w) const
{
    Q_D(const QLabel);
    if (d->doc)
        return d->sizeForWidth(w).height();
    return QWidget::heightForWidth(w);
}



/*!\reimp
*/
QSize QLabel::sizeHint() const
{
    Q_D(const QLabel);
    if (!d->valid_hints)
        (void) QLabel::minimumSizeHint();
    return d->sh;
}

/*!
  \reimp
*/
QSize QLabel::minimumSizeHint() const
{
    Q_D(const QLabel);
    if (d->valid_hints)
        return d->msh;

    ensurePolished();
    d->valid_hints = true;
    d->sh = d->sizeForWidth(-1); // wrap ? golden ratio : min doc size
    QSize msh(-1, -1);

    if (!d->doc) {
        msh = d->sh;
    } else {
        msh.rheight() = d->sizeForWidth(QWIDGETSIZE_MAX).height(); // height for one line
        msh.rwidth() = d->sizeForWidth(0).width(); // wrap ? size of biggest word : min doc size
        if (d->sh.height() < msh.height())
            msh.rheight() = d->sh.height();
    }
    if (sizePolicy().horizontalPolicy() == QSizePolicy::Ignored)
        msh.rwidth() = -1;
    if (sizePolicy().verticalPolicy() == QSizePolicy::Ignored)
        msh.rheight() = -1;
    d->msh = msh;
    return msh;
}

/*!\reimp
*/
void QLabel::mousePressEvent(QMouseEvent *ev)
{
    Q_D(QLabel);
    if (!d->doc)
        return;

    QPoint p = d->layoutPoint(ev->pos());
    d->anchorWhenMousePressed = d->doc->documentLayout()->anchorAt(p);

    if (!d->selectable || !(ev->button() & Qt::LeftButton))
        return;

    d->mousePressPos = ev->pos();
    int docPos = d->doc->documentLayout()->hitTest(p, Qt::FuzzyHit);
    // check if user clicked on a selection. dont clear selection since it might be a drag
    d->mightBeDrag = d->selection.hasSelection() && (d->selection.selectionStart() <= docPos)
                     && (d->selection.selectionEnd() >= docPos);

    if (!d->mightBeDrag) {
        d->selection = QTextCursor(); // clear the selection since its not a drag
        update(contentsRect());
    }
}

/*!\reimp
*/
void QLabel::mouseMoveEvent(QMouseEvent *ev)
{
    Q_D(QLabel);
    if (!d->doc)
        return;

    QPoint p = d->layoutPoint(mapFromGlobal(ev->globalPos()));
    // check for links below mouse and change cursor
    QString anchor = d->doc->documentLayout()->anchorAt(p);
    if (anchor != d->highlightedAnchor) {
        if (anchor.isEmpty()) { // restore cursor
#ifndef QT_NO_CURSOR
            setCursor(d->hasCustomCursor ? d->cursor : Qt::ArrowCursor);
#endif
            emit highlighted(QString());
        } else {
#ifndef QT_NO_CURSOR
            d->hasCustomCursor = testAttribute(Qt::WA_SetCursor);
            if (d->hasCustomCursor)
                d->cursor = cursor();
            setCursor(Qt::PointingHandCursor);
#endif
            emit highlighted(anchor);
        }
        d->highlightedAnchor = anchor; // save it so we dont keep emitting highlighted
    }

    if (!d->selectable || !(ev->buttons() & Qt::LeftButton))
        return;

    if (d->mightBeDrag) {
        if ((ev->pos() - d->mousePressPos).manhattanLength() < QApplication::startDragDistance())
            return;
        QDrag *drag = new QDrag(this);
        const QTextDocumentFragment fragment(d->selection.selection());
        QTextEditMimeData *md = new QTextEditMimeData(fragment);
        drag->setMimeData(md);
        drag->start();
        d->mightBeDrag = false;
    } else {
        if (!d->selection.hasSelection()) {
            // Lazy marking of the start of selection
            QPoint docPos = d->layoutPoint(d->mousePressPos);
            int startPos = d->doc->documentLayout()->hitTest(docPos, Qt::FuzzyHit);
            d->selection = QTextCursor(d->doc);
            d->selection.setPosition(startPos);
        }
        int end = d->doc->documentLayout()->hitTest(p, Qt::FuzzyHit);
        d->selection.setPosition(end, QTextCursor::KeepAnchor);
        update(contentsRect());
    }
}

/*!\reimp
*/
void QLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_D(QLabel);
    if (!d->doc || !(ev->button() & Qt::LeftButton))
        return;

    if (d->mightBeDrag) { // a drag was detected but never started
        d->mightBeDrag = false;
        d->selection = QTextCursor();
        update(contentsRect());
    }

    if (d->selection.hasSelection()) { // user completed a selection
#ifndef QT_NO_CLIPBOARD
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard->supportsSelection()) {
            const QTextDocumentFragment fragment(d->selection.selection());
            QTextEditMimeData *md = new QTextEditMimeData(fragment);
            clipboard->setMimeData(md, QClipboard::Selection);
        }
#endif
        return;
    }

    // check for link clicks. ensure that the mouse press and release happenned on the same anchor
    QPoint p = d->layoutPoint(ev->pos());
    QString anchor = d->doc->documentLayout()->anchorAt(p);
    if (!anchor.isEmpty() && anchor == d->anchorWhenMousePressed)
        emit anchorClicked(anchor);
}

/*!\reimp
*/
void QLabel::contextMenuEvent(QContextMenuEvent *ev)
{
    Q_D(QLabel);
    if (!d->selectable) {
        QFrame::contextMenuEvent(ev);
    }
#ifndef QT_NO_MENU
    else {
        QMenu *menu = d->createStandardContextMenu();
        menu->exec(ev->globalPos());
        delete menu;
    }
#endif
}

/*!\reimp
*/
bool QLabel::event(QEvent *e)
{
#ifndef QT_NO_SHORTCUT
    Q_D(QLabel);
    QEvent::Type type = e->type();
    if (type == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->shortcutId) {
            QWidget * w = d->buddy;
            QAbstractButton *button = qobject_cast<QAbstractButton *>(w);
            if (w->focusPolicy() != Qt::NoFocus)
                w->setFocus(Qt::ShortcutFocusReason);
            if (button && !se->isAmbiguous())
                button->animateClick();
            else
                window()->setAttribute(Qt::WA_KeyboardFocusChange);
            return true;
        }
    }
#endif
    return QFrame::event(e);
}

/*!\reimp
*/
void QLabel::paintEvent(QPaintEvent *)
{
    Q_D(QLabel);
    QStyle *style = QWidget::style();
    QPainter painter(this);
    drawFrame(&painter);
    QRect cr = contentsRect();
    cr.adjust(d->margin, d->margin, -d->margin, -d->margin);
    int align = QStyle::visualAlignment(layoutDirection(), QFlag(d->align));

#ifndef QT_NO_MOVIE
    if (d->movie) {
        if (d->scaledcontents)
            style->drawItemPixmap(&painter, cr, align, d->movie->currentPixmap().scaled(cr.size()));
        else
            style->drawItemPixmap(&painter, cr, align, d->movie->currentPixmap());
    }
    else
#endif
    if (d->doc) {
        QAbstractTextDocumentLayout *layout = d->doc->documentLayout();
        QRect dr = d->documentRect();
        d->doc->setPageSize(QSizeF(dr.size().width(), INT_MAX));
        QRect lr = d->layoutRect();

        QAbstractTextDocumentLayout::PaintContext context;
        QStyleOption opt(0);
        opt.init(this);

        if (!isEnabled() && style->styleHint(QStyle::SH_EtchDisabledText, &opt, this)) {
            context.palette = palette();
            context.palette.setColor(QPalette::Text, context.palette.light().color());
            painter.save();
            painter.translate(lr.x() + 1, lr.y() + 1);
            painter.setClipRect(lr.translated(-lr.x() - 1, -lr.y() - 1));
            layout->draw(&painter, context);
            painter.restore();
        }

        // Adjust the palette
        context.palette = palette();
        if (foregroundRole() != QPalette::Text && isEnabled())
            context.palette.setColor(QPalette::Text, context.palette.color(foregroundRole()));

        painter.save();
        painter.translate(lr.topLeft());
        painter.setClipRect(lr.translated(-lr.x(), -lr.y()));
        if (d->selection.hasSelection()) {
            QAbstractTextDocumentLayout::Selection s;
            s.cursor = d->selection;
            s.format.setBackground(palette().brush(QPalette::Highlight));
            s.format.setForeground(palette().brush(QPalette::HighlightedText));
            context.selections.append(s);
        }
        layout->draw(&painter, context);
        painter.restore();
    } else
#ifndef QT_NO_PICTURE
    if (d->picture) {
        QRect br = d->picture->boundingRect();
        int rw = br.width();
        int rh = br.height();
        if (d->scaledcontents) {
            painter.save();
            painter.translate(cr.x(), cr.y());
            painter.scale((double)cr.width()/rw, (double)cr.height()/rh);
            painter.drawPicture(-br.x(), -br.y(), *d->picture);
            painter.restore();
        } else {
            int xo = 0;
            int yo = 0;
            if (align & Qt::AlignVCenter)
                yo = (cr.height()-rh)/2;
            else if (align & Qt::AlignBottom)
                yo = cr.height()-rh;
            if (align & Qt::AlignRight)
                xo = cr.width()-rw;
            else if (align & Qt::AlignHCenter)
                xo = (cr.width()-rw)/2;
            painter.drawPicture(cr.x()+xo-br.x(), cr.y()+yo-br.y(), *d->picture);
        }
    } else
#endif
    if (d->pixmap && !d->pixmap->isNull()) {
        QPixmap pix;
        if (d->scaledcontents) {
            if (!d->scaledpixmap || d->scaledpixmap->size() != cr.size()) {
                if (!d->cachedimage)
                    d->cachedimage = new QImage(d->pixmap->toImage());
                delete d->scaledpixmap;
                d->scaledpixmap = new QPixmap(QPixmap::fromImage(d->cachedimage->scaled(cr.size())));
            }
            pix = *d->scaledpixmap;
        } else
            pix = *d->pixmap;
        QStyleOption opt;
        opt.initFrom(this);
        if (!isEnabled())
            pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
        style->drawItemPixmap(&painter, cr, align, pix);
    }
}


/*!
    Updates the label, but not the frame.
*/

void QLabelPrivate::updateLabel()
{
    Q_Q(QLabel);
    valid_hints = false;

    if (doc) {
        QSizePolicy policy = q->sizePolicy();
        const bool wrap = align & Qt::TextWordWrap;;
        policy.setHeightForWidth(wrap);
        if (policy != q->sizePolicy())
            q->setSizePolicy(policy);

        // Select all and apply the block format
        QTextCursor c(doc);
        c.movePosition(QTextCursor::Start);
        c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        QTextBlockFormat f;
        f.setAlignment(q->alignment());
        f.setLayoutDirection(q->layoutDirection());
        f.setNonBreakableLines(!wrap);
        c.mergeBlockFormat(f);

        if (wrap) {
            // ensure that we break at words and not just about anywhere
            QTextDocumentLayout *l = qobject_cast<QTextDocumentLayout *>(doc->documentLayout());
            Q_ASSERT(l != 0);
            l->setWordWrapMode(QTextOption::WordWrap);
        }

        QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
        fmt.setMargin(0);
        doc->rootFrame()->setFrameFormat(fmt);
    }
    q->updateGeometry();
    q->update(q->contentsRect());
}

#ifndef QT_NO_SHORTCUT
/*!
    Sets this label's buddy to \a buddy.

    When the user presses the shortcut key indicated by this label,
    the keyboard focus is transferred to the label's buddy widget.

    The buddy mechanism is only available for QLabels that contain
    plain text in which one letter is prefixed with an ampersand, \&.
    This letter is set as the shortcut key. The letter is displayed
    underlined, and the '\&' is not displayed (i.e. the Qt::TextShowMnemonic
    alignment flag is turned on; see setAlignment()).

    In a dialog, you might create two data entry widgets and a label
    for each, and set up the geometry layout so each label is just to
    the left of its data entry widget (its "buddy"), for example:
    \code
    QLineEdit *nameEd  = new QLineEdit(this);
    QLabel    *nameLb  = new QLabel("&Name:", this);
    nameLb->setBuddy(nameEd);
    QLineEdit *phoneEd = new QLineEdit(this);
    QLabel    *phoneLb = new QLabel("&Phone:", this);
    phoneLb->setBuddy(phoneEd);
    // (layout setup not shown)
    \endcode

    With the code above, the focus jumps to the Name field when the
    user presses Alt+N, and to the Phone field when the user presses
    Alt+P.

    To unset a previously set buddy, call this function with \a buddy
    set to 0.

    \sa buddy(), setText(), QShortcut, setAlignment()
*/

void QLabel::setBuddy(QWidget *buddy)
{
    Q_D(QLabel);
    d->buddy = buddy;
    if (d->doc && !d->isRichText()) {
        releaseShortcut(d->shortcutId);
        d->shortcutId = -1;
        d->doc->setPlainText(d->text); // restore the old text
        if (buddy)
            d->updateShortcut(); // grab new shortcut
        d->updateLabel();
    }
}


/*!
    Returns this label's buddy, or 0 if no buddy is currently set.

    \sa setBuddy()
*/

QWidget * QLabel::buddy() const
{
    Q_D(const QLabel);
    return d->buddy;
}

void QLabelPrivate::updateShortcut()
{
    Q_Q(QLabel);
    Q_ASSERT(shortcutId == -1);
    Q_ASSERT(!isRichText());

    // Underline the first character that follows an ampersand
    QTextCursor cursor = doc->find(QLatin1String("&"));
    if (cursor.isNull())
        return;
    shortcutId = q->grabShortcut(QKeySequence::mnemonic(text));
    cursor.deleteChar(); // remove the ampersand
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QTextCharFormat fmt;
    fmt.setFontUnderline(true);
    cursor.mergeCharFormat(fmt);
}

#endif // QT_NO_SHORTCUT

#ifndef QT_NO_MOVIE
void QLabelPrivate::_q_movieUpdated(const QRect& rect)
{
    Q_Q(QLabel);
    if (movie && movie->isValid()) {
        QRect r;
        if (scaledcontents) {
            QRect cr = q->contentsRect();
            QRect pixmapRect(cr.topLeft(), movie->currentPixmap().size());
            if (pixmapRect.isEmpty())
                return;
            r.setRect(cr.left(), cr.top(),
                      (rect.width() * cr.width()) / pixmapRect.width(),
                      (rect.height() * cr.height()) / pixmapRect.height());
        } else {
            r = q->style()->itemPixmapRect(q->contentsRect(), align, movie->currentPixmap());
            r.translate(rect.x(), rect.y());
            r.setWidth(qMin(r.width(), rect.width()));
            r.setHeight(qMin(r.height(), rect.height()));
        }
        q->update(r);
    }
}

void QLabelPrivate::_q_movieResized(const QSize& size)
{
    Q_Q(QLabel);
    valid_hints = false;
    _q_movieUpdated(QRect(QPoint(0,0), size));
    q->updateGeometry();
}

/*!
    Sets the label contents to \a movie. Any previous content is
    cleared.

    The buddy shortcut, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa movie(), setBuddy()
*/

void QLabel::setMovie(QMovie *movie)
{
    Q_D(QLabel);
    d->clearContents();

    d->movie = movie;
    connect(movie, SIGNAL(resized(QSize)), this, SLOT(_q_movieResized(QSize)));
    connect(movie, SIGNAL(updated(QRect)), this, SLOT(_q_movieUpdated(QRect)));

    // Assume that if the movie is running,
    // resize/update signals will come soon enough
    if (movie->state() != QMovie::Running)
        d->updateLabel();
}

#endif // QT_NO_MOVIE

/*!
  \internal

  Clears any contents, without updating/repainting the label.
*/

void QLabelPrivate::clearContents()
{
    delete doc;
    doc = 0;

#ifndef QT_NO_PICTURE
    delete picture;
    picture = 0;
#endif
    delete scaledpixmap;
    scaledpixmap = 0;
    delete cachedimage;
    cachedimage = 0;
    delete pixmap;
    pixmap = 0;

    text.clear();
#ifndef QT_NO_SHORTCUT
    Q_Q(QLabel);
    q->releaseShortcut(shortcutId);
    shortcutId = -1;
#endif
#ifndef QT_NO_MOVIE
    movie = 0;
#endif
}


#ifndef QT_NO_MOVIE

/*!
    Returns a pointer to the label's movie, or 0 if no movie has been
    set.

    \sa setMovie()
*/

QMovie *QLabel::movie() const
{
    Q_D(const QLabel);
    return d->movie;
}

#endif  // QT_NO_MOVIE

/*!
    \property QLabel::textFormat
    \brief the label's text format

    See the Qt::TextFormat enum for an explanation of the possible
    options.

    The default format is Qt::AutoText.

    \sa text()
*/

Qt::TextFormat QLabel::textFormat() const
{
    Q_D(const QLabel);
    return d->textformat;
}

void QLabel::setTextFormat(Qt::TextFormat format)
{
    Q_D(QLabel);
    if (format != d->textformat) {
        d->textformat = format;
        QString t = d->text;
        if (!t.isNull()) {
            d->text.clear();
            setText(t);
        }
    }
}

/*!
  \reimp
*/
void QLabel::changeEvent(QEvent *ev)
{
    Q_D(QLabel);
    if(ev->type() == QEvent::FontChange) {
        if (d->doc) {
            d->doc->setDefaultFont(font());
            d->updateLabel();
        }
    }
    QFrame::changeEvent(ev);
}

/*!
    \property QLabel::scaledContents
    \brief whether the label will scale its contents to fill all
    available space.

    When enabled and the label shows a pixmap, it will scale the
    pixmap to fill the available space.

    This property's default is false.
*/
bool QLabel::hasScaledContents() const
{
    Q_D(const QLabel);
    return d->scaledcontents;
}

void QLabel::setScaledContents(bool enable)
{
    Q_D(QLabel);
    if ((bool)d->scaledcontents == enable)
        return;
    d->scaledcontents = enable;
    if (!enable) {
        delete d->scaledpixmap;
        d->scaledpixmap = 0;
        delete d->cachedimage;
        d->cachedimage = 0;
    }
    update(contentsRect());
}


/*!
    \fn void QLabel::setAlignment(Qt::AlignmentFlag flag)
    \internal

    Without this function, a call to e.g. setAlignment(Qt::AlignTop)
    results in the \c QT3_SUPPORT function setAlignment(int) being called,
    rather than setAlignment(Qt::Alignment).
*/

// Returns the rect that is available for us to draw the document
QRect QLabelPrivate::documentRect() const
{
    Q_Q(const QLabel);
    Q_ASSERT_X(doc, "documentRect", "document rect called when doc not set!");
    QRect cr = q->contentsRect();
    cr.adjust(margin, margin, -margin, -margin);
    const int align = QStyle::visualAlignment(q->layoutDirection(), QFlag(this->align));
    int m = indent;
    if (m < 0 && q->frameWidth()) // no indent, but we do have a frame
        m = q->fontMetrics().width('x') / 2 - margin;
    if (m > 0) {
        if (align & Qt::AlignLeft)
            cr.setLeft(cr.left() + m);
        if (align & Qt::AlignRight)
            cr.setRight(cr.right() - m);
        if (align & Qt::AlignTop)
            cr.setTop(cr.top() + m);
        if (align & Qt::AlignBottom)
            cr.setBottom(cr.bottom() - m);
    }
    return cr;
}

// Return the layout rect - this is the rect that is given to the layout painting code
// This may be different from the document rect since vertical alignment is not
// done by the text layout code
QRect QLabelPrivate::layoutRect() const
{
    QRect cr = documentRect();
    // Caculate y position manually
    int rh = qRound(doc->documentLayout()->documentSize().height());
    int yo = 0;
    if (align & Qt::AlignVCenter)
        yo = qMax((cr.height()-rh)/2, 0);
    else if (align & Qt::AlignBottom)
        yo = qMax(cr.height()-rh, 0);
    return QRect(cr.x(), yo + cr.y(), cr.width(), cr.height());
}

// Returns the point in the document rect adjusted with p
QPoint QLabelPrivate::layoutPoint(const QPoint& p) const
{
    QRect lr = layoutRect();
    return p - lr.topLeft();
}

#ifndef QT_NO_MENU
QMenu *QLabelPrivate::createStandardContextMenu()
{
    Q_Q(QLabel);
    QMenu *popup = new QMenu(q);
    QAction *a = popup->addAction(QLabel::tr("&Copy"), q, SLOT(_q_copy()));
    a->setEnabled(selection.hasSelection());

    if (isRichText()) {
        QPoint p = layoutPoint(q->mapFromGlobal(QCursor::pos()));
        linkToCopy = doc->documentLayout()->anchorAt(p);
        if (!linkToCopy.isEmpty())
            popup->addAction(QLabel::tr("Copy &Link Location"), q, SLOT(_q_copyLink()));
    }
    popup->addAction(QLabel::tr("Select &All"), q, SLOT(_q_selectAll()));

    return popup;
}
#endif

void QLabelPrivate::_q_copy()
{
    const QTextDocumentFragment fragment(selection.selection());
    QTextEditMimeData *md = new QTextEditMimeData(fragment);
    QApplication::clipboard()->setMimeData(md);
}

void QLabelPrivate::_q_copyLink()
{
    QMimeData *md = new QMimeData;
    md->setText(linkToCopy);
    QApplication::clipboard()->setMimeData(md);
}

void QLabelPrivate::_q_selectAll()
{
    Q_Q(QLabel);
    if (!doc || !selectable)
        return;
    selection = QTextCursor(doc);
    selection.select(QTextCursor::Document);
    q->update(q->contentsRect());
}


/*!  \fn void QLabel::highlighted(const QString &link)
     \overload

     Convenience signal that allows connecting to a slot
     that takes just a QString, like for example QStatusBar's
     message().
*/


/*!
    \fn void QLabel::anchorClicked(const QString &link)

    This signal is emitted when the user clicks an anchor. The
    URL referred to by the anchor is passed in \a link.
*/

#include "moc_qlabel.cpp"
