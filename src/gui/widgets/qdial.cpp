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
        opt.tickInterval = q->notchSize();
    } else {
        opt.tickPosition = QSlider::NoTicks;
        opt.tickInterval = 0;
    }
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

    return bound(v);
}

/*!
    \class QDial qdial.h

    \brief The QDial class provides a rounded range control (like a speedometer or potentiometer).

    \ingroup basic
    \mainclass

    QDial is used when the user needs to control a value within a
    program-definable range, and the range either wraps around
    (typically, 0..359 degrees) or the dialog layout needs a square
    widget.

    Both API- and UI-wise, the dial is very similar to a \link QSlider
    slider. \endlink Indeed, when wrapping() is false (the default)
    there is no real difference between a slider and a dial. They
    have the same signals, slots and member functions, all of which do
    the same things. Which one you use depends only on your taste
    and on the application.

    The dial initially emits valueChanged() signals continuously while
    the slider is being moved; you can make it emit the signal less
    often by calling setTracking(false). dialMoved() is emitted
    continuously even when tracking() is false.

    The slider also emits dialPressed() and dialReleased() signals
    when the mouse button is pressed and released. But note that the
    dial's value can change without these signals being emitted; the
    keyboard and wheel can be used to change the value.

    Unlike the slider, QDial attempts to draw a "nice" number of
    notches rather than one per lineStep(). If possible, the number
    of notches drawn is one per lineStep(), but if there aren't enough
    pixels to draw every one, QDial will draw every second, third
    etc., notch. notchSize() returns the number of units per notch,
    hopefully a multiple of lineStep(); setNotchTarget() sets the
    target distance between neighbouring notches in pixels. The
    default is 3.75 pixels.

    Like the slider, the dial makes the QAbstractSlider functions
    setValue(), addLine(), subtractLine(), addPage() and
    subtractPage() available as slots.

    The dial's keyboard interface is fairly simple: The left/up and
    right/down arrow keys move by lineStep(), page up and page down by
    pageStep() and Home and End to minValue() and maxValue().

    \inlineimage qdial-m.png Screenshot in Motif style
    \inlineimage qdial-w.png Screenshot in Windows style

    \sa QScrollBar, QSpinBox, {fowler}{GUI Design Handbook: Slider}
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
    setObjectName(name);
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
    setObjectName(name);
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
    emit sliderPressed();
}


/*!
  \reimp
*/

void QDial::mouseReleaseEvent(QMouseEvent * e)
{
    Q_D(QDial);
    if (e->buttons() ^ e->button()) {
        e->ignore();
        return;
    }
    e->accept();
    setValue(d->valueFromPoint(e->pos()));
    emit sliderReleased();
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

    Reimplemented to ensure the display is correct and to emit the
    valueChanged(int) signal when appropriate and to ensure
    tickmarks are consistent with the new range. The \a change
    parameter indicates what type of change that has taken place.
*/

void QDial::sliderChange(SliderChange change)
{
    Q_D(QDial);
    if (change == SliderRangeChange || change == SliderValueChange) {
        update();
        if (change == SliderValueChange && (d->tracking || !d->doNotEmit)) {
            emit valueChanged(d->value);
#ifndef QT_NO_ACCESSIBILITY
            QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
        }
    }
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

    If true, wrapping is enabled. This means that the arrow can be
    turned around 360°. Otherwise there is some space at the bottom of
    the dial which is skipped by the arrow.

    This property's default is false.
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

    \sa notchTarget() lineStep()
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
    l = l * d->singleStep / d->pageStep;
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
*/
qreal QDial::notchTarget() const
{
    Q_D(const QDial);
    return d->target;
}


void QDial::setNotchesVisible(bool visible)
{
    // d->showNotches = visible;
    // update();
    // ### fix after beta2
    Q_UNUSED(visible);
}

/*!
    \property QDial::notchesVisible
    \brief whether the notches are shown

    If true, the notches are shown. If false (the default) notches are
    not shown.
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


#endif // QT_FEATURE_DIAL
