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

    A scroll bar is a control that enables the user to access parts of a
    document that is larger than the widget used to display it. It provides
    a visual indication of the user's current position within the document
    and the amount of the document that is visible. Scroll bars are usually
    equipped with other controls that enable more accurate navigation.
    Qt displays scroll bars in a way that is appropriate for each platform.

    If you need to provide a scrolling view onto another widget, it may be
    more convenient to use the QScrollArea class because this provides a
    viewport widget and scroll bars. QScrollBar is useful if you need to
    implement similar functionality for specialized widgets using QAbstractScrollArea;
    for example, if you decide to subclass QAbstractItemView.
    For most other situations where a slider control is used to obtain a value
    within a given range, the QSlider class may be more appropriate for your
    needs.

    \table
    \row \i \image qscrollbar-picture.png
    \i Scroll bars typically include four separate controls: a slider,
    scroll arrows, and a page control.

    \list
    \i a. The slider provides a way to quickly go to any part of the
    document, but does not support accurate navigation within large
    documents.
    \i b. The scroll arrows are push buttons which can be used to accurately
    navigate to a particular place in a document. For a vertical scroll bar
    connected to a text editor, these typically move the current position one
    "line" up or down, and adjust the position of the slider by a small
    amount. In editors and list boxes a "line" might mean one line of text;
    in an image viewer it might mean 20 pixels.
    \i c. The page control is the area over which the slider is dragged (the
    scroll bar's background). Clicking here moves the scroll bar towards
    the click by one "page". This value is usually the same as the length of
    the slider.
    \endlist
    \endtable

    Each scroll bar has a value that indicates how far the slider is from
    the start of the scroll bar; this is obtained with value() and set
    with setValue(). This value always lies within the range of values
    defined for the scroll bar, from \l{QAbstractSlider::minimum()}{minimum()}
    to \l{QAbstractSlider::minimum()}{maximum()} inclusive. The range of
    acceptable values can be set with setMinimum() and setMaximum().
    At the minimum value, the top edge of the slider (for a vertical scroll
    bar) or left edge (for a horizontal scroll bar) will be at the top (or
    left) end of the scroll bar. At the maximum value, the bottom (or right)
    edge of the slider will be at the bottom (or right) end of the scroll bar.

    The length of the slider is usually related to the value of the page step,
    and typically represents the proportion of the document area shown in a
    scrolling view. The page step is the amount that the value changes by
    when the user presses the \key{Page Up} and \key{Page Down} keys, and is
    set with setPageStep(). Smaller changes to the value defined by the
    line step are made using the cursor keys, and this quantity is set with
    setLineStep().

    Note that the range of values used is independent of the actual size
    of the scroll bar widget. You do not need to take this into account when
    you choose values for the range and the page step.

    The range of values specified for the scroll bar are often determined
    differently to those for a QSlider because the length of the slider
    needs to be taken into account. If we have a document with 100 lines,
    and we can only show 20 lines in a widget, we may wish to construct a
    scroll bar with a page step of 20, a minimum value of 0, and a maximum
    value of 80. This would give us a scroll bar with five "pages".

    \table
    \row \i \inlineimage qscrollbar-values.png
    \i The relationship between a document length, the range of values used
    in a scroll bar, and the page step is simple in many common situations.
    The scroll bar's range of values is determined by subtracting a
    chosen page step from some value representing the length of the document.
    In such cases, the following equation is useful:

    \e{document length} = maximum() - minimum() + pageStep().
    \endtable

    QScrollBar only provides integer ranges. Note that although
    QScrollBar handles very large numbers, scroll bars on current
    screens cannot usefully represent ranges above about 100,000 pixels.
    Beyond that, it becomes difficult for the user to control the
    slider using either the keyboard or the mouse, and the scroll
    arrows will have limited use.

    ScrollBar inherits a comprehensive set of signals from QAbstractSlider:
    \list
    \i \l{QAbstractSlider::valueChanged()}{valueChanged()} is emitted when the
       scroll bar's value has changed. The tracking() determines whether this
       signal is emitted during user interaction.
    \i \l{QAbstractSlider::rangeChanged()}{rangeChanged()} is emitted when the
       scroll bar's range of values has changed.
    \i \l{QAbstractSlider::sliderPressed()}{sliderPressed()} is emitted when
       the user starts to drag the slider.
    \i \l{QAbstractSlider::sliderMoved()}{sliderMoved()} is emitted when the user
       drags the slider.
    \i \l{QAbstractSlider::sliderReleased()}{sliderReleased()} is emitted when
       the user releases the slider.
    \i \l{QAbstractSlider::actionTriggered()}{actionTriggered()} is emitted
       when the scroll bar is changed by user interaction or via the
       \l{QAbstractSlider::triggerAction()}{triggerAction()} function.
    \endlist

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

    The slider itself can be controlled by using the
    \l{QAbstractSlider::triggerAction()}{triggerAction()} function to simulate
    user interaction with the scroll bar controls. This is useful if you have
    many different widgets that use a common range of values.

    \inlineimage qscrbar-m.png Screenshot in Motif style
    \inlineimage qscrbar-w.png Screenshot in Windows style

    Most GUI styles use the pageStep() value to calculate the size of the
    slider.

    \sa QScrollArea QSlider QDial QSpinBox
    \link guibooks.html#fowler GUI Design Handbook: Scroll Bar\endlink
*/




class QScrollBarPrivate : public QAbstractSliderPrivate
{
    Q_DECLARE_PUBLIC(QScrollBar)
public:
    QStyle::SubControl pressedControl;
    bool pointerOutsidePressedControl;

    int clickOffset, snapBackPosition;

    void activateControl(uint control);
    int pixelPosToRangeValue(int pos) const;
    QStyleOptionSlider getStyleOption() const;
    void init();
    bool updateHoverControl(const QPoint &pos);
    QStyle::SubControl newHoverControl(const QPoint &pos);

    QStyle::SubControl hoverControl;
    QRect hoverRect;
};
#define d d_func()
#define q q_func()

bool QScrollBarPrivate::updateHoverControl(const QPoint &pos)
{
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

QStyle::SubControl QScrollBarPrivate::newHoverControl(const QPoint &pos)
{
    QStyleOptionSlider opt = getStyleOption();
    opt.subControls = QStyle::SC_All;
    QRect addLineRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarAddLine, q);
    QRect subLineRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSubLine, q);
    QRect addPageRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarAddPage, q);
    QRect subPageRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSubPage, q);
    QRect firstRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarFirst, q);
    QRect lastRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarLast, q);
    QRect sliderRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, q);
    QRect grooveRect = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarGroove, q);

    if (sliderRect.contains(pos)) {
        hoverRect = sliderRect;
        hoverControl = QStyle::SC_ScrollBarSlider;
    } else if (grooveRect.contains(pos)) {
        hoverRect = grooveRect;
        hoverControl = QStyle::SC_ScrollBarGroove;
    } else if (addLineRect.contains(pos)) {
        hoverRect = addLineRect;
        hoverControl = QStyle::SC_ScrollBarAddLine;
    } else if (subLineRect.contains(pos)) {
        hoverRect = subLineRect;
        hoverControl = QStyle::SC_ScrollBarSubLine;
    } else if (addPageRect.contains(pos)) {
        hoverRect = addPageRect;
        hoverControl = QStyle::SC_ScrollBarAddPage;
    } else if (subPageRect.contains(pos)) {
        hoverRect = subPageRect;
        hoverControl = QStyle::SC_ScrollBarSubPage;
    } else if (firstRect.contains(pos)) {
        hoverRect = firstRect;
        hoverControl = QStyle::SC_ScrollBarFirst;
    } else if (lastRect.contains(pos)) {
        hoverRect = lastRect;
        hoverControl = QStyle::SC_ScrollBarLast;
    } else {
        hoverRect = QRect();
        hoverControl = QStyle::SC_None;
    }

    return hoverControl;
}

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
    QStyleOptionSlider opt;
    opt.init(q);
    opt.subControls = QStyle::SC_None;
    opt.activeSubControls = QStyle::SC_None;
    opt.orientation = orientation;
    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.sliderPosition = position;
    opt.sliderValue = value;
    opt.singleStep = singleStep;
    opt.pageStep = pageStep;
    opt.upsideDown = invertedAppearance;
    if (orientation == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    return opt;
}


#define HORIZONTAL (d->orientation == Qt::Horizontal)
#define VERTICAL !HORIZONTAL

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


#ifdef QT3_SUPPORT
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
#endif // QT3_SUPPORT

/*!
    Destroys the scroll bar.
*/
QScrollBar::~QScrollBar()
{
}

void QScrollBarPrivate::init()
{
    invertedControls = true;
    pressedControl = hoverControl = QStyle::SC_None;
    pointerOutsidePressedControl = false;
    q->setFocusPolicy(Qt::NoFocus);
    QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Fixed);
    if (orientation == Qt::Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}



/*! \reimp */
QSize QScrollBar::sizeHint() const
{
    ensurePolished();
    QStyleOptionSlider opt = d->getStyleOption();
    int sbextent = style()->pixelMetric(QStyle::PM_ScrollBarExtent, &opt, this);
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
           && style()->styleHint(QStyle::SH_ScrollBar_StopMouseOverSlider, 0, this)
           && style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt,
                                             mapFromGlobal(QCursor::pos()),
                                             this) == QStyle::SC_ScrollBarSlider) {
            setRepeatAction(SliderNoAction);
        }
    }
    QAbstractSlider::sliderChange(change);
}

/*!
    \reimp
*/
bool QScrollBar::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
    if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
        d->updateHoverControl(he->pos());
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

/*!
    \reimp
*/
void QScrollBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionSlider opt = d->getStyleOption();
    opt.subControls = QStyle::SC_All;
    if (d->pressedControl) {
        opt.activeSubControls = (QStyle::SubControl)d->pressedControl;
        if (!d->pointerOutsidePressedControl)
            opt.state |= QStyle::State_Sunken;
    } else {
        opt.activeSubControls = (QStyle::SubControl)d->hoverControl;
    }
    style()->drawComplexControl(QStyle::CC_ScrollBar, &opt, &p, this);
}

/*!
    \reimp
*/
void QScrollBar::mousePressEvent(QMouseEvent *e)
{
    bool midButtonAbsPos = style()->styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition,
                                             0, this);
    QStyleOptionSlider opt = d->getStyleOption();

    if (d->maximum == d->minimum // no range
        || (e->buttons() & (~e->button())) // another button was clicked before
        || !(e->button() == Qt::LeftButton || (midButtonAbsPos && e->button() == Qt::MidButton)))
        return;

    d->pressedControl = style()->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, e->pos(), this);
    d->pointerOutsidePressedControl = false;

    QRect sr = style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                       QStyle::SC_ScrollBarSlider, this);
    QPoint click = QStyle::visualPos(opt.direction, opt.rect, e->pos());
    if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
        d->clickOffset = HORIZONTAL ? (click.x()-sr.x()) : (click.y()-sr.y());
        d->snapBackPosition = d->position;
    }

    if ((d->pressedControl == QStyle::SC_ScrollBarAddPage
          || d->pressedControl == QStyle::SC_ScrollBarSubPage
          || d->pressedControl == QStyle::SC_ScrollBarSlider)
        && ((midButtonAbsPos && e->button() == Qt::MidButton)
            || style()->styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition, &opt, this)
            && e->button() == Qt::LeftButton)) {
        int sliderLength = HORIZONTAL ? sr.width() : sr.height();
        setSliderPosition(d->pixelPosToRangeValue((HORIZONTAL ? e->pos().x()
                                                              : e->pos().y()) - sliderLength / 2));
        d->pressedControl = QStyle::SC_ScrollBarSlider;
        d->clickOffset = sliderLength / 2;
    }
    d->activateControl(d->pressedControl);
    repaint(QStyle::visualRect(opt.direction, opt.rect,
                               style()->subControlRect(QStyle::CC_ScrollBar, &opt, d->pressedControl, this)));
}


/*!
    \reimp
*/
void QScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (!d->pressedControl)
        return;

    if (e->buttons() & (~e->button())) // some other button is still pressed
        return;

    QStyle::SubControl tmp = d->pressedControl;
    setRepeatAction(SliderNoAction);
    d->pressedControl = QStyle::SC_None;
    if (tmp == QStyle::SC_ScrollBarSlider)
        setSliderDown(false);
    QStyleOptionSlider opt = d->getStyleOption();
    repaint(QStyle::visualRect(opt.direction, opt.rect,
                               style()->subControlRect(QStyle::CC_ScrollBar, &opt, tmp, this)));
}


/*!
    \reimp
*/
void QScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    if (!d->pressedControl)
        return;

    QStyleOptionSlider opt = d->getStyleOption();
    if (!(e->buttons() & Qt::LeftButton
          ||  ((e->buttons() & Qt::MidButton)
               && style()->styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, &opt, this))))
        return;

    if (d->pressedControl == QStyle::SC_ScrollBarSlider) {
        QPoint click = QStyle::visualPos(opt.direction, opt.rect, e->pos());
        int newPosition = d->pixelPosToRangeValue((HORIZONTAL ? click.x() : click.y()) -d->clickOffset);
        int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
        if (m >= 0) {
            QRect r = rect();
            r.adjust(-m, -m, m, m);
            if (! r.contains(e->pos()))
                newPosition = d->snapBackPosition;
        }
        setSliderPosition(newPosition);
    } else if (!style()->styleHint(QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl, &opt, this)) {
        // stop scrolling when the mouse pointer leaves a control
        // similar to push buttons
        opt.subControls = d->pressedControl;
        QRect pr = QStyle::visualRect(opt.direction, opt.rect,
                                      style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                                              d->pressedControl, this));
        if (pr.contains(e->pos()) == d->pointerOutsidePressedControl) {
            if ((d->pointerOutsidePressedControl = !d->pointerOutsidePressedControl)) {
                d->pointerOutsidePressedControl = true;
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
    QRect gr = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                          QStyle::SC_ScrollBarGroove, q);
    QRect sr = q->style()->subControlRect(QStyle::CC_ScrollBar, &opt,
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

    return  QStyle::sliderValueFromPosition(d->minimum, d->maximum, pos - sliderMin,
                                            sliderMax - sliderMin, opt.upsideDown);
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

