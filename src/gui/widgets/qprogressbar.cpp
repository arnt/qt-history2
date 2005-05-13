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

#include "qprogressbar.h"
#ifndef QT_NO_PROGRESSBAR
#include <qevent.h>
#include <qpainter.h>
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <private/qwidget_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif
#include <limits.h>

class QProgressBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QProgressBar)
public:
    QProgressBarPrivate();
    void init();

    int minimum;
    int maximum;
    int value;
    Qt::Alignment alignment;
    uint textVisible : 1;
    int lastPaintedValue;
    QStyleOptionProgressBar getStyleOption() const;
    inline int bound(int val) const { return qMax(minimum-1, qMin(maximum, val)); }
    bool repaintRequired() const;
};

QProgressBarPrivate::QProgressBarPrivate()
    : minimum(0), maximum(100), value(-1), alignment(Qt::AlignLeft), textVisible(true),
      lastPaintedValue(-1)
{
}

void QProgressBarPrivate::init()
{
    q_func()->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

QStyleOptionProgressBar QProgressBarPrivate::getStyleOption() const
{
    QStyleOptionProgressBar opt;
    opt.init(q_func());

    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.progress = value;
    opt.textAlignment = alignment;
    opt.textVisible = textVisible;
    opt.text = q_func()->text();

    return opt;
}

bool QProgressBarPrivate::repaintRequired() const
{
    Q_Q(const QProgressBar);
    if (value == lastPaintedValue)
        return false;

    int valueDifference = qAbs(value - lastPaintedValue);

    // Check if the text needs to be repainted
    if ((value == minimum || value == maximum)
            || (textVisible && valueDifference >= qAbs((maximum - minimum) / 100)))
        return true;

    // Check if the bar needs to be repainted
    QStyleOptionProgressBar opt = getStyleOption();
    int cw = q->style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, q);
    QRect groove  = q->style()->subElementRect(QStyle::SE_ProgressBarGroove, &opt, q);
    // This expression is basically
    // (valueDifference / (maximum - minimum) > cw / groove.width())
    // transformed to avoid integer division.
    return (valueDifference * groove.width() > cw * (maximum - minimum));
}

/*!
    \class QProgressBar qprogressbar.h
    \brief The QProgressBar widget provides a horizontal progress bar.

    \ingroup advanced
    \mainclass

    A progress bar is used to give the user an indication of the
    progress of an operation and to reassure them that the application
    is still running.

    The progress bar uses the concept of \e steps. You set it up by
    specifying the minumum and maximum possible step values, and it
    will display the percentage of steps that have been completed
    when you later give it the current step value. The percentage is
    calculated by dividing the progress (value() - minimum()) divided
    by maximum() - minimum().

    You can specify the minimum and maximum number of steps with
    setMinimum() and setMaximum. The current number of steps is set
    with setValue(). The progress bar can be rewound to the
    beginning with reset().

    If minimum and maximum both are set to 0, the bar shows a busy indicator
    instead of a percentage of steps. This is useful, for example, when using
    QFtp or QHttp to download items when they are unable to determine the
    size of the item being downloaded.

    \inlineimage macintosh-progressbar.png Screenshot in Macintosh style
    \inlineimage windows-progressbar.png Screenshot in Windows style

    \sa QProgressDialog, {fowler}{GUI Design Handbook: Progress Indicator}
*/

/*!
    \fn void QProgressBar::valueChanged(int value)

    This signal is emitted when the value shown in the progress bar changes.
    \a value is the new value shown by the progress bar.
*/

/*!
    Constructs a progress bar with the given \a parent.

    By default, the minimum step value is set to 0, and the maximum to 100.

    \sa setRange()
*/

QProgressBar::QProgressBar(QWidget *parent)
    : QWidget(*(new QProgressBarPrivate), parent, 0)
{
    d_func()->init();
}

/*!
    Reset the progress bar. The progress bar "rewinds" and shows no
    progress.
*/

void QProgressBar::reset()
{
    Q_D(QProgressBar);
    d->value = d->minimum - 1;
    if (d->minimum == INT_MIN)
        d->value = INT_MIN;
    repaint();
}

/*!
    \property QProgressBar::minimum
    \brief the progress bar's minimum value

    When setting this property, the \l maximum is adjusted if
    necessary to ensure that the range remains valid. If the
    current value falls outside the new range, the progressbar is reset
    with reset().
*/
void QProgressBar::setMinimum(int minimum)
{
    setRange(minimum, qMax(d_func()->maximum, minimum));
}

int QProgressBar::minimum() const
{
    return d_func()->minimum;
}


/*!
    \property QProgressBar::maximum
    \brief the progress bar's maximum value

    When setting this property, the \l minimum is adjusted if
    necessary to ensure that the range remains valid. If the
    current value falls outside the new range, the progressbar is reset
    with reset().
*/

void QProgressBar::setMaximum(int maximum)
{
    setRange(qMin(d_func()->minimum, maximum), maximum);
}

int QProgressBar::maximum() const
{
    return d_func()->maximum;
}

/*!
    \property QProgressBar::value
    \brief the progressbar's current value

    Attemting to change the current value to one outside
    the minimum-maximum range has no effect the current value.
*/
void QProgressBar::setValue(int value)
{
    Q_D(QProgressBar);
    if (d->value == value
            || ((value > d->maximum || value < d->minimum)
                && (d->maximum != 0 || d->minimum != 0)))
        return;
    d->value = value;
    emit valueChanged(value);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
    if (d->repaintRequired())
        repaint();
}

int QProgressBar::value() const
{
    return d_func()->value;
}

/*!
    Sets the progressbar's minimum and maximum values to \a minimum and
    \a maximum respectively.

    If \a maximum is smaller than \a minimum, \a minimum becomes the only
    legal value.

    If the current value falls outside the new range, the progressbar is reset
    with reset().

    \sa minimum maximum
*/
void QProgressBar::setRange(int minimum, int maximum)
{
    Q_D(QProgressBar);
    d->minimum = minimum;
    d->maximum = qMax(minimum, maximum);
    if ( d->value <(d->minimum-1) || d->value > d->maximum)
        reset();
}
/*!
    \property QProgressBar::textVisible
    \brief whether the current completed percentage should be displayed
*/
void QProgressBar::setTextVisible(bool visible)
{
    Q_D(QProgressBar);
    if (d->textVisible != visible) {
        d->textVisible = visible;
        repaint();
    }
}

bool QProgressBar::isTextVisible() const
{
    return d_func()->textVisible;
}

/*!
    \property QProgressBar::alignment
    \brief the alignment of the progress bar
*/
void QProgressBar::setAlignment(Qt::Alignment alignment)
{
    if (d_func()->alignment != alignment) {
        d_func()->alignment = alignment;
        repaint();
    }
}

Qt::Alignment QProgressBar::alignment() const
{
    return d_func()->alignment;
}

/*!
    \reimp
*/
void QProgressBar::paintEvent(QPaintEvent *)
{
    QStylePainter paint(this);
    QStyleOptionProgressBar opt = d_func()->getStyleOption();
    paint.drawControl(QStyle::CE_ProgressBar, opt);
    d_func()->lastPaintedValue = d_func()->value;
}

/*!
    \reimp
*/
QSize QProgressBar::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    QStyleOptionProgressBar opt = d_func()->getStyleOption();
    int cw = style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, this);
    return style()->sizeFromContents(QStyle::CT_ProgressBar, &opt,
                                    QSize(cw * 7 + fm.width('0') * 4, fm.height() + 8), this);
}

/*!
    \reimp
*/
QSize QProgressBar::minimumSizeHint() const
{
    return QSize(sizeHint().width(), fontMetrics().height() + 2);
}

/*!
    \property QProgressBar::text
    \brief the descriptive text shown with the progress bar

    The text returned is the same as the text displayed in the center
    (or in some styles, to the left) of the progress bar.

    The progress shown in the text may be smaller than the minimum value,
    indicating that the progress bar is in the "reset" state before any
    progress is set.

    In the default implementation, the text either contains a percentage
    value that indicates the progress so far, or it is blank because the
    progress bar is in the reset state.
*/
QString QProgressBar::text() const
{
    Q_D(const QProgressBar);
    if (d->maximum == 0 || d->value < d->minimum
            || (d->value == INT_MIN && d->minimum == INT_MIN))
        return QString();

    int totalSteps = d->maximum - d->minimum;
    int progress = d->value - d->minimum;
    // Get the values down to something usable.
    if (totalSteps > INT_MAX / 1000) {
        progress /= 1000;
        totalSteps /= 1000;
    }
    return tr("%1%").arg(progress * 100 / totalSteps);
}


#endif
