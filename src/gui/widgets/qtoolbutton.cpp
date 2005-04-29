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

class QToolButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QToolButton)
public:
    void init();
    void buttonPressed();
    void popupTimerDone();
    void actionTriggered();
    QStyleOptionToolButton getStyleOption() const;
    QPointer<QMenu> menu; //the menu set by the user (setMenu)
    QBasicTimer popupTimer;
    QSize iconSize;
    int delay;
    Qt::ArrowType arrowType;
    Qt::ToolButtonStyle toolButtonStyle;
    QToolButton::ToolButtonPopupMode popupMode;
    uint menuButtonDown          : 1;
    uint autoRaise             : 1;
    uint repeat                : 1;
    QAction *defaultAction;
    bool hasMenu() const;
};

bool QToolButtonPrivate::hasMenu() const
{
    Q_Q(const QToolButton);
    return (menu || q->actions().size() > (defaultAction ? 1 : 0));
}

/*!
    \class QToolButton qtoolbutton.h
    \brief The QToolButton class provides a quick-access button to
    commands or options, usually used inside a QToolBar.

    \ingroup basic
    \mainclass

    A tool button is a special button that provides quick-access to
    specific commands or options. As opposed to a normal command
    button, a tool button usually doesn't show a text label, but shows
    an icon instead. Its classic usage is to select tools, for example
    the "pen" tool in a drawing program. This would be implemented
    with a QToolButton as toggle button (see setToggleButton()).

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
    of possible pages to jump to. The default delay is 600ms; you can
    adjust it with setPopupDelay().

    \img qdockwindow.png Toolbar with Toolbuttons \caption A floating
    QToolbar with QToolbuttons

    \sa QPushButton QToolBar QMainWindow \link guibooks.html#fowler
    GUI Design Handbook: Push Button\endlink
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
    setObjectName(name);
    d->init();
}

/*!
    Constructs a tool button called \a name, that is a child of \a
    parent.

    The tool button will display \a iconSet, with its text label and
    tool tip set to \a textLabel and its status bar message set to \a
    statusTip. It will be connected to the \a slot in object \a
    receiver.
*/

QToolButton::QToolButton(const QIcon& iconSet, const QString &textLabel,
                          const QString& statusTip,
                          QObject * receiver, const char *slot,
                          QWidget * parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    Q_D(QToolButton);
    setObjectName(name);
    d->init();
    setIcon(iconSet);
    setText(textLabel);
    if (receiver && slot)
        connect(this, SIGNAL(clicked()), receiver, slot);
    if (!textLabel.isEmpty())
        setToolTip(textLabel);
    if (!statusTip.isEmpty())
        setStatusTip(statusTip);
}


/*!
    Constructs a tool button as an arrow button. The \c Qt::ArrowType \a
    type defines the arrow direction. Possible values are \c
    Qt::LeftArrow, \c Qt::RightArrow, \c Qt::UpArrow and \c Qt::DownArrow.

    An arrow button has auto-repeat turned on by default.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/
QToolButton::QToolButton(Qt::ArrowType type, QWidget *parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    Q_D(QToolButton);
    setObjectName(name);
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
    menu = 0;
    defaultAction = 0;
    autoRaise = false;
    arrowType = Qt::NoArrow;
    menuButtonDown = false;
    popupMode = QToolButton::DelayedPopup;

    int e = q->style()->pixelMetric(QStyle::PM_SmallIconSize);
    iconSize = QSize(e, e);

    toolButtonStyle = Qt::ToolButtonIconOnly;

    q->setFocusPolicy(Qt::TabFocus);
    q->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QObject::connect(q, SIGNAL(pressed()), q, SLOT(buttonPressed()));
}


QStyleOptionToolButton QToolButtonPrivate::getStyleOption() const
{
    Q_Q(const QToolButton);
    QStyleOptionToolButton opt;
    opt.init(q);
    bool down = q->isDown();
    bool checked = q->isChecked();
    opt.text = text;
    opt.icon = icon;
    opt.iconSize = iconSize;
    opt.arrowType = arrowType;
    if (down)
        opt.state |= QStyle::State_Sunken;
    if (checked)
        opt.state |= QStyle::State_On;
    if (autoRaise)
        opt.state |= QStyle::State_AutoRaise;
    if (!checked && !down)
        opt.state |= QStyle::State_Raised;

    opt.subControls = QStyle::SC_ToolButton;
    opt.activeSubControls = QStyle::SC_None;
//     if (down && !menuButtonDown)
//         opt.activeSubControls |= QStyle::SC_ToolButton;

    opt.features = QStyleOptionToolButton::None;
    if (popupMode == QToolButton::MenuButtonPopup) {
        opt.subControls |= QStyle::SC_ToolButtonMenu;
        opt.features |= QStyleOptionToolButton::Menu;
        if (menuButtonDown || down) {
            opt.state |= QStyle::State_MouseOver;
            opt.activeSubControls |= QStyle::SC_ToolButtonMenu;
        }
    } else {
        if (menuButtonDown)
            opt.state  |= QStyle::State_Sunken;
    }
    if (arrowType != Qt::NoArrow)
        opt.features |= QStyleOptionToolButton::Arrow;
    if (popupMode == QToolButton::DelayedPopup)
        opt.features |= QStyleOptionToolButton::PopupDelay;
    opt.toolButtonStyle = toolButtonStyle;
    if (icon.isNull()) {
        if (!text.isEmpty())
            opt.toolButtonStyle = Qt::ToolButtonTextOnly;
        else if (opt.toolButtonStyle != Qt::ToolButtonTextOnly)
            opt.toolButtonStyle = Qt::ToolButtonIconOnly;
    } else {
        if (text.isEmpty() && opt.toolButtonStyle != Qt::ToolButtonIconOnly)
            opt.toolButtonStyle = Qt::ToolButtonIconOnly;
    }

    opt.pos = q->pos();
    opt.font = q->font();
    return opt;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QToolButton::~QToolButton()
{
}


/*!
    \property QToolButton::backgroundMode
    \brief the toolbutton's background mode

    Get this property with backgroundMode().
*/


/*!
    \reimp
*/
QSize QToolButton::sizeHint() const
{
    Q_D(const QToolButton);
    ensurePolished();

    int w = 0, h = 0;
    QStyleOptionToolButton opt = d->getStyleOption();

    QFontMetrics fm = fontMetrics();
    if (opt.toolButtonStyle != Qt::ToolButtonTextOnly) {
        w = d->iconSize.width();
        h = d->iconSize.height();
    }

    if (opt.toolButtonStyle != Qt::ToolButtonIconOnly) {
        QSize textSize = fm.size(Qt::TextShowMnemonic, text());
        textSize.setWidth(textSize.width() + fm.width(' ')*2);
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
    \property QToolButton::iconSize
    \brief the icon size used for this button.

    QToolButton automatically connects this property to the relevant
    signal in the QMainWindow in which it resides. We strongly
    recommend that you use QMainWindow::iconSize() instead.

    The default size is defined by the GUI style.
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

QSize QToolButton::iconSize() const
{
    Q_D(const QToolButton);
    return d->iconSize;
}

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

void QToolButton::setIconSize(const QSize &size)
{
    Q_D(QToolButton);
    if (d->iconSize == size)
        return;

    d->iconSize = size;
    updateGeometry();
    if (isVisible()) {
        update();
    }
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
    Q_D(QToolButton);
    QStylePainter p(this);
    p.drawComplexControl(QStyle::CC_ToolButton, d->getStyleOption());
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
        connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        break;
    case QEvent::ActionRemoved:
        if (d->defaultAction == action) {
            d->defaultAction = 0;
            if (action->menu() == d->menu)
                d->menu = 0;
        }
        action->disconnect(this);
        break;
    default:
        ;
    }
    QAbstractButton::actionEvent(event);
}

void QToolButtonPrivate::actionTriggered()
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
    Q_D(QToolButton);
    if (e->timerId() == d->popupTimer.timerId()) {
        d->popupTimerDone();
        return;
    }
    QAbstractButton::timerEvent(e);
}


/*!
    \reimp
*/
void QToolButton::changeEvent(QEvent *e)
{
    Q_D(QToolButton);
    if (e->type() == QEvent::ParentChange) {
        if (qobject_cast<QToolBar*>(parentWidget()))
            d->autoRaise = true;
    }
    QAbstractButton::changeEvent(e);
}

/*!
    \reimp
*/
void QToolButton::mousePressEvent(QMouseEvent *e)
{
    Q_D(QToolButton);
    QStyleOptionToolButton opt = d->getStyleOption();
    if (e->button() == Qt::LeftButton && d->popupMode == MenuButtonPopup) {
        QRect popupr = style()->subControlRect(QStyle::CC_ToolButton, &opt,
                                               QStyle::SC_ToolButtonMenu, this);
        if (popupr.isValid() && popupr.contains(e->pos())) {
            showMenu();
            return;
        }
    }

    QAbstractButton::mousePressEvent(e);
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
    /*
      ### Get rid of all qWarning in this file in 4.0.
      Also consider inlining the obsolete functions then.
    */
    qWarning("QToolButton::setOnIconSet(): This function is not supported"
              " anymore");
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
  sets the \l iconSet property. If you relied on the \a on parameter,
  you probably want to update your code to use the QIcon On/Off
  mechanism.

  \sa iconSet QIcon::State
*/

void QToolButton::setIconSet(const QIcon & set, bool /* on */)
{
    QAbstractButton::setIcon(set);
    qWarning("QToolButton::setIconSet(): 'on' parameter ignored");
}

/*! \overload
    \obsolete

  Since Qt 3.0, QIcon contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  returns the \l iconSet property. If you relied on the \a on
  parameter, you probably want to update your code to use the QIcon
  On/Off mechanism.
*/
QIcon QToolButton::iconSet(bool /* on */) const
{
    return QAbstractButton::icon();
}

#endif

/*!
    Associates the given \a menu with this tool button.

    The menu will be shown according to the button's \l popupMode.
.

    Ownership of the menu is not transferred to the tool button.

    \sa menu()
*/
void QToolButton::setMenu(QMenu* menu)
{
    Q_D(QToolButton);
    d->menu = menu;
    update();
}

/*!
    Returns the associated menu, or 0 if no menu has been defined.

    \sa setMenu()
*/
QMenu* QToolButton::menu() const
{
    Q_D(const QToolButton);
    return d->menu;
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

void QToolButtonPrivate::buttonPressed()
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
    if(menu) {
        actualMenu = menu;
        if (q->actions().size() > 1)
            qWarning("QToolButton: menu in setMenu() overriding actions set in addAction!");
    } else {
        actualMenu = new QMenu(q);
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
    QSize sh = actualMenu->sizeHint();
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
            if (q->mapToGlobal(QPoint(rect.right(), 0)).x() + sh.width() <= screen.width()) {
                p = q->mapToGlobal(rect.topRight());
            } else {
                p = q->mapToGlobal(rect.topLeft() - QPoint(sh.width(), 0));
            }
        }
    }
    p.rx() = qMax(0, qMin(p.x(), screen.right() - sh.width()));
    p.ry() += 1;
    QPointer<QToolButton> that = q;
    actualMenu->setNoReplayFor(q);
    actualMenu->exec(p);
    if (actualMenu != menu)
        delete actualMenu;
    if (!that)
        return;

    if (repeat)
        q->setAutoRepeat(true);
    menuButtonDown = false;
    if (q->isDown())
        q->setDown(false);
    else
        q->repaint();
}

#ifdef QT3_SUPPORT
/*!
    \fn void QToolButton::setPopupDelay(int delay)

    Use the style hint QStyle::SH_ToolButton_PopupDelay instead.
*/
void QToolButton::setPopupDelay(int delay)
{
    Q_D(QToolButton);
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


/*!
    \property QToolButton::autoRaise
    \brief whether auto-raising is enabled or not.

    The default is disabled (i.e. false).
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
    if (d->defaultAction && d->menu == d->defaultAction->menu())
        d->menu = 0;
    d->defaultAction = action;
    if (!action)
        return;
    if (!actions().contains(action))
        addAction(action);
    setText(action->iconText());
    setIcon(action->icon());
    setToolTip(action->toolTip());
    setStatusTip(action->statusTip());
    setWhatsThis(action->whatsThis());
    if (QMenu *menu = action->menu())
        setMenu(menu);
    setCheckable(action->isCheckable());
    setChecked(action->isChecked());
    setEnabled(action->isEnabled());
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
    \fn void QToolButton::setTextPosition(TextPosition pos)

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
