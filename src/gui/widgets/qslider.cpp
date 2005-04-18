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

#include "qslider.h"
#ifndef QT_NO_SLIDER
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include "qapplication.h"
#include "qevent.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "private/qabstractslider_p.h"
#include "qdebug.h"

static const int thresholdTime = 300;
static const int repeatTime = 100;

class QSliderPrivate : public QAbstractSliderPrivate
{
    Q_DECLARE_PUBLIC(QSlider)
public:
    QStyle::SubControl pressedControl;
    int tickInterval;
    QSlider::TickPosition tickPosition;
    int clickOffset;
    int snapBackPosition;
    void init();
    int pixelPosToRangeValue(int pos) const;
    inline int pick(const QPoint &pt) const;
    QStyleOptionSlider getStyleOption() const;

    QStyle::SubControl newHoverControl(const QPoint &pos);
    bool updateHoverControl(const QPoint &pos);
    QStyle::SubControl hoverControl;
    QRect hoverRect;
};

void QSliderPrivate::init()
{
    Q_Q(QSlider);
    pressedControl = QStyle::SC_None;
    tickInterval = 0;
    tickPosition = QSlider::NoTicks;
    hoverControl = QStyle::SC_None;
    q->setFocusPolicy(Qt::WheelFocus);
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (orientation == Qt::Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}


int QSliderPrivate::pixelPosToRangeValue(int pos) const
{
    Q_Q(const QSlider);
    QStyleOptionSlider opt = getStyleOption();
    QRect gr = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, q);
    QRect sr = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, q);
    int sliderMin, sliderMax, sliderLength;

    if (orientation == Qt::Horizontal) {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    } else {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::sliderValueFromPosition(minimum, maximum, pos - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

inline int QSliderPrivate::pick(const QPoint &pt) const
{
    return orientation == Qt::Horizontal ? pt.x() : pt.y();
}


QStyleOptionSlider QSliderPrivate::getStyleOption() const
{
    Q_Q(const QSlider);
    QStyleOptionSlider opt;
    opt.init(q);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation;
    opt.maximum = maximum;
    opt.minimum = minimum;
    opt.tickPosition = (QSlider::TickPosition)tickPosition;
    opt.tickInterval = tickInterval;
    opt.upsideDown = (orientation == Qt::Horizontal) ?
                     (invertedAppearance ^  (opt.direction == Qt::RightToLeft))
                     : (!invertedAppearance);
    opt.direction = Qt::LeftToRight; // we use the upsideDown option instead
    opt.sliderPosition = position;
    opt.sliderValue = value;
    opt.singleStep = singleStep;
    opt.pageStep = pageStep;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    return opt;
}

bool QSliderPrivate::updateHoverControl(const QPoint &pos)
{
    Q_Q(QSlider);
    QRect lastHoverRect = hoverRect;
    QStyle::SubControl lastHoverControl = hoverControl;
    bool doesHover = q->testAttribute(Qt::WA_Hover);
    if (lastHoverControl != newHoverControl(pos) && doesHover) {
        q->update(lastHoverRect);
        q->update(hoverRect);
        return true;
    }
    return !doesHover;
}

QStyle::SubControl QSliderPrivate::newHoverControl(const QPoint &pos)
{
    Q_Q(QSlider);
    QStyleOptionSlider opt = getStyleOption();
    opt.subControls = QStyle::SC_All;
    QRect handleRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, q);
    QRect grooveRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, q);
    QRect tickmarksRect = q->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderTickmarks, q);

    if (handleRect.contains(pos)) {
        hoverRect = handleRect;
        hoverControl = QStyle::SC_SliderHandle;
    } else if (grooveRect.contains(pos)) {
        hoverRect = grooveRect;
        hoverControl = QStyle::SC_SliderGroove;
    } else if (tickmarksRect.contains(pos)) {
        hoverRect = tickmarksRect;
        hoverControl = QStyle::SC_SliderTickmarks;
    } else {
        hoverRect = QRect();
        hoverControl = QStyle::SC_None;
    }

    return hoverControl;
}

/*!
    \class QSlider
    \brief The QSlider widget provides a vertical or horizontal slider.

    \ingroup basic
    \mainclass

    The slider is the classic widget for controlling a bounded value.
    It lets the user move a slider handle along a horizontal or vertical
    groove and translates the handle's position into an integer value
    within the legal range.

    QSlider has very few of its own functions; most of the functionality is in
    QAbstractSlider. The most useful functions are setValue() to set
    the slider directly to some value; triggerAction() to simulate
    the effects of clicking (useful for shortcut keys);
    setSingleStep(), setPageStep() to set the steps; and setMinimum()
    and setMaximum() to define the range of the scroll bar.

    QSlider provides methods for controlling tickmarks.
    You can use setTickPosition() to indicate where
    you want the tickmarks to be, setTickInterval() to indicate how
    many of them you want.

    QSlider inherits a comprehensive set of signals:
    \table
    \header \i Signal \i Emitted when
    \row \i \l valueChanged()
    \i the slider's value has changed. The tracking()
       determines whether this signal is emitted during user
       interaction.
    \row \i \l sliderPressed()
    \i the user starts to drag the slider.
    \row \i \l sliderMoved()
    \i the user drags the slider.
    \row \i \l sliderReleased()
    \i the user releases the slider.
    \endtable

    QSlider only provides integer ranges. Note that although
    QSlider handles very large numbers, it becomes difficult for users
    to use a slider accurately for very large ranges.

    A slider accepts focus on Tab and provides both a mouse wheel and a
    keyboard interface. The keyboard interface is the following:

    \list
        \i Left/Right move a horizontal slider by one single step.
        \i Up/Down move a vertical slider by one single step.
        \i PageUp moves up one page.
        \i PageDown moves down one page.
        \i Home moves to the start (mininum).
        \i End moves to the end (maximum).
    \endlist

    \image macintosh-slider.png Screenshot in Macintosh style

    \sa QScrollBar QSpinBox
    \link guibooks.html#fowler GUI Design Handbook: Slider\endlink
*/


/*!
    \enum QSlider::TickPosition

    This enum specifies where the tick marks are to be drawn relative
    to the slider's groove and the handle the user moves.

    \value NoTicks do not draw any tick marks.
    \value TicksBothSides draw tick marks on both sides of the groove.
    \value TicksAbove draw tick marks above the (horizontal) slider
    \value TicksBelow draw tick marks below the (horizontal) slider
    \value TicksLeft draw tick marks to the left of the (vertical) slider
    \value TicksRight draw tick marks to the right of the (vertical) slider

    \omitvalue NoMarks
    \omitvalue Above
    \omitvalue Left
    \omitvalue Below
    \omitvalue Right
    \omitvalue Both
*/


/*!
    Constructs a vertical slider.

    The \a parent argument is sent to the QAbstractSlider
    constructor.
*/
QSlider::QSlider(QWidget *parent)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    d_func()->orientation = Qt::Vertical;
    d_func()->init();
}

/*!
    Constructs a slider.

    The \a orientation must be \l Qt::Vertical or \l Qt::Horizontal.

    The \a parent argument is sent on to the QAbstractSlider
    constructor.
*/

QSlider::QSlider(Qt::Orientation orientation, QWidget *parent)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    d_func()->orientation = orientation;
    d_func()->init();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSlider::QSlider(QWidget *parent, const char *name)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    setObjectName(name);
    d_func()->orientation = Qt::Vertical;
    d_func()->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSlider::QSlider(Qt::Orientation orientation, QWidget *parent, const char *name)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    setObjectName(name);
    d_func()->orientation = orientation;
    d_func()->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QSlider::QSlider(int minValue, int maxValue, int pageStep, int value, Qt::Orientation orientation,
                 QWidget *parent, const char *name)
    : QAbstractSlider(*new QSliderPrivate, parent)
{
    Q_D(QSlider);
    setObjectName(name);
    d->minimum = minValue;
    d->maximum = maxValue;
    d->pageStep = pageStep;
    d->position = d->value = value;
    d->orientation = orientation;
    d->init();
}
#endif

/*!
    Destructor.
*/
QSlider::~QSlider()
{
}

/*!
    \reimp
*/
void QSlider::paintEvent(QPaintEvent *)
{
    Q_D(QSlider);
    QPainter p(this);
    QStyleOptionSlider opt = d->getStyleOption();

    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
    if (d->tickPosition != NoTicks)
        opt.subControls |= QStyle::SC_SliderTickmarks;
    if (d->pressedControl) {
        opt.activeSubControls = d->pressedControl;
        opt.state |= QStyle::State_Sunken;
    } else {
        opt.activeSubControls = d->hoverControl;
    }

    style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
}

/*!
    \reimp
*/

bool QSlider::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
    if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
        d_func()->updateHoverControl(he->pos());
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

/*!
    \reimp
*/
void QSlider::mousePressEvent(QMouseEvent *ev)
{
    Q_D(QSlider);
    if (d->maximum == d->minimum
        || (ev->buttons() ^ ev->button())
        || (ev->button() != Qt::LeftButton)) {
        ev->ignore();
        return;
    }
    ev->accept();
    QStyleOptionSlider opt = d->getStyleOption();
    d->pressedControl = style()->hitTestComplexControl(QStyle::CC_Slider, &opt, ev->pos(), this);
    SliderAction action = SliderNoAction;
    if (d->pressedControl == QStyle::SC_SliderGroove) {
        int pressValue = d->pixelPosToRangeValue(d->pick(ev->pos()));
        if (pressValue > d->value)
            action = SliderPageStepAdd;
        else if (pressValue < d->value)
            action = SliderPageStepSub;
        if (action) {
            triggerAction(action);
            setRepeatAction(action);
        }
    } else if (d->pressedControl == QStyle::SC_SliderHandle) {
        setRepeatAction(SliderNoAction);
        QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        d->clickOffset = d->pick(ev->pos() - sr.topLeft());
        d->snapBackPosition = d->position;
        update(sr);
    }
}

/*!
    \reimp
*/
void QSlider::mouseMoveEvent(QMouseEvent *ev)
{
    Q_D(QSlider);
    if (d->pressedControl != QStyle::SC_SliderHandle || !(ev->buttons() & Qt::LeftButton)) {
        ev->ignore();
        return;
    }
    ev->accept();
    int newPosition = d->pixelPosToRangeValue(d->pick(ev->pos()) - d->clickOffset);
    QStyleOptionSlider opt = d->getStyleOption();
    int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
    if (m >= 0) {
        QRect r = rect();
        r.adjust(-m, -m, m, m);
        if (!r.contains(ev->pos()))
            newPosition = d->snapBackPosition;
    }
    setSliderPosition(newPosition);
}


/*!
    \reimp
*/
void QSlider::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_D(QSlider);
    if (d->pressedControl == QStyle::SC_None || ev->buttons()) {
        ev->ignore();
        return;
    }
    ev->accept();
    QStyle::SubControl oldPressed = QStyle::SubControl(d->pressedControl);
    d->pressedControl = QStyle::SC_None;
    setRepeatAction(SliderNoAction);
    if (oldPressed == QStyle::SC_SliderHandle)
        setSliderDown(false);
    QStyleOptionSlider opt = d->getStyleOption();
    opt.subControls = oldPressed;
    update(style()->subControlRect(QStyle::CC_Slider, &opt, oldPressed, this));
}

/*!
    \reimp
*/
QSize QSlider::sizeHint() const
{
    Q_D(const QSlider);
    ensurePolished();
    const int SliderLength = 84, TickSpace = 5;
    QStyleOptionSlider opt = d->getStyleOption();
    int thick = style()->pixelMetric(QStyle::PM_SliderThickness, &opt, this);
    if (d->tickPosition & TicksAbove)
        thick += TickSpace;
    if (d->tickPosition & TicksBelow)
        thick += TickSpace;
    int w = thick, h = SliderLength;
    if (d->orientation == Qt::Horizontal) {
        w = SliderLength;
        h = thick;
    }
    return style()->sizeFromContents(QStyle::CT_Slider, &opt, QSize(w, h), this).expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/
QSize QSlider::minimumSizeHint() const
{
    Q_D(const QSlider);
    QSize s = sizeHint();
    QStyleOptionSlider opt = d->getStyleOption();
    int length = style()->pixelMetric(QStyle::PM_SliderLength, &opt, this);
    if (d->orientation == Qt::Horizontal)
        s.setWidth(length);
    else
        s.setHeight(length);
    return s;
}

/*!
    \property QSlider::tickPosition
    \brief the tickmark position for this slider

    The valid values are in \l{QSlider::TickPosition}. The default is
    \c NoTicks.

    \sa tickInterval
*/

void QSlider::setTickPosition(TickPosition position)
{
    d_func()->tickPosition = position;
    update();
}

QSlider::TickPosition QSlider::tickPosition() const
{
    return d_func()->tickPosition;
}

/*!
    \fn TickPosition QSlider::tickmarks() const
    \compat

    Use tickPosition() instead.
*/

/*!
    \fn QSlider::setTickmarks(TickPosition position)
    \compat

    Use setTickPosition() instead.
*/

/*!
    \property QSlider::tickInterval
    \brief the interval between tickmarks

    This is a value interval, not a pixel interval. If it is 0, the
    slider will choose between lineStep() and pageStep(). The initial
    value of tickInterval is 0.

    \sa tickPosition, lineStep(), pageStep()
*/

void QSlider::setTickInterval(int ts)
{
    d_func()->tickInterval = qMax(0, ts);
    update();
}

int QSlider::tickInterval() const
{
    return d_func()->tickInterval;
}


/*!
    \fn void QSlider::addStep()

    Use setValue() instead.
*/

/*!
    \fn void QSlider::subtractStep()

    Use setValue() instead.
*/


#endif
