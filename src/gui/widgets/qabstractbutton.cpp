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

#include "qabstractbutton.h"
#include "qbuttongroup.h"
#include "qabstractbutton_p.h"
#include "qevent.h"
#include "qpainter.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qaction.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

#define AUTO_REPEAT_DELAY  300
#define AUTO_REPEAT_PERIOD 100


/*!
\class QAbstractButton qabstractbutton.h

\brief The QAbstractButton class is the abstract base class of
button widgets, providing functionality common to buttons.

\ingroup abstractwidgets

This class implements an \e abstract button.
Subclasses of this class handle user actions, and specify how the button
is drawn.

QAbstractButton provides support for both push buttons and checkable
(toggle) buttons. Checkable buttons are implemented in the QRadioButton
and QCheckBox classes. Push buttons are implemented in the
QPushButton and QToolButton classes; these also provide toggle
behavior if required.

Any button can display a label containing text and an icon. setText()
sets the text; setIcon() sets the icon. If a button is disabled, its label
is changed to give the button a "disabled" appearance.

If the button is a text button with a string containing an ampersand (\&),
QButton creates an automatic shortcut key, called a mnemonic, that may
change based on the button translation. The following code creates
a push button labelled "Ro\underline{c}k & Roll" (where the c is underlined):

\code
      QPushButton *p = new QPushButton("Ro&ck && Roll", this);
\endcode

In this example, the shortcut Alt+C is assigned to the button, so that when
the user presses Alt+C the button will call animateClick().

You can also set a custom shortcut key using the setShortcut()
function. This is useful mostly for buttons that do not have any
text, because they have no automatic shortcut.

\code
      p->setPixmap(QPixmap("print.png"));
      p->setShortcut(Qt::ALT+Qt::Key_F7);
\endcode

All of the buttons provided by Qt (\l QPushButton, \l QToolButton,
\l QCheckBox and \l QRadioButton) can display both text and
pixmaps.

A button can be made the default button in a dialog are provided by
QPushButton::setDefault() and QPushButton::setAutoDefault().

QAbstractButton provides most of the states used for buttons:

\list

\i isDown() indicates whether the button is \e pressed down.

\i isChecked() indicates whether the button is \e checked.  Only
checkable buttons can be checked and unchecked (see below).

\i isEnabled() indicates whether the button can be pressed by the
user.

\i setAutoRepeat() sets whether the button will auto-repeat if the
user holds it down.

\i setCheckable() sets whether the button is a toggle button or not.

\endlist

The difference between isDown() and isChecked() is as follows.
When the user clicks a toggle button to check it, the button is first
\e pressed then released into the \e checked state. When the user
clicks it again (to uncheck it), the button moves first to the
\e pressed state, then to the \e unchecked state (isChecked() and
isDown() are both false).

QButton provides five signals:

\list 1

\i pressed() is emitted when the left mouse button is pressed while
the mouse cursor is inside the button.

\i released() is emitted when the left mouse button is released.

\i clicked() is emitted when the button is first pressed and then
released when the shortcut key is typed, or when animateClick()
is called.

\i toggled(bool) is emitted when the state of a toggle button
changes.

\endlist

To subclass QAbstractButton, you must reimplement at least
paintEvent() to draw the button's outline and its text or pixmap. It
is generally advisable to reimplement sizeHint() as well, and
sometimes hitButton() (to determine whether a button press is within
the button). For buttons with more than two states (like tri-state
buttons), you will also have to reimplement checkStateSet() and
nextCheckState().

\sa QButtonGroup
*/

class QButtonGroupPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QButtonGroup)

public:
    QButtonGroupPrivate():exclusive(true){}
    QList<QAbstractButton *> buttonList;
    QPointer<QAbstractButton> checkedButton;
    void notifyChecked(QAbstractButton *button);
    bool exclusive;
};


#define d d_func()
#define q q_func()

QAbstractButtonPrivate::QAbstractButtonPrivate()
    :shortcutId(0), checkable(false), checked(false), autoRepeat(false), autoExclusive(false),
     down(false), mlbDown(false), blockRefresh(false), group(0)
{}

QButtonGroup::QButtonGroup(QObject *parent)
    : QObject(*new QButtonGroupPrivate, parent)
{
}

QButtonGroup::~QButtonGroup()
{
    for (int i = 0; i < d->buttonList.count(); ++i)
        d->buttonList.at(i)->d->group = 0;
}


bool QButtonGroup::exclusive() const
{
    return d->exclusive;
}

void QButtonGroup::setExclusive(bool exclusive)
{
    d->exclusive = exclusive;
}

void QButtonGroup::addButton(QAbstractButton *button)
{
    if (QButtonGroup *previous = button->d->group)
        if (previous && previous != this)
            previous->removeButton(button);
    button->d->group = this;
    d->buttonList.append(button);
    if (d->exclusive && button->isChecked())
        button->d->notifyChecked();
}

void QButtonGroup::removeButton(QAbstractButton *button)
{
    if (d->checkedButton == button)
        d->checkedButton = 0;
    if (button->d->group == this) {
        button->d->group = 0;
        d->buttonList.removeAll(button);
    }
}

/*!
    Returns the number of buttons in the group.
*/
int QButtonGroup::count() const
{
    return d->buttonList.count();
}

QAbstractButton *QButtonGroup::checkedButton() const
{
    return d->checkedButton;
}

QList<QAbstractButton *>QAbstractButtonPrivate::queryButtonList() const
{
    if (group)
        return group->d->buttonList;
    QList<QAbstractButton*>candidates;
    if (q->parentWidget() && autoExclusive) {
        candidates =  qFindChildren<QAbstractButton *>(q->parentWidget());
        for (int i = candidates.count() - 1; i >= 0; --i) {
            QAbstractButton *candidate = candidates.at(i);
            if (!candidate->autoExclusive() || candidate->group())
                candidates.removeAt(i);
        }
    }
    return candidates;
}

QAbstractButton *QAbstractButtonPrivate::queryCheckedButton() const
{
    if (group)
        return group->d->checkedButton;
    QList<QAbstractButton *> buttonList = queryButtonList();
    if (buttonList.count() == 1) // no group
        return 0;

    for (int i = 0; i < buttonList.count(); ++i) {
        QAbstractButton *b = buttonList.at(i);
        if (b->d->checked && b != q)
            return b;
    }
    return checked  ? const_cast<QAbstractButton *>(q) : 0;
}

void QAbstractButtonPrivate::notifyChecked()
{
    if (group) {
        QAbstractButton *previous = group->d->checkedButton;
        group->d->checkedButton = q;
        if (group->d->exclusive && previous && previous != q)
            previous->setChecked(false);
        emit group->buttonChecked(q);
    } else if (autoExclusive) {
        if (QAbstractButton *b = queryCheckedButton())
            b->setChecked(false);
        }
}

void QAbstractButtonPrivate::moveFocus(int key)
{
    QList<QAbstractButton *> buttonList = queryButtonList();;
    bool exclusive = group ? group->d->exclusive : autoExclusive;
    QWidget *f = qApp->focusWidget();
    QAbstractButton *fb = ::qt_cast<QAbstractButton *>(f);
    if (!fb || !buttonList.contains(fb))
        return;

    QAbstractButton *candidate = 0;
    int bestScore = -1;
    QRect fGeometry = f->geometry();

    QPoint goal(f->mapToGlobal(fGeometry.center()));

    for (int i = 0; i < buttonList.count(); ++i) {
        QAbstractButton *button = buttonList.at(i);
        if (button != f && button->isEnabled()) {
            QRect buttonGeometry = button->geometry();
            QPoint p(button->mapToGlobal(buttonGeometry.center()));
            int score = (p.y() - goal.y())*(p.y() - goal.y()) +
                        (p.x() - goal.x())*(p.x() - goal.x());
            bool betterScore = score < bestScore || !candidate;
            switch(key) {
            case Qt::Key_Up:
                if (p.y() < goal.y() && betterScore) {
                    if (QABS(p.x() - goal.x()) < QABS(p.y() - goal.y())) {
                        candidate = button;
                        bestScore = score;
                    } else if (buttonGeometry.x() == fGeometry.x()) {
                        candidate = button;
                        bestScore = score/2;
                    }
                }
                break;
            case Qt::Key_Down:
                if (p.y() > goal.y() && betterScore) {
                    if (QABS(p.x() - goal.x()) < QABS(p.y() - goal.y())) {
                        candidate = button;
                        bestScore = score;
                    } else if (buttonGeometry.x() == fGeometry.x()) {
                        candidate = button;
                        bestScore = score/2;
                    }
                }
                break;
            case Qt::Key_Left:
                if (p.x() < goal.x() && betterScore) {
                    if (QABS(p.y() - goal.y()) < QABS(p.x() - goal.x())) {
                        candidate = button;
                        bestScore = score;
                    } else if (buttonGeometry.y() == fGeometry.y()) {
                        candidate = button;
                        bestScore = score/2;
                    }
                }
                break;
            case Qt::Key_Right:
                if (p.x() > goal.x() && betterScore) {
                    if (QABS(p.y() - goal.y()) < QABS(p.x() - goal.x())) {
                        candidate = button;
                        bestScore = score;
                    } else if (buttonGeometry.y() == fGeometry.y()) {
                        candidate = button;
                        bestScore = score/2;
                    }
                }
                break;
            }
        }
    }

    if (exclusive
        && candidate
        && fb->d->checked
        && candidate->d->checkable)
        candidate->setChecked(true);

    if (candidate) {
        if (key == Qt::Key_Up || key == Qt::Key_Left)
            QFocusEvent::setReason(QFocusEvent::Backtab);
        else
            QFocusEvent::setReason(QFocusEvent::Tab);
        candidate->setFocus();
        QFocusEvent::resetReason();
    }
}

void QAbstractButtonPrivate::fixFocusPolicy()
{
    QList<QAbstractButton *> buttonList = queryButtonList();;
    for (int i = 0; i < buttonList.count(); ++i) {
        QAbstractButton *b = buttonList.at(i);
        if (!b->isCheckable())
            continue;
        b->setFocusPolicy((Qt::FocusPolicy) ((b == q)
                                         ? (b->focusPolicy() | Qt::TabFocus)
                                         :  (b->focusPolicy() & ~Qt::TabFocus)));
    }
}

void QAbstractButtonPrivate::init()
{
    q->setFocusPolicy(Qt::StrongFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void QAbstractButtonPrivate::refresh()
{
    if (blockRefresh)
        return;
    if (q->autoMask())
        q->updateMask();
    q->repaint();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(q, 0, QAccessible::StateChanged);
#endif
}

void QAbstractButtonPrivate::click()
{
    d->down = false;
    if (checkable) {
        blockRefresh = true;
        q->nextCheckState();
        blockRefresh = false;
    }
    refresh();
    emit q->released();
    emit q->clicked();
}


/*!
    Constructs an abstract button with a \a parent.
*/
QAbstractButton::QAbstractButton(QWidget *parent)
    :QWidget(*new QAbstractButtonPrivate, parent, 0)
{
    d->init();
}

/*!
    Destroys the button.
 */
 QAbstractButton::~QAbstractButton()
{
    if (d->group)
        d->group->removeButton(this);
}


/*! \internal
 */
QAbstractButton::QAbstractButton(QAbstractButtonPrivate &dd, QWidget *parent)
    :QWidget(dd, parent, 0)
{
    d->init();
}

/*!
\property QAbstractButton::text
\brief the text shown on the button

This property will return a an empty string if the button has no text.
If the text contains an ampersand character (\&), a mnemonic is
automatically created for it. The character that follows the '\&' will be
used as the shortcut key. Any previous mnemonic will be overwritten,
or cleared if no mnemonic is defined by the text.

There is no default text.
*/

void QAbstractButton::setText(const QString &text)
{
    if (d->text == text)
        return;
    d->text = text;
    QKeySequence newMnemonic = QKeySequence::mnemonic(text);
    if (!newMnemonic.isEmpty()) {
        releaseShortcut(d->shortcutId);
        d->shortcutId = grabShortcut(newMnemonic);
    }
    if (q->autoMask())
        q->updateMask();
    update();
    updateGeometry();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(q, 0, QAccessible::NameChanged);
#endif
}

QString QAbstractButton::text() const
{
    return d->text;
}


/*!
  \property QAbstractButton::icon
  \brief the icon shown on the button
*/
void QAbstractButton::setIcon(const QIconSet &icon)
{
    d->icon = icon;
    if (autoMask())
        updateMask();
    update();
    updateGeometry();
}

QIconSet QAbstractButton::icon() const
{
    return d->icon;
}


/*!
\property QAbstractButton::shortcut
\brief the mnemonic associated with the button
*/

void QAbstractButton::setShortcut(const QKeySequence &key)
{
    if (d->shortcutId != 0)
        releaseShortcut(d->shortcutId);
    d->shortcut = key;
    d->shortcutId = grabShortcut(key);
}

QKeySequence QAbstractButton::shortcut() const
{
    return d->shortcut;
}
/*!
\property QAbstractButton::checkable
\brief whether the button is checkable

By default, this is false (the button is not checkable).

\sa checked
*/
void QAbstractButton::setCheckable(bool checkable)
{
    d->checkable = checkable;
}

bool QAbstractButton::isCheckable() const
{
    return d->checkable;
}

/*!
\property QAbstractButton::checked
\brief whether the button is checked

Only checkable buttons can be checked.  By default, this is false
(the button is unchecked).

\sa checkable
*/
void QAbstractButton::setChecked(bool checked)
{
    if (!d->checkable || d->checked == checked) {
        if (!d->blockRefresh)
            checkStateSet();
        return;
    }

    if (!checked && d->queryCheckedButton() == this) {
            // the checked button of an exclusive or autoexclusive group cannot be  unchecked
            if ((d->group && d->group->d->exclusive) || d->autoExclusive )
                return;
    }

    d->checked = checked;
    if (!d->blockRefresh)
        checkStateSet();
    d->refresh();

    if (checked)
        d->notifyChecked();
    emit toggled(checked);
}

bool QAbstractButton::isChecked() const
{
    return d->checked;
}

/*!
  \property QAbstractButton::down
  \brief whether the button is pressed down

  If this property is true, the button is pressed down. The signals
  pressed() and clicked() are not emitted if you set this property
  to true. The default is false.
*/

void QAbstractButton::setDown(bool down)
{
    if (d->down == down)
        return;
    d->down = down;
    d->refresh();
    if (d->autoRepeat && d->down)
        d->repeatTimer.start(AUTO_REPEAT_DELAY, this);
    else
        d->repeatTimer.stop();
}

bool QAbstractButton::isDown() const
{
    return d->down;
}

/*!
\property QAbstractButton::autoRepeat
\brief whether autoRepeat is enabled

If autoRepeat is enabled then the clicked() signal is emitted at
regular intervals when the button is down. This property has no
effect on toggle buttons. autoRepeat is off by default.
*/

void QAbstractButton::setAutoRepeat(bool autoRepeat)
{
    if (d->autoRepeat == autoRepeat)
        return;
    d->autoRepeat = autoRepeat;
    if (d->autoRepeat && d->mlbDown && d->down)
        d->repeatTimer.start(AUTO_REPEAT_DELAY, this);
    else
        d->repeatTimer.stop();
}

bool QAbstractButton::autoRepeat() const
{
    return d->autoRepeat;
}

/*!
\property QAbstractButton::autoExclusive
\brief whether autoExclusive is enabled

If autoExclusive is enabled, checkable buttons that belong to the
same parent widget behave as if they were part of the same
exclusive button group. In an exclusive button group, only one button
can be checked at any time; checking another button automatically
unchecks the previously checked one.

The property has no effect on buttons that belong to a button
group.

autoExclusive is off by default, except for radio buttons.

\sa QRadioButton
*/
void QAbstractButton::setAutoExclusive(bool autoExclusive)
{
    d->autoExclusive = autoExclusive;
}

bool QAbstractButton::autoExclusive() const
{
    return d->autoExclusive;
}

/*!
  Returns the group that this button belongs to.

  If the button is not a member of any QButtonGroup, this function
  returns 0.

  \sa QButtonGroup
*/
QButtonGroup *QAbstractButton::group() const
{
    return d->group;
}

/*!
Performs an animated click: the button is pressed and released
\a msec milliseconds later (the default is 100 msecs).

All signals associated with a click are emitted as appropriate.

This function does nothing if the button is \link setEnabled()
disabled. \endlink

\sa click()
*/
void QAbstractButton::animateClick(int msec)
{
    if (!isEnabled())
        return;
    if (d->checkable)
        setFocus();
    setDown(true);
    emit pressed();
    d->animateTimer.start(msec, this);
}

/*!
Performs a click.

All the usual signals associated with a click are emitted as appropriate.

This function does nothing if the button is \link setEnabled()
disabled. \endlink

\sa animateClick()
 */
void QAbstractButton::click()
{
    if (!isEnabled())
        return;
    d->down = true;
    emit pressed();
    d->down = false;
    if (d->checkable)
        nextCheckState();
    emit released();
    emit clicked();
}

/*!
    Toggles the state of a checkable button.

    \sa checked
*/
void QAbstractButton::toggle()
{
    setChecked(!d->checked);
}

/*! This virtual handler is called when setChecked() was called,
unless it was called from within nextCheckState(). It allows
subclasses to reset their intermediate button states.

\sa nextCheckState()
 */
void QAbstractButton::checkStateSet()
{
}

/*! This virtual handler is called when a checkable button is
clicked. The default implementation calls setChecked(!isChecked()).
It allows subclasses to implement intermediate button states.

\sa checkStateSet()
*/
void QAbstractButton::nextCheckState()
{
    setChecked(!isChecked());
}

/*!
Returns true if \a pos is inside the clickable button rectangle;
otherwise returns false.

By default, the clickable area is the entire widget. Subclasses
may reimplement this function to provide support for clickable
areas of different shapes and sizes.
*/
bool QAbstractButton::hitButton(const QPoint &pos) const
{
    return rect().contains(pos);
}

/*! \reimp */
bool QAbstractButton::event(QEvent *e)
{
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (d->shortcutId != se->shortcutId())
            return false;
        if (se->isAmbiguous())
            setFocus();
        else
            animateClick();
        return true;
    }
    return QWidget::event(e);
}

/*! \reimp */
void QAbstractButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    if (hitButton(e->pos())) {
        d->mlbDown = true;
        setDown(true);
        emit pressed();
    }
}

/*! \reimp */
void QAbstractButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        // clean up apperance if left button has been pressed
        if (d->mlbDown || d->down) {
            d->mlbDown = false;
            setDown(false);
        }
        e->ignore();
        return;
    }

    if (!d->mlbDown)
        return;
    d->mlbDown = false;

    if (!d->down)
        return;

    if (hitButton(e->pos()))
        d->click();
    else
        setDown(false);
}

/*! \reimp */
void QAbstractButton::mouseMoveEvent(QMouseEvent *e)
{
    if (!d->mlbDown || (e->state() & Qt::LeftButton) == 0) {
        e->ignore();
        return;
    }

    if (hitButton(e->pos()) != d->down) {
        setDown(!d->down);
        if (d->down)
            emit pressed();
        else
            emit released();
    }
}

/*! \reimp */
void QAbstractButton::keyPressEvent(QKeyEvent *e)
{
    bool next = true;
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        e->ignore();
        break;
    case Qt::Key_Space:
        if (!e->isAutoRepeat()) {
            setDown(true);
            emit pressed();
        }
        break;
    case Qt::Key_Up:
    case Qt::Key_Left:
        next = false;
        // fall through
    case Qt::Key_Right:
    case Qt::Key_Down:
        if (d->group || d->autoExclusive) {
            d->moveFocus(e->key());
            if (hasFocus()) // nothing happend, propagate
                e->ignore();
        } else {
            QFocusEvent::setReason(next ? QFocusEvent::Tab : QFocusEvent::Backtab);
            focusNextPrevChild(next);
            QFocusEvent::resetReason();
        }
        break;
    case Qt::Key_Escape:
        if (d->down) {
            setDown(false);
            emit released();
            break;
        }
        // fall through
    default:
        e->ignore();
    }
}

/*! \reimp */
void QAbstractButton::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Space:
        if (!e->isAutoRepeat() && d->down)
            d->click();
        break;
    default:
        e->ignore();
    }
}

/*!\reimp
 */
void QAbstractButton::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->repeatTimer.timerId()) {
        d->repeatTimer.start(AUTO_REPEAT_PERIOD, this);
        if (d->mlbDown && d->down) {
            emit released();
            emit clicked();
            emit pressed();
        }
    } else if (e->timerId() == d->animateTimer.timerId()) {
        d->animateTimer.stop();
        d->click();
    }
}

/*! \reimp */
void QAbstractButton::focusInEvent(QFocusEvent *e)
{
    d->fixFocusPolicy();
    QWidget::focusInEvent(e);
}

/*! \reimp */
void QAbstractButton::focusOutEvent(QFocusEvent *e)
{
    d->down = false;
    QWidget::focusOutEvent(e);
}

/*! \reimp */
void QAbstractButton::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::EnabledChange:
        if (!isEnabled())
            setDown(false);
        break;
    case QEvent::PaletteChange:
        d->icon.clearGenerated();
    default:
        ;
    }
    QWidget::changeEvent(e);
}

/*!
    \fn void QAbstractButton::paintEvent(QPaintEvent *e)
    \reimp
*/

/*!
    \fn void QAbstractButton::pressed()

    This signal is emitted when the button is pressed down.

    \sa released(), clicked()
*/

/*!
    \fn void QAbstractButton::released()

    This signal is emitted when the button is released.

    \sa pressed(), clicked(), toggled()
*/

/*!
\fn void QAbstractButton::clicked()

This signal is emitted when the button is activated (i.e. pressed down
then released while the mouse cursor is inside the button), when the
shortcut key is typed, or when animateClick() is called.
This signal is \e not emitted if you call setDown().

\sa pressed(), released(), toggled()
*/

/*!
\fn void QAbstractButton::toggled(bool checked)

This signal is emitted whenever a toggle button changes its state.
\a checked is true if the button is checked, or false if the button
is unchecked.

This may be the result of a user action, toggle() slot activation,
or because setChecked() was called.

\sa clicked()
*/


#ifdef QT_COMPAT
/*!
    Use icon() instead.
*/
QIconSet *QAbstractButton::iconSet() const
{
    if (!d->icon.isNull())
        return const_cast<QIconSet *>(&d->icon);
    return 0;
}

/*!
    Use QAbstractButton(QWidget *) instead.

    Call setObjectName() if you want to specify an object name, and
    setParent() if you want to set the window flags.
*/
QAbstractButton::QAbstractButton(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(*new QAbstractButtonPrivate, parent, f)
{
    setObjectName(name);
    d->init();
}

/*! \fn bool QAbstractButton::isOn() const

    Use isChecked() instead.
*/

/*! \fn QPixmap *QAbstractButton::pixmap() const

    This compatibility function always returns 0.

    Use icon() instead.
*/

/*! \fn void QAbstractButton::setPixmap(const QPixmap &p)

    Use setIcon() instead.
*/

/*! \fn void QAbstractButton::setIconSet(const QIconSet &icon)

    Use setIcon() instead.
*/

/*! \fn void QAbstractButton::setOn(bool b)

    Use setChecked() instead.
*/

/*! \fn bool QAbstractButton::isToggleButton() const

    Use isCheckable() instead.
*/

/*! 
    \fn void QAbstractButton::setToggleButton(bool b)

    Use setCheckable() instead.
*/

/*! \fn void QAbstractButton::setAccel(const QKeySequence &key)

    Use setShortcut() instead.
*/

/*! \fn QKeySequence QAbstractButton::accel() const

    Use shortcut() instead.
*/

#endif
