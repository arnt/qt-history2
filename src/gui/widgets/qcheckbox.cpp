/****************************************************************************
**
** Implementation of QCheckBox class.
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

#include "qcheckbox.h"
#ifndef QT_NO_CHECKBOX
#include "qapplication.h"
#include "qbitmap.h"
#include "qiconset.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"

#include "private/qabstractbutton_p.h"

class QCheckBoxPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QCheckBox)
public:
    QCheckBoxPrivate():tristate(false), noChange(false){}
    uint tristate : 1;
    uint noChange : 1;
    void init();
    QStyleOptionButton getStyleOption() const;
};

#define d d_func()
#define q q_func()

/*!
    \class QCheckBox
    \brief The QCheckBox widget provides a checkbox with a text label.

    \ingroup basic
    \mainclass

    A QCheckBox is an option button that can be switched on (checked)
    or off (unchecked). Checkboxes are typically used to represent
    features in an application that can be enabled or disabled without
    affecting others, but different types of behavior can be
    implemented.

    A QButtonGroup can be used to group check buttons visually.

    Whenever a checkbox is checked or cleared it emits the signal
    stateChanged(). Connect to this signal if you want to trigger an
    action each time the checkbox changes state. You can use
    isChecked() to query whether or not a checkbox is checked.

    In addition to the usual checked and unchecked states, QCheckBox
    optionally provides a third state to indicate "no change". This
    is useful whenever you need to give the user the option of neither
    checking nor unchecking a checkbox. If you need this third state,
    enable it with setTristate(), and use state() to query the current
    toggle state.

    Just like QPushButton, a checkbox can display text or a pixmap.
    The text can be set in the constructor or with setText(); the
    pixmap is set with setPixmap().

    Important inherited functions: text(), setText(), text(),
    pixmap(), setPixmap(), accel(), setAccel(), isToggleButton(),
    setDown(), isDown(), isOn(), state(), autoRepeat(),
    isExclusiveToggle(), group(), setAutoRepeat(), toggle(),
    pressed(), released(), clicked(), toggled(), state(), and
    stateChanged().

    \inlineimage qchkbox-m.png Screenshot in Motif style
    \inlineimage qchkbox-w.png Screenshot in Windows style

    \sa QButton QRadioButton
    \link guibooks.html#fowler Fowler: Check Box \endlink
*/

/*!
    \enum QCheckBox::ToggleState

    \value Off
    \value NoChange
    \value On
*/

/*!
    \fn void QCheckBox::stateChanged(int state)

    This signal is emitted whenever the check box's state changes,
    i.e. whenever the user checks or unchecks it.

    \a state contains the check box's new \c ToggleState.
*/

/*!
    \property QCheckBox::autoMask
    \brief whether the checkbox is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QCheckBox::tristate
    \brief whether the checkbox is a tri-state checkbox

    The default is false; i.e. the checkbox has only two states.
*/



void QCheckBoxPrivate::init()
{
    q->setCheckable(true);
}

QStyleOptionButton QCheckBoxPrivate::getStyleOption() const
{
    QStyleOptionButton opt(0);
    opt.init(q);
    if (down)
        opt.state |= QStyle::Style_Down;
    if (q->testAttribute(Qt::WA_UnderMouse))
        opt.state |= QStyle::Style_HasFocus;
    if (tristate && noChange)
        opt.state |= QStyle::Style_NoChange;
    else
        opt.state |= checked ? QStyle::Style_On : QStyle::Style_Off;
    opt.text = text;
    opt.icon = icon;
    return opt;
}

/*!
    Constructs a checkbox with the given \a parent, but with no text.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QCheckBox::QCheckBox(QWidget *parent)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    d->init();
}

/*!
    Constructs a checkbox with the given \a parent and \a text.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QCheckBox::QCheckBox(const QString &text, QWidget *parent)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    d->init();
    setText(text);
}

void QCheckBox::setTristate(bool y)
{
    d->tristate = y;
}

bool QCheckBox::isTristate() const
{
    return d->tristate;
}


/*!
    Returns the check box's toggle state.

    \sa setState() ToggleState
*/
QCheckBox::ToggleState QCheckBox::state() const
{
    if (d->tristate &&  d->noChange)
        return NoChange;
    return d->checked ? On : Off;
}

/*!
    Sets the check box's toggle state to \a state.

    \sa state() ToggleState
*/
void QCheckBox::setState(ToggleState state)
{
    if (state == NoChange) {
        d->tristate = true;
        d->noChange = true;
    } else {
        d->noChange = false;
    }
    d->blockRefresh = true;
    setChecked(state != Off);
    d->blockRefresh = false;
    d->refresh();
    emit stateChanged(state);
}


/*!\reimp
*/
QSize QCheckBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    QStyleOptionButton opt = d->getStyleOption();
    QSize sz = style().itemRect(fm, QRect(0, 0, 1, 1), Qt::TextShowMnemonic, false, text()).size();
    return (style().sizeFromContents(QStyle::CT_CheckBox, &opt, sz, fm, this)
                   .expandedTo(QApplication::globalStrut()));
}

/*!
    Draws the checkbox bevel on painter \a paint. Called from paintEvent().

    \sa drawLabel()
*/
void QCheckBox::drawBevel(QPainter *paint)
{
    QStyleOptionButton opt = d->getStyleOption();
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxIndicator, &opt, this), this);
    style().drawControl(QStyle::CE_CheckBox, &opt, paint, this);
}


/*!
    Draws the check box label on painter \a p. Called from paintEvent().

    \sa drawBevel()
*/
void QCheckBox::drawLabel(QPainter *p)
{
    QStyleOptionButton opt = d->getStyleOption();
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxContents, &opt, this), this);
    style().drawControl(QStyle::CE_CheckBoxLabel, &opt, p, this);
}

/*!
    \fn void QCheckBox::paintEvent(QPaintEvent *event)

    Paints the button in response to the paint \a event, by first
    calling drawBevel() and then drawLabel(). If you reimplement
    paintEvent() in order to draw a different label only, you can call
    drawBevel() from your own code.

    \code
        QPainter p(this);
        drawBevel(&p);
        // ... your label drawing code
    \endcode
*/
void QCheckBox::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawBevel(&p);
    drawLabel(&p);
}


/*!
  \reimp
*/
void QCheckBox::updateMask()
{
    QStyleOptionButton opt = d->getStyleOption();
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxIndicator, &opt, this), this);

    QBitmap bm(width(), height());
    bm.fill(Qt::color0);

    QPainter p(&bm);
    style().drawControlMask(QStyle::CE_CheckBox, &opt, &p, this);
    if (!text().isNull() || !icon().isNull()) {
        QStyleOptionButton opt = d->getStyleOption();
        QRect crect = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxContents, &opt, this),
                                         this);
        QRect frect = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxFocusRect, &opt, this),
                                         this);
        QRect label(crect.unite(frect));
        p.fillRect(label, Qt::color1);
    }
    p.end();

    setMask(bm);
}

/*!\reimp*/
bool QCheckBox::hitButton(const QPoint &pos) const
{
    QStyleOptionButton opt = d->getStyleOption();
    QRect r = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxFocusRect, &opt, this), this);
    if (qApp->reverseLayout()) {
        r.setRight(width());
    } else {
        r.setLeft(0);
    }
    return r.contains(pos);
}

/*!\reimp*/
void QCheckBox::checkStateSet()
{
    d->noChange = false;
    emit stateChanged(state());
}

/*!\reimp*/
void QCheckBox::nextCheckState()
{
    if (d->tristate)
        setState((ToggleState)((state() + 1) % 3));
    else
        QAbstractButton::nextCheckState();
}

#ifdef QT_COMPAT
QCheckBox::QCheckBox(QWidget *parent, const char* name)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    setObjectName(name);
    d->init();
}

QCheckBox::QCheckBox(const QString &text, QWidget *parent, const char* name)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    setObjectName(name);
    d->init();
    setText(text);
}

#endif

#endif
