/****************************************************************************
**
** Implementation of QLabel widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qlabel.h"
#ifndef QT_NO_LABEL
#include "qpainter.h"
#include "qevent.h"
#include "qdrawutil.h"
#include "qmovie.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qpicture.h"
#include "qapplication.h"
#include "qtextdocument.h"
#include "qstylesheet.h"
#include "qstyle.h"
#include "qframe_p.h"
#include <limits.h>
#include "../text/qtextdocumentlayout_p.h"

class QLabelPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QLabel)
public:
    QLabelPrivate()
        :img(0), pix(0), valid_hints(false), margin(0)
    {}

    void init();
    void clearContents();
    void updateLabel();
    QSize sizeForWidth(int w) const;

    QImage* img; // for scaled contents
    QPixmap* pix; // for scaled contents
    mutable QSize sh;
    mutable QSize msh;
    mutable bool valid_hints;
    int margin;
    QString ltext;
    QPixmap *lpixmap;
#ifndef QT_NO_PICTURE
    QPicture *lpicture;
#endif
#ifndef QT_NO_MOVIE
    QMovie *lmovie;
#endif
    QPointer<QWidget> lbuddy;
    int shortcutId;
    ushort align;
    short extraMargin;
    uint scaledcontents :1;
    QLabel::TextFormat textformat;
#ifndef QT_NO_RICHTEXT
    QTextDocument* doc;
#endif
};

#define d d_func()
#define q q_func()


/*!
    \class QLabel qlabel.h
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
    \header \i Content \i Setting
    \row \i Plain text
         \i Pass a QString to setText().
    \row \i Rich text
         \i Pass a QString that contains rich text to setText().
    \row \i A pixmap
         \i Pass a QPixmap to setPixmap().
    \row \i A movie
         \i Pass a QMovie to setMovie().
    \row \i A number
         \i Pass an \e int or a \e double to setNum(), which converts
            the number to plain text.
    \row \i Nothing
         \i The same as an empty plain text. This is the default. Set
            by clear().
    \endtable

    When the content is changed using any of these functions, any
    previous content is cleared.

    The look of a QLabel can be tuned in several ways. All the
    settings of QFrame are available for specifying a widget frame.
    The positioning of the content within the QLabel widget area can
    be tuned with setAlignment() and setIndent(). For example, this
    code sets up a sunken panel with a two-line text in the bottom
    right corner (both lines being flush with the right side of the
    label):
    \code
    QLabel *label = new QLabel(this);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label->setText("first line\nsecond line");
    label->setAlignment(AlignBottom | AlignRight);
    \endcode

    A QLabel is often used as a label for an interactive widget. For
    this use QLabel provides a useful mechanism for adding an
    mnemonic (see QKeysequence) that will set the keyboard focus to
    the other widget (called the QLabel's "buddy"). For example:
    \code
    QLineEdit* phoneEdit = new QLineEdit(this, "phoneEdit");
    QLabel* phoneLabel = new QLabel(phoneEdit, "&Phone:", this, "phoneLabel");
    \endcode

    In this example, keyboard focus is transferred to the label's
    buddy (the QLineEdit) when the user presses Alt+P. You can
    also use the setBuddy() function to accomplish the same thing.

    <img src=qlabel-m.png> <img src=qlabel-w.png>

    \sa QLineEdit, QTextEdit, QPixmap, QMovie,
    \link guibooks.html#fowler GUI Design Handbook: Label\endlink
*/

#ifndef QT_NO_PICTURE
QPicture *QLabel::picture() const
{
    return d->lpicture;
}
#endif
/*!
    Returns the label's picture or 0 if the label doesn't have a
    picture.
*/



/*!
    Constructs an empty label.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setAlignment(), setFrameStyle(), setIndent()
*/
QLabel::QLabel(QWidget *parent, WFlags f)
    : QFrame(*new QLabelPrivate(), parent, f | WMouseNoMask )
{
    d->init();
}

/*!
    Constructs a label that displays the text, \a text.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setAlignment(), setFrameStyle(), setIndent()
*/
QLabel::QLabel(const QString &text, QWidget *parent, WFlags f)
        : QFrame(*new QLabelPrivate(), parent, f | WMouseNoMask )
{
    d->init();
    setText(text);
}


/*!
    Constructs an empty label.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setAlignment(), setFrameStyle(), setIndent()
*/

QLabel::QLabel(QWidget *parent, const char *name, WFlags f)
    : QFrame(*new QLabelPrivate(), parent, f | WMouseNoMask )
{
    if (name)
        setObjectName(name);
    d->init();
}


/*!
    Constructs a label that displays the text, \a text.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setAlignment(), setFrameStyle(), setIndent()
*/

QLabel::QLabel(const QString &text, QWidget *parent, const char *name,
                WFlags f)
        : QFrame(*new QLabelPrivate(), parent, f | WMouseNoMask )
{
    if (name)
        setObjectName(name);
    d->init();
    setText(text);
}


/*!
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
QLabel::QLabel(QWidget *buddy,  const QString &text,
                QWidget *parent, const char *name, WFlags f)
    : QFrame(*new QLabelPrivate(), parent, f | WMouseNoMask)
{
    if (name)
        setObjectName(name);
    d->init();
    setBuddy(buddy);
    setText(text);
}

/*!
    Destroys the label.
*/

QLabel::~QLabel()
{
    d->clearContents();
}


void QLabelPrivate::init()
{
    lpixmap = 0;
#ifndef QT_NO_MOVIE
    lmovie = 0;
#endif
    shortcutId = 0;
    lpixmap = 0;
#ifndef QT_NO_PICTURE
    lpicture = 0;
#endif
    align = Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs;
    extraMargin = -1;
    scaledcontents = false;
    textformat = Qt::AutoText;
#ifndef QT_NO_RICHTEXT
    doc = 0;
#endif

    q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
}


/*!
    \property QLabel::text
    \brief the label's text

    If no text has been set this will return an empty string. Setting
    the text clears any previous content, unless they are the same.

    The text will be interpreted either as a plain text or as a rich
    text, depending on the text format setting; see setTextFormat().
    The default setting is \c AutoText, i.e. QLabel will try to
    auto-detect the format of the text set.

    If the text is interpreted as a plain text and a buddy has been
    set, the buddy mnemonic key is updated from the new text.

    The label resizes itself if auto-resizing is enabled.

    Note that Qlabel is well-suited to display small rich text
    documents, i.e. those small documents that get their document
    specific settings (font, text color, link color) from the label's
    palette and font properties. For large documents, use QTextEdit
    in read-only mode instead. QTextEdit will flicker less on resize
    and can also provide a scrollbar when necessary.

    \sa text, setTextFormat(), setBuddy(), alignment
*/

void QLabel::setText(const QString &text)
{
    if (d->ltext == text)
        return;
#ifndef QT_NO_RICHTEXT
    bool hadRichtext = d->doc != 0;
#endif
    d->clearContents();
    d->ltext = text;
#ifndef QT_NO_RICHTEXT
    bool useRichText = (d->textformat == RichText ||
      ((d->textformat == AutoText) && QStyleSheet::mightBeRichText(d->ltext)));
#else
    bool useRichText = true;
#endif
#ifndef QT_NO_ACCEL
    // ### Setting accelerators for rich text labels will not work.
    // Eg. <b>&gt;Hello</b> will return ALT+G which is clearly
    // not intended.
    if (!useRichText) {
        releaseShortcut(d->shortcutId);
        d->shortcutId = grabShortcut(QKeySequence::mnemonic(d->ltext));
    }
#endif
#ifndef QT_NO_RICHTEXT
    if (useRichText) {
        if (!hadRichtext)
            d->align |= WordBreak;
        QString t = d->ltext;
        if (d->align & AlignRight)
            t.prepend("<div d->align=\"right\">");
        else if (d->align & AlignHCenter)
            t.prepend("<div d->align=\"center\">");
        if ((d->align & WordBreak) == 0 )
            t.prepend("<nobr>");
        d->doc = new QTextDocument();
        d->doc->setUndoRedoEnabled(false);
        d->doc->setHtml(text);
    }
#endif

    d->updateLabel();
}

QString QLabel::text() const
{
    return d->ltext;
}

/*!
    Clears any label contents. Equivalent to setText("").
*/

void QLabel::clear()
{
    setText(QString::fromLatin1(""));
}

/*!
    \property QLabel::pixmap
    \brief the label's pixmap

    If no pixmap has been set this will return an invalid pixmap.

    Setting the pixmap clears any previous content. The buddy
    accelerator, if any, is disabled.
*/
void QLabel::setPixmap(const QPixmap &pixmap)
{
    if (!d->lpixmap || d->lpixmap->serialNumber() != pixmap.serialNumber()) {
        d->clearContents();
        d->lpixmap = new QPixmap(pixmap);
    }

    if (d->lpixmap->depth() == 1 && !d->lpixmap->mask())
        d->lpixmap->setMask(*((QBitmap *)d->lpixmap));

    d->updateLabel();
}

QPixmap *QLabel::pixmap() const
{
    return d->lpixmap;
}

#ifndef QT_NO_PICTURE
/*!
    Sets the label contents to \a picture. Any previous content is
    cleared.

    The buddy accelerator, if any, is disabled.

    \sa picture(), setBuddy()
*/

void QLabel::setPicture(const QPicture &picture)
{
    d->clearContents();
    d->lpicture = new QPicture(picture);

    d->updateLabel();
}
#endif // QT_NO_PICTURE

/*!
    Sets the label contents to plain text containing the textual
    representation of integer \a num. Any previous content is cleared.
    Does nothing if the integer's string representation is the same as
    the current contents of the label.

    The buddy accelerator, if any, is disabled.

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

    The buddy accelerator, if any, is disabled.

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
    \property QLabel::d->alignment
    \brief the alignment of the label's contents

    The alignment is a bitwise OR of \c Qt::AlignmentFlags and \c
    Qt::TextFlags values. The \c ExpandTabs, \c SingleLine and \c
    ShowPrefix flags apply only if the label contains plain text;
    otherwise they are ignored. The \c DontClip flag is always
    ignored. \c WordBreak applies to both rich text and plain text
    labels. The \c BreakAnywhere flag is not supported in QLabel.

    If the label has a buddy, the \c ShowPrefix flag is forced to
    true.

    The default alignment is \c{AlignAuto | AlignVCenter | ExpandTabs}
    if the label doesn't have a buddy and \c{AlignAuto | AlignVCenter
    | ExpandTabs | ShowPrefix} if the label has a buddy. If the label
    contains rich text, additionally \c WordBreak is turned on.

    \sa Qt::AlignmentFlags, alignment, setBuddy(), text
*/

void QLabel::setAlignment(int alignment)
{
    if (alignment == d->align)
        return;
#ifndef QT_NO_ACCEL
    if (d->lbuddy)
        d->align = alignment | ShowPrefix;
    else
#endif
        d->align = alignment;

#ifndef QT_NO_RICHTEXT
    QString t = d->ltext;
    if (!t.isNull()) {
        d->ltext = QString::null;
        setText(t);
    }
#endif

    d->updateLabel();
}

int QLabel::alignment() const
{
    return d->align;
}

/*!
    \property QLabel::indent
    \brief the label's text indent in pixels

    If a label displays text, the indent applies to the left edge if
    alignment() is \c AlignLeft, to the right edge if alignment() is
    \c AlignRight, to the top edge if alignment() is \c AlignTop, and
    to to the bottom edge if alignment() is \c AlignBottom.

    If indent is negative, or if no indent has been set, the label
    computes the effective indent as follows: If frameWidth() is 0,
    the effective indent becomes 0. If frameWidth() is greater than 0,
    the effective indent becomes half the width of the "x" character
    of the widget's current font().

    \sa alignment, margin, frameWidth(), font()
*/

void QLabel::setIndent(int indent)
{
    d->extraMargin = indent;
    d->updateLabel();
}

int QLabel::indent() const
{
    return d->extraMargin;
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
    return d->margin;
}

void QLabel::setMargin(int margin)
{
    if (d->margin == margin)
        return;
    d->margin = margin;
    d->updateLabel();
}

/*!
    Returns the size that will be used if the width of the label is \a
    w. If \a w is -1, the sizeHint() is returned.
*/

QSize QLabelPrivate::sizeForWidth(int w) const
{
    QSize contentsMargin = q->contentsMarginSize();
    w -= contentsMargin.width();
    QRect br;
    QPixmap *pix = lpixmap;
#ifndef QT_NO_PICTURE
    QPicture *pic = lpicture;
#else
    const int pic = 0;
#endif
#ifndef QT_NO_MOVIE
    QMovie *mov = lmovie;
#else
    const int mov = 0;
#endif
    int hextra = 2 * d->margin;
    int vextra = hextra;
    QFontMetrics fm(q->fontMetrics());
    int xw = fm.width('x');
    if (!mov && !pix && !pic) {
        int m = extraMargin;
        if (m < 0 && hextra) // no indent, but we do have a frame
            m = xw / 2 - d->margin;
        if (m >= 0) {
            int horizAlign = QApplication::horizontalAlignment(QFlag(d->align));
            if ((horizAlign & AlignLeft) || (horizAlign & AlignRight))
                hextra += m;
            if ((d->align & AlignTop) || (d->align & AlignBottom))
                vextra += m;
        }
    }

    if (pix)
        br = pix->rect();
#ifndef QT_NO_PICTURE
    else if (pic)
        br = pic->boundingRect();
#endif
#ifndef QT_NO_MOVIE
    else if (mov)
        br = mov->framePixmap().rect();
#endif
#ifndef QT_NO_RICHTEXT
    else if (doc) {
        QTextDocumentLayout *layout = qt_cast<QTextDocumentLayout *>(doc->documentLayout());
        Q_ASSERT(layout);
        int oldW = layout->pageSize().width();
        if (d->align & WordBreak) {
            if (w < 0)
                layout->adjustSize();
            else
                layout->setPageSize(QSize(w-hextra, INT_MAX));
        }
        br = QRect(0, 0, layout->widthUsed(), layout->totalHeight());
        layout->setPageSize(QSize(oldW, INT_MAX));
    }
#endif
    else {
        bool tryWidth = (w < 0) && (d->align & WordBreak);
        if (tryWidth)
            w = xw * 80;
        else if (w < 0)
            w = 2000;
        w -= hextra;
        QString text = q->text();
        br = fm.boundingRect(0, 0, w ,2000, align, text);
        if (tryWidth && br.height() < 4*fm.lineSpacing() && br.width() > w/2)
            br = fm.boundingRect(0, 0, w/2, 2000, align, text);
        if (tryWidth && br.height() < 2*fm.lineSpacing() && br.width() > w/4)
            br = fm.boundingRect(0, 0, w/4, 2000, align, text);
    }
    int wid = br.width() + hextra;
    int hei = br.height() + vextra;

    return QSize(wid, hei) + contentsMargin;
}


/*!
  \reimp
*/

int QLabel::heightForWidth(int w) const
{
    if (
#ifndef QT_NO_RICHTEXT
        d->doc ||
#endif
        (d->align & WordBreak))
        return d->sizeForWidth(w).height();
    return QWidget::heightForWidth(w);
}



/*!\reimp
*/
QSize QLabel::sizeHint() const
{
    if (!d->valid_hints)
        (void) QLabel::minimumSizeHint();
    return d->sh;
}

/*!
  \reimp
*/

QSize QLabel::minimumSizeHint() const
{
    if (d->valid_hints)
        return d->msh;

    ensurePolished();
    d->valid_hints = true;
    d->sh = d->sizeForWidth(-1);
    QSize sz(-1, -1);

    if (
#ifndef QT_NO_RICHTEXT
         !d->doc &&
#endif
         (d->align & WordBreak) == 0) {
        sz = d->sh;
    } else {
        // think about caching these for performance
        sz.rwidth() = d->sizeForWidth(0).width();
        sz.rheight() = d->sizeForWidth(QWIDGETSIZE_MAX).height();
        if (d->sh.height() < sz.height())
            sz.rheight() = d->sh.height();
    }
    if (sizePolicy().horData() == QSizePolicy::Ignored)
        sz.rwidth() = -1;
    if (sizePolicy().verData() == QSizePolicy::Ignored)
        sz.rheight() = -1;
    d->msh = sz;
    return sz;
}

/*!\reimp
*/
bool QLabel::event(QEvent *e)
{
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->shortcutId) {
            mnemonicSlot();
            return true;
        }
    }
    return QFrame::event(e);
}

/*!\reimp
*/
void QLabel::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    drawFrame(&paint);
    QRect cr = contentsRect();
    cr.addCoords(d->margin, d->margin, -d->margin, -d->margin);

    QPixmap pix;
    if (pixmap())
        pix = *pixmap();
#ifndef QT_NO_PICTURE
    QPicture *pic = picture();
#else
    const int pic = 0;
#endif
#ifndef QT_NO_MOVIE
    QMovie *mov = movie();
#else
    const int mov = 0;
#endif

    if (!mov && !pix && !pic) {
        int m = indent();
        if (m < 0 && frameWidth()) // no indent, but we do have a frame
            m = fontMetrics().width('x') / 2 - d->margin;
        if (m > 0) {
            int hAlign = QApplication::horizontalAlignment(QFlag(d->align));
            if (hAlign & AlignLeft)
                cr.setLeft(cr.left() + m);
            if (hAlign & AlignRight)
                cr.setRight(cr.right() - m);
            if (d->align & AlignTop)
                cr.setTop(cr.top() + m);
            if (d->align & AlignBottom)
                cr.setBottom(cr.bottom() - m);
        }
    }

#ifndef QT_NO_MOVIE
    if (mov) {
        // ### should add movie to qDrawItem
        QRect r = style().itemRect(&paint, cr, d->align, isEnabled(), mov->framePixmap(),
                                    QString::null);
        // ### could resize movie frame at this point
        paint.drawPixmap(r.x(), r.y(), mov->framePixmap());
    }
    else
#endif
#ifndef QT_NO_RICHTEXT
    if (d->doc) {
        QTextDocumentLayout *layout = qt_cast<QTextDocumentLayout *>(d->doc->documentLayout());
        Q_ASSERT(layout);
        layout->setPageSize(QSize(cr.width(), INT_MAX));
        int rh = layout->totalHeight();
        int yo = 0;
        if (d->align & AlignVCenter)
            yo = (cr.height()-rh)/2;
        else if (d->align & AlignBottom)
            yo = cr.height()-rh;
        QAbstractTextDocumentLayout::PaintContext context;
        context.textColorFromPalette = true;
        if (!isEnabled() && style().styleHint(QStyle::SH_EtchDisabledText, this)) {
            context.palette = palette();
            context.palette.setColor(QPalette::Text, context.palette.light());
            QRect r = cr;
            r.moveBy(-cr.x()-1, -cr.y()-yo-1);
            paint.save();
            paint.translate(cr.x()+1, cr.y()+yo+1);
            paint.setClipRect(r);
            layout->draw(&paint, context);
            paint.restore();
        }

        // QSimpleRichText always draws with QPalette::Text as with
        // background mode PaletteBase. QLabel typically has
        // background mode PaletteBackground, so we create a temporary
        // color group with the text color adjusted.
        context.palette = palette();
        if (foregroundRole() != QPalette::Text && isEnabled())
            context.palette.setColor(QPalette::Foreground, context.palette.color(foregroundRole()));
        QRect r = cr;
        r.moveBy(-cr.x(), -cr.y()-yo);
        paint.save();
        paint.translate(cr.x(), cr.y()+yo);
        paint.setClipRect(r);
        layout->draw(&paint, context);
        paint.restore();
    } else
#endif
#ifndef QT_NO_PICTURE
    if (pic) {
        QRect br = pic->boundingRect();
        int rw = br.width();
        int rh = br.height();
        if (d->scaledcontents) {
            paint.save();
            paint.translate(cr.x(), cr.y());
#ifndef QT_NO_TRANSFORMATIONS
            paint.scale((double)cr.width()/rw, (double)cr.height()/rh);
#endif
            paint.drawPicture(-br.x(), -br.y(), *pic);
            paint.restore();
        } else {
            int xo = 0;
            int yo = 0;
            if (d->align & AlignVCenter)
                yo = (cr.height()-rh)/2;
            else if (d->align & AlignBottom)
                yo = cr.height()-rh;
            if (d->align & AlignRight)
                xo = cr.width()-rw;
            else if (d->align & AlignHCenter)
                xo = (cr.width()-rw)/2;
            paint.drawPicture(cr.x()+xo-br.x(), cr.y()+yo-br.y(), *pic);
        }
    } else
#endif
    {
#ifndef QT_NO_IMAGE_SMOOTHSCALE
        if (d->scaledcontents && !pix.isNull()) {
            if (!d->img)
                d->img = new QImage(d->lpixmap->convertToImage());

            if (!d->pix)
                d->pix = new QPixmap;
            if (d->pix->size() != cr.size())
                d->pix->convertFromImage(d->img->smoothScale(cr.width(), cr.height()));
            pix = *d->pix;
        }
#endif
        int alignment = d->align;
        if ((alignment & ShowPrefix) && !style().styleHint(QStyle::SH_UnderlineAccelerator, this))
            alignment |= NoAccel;
        // ordinary text or pixmap label
        style().drawItem(&paint, cr, alignment, palette(), isEnabled(), pix, d->ltext);
    }
}


/*!
    Updates the label, but not the frame.
*/

void QLabelPrivate::updateLabel()
{
    valid_hints = false;
    QSizePolicy policy = q->sizePolicy();
    bool wordBreak = align & WordBreak;
    policy.setHeightForWidth(wordBreak);
    if (policy != q->sizePolicy())
        q->setSizePolicy(policy);
    q->updateGeometry();
    q->update(q->contentsRect());
}


/*!
  \internal

  Internal slot, used to set focus for accelerator labels.
*/
#ifndef QT_NO_ACCEL
void QLabel::mnemonicSlot()
{
    if (!d->lbuddy)
        return;
    QWidget * w = d->lbuddy;
    while (w->focusProxy())
        w = w->focusProxy();
    if (!w->hasFocus() &&
         w->isEnabled() &&
         w->isVisible() &&
         w->focusPolicy() != NoFocus) {
        QFocusEvent::setReason(QFocusEvent::Shortcut);
        w->setFocus();
        QFocusEvent::resetReason();
    }
}
#endif

#ifndef QT_NO_ACCEL
/*!
    Sets this label's buddy to \a buddy.

    When the user presses the accelerator key indicated by this label,
    the keyboard focus is transferred to the label's buddy widget.

    The buddy mechanism is only available for QLabels that contain
    plain text in which one letter is prefixed with an ampersand, \&.
    This letter is set as the accelerator key. The letter is displayed
    underlined, and the '\&' is not displayed (i.e. the \c ShowPrefix
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

    \sa buddy(), setText(), QAccel, setAlignment()
*/

void QLabel::setBuddy(QWidget *buddy)
{
    if (buddy)
        setAlignment(alignment() | ShowPrefix);
    else
        setAlignment(alignment() & ~ShowPrefix);

    d->lbuddy = buddy;

    if (!d->lbuddy)
        return;
}


/*!
    Returns this label's buddy, or 0 if no buddy is currently set.

    \sa setBuddy()
*/

QWidget * QLabel::buddy() const
{
    return d->lbuddy;
}
#endif //QT_NO_ACCEL


#ifndef QT_NO_MOVIE
void QLabel::movieUpdated(const QRect& rect)
{
    QMovie *mov = movie();
    if (mov && !mov->isNull()) {
        QRect r = contentsRect();
        r = style().itemRect(0, r, d->align, isEnabled(), mov->framePixmap(),
                              QString::null);
        r.moveBy(rect.x(), rect.y());
        r.setWidth(qMin(r.width(), rect.width()));
        r.setHeight(qMin(r.height(), rect.height()));
        repaint(r);
    }
}

void QLabel::movieResized(const QSize& size)
{
    d->valid_hints = false;
    movieUpdated(QRect(QPoint(0,0), size));
    updateGeometry();
}

/*!
    Sets the label contents to \a movie. Any previous content is
    cleared.

    The buddy accelerator, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa movie(), setBuddy()
*/

void QLabel::setMovie(const QMovie& movie)
{
    d->clearContents();

    d->lmovie = new QMovie(movie);
        d->lmovie->connectResize(this, SLOT(movieResized(QSize)));
        d->lmovie->connectUpdate(this, SLOT(movieUpdated(QRect)));

    if (!d->lmovie->running())        // Assume that if the movie is running,
        d->updateLabel();        // resize/update signals will come soon enough
}

#endif // QT_NO_MOVIE

/*!
  \internal

  Clears any contents, without updating/repainting the label.
*/

void QLabelPrivate::clearContents()
{
#ifndef QT_NO_RICHTEXT
    delete doc;
    doc = 0;
#endif

    delete d->lpixmap;
    d->lpixmap = 0;
#ifndef QT_NO_PICTURE
    delete lpicture;
    lpicture = 0;
#endif
    delete d->img;
    d->img = 0;
    delete d->pix;
    d->pix = 0;

    d->ltext = QString::null;
    q->releaseShortcut(shortcutId);
    shortcutId = 0;
#ifndef QT_NO_MOVIE
    if (d->lmovie) {
        d->lmovie->disconnectResize(q, SLOT(movieResized(QSize)));
        d->lmovie->disconnectUpdate(q, SLOT(movieUpdated(QRect)));
        delete d->lmovie;
        d->lmovie = 0;
    }
#endif
}


#ifndef QT_NO_MOVIE

/*!
    Returns a pointer to the label's movie, or 0 if no movie has been
    set.

    \sa setMovie()
*/

QMovie* QLabel::movie() const
{
    return d->lmovie;
}

#endif  // QT_NO_MOVIE

/*!
    \property QLabel::backgroundMode
    \brief the label's background mode

    Get this property with backgroundMode().
*/

/*!
    \property QLabel::d->textformat
    \brief the label's text format

    See the \c Qt::D->Textformat enum for an explanation of the possible
    options.

    The default format is \c AutoText.

    \sa text
*/

Qt::TextFormat QLabel::textFormat() const
{
    return d->textformat;
}

void QLabel::setTextFormat(Qt::TextFormat format)
{
    if (format != d->textformat) {
        d->textformat = format;
        QString t = d->ltext;
        if (!t.isNull()) {
            d->ltext = QString::null;
            setText(t);
        }
    }
}

/*!
  \reimp
*/
void QLabel::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::FontChange) {
        if (!d->ltext.isEmpty()) {
#ifndef QT_NO_RICHTEXT
            // #########################
//             if (d->doc)
//                 d->doc->setDefaultFont(font());
#endif
            d->updateLabel();
        }
    }
    QFrame::changeEvent(ev);
}

#ifndef QT_NO_IMAGE_SMOOTHSCALE
/*!
    \property QLabel::scaledContents
    \brief whether the label will scale its contents to fill all
    available space.

    When enabled and the label shows a pixmap, it will scale the
    pixmap to fill the available space.

    This property's default is false.

    \sa setScaledContents()
*/
bool QLabel::hasScaledContents() const
{
    return d->scaledcontents;
}

void QLabel::setScaledContents(bool enable)
{
    if ((bool)d->scaledcontents == enable)
        return;
    d->scaledcontents = enable;
    if (!enable) {
        delete d->img;
        d->img = 0;
        delete d->pix;
        d->pix = 0;
    }
    update(contentsRect());
}

#endif // QT_NO_IMAGE_SMOOTHSCALE

#endif // QT_NO_LABEL
