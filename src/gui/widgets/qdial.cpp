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

#include "qapplication.h"
#include "qbitmap.h"
#include "qcolor.h"
#include "qevent.h"
#include "qpainter.h"
#include "qpointarray.h"
#include "qregion.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include <private/qabstractslider_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

#include <math.h>

static const double m_pi = 3.14159265358979323846;
static const double rad_factor = 180.0 / m_pi;


class QDialPrivate : public QAbstractSliderPrivate
{
    Q_DECLARE_PUBLIC(QDial);
public:
    QDialPrivate()
    {
        wrapping = false;
        tracking = true;
        doNotEmit = false;
        target = 3.7;
    }

    QRect eraseArea;
    QPointArray lines;
    double target;
    uint eraseAreaValid : 1;
    uint showNotches : 1;
    uint onlyOutside : 1;
    uint wrapping : 1;
    uint doNotEmit : 1;

    int valueFromPoint(const QPoint &) const;
    double angle(const QPoint &, const QPoint &) const;
    QPointArray calcArrow(double &a) const;
    QRect calcDial() const;
    int calcBigLineSize() const;
    void calcLines();
    void init();
    void repaintScreen(const QRect &cr = QRect());
};

#define d d_func()
#define q q_func()

void QDialPrivate::init()
{
    eraseAreaValid = false;
    showNotches = false;
    onlyOutside = false;
    q->setFocusPolicy(Qt::WheelFocus);
#ifdef QT_COMPAT
    QObject::connect(q, SIGNAL(sliderPressed()), q, SIGNAL(dialPressed()));
    QObject::connect(q, SIGNAL(sliderMoved(int)), q, SIGNAL(dialMoved(int)));
    QObject::connect(q, SIGNAL(sliderReleased()), q, SIGNAL(dialReleased()));
#endif
}

void QDialPrivate::repaintScreen(const QRect &cr)
{
    QPainter p;
    p.begin(q);

    bool resetClipping = false;
    int width = q->width();
    int height = q->height();
    QRect br(calcDial());
    QRect te = br;
    te.setWidth(te.width() + 2);
    te.setHeight(te.height() + 2);

    // calculate clip-region for erasing background
    if (!cr.isEmpty()) {
        p.setClipRect(cr);
    } else if (!onlyOutside && eraseAreaValid) {
        QRegion reg = eraseArea;
        double a;
        reg = reg.subtract(calcArrow(a));
        p.setClipRegion(reg);
        resetClipping = true;
    } else if (onlyOutside) {
        QRegion eraseReg(0, 0, width, height);
        eraseReg = eraseReg.subtract(QRegion(te, QRegion::Ellipse));
        p.setClipRegion(eraseReg);
        resetClipping = true;
    }

    p.eraseRect(te);

    if (resetClipping) {
        if (!cr.isEmpty())
            p.setClipRect(cr);
        else
            p.setClipRect(QRect(0, 0, width, height));
    }
    QPalette pal = q->palette();
    // draw notches
    if (showNotches) {
        calcLines();
        p.setPen(pal.foreground());
        p.drawLineSegments(lines);
    }

    // calculate and paint arrow
    p.setPen(QPen(pal.dark()));
    p.drawArc(te, 60 * 16, 180 * 16);
    p.setPen(QPen(pal.light()));
    p.drawArc(te, 240 * 16, 180 * 16);

    double a;
    QPointArray arrow(calcArrow(a));
    QRect ea(arrow.boundingRect());
    d->eraseArea = ea;
    d->eraseAreaValid = true;

    p.setPen(Qt::NoPen);
    p.setBrush(pal.brush(QPalette::Button));
    if (!d->onlyOutside)
        p.drawPolygon(arrow);

    a = angle(QPoint(width / 2, height / 2), arrow[0]);
    p.setBrush(Qt::NoBrush);

    // that's still a hack...
    if (a <= 0 || a > 200) {
        p.setPen(pal.light());
        p.drawLine(arrow[2], arrow[0]);
        p.drawLine(arrow[1], arrow[2]);
        p.setPen(pal.dark());
        p.drawLine(arrow[0], arrow[1]);
    } else if (a > 0 && a < 45) {
        p.setPen(pal.light());
        p.drawLine(arrow[2], arrow[0]);
        p.setPen(pal.dark());
        p.drawLine(arrow[1], arrow[2]);
        p.drawLine(arrow[0], arrow[1]);
    } else if (a >= 45 && a < 135) {
        p.setPen(pal.dark());
        p.drawLine(arrow[2], arrow[0]);
        p.drawLine(arrow[1], arrow[2]);
        p.setPen(pal.light());
        p.drawLine(arrow[0], arrow[1]);
    } else if (a >= 135 && a < 200) {
        p.setPen(pal.dark());
        p.drawLine(arrow[2], arrow[0]);
        p.setPen(pal.light());
        p.drawLine(arrow[0], arrow[1]);
        p.drawLine(arrow[1], arrow[2]);
    }

    // draw focus rect around the dial
    if (q->hasFocus()) {
        p.setClipping(false);
        br.setWidth(br.width() + 2);
        br.setHeight(br.height() + 2);
        if (d->showNotches) {
            int r = qMin(width, height) / 2;
            br.moveBy(-r / 6, - r / 6);
            br.setWidth(br.width() + r / 3);
            br.setHeight(br.height() + r / 3);
        }
        // strange, but else we get redraw errors on Windows
        p.end();
        p.begin(q);
        p.save();
        p.setPen(QPen(pal.background()));
        p.setBrush(Qt::NoBrush);
        p.drawRect(br);
        p.restore();
        QStyleOptionFocusRect opt(0);
        opt.rect = br;
        opt.palette = pal;
        opt.state = QStyle::Style_Default;
        q->style().drawPrimitive(QStyle::PE_FocusRect, &opt, &p, q);
    }
    p.end();
}

QPointArray QDialPrivate::calcArrow(double &a) const
{
    int width = q->width();
    int height = q->height();
    int r = qMin(width, height) / 2;
    if (maximum == minimum)
        a = m_pi / 2;
    else if (wrapping)
        a = m_pi * 3 / 2 - (value - minimum) * 2 * m_pi / (maximum - minimum);
    else
        a = (m_pi * 8 - (value - minimum) * 10 * m_pi / (maximum - minimum)) / 6;

    int xc = width / 2;
    int yc = height / 2;

    int len = r - calcBigLineSize() - 5;
    if (len < 5)
        len = 5;
    int back = len / 4;
    if (back < 1)
        back = 1;

    QPointArray arrow(3);
    arrow[0] = QPoint((int)(0.5 + xc + len * cos(a)),
                      (int)(0.5 + yc - len * sin(a)));
    arrow[1] = QPoint((int)(0.5 + xc + back * cos(a + m_pi * 5 / 6)),
                      (int)(0.5 + yc - back * sin(a + m_pi * 5 / 6)));
    arrow[2] = QPoint((int)(0.5 + xc + back * cos(a - m_pi * 5 / 6)),
                      (int)(0.5 + yc - back * sin(a - m_pi * 5 / 6)));
    return arrow;
}

QRect QDialPrivate::calcDial() const
{
    int width = q->width();
    int height = q->height();
    double r = qMin(width, height) / 2.0;
    double d_ = r / 6.0;
    double dx = d_ + (width - 2 * r) / 2.0 + 1;
    double dy = d_ + (height - 2 * r) / 2.0 + 1;
    return QRect(int(dx), int(dy),
                 int(r * 2 - 2 * d_ - 2), int(r * 2 - 2 * d_ - 2));
}

int QDialPrivate::calcBigLineSize() const
{
    int r = qMin(q->width(), q->height()) / 2;
    int bigLineSize = r / 6;
    if (bigLineSize < 4)
        bigLineSize = 4;
    if (bigLineSize > r / 2)
        bigLineSize = r / 2;
    return bigLineSize;
}

void QDialPrivate::calcLines()
{
    if (lines.isEmpty()) {
        int width = q->width();
        int height = q->height();
        double r = qMin(width, height) / 2.0;
        int bigLineSize = calcBigLineSize();
        double xc = width / 2.0;
        double yc = height / 2.0;
        int ns = q->notchSize();
        int notches = (maximum + ns - 1 - minimum) / ns;
        d->lines.resize(2 + 2 * notches);
        int smallLineSize = bigLineSize / 2;
        for (int i = 0; i <= notches; ++i) {
            double angle = wrapping ? m_pi * 3 / 2 - i * 2 * m_pi / notches
                                    : (m_pi * 8 - i * 10 * m_pi / notches) / 6;
            double s = sin(angle);
            double c = cos(angle);
            if (i == 0 || (((ns * i) % pageStep) == 0)) {
                d->lines[2 * i] = QPoint((int)(xc + (r - bigLineSize) * c),
                                         (int)(yc - (r - bigLineSize) * s));
                d->lines[2 * i + 1] = QPoint((int)(xc + r * c), (int)(yc - r * s));
            } else {
                d->lines[2 * i] = QPoint((int)(xc + (r - 1 - smallLineSize) * c),
                                       (int)(yc - (r - 1 - smallLineSize) * s));
                d->lines[2 * i + 1] = QPoint((int)(xc + (r - 1) * c), (int)(yc -(r - 1) * s));
            }
        }
    }
}

int QDialPrivate::valueFromPoint(const QPoint &p) const
{
    double a = atan2((double)q->height() / 2.0 - p.y(),
                     (double)p.x() - q->width() / 2.0);
    if (a < m_pi / -2)
        a = a + m_pi * 2;

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
        v =  (int)(0.5 + minv + r * (m_pi * 3 / 2 - a) / (2 * m_pi));
    else
        v =  (int)(0.5 + minv + r* (m_pi * 4 / 3 - a) / (m_pi * 10 / 6));

    if (dist > 0)
        v -= dist;

    return bound(v);
}

double QDialPrivate::angle(const QPoint &p1, const QPoint &p2) const
{
    double _angle = 0.0;

    if (p1.x() == p2.x()) {
        if (p1.y() < p2.y())
            _angle = 270.0;
        else
            _angle = 90.0;
    } else  {
        double x1, x2, y1, y2;

        if (p1.x() <= p2.x()) {
            x1 = p1.x(); y1 = p1.y();
            x2 = p2.x(); y2 = p2.y();
        } else {
            x2 = p1.x(); y2 = p1.y();
            x1 = p2.x(); y1 = p2.y();
        }

        double m = -(y2 - y1) / (x2 - x1);
        _angle = atan(m) *  rad_factor;

        if (p1.x() < p2.x())
            _angle = 180.0 - _angle;
        else
            _angle = -_angle;
    }
    return _angle;
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

    Like the slider, the dial makes the QRangeControl functions
    setValue(), addLine(), subtractLine(), addPage() and
    subtractPage() available as slots.

    The dial's keyboard interface is fairly simple: The left/up and
    right/down arrow keys move by lineStep(), page up and page down by
    pageStep() and Home and End to minValue() and maxValue().

    \inlineimage qdial-m.png Screenshot in Motif style
    \inlineimage qdial-w.png Screenshot in Windows style

    \sa QScrollBar QSpinBox
    \link guibooks.html#fowler GUI Design Handbook: Slider\endlink
*/

/*!
    Constructs a dial.

    The \a parent argument is sent to the QAbstractSlider constructor.
*/
QDial::QDial(QWidget *parent)
    : QAbstractSlider(*new QDialPrivate, parent)
{
    d->init();
}

#ifdef QT_COMPAT
QDial::QDial(QWidget *parent, const char *name)
    : QAbstractSlider(*new QDialPrivate, parent)
{
    setObjectName(name);
    d->init();
}

QDial::QDial(int minValue, int maxValue, int pageStep, int value,
              QWidget *parent, const char *name)
    : QAbstractSlider(*new QDialPrivate, parent)
{
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
    d->lines.clear();
    QWidget::resizeEvent(e);
}

/*!
  \reimp
*/

void QDial::paintEvent(QPaintEvent *e)
{
    d->repaintScreen(e->rect());
}

/*!
  \reimp
*/

void QDial::mousePressEvent(QMouseEvent *e)
{
    if (d->maximum == d->minimum || (e->state() & Qt::MouseButtonMask) || (e->button() != Qt::LeftButton)) {
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
    if (e->stateAfter() & Qt::MouseButtonMask) {
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
    if (!d->tracking || !(e->state() & Qt::LeftButton)) {
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

void QDial::focusInEvent(QFocusEvent *)
{
    d->onlyOutside = true;
    repaint();
    d->onlyOutside = false;
}


/*!
  \reimp
*/

void QDial::focusOutEvent(QFocusEvent *)
{
    d->onlyOutside = true;
    repaint();
    d->onlyOutside = false;
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
    if (change == SliderRangeChange || change == SliderValueChange) {
        d->lines.clear();
        repaint();
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
    if (d->wrapping == enable)
        return;
    d->lines.clear();
    d->wrapping = enable;
    d->eraseAreaValid = false;
    update();
}


/*!
    \property QDial::wrapping
    \brief whether wrapping is enabled

    If true, wrapping is enabled. This means that the arrow can be
    turned around 360�. Otherwise there is some space at the bottom of
    the dial which is skipped by the arrow.

    This property's default is false.
*/

bool QDial::wrapping() const
{
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
    // radius of the arc
    int r = qMin(width(), height())/2;
    // length of the whole arc
    int l = (int)(r * (d->wrapping ? 6 : 5) * m_pi / 6);
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
    d->lines.resize(0);
    d->target = target;
    d->eraseAreaValid = false;
    d->onlyOutside = true;
    repaint();
    d->onlyOutside = false;
}

/*!
    \property QDial::notchTarget
    \brief the target number of pixels between notches

    The notch target is the number of pixels QDial attempts to put
    between each notch.

    The actual size may differ from the target size.
*/
double QDial::notchTarget() const
{
    return d->target;
}


void QDial::setNotchesVisible(bool visible)
{
    d->showNotches = visible;
    d->eraseAreaValid = false;
    d->onlyOutside = true;
    repaint();
    d->onlyOutside = false;
}

/*!
    \property QDial::notchesVisible
    \brief whether the notches are shown

    If true, the notches are shown. If false (the default) notches are
    not shown.
*/
bool QDial::notchesVisible() const
{
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
#endif // QT_FEATURE_DIAL
