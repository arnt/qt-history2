/****************************************************************************
**
** Implementation of QGroupBox widget class.
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

#include "qgroupbox.h"
#ifndef QT_NO_GROUPBOX
#include "qlayout.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qaccel.h"
#include "qradiobutton.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qevent.h"
#include "qstyle.h"
#include "qcheckbox.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
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
#ifndef QT_NO_CHECKBOX
    void updateCheckBoxGeometry();
#endif
    QString str;
    int align;
    int lenvisible;
#ifndef QT_NO_ACCEL
    QAccel * accel;
#endif

    void fixFocus();
    void setChildrenEnabled(bool b);
    uint bFlat : 1;

    QCheckBox *checkbox;
    int topMargin;
};

#define d d_func()
#define q q_func()


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
    constructor) and the title's alignment().

    To minimize space consumption, you can remove the right, left and
    bottom edges of the frame with setFlat().

    <img src=qgrpbox-w.png>

    \sa QButtonGroup
*/



/*!
    Constructs a group box widget with no title and parent \a parent.
*/

QGroupBox::QGroupBox(QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    d->init();
}

/*!
    Constructs a group box with the title \a title and parent \a
    parent.
*/

QGroupBox::QGroupBox(const QString &title, QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
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
    align = AlignAuto;
#ifndef QT_NO_ACCEL
    accel = 0;
#endif
    lenvisible = 0;
    bFlat = false;
}


void QGroupBox::setTitle(const QString &title)
{
    if (d->str == title)                                // no change
        return;
    d->str = title;
#ifndef QT_NO_ACCEL
    if (d->accel)
        delete d->accel;
    d->accel = 0;
    int s = QAccel::shortcutKey(title);
    if (s) {
        d->accel = new QAccel(this, "automatic focus-change accelerator");
        d->accel->connectItem(d->accel->insertItem(s, 0),
                            this, SLOT(fixFocus()));
    }
#endif
    if (d->checkbox) {
        d->checkbox->setText(d->str);
        d->updateCheckBoxGeometry();
    }
    d->calculateFrame();

    update();
    updateGeometry();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::NameChanged);
#endif
}

/*!
    \property QGroupBox::title
    \brief the group box title text.

    The group box title text will have a focus-change keyboard
    accelerator if the title contains \&, followed by a letter.

    \code
        g->setTitle("&User information");
    \endcode
    This produces "<u>U</u>ser information"; Alt+U moves the keyboard
    focus to the group box.

    There is no default title text.
*/

QString QGroupBox::title() const
{
    return d->str;
}

/*!
    \property QGroupBox::alignment
    \brief the alignment of the group box title.

    The title is always placed on the upper frame line. The horizontal
    alignment can be specified by the alignment parameter.

    The alignment is one of the following flags:
    \list
    \i \c AlignAuto aligns the title according to the language,
    usually to the left.
    \i \c AlignLeft aligns the title text to the left.
    \i \c AlignRight aligns the title text to the right.
    \i \c AlignHCenter aligns the title text centered.
    \endlist

    The default alignment is \c AlignAuto.

    \sa Qt::AlignmentFlags
*/
int QGroupBox::alignment() const
{
    return d->align;
}

void QGroupBox::setAlignment(int alignment)
{
    d->align = alignment;
    d->updateCheckBoxGeometry();
    update();
}

/*! \reimp
*/
void QGroupBox::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    d->calculateFrame();
    if (d->align & AlignRight || d->align & AlignCenter ||
         (QApplication::reverseLayout() && !(d->align & AlignLeft)))
        d->updateCheckBoxGeometry();
}

/*! \reimp
*/

void QGroupBox::paintEvent(QPaintEvent *event)
{
    QPainter paint(this);

    QRect frameRect = rect();
    frameRect.setTop(d->topMargin);

    if (d->lenvisible && !isCheckable()) {        // draw title
        QFontMetrics fm = paint.fontMetrics();
        int h = fm.height();
        int tw = fm.width(d->str, d->lenvisible) + fm.width(QChar(' '));
        int x;
        int marg = d->bFlat ? 0 : 8;
        if (d->align & AlignHCenter)                // center alignment
            x = frameRect.width()/2 - tw/2;
        else if (d->align & AlignRight)        // right alignment
            x = frameRect.width() - tw - marg;
        else if (d->align & AlignLeft)                 // left alignment
            x = marg;
        else { // auto align
            if(QApplication::reverseLayout())
                x = frameRect.width() - tw - marg;
            else
                x = marg;
        }
        QRect r(x, 0, tw, h);
        int va = style().styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, this);
        if(va & AlignTop)
            r.moveBy(0, fm.descent());
        QColor pen((QRgb) style().styleHint(QStyle::SH_GroupBox_TextLabelColor, this));
        if (!style().styleHint(QStyle::SH_UnderlineAccelerator, this))
            va |= NoAccel;
        style().drawItem(&paint, r, ShowPrefix | AlignHCenter | va, palette(),
                          isEnabled(), d->str, -1, testAttribute(WA_SetPalette) ? 0 : &pen);
        paint.setClipRegion(event->region().subtract(r)); // clip everything but title
    } else if (d->checkbox) {
        QRect cbClip = d->checkbox->geometry();
        QFontMetrics fm = paint.fontMetrics();
        cbClip.setX(cbClip.x() - fm.width(QChar(' ')));
        cbClip.setWidth(cbClip.width() + fm.width(QChar(' ')));
        paint.setClipRegion(event->region().subtract(cbClip));
    }
    if (d->bFlat) {
            QRect fr = frameRect;
            QPoint p1(fr.x(), fr.y() + 1);
            QPoint p2(fr.x() + fr.width(), p1.y());
            // ### This should probably be a style primitive.
            qDrawShadeLine(&paint, p1, p2, palette(), true, 1, 0);
    } else {
        QStyleOption opt(1, 0);
        QStyle::SFlags flags = QStyle::Style_Default | QStyle::Style_Sunken;
        if (hasFocus())
            flags |= QStyle::Style_HasFocus;
        if (testAttribute(WA_UnderMouse))
            flags |= QStyle::Style_MouseOver;
        style().drawPrimitive(QStyle::PE_PanelGroupBox, &paint, frameRect, palette(), flags, opt);
    }
}

/*! \reimp  */
bool QGroupBox::event(QEvent * e)
{
    return QWidget::event(e);
}

/*!\reimp */
void QGroupBox::childEvent(QChildEvent *c)
{
    if (c->type() != QEvent::ChildInserted || !c->child()->isWidgetType())
        return;
    QWidget *w = (QWidget*)c->child();
    if (d->checkbox) {
        if (w == d->checkbox)
            return;
        if (d->checkbox->isChecked()) {
            if (!w->testAttribute(WA_ForceDisabled))
                w->setEnabled(true);
        } else {
            if (w->isEnabled()) {
                w->setEnabled(false);
                w->setAttribute(WA_ForceDisabled, false);
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
    QWidget *fw = q->focusWidget();
    if (!fw) {
#ifndef QT_NO_RADIOBUTTON
        QWidget * best = 0;
#endif
        QWidget * candidate = 0;
        QWidget * w = q;
        while ((w = w->nextInFocusChain()) != q) {
            if (q->isAncestorOf(w) && (w->focusPolicy() & TabFocus) == TabFocus && w->isVisibleTo(q)) {
#ifndef QT_NO_RADIOBUTTON
                if (!best && qt_cast<QRadioButton*>(w) && ((QRadioButton*)w)->isChecked())
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
    Sets the right frame rect depending on the title. Also calculates
    the visible part of the title.
*/
void QGroupBoxPrivate::calculateFrame()
{
    lenvisible = str.length();

    d->topMargin = 0;
    QFontMetrics fm = q->fontMetrics();
    if (lenvisible && !checkbox) { // do we have a label?
        while (lenvisible) {
            int tw = fm.width(str, lenvisible) + 4*fm.width(QChar(' '));
            if (tw < q->width())
                break;
            lenvisible--;
        }
        if (lenvisible) { // but do we also have a visible label?
            int va = q->style().styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, q);
            if(va & AlignVCenter)
                d->topMargin = fm.height()/2;
            else if(va & AlignTop)
                d->topMargin = fm.ascent();
        }
    }
    else if (checkbox) {
        int va = q->style().styleHint(QStyle::SH_GroupBox_TextLabelVerticalAlignment, q);
        if(va & AlignVCenter)
            topMargin = checkbox->height()/2;
        else if(va & AlignTop)
            topMargin = fm.ascent();
    }

    int marg = bFlat ? 2 : 8;
    marg += q->fontMetrics().width(QChar(' '));
    q->setContentsMargins(marg, d->topMargin + marg, marg, marg);
}



/*! \reimp
 */
void QGroupBox::focusInEvent(QFocusEvent *)
{ // note no call to super
    d->fixFocus();
}


/*!
  \reimp
*/

QSize QGroupBox::sizeHint() const
{
    const_cast<QGroupBoxPrivate*>(d)->calculateFrame();
    return QWidget::sizeHint();
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
    return d->bFlat;
}

void QGroupBox::setFlat(bool b)
{
    if ((bool)d->bFlat == b)
        return;
    d->bFlat = b;
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
    if ((d->checkbox != 0) == b)
        return;

    if (b) {
        if (!d->checkbox) {
            d->checkbox = new QCheckBox(title(), this, "qt_groupbox_checkbox");
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
    d->calculateFrame();
    update();
}

bool QGroupBox::isCheckable() const
{
    return (d->checkbox != 0);
}


bool QGroupBox::isChecked() const
{
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
    if (d->checkbox)
        d->checkbox->setChecked(b);
}

/*
  sets all children of the group box except the qt_groupbox_checkbox
  to either disabled/enabled
*/
void QGroupBoxPrivate::setChildrenEnabled(bool b)
{
    QObjectList childs = q->children();
    if (childs.isEmpty())
        return;
    for (int i = 0; i < childs.size(); ++i) {
        QObject *o = childs.at(i);
        if (o->isWidgetType()
             && o != d->checkbox
           ) {
            QWidget *w = static_cast<QWidget *>(o);
            if (b) {
                if (!w->testAttribute(WA_ForceDisabled))
                    w->setEnabled(true);
            } else {
                if (w->isEnabled()) {
                    w->setEnabled(false);
                    w->setAttribute(WA_ForceDisabled, false);
                }
            }
        }
    }
}

/*! \reimp */
void QGroupBox::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::EnabledChange) {
        if (!d->checkbox || !isEnabled())
            return;

        // we are being enabled - disable children
        if (!d->checkbox->isChecked())
            d->setChildrenEnabled(false);
    } else if(ev->type() == QEvent::FontChange) {
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
    if (d->checkbox) {
        QSize cbSize = d->checkbox->sizeHint();
        QRect cbRect(0, 0, cbSize.width(), cbSize.height());
        QRect frameRect = q->rect();
        frameRect.setTop(topMargin);

        int marg = bFlat ? 2 : 8;
        marg += q->fontMetrics().width(QChar(' '));

        if (align & AlignHCenter) {
            cbRect.moveCenter(frameRect.center());
            cbRect.moveTop(0);
        } else if (align & AlignRight) {
            cbRect.moveRight(frameRect.right() - marg);
        } else if (align & AlignLeft) {
            cbRect.moveLeft(frameRect.left() + marg);
        } else { // auto align
            if(QApplication::reverseLayout())
                cbRect.moveRight(frameRect.right() - marg);
            else
                cbRect.moveLeft(frameRect.left() + marg);
        }

        d->checkbox->setGeometry(cbRect);
    }
}

#ifdef QT_COMPAT
QGroupBox::QGroupBox(QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    setObjectName(name);
    d->init();
}

QGroupBox::QGroupBox(const QString &title, QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    setObjectName(name);
    d->init();
    setTitle(title);
}
#endif QT_COMPAT

#include "moc_qgroupbox.cpp"

#endif //QT_NO_GROUPBOX
