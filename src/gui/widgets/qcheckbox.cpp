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

#include "qcheckbox.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qicon.h"
#include "qstylepainter.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qevent.h"

#include "private/qabstractbutton_p.h"

class QCheckBoxPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QCheckBox)
public:
    QCheckBoxPrivate():tristate(false), noChange(false), hovering(true){}
    uint tristate : 1;
    uint noChange : 1;
    uint hovering : 1;
    void init();
    QStyleOptionButton getStyleOption() const;
};

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
    enable it with setTristate(), and use checkState() to query the current
    toggle state.

    Just like QPushButton, a checkbox button displays text, and
    optionally a small icon. The text can be set in the constructor or
    with setText(); the icon is set with setIcon().

    Important inherited functions: text(), setText(), text(),
    pixmap(), setPixmap(), accel(), setAccel(), isToggleButton(),
    setDown(), isDown(), isOn(), checkState(), autoRepeat(),
    isExclusiveToggle(), group(), setAutoRepeat(), toggle(),
    pressed(), released(), clicked(), toggled(), checkState(), and
    stateChanged().

    \inlineimage macintosh-checkbox.png Screenshot in Macintosh style
    \inlineimage windows-checkbox.png Screenshot in Windows style

    \sa QAbstractButton, QRadioButton, {fowler}{GUI Design Handbook: Check Box}
*/

/*!
    \enum QCheckBox::ToggleState
    \compat

    \value Off  Use Qt::Unchecked instead.
    \value NoChange  Use Qt::PartiallyChecked instead.
    \value On  Use Qt::Checked instead.
*/

/*!
    \fn void QCheckBox::stateChanged(int state)

    This signal is emitted whenever the check box's state changes,
    i.e. whenever the user checks or unchecks it.

    \a state contains the check box's new ToggleState.
*/

/*!
    \property QCheckBox::tristate
    \brief whether the checkbox is a tri-state checkbox

    The default is false; i.e. the checkbox has only two states.
*/



void QCheckBoxPrivate::init()
{
    Q_Q(QCheckBox);
    q->setCheckable(true);
    q->setMouseTracking(true);
}

QStyleOptionButton QCheckBoxPrivate::getStyleOption() const
{
    Q_Q(const QCheckBox);
    QStyleOptionButton opt;
    opt.init(q);
    if (down)
        opt.state |= QStyle::State_Sunken;
    if (tristate && noChange)
        opt.state |= QStyle::State_NoChange;
    else
        opt.state |= checked ? QStyle::State_On : QStyle::State_Off;
    if (q->underMouse()) {
        if (hovering) 
            opt.state |= QStyle::State_MouseOver;
        else
            opt.state &= ~QStyle::State_MouseOver;
    }
    opt.text = text;
    opt.icon = icon;
    opt.iconSize = q->iconSize();
    return opt;
}

/*!
    Constructs a checkbox with the given \a parent, but with no text.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QCheckBox::QCheckBox(QWidget *parent)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    Q_D(QCheckBox);
    d->init();
}

/*!
    Constructs a checkbox with the given \a parent and \a text.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QCheckBox::QCheckBox(const QString &text, QWidget *parent)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    Q_D(QCheckBox);
    d->init();
    setText(text);
}

void QCheckBox::setTristate(bool y)
{
    Q_D(QCheckBox);
    d->tristate = y;
}

bool QCheckBox::isTristate() const
{
    Q_D(const QCheckBox);
    return d->tristate;
}


/*!
    Returns the check box's check state.

    \sa setCheckState() Qt::CheckState
*/
Qt::CheckState QCheckBox::checkState() const
{
    Q_D(const QCheckBox);
    if (d->tristate &&  d->noChange)
        return Qt::PartiallyChecked;
    return d->checked ? Qt::Checked : Qt::Unchecked;
}

/*!
    Sets the check box's check state to \a state.

    \sa checkState() Qt::CheckState
*/
void QCheckBox::setCheckState(Qt::CheckState state)
{
    Q_D(QCheckBox);
    if (state == Qt::PartiallyChecked) {
        d->tristate = true;
        d->noChange = true;
    } else {
        d->noChange = false;
    }
    d->blockRefresh = true;
    setChecked(state != Qt::Unchecked);
    d->blockRefresh = false;
    d->refresh();
    emit stateChanged(state);
}


/*!\reimp
*/
QSize QCheckBox::sizeHint() const
{
    Q_D(const QCheckBox);
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    QStyleOptionButton opt = d->getStyleOption();
    QSize sz = style()->itemTextRect(fm, QRect(0, 0, 1, 1), Qt::TextShowMnemonic, false,
                                     text()).size();
    if (!opt.icon.isNull())
        sz = QSize(sz.width() + opt.iconSize.width() + 4, qMax(sz.height(), opt.iconSize.height()));
    return (style()->sizeFromContents(QStyle::CT_CheckBox, &opt, sz, this)
            .expandedTo(QApplication::globalStrut()));
}

/*!\reimp
*/
void QCheckBox::paintEvent(QPaintEvent *)
{
    Q_D(QCheckBox);
    QStylePainter p(this);
    QStyleOptionButton opt = d->getStyleOption();
    p.drawControl(QStyle::CE_CheckBox, opt);
}

void QCheckBox::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QCheckBox);
    bool hit = false;
    if (underMouse())
        hit = hitButton(e->pos());

    if (hit != d->hovering) {
        update(rect());
        d->hovering = hit;
    }

    QAbstractButton::mouseMoveEvent(e);
}


/*!\reimp*/
bool QCheckBox::hitButton(const QPoint &pos) const
{
    Q_D(const QCheckBox);
    QStyleOptionButton opt = d->getStyleOption();
    return style()->subElementRect(QStyle::SE_CheckBoxClickRect, &opt, this).contains(pos);
}

/*!\reimp*/
void QCheckBox::checkStateSet()
{
    Q_D(QCheckBox);
    d->noChange = false;
    emit stateChanged(checkState());
}

/*!\reimp*/
void QCheckBox::nextCheckState()
{
    Q_D(QCheckBox);
    if (d->tristate)
        setCheckState((Qt::CheckState)((checkState() + 1) % 3));
    else {
        QAbstractButton::nextCheckState();
        QCheckBox::checkStateSet();
    }
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QCheckBox::QCheckBox(QWidget *parent, const char* name)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    Q_D(QCheckBox);
    setObjectName(name);
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QCheckBox::QCheckBox(const QString &text, QWidget *parent, const char* name)
    : QAbstractButton (*new QCheckBoxPrivate, parent)
{
    Q_D(QCheckBox);
    setObjectName(name);
    d->init();
    setText(text);
}

#endif


/*!
    \fn void QCheckBox::setNoChange()
    \compat

    Use setCheckState() instead.
*/

/*!
    \fn void QCheckBox::setState(ToggleState state)
    \compat

    Use setCheckState() instead.
*/

/*!
    \fn QCheckBox::ToggleState QCheckBox::state() const
    \compat

    Use checkState() instead.
*/
