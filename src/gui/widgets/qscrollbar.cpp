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

#include "qapplication.h"
#include "qcursor.h"
#include "qevent.h"
#include "qpainter.h"
#include "qscrollbar.h"
#include "qstyle.h"
#include "qstyleoption.h"

#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include <limits.h>
#include "qabstractslider_p.h"

/*!
    \class QScrollBar
    \brief The QScrollBar widget provides a vertical or horizontal scroll bar.

    \ingroup basic

    A scroll bar allows the user to control a value within a
    program-definable range and gives users a visible indication of
    the current value.

    Scroll bars include four separate controls:

    \list

    \i The \e line-up and \e line-down controls are little buttons
    which the user can use to move one "line" up or down. The meaning
    of line is configurable. In editors and list boxes it means one
    line of text; in an image viewer it might mean 20 pixels.

    \i The \e slider is the handle that indicates the current value of
    the scroll bar, which the user can drag to change the value. This
    part of the scroll bar is sometimes called the "thumb".

    \i The \e page-up/page-down control is the area on which the
    slider slides (the scroll bar's background). Clicking here moves
    the scroll bar towards the click. The meaning of "page" is also
    configurable: in editors and list boxes it means as many lines as
    there is space for in the widget.

    \endlist

    QScrollBar has very few of its own functions; it mostly relies on
    QAbstractSlider. The most useful functions are setValue() to set
    the scroll bar directly to some value; triggerAction() to simulate
    the effects of clicking (useful for shortcut keys);
    setSingleStep(), setPageStep() to set the steps; and setMinimum()
    and setMaximum() to define the range of the scroll bar.

    Some GUI styles (for example, the Windows and Motif styles
    provided with Qt), also use the pageStep() value to calculate the
    size of the slider.

    ScrollBar inherits a comprehensive set of signals:
    \table
    \header \i Signal \i Emitted when
    \row \i \l valueChanged()
         \i the scroll bar's value has changed. The tracking()
            determines whether this signal is emitted during user
            interaction.
    \row \i \l sliderPressed()
         \i the user starts to drag the slider.
    \row \i \l sliderMoved()
         \i the user drags the slider.
    \row \i \l sliderReleased()
         \i the user releases the slider.
    \endtable

    QScrollBar only provides integer ranges. Note that although
    QScrollBar handles very large numbers, scroll bars on current
    screens cannot usefully control ranges above about 100,000 pixels.
    Beyond that, it becomes difficult for the user to control the
    scroll bar using either the keyboard or the mouse.

    A scroll bar can be controlled by the keyboard, but it has a
    default focusPolicy() of \c Qt::NoFocus. Use setFocusPolicy() to
    enable keyboard interaction with the scrollbar:
    \list
         \i Left/Right move a horizontal scrollbar by one single step.
         \i Up/Down move a vertical scrollbar by one single step.
         \i PageUp moves up one page.
         \i PageDown moves down one page.
         \i Home moves to the start (mininum).
         \i End moves to the end (maximum).
     \endlist

    If you need to add scroll bars to an interface, consider using the
    QScrollView class, which encapsulates the common uses for scroll
    bars.

    \inlineimage qscrbar-m.png Screenshot in Motif style
    \inlineimage qscrbar-w.png Screenshot in Windows style

    \sa QSlider QSpinBox QScrollView
    \link guibooks.html#fowler GUI Design Handbook: Scroll Bar\endlink
*/




class QScrollBarPrivate : public QAbstractSliderPrivate
{
    Q_DECLARE_PUBLIC(QScrollBar)
public:
    uint pressedControl;
    bool pointerLeftControl;

    int clickOffset, snapBackPosition;

    void activateControl(uint control);
    int pixelPosToRangeValue(int pos) const;
    QStyleOptionSlider getStyleOption() const;
    void init();
};
#define d d_func()
#define q q_func()

void QScrollBarPrivate::activateControl(uint control)
{
    QAbstractSlider::SliderAction action = QAbstractSlider::SliderNoAction;
    switch (control) {
    case QStyle::SC_ScrollBarAddPage:
        action = QAbstractSlider::SliderPageStepAdd;
        break;
    case QStyle::SC_ScrollBarSubPage:
        action = QAbstractSlider::SliderPageStepSub;
        break;
    case QStyle::SC_ScrollBarAddLine:
        action = QAbstractSlider::SliderSingleStepAdd;
        break;
    case QStyle::SC_ScrollBarSubLine:
        action = QAbstractSlider::SliderSingleStepSub;
        break;
    case QStyle::SC_ScrollBarFirst:
        action = QAbstractSlider::SliderToMinimum;
        break;
    case QStyle::SC_ScrollBarLast:
        action = QAbstractSlider::SliderToMaximum;
        break;
    default:
        break;
    }

    if (action) {
        q->triggerAction(action);
        q->setRepeatAction(action);
    }
}

QStyleOptionSlider QScrollBarPrivate::getStyleOption() const
{
    QStyleOptionSlider opt(0);
    opt.init(q);
    opt.parts = QStyle::SC_None;
    opt.activeParts = QStyle::SC_None;
    opt.orientation = orientation;
    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.sliderPosition = position;
    opt.sliderValue = value;
    opt.singleStep = singleStep;
    opt.pageStep = pageStep;
    opt.useRightToLeft = invertedAppearance;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::Style_Horizontal;
    return opt;
}


#define HORIZONTAL        (d->orientation == Qt::Horizontal)
#define VERTICAL        !HORIZONTAL

/*!
    Constructs a vertical scroll bar.

    The \a parent arguments is sent to the QWidget constructor.

    The \l minimum defaults to 0, the \l maximum to 99, with a \l
    singleStep size of 1 and a \l pageStep size of 10, and an initial
    \l value of 0.
*/
QScrollBar::QScrollBar(QWidget *parent)
    : QAbstractSlider(*new QScrollBarPrivate, parent)
{
    d->orientation = Qt::Vertical;
    d->init();
}

/*!
    Constructs a scroll bar with the given \a orientation.

    The \a parent argument is passed to the QWidget constructor.

    The \l minimum defaults to 0, the \l maximum to 99, with a \l
    singleStep size of 1 and a \l pageStep size of 10, and an initial
    \l value of 0.
*/
QScrollBar::QScrollBar(Qt::Orientation orientation, QWidget *parent)
    : QAbstractSlider(*new QScrollBarPrivate, parent)
{
    d->orientation = orientation;
    d->init();
}


#ifdef QT_COMPAT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QScrollBar::QScrollBar(QWidget *parent, const char *name)
    : QAbstractSlider(*new QScrollBarPrivate,  parent)
{
    setObjectName(name);
    d->orientation = Qt::Vertical;
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QScrollBar::QScrollBar(Qt::Orientation orientation, QWidget *parent, const char *name)
    : QAbstractSlider(*new QScrollBarPrivate,  parent)
{
    setObjectName(name);
    d->orientation = orientation;
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QScrollBar::QScrollBar(int minimum, int maximum, int lineStep, int pageStep,
                        int value,  Qt::Orientation orientation,
                        QWidget *parent, const char *name)
    : QAbstractSlider(*new QScrollBarPrivate,  parent)
{
    setObjectName(name);
    d->minimum = minimum;
    d->maximum = maximum;
    d->singleStep = lineStep;
    d->pageStep = pageStep;
    d->value = value;
    d->orientation = orientation;
    d->init();
}
#endif // QT_COMPAT

/*!
    Destroys the scroll bar.
*/
QScrollBar::~QScrollBar()
{
}

void QScrollBarPrivate::init()
{
    invertedControls = true;
    pressedControl = QStyle::SC_None;
    pointerLeftControl = false;
    q->setFocusPolicy(Qt::NoFocus);
    QStyleOptionSlider opt = d->getStyleOption();
    q->setBackgroundRole((QPalette::ColorRole)
                         q->style().styleHint(QStyle::SH_ScrollBar_BackgroundRole, &opt, q));
    QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Fixed);
    if (orientation == Qt::Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->clearWState(Qt::WState_OwnSizePolicy);
}



/*! \reimp */
QSize QScrollBar::sizeHint() const
{
    ensurePolished();
    int sbextent = style().pixelMetric(QStyle::PM_ScrollBarExtent, this);
    if (d->orientation == Qt::Horizontal)
        return QSize(30, sbextent);
    else
        return QSize(sbextent, 30);
}

/*!\reimp */
void QScrollBar::sliderChange(SliderChange change)
{
    if (change == SliderValueChange && repeatAction()) {
       QStyleOptionSlider opt = d->getStyleOption();
        if((d->pressedControl == QStyle::SC_ScrollBarAddPage
            || d->pressedControl == QStyle::SC_ScrollBarSubPage)
           && style().styleHint(QStyle::SH_ScrollBar_StopMouseOverSlider, 0, this)
           && style().querySubControl(QStyle::CC_ScrollBar, &opt, mapFromGlobal(QCursor::pos()),
                                   this) == QStyle::SC_ScrollBarSlider) {
            setRepeatAction(SliderNoAction);
        }
    }
    QAbstractSlider::sliderChange(change);
}

/*!
    \reimp
*/
void QScrollBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionSlider opt = d->getStyleOption();
    opt.parts = QStyle::SC_All;
    if (!d->pointerLeftControl)
        opt.activeParts = (QStyle::SubControl)d->pressedControl;
    style().drawComplexControl(QStyle::CC_ScrollBar, &opt, &p, this);
}

/*!
    \reimp
*/
void QScrollBar::mousePressEvent(QMouseEvent *e)
{
    bool midButtonAbsPos = style().styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition,
                                             0, this);
    QStyleOptionSlider opt = d->getStyleOption();

    if (d->maximum == d->minimum // no range
        || (e->state() & Qt::MouseButtonMask) // another button was clicked before
        || !(e->button() == Qt::LeftButton || (midButtonAbsPos && e->button() == Qt::MidButton)))
        return;

    d->pressedControl = style().querySubControl(QStyle::CC_ScrollBar, &opt, e->pos(), this);
    d->pointerLeftControl = false;

    QRect sr = style().querySubControlMetrics(QStyle::CC_ScrollBar, &opt,
                                              QStyle::SC_ScrollBarSlider, this);
    QPoint click = QStyle::visualPos(e->pos(), this);
    if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
        d->clickOffset = (QCOORD)((HORIZONTAL ? (click.x()-sr.x()) : (click.y()-sr.y())));
        d->snapBackPosition = d->position;
    }

    if ((d->pressedControl == QStyle::SC_ScrollBarAddPage
          || d->pressedControl == QStyle::SC_ScrollBarSubPage
          || d->pressedControl == QStyle::SC_ScrollBarSlider)
        && ((midButtonAbsPos && e->button() == Qt::MidButton)
            || style().styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition, &opt, this)
            && e->button() == Qt::LeftButton)) {
        int sliderLength = HORIZONTAL ? sr.width() : sr.height();
        setSliderPosition(d->pixelPosToRangeValue((HORIZONTAL ? e->pos().x()
                                                              : e->pos().y()) - sliderLength / 2));
        d->pressedControl = QStyle::SC_ScrollBarSlider;
        d->clickOffset = sliderLength / 2;
    }
    d->activateControl(d->pressedControl);
}


/*!
    \reimp
*/
void QScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (!d->pressedControl)
        return;

    if (e->stateAfter() & Qt::MouseButtonMask) // some other button is still pressed
        return;

    QStyle::SubControl tmp = (QStyle::SubControl) d->pressedControl;
    setRepeatAction(SliderNoAction);
    d->pressedControl = QStyle::SC_None;
    if (tmp == QStyle::SC_ScrollBarSlider)
        setSliderDown(false);
    QStyleOptionSlider opt = d->getStyleOption();
    repaint(QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_ScrollBar, &opt, tmp,
                                                              this), this));
}


/*!
    \reimp
*/
void QScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    if (!d->pressedControl)
        return;

    QStyleOptionSlider opt = d->getStyleOption();
    if (!(e->state() & Qt::LeftButton
          ||  ((e->state() & Qt::MidButton)
               && style().styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, &opt, this))))
        return;

    if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
        QPoint click = QStyle::visualPos(e->pos(), this);
        int newPosition = d->pixelPosToRangeValue((HORIZONTAL ? click.x() : click.y()) -d->clickOffset);
        int m = style().pixelMetric(QStyle::PM_MaximumDragDistance, this);
        if (m >= 0) {
            QRect r = rect();
            r.addCoords(-m, -m, m, m);
            if (! r.contains(e->pos()))
                newPosition = d->snapBackPosition;
        }
        setSliderPosition(newPosition);
    } else if (!style().styleHint(QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl, &opt, this)) {
        // stop scrolling when the mouse pointer leaves a control
        // similar to push buttons
        opt.parts = d->pressedControl;
        QRect pr = QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_ScrollBar,
                                                                     &opt,
                                                                     QStyle::SubControl(opt.parts),
                                                                     this), this);
        if (pr.contains(e->pos()) == d->pointerLeftControl) {
            if ((d->pointerLeftControl = !d->pointerLeftControl)) {
                setRepeatAction(SliderNoAction);
                repaint(pr);
            } else  {
                d->activateControl(d->pressedControl);
            }
        }
    }
}


int QScrollBarPrivate::pixelPosToRangeValue(int pos) const
{
    QStyleOptionSlider opt = getStyleOption();
    QRect gr = q->style().querySubControlMetrics(QStyle::CC_ScrollBar, &opt,
                                                 QStyle::SC_ScrollBarGroove, q);
    QRect sr = q->style().querySubControlMetrics(QStyle::CC_ScrollBar, &opt,
                                                 QStyle::SC_ScrollBarSlider, q);
    int sliderMin, sliderMax, sliderLength;

    if (HORIZONTAL) {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    } else {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }

    return  QStyle::valueFromPosition(d->minimum, d->maximum, pos - sliderMin,
                                      sliderMax - sliderMin, d->invertedAppearance);
}


/*! \reimp
*/
void QScrollBar::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        QStyleOptionSlider opt = d->getStyleOption();
        setBackgroundRole((QPalette::ColorRole)
                          style().styleHint(QStyle::SH_ScrollBar_BackgroundRole, &opt, this));
    }
    QWidget::changeEvent(ev);
}

/*! \reimp
*/
void QScrollBar::hideEvent(QHideEvent *)
{
    if (d->pressedControl) {
        d->pressedControl = QStyle::SC_None;
        setRepeatAction(SliderNoAction);
    }
}

/*!
    \fn bool QScrollBar::draggingSlider()

    Use isSliderDown() instead.
*/

