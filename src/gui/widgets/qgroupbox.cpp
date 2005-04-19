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

#include "qgroupbox.h"
#ifndef QT_NO_GROUPBOX
#include "qapplication.h"
#include "qbitmap.h"
#include "qcheckbox.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qradiobutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include <private/qwidget_p.h>

class QGroupBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGroupBox)
public:

    QGroupBoxPrivate():
        checkbox(0),
        topMargin(0){}
    void skip();
    void init();
    void calculateFrame();
    void updateCheckBoxGeometry();
    QString title;
    int align;
    int shortcutId;

    void fixFocus();
    void setChildrenEnabled(bool b);
    uint bFlat : 1;

    QCheckBox *checkbox;
    int topMargin;
};

/*!
    \class QGroupBox qgroupbox.h
    \brief The QGroupBox widget provides a group box frame with a title.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    A group box provides a frame, a title and a keyboard shortcut, and
    displays various other widgets inside itself. The title is on top,
    the keyboard shortcut moves keyboard focus to one of the group
    box's child widgets.

    QGroupBox also lets you set the \l title (normally set in the
    constructor) and the title's alignment(). If setCheckable(true) is
    called then the group box is checkable(), and it can be
    setChecked(). Checkable group boxes child widgets are enabled or
    disabled depending on whether or not the group box is checked().

    To minimize space consumption, you can remove the right, left and
    bottom edges of the frame with setFlat().

    \inlineimage qgrpbox-w.png Screenshot in Windows style

    \sa QButtonGroup
*/



/*!
    Constructs a group box widget with no title and parent \a parent.
*/

QGroupBox::QGroupBox(QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    d->init();
}

/*!
    Constructs a group box with the title \a title and parent \a
    parent.
*/

QGroupBox::QGroupBox(const QString &title, QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    d->init();
    setTitle(title);
}


/*!
    Destroys the group box.
*/
QGroupBox::~QGroupBox()
{
}

void QGroupBoxPrivate::init()
{
    align = Qt::AlignLeft;
    shortcutId = 0;
    bFlat = false;
    calculateFrame();
}


void QGroupBox::setTitle(const QString &title)
{
    Q_D(QGroupBox);
    if (d->title == title)                                // no change
        return;
    d->title = title;
    releaseShortcut(d->shortcutId);
    if (d->checkbox) {
        d->checkbox->setText(d->title);
        d->shortcutId = 0; // the checkbox does the shortcut for us
    } else {
        d->shortcutId = grabShortcut(QKeySequence::mnemonic(title));
    }
    d->calculateFrame();
    d->updateCheckBoxGeometry();

    update();
    updateGeometry();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::NameChanged);
#endif
}

/*!
    \property QGroupBox::title
    \brief the group box title text.

    The group box title text will have a focus-change keyboard
    shortcut if the title contains \&, followed by a letter.

    \code
        g->setTitle("&User information");
    \endcode
    This produces "<u>U</u>ser information"; Alt+U moves the keyboard
    focus to the group box.

    There is no default title text.
*/

QString QGroupBox::title() const
{
    Q_D(const QGroupBox);
    return d->title;
}

/*!
    \property QGroupBox::alignment
    \brief the alignment of the group box title.

    The title is always placed on the upper frame line. The horizontal
    alignment can be specified by the alignment parameter.

    The alignment is one of the following flags:
    \list
    \i \c Qt::AlignLeft aligns the title text to the left.
    \i \c Qt::AlignRight aligns the title text to the right.
    \i \c Qt::AlignHCenter aligns the title text centered.
    \endlist

    The default alignment is \c Qt::AlignLeft.

    \sa Qt::Alignment
*/
Qt::Alignment QGroupBox::alignment() const
{
    Q_D(const QGroupBox);
    return QFlag(d->align);
}

void QGroupBox::setAlignment(int alignment)
{
    Q_D(QGroupBox);
    d->align = alignment;
    d->updateCheckBoxGeometry();
    update();
}

/*! \reimp
*/
void QGroupBox::resizeEvent(QResizeEvent *e)
{
    Q_D(QGroupBox);
    QWidget::resizeEvent(e);
    d->updateCheckBoxGeometry();
}

/*! \reimp
*/

void QGroupBox::paintEvent(QPaintEvent *event)
{
    Q_D(QGroupBox);
    QPainter paint(this);

    QRect frameRect = rect();
    frameRect.setTop(d->topMargin);
    QStyleOptionFrame opt;
    opt.init(this);
    opt.rect = frameRect;

    opt.state |= QStyle::State_Sunken;
    opt.lineWidth = 1;
    opt.midLineWidth = 0;

    if (d->title.size() && !d->checkbox) {        // draw title
        QFontMetrics fm = paint.fontMetrics();
        int h = fm.height();
        int tw = fm.width(d->title + QLatin1Char(' '));
        int marg = d->bFlat ? 0 : (8 + fm.width(QLatin1Char(' ')));
        QRect rect = this->rect().adjusted(marg, 0, -marg, 0);
        QRect r = QStyle::alignedRect(opt.direction, QFlag(d->align | Qt::AlignTop), QSize(tw, h), rect);
        int va = style()->styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, &opt, this);
        if(va & Qt::AlignTop)
            r.translate(0, -fm.descent());
        int textColor = style()->styleHint(QStyle::SH_GroupBox_TextLabelColor, &opt, this);
        if (textColor != 0 && !testAttribute(Qt::WA_SetPalette))
            paint.setPen(QColor::fromRgba((QRgb)textColor));
        if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &opt, this))
            va |= Qt::TextHideMnemonic;
        style()->drawItemText(&paint, r, Qt::TextShowMnemonic | Qt::AlignHCenter | va, palette(),
                              isEnabled(), d->title);
        paint.setClipRegion(event->region().subtract(r)); // clip everything but title
    } else if (d->checkbox) {
        QRect cbClip = d->checkbox->geometry();
        QFontMetrics fm = paint.fontMetrics();
        cbClip.setX(cbClip.x() - fm.width(QLatin1Char(' ')));
        paint.setClipRegion(event->region().subtract(cbClip));
    }
    if (d->bFlat) {
            QRect fr = frameRect;
            QPoint p1(fr.x(), fr.y() + 1);
            QPoint p2(fr.x() + fr.width(), p1.y());
            // ### This should probably be a style primitive.
            qDrawShadeLine(&paint, p1, p2, palette(), true, 1, 0);
    } else {
        style()->drawPrimitive(QStyle::PE_FrameGroupBox, &opt, &paint, this);
    }
}

/*! \reimp  */
bool QGroupBox::event(QEvent *e)
{
    Q_D(QGroupBox);
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->shortcutId) {
            d->fixFocus();
            return true;
        }
    }
    return QWidget::event(e);
}

/*!\reimp */
void QGroupBox::childEvent(QChildEvent *c)
{
    Q_D(QGroupBox);
    if (c->type() != QEvent::ChildAdded || !c->child()->isWidgetType())
        return;
    QWidget *w = (QWidget*)c->child();
    if (d->checkbox) {
        if (w == d->checkbox)
            return;
        if (d->checkbox->isChecked()) {
            if (!w->testAttribute(Qt::WA_ForceDisabled))
                w->setEnabled(true);
        } else {
            if (w->isEnabled()) {
                w->setEnabled(false);
                w->setAttribute(Qt::WA_ForceDisabled, false);
            }
        }
    }
}


/*!
    \internal

    This private slot finds a widget in this group box that can accept
    focus, and gives the focus to that widget.
*/

void QGroupBoxPrivate::fixFocus()
{
    Q_Q(QGroupBox);
    QWidget *fw = q->focusWidget();
    if (!fw) {
#ifndef QT_NO_RADIOBUTTON
        QWidget * best = 0;
#endif
        QWidget * candidate = 0;
        QWidget * w = q;
        while ((w = w->nextInFocusChain()) != q) {
            if (q->isAncestorOf(w) && (w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus && w->isVisibleTo(q)) {
#ifndef QT_NO_RADIOBUTTON
                if (!best && qobject_cast<QRadioButton*>(w) && ((QRadioButton*)w)->isChecked())
                    // we prefer a checked radio button or a widget that
                    // already has focus, if there is one
                    best = w;
                else
#endif
                    if (!candidate)
                        // but we'll accept anything that takes focus
                        candidate = w;
            }
        }
#ifndef QT_NO_RADIOBUTTON
        if (best)
            fw = best;
        else
#endif
            if (candidate)
                fw = candidate;
    }
    if (fw)
        fw->setFocus();
}


/*
    Sets the right frame rect depending on the title.
*/
void QGroupBoxPrivate::calculateFrame()
{
    Q_Q(QGroupBox);
    int va = q->style()->styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, 0, q);

    topMargin = 0;
    QFontMetrics fm = q->fontMetrics();
    if (checkbox) {
        if (va & Qt::AlignTop)
            topMargin = checkbox->sizeHint().height();
	else
	    topMargin = checkbox->sizeHint().height()/2;
    } else if (title.size()) {
        if (va & Qt::AlignVCenter)
            topMargin = fm.height()/2;
        else if (va & Qt::AlignTop)
            topMargin = fm.height();
    }

    int marg = bFlat ? 0 : 2; // ###NEEDS TO BE A STYLE ATTRIBUTE
    q->setContentsMargins(marg, topMargin + marg + 2, marg, marg);
}



/*! \reimp
 */
void QGroupBox::focusInEvent(QFocusEvent *)
{ // note no call to super
    Q_D(QGroupBox);
    d->fixFocus();
}


/*!
  \reimp
*/

QSize QGroupBox::minimumSizeHint() const
{
    Q_D(const QGroupBox);
    QSize sh = QWidget::minimumSizeHint();
    if (!sh.isValid())
        sh = QSize(d->leftmargin + d->rightmargin, d->topmargin + d->bottommargin);
    sh += QSize(2,2);
    QSize m((d->bFlat ? 0 : 2*8), 0);
    if (d->checkbox) {
        if (!d->bFlat)
            m.rwidth() += fontMetrics().width(QLatin1Char(' '));
        sh = sh.expandedTo(d->checkbox->sizeHint() + m);
    } else if (!d->title.isEmpty()) {
        sh += fontMetrics().size(0, d->title + QLatin1Char(' ')) + m;
    }
    return sh;
}

/*!
    \property QGroupBox::flat
    \brief whether the group box is painted flat or has a frame

    By default a group box has a surrounding frame, with the title
    being placed on the upper frame line. In flat mode the right, left
    and bottom frame lines are omitted, and only the thin line at the
    top is drawn.

    \sa title
*/
bool QGroupBox::isFlat() const
{
    Q_D(const QGroupBox);
    return d->bFlat;
}

void QGroupBox::setFlat(bool b)
{
    Q_D(QGroupBox);
    if ((bool)d->bFlat == b)
        return;
    d->bFlat = b;
    d->updateCheckBoxGeometry();
    update();
}


/*!
    \property QGroupBox::checkable
    \brief Whether the group box has a checkbox in its title.

    If this property is true, the group box has a checkbox. If the
    checkbox is checked (which is the default), the group box's
    children are enabled.

    setCheckable() controls whether or not the group box has a
    checkbox, and isCheckable() controls whether the checkbox is
    checked or not.
*/
void QGroupBox::setCheckable(bool b)
{
    Q_D(QGroupBox);
    if ((d->checkbox != 0) == b)
        return;

    if (b) {
        if (!d->checkbox) {
            d->checkbox = new QCheckBox(title(), this);
            d->checkbox->setObjectName(QLatin1String("qt_groupbox_checkbox"));
            setChecked(true);
            d->setChildrenEnabled(true);
            connect(d->checkbox, SIGNAL(toggled(bool)),
                     this, SLOT(setChildrenEnabled(bool)));
            connect(d->checkbox, SIGNAL(toggled(bool)),
                     this, SIGNAL(toggled(bool)));
            d->updateCheckBoxGeometry();
        }
        d->checkbox->show();
    } else {
        d->setChildrenEnabled(true);
        delete d->checkbox;
        d->checkbox = 0;
    }
    QString title = d->title;
    d->title.clear();
    setTitle(title); // update, including the shortcut
}

bool QGroupBox::isCheckable() const
{
    Q_D(const QGroupBox);
    return (d->checkbox != 0);
}


bool QGroupBox::isChecked() const
{
    Q_D(const QGroupBox);
    return d->checkbox && d->checkbox->isChecked();
}


/*!
    \fn void QGroupBox::toggled(bool on)

    If the group box has a check box (see \l isCheckable()) this signal
    is emitted when the check box is toggled. \a on is true if the check
    box is checked; otherwise it is false.
*/

/*!
    \property QGroupBox::checked
    \brief Whether the group box's checkbox is checked.

    If the group box has a check box (see \l isCheckable()), and the
    check box is checked (see \l isChecked()), the group box's children
    are enabled. If the checkbox is unchecked the children are
    disabled.
*/
void QGroupBox::setChecked(bool b)
{
    Q_D(QGroupBox);
    if (d->checkbox)
        d->checkbox->setChecked(b);
}

/*
  sets all children of the group box except the qt_groupbox_checkbox
  to either disabled/enabled
*/
void QGroupBoxPrivate::setChildrenEnabled(bool b)
{
    Q_Q(QGroupBox);
    QObjectList childs = q->children();
    if (childs.isEmpty())
        return;
    for (int i = 0; i < childs.size(); ++i) {
        QObject *o = childs.at(i);
        if (o->isWidgetType()
             && o != checkbox
           ) {
            QWidget *w = static_cast<QWidget *>(o);
            if (b) {
                if (!w->testAttribute(Qt::WA_ForceDisabled))
                    w->setEnabled(true);
            } else {
                if (w->isEnabled()) {
                    w->setEnabled(false);
                    w->setAttribute(Qt::WA_ForceDisabled, false);
                }
            }
        }
    }
}

/*! \reimp */
void QGroupBox::changeEvent(QEvent *ev)
{
    Q_D(QGroupBox);
    if(ev->type() == QEvent::EnabledChange) {
        if (d->checkbox && isEnabled()) {
            // we are being enabled - disable children
            if (!d->checkbox->isChecked())
                d->setChildrenEnabled(false);
        }
    } else if(ev->type() == QEvent::FontChange || ev->type() == QEvent::StyleChange) {
        d->updateCheckBoxGeometry();
        d->calculateFrame();
    }
    QWidget::changeEvent(ev);
}

/*
  recalculates and sets the checkbox setGeometry
*/
void QGroupBoxPrivate::updateCheckBoxGeometry()
{
    Q_Q(QGroupBox);
    if (checkbox) {
        QSize cbSize = checkbox->sizeHint();
        int marg = bFlat ? 0 : (8 + q->fontMetrics().width(QLatin1Char(' ')));
        QRect rect = q->rect().adjusted(marg, 0, -marg, 0);
        QRect cbRect = QStyle::alignedRect(q->layoutDirection(), QFlag(this->align | Qt::AlignTop), cbSize, rect);
        checkbox->setGeometry(cbRect);
    }
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGroupBox::QGroupBox(QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    setObjectName(name);
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGroupBox::QGroupBox(const QString &title, QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    setObjectName(name);
    d->init();
    setTitle(title);
}
#endif // QT3_SUPPORT

#include "moc_qgroupbox.cpp"

#endif //QT_NO_GROUPBOX
