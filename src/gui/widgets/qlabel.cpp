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
#include "qabstractbutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qframe_p.h"
#include <limits.h>
#include "../text/qtextdocumentlayout_p.h"

class QLabelPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QLabel)
public:
    QLabelPrivate()
        : img(0), pix(0), valid_hints(false), margin(0)
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
    void movieUpdated(const QRect&);
    void movieResized(const QSize&);
#endif
    QPointer<QWidget> lbuddy;
    int shortcutId;
    ushort align;
    short extraMargin;
    uint scaledcontents :1;
    Qt::TextFormat textformat;
#ifndef QT_NO_RICHTEXT
    QTextDocument* doc;
#endif
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

    \inlineimage macintosh-label.png Screenshot in Macintosh style
    \inlineimage windows-label.png Screenshot in Windows style

    \sa QLineEdit, QTextEdit, QPixmap, QMovie,
    \link guibooks.html#fowler GUI Design Handbook: Label\endlink
*/

#ifndef QT_NO_PICTURE
/*!
    Returns the label's picture or 0 if the label doesn't have a
    picture.
*/

const QPicture *QLabel::picture() const
{
    Q_D(const QLabel);
    return d->lpicture;
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
        setObjectName(name);
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
        setObjectName(name);
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
        setObjectName(name);
    d->init();
    setBuddy(buddy);
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
    lpixmap = 0;
#ifndef QT_NO_MOVIE
    lmovie = 0;
#endif
    shortcutId = 0;
    lpixmap = 0;
#ifndef QT_NO_PICTURE
    lpicture = 0;
#endif
    align = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs;
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
    The default setting is \c Qt::AutoText, i.e. QLabel will try to
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
    Q_D(QLabel);
    if (d->ltext == text)
        return;
    d->clearContents();
    d->ltext = text;
#ifndef QT_NO_RICHTEXT
    if (d->textformat == Qt::RichText
        || ((d->textformat == Qt::AutoText) && Qt::mightBeRichText(d->ltext))) {
        if (!d->doc)
            d->doc = new QTextDocument();
        d->doc->setUndoRedoEnabled(false);
        d->doc->setDefaultFont(font());
        d->doc->setHtml(d->ltext);
    }
#endif

    d->updateLabel();
}

QString QLabel::text() const
{
    Q_D(const QLabel);
    return d->ltext;
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
    if (!d->lpixmap || d->lpixmap->serialNumber() != pixmap.serialNumber()) {
        d->clearContents();
        d->lpixmap = new QPixmap(pixmap);
    }

    if (d->lpixmap->depth() == 1 && !d->lpixmap->mask())
        d->lpixmap->setMask(*((QBitmap *)d->lpixmap));

    d->updateLabel();
}

const QPixmap *QLabel::pixmap() const
{
    Q_D(const QLabel);
    return d->lpixmap;
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
    d->lpicture = new QPicture(picture);

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
#ifndef QT_NO_ACCEL
    if (d->lbuddy)
        d->align |= Qt::TextShowMnemonic;
#endif
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
    alignment() is \c Qt::AlignLeft, to the right edge if alignment() is
    \c Qt::AlignRight, to the top edge if alignment() is \c Qt::AlignTop, and
    to to the bottom edge if alignment() is \c Qt::AlignBottom.

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
    d->extraMargin = indent;
    d->updateLabel();
}

int QLabel::indent() const
{
    Q_D(const QLabel);
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
    Returns the size that will be used if the width of the label is \a
    w. If \a w is -1, the sizeHint() is returned.
*/
QSize QLabelPrivate::sizeForWidth(int w) const
{
    Q_Q(const QLabel);
    QSize contentsMargin(leftmargin + rightmargin, topmargin + bottommargin);
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
    int hextra = 2 * margin;
    int vextra = hextra;
    QFontMetrics fm(q->fontMetrics());
    int xw = fm.width('x');
    if (!mov && !pix && !pic) {
        int m = extraMargin;
        if (m < 0 && frameWidth) // no indent, but we do have a frame
            m = (xw / 2 - margin) * 2;
        if (m >= 0) {
            int align = QStyle::visualAlignment(q->layoutDirection(), QFlag(this->align));
            if ((align & Qt::AlignLeft) || (align & Qt::AlignRight))
                hextra += m;
            if ((align & Qt::AlignTop) || (align & Qt::AlignBottom))
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
        br = mov->currentPixmap().rect();
#endif
#ifndef QT_NO_RICHTEXT
    else if (doc) {
        QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(doc->documentLayout());
        Q_ASSERT(layout);
        if (align & Qt::TextWordWrap) {
            if (w > 0)
                doc->setPageSize(QSize(w-hextra, INT_MAX));
            else
                layout->adjustSize();
        } else {
            doc->setPageSize(QSize(0, 100000));
        }
        br = QRect(QPoint(0, 0), layout->documentSize().toSize());
    }
#endif
    else {
        bool tryWidth = (w < 0) && (align & Qt::TextWordWrap);
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
    Q_D(const QLabel);
    if (
#ifndef QT_NO_RICHTEXT
        d->doc ||
#endif
        (d->align & Qt::TextWordWrap))
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
    d->sh = d->sizeForWidth(-1);
    QSize sz(-1, -1);

    if (
#ifndef QT_NO_RICHTEXT
         !d->doc &&
#endif
         (d->align & Qt::TextWordWrap) == 0) {
        sz = d->sh;
    } else {
        // think about caching these for performance
        sz.rwidth() = d->sizeForWidth(0).width();
        sz.rheight() = d->sizeForWidth(QWIDGETSIZE_MAX).height();
        if (d->sh.height() < sz.height())
            sz.rheight() = d->sh.height();
    }
    if (sizePolicy().horizontalPolicy() == QSizePolicy::Ignored)
        sz.rwidth() = -1;
    if (sizePolicy().verticalPolicy() == QSizePolicy::Ignored)
        sz.rheight() = -1;
    d->msh = sz;
    return sz;
}

/*!\reimp
*/
bool QLabel::event(QEvent *e)
{
    Q_D(QLabel);
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->shortcutId) {
            QWidget * w = d->lbuddy;
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
    return QFrame::event(e);
}

/*!\reimp
*/
void QLabel::paintEvent(QPaintEvent *)
{
    Q_D(QLabel);
    QStyle *style = QWidget::style();
    QPainter paint(this);
    drawFrame(&paint);
    QRect cr = contentsRect();
    cr.adjust(d->margin, d->margin, -d->margin, -d->margin);

    QPixmap pix;
    if (pixmap())
        pix = *pixmap();
#ifndef QT_NO_PICTURE
    const QPicture *pic = picture();
#else
    const int pic = 0;
#endif
#ifndef QT_NO_MOVIE
    const QMovie *mov = movie();
#else
    const int mov = 0;
#endif

    int align = QStyle::visualAlignment(layoutDirection(), QFlag(d->align));

    if (!mov && !pix && !pic) {
        int m = indent();
        if (m < 0 && frameWidth()) // no indent, but we do have a frame
            m = fontMetrics().width('x') / 2 - d->margin;
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
    }

#ifndef QT_NO_MOVIE
    if (mov) {
        QRect r = style->itemPixmapRect(cr, align, mov->currentPixmap());
        // ### could resize movie frame at this point
        paint.drawPixmap(r.x(), r.y(), mov->currentPixmap());
    }
    else
#endif
#ifndef QT_NO_RICHTEXT
    if (d->doc) {
        QTextDocumentLayout *layout = qobject_cast<QTextDocumentLayout *>(d->doc->documentLayout());
        Q_ASSERT(layout);
        d->doc->setPageSize(QSize(cr.width(), INT_MAX));
        int rh = qRound(layout->documentSize().height());
        int yo = 0;
        if (align & Qt::AlignVCenter)
            yo = (cr.height()-rh)/2;
        else if (align & Qt::AlignBottom)
            yo = cr.height()-rh;
        QAbstractTextDocumentLayout::PaintContext context;
        QStyleOption opt(0);
        opt.init(this);
        if (!isEnabled() && style->styleHint(QStyle::SH_EtchDisabledText, &opt, this)) {
            context.palette = palette();
            context.palette.setColor(QPalette::Text, context.palette.light().color());
            QRect r = cr;
            r.translate(-cr.x()-1, -cr.y()-yo-1);
            paint.save();
            paint.translate(cr.x()+1, cr.y()+yo+1);
            paint.setClipRect(r);
            layout->draw(&paint, context);
            paint.restore();
        }

        // QSimpleRichText always draws with QPalette::Text as with
        // background mode Qt::PaletteBase. QLabel typically has
        // background mode Qt::PaletteBackground, so we create a temporary
        // color group with the text color adjusted.
        context.palette = palette();
        if (foregroundRole() != QPalette::Text && isEnabled())
            context.palette.setColor(QPalette::Foreground, context.palette.color(foregroundRole()));
        QRect r = cr;
        r.translate(-cr.x(), -cr.y()-yo);
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
            if (align & Qt::AlignVCenter)
                yo = (cr.height()-rh)/2;
            else if (align & Qt::AlignBottom)
                yo = cr.height()-rh;
            if (align & Qt::AlignRight)
                xo = cr.width()-rw;
            else if (align & Qt::AlignHCenter)
                xo = (cr.width()-rw)/2;
            paint.drawPicture(cr.x()+xo-br.x(), cr.y()+yo-br.y(), *pic);
        }
    } else
#endif
    {
#ifndef QT_NO_IMAGE_SMOOTHSCALE
        if (d->scaledcontents && !pix.isNull()) {
            if (!d->img)
                d->img = new QImage(d->lpixmap->toImage());

            if (!d->pix)
                d->pix = new QPixmap;
            if (d->pix->size() != cr.size())
                *d->pix = QPixmap::fromImage(d->img->scaled(cr.width(), cr.height()));
            pix = *d->pix;
        }
#endif
        QStyleOption opt(0);
        opt.init(this);
        if ((align & Qt::TextShowMnemonic) && !style->styleHint(QStyle::SH_UnderlineShortcut, &opt, this))
            align |= Qt::TextHideMnemonic;
        // ordinary text or pixmap label
        if (!pix.isNull()) {
            if (!isEnabled() )
                pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
            style->drawItemPixmap(&paint, cr, align, pix);
        } else {
            style->drawItemText(&paint, cr, align, palette(), isEnabled(), d->ltext);
        }

    }
}


/*!
    Updates the label, but not the frame.
*/

void QLabelPrivate::updateLabel()
{
    Q_Q(QLabel);
    valid_hints = false;
    QSizePolicy policy = q->sizePolicy();
    bool wordWrap = align & Qt::TextWordWrap;
    policy.setHeightForWidth(wordWrap);
    if (policy != q->sizePolicy())
        q->setSizePolicy(policy);
    q->releaseShortcut(shortcutId);
    if (lbuddy
#ifndef QT_NO_RICHTEXT
        && !doc
#endif
        )
        shortcutId = q->grabShortcut(QKeySequence::mnemonic(ltext));

    if (doc) {
        int align = QStyle::visualAlignment(q->layoutDirection(), QFlag(this->align));
        int flags = (wordWrap? 0 : Qt::TextSingleLine) | align;
        flags |= (q->layoutDirection() == Qt::RightToLeft) ? QTextDocumentLayout::RTL : QTextDocumentLayout::LTR;
        qobject_cast<QTextDocumentLayout *>(doc->documentLayout())->setBlockTextFlags(flags);
    }

    q->updateGeometry();
    q->update(q->contentsRect());
}

/*!
    Sets this label's buddy to \a buddy.

    When the user presses the shortcut key indicated by this label,
    the keyboard focus is transferred to the label's buddy widget.

    The buddy mechanism is only available for QLabels that contain
    plain text in which one letter is prefixed with an ampersand, \&.
    This letter is set as the shortcut key. The letter is displayed
    underlined, and the '\&' is not displayed (i.e. the \c Qt::TextShowMnemonic
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
    if (buddy)
        d->align |= Qt::TextShowMnemonic;
    else
        d->align &= ~Qt::TextShowMnemonic;

    d->lbuddy = buddy;

    d->updateLabel();
}


/*!
    Returns this label's buddy, or 0 if no buddy is currently set.

    \sa setBuddy()
*/

QWidget * QLabel::buddy() const
{
    Q_D(const QLabel);
    return d->lbuddy;
}


#ifndef QT_NO_MOVIE
void QLabelPrivate::movieUpdated(const QRect& rect)
{
    Q_Q(QLabel);
    if (lmovie && lmovie->isValid()) {
        QRect r = q->contentsRect();
        r = q->style()->itemPixmapRect(r, align, lmovie->currentPixmap());
        r.translate(rect.x(), rect.y());
        r.setWidth(qMin(r.width(), rect.width()));
        r.setHeight(qMin(r.height(), rect.height()));
        q->repaint(r);
    }
}

void QLabelPrivate::movieResized(const QSize& size)
{
    Q_Q(QLabel);
    valid_hints = false;
    movieUpdated(QRect(QPoint(0,0), size));
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

    d->lmovie = movie;
    connect(movie, SIGNAL(resized(QSize)), this, SLOT(movieResized(QSize)));
    connect(movie, SIGNAL(updated(QRect)), this, SLOT(movieUpdated(QRect)));

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
    Q_Q(QLabel);
#ifndef QT_NO_RICHTEXT
    delete doc;
    doc = 0;
#endif

    delete lpixmap;
    lpixmap = 0;
#ifndef QT_NO_PICTURE
    delete lpicture;
    lpicture = 0;
#endif
    delete img;
    img = 0;
    delete pix;
    pix = 0;

    ltext.clear();
    q->releaseShortcut(shortcutId);
    shortcutId = 0;
#ifndef QT_NO_MOVIE
    lmovie = 0;
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
    return d->lmovie;
}

#endif  // QT_NO_MOVIE

/*!
    \property QLabel::backgroundMode
    \brief the label's background mode

    Get this property with backgroundMode().
*/

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
        QString t = d->ltext;
        if (!t.isNull()) {
            d->ltext.clear();
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
        if (!d->ltext.isEmpty()) {
#ifndef QT_NO_RICHTEXT
            if (d->doc)
                d->doc->setDefaultFont(font());
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
        delete d->img;
        d->img = 0;
        delete d->pix;
        d->pix = 0;
    }
    update(contentsRect());
}

#endif // QT_NO_IMAGE_SMOOTHSCALE

/*!
    \fn void QLabel::setAlignment(Qt::AlignmentFlag flag)
    \internal

    Without this function, a call to e.g. setAlignment(Qt::AlignTop)
    results in the \c QT3_SUPPORT function setAlignment(int) being called,
    rather than setAlignment(Qt::Alignment).
*/


#include "moc_qlabel.cpp"

#endif // QT_NO_LABEL
