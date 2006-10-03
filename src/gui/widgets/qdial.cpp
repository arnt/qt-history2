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

#include "qdial.h"

#ifndef QT_NO_DIAL

#include <qapplication.h>
#include <qbitmap.h>
#include <qcolor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpolygon.h>
#include <qregion.h>
#include <qstyle.h>
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <qslider.h>
#include <private/qabstractslider_p.h>
#include <private/qmath_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

class QDialPrivate : public QAbstractSliderPrivate
{
    Q_DECLARE_PUBLIC(QDial)
public:
    QDialPrivate()
    {
        wrapping = false;
        tracking = true;
        doNotEmit = false;
        target = 3.7;
    }

    qreal target;
    uint showNotches : 1;
    uint wrapping : 1;
    uint doNotEmit : 1;

    int valueFromPoint(const QPoint &) const;
    double angle(const QPoint &, const QPoint &) const;
    void init();
    QStyleOptionSlider getStyleOption() const;
};

void QDialPrivate::init()
{
    Q_Q(QDial);
    showNotches = false;
    q->setFocusPolicy(Qt::WheelFocus);
#ifdef QT3_SUPPORT
    QObject::connect(q, SIGNAL(sliderPressed()), q, SIGNAL(dialPressed()));
    QObject::connect(q, SIGNAL(sliderMoved(int)), q, SIGNAL(dialMoved(int)));
    QObject::connect(q, SIGNAL(sliderReleased()), q, SIGNAL(dialReleased()));
#endif
}

QStyleOptionSlider QDialPrivate::getStyleOption() const
{
    Q_Q(const QDial);
    QStyleOptionSlider opt;
    opt.init(q);
    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.sliderPosition = position;
    opt.sliderValue = value;
    opt.singleStep = singleStep;
    opt.pageStep = pageStep;
    opt.upsideDown = !invertedAppearance;
    opt.notchTarget = target;
    opt.dialWrapping = wrapping;
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_None;
    if (!showNotches) {
        opt.subControls &= ~QStyle::SC_DialTickmarks;
        opt.tickPosition = QSlider::TicksAbove;
    } else {
        opt.tickPosition = QSlider::NoTicks;
    }
    opt.tickInterval = q->notchSize();
    return opt;
}

int QDialPrivate::valueFromPoint(const QPoint &p) const
{
    Q_Q(const QDial);
    double yy = (double)q->height()/2.0 - p.y();
    double xx = (double)p.x() - q->width()/2.0;
    double a = (xx || yy) ? atan2(yy, xx) : 0;

    if (a < Q_PI / -2)
        a = a + Q_PI * 2;

    int dist = 0;
    int minv = minimum, maxv = maximum;

    if (minimum < 0) {
        dist = -minimum;
        minv = 0;
        maxv = maximum + dist;
    }

    int r = maxv - minv;
    int v;
    if (wrapping)
        v =  (int)(0.5 + minv + r * (Q_PI * 3 / 2 - a) / (2 * Q_PI));
    else
        v =  (int)(0.5 + minv + r* (Q_PI * 4 / 3 - a) / (Q_PI * 10 / 6));

    if (dist > 0)
        v -= dist;

    return !invertedAppearance ? bound(v) : maximum - bound(v);
}

/*!
    \class QDial qdial.h

    \brief The QDial class provides a rounded range control (like a speedometer or potentiometer).

    \ingroup basic
    \mainclass

    QDial is used when the user needs to control a value within a
    program-definable range, and the range either wraps around
    (for example, with angles measured from 0 to 359 degrees) or the
    dialog layout needs a square widget.

    Since QDial inherits from QAbstractSlider, the dial behaves in
    a similar way to a \l{QSlider}{slider}. When wrapping() is false
    (the default setting) there is no real difference between a slider
    and a dial. They both share the same signals, slots and member
    functions. Which one you use depends on the expectations of
    your users and on the type of application.

    The dial initially emits valueChanged() signals continuously while
    the slider is being moved; you can make it emit the signal less
    often by disabling the \l tracking property. The sliderMoved() signal
    is emitted continuously even when tracking is disabled.

    The dial also emits sliderPressed() and sliderReleased() signals
    when the mouse button is pressed and released. Note that the
    dial's value can change without these signals being emitted since
    the keyboard and wheel can also be used to change the value.

    Unlike the slider, QDial attempts to draw a "nice" number of
    notches rather than one per line step. If possible, the number of
    notches drawn is one per line step, but if there aren't enough pixels
    to draw every one, QDial will skip notches to try and draw a uniform
    set (e.g. by drawing every second or third notch).

    Like the slider, the dial makes the QAbstractSlider functions
    setValue(), addLine(), subtractLine(), addPage() and
    subtractPage() available as slots.

    The dial's keyboard interface is fairly simple: The \key{left}/\key{up}
    and \key{right}/\key{down} arrow keys adjust the dial's \l value by the
    defined \l singleStep, \key{Page Up} and \key{Page Down} by the defined
    \l pageStep, and the \key Home and \key End keys set the value to the
    defined \l minimum and \l maximum values.

    \table
    \row \o \inlineimage plastique-dial.png Screenshot of a dial in the Plastique widget style
    \o \inlineimage windowsxp-dial.png Screenshot of a dial in the Windows XP widget style
    \o \inlineimage macintosh-dial.png Screenshot of a dial in the Macintosh widget style
    \row \o {3,1} Dials shown in various widget styles (from left to right):
         \l{Plastique Style Widget Gallery}{Plastique},
         \l{Windows XP Style Widget Gallery}{Windows XP},
         \l{Macintosh Style Widget Gallery}{Macintosh}.
    \endtable

    \sa QScrollBar, QSpinBox, QSlider, {fowler}{GUI Design Handbook: Slider}, {Sliders Example}
*/

/*!
    Constructs a dial.

    The \a parent argument is sent to the QAbstractSlider constructor.
*/
QDial::QDial(QWidget *parent)
    : QAbstractSlider(*new QDialPrivate, parent)
{
    Q_D(QDial);
    d->init();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QDial::QDial(QWidget *parent, const char *name)
    : QAbstractSlider(*new QDialPrivate, parent)
{
    Q_D(QDial);
    setObjectName(QString::fromAscii(name));
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QDial::QDial(int minValue, int maxValue, int pageStep, int value,
              QWidget *parent, const char *name)
    : QAbstractSlider(*new QDialPrivate, parent)
{
    Q_D(QDial);
    setObjectName(QString::fromAscii(name));
    d->minimum = minValue;
    d->maximum = maxValue;
    d->pageStep = pageStep;
    d->position = d->value = value;
    d->init();
}
#endif
/*!
    Destroys the dial.
*/
QDial::~QDial()
{
}

/*! \reimp */
void QDial::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

/*!
  \reimp
*/

void QDial::paintEvent(QPaintEvent *)
{
    Q_D(QDial);
    QStylePainter p(this);
    p.drawComplexControl(QStyle::CC_Dial, d->getStyleOption());
}

/*!
  \reimp
*/

void QDial::mousePressEvent(QMouseEvent *e)
{
    Q_D(QDial);
    if (d->maximum == d->minimum ||
        (e->button() != Qt::LeftButton)  ||
        (e->buttons() ^ e->button())) {
        e->ignore();
        return;
    }
    e->accept();
    setSliderPosition(d->valueFromPoint(e->pos()));
    // ### This isn't quite right,
    // we should be doing a hit test and only setting this if it's
    // the actual dial thingie (similar to what QSlider does), but we have no
    // subControls for QDial.
    setSliderDown(true);
}


/*!
  \reimp
*/

void QDial::mouseReleaseEvent(QMouseEvent * e)
{
    Q_D(QDial);
    if (e->buttons() & (~e->button())) {
        e->ignore();
        return;
    }
    e->accept();
    setValue(d->valueFromPoint(e->pos()));
    setSliderDown(false);
}


/*!
  \reimp
*/

void QDial::mouseMoveEvent(QMouseEvent * e)
{
    Q_D(QDial);
    if (!d->tracking || !(e->buttons() & Qt::LeftButton)) {
        e->ignore();
        return;
    }
    e->accept();
    d->doNotEmit = true;
    setSliderPosition(d->valueFromPoint(e->pos()));
    emit sliderMoved(d->value);
    d->doNotEmit = false;
}


/*!
    \reimp
*/

void QDial::sliderChange(SliderChange change)
{
    QAbstractSlider::sliderChange(change);
}

void QDial::setWrapping(bool enable)
{
    Q_D(QDial);
    if (d->wrapping == enable)
        return;
    d->wrapping = enable;
    update();
}


/*!
    \property QDial::wrapping
    \brief whether wrapping is enabled

    If true, wrapping is enabled; otherwise some space is inserted at the bottom
    of the dial to separate the ends of the range of vaild values.

    If enabled, the arrow can be oriented at any angle on the dial. If disabled,
    the arrow will be restricted to the upper part of the dial; if it is rotated
    into the space at the bottom of the dial, it will be clamped to the closest
    end of the valid range of values.

    By default this property is false.
*/

bool QDial::wrapping() const
{
    Q_D(const QDial);
    return d->wrapping;
}


/*!
    \property QDial::notchSize
    \brief the current notch size

    The notch size is in range control units, not pixels, and if
    possible it is a multiple of lineStep() that results in an
    on-screen notch size near notchTarget().

    \sa notchTarget, lineStep()
*/

int QDial::notchSize() const
{
    Q_D(const QDial);
    // radius of the arc
    int r = qMin(width(), height())/2;
    // length of the whole arc
    int l = (int)(r * (d->wrapping ? 6 : 5) * Q_PI / 6);
    // length of the arc from minValue() to minValue()+pageStep()
    if (d->maximum > d->minimum + d->pageStep)
        l = (int)(0.5 + l * d->pageStep / (d->maximum - d->minimum));
    // length of a singleStep arc
    l = l * d->singleStep / (d->pageStep ? d->pageStep : 1);
    if (l < 1)
        l = 1;
    // how many times singleStep can be draw in d->target pixels
    l = (int)(0.5 + d->target / l);
    // we want notchSize() to be a non-zero multiple of lineStep()
    if (!l)
        l = 1;
    return d->singleStep * l;
}

void QDial::setNotchTarget(double target)
{
    Q_D(QDial);
    d->target = target;
    update();
}

/*!
    \property QDial::notchTarget
    \brief the target number of pixels between notches

    The notch target is the number of pixels QDial attempts to put
    between each notch.

    The actual size may differ from the target size.

    The default notch target is 3.7 pixels.
*/
qreal QDial::notchTarget() const
{
    Q_D(const QDial);
    return d->target;
}


void QDial::setNotchesVisible(bool visible)
{
    Q_D(QDial);
    d->showNotches = visible;
    update();
}

/*!
    \property QDial::notchesVisible
    \brief whether the notches are shown

    If the property is true, a series of notches are drawn around the dial
    to indicate the range of values available; otherwise no notches are
    shown.

    By default, this property is disabled.
*/
bool QDial::notchesVisible() const
{
    Q_D(const QDial);
    return d->showNotches;
}

/*!
  \reimp
*/

QSize QDial::minimumSizeHint() const
{
    return QSize(50, 50);
}

/*!
  \reimp
*/

QSize QDial::sizeHint() const
{
    return QSize(100, 100).expandedTo(QApplication::globalStrut());
}

/*!
  \reimp
*/
bool QDial::event(QEvent *e)
{
    return QAbstractSlider::event(e);
}

/*!
    \fn void QDial::dialPressed();

    Use QAbstractSlider::sliderPressed() instead.
*/

/*!
    \fn void QDial::dialMoved(int value);

    Use QAbstractSlider::sliderMoved() instead.
*/

/*!
    \fn void QDial::dialReleased();

    Use QAbstractSlider::sliderReleased() instead.
*/


#endif // QT_NO_DIAL
