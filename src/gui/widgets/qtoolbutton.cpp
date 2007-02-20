/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtoolbutton.h"
#ifndef QT_NO_TOOLBUTTON

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qicon.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qvariant.h>
#include <qstylepainter.h>
#include <private/qabstractbutton_p.h>
#include <private/qaction_p.h>
#include <private/qmenu_p.h>

class QToolButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QToolButton)
public:
    void init();
#ifndef QT_NO_MENU
    void _q_buttonPressed();
    void popupTimerDone();
    void _q_updateButtonDown();
#endif
    void _q_actionTriggered();
    QPointer<QAction> menuAction; //the menu set by the user (setMenu)
    QBasicTimer popupTimer;
    int delay;
    Qt::ArrowType arrowType;
    Qt::ToolButtonStyle toolButtonStyle;
    QToolButton::ToolButtonPopupMode popupMode;
    enum { NoButtonPressed=0, MenuButtonPressed=1, ToolButtonPressed=2 };
    uint buttonPressed : 2;
    uint menuButtonDown          : 1;
    uint autoRaise             : 1;
    uint repeat                : 1;
    QAction *defaultAction;
#ifndef QT_NO_MENU
    bool hasMenu() const;
#endif
#ifdef QT3_SUPPORT
    bool userDefinedPopupDelay;
#endif
};

#ifndef QT_NO_MENU
bool QToolButtonPrivate::hasMenu() const
{
    Q_Q(const QToolButton);
    return ((defaultAction && defaultAction->menu())
            || (menuAction && menuAction->menu())
            || q->actions().size() > (defaultAction ? 1 : 0));
}
#endif

/*!
    \class QToolButton qtoolbutton.h
    \brief The QToolButton class provides a quick-access button to
    commands or options, usually used inside a QToolBar.

    \ingroup basic
    \mainclass

    A tool button is a special button that provides quick-access to
    specific commands or options. As opposed to a normal command
    button, a tool button usually doesn't show a text label, but shows
    an icon instead.

    Tool buttons are normally created when new QAction instances are
    created with QToolBar::addAction() or existing actions are added
    to a toolbar with QToolBar::addAction(). It is also possible to
    construct tool buttons in the same way as any other widget, and
    arrange them alongside other widgets in layouts.

    One classic use of a tool button is to select tools; for example,
    the "pen" tool in a drawing program. This would be implemented
    by using a QToolButton as a toggle button (see setToggleButton()).

    QToolButton supports auto-raising. In auto-raise mode, the button
    draws a 3D frame only when the mouse points at it. The feature is
    automatically turned on when a button is used inside a QToolBar.
    Change it with setAutoRaise().

    A tool button's icon is set as QIcon. This makes it possible to
    specify different pixmaps for the disabled and active state. The
    disabled pixmap is used when the button's functionality is not
    available. The active pixmap is displayed when the button is
    auto-raised because the mouse pointer is hovering over it.

    The button's look and dimension is adjustable with
    setToolButtonStyle() and setIconSize(). When used inside a
    QToolBar in a QMainWindow, the button automatically adjusts to
    QMainWindow's settings (see QMainWindow::setToolButtonStyle() and
    QMainWindow::setIconSize()). Instead of an icon, a tool button can
    also display an arrow symbol, specified with \l arrowType.

    A tool button can offer additional choices in a popup menu. The
    popup menu can be set using setMenu(). Use setPopupMode() to
    configure the different modes available for tool buttons with a
    menu set. The default mode is DelayedPopupMode which is sometimes
    used with the "Back" button in a web browser.  After pressing and
    holding the button down for a while, a menu pops up showing a list
    of possible pages to jump to. The default delay is 600 ms; you can
    adjust it with setPopupDelay().

    \table 100%
    \row \o \inlineimage assistant-toolbar1.png Qt Assistant's toolbar with tool buttons
    \row \o Qt Assistant's toolbar contains tool buttons that are associated
         with actions used in other parts of the main window.
    \endtable

    \sa QPushButton, QToolBar, QMainWindow, QAction,
        {fowler}{GUI Design Handbook: Push Button}
*/

/*!
    \fn void QToolButton::triggered(QAction *action)

    This signal is emitted when the given \a action is triggered.

    The action may also be associated with other parts of the user interface,
    such as menu items and keyboard shortcuts. Sharing actions in this
    way helps make the user interface more consistent and is often less work
    to implement.
*/

/*!
    Constructs an empty tool button with parent \a
    parent.
*/
QToolButton::QToolButton(QWidget * parent)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    Q_D(QToolButton);
    d->init();
}

#ifdef QT3_SUPPORT
/*!
    Constructs an empty tool button called \a name, with parent \a
    parent.
*/

QToolButton::QToolButton(QWidget * parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    Q_D(QToolButton);
    setObjectName(QString::fromAscii(name));
    d->init();
}

/*!
    Constructs a tool button called \a name, that is a child of \a
    parent.

    The tool button will display the given \a icon, with its text
    label and tool tip set to \a textLabel and its status bar message
    set to \a statusTip. It will be connected to the \a slot in
    object \a receiver.
*/

QToolButton::QToolButton(const QIcon& icon, const QString &textLabel,
                         const QString& statusTip,
                         QObject * receiver, const char *slot,
                         QWidget * parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    Q_D(QToolButton);
    setObjectName(QString::fromAscii(name));
    d->init();
    setIcon(icon);
    setText(textLabel);
    if (receiver && slot)
        connect(this, SIGNAL(clicked()), receiver, slot);
#ifndef QT_NO_TOOLTIP
    if (!textLabel.isEmpty())
        setToolTip(textLabel);
#endif
#ifndef QT_NO_STATUSTIP
    if (!statusTip.isEmpty())
        setStatusTip(statusTip);
#else
    Q_UNUSED(statusTip);
#endif
}


/*!
    Constructs a tool button as an arrow button. The Qt::ArrowType \a
    type defines the arrow direction. Possible values are
    Qt::LeftArrow, Qt::RightArrow, Qt::UpArrow, and Qt::DownArrow.

    An arrow button has auto-repeat turned on by default.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/
QToolButton::QToolButton(Qt::ArrowType type, QWidget *parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    Q_D(QToolButton);
    setObjectName(QString::fromAscii(name));
    d->init();
    setAutoRepeat(true);
    d->arrowType = type;
}

#endif


/*  Set-up code common to all the constructors */

void QToolButtonPrivate::init()
{
    Q_Q(QToolButton);
    delay = q->style()->styleHint(QStyle::SH_ToolButton_PopupDelay, 0, q);
#ifdef QT3_SUPPORT
    userDefinedPopupDelay = false;
#endif
    defaultAction = 0;
#ifndef QT_NO_TOOLBAR
    if (qobject_cast<QToolBar*>(q->parentWidget()))
        autoRaise = true;
    else
#endif
        autoRaise = false;
    arrowType = Qt::NoArrow;
    menuButtonDown = false;
    popupMode = QToolButton::DelayedPopup;
    buttonPressed = QToolButtonPrivate::NoButtonPressed;

    toolButtonStyle = Qt::ToolButtonIconOnly;

    q->setFocusPolicy(Qt::TabFocus);
    q->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, 
                                 QSizePolicy::ToolButton));

    QObject::connect(q, SIGNAL(pressed()), q, SLOT(_q_buttonPressed()));

    setLayoutItemMargins(QStyle::SE_ToolButtonLayoutItem);

}

/*!
    Initialize \a option with the values from this QToolButton. This method
    is useful for subclasses when they need a QStyleOptionToolButton, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom()
*/
void QToolButton::initStyleOption(QStyleOptionToolButton *option) const
{
    if (!option)
        return;

    Q_D(const QToolButton);
    option->initFrom(this);
    bool forceNoText = false;

#ifndef QT_NO_TOOLBAR
    if (parentWidget()) {
#ifdef QT3_SUPPORT
        if (parentWidget()->inherits("Q3ToolBar")) {
            int iconSize = style()->pixelMetric(QStyle::PM_ToolBarIconSize, option, this);
            option->iconSize = d->icon.actualSize(QSize(iconSize, iconSize));
            forceNoText = d->toolButtonStyle == Qt::ToolButtonIconOnly;
        } else
#endif
            if (QToolBar *toolBar = qobject_cast<QToolBar *>(parentWidget())) {
                option->iconSize = toolBar->iconSize();
            } else {
                option->iconSize = iconSize();
            }
    }
#endif // QT_NO_TOOLBAR

    if (!forceNoText)
        option->text = d->text;
    option->icon = d->icon;
    option->arrowType = d->arrowType;
    if (d->down)
        option->state |= QStyle::State_Sunken;
    if (d->checked)
        option->state |= QStyle::State_On;
    if (d->autoRaise)
        option->state |= QStyle::State_AutoRaise;
    if (!d->checked && !d->down)
        option->state |= QStyle::State_Raised;

    option->subControls = QStyle::SC_ToolButton;
    option->activeSubControls = QStyle::SC_None;
//     if (d->down && !d->menuButtonDown)
//         option->activeSubControls |= QStyle::SC_ToolButton;

    option->features = QStyleOptionToolButton::None;
    if (d->popupMode == QToolButton::MenuButtonPopup) {
        option->subControls |= QStyle::SC_ToolButtonMenu;
        option->features |= QStyleOptionToolButton::Menu;
        if (d->menuButtonDown || d->down) {
            option->state |= QStyle::State_MouseOver;
            option->activeSubControls |= QStyle::SC_ToolButtonMenu;
        }
    } else {
        if (d->menuButtonDown)
            option->state  |= QStyle::State_Sunken;
        if (d->hasMenu())
            option->features |= QStyleOptionToolButton::Menu;
    }
    if (d->arrowType != Qt::NoArrow)
        option->features |= QStyleOptionToolButton::Arrow;
    if (d->popupMode == QToolButton::DelayedPopup)
        option->features |= QStyleOptionToolButton::PopupDelay;
    option->toolButtonStyle = d->toolButtonStyle;
    if (d->icon.isNull() && d->arrowType == Qt::NoArrow && !forceNoText) {
        if (!d->text.isEmpty())
            option->toolButtonStyle = Qt::ToolButtonTextOnly;
        else if (option->toolButtonStyle != Qt::ToolButtonTextOnly)
            option->toolButtonStyle = Qt::ToolButtonIconOnly;
    } else {
        if (d->text.isEmpty() && option->toolButtonStyle != Qt::ToolButtonIconOnly)
            option->toolButtonStyle = Qt::ToolButtonIconOnly;
    }

    option->pos = pos();
    option->font = font();
}

/*!
    Destroys the object and frees any allocated resources.
*/

QToolButton::~QToolButton()
{
}

/*!
    \reimp
*/
QSize QToolButton::sizeHint() const
{
    Q_D(const QToolButton);
    ensurePolished();

    int w = 0, h = 0;
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QFontMetrics fm = fontMetrics();
    if (opt.toolButtonStyle != Qt::ToolButtonTextOnly) {
        QSize icon = opt.iconSize;
        w = icon.width();
        h = icon.height();
    }

    if (opt.toolButtonStyle != Qt::ToolButtonIconOnly) {
        QSize textSize = fm.size(Qt::TextShowMnemonic, text());
        textSize.setWidth(textSize.width() + fm.width(QLatin1Char(' '))*2);
        if (opt.toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
            h += 4 + textSize.height();
            if (textSize.width() > w)
                w = textSize.width();
        } else if (opt.toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
            w += 4 + textSize.width();
            if (textSize.height() > h)
                h = textSize.height();
        } else { // TextOnly
            w = textSize.width();
            h = textSize.height();
        }
    }

    opt.rect.setHeight(h); // PM_MenuButtonIndicator depends on the height
    if (d->popupMode == MenuButtonPopup)
        w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);

    return style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(w, h), this).
            expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
 */
QSize QToolButton::minimumSizeHint() const
{
    return sizeHint();
}

/*!
    \enum QToolButton::TextPosition
    \compat

    This enum describes the position of the tool button's text label in
    relation to the tool button's icon.

    \value BesideIcon The text appears beside the icon.
    \value BelowIcon The text appears below the icon.
    \omitvalue Right
    \omitvalue Under
*/

/*!
    \property QToolButton::toolButtonStyle
    \brief whether the tool button displays an icon only, text only,
    or text beside/below the icon.

    The default is Qt::ToolButtonIconOnly.

    QToolButton automatically connects this slot to the relevant
    signal in the QMainWindow in which is resides.
*/

/*!
    \property QToolButton::arrowType
    \brief whether the button displays an arrow instead of a normal icon

    This displays an arrow as the icon for the QToolButton.
*/

Qt::ToolButtonStyle QToolButton::toolButtonStyle() const
{
    Q_D(const QToolButton);
    return d->toolButtonStyle;
}

Qt::ArrowType QToolButton::arrowType() const
{
    Q_D(const QToolButton);
    return d->arrowType;
}


void QToolButton::setToolButtonStyle(Qt::ToolButtonStyle style)
{
    Q_D(QToolButton);
    if (d->toolButtonStyle == style)
        return;

    d->toolButtonStyle = style;
    updateGeometry();
    if (isVisible()) {
        update();
    }
}

void QToolButton::setArrowType(Qt::ArrowType type)
{
    Q_D(QToolButton);
    if (d->arrowType == type)
        return;

    d->arrowType = type;
    updateGeometry();
    if (isVisible()) {
        update();
    }
}

/*!
    \fn void QToolButton::paintEvent(QPaintEvent *event)

    Paints the button in response to the paint \a event.
*/
void QToolButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

/*!
    \reimp
 */
void QToolButton::actionEvent(QActionEvent *event)
{
    Q_D(QToolButton);
    QAction *action = event->action();
    switch (event->type()) {
    case QEvent::ActionChanged:
        if (action == d->defaultAction)
            setDefaultAction(action); // update button state
        break;
    case QEvent::ActionAdded:
        connect(action, SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
        break;
    case QEvent::ActionRemoved:
        if (d->defaultAction == action)
            d->defaultAction = 0;
#ifndef QT_NO_MENU
        if (action == d->menuAction)
            d->menuAction = 0;
#endif
        action->disconnect(this);
        break;
    default:
        ;
    }
    QAbstractButton::actionEvent(event);
}

void QToolButtonPrivate::_q_actionTriggered()
{
    Q_Q(QToolButton);
    if (QAction *action = qobject_cast<QAction *>(q->sender()))
        emit q->triggered(action);
}

/*!
    \reimp
 */
void QToolButton::enterEvent(QEvent * e)
{
    Q_D(QToolButton);
    if (d->autoRaise)
        update();
    if (d->defaultAction)
        d->defaultAction->hover();
    QAbstractButton::enterEvent(e);
}


/*!
    \reimp
 */
void QToolButton::leaveEvent(QEvent * e)
{
    Q_D(QToolButton);
    if (d->autoRaise)
        update();

    QAbstractButton::leaveEvent(e);
}


/*!
    \reimp
 */
void QToolButton::timerEvent(QTimerEvent *e)
{
#ifndef QT_NO_MENU
    Q_D(QToolButton);
    if (e->timerId() == d->popupTimer.timerId()) {
        d->popupTimerDone();
        return;
    }
#endif
    QAbstractButton::timerEvent(e);
}


/*!
    \reimp
*/
void QToolButton::changeEvent(QEvent *e)
{
#ifndef QT_NO_TOOLBAR
    Q_D(QToolButton);
    if (e->type() == QEvent::ParentChange) {
        if (qobject_cast<QToolBar*>(parentWidget()))
            d->autoRaise = true;
    } else if (e->type() == QEvent::StyleChange
#ifdef Q_WS_MAC
               || e->type() == QEvent::MacSizeChange
#endif
               ) {
#ifdef QT3_SUPPORT
        if (!d->userDefinedPopupDelay)
#endif
        d->delay = style()->styleHint(QStyle::SH_ToolButton_PopupDelay, 0, this);
        d->setLayoutItemMargins(QStyle::SE_ToolButtonLayoutItem);
    }
#endif
    QAbstractButton::changeEvent(e);
}

/*!
    \reimp
*/
void QToolButton::mousePressEvent(QMouseEvent *e)
{
    Q_D(QToolButton);
#ifndef QT_NO_MENU
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    if (e->button() == Qt::LeftButton && d->popupMode == MenuButtonPopup) {
        QRect popupr = style()->subControlRect(QStyle::CC_ToolButton, &opt,
                                               QStyle::SC_ToolButtonMenu, this);
        if (popupr.isValid() && popupr.contains(e->pos())) {
            d->buttonPressed = QToolButtonPrivate::MenuButtonPressed;
            showMenu();
            return;
        }
    }
#endif
    d->buttonPressed = QToolButtonPrivate::ToolButtonPressed;
    QAbstractButton::mousePressEvent(e);
}

/*!
    \reimp
*/
void QToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QToolButton);
    QAbstractButton::mouseReleaseEvent(e);
    d->buttonPressed = QToolButtonPrivate::NoButtonPressed;
}

/*!
    \reimp
*/
bool QToolButton::hitButton(const QPoint &pos) const
{
    Q_D(const QToolButton);
    if(QAbstractButton::hitButton(pos))
        return (d->buttonPressed != QToolButtonPrivate::MenuButtonPressed);
    return false;
}

#ifdef QT3_SUPPORT

/*!
    Use icon() instead.
*/
QIcon QToolButton::onIconSet() const
{
    return icon();
}

/*!
    Use icon() instead.
*/
QIcon QToolButton::offIconSet() const
{
    return icon();
}


/*!
  \obsolete

  Use setIcon() instead.
*/
void QToolButton::setOnIconSet(const QIcon& set)
{
    setIcon(set);
}

/*!
  \obsolete

  Use setIcon() instead.
*/
void QToolButton::setOffIconSet(const QIcon& set)
{
    setIcon(set);
}


/*! \overload
    \obsolete

  Since Qt 3.0, QIcon contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  sets the \l icon property. If you relied on the \a on parameter,
  you probably want to update your code to use the QIcon On/Off
  mechanism.

  \sa icon QIcon::State
*/

void QToolButton::setIconSet(const QIcon & set, bool /* on */)
{
    QAbstractButton::setIcon(set);
}

/*! \overload
    \obsolete

  Since Qt 3.0, QIcon contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  returns the \l icon property. If you relied on the \a on
  parameter, you probably want to update your code to use the QIcon
  On/Off mechanism.
*/
QIcon QToolButton::iconSet(bool /* on */) const
{
    return QAbstractButton::icon();
}

#endif

#ifndef QT_NO_MENU
/*!
    Associates the given \a menu with this tool button.

    The menu will be shown according to the button's \l popupMode.

    Ownership of the menu is not transferred to the tool button.

    \sa menu()
*/
void QToolButton::setMenu(QMenu* menu)
{
    Q_D(QToolButton);

    if (d->menuAction)
        removeAction(d->menuAction);

    if (menu) {
        d->menuAction = menu->menuAction();
        addAction(d->menuAction);
    } else {
        d->menuAction = 0;
    }
    update();
}

/*!
    Returns the associated menu, or 0 if no menu has been defined.

    \sa setMenu()
*/
QMenu* QToolButton::menu() const
{
    Q_D(const QToolButton);
    if (d->menuAction)
        return d->menuAction->menu();
    return 0;
}

/*!
    Shows (pops up) the associated popup menu. If there is no such
    menu, this function does nothing. This function does not return
    until the popup menu has been closed by the user.
*/
void QToolButton::showMenu()
{
    Q_D(QToolButton);
    if (!d->hasMenu()) {
        d->menuButtonDown = false;
        return; // no menu to show
    }

    d->menuButtonDown = true;
    repaint();
    d->popupTimer.stop();
    d->popupTimerDone();
}

void QToolButtonPrivate::_q_buttonPressed()
{
    Q_Q(QToolButton);
    if (!hasMenu())
        return; // no menu to show

    if (delay > 0 && popupMode == QToolButton::DelayedPopup)
        popupTimer.start(delay, q);
    else if  (popupMode == QToolButton::InstantPopup)
        q->showMenu();
}

void QToolButtonPrivate::popupTimerDone()
{
    Q_Q(QToolButton);
    popupTimer.stop();
    if (!menuButtonDown && !down)
        return;

    menuButtonDown = true;
    QPointer<QMenu> actualMenu;
    bool mustDeleteActualMenu = false;
    if(menuAction) {
        actualMenu = menuAction->menu();
        if (q->actions().size() > 1)
            qWarning("QToolButton: Menu in setMenu() overriding actions set in addAction!");
    } else if (defaultAction && defaultAction->menu()) {
        actualMenu = defaultAction->menu();
    } else {
        actualMenu = new QMenu(q);
        mustDeleteActualMenu = true;
        QList<QAction*> actions = q->actions();
        for(int i = 0; i < actions.size(); i++)
            actualMenu->addAction(actions.at(i));
    }
    repeat = q->autoRepeat();
    q->setAutoRepeat(false);
    bool horizontal = true;
#if !defined(QT_NO_TOOLBAR)
    QToolBar *tb = qobject_cast<QToolBar*>(q->parentWidget());
    if (tb && tb->orientation() == Qt::Vertical)
        horizontal = false;
#endif
    QPoint p;
    QRect screen = qApp->desktop()->availableGeometry(q);
    QSize sh = ((QToolButton*)(QMenu*)actualMenu)->receivers(SIGNAL(aboutToShow()))? QSize() : actualMenu->sizeHint();
    QRect rect = q->rect();
    if (horizontal) {
        if (q->isRightToLeft()) {
            if (q->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
                p = q->mapToGlobal(rect.bottomRight());
            } else {
                p = q->mapToGlobal(rect.topRight() - QPoint(0, sh.height()));
            }
            p.rx() -= sh.width();
        } else {
            if (q->mapToGlobal(QPoint(0, rect.bottom())).y() + sh.height() <= screen.height()) {
                p = q->mapToGlobal(rect.bottomLeft());
            } else {
                p = q->mapToGlobal(rect.topLeft() - QPoint(0, sh.height()));
            }
        }
    } else {
        if (q->isRightToLeft()) {
            if (q->mapToGlobal(QPoint(rect.left(), 0)).x() - sh.width() <= screen.x()) {
                p = q->mapToGlobal(rect.topRight());
            } else {
                p = q->mapToGlobal(rect.topLeft());
                p.rx() -= sh.width();
            }
        } else {
            if (q->mapToGlobal(QPoint(rect.right(), 0)).x() + sh.width() <= screen.right()) {
                p = q->mapToGlobal(rect.topRight());
            } else {
                p = q->mapToGlobal(rect.topLeft() - QPoint(sh.width(), 0));
            }
        }
    }
    p.rx() = qMax(screen.left(), qMin(p.x(), screen.right() - sh.width()));
    p.ry() += 1;
    QPointer<QToolButton> that = q;
    actualMenu->setNoReplayFor(q);
    QObject::connect(actualMenu, SIGNAL(aboutToHide()), q, SLOT(_q_updateButtonDown()));
    actualMenu->d_func()->causedPopup.widget = q;
    actualMenu->d_func()->causedPopup.action = defaultAction;
    actualMenu->exec(p);
    QObject::disconnect(actualMenu, SIGNAL(aboutToHide()), q, SLOT(_q_updateButtonDown()));
    if (mustDeleteActualMenu)
        delete actualMenu;
    if (!that)
        return;

    if (repeat)
        q->setAutoRepeat(true);
}

void QToolButtonPrivate::_q_updateButtonDown()
{
    Q_Q(QToolButton);
    menuButtonDown = false;
    if (q->isDown())
        q->setDown(false);
    else
        q->repaint();
}
#endif // QT_NO_MENU

#ifdef QT3_SUPPORT
/*!
    \fn void QToolButton::setPopupDelay(int delay)

    Use the style hint QStyle::SH_ToolButton_PopupDelay instead.
*/
void QToolButton::setPopupDelay(int delay)
{
    Q_D(QToolButton);
    d->userDefinedPopupDelay = true;
    d->delay = delay;

    update();
}

/*!
    Use the style hint QStyle::SH_ToolButton_PopupDelay instead.
*/
int QToolButton::popupDelay() const
{
    Q_D(const QToolButton);
    return d->delay;
}
#endif

#ifndef QT_NO_MENU
/*! \enum QToolButton::ToolButtonPopupMode

    Describes how a menu should be popped up for tool buttons that has
    a menu set or contains a list of actions.

    \value DelayedPopup After pressing and holding the tool button
    down for a certain amount of time (the timeout is style dependant,
    see QStyle::SH_ToolButton_PopupDelay), the menu is displayed.  A
    typical application example is the "back" button in some web
    browsers's tool bars. If the user clicks it, the browser simply
    browses back to the previous page.  If the user presses and holds
    the button down for a while, the tool button shows a menu
    containing the current history list

    \value MenuButtonPopup In this mode the tool button displays a
    special arrow to indicate that a menu is present. The menu is
    displayed when the arrow part of the button is pressed.

    \value InstantPopup The menu is displayed, without delay, when
    the tool button is pressed. In this mode, the button's own action
    is not triggered.
*/

/*!
    \property QToolButton::popupMode
    \brief describes the way that popup menus are used with tool buttons
*/

void QToolButton::setPopupMode(ToolButtonPopupMode mode)
{
    Q_D(QToolButton);
    d->popupMode = mode;
}

QToolButton::ToolButtonPopupMode QToolButton::popupMode() const
{
    Q_D(const QToolButton);
    return d->popupMode;
}
#endif

/*!
    \property QToolButton::autoRaise
    \brief whether auto-raising is enabled or not.

    The default is disabled (i.e. false).

    This property is currently ignored on Mac OS X when using QMacStyle.
*/
void QToolButton::setAutoRaise(bool enable)
{
    Q_D(QToolButton);
    d->autoRaise = enable;

    update();
}

bool QToolButton::autoRaise() const
{
    Q_D(const QToolButton);
    return d->autoRaise;
}

/*!
  Sets the default action to \a action.

  If a tool button has a default action, the action defines the
  button's properties like text, icon, tool tip, etc.
 */
void QToolButton::setDefaultAction(QAction *action)
{
    Q_D(QToolButton);
#ifndef QT_NO_MENU
    bool hadMenu = false;
    hadMenu = d->hasMenu();
#endif
    d->defaultAction = action;
    if (!action)
        return;
    if (!actions().contains(action))
        addAction(action);
    setText(action->iconText());
    setIcon(action->icon());
#ifndef QT_NO_TOOLTIP
    setToolTip(action->toolTip());
#endif
#ifndef QT_NO_STATUSTIP
    setStatusTip(action->statusTip());
#endif
#ifndef QT_NO_WHATSTHIS
    setWhatsThis(action->whatsThis());
#endif
#ifndef QT_NO_MENU
    if (action->menu() && !hadMenu) {
        // new 'default' popup mode defined introduced by tool bar. We
        // should have changed QToolButton's default instead. Do that
        // in 4.2.
        setPopupMode(QToolButton::MenuButtonPopup);
    }
#endif
    setCheckable(action->isCheckable());
    setChecked(action->isChecked());
    setEnabled(action->isEnabled());
    if (action->d_func()->fontSet)
        setFont(action->font());
}


/*!
  Returns the default action.

  \sa setDefaultAction()
 */
QAction *QToolButton::defaultAction() const
{
    Q_D(const QToolButton);
    return d->defaultAction;
}



/*!
  \reimp
 */
void QToolButton::nextCheckState()
{
    Q_D(QToolButton);
    if (!d->defaultAction)
        QAbstractButton::nextCheckState();
    else
        d->defaultAction->trigger();
}

/*! \reimp */
bool QToolButton::event(QEvent *e)
{
    return QAbstractButton::event(e);
}

/*! \internal
 */
QToolButton::QToolButton(QToolButtonPrivate &dd, QWidget *parent)
    :QAbstractButton(dd, parent)
{
    Q_D(QToolButton);
    d->init();
}

/*!
    \fn void QToolButton::setPixmap(const QPixmap &pixmap)

    Use setIcon(QIcon(pixmap)) instead.
*/

/*!
    \fn void QToolButton::setIconSet(const QIcon &icon)

    Use setIcon() instead.
*/

/*!
    \fn void QToolButton::setTextLabel(const QString &text, bool tooltip)

    Use setText() and setToolTip() instead.
*/

/*!
    \fn QString QToolButton::textLabel() const

    Use text() instead.
*/

/*!
    \fn QIcon QToolButton::iconSet() const

    Use icon() instead.
*/

/*!
    \fn void QToolButton::openPopup()

    Use showMenu() instead.
*/

/*!
    \fn void QToolButton::setPopup(QMenu* popup)

    Use setMenu() instead.
*/

/*!
    \fn QMenu* QToolButton::popup() const

    Use menu() instead.
*/

/*!
    \fn TextPosition QToolButton::textPosition() const

    Use toolButtonStyle() instead.
*/

/*!
    \fn void QToolButton::setTextPosition(QToolButton::TextPosition pos)

    Use setToolButtonStyle() instead.
*/

/*!
    \fn bool QToolButton::usesBigPixmap() const

    Use iconSize() instead.
*/

/*!
    \fn void QToolButton::setUsesBigPixmap(bool enable)

    Use setIconSize() instead.
*/

/*!
    \fn bool QToolButton::usesTextLabel() const

    Use toolButtonStyle() instead.
*/

/*!
    \fn void QToolButton::setUsesTextLabel(bool enable)

    Use setToolButtonStyle() instead.
*/
#include "moc_qtoolbutton.cpp"

#endif
