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
    QStyleOptionProgressBar getStyleOption() const;
};

#define d d_func()
#define q q_func()

QProgressBarPrivate::QProgressBarPrivate()
    : minimum(0), maximum(100), value(-1), alignment(Qt::AlignCenter), textVisible(true)
{
}

void QProgressBarPrivate::init()
{
    q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

QStyleOptionProgressBar QProgressBarPrivate::getStyleOption() const
{
    QStyleOptionProgressBar opt;
    opt.init(q);

    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.progress = value;
    opt.textAlignment = alignment;
    opt.textVisible = textVisible;
    opt.text = q->text();
    opt.q3IndicatorFollowsStyle = false;

    return opt;
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
    when you later give it the current step value.

    You can specify the minimum and maximum number of steps with
    setMinimum() and setMaximum. The current number of steps is set
    with setValue(). The progress bar can be rewound to the
    beginning with reset().

    If minimum and maximum both are set to 0, the bar shows a busy indicator
    instead of a percentage of steps. This is useful, for example, when using
    QFtp or QHttp to download items when they are unable to determine the
    size of the item being downloaded.

    \sa QProgressDialog

    \inlineimage qprogbar-m.png Screenshot in Motif style
    \inlineimage qprogbar-w.png Screenshot in Windows style

    \sa QProgressDialog
    \link guibooks.html#fowler GUI Design Handbook: Progress Indicator\endlink
*/


/*!
    Constructs a progress bar.

    By default, the minimum step value is set to 0, and the
    maximum to 100.

    The \a parent, ais passed on to the QWidget::QWidget() constructor.

    \sa setTotalSteps()
*/

QProgressBar::QProgressBar(QWidget *parent)
    : QWidget(*(new QProgressBarPrivate), parent, 0)
{
    d->init();
}

/*!
    Reset the progress bar. The progress bar "rewinds" and shows no
    progress.
*/

void QProgressBar::reset()
{
    d->value = d->minimum - 1;
    if (d->minimum == INT_MIN)
        d->value = INT_MIN;
    repaint();
}

/*!
    \property QProgressBarr::minimum
    \brief the progressbars's minimum value

    When setting this property, the \l maximum is adjusted if
    necessary to ensure that the range remains valid. If the
    current value falls outside the new range, the progressbar is reset
    with reset().
*/
void QProgressBar::setMinimum(int minimum)
{
    d->minimum = minimum;
}

int QProgressBar::minimum() const
{
    return d->minimum;
}


/*!
    \property QProgressBarr::maximum
    \brief the progressbars's maximum value

    When setting this property, the \l minimum is adjusted if
    necessary to ensure that the range remains valid. If the
    current value falls outside the new range, the progressbar is reset
    with reset().
*/

void QProgressBar::setMaximum(int maximum)
{
    d->maximum = maximum;
}

int QProgressBar::maximum() const
{
    return d->maximum;
}

/*!
    \property QProgressBar::value
    \brief the progressbar's current value

    Attemting to change the current value to one outside
    the minimum-maximum range has no effect the current value.
*/
void QProgressBar::setValue(int value)
{
    if (d->value == value
            || ((value > d->maximum || value < d->minimum)
                && (d->maximum != 0 || d->minimum != 0)))
        return;
    d->value = value;
    emit valueChanged(value);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
    repaint();
}

int QProgressBar::value() const
{
    return d->value;
}

/*!
    Sets the progressbar's minimum to \a min and its maximum to \a max.

    If \a max is smaller than \a min, \a min becomes the only legal
    value.

    \sa minimum maximum
*/
void QProgressBar::setRange(int minimum, int maximum)
{
    setMinimum(minimum);
    setMaximum(maximum);
}
/*!
    Sets wether the  the current completed percentage
    shoud be displayed.
*/
void QProgressBar::setTextVisible(bool visible)
{
    if (d->textVisible != visible) {
        d->textVisible = visible;
        repaint();
    }
}

bool QProgressBar::isTextVisible() const
{
    return d->textVisible;
}
/*!
    Sets the allignment for the progressbar.

    \sa Qt::allignment
*/
void QProgressBar::setAlignment(Qt::Alignment alignment)
{
    if (d->alignment != alignment) {
        d->alignment = alignment;
        repaint();
    }
}

Qt::Alignment QProgressBar::alignment() const
{
    return d->alignment;
}

/*!
    \reimp
*/
void QProgressBar::paintEvent(QPaintEvent *)
{
    QStylePainter paint(this);

    QStyleOptionProgressBar opt = d->getStyleOption();
    const QFontMetrics &fm = fontMetrics();
    opt.rect = QStyle::visualRect(style()->subRect(QStyle::SR_ProgressBarGroove, &opt, fm, this),
                                  this);

    paint.drawControl(QStyle::CE_ProgressBarGroove, opt);
    opt.rect = rect();
    opt.rect = QStyle::visualRect(style()->subRect(QStyle::SR_ProgressBarContents, &opt, fm, this),
                                  this);
    paint.drawControl(QStyle::CE_ProgressBarContents, opt);

    if (d->textVisible) {
        opt.rect = rect();
        opt.rect = QStyle::visualRect(style()->subRect(QStyle::SR_ProgressBarLabel, &opt, fm, this),
                                      this);
        paint.drawControl(QStyle::CE_ProgressBarLabel, opt);
    }
}

/*!
    \reimp
*/
QSize QProgressBar::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    QStyleOptionProgressBar opt = d->getStyleOption();
    int cw = style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, this);
    return style()->sizeFromContents(QStyle::CT_ProgressBar, &opt,
                                    QSize(cw * 7 + fm.width('0') * 4, fm.height() + 8), fm, this);
}

/*!
    \reimp
*/
QSize QProgressBar::minimumSizeHint() const
{
    return sizeHint();
}

/*!
    This method is called to generate the text displayed in the center
    (or in some styles, to the left) of the progress bar.

    The \a progress may be smaller than minimum, indicating that the
    progress bar is in the "reset" state before any progress is set.

    The default implementation is the percentage of completion or
    blank in the reset state. The percentage is calculated as
    the \a progress divided by \a maximum() - \a munimum().
*/
QString QProgressBar::text() const
{
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
    return QString::number(progress * 100 / totalSteps) + "%";
}


#endif
