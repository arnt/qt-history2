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

    The progress bar uses the concept of \e steps; you give it the
    total number of steps and the number of steps completed so far and
    it will display the percentage of steps that have been completed.
    You can specify the total number of steps in the constructor or
    later with setTotalSteps(). The current number of steps is set
    with setProgress(). The progress bar can be rewound to the
    beginning with reset().

    If the total is given as 0 the progress bar shows a busy indicator
    instead of a percentage of steps. This is useful, for example,
    when using QFtp or QHttp to download items when they are unable to
    determine the size of the item being downloaded.

    \sa QProgressDialog

    \inlineimage qprogbar-m.png Screenshot in Motif style
    \inlineimage qprogbar-w.png Screenshot in Windows style

    \sa QProgressDialog
    \link guibooks.html#fowler GUI Design Handbook: Progress Indicator\endlink
*/


/*!
    Constructs a progress bar.

    The total number of steps is set to 100 by default.

    The \a parent, and widget flags, \a f, are passed on to
    the QFrame::QFrame() constructor.

    \sa setTotalSteps()
*/

QProgressBar::QProgressBar(QWidget *parent, Qt::WFlags f)
    : QWidget(*(new QProgressBarPrivate), parent, f)
{
    d->init();
}


/*!
    Constructs a progress bar.

    The \a totalSteps is the total number of steps that need to be
    completed for the operation which this progress bar represents.
    For example, if the operation is to examine 50 files, this value
    would be 50. Before examining the first file, call setProgress(0);
    call setProgress(50) after examining the last file.

    The \a parent, and widget flags, \a f, are passed to the
    QFrame::QFrame() constructor.

    \sa setTotalSteps(), setProgress()
*/

QProgressBar::QProgressBar(int minimum, int maximum, QWidget *parent, Qt::WFlags f)
    : QWidget(*(new QProgressBarPrivate), parent, f)
{
    d->init();
    setMinimum(minimum);
    setMaximum(maximum);
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


void QProgressBar::setMinimum(int minimum)
{
    d->minimum = minimum;
}

int QProgressBar::minimum() const
{
    return d->minimum;
}

void QProgressBar::setMaximum(int maximum)
{
    d->maximum = maximum;
}

int QProgressBar::maximum() const
{
    return d->maximum;
}

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

    The \a progress may be negative, indicating that the progress bar
    is in the "reset" state before any progress is set.

    The default implementation is the percentage of completion or
    blank in the reset state. The percentage is calculated based on
    the \a progress and \a totalSteps. You can set the \a indicator
    text if you wish.

    To allow efficient repainting of the progress bar, this method
    should return false if the string is unchanged from the last call
    to this function.
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
