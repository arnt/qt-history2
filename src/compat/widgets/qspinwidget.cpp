/****************************************************************************
**
** Implementation of QSpinWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qrangecontrol.h"

#ifndef QT_NO_SPINWIDGET

#include "qabstractspinbox.h"
#include "qevent.h"
#include "qpainter.h"
#include "qrect.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtimer.h"

class QSpinWidgetPrivate
{
public:
    QSpinWidgetPrivate()
        : upEnabled(true),
          downEnabled(true),
          theButton(0),
          buttonDown(0),
          timerUp(0),
          bsyms(QSpinWidget::UpDownArrows),
          ed (0) {}
    uint upEnabled :1;
    uint downEnabled :1;
    uint theButton :2;
    uint buttonDown :2;
    uint timerUp : 1;
    QRect up;
    QRect down;
    QTimer auRepTimer;
    QSpinWidget::ButtonSymbols bsyms;
    QWidget *ed;
    void startTimer(int msec) { auRepTimer.start(msec, true); }
    void startTimer(bool up, int msec) { timerUp = up; startTimer(msec); }
    void stopTimer() { auRepTimer.stop(); }
};

/*!

    \class QSpinWidget qspinwidget.h
    \brief The QSpinWidget class is an internal range control related class.

    \internal

    \ingroup basic

    Constructs an empty range control widget with parent \a parent
    called \a name.

*/

QSpinWidget::QSpinWidget(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    d = new QSpinWidgetPrivate();
    connect(&d->auRepTimer, SIGNAL(timeout()), this, SLOT(timerDone()));
    setFocusPolicy(Qt::StrongFocus);

    arrange();
    updateDisplay();
}


/*! Destroys the object and frees any allocated resources.

*/

QSpinWidget::~QSpinWidget()
{
    delete d;
}

/*! */
QWidget * QSpinWidget::editWidget()
{
    return d->ed;
}

/*!
    Sets the editing widget to \a w.
*/
void QSpinWidget::setEditWidget(QWidget * w)
{
    if (w) {
        if (w->parentWidget() != this)
            w->setParent(this);
        setFocusProxy(w);
    }
    d->ed = w;
    arrange();
    updateDisplay();
}

/*! \reimp

*/

void QSpinWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        d->stopTimer();
        d->buttonDown = 0;
        d->theButton = 0;
        repaint(d->down.unite(d->up));
        return;
    }

    uint oldButtonDown = d->buttonDown;

    if (d->down.contains(e->pos()) && d->downEnabled)
        d->buttonDown = 1;
    else if (d->up.contains(e->pos()) && d->upEnabled)
        d->buttonDown = 2;
    else
        d->buttonDown = 0;

    d->theButton = d->buttonDown;
    if (oldButtonDown != d->buttonDown) {
        if (!d->buttonDown) {
            repaint(d->down.unite(d->up));
        } else if (d->buttonDown & 1) {
            repaint(d->down);
            stepDown();
            d->startTimer(false, 300);
        } else if (d->buttonDown & 2) {
            repaint(d->up);
            stepUp();
            d->startTimer(true, 300);
        }
    }

    if (!oldButtonDown && !d->buttonDown)
        e->ignore();

}

static QStyleOptionSpinBox getStyleOption(const QSpinWidget *spin)
{
    QStyleOptionSpinBox opt(0);
    opt.init(spin);
    opt.parts = 0;
    opt.buttonSymbols = (QAbstractSpinBox::ButtonSymbols)spin->buttonSymbols();
    opt.percentage = 0; // no way to get this information as it is in QRangeControl.
    opt.slider = false;
    opt.stepEnabled = 0;
    if (spin->isUpEnabled())
        opt.stepEnabled |= QAbstractSpinBox::StepUpEnabled;
    if (spin->isDownEnabled())
        opt.stepEnabled |= QAbstractSpinBox::StepDownEnabled;
    opt.activeParts = 0;
    return opt;
}

/*!

*/

void QSpinWidget::arrange()
{
    QStyleOptionSpinBox opt = getStyleOption(this);
    d->up = QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_SpinBox, &opt,
                                                              QStyle::SC_SpinBoxUp, this), this);
    d->down = QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_SpinBox, &opt,
                                                                QStyle::SC_SpinBoxDown, this),
                                                                this);
    if (d->ed) {
        QRect r = QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_SpinBox, &opt,
                                                                    QStyle::SC_SpinBoxEditField,
                                                                    this), this);
        d->ed->setGeometry(r);
    }
}

/*!

*/

void QSpinWidget::stepUp()
{
    emit stepUpPressed();
}

void QSpinWidget::resizeEvent(QResizeEvent*)
{
    arrange();
}

/*!

*/

void QSpinWidget::stepDown()
{
    emit stepDownPressed();
}


void QSpinWidget::timerDone()
{
    // we use a double timer to make it possible for users to do
    // something with 0-timer on valueChanged.
    QTimer::singleShot(1, this, SLOT(timerDoneEx()));
}

void QSpinWidget::timerDoneEx()
{
    if (!d->buttonDown)
        return;
    if (d->timerUp)
        stepUp();
    else
        stepDown();
    d->startTimer(100);
}


/*!
    The event is passed in \a e.
*/

void QSpinWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    uint oldButtonDown = d->theButton;
    d->theButton = 0;
    if (oldButtonDown != d->theButton) {
        if (oldButtonDown & 1)
            repaint(d->down);
        else if (oldButtonDown & 2)
            repaint(d->up);
    }
    d->stopTimer();
    d->buttonDown = 0;

    if (!oldButtonDown && !d->buttonDown)
        e->ignore();
}


/*!
    The event is passed in \a e.
*/

void QSpinWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->state() & Qt::LeftButton))
        return;

    uint oldButtonDown = d->theButton;
    if (oldButtonDown & 1 && !d->down.contains(e->pos())) {
        d->stopTimer();
        d->theButton = 0;
        repaint(d->down);
    } else if (oldButtonDown & 2 && !d->up.contains(e->pos())) {
        d->stopTimer();
        d->theButton = 0;
        repaint(d->up);
    } else if (!oldButtonDown && d->up.contains(e->pos()) && d->buttonDown & 2) {
        d->startTimer(500);
        d->theButton = 2;
        repaint(d->up);
    } else if (!oldButtonDown && d->down.contains(e->pos()) && d->buttonDown & 1) {
        d->startTimer(500);
        d->theButton = 1;
        repaint(d->down);
    }

    if (!oldButtonDown && !d->buttonDown)
        e->ignore();
}


/*!
    The event is passed in \a e.
*/
#ifndef QT_NO_WHEELEVENT
void QSpinWidget::wheelEvent(QWheelEvent *e)
{
    e->accept();
    static float offset = 0;
    static QSpinWidget* offset_owner = 0;
    if (offset_owner != this) {
        offset_owner = this;
        offset = 0;
    }
    offset += -e->delta()/120;
    if (QABS(offset) < 1)
        return;
    int ioff = int(offset);
    int i;
    for(i=0; i < QABS(ioff); i++)
        offset > 0 ? stepDown() : stepUp();
    offset -= ioff;
}
#endif

/*!

*/
void QSpinWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionSpinBox opt = getStyleOption(this);

    if (d->theButton & 1)
        opt.activeParts = QStyle::SC_SpinBoxDown;
    else if (d->theButton & 2)
        opt.activeParts = QStyle::SC_SpinBoxUp;
    else
        opt.activeParts = QStyle::SC_None;
    opt.rect = QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_SpinBox, &opt,
                                                                 QStyle::SC_SpinBoxFrame, this),
                                  this);
    opt.parts = QStyle::SC_All;
    style().drawComplexControl(QStyle::CC_SpinBox, &opt, &p, this);
}


// ### What does the QEvent passed in contain? It used to be the previous style.
/*!
    The previous style is passed in \a ev.
*/
void QSpinWidget::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        arrange();
    } else if(ev->type() == QEvent::ActivationChange) {
        if (!isActiveWindow() && d->buttonDown) {         //was active, but lost focus
            d->stopTimer();
            d->buttonDown = 0;
            d->theButton = 0;
        }
    } else if(ev->type() == QEvent::EnabledChange) {
        d->upEnabled = isEnabled();
        d->downEnabled = isEnabled();
        updateDisplay();
    }
    QWidget::changeEvent(ev);
}

/*!
*/

QRect QSpinWidget::upRect() const
{
    return d->up;
}

/*!
*/

QRect QSpinWidget::downRect() const
{
    return d->down;
}

/*!
*/

void QSpinWidget::updateDisplay()
{
    if (!isEnabled()) {
        d->upEnabled = false;
        d->downEnabled = false;
    }
    if (d->theButton & 1 && (d->downEnabled) == 0) {
        d->theButton &= ~1;
        d->buttonDown &= ~1;
    }

    if (d->theButton & 2 && (d->upEnabled) == 0) {
        d->theButton &= ~2;
        d->buttonDown &= ~2;
    }
    repaint();
}

/*!
    Sets up-enabled to \a on.
*/

void QSpinWidget::setUpEnabled(bool on)
{
    if ((bool)d->upEnabled != on) {
        d->upEnabled = on;
        updateDisplay();
    }
}

/*!
*/

bool QSpinWidget::isUpEnabled() const
{
    return d->upEnabled;
}

/*!
    Sets down-enabled to \a on.
*/

void QSpinWidget::setDownEnabled(bool on)
{
    if ((bool)d->downEnabled != on) {
        d->downEnabled = on;
        updateDisplay();
    }
}

/*!
*/

bool QSpinWidget::isDownEnabled() const
{
    return d->downEnabled;
}

/*!
    Sets the button symbol to \a bs.
*/

void QSpinWidget::setButtonSymbols(ButtonSymbols bs)
{
    d->bsyms = bs;
}

/*!
*/

QSpinWidget::ButtonSymbols QSpinWidget::buttonSymbols() const
{
    return d->bsyms;
}

#endif
