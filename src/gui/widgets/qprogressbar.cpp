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
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include <private/qinternal_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include "qevent.h"
#include <limits.h>

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


#ifdef QT_COMPAT
/*! \obsolete
    Constructs a progress bar.

    The total number of steps is set to 100 by default.

    The \a parent, \a name and widget flags, \a f, are passed on to
    the QFrame::QFrame() constructor.

    \sa setTotalSteps()
*/

QProgressBar::QProgressBar(QWidget *parent, const char *name, Qt::WFlags f)
    : QFrame(parent, f),
      total_steps(100),
      progress_val(-1),
      percentage(-1),
      center_indicator(true),
      auto_indicator(true),
      percentage_visible(true),
      d(0)
{
    setObjectName(name);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    initFrame();
}


/*! \obsolete
    Constructs a progress bar.

    The \a totalSteps is the total number of steps that need to be
    completed for the operation which this progress bar represents.
    For example, if the operation is to examine 50 files, this value
    would be 50. Before examining the first file, call setProgress(0);
    call setProgress(50) after examining the last file.

    The \a parent, \a name and widget flags, \a f, are passed to the
    QFrame::QFrame() constructor.

    \sa setTotalSteps(), setProgress()
*/

QProgressBar::QProgressBar(int totalSteps, QWidget *parent, const char *name, Qt::WFlags f)
    : QFrame(parent, f),
      total_steps(totalSteps),
      progress_val(-1),
      percentage(-1),
      center_indicator(true),
      auto_indicator(true),
      percentage_visible(true),
      d(0)
{
    setObjectName(name);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    initFrame();
}
#endif

/*!
    Constructs a progress bar.

    The total number of steps is set to 100 by default.

    The \a parent, and widget flags, \a f, are passed on to
    the QFrame::QFrame() constructor.

    \sa setTotalSteps()
*/

QProgressBar::QProgressBar(QWidget *parent, Qt::WFlags f)
    : QFrame(parent, f),
      total_steps(100),
      progress_val(-1),
      percentage(-1),
      center_indicator(true),
      auto_indicator(true),
      percentage_visible(true),
      d(0)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    initFrame();
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

QProgressBar::QProgressBar(int totalSteps, QWidget *parent, Qt::WFlags f)
    : QFrame(parent, f),
      total_steps(totalSteps),
      progress_val(-1),
      percentage(-1),
      center_indicator(true),
      auto_indicator(true),
      percentage_visible(true),
      d(0)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    initFrame();
}


/*!
    Reset the progress bar. The progress bar "rewinds" and shows no
    progress.
*/

void QProgressBar::reset()
{
    progress_val = -1;
    percentage = -1;
    setIndicator(progress_str, progress_val, total_steps);
    repaint();
}


/*!
    \property QProgressBar::totalSteps
    \brief The total number of steps.

    If totalSteps is 0, the progress bar will display a busy
    indicator.

    \sa totalSteps()
*/

void QProgressBar::setTotalSteps(int totalSteps)
{
    total_steps = totalSteps;

    // Current progress is invalid if larger than total
    if (total_steps < progress_val)
        progress_val = -1;

    if (isVisible() &&
         (setIndicator(progress_str, progress_val, total_steps) || !total_steps))
        repaint();
}


/*!
    \property QProgressBar::progress
    \brief The current amount of progress

    This property is -1 if progress counting has not started.
*/

void QProgressBar::setProgress(int progress)
{
    if (progress == progress_val ||
         progress < 0 || ((progress > total_steps) && total_steps))
        return;

    progress_val = progress;

    setIndicator(progress_str, progress_val, total_steps);

    repaint();

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
}

/*!
    \overload

    Sets the amount of progress to \a progress and the total number of
    steps to \a totalSteps.

    \sa setTotalSteps()
*/

void QProgressBar::setProgress(int progress, int totalSteps)
{
    if (total_steps != totalSteps)
        setTotalSteps(totalSteps);
    setProgress(progress);
}

/*!
  \property QProgressBar::progressString
  \brief the amount of progress as a string

    This property is an empty string if progress counting has not started.
*/

static QStyleOptionProgressBar getStyleOption(const QProgressBar *pb)
{
    QStyleOptionProgressBar opt(0);
    opt.init(pb);
    opt.totalSteps = pb->totalSteps();
    opt.progress = pb->progress();
    opt.progressString = pb->progressString();
    opt.features = QStyleOptionProgressBar::None;
    if (pb->centerIndicator())
        opt.features |= QStyleOptionProgressBar::CenterIndicator;
    if (pb->percentageVisible())
        opt.features |= QStyleOptionProgressBar::PercentageVisible;
    if (pb->indicatorFollowsStyle())
        opt.features |= QStyleOptionProgressBar::IndicatorFollowsStyle;
    return opt;
}

/*!
    \reimp
*/
QSize QProgressBar::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    int cw = style().pixelMetric(QStyle::PM_ProgressBarChunkWidth, this);
    QStyleOptionProgressBar opt = getStyleOption(this);
    return style().sizeFromContents(QStyle::CT_ProgressBar, &opt,
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
    \property QProgressBar::centerIndicator
    \brief whether the indicator string should be centered

    Changing this property sets \l QProgressBar::indicatorFollowsStyle
    to false. The default is true.
*/

void QProgressBar::setCenterIndicator(bool on)
{
    if (!auto_indicator && on == center_indicator)
        return;
    auto_indicator = false;
    center_indicator = on;
    repaint();
}

/*!
    \property QProgressBar::indicatorFollowsStyle
    \brief whether the display of the indicator string should follow the GUI style

    The default is true.

    \sa centerIndicator
*/

void QProgressBar::setIndicatorFollowsStyle(bool on)
{
    if (on == auto_indicator)
        return;
    auto_indicator = on;
    repaint();
}

/*!
    \property QProgressBar::percentageVisible
    \brief whether the current progress value is displayed

    The default is true.

    \sa centerIndicator, indicatorFollowsStyle
*/
void QProgressBar::setPercentageVisible(bool on)
{
    if (on == percentage_visible)
        return;
    percentage_visible = on;
    repaint();
}

/*!
    \reimp
*/
void QProgressBar::show()
{
    setIndicator(progress_str, progress_val, total_steps);
    QFrame::show();
}

void QProgressBar::initFrame()
{
    setFrameStyle(QFrame::NoFrame);
}

/*!
    \reimp
*/
void QProgressBar::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        initFrame();
    QFrame::changeEvent(ev);
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

bool QProgressBar::setIndicator(QString & indicator, int progress,
                                 int totalSteps)
{
    if (!totalSteps)
        return false;
    if (progress < 0) {
        indicator = QString::fromLatin1("");
        return true;
    } else {
        // Get the values down to something usable.
        if (totalSteps > INT_MAX/1000) {
            progress /= 1000;
            totalSteps /= 1000;
        }

        int np = progress * 100 / totalSteps;
        if (np != percentage) {
            percentage = np;
            indicator.sprintf("%d%%", np);
            return true;
        } else {
            return false;
        }
    }
}


/*!
    \reimp
*/
void QProgressBar::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    QPainter *p = &paint;
    drawFrame(p);
    
    QStyleOptionProgressBar opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_ProgressBarGroove, &opt, this), this);
    
    style().drawControl(QStyle::CE_ProgressBarGroove, &opt, p, this);
    opt.rect = rect();
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_ProgressBarContents, &opt, this),
                                  this);
    style().drawControl(QStyle::CE_ProgressBarContents, &opt, p, this);

    if (percentageVisible()) {
        opt.rect = rect();
        opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_ProgressBarLabel, &opt, this),
                                      this);
        style().drawControl(QStyle::CE_ProgressBarLabel, &opt, p, this);
    }
}

#endif
