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
#include <private/qtoolbutton_p.h>


#define d d_func()
#define q q_func()

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
    setUsesBigPixmap() and setUsesTextLabel(). When used inside a
    QToolBar in a QMainWindow, the button automatically adjusts to
    QMainWindow's settings (see QMainWindow::setUsesTextLabel() and
    QMainWindow::setUsesBigPixmaps()). The pixmap set on a QToolButton
    will be set to 22x22 if it is bigger than this size. If
    usesBigPixmap() is true, then the pixmap will be set to 32x32.

    A tool button can offer additional choices in a popup menu. The
    feature is sometimes used with the "Back" button in a web browser.
    After pressing and holding the button down for a while, a menu
    pops up showing a list of possible pages to jump to. With
    QToolButton you can set a popup menu using setMenu(). The default
    delay is 600ms; you can adjust it with setPopupDelay().

    \img qdockwindow.png Toolbar with Toolbuttons \caption A floating
    QToolbar with QToolbuttons

    \sa QPushButton QToolBar QMainWindow \link guibooks.html#fowler
    GUI Design Handbook: Push Button\endlink
*/

/*!
    \enum QToolButton::TextPosition

    The position of the tool button's textLabel in relation to the
    tool button's icon.

    \value BesideIcon The text appears beside the icon.
    \value BelowIcon The text appears below the icon.

    \omitvalue Right
    \omitvalue Under
*/


/*!
    Constructs an empty tool button with parent \a
    parent.
*/
QToolButton::QToolButton(QWidget * parent)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    d->init();
}

#ifdef QT_COMPAT
/*!
    Constructs an empty tool button called \a name, with parent \a
    parent.
*/

QToolButton::QToolButton(QWidget * parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
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
    setObjectName(name);
    d->init();
    d->autoRaise = true;
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
    setObjectName(name);
    d->init();
    setAutoRepeat(true);
    d->arrow = type;
    d->hasArrow = true;
}

#endif

/*!
    Constructs a tool button as an arrow button. The \c Qt::ArrowType \a
    type defines the arrow direction. Possible values are \c
    Qt::LeftArrow, \c Qt::RightArrow, \c Qt::UpArrow and \c Qt::DownArrow.

    An arrow button has auto-repeat turned on by default.

    The \a parent argument is passed to the QWidget constructor.
*/
QToolButton::QToolButton(Qt::ArrowType type, QWidget *parent)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    d->init();
    setAutoRepeat(true);
    d->arrow = type;
    d->hasArrow = true;
}


/*  Set-up code common to all the constructors */

void QToolButtonPrivate::init()
{
    delay = 600;
    menu = 0;
    autoRaise = false;
    arrow = Qt::LeftArrow;
    instantPopup = false;
    discardNextMouseEvent = false;
    popupMode = QToolButton::DelayedPopupMode;

    toolButtonStyle = Qt::ToolButtonIconOnly;
    iconSize = Qt::SmallIconSize;
    hasArrow = false;

    q->setFocusPolicy(Qt::NoFocus);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QObject::connect(q, SIGNAL(pressed()), q, SLOT(popupPressed()));
}

QStyleOptionToolButton QToolButtonPrivate::getStyleOption() const
{
    QStyleOptionToolButton opt;
    opt.init(q);
    bool down = q->isDown();
    bool checked = q->isChecked();
    opt.text = q->text();
    opt.icon = q->icon();
    opt.arrowType = arrow;
    if (down)
        opt.state |= QStyle::Style_Down;
    if (checked)
        opt.state |= QStyle::Style_On;
    if (autoRaise) {
        opt.state |= QStyle::Style_AutoRaise;
        if (q->uses3D()) {
            opt.state |= QStyle::Style_MouseOver;
            if (!checked && !down)
                opt.state |= QStyle::Style_Raised;
        }
    } else if (!checked && !down) {
        opt.state |= QStyle::Style_Raised;
    }

    opt.subControls = QStyle::SC_ToolButton;
    opt.activeSubControls = QStyle::SC_None;
    if (down)
        opt.activeSubControls |= QStyle::SC_ToolButton;

    if ((menu || !q->actions().isEmpty()) && !delay) {
        opt.subControls |= QStyle::SC_ToolButtonMenu;
        if (instantPopup || down)
            opt.activeSubControls |= QStyle::SC_ToolButtonMenu;
    }
    opt.features = QStyleOptionToolButton::None;
    if (q->toolButtonStyle() != Qt::ToolButtonIconOnly)
        opt.features |= QStyleOptionToolButton::TextLabel;
    if (hasArrow)
        opt.features |= QStyleOptionToolButton::Arrow;
    if (menu)
        opt.features |= QStyleOptionToolButton::Menu;
    if (delay)
        opt.features |= QStyleOptionToolButton::PopupDelay;
    if (q->iconSize() == Qt::LargeIconSize)
        opt.features |= QStyleOptionToolButton::BigPixmap;
    opt.toolButtonStyle = q->toolButtonStyle();
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
    ensurePolished();

    int w = 0, h = 0;
    QFontMetrics fm = fontMetrics();
    if (icon().isNull() && !text().isNull() && toolButtonStyle() == Qt::ToolButtonIconOnly) {
        w = fm.width(text());
        h = fm.height(); // boundingRect()?
    } else if (iconSize() == Qt::LargeIconSize) {
        QPixmap pm = icon().pixmap(Qt::LargeIconSize, QIcon::Normal);
        QSize iconSize = QIcon::pixmapSize(Qt::LargeIconSize);
        w = qMax(pm.width(), iconSize.width());
        h = qMax(pm.height(), iconSize.height());
    } else if (!icon().isNull()) {
        // ### in 3.1, use QIcon::iconSize(Qt::SmallIconSize);
        QPixmap pm = icon().pixmap(Qt::SmallIconSize, QIcon::Normal);
        w = qMax(pm.width(), 16);
        h = qMax(pm.height(), 16);
    }

    if (toolButtonStyle() != Qt::ToolButtonIconOnly) {
        QSize textSize = fm.size(Qt::TextShowMnemonic, text());
        textSize.setWidth(textSize.width() + fm.width(' ')*2);
        if (d->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
            h += 4 + textSize.height();
            if (textSize.width() > w)
                w = textSize.width();
        } else { // BesideIcon
            w += 4 + textSize.width();
            if (textSize.height() > h)
                h = textSize.height();
        }
    }

    QStyleOptionToolButton opt = d->getStyleOption();
    if ((d->menu || actions().size() > 1) && ! popupDelay())
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
    \property QToolButton::usesBigPixmap
    \brief whether this toolbutton uses big pixmaps.

    QToolButton automatically connects this property to the relevant
    signal in the QMainWindow in which it resides. We strongly
    recommend that you use QMainWindow::setUsesBigPixmaps() instead.

    This property's default is true.

    \warning If you set some buttons (in a QMainWindow) to have big
    pixmaps and others to have small pixmaps, QMainWindow may not get
    the geometry right.
*/

/*!
    \property QToolButton::usesTextLabel
    \brief whether the toolbutton displays a text label below the button pixmap.

    The default is false.

    QToolButton automatically connects this slot to the relevant
    signal in the QMainWindow in which is resides.
*/

Qt::IconSize QToolButton::iconSize() const
{
    return d->iconSize;
}

Qt::ToolButtonStyle QToolButton::toolButtonStyle() const
{
    return d->toolButtonStyle;
}

void QToolButton::setIconSize(Qt::IconSize size)
{
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
    if (d->toolButtonStyle == style)
        return;

    d->toolButtonStyle = style;
    updateGeometry();
    if (isVisible()) {
        update();
    }
}


/*!
    Draws the tool button bevel on painter \a p. Called from
    paintEvent().

    \sa drawLabel()
*/
void QToolButton::drawBevel(QPainter *p)
{
    QStyleOptionToolButton opt = d->getStyleOption();
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, p, this);
}


/*!
    Draws the tool button label on painter \a p. Called from paintEvent().

    \sa drawBevel()
*/
void QToolButton::drawLabel(QPainter *p)
{
    QStyleOptionToolButton opt = d->getStyleOption();
    opt.rect = QStyle::visualRect(opt.direction, opt.rect, style()->subRect(QStyle::SR_ToolButtonContents, &opt, this));
    style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, p, this);
}

/*!
    \fn void QToolButton::paintEvent(QPaintEvent *event)

    Paints the button in response to the paint \a event, by first
    calling drawBevel() and then drawLabel(). If you reimplement
    paintEvent() just to draw a different label, you can call
    drawBevel() from your own code. For example:
    \code
        QPainter p(this);
        drawBevel(&p);
        // ... your label drawing code
    \endcode
*/
void QToolButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawBevel(&p);
    drawLabel(&p);
}

/*!
    \reimp
 */
void QToolButton::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
    case QEvent::ActionAdded:
        Q_ASSERT(actions().size() == 1);
        action->connect(q, SIGNAL(clicked()), SLOT(trigger()));
        // fall through intended

    case QEvent::ActionChanged:
        setText(action->iconText());
        setIcon(action->icon());
        setToolTip(action->toolTip());
        setStatusTip(action->statusTip());
        setWhatsThis(action->whatsThis());
        setMenu(action->menu());
        setCheckable(action->isCheckable());
        setChecked(action->isChecked());
        setEnabled(action->isEnabled());
        setFont(actions().at(0)->font());
        break;

    default:
        break;
    }

    QAbstractButton::actionEvent(event);
}

/*!
    \reimp
 */
void QToolButton::enterEvent(QEvent * e)
{
    if (d->autoRaise && isEnabled())
        repaint();

    QAbstractButton::enterEvent(e);
}


/*!
    \reimp
 */
void QToolButton::leaveEvent(QEvent * e)
{
    if (d->autoRaise && isEnabled())
        repaint();

    QAbstractButton::leaveEvent(e);
}


/*!
    \reimp
 */
void QToolButton::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->popupTimer.timerId()) {
        d->popupTimerDone();
        return;
    }
    QAbstractButton::timerEvent(e);
}

/*!
    \reimp
*/
void QToolButton::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionToolButton opt = d->getStyleOption();
    QRect popupr =
        QStyle::visualRect(opt.direction, opt.rect, style()->querySubControlMetrics(QStyle::CC_ToolButton, &opt,
                                                          QStyle::SC_ToolButtonMenu, this));
    d->instantPopup = (popupr.isValid() && popupr.contains(e->pos()));

    if (d->discardNextMouseEvent) {
        d->discardNextMouseEvent = false;
        d->instantPopup = false;
        return;
    }
    if (e->button() == Qt::LeftButton && d->delay <= 0 && d->instantPopup && !d->actualMenu
        && (d->menu || actions().size() > 1)) {
        showMenu();
        return;
    }

    d->instantPopup = false;
    QAbstractButton::mousePressEvent(e);
}


/*!
    \internal

    Returns true if this button has a 3D effect; otherwise returns
    false.
*/
bool QToolButton::uses3D() const
{
    return style()->styleHint(QStyle::SH_ToolButton_Uses3D, 0, this)
        && (!d->autoRaise || (underMouse() && isEnabled())
            || (d->menu && d->delay <= 0) || d->instantPopup);
}


#ifdef QT_COMPAT

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

    The menu will be shown each time the tool button has been pressed
    down for \l popupDelay amount of time. A typical application
    example is the "back" button in some web browsers's tool bars. If
    the user clicks it, the browser simply browses back to the
    previous page.  If the user presses and holds the button down for
    a while, the tool button shows a menu containing the current
    history list.

    Ownership of the menu is not transferred to the tool button.

    \sa menu()
*/
void QToolButton::setMenu(QMenu* menu)
{
    d->menu = menu;
    update();
}

/*!
    Returns the associated menu, or 0 if no menu has been defined.

    \sa setMenu()
*/
QMenu* QToolButton::menu() const
{
    return d->menu;
}

/*!
    Shows (pops up) the associated popup menu. If there is no such
    menu, this function does nothing. This function does not return
    until the popup menu has been closed by the user.
*/
void QToolButton::showMenu()
{
    if (!d->menu && actions().isEmpty())
        return;

    d->instantPopup = true;
    repaint();
    d->popupTimer.stop();
    QPointer<QToolButton> that = this;
    d->popupTimerDone();
    if (!that)
        return;
    d->instantPopup = false;
    repaint();
}

void QToolButtonPrivate::popupPressed()
{
    if (delay > 0)
        popupTimer.start(delay, q);
}

void QToolButtonPrivate::popupTimerDone()
{
    popupTimer.stop();
    if ((!q->isDown() && delay > 0) || (!menu && q->actions().size() <= 1))
        return;

    if(menu) {
        actualMenu = menu;
        if(!q->actions().isEmpty())
            qWarning("QToolButton: menu in setMenu() overriding actions set in addAction!");
    } else {
        actualMenu = new QMenu(q);
        QList<QAction*> actions = q->actions();
        for(int i = 0; i < actions.size(); i++) //skip the first
            actualMenu->addAction(actions[i]);
    }
    repeat = q->autoRepeat();
    q->setAutoRepeat(false);
    bool horizontal = true;
#if !defined(QT_NO_TOOLBAR)
    QToolBar *tb = qt_cast<QToolBar*>(q->parentWidget());
    if (tb && tb->orientation() == Qt::Vertical)
        horizontal = false;
#endif
    QPoint p;
    QRect screen = qApp->desktop()->availableGeometry(q);
    if (horizontal) {
        if (q->isRightToLeft()) {
            if (q->mapToGlobal(QPoint(0, q->rect().bottom())).y() + actualMenu->sizeHint().height() <= screen.height()) {
                p = q->mapToGlobal(q->rect().bottomRight());
            } else {
                p = q->mapToGlobal(q->rect().topRight() - QPoint(0, actualMenu->sizeHint().height()));
            }
            p.rx() -= actualMenu->sizeHint().width();
        } else {
            if (q->mapToGlobal(QPoint(0, q->rect().bottom())).y() + actualMenu->sizeHint().height() <= screen.height()) {
                p = q->mapToGlobal(q->rect().bottomLeft());
            } else {
                p = q->mapToGlobal(q->rect().topLeft() - QPoint(0, actualMenu->sizeHint().height()));
            }
        }
    } else {
        if (q->isRightToLeft()) {
            if (q->mapToGlobal(QPoint(q->rect().left(), 0)).x() - actualMenu->sizeHint().width() <= screen.x()) {
                p = q->mapToGlobal(q->rect().topRight());
            } else {
                p = q->mapToGlobal(q->rect().topLeft());
                p.rx() -= actualMenu->sizeHint().width();
            }
        } else {
            if (q->mapToGlobal(QPoint(q->rect().right(), 0)).x() + actualMenu->sizeHint().width() <= screen.width()) {
                p = q->mapToGlobal(q->rect().topRight());
            } else {
                p = q->mapToGlobal(q->rect().topLeft() - QPoint(actualMenu->sizeHint().width(), 0));
            }
        }
    }
    QPointer<QToolButton> that = q;
    actualMenu->setNoReplayFor(q);
    actualMenu->exec(p);
    if (actualMenu != menu)
        delete actualMenu;
    actualMenu = 0; //no longer a popup menu
    if (!that)
        return;

    q->setDown(false);
    if (repeat)
        q->setAutoRepeat(true);
}

/*!
    \property QToolButton::popupDelay

    \brief the time delay between pressing the button and the moment
    the associated menu pops up in milliseconds.

    Usually this is around half a second. A value of 0 will add a
    special section to the tool button that can be used to open the
    menu.

    \sa setMenu()
*/
void QToolButton::setPopupDelay(int delay)
{
    d->delay = delay;

    update();
}

int QToolButton::popupDelay() const
{
    return d->delay;
}

void QToolButton::setPopupMode(QToolButton::ToolButtonPopupMode mode)
{
    d->popupMode = mode;
}

QToolButton::ToolButtonPopupMode QToolButton::popupMode()
{
    return d->popupMode;
}


/*!
    \property QToolButton::autoRaise
    \brief whether auto-raising is enabled or not.

    The default is disabled (i.e. false).
*/
void QToolButton::setAutoRaise(bool enable)
{
    d->autoRaise = enable;

    update();
}

bool QToolButton::autoRaise() const
{
    return d->autoRaise;
}

/*!
    \property QToolButton::textPosition
    \brief the position of the text label of this button.
*/

/*! \internal
 */
QToolButton::QToolButton(QToolButtonPrivate &dd, QWidget *parent)
    :QAbstractButton(dd, parent)
{
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

#include "moc_qtoolbutton.cpp"

#endif
