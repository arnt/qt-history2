/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpainter.h"
#include "qevent.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qabstractbutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include <limits.h>
#include "qaction.h"
#include "qclipboard.h"
#include <qdebug.h>
#include <qurl.h>
#include "qlabel_p.h"

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
QLabel::QLabel(QWidget *parent, Qt::WindowFlags f)
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
QLabel::QLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
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

QLabel::QLabel(QWidget *parent, const char *name, Qt::WindowFlags f)
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
                Qt::WindowFlags f)
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
    an ampersand, \&), when the user presses Alt+ the underlined letter,
    focus is passed to the buddy widget.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setBuddy(), setAlignment(), setFrameStyle(),
    setIndent()
*/
QLabel::QLabel(QWidget *buddy, const QString &text,
                QWidget *parent, const char *name, Qt::WindowFlags f)
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
    shortcutId = 0;
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
    textLayoutDirty = false;
    textDirty = false;
    textformat = Qt::AutoText;
    doc = 0;
    control = 0;
    textInteractionFlags = Qt::LinksAccessibleByMouse;

    q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    hasCustomCursor = false;
    openExternalLinks = false;
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

    If a buddy has been set, the buddy mnemonic key is updated
    from the new text.

    The label resizes itself if auto-resizing is enabled.

    Note that QLabel is well-suited to display small rich text
    documents, such as small documents that get their document
    specific settings (font, text color, link color) from the label's
    palette and font properties. For large documents, use QTextEdit
    in read-only mode instead. QTextEdit can also provide a scrollbar
    when necessary.

    Note: This function enables mouse tracking if \atext contains rich
    text.

    \sa setTextFormat(), setBuddy(), alignment
*/

void QLabel::setText(const QString &text)
{
    Q_D(QLabel);
    if (d->text == text)
        return;

    // don't delete the document in clearContents() if we already have one, we're
    // going to need it for sure.
    QTextDocument *currentDoc = d->doc;
    d->doc = 0;

    d->clearContents();
    d->text = text;
    d->textDirty = true;

    d->doc = currentDoc;

    if (!d->doc) {
        d->doc = new QTextDocument(this);
        d->doc->setUndoRedoEnabled(false);
        d->doc->setDefaultFont(font());
    }

    if (d->isRichText()) {
        setMouseTracking(true);
    } else {
        // Note: mouse tracking not disabled intentionally
        if (d->textInteractionFlags & (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard)) {
            d->ensureTextControl();
        }
    }
#ifndef QT_NO_SHORTCUT
    if (d->buddy)
        d->updateShortcut();
#endif

    d->textInteractionFlagsChanged(); // create text control as necessary
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
    if (!d->pixmap || d->pixmap->cacheKey() != pixmap.cacheKey()) {
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
        ensureTextLayouted();
        const qreal oldTextWidth = doc->textWidth();
        // Add indentation
        QFontMetrics fm(q->fontMetrics());
        int m = indent;

        if (m < 0 && q->frameWidth()) // no indent, but we do have a frame
            m = fm.width(QLatin1Char('x')) - margin*2;
        if (m > 0) {
            int align = QStyle::visualAlignment(q->layoutDirection(), QFlag(this->align));
            if ((align & Qt::AlignLeft) || (align & Qt::AlignRight))
                hextra += m;
            if ((align & Qt::AlignTop) || (align & Qt::AlignBottom))
                vextra += m;
        }

        // Calculate the length of document if w is the width
        if (align & Qt::TextWordWrap) {
            if (w >= 0) {
                w = qMax(w-hextra-contentsMargin.width(), 0); // strip margin and indent
                doc->setTextWidth(w);
            } else {
                doc->adjustSize();
            }
        } else {
            doc->setTextWidth(-1);
        }
        br = QRect(QPoint(0, 0), doc->size().toSize());

        // restore state
        doc->setTextWidth(oldTextWidth);
    } else {
        QFontMetrics fm = q->fontMetrics();
        br = QRect(QPoint(0, 0), QSize(fm.averageCharWidth(), fm.lineSpacing()));
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

/*!
    \property QLabel::openExternalLinks
    \since 4.2

    Specifies whether QLabel should automatically open links using
    QDesktopServices::openUrl() instead of emitting the
    anchorClicked() signal.

    \bold{Note:} The textInteractionFlags set on the label need to include
    either LinksAccessibleByMouse or LinksAccessibleByKeyboard.

    The default value is false.

    \sa textInteractionFlags()
*/
bool QLabel::openExternalLinks() const
{
    Q_D(const QLabel);
    return d->openExternalLinks;
}

void QLabel::setOpenExternalLinks(bool open)
{
    Q_D(QLabel);
    d->openExternalLinks = open;
    if (d->control)
        d->control->setOpenExternalLinks(open);
}

void QLabelPrivate::textInteractionFlagsChanged()
{
    Q_Q(QLabel);
    if (!doc)
        return;
    if (textInteractionFlags & Qt::LinksAccessibleByKeyboard)
        q->setFocusPolicy(Qt::StrongFocus);
    else if (textInteractionFlags & Qt::TextSelectableByKeyboard)
        q->setFocusPolicy(Qt::ClickFocus);
    else
        q->setFocusPolicy(Qt::NoFocus);

    bool richText = isRichText();
    if ((richText && textInteractionFlags != Qt::NoTextInteraction)
        || (!richText && (textInteractionFlags & (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard)))) {
        ensureTextControl();
        control->setTextInteractionFlags(textInteractionFlags);
    } else {
        delete control;
        control = 0;
    }
}

/*!
    \property QLabel::textInteractionFlags
    \since 4.2

    Specifies how the label should interact with user input if it displays text.

    If the flags contain Qt::LinksAccessibleByKeyboard the focus policy is also
    automatically set to Qt::StrongFocus. If Qt::TextSelectableByKeyboard is set
    then the focus policy is set to Qt::ClickFocus.

    The default value is Qt::LinksAccessibleByMouse.
*/
void QLabel::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    Q_D(QLabel);
    if (d->textInteractionFlags == flags)
        return;
    d->textInteractionFlags = flags;
    d->textInteractionFlagsChanged();
}

Qt::TextInteractionFlags QLabel::textInteractionFlags() const
{
    Q_D(const QLabel);
    return d->textInteractionFlags;
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
    if (d->valid_hints) {
        if (d->sizePolicy == sizePolicy())
            return d->msh;
    }

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
    d->msh = msh;
    d->sizePolicy = sizePolicy();
    return msh;
}

/*!\reimp
*/
void QLabel::mousePressEvent(QMouseEvent *ev)
{
    Q_D(QLabel);
    if (!d->doc) {
        ev->ignore();
        return;
    }

    d->sendControlEvent(ev);
}

/*!\reimp
*/
void QLabel::mouseMoveEvent(QMouseEvent *ev)
{
    Q_D(QLabel);
    if (!d->doc) {
        ev->ignore();
        return;
    }

    d->sendControlEvent(ev);
}

/*!\reimp
*/
void QLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_D(QLabel);
    if (!d->doc) {
        ev->ignore();
        return;
    }

    d->sendControlEvent(ev);
}

/*!\reimp
*/
void QLabel::contextMenuEvent(QContextMenuEvent *ev)
{
#ifdef QT_NO_CONTEXTMENU
    Q_UNUSED(ev);
#else
    Q_D(QLabel);
    if (!d->doc) {
        ev->ignore();
        return;
    }
    QMenu *menu = d->createStandardContextMenu(ev->pos());
    if (!menu) {
        ev->ignore();
        return;
    }
    ev->accept();
    menu->exec(ev->globalPos());
    delete menu;
#endif
}

/*!
    \reimp
*/
void QLabel::focusInEvent(QFocusEvent *ev)
{
    Q_D(QLabel);
    if (d->doc) {
        d->ensureTextControl();
        d->sendControlEvent(ev);
    }
    QFrame::focusInEvent(ev);
}

/*!
    \reimp
*/
void QLabel::focusOutEvent(QFocusEvent *ev)
{
    Q_D(QLabel);
    d->sendControlEvent(ev);
    QFrame::focusOutEvent(ev);
}

/*!\reimp
*/
bool QLabel::focusNextPrevChild(bool next)
{
    Q_D(QLabel);
    if (d->control && d->control->setFocusToNextOrPreviousAnchor(next))
        return true;
    return QFrame::focusNextPrevChild(next);
}

/*!\reimp
*/
void QLabel::keyPressEvent(QKeyEvent *ev)
{
    Q_D(QLabel);
    d->sendControlEvent(ev);
}

/*!\reimp
*/
bool QLabel::event(QEvent *e)
{
    Q_D(QLabel);
    QEvent::Type type = e->type();

#ifndef QT_NO_SHORTCUT
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
    } else
#endif
    if (type == QEvent::Resize && d->doc) {
        d->textLayoutDirty = true;
    } else if (e->type() == QEvent::StyleChange) {
        d->updateLabel();
    }

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
        const bool underline = (bool)style->styleHint(QStyle::SH_UnderlineShortcut, 0, this, 0);
#ifndef QT_NO_SHORTCUT
        if (d->shortcutId != 0
            && underline != d->shortcutCursor.charFormat().fontUnderline()) {
                QTextCharFormat fmt;
                fmt.setFontUnderline(underline);
                d->shortcutCursor.mergeCharFormat(fmt);
        }
#endif
        d->ensureTextLayouted();
        QAbstractTextDocumentLayout *layout = d->doc->documentLayout();
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
        if (d->control) {
            d->control->setPalette(context.palette);
            d->control->drawContents(&painter);
        } else {
            layout->draw(&painter, context);
        }
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
                d->scaledpixmap = new QPixmap(QPixmap::fromImage(d->cachedimage->scaled(cr.size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
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
        const bool wrap = align & Qt::TextWordWrap;
        policy.setHeightForWidth(wrap);
        if (policy != q->sizePolicy())
            q->setSizePolicy(policy);
        textLayoutDirty = true;
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
    text in which one character is prefixed with an ampersand,
    '&'.  This character is set as the shortcut key. See the \l
    {QShortcut#mnemonic}{QShortcut} documentation for details (to
    display an actual ampersand, use '&&').

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
    if (d->doc) {
        if (d->shortcutId)
            releaseShortcut(d->shortcutId);
        d->shortcutId = 0;
        d->textDirty = true;
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
    Q_ASSERT(shortcutId == 0);
    ensureTextLayouted();

    // Underline the first character that follows an ampersand
    shortcutCursor = doc->find(QLatin1String("&"));
    if (shortcutCursor.isNull())
        return;
    shortcutId = q->grabShortcut(QKeySequence::mnemonic(text));
    shortcutCursor.deleteChar(); // remove the ampersand
    shortcutCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
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
    cleared. The label does NOT take ownership of the movie.

    The buddy shortcut, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa movie(), setBuddy()
*/

void QLabel::setMovie(QMovie *movie)
{
    Q_D(QLabel);
    d->clearContents();

    if (!movie)
        return;

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
    delete control;
    control = 0;
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
    Q_Q(QLabel);
#ifndef QT_NO_SHORTCUT
    if (shortcutId)
        q->releaseShortcut(shortcutId);
    shortcutId = 0;
#endif
#ifndef QT_NO_MOVIE
    if (movie) {
        QObject::disconnect(movie, SIGNAL(resized(QSize)), q, SLOT(_q_movieResized(QSize)));
        QObject::disconnect(movie, SIGNAL(updated(QRect)), q, SLOT(_q_movieUpdated(QRect)));
    }
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
    if(ev->type() == QEvent::FontChange || ev->type() == QEvent::ApplicationFontChange) {
        if (d->doc) {
            d->doc->setDefaultFont(font());
            d->updateLabel();
        }
    } else if (ev->type() == QEvent::PaletteChange && d->control) {
        d->control->setPalette(palette());
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
        m = q->fontMetrics().width(QLatin1Char('x')) / 2 - margin;
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

void QLabelPrivate::ensureTextLayouted() const
{
    if (!textLayoutDirty && !textDirty)
        return;
    Q_Q(const QLabel);
    if (doc) {
        if (textDirty) {
            if (isRichText())
                doc->setHtml(text);
            else
                doc->setPlainText(text);
            doc->setUndoRedoEnabled(false);
        }

        QTextOption opt = doc->defaultTextOption();

        Qt::Alignment align = QStyle::visualAlignment(q->layoutDirection(), QFlag(this->align));
        opt.setAlignment(align);

        if (this->align & Qt::TextWordWrap)
            opt.setWrapMode(QTextOption::WordWrap);
        else
            opt.setWrapMode(QTextOption::ManualWrap);

        opt.setTextDirection(q->layoutDirection());

        doc->setDefaultTextOption(opt);

        QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
        fmt.setMargin(0);
        doc->rootFrame()->setFrameFormat(fmt);
        doc->setTextWidth(documentRect().width());
    }
    textLayoutDirty = false;
    textDirty = false;
}

void QLabelPrivate::ensureTextControl()
{
    Q_Q(QLabel);
    if (control || !doc)
        return;
    ensureTextLayouted();
    control = new QTextControl(doc, q);
    control->setTextInteractionFlags(textInteractionFlags);
    control->setOpenExternalLinks(openExternalLinks);
    control->setPalette(q->palette());
    control->setFocus(q->hasFocus());
    QObject::connect(control, SIGNAL(updateRequest(QRectF)),
                     q, SLOT(update()));
    QObject::connect(control, SIGNAL(linkHovered(QString)),
                     q, SLOT(_q_linkHovered(QString)));
    QObject::connect(control, SIGNAL(linkActivated(QString)),
                     q, SIGNAL(linkActivated(QString)));
}

void QLabelPrivate::sendControlEvent(QEvent *e)
{
    Q_Q(QLabel);
    if (!control) {
        e->ignore();
        return;
    }
    control->processEvent(e, -layoutRect().topLeft(), q);
}

void QLabelPrivate::_q_linkHovered(const QString &anchor)
{
    Q_Q(QLabel);
    if (anchor.isEmpty()) { // restore cursor
#ifndef QT_NO_CURSOR
        q->setCursor(hasCustomCursor ? cursor : Qt::ArrowCursor);
#endif
    } else {
#ifndef QT_NO_CURSOR
        hasCustomCursor = q->testAttribute(Qt::WA_SetCursor);
        if (hasCustomCursor)
            cursor = q->cursor();
        q->setCursor(Qt::PointingHandCursor);
#endif
    }
    emit q->linkHovered(anchor);
}

// Return the layout rect - this is the rect that is given to the layout painting code
// This may be different from the document rect since vertical alignment is not
// done by the text layout code
QRect QLabelPrivate::layoutRect() const
{
    ensureTextLayouted();
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

#ifndef QT_NO_CONTEXTMENU
QMenu *QLabelPrivate::createStandardContextMenu(const QPoint &pos)
{
    QString linkToCopy;
    QPoint p;
    if (isRichText()) {
        p = layoutPoint(pos);
        linkToCopy = doc->documentLayout()->anchorAt(p);
    }

    if (linkToCopy.isEmpty() && !control)
        return 0;

    return control->createStandardContextMenu(p, q_func());
}
#endif

/*!
    \fn void QLabel::linkHovered(const QString &link)
    \since 4.2

    This signal is emitted when the user hovers over a link. The URL
    referred to by the anchor is passed in \a link.

    \sa linkActivated()
*/


/*!
    \fn void QLabel::linkActivated(const QString &link)
    \since 4.2

    This signal is emitted when the user clicks a link. The URL
    referred to by the anchor is passed in \a link.

    \sa linkHovered()
*/

#include "moc_qlabel.cpp"
