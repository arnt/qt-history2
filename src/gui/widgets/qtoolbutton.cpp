/****************************************************************************
**
** Implementation of QToolButton class.
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

#include "qtoolbutton.h"
#ifndef QT_NO_TOOLBUTTON

#include "qevent.h"
#include "qdesktopwidget.h"
#include "qdrawutil.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qmainwindow.h"
#include "qtooltip.h"
#include "qtoolbar.h"
#include "qimage.h"
#include "qiconset.h"
#include "qtimer.h"
#include "qpopupmenu.h"
#include "qpointer.h"

#include "private/qabstractbutton_p.h"


class QToolButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QToolButton)
public:
    void init(bool doMainWindowConnections);
    void popupPressed();
    void popupTimerDone();
    QPointer<QMenu> menu; //the menu set by the user (setMenu)
    QPointer<QMenu> popupMenu; //the menu being displayed (could be the same as menu above)
    QBasicTimer popupTimer;
    int delay;
    Qt::ArrowType arrow;
    uint instantPopup            : 1;
    uint autoRaise            : 1;
    uint repeat                    : 1;
    uint usesTextLabel : 1;
    uint usesBigPixmap : 1;
    uint hasArrow : 1;
    uint discardNextMouseEvent : 1;
    QToolButton::TextPosition textPos;
};

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

    A tool button's icon is set as QIconSet. This makes it possible to
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
    QToolButton you can set a popup menu using setPopup(). The default
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
*/


/*!
    Constructs an empty tool button with parent \a
    parent.
*/
QToolButton::QToolButton(QWidget * parent)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    d->init(true);
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
    d->init(true);
}
#endif

/*!
    Constructs a tool button as an arrow button. The \c ArrowType \a
    type defines the arrow direction. Possible values are \c
    LeftArrow, \c RightArrow, \c UpArrow and \c DownArrow.

    An arrow button has auto-repeat turned on by default.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/
QToolButton::QToolButton(ArrowType type, QWidget *parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    setObjectName(name);
    d->init(false);
    setAutoRepeat(true);
    d->arrow = type;
    d->hasArrow = true;
}


/*  Set-up code common to all the constructors */

void QToolButtonPrivate::init(bool doMainWindowConnections)
{
    textPos = QToolButton::Under;
    delay = 600;
    menu = 0;
    autoRaise = false;
    arrow = LeftArrow;
    instantPopup = false;
    discardNextMouseEvent = false;

    usesTextLabel = false;
    usesBigPixmap = false;
    hasArrow = false;

    q->setFocusPolicy(NoFocus);
    q->setAttribute(QWidget::WA_BackgroundInherited);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    if (!doMainWindowConnections)
        return;
#ifndef QT_NO_TOOLBAR
    if (QToolBar* tb = qt_cast<QToolBar*>(q->parentWidget())) {
        autoRaise = true;
        if (tb->mainWindow()) {
            QObject::connect(tb->mainWindow(), SIGNAL(pixmapSizeChanged(bool)),
                    q, SLOT(setUsesBigPixmap(bool)));
            usesBigPixmap = tb->mainWindow()->usesBigPixmaps();
            QObject::connect(tb->mainWindow(), SIGNAL(usesTextLabelChanged(bool)),
                    q, SLOT(setUsesTextLabel(bool)));
            usesTextLabel = tb->mainWindow()->usesTextLabel();
        }
    }
#endif
    QObject::connect(q, SIGNAL(pressed()), q, SLOT(popupPressed()));
}

#ifndef QT_NO_TOOLBAR

/*!
    Constructs a tool button called \a name, that is a child of \a
    parent (which must be a QToolBar).

    The tool button will display \a iconSet, with its text label and
    tool tip set to \a textLabel and its status bar message set to \a
    statusTip. It will be connected to the \a slot in object \a
    receiver.
*/

QToolButton::QToolButton(const QIconSet& iconSet, const QString &textLabel,
                          const QString& statusTip,
                          QObject * receiver, const char *slot,
                          QToolBar * parent, const char *name)
    : QAbstractButton(*new QToolButtonPrivate, parent)
{
    setObjectName(name);
    d->init(true);
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

#endif


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

    if (icon().isNull() && !text().isNull() && !usesTextLabel()) {
        w = fontMetrics().width(text());
        h = fontMetrics().height(); // boundingRect()?
    } else if (usesBigPixmap()) {
        QPixmap pm = icon().pixmap(QIconSet::Large, QIconSet::Normal);
        w = pm.width();
        h = pm.height();
        QSize iconSize = QIconSet::iconSize(QIconSet::Large);
        if (w < iconSize.width())
            w = iconSize.width();
        if (h < iconSize.height())
            h = iconSize.height();
    } else if (!icon().isNull()) {
        // ### in 3.1, use QIconSet::iconSize(QIconSet::Small);
        QPixmap pm = icon().pixmap(QIconSet::Small, QIconSet::Normal);
        w = pm.width();
        h = pm.height();
        if (w < 16)
            w = 16;
        if (h < 16)
            h = 16;
    }

    if (usesTextLabel()) {
        QSize textSize = fontMetrics().size(Qt::ShowPrefix, text());
        textSize.setWidth(textSize.width() + fontMetrics().width(' ')*2);
        if (d->textPos == Under) {
            h += 4 + textSize.height();
            if (textSize.width() > w)
                w = textSize.width();
        } else { // Right
            w += 4 + textSize.width();
            if (textSize.height() > h)
                h = textSize.height();
        }
    }

    if ((d->menu || actions().count() > 1) && ! popupDelay())
        w += style().pixelMetric(QStyle::PM_MenuButtonIndicator, this);
    return (style().sizeFromContents(QStyle::CT_ToolButton, this, QSize(w, h)).
            expandedTo(QApplication::globalStrut()));
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

void QToolButton::setUsesBigPixmap(bool enable)
{
    if (d->usesBigPixmap == enable)
        return;

    d->usesBigPixmap = enable;
    if (isVisible()) {
        update();
        updateGeometry();
    }
}

bool QToolButton::usesBigPixmap() const
{
    return d->usesBigPixmap;
}

bool QToolButton::usesTextLabel() const
{
    return d->usesTextLabel;
}

/*!
    \property QToolButton::usesTextLabel
    \brief whether the toolbutton displays a text label below the button pixmap.

    The default is false.

    QToolButton automatically connects this slot to the relevant
    signal in the QMainWindow in which is resides.
*/

void QToolButton::setUsesTextLabel(bool enable)
{
    if (d->usesTextLabel == enable)
        return;

    d->usesTextLabel = enable;
    if (isVisible()) {
        update();
        updateGeometry();
    }
}


/*!
    Draws the tool button bevel. Called from paintEvent().

    \sa drawLabel()
*/
void QToolButton::drawBevel(QPainter * p)
{
    QStyle::SCFlags controls = QStyle::SC_ToolButton;
    QStyle::SCFlags active = QStyle::SC_None;

    Qt::ArrowType arrowtype = d->arrow;

    if (isDown())
        active |= QStyle::SC_ToolButton;

    if ((d->menu || actions().count() > 1) && !d->delay) {
        controls |= QStyle::SC_ToolButtonMenu;
        if (d->instantPopup || isDown())
            active |= QStyle::SC_ToolButtonMenu;
    }

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (hasFocus())
        flags |= QStyle::Style_HasFocus;
    if (isDown())
        flags |= QStyle::Style_Down;
    if (isChecked())
        flags |= QStyle::Style_On;
    if (d->autoRaise) {
        flags |= QStyle::Style_AutoRaise;
        if (uses3D()) {
            flags |= QStyle::Style_MouseOver;
            if (! isChecked() && ! isDown())
                flags |= QStyle::Style_Raised;
        }
    } else if (! isChecked() && ! isDown())
        flags |= QStyle::Style_Raised;

    style().drawComplexControl(QStyle::CC_ToolButton, p, this, rect(), palette(),
                               flags, controls, active,
                                d->hasArrow ? QStyleOption(arrowtype) :
                                    QStyleOption());
}


/*!
    Draws the tool button label. Called from paintEvent().

    \sa drawBevel()
*/
void QToolButton::drawLabel(QPainter *p)
{
    QRect r =
        QStyle::visualRect(style().subRect(QStyle::SR_ToolButtonContents, this), this);

    Qt::ArrowType arrowtype = d->arrow;

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (hasFocus())
        flags |= QStyle::Style_HasFocus;
    if (isDown())
        flags |= QStyle::Style_Down;
    if (isChecked())
        flags |= QStyle::Style_On;
    if (d->autoRaise) {
        flags |= QStyle::Style_AutoRaise;
        if (uses3D()) {
            flags |= QStyle::Style_MouseOver;
            if (! isChecked() && ! isDown())
                flags |= QStyle::Style_Raised;
        }
    } else if (! isChecked() && ! isDown())
        flags |= QStyle::Style_Raised;

    style().drawControl(QStyle::CE_ToolButtonLabel, p, this, r,
                        palette(), flags,
                        d->hasArrow ? QStyleOption(arrowtype) :
                            QStyleOption());
}

/*
  Paints the button, by first calling drawBevel() and then
  drawLabel(). If you reimplement paintEvent() in order to draw a
  different label only, you can call drawBevel() from your code.

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
void QToolButton::actionEvent(QActionEvent *)
{
    update();
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


/*\!reimp
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
    QRect popupr =
        QStyle::visualRect(style().querySubControlMetrics(QStyle::CC_ToolButton, this,
                                       QStyle::SC_ToolButtonMenu), this);
    d->instantPopup = (popupr.isValid() && popupr.contains(e->pos()));

    if (d->discardNextMouseEvent) {
        d->discardNextMouseEvent = false;
        d->instantPopup = false;
        return;
    }
    if (e->button() == LeftButton && d->delay <= 0 && d->instantPopup && !d->popupMenu
        && (d->menu || actions().count())) {
        showMenu();
        return;
    }

    d->instantPopup = false;
    QAbstractButton::mousePressEvent(e);
}

/*!
    \reimp
*/
bool QToolButton::eventFilter(QObject *o, QEvent *e)
{
    if (o != d->popupMenu)
        return QAbstractButton::eventFilter(o, e);
    switch (e->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        //when we click on the button and the menu is up just discardNextMouseEvent
        QMouseEvent *me = (QMouseEvent*)e;
        QPoint p = me->globalPos();
        if (QApplication::widgetAt(p) == this)
            d->discardNextMouseEvent = true;
    break; }
    default:
        break;
    }
    return false;
}

bool QToolButton::uses3D() const
{
    return style().styleHint(QStyle::SH_ToolButton_Uses3D)
        && (!d->autoRaise || (underMouse() && isEnabled())
            || (d->popupMenu && d->delay <= 0) || d->instantPopup);
}


#ifdef QT_COMPAT

QIconSet QToolButton::onIconSet() const
{
    return icon();
}

QIconSet QToolButton::offIconSet() const
{
    return icon();
}


/*!
  \property QToolButton::onIconSet
  \brief the icon set that is used when the button is in an "on" state

  \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons. There is
  now an \l QToolButton::iconSet property that replaces both \l
  QToolButton::onIconSet and \l QToolButton::offIconSet.

  For ease of porting, this property is a synonym for \l
  QToolButton::iconSet. You probably want to go over your application
  code and use the QIconSet On/Off mechanism.

  \sa iconSet QIconSet::State
*/
void QToolButton::setOnIconSet(const QIconSet& set)
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
  \property QToolButton::offIconSet
  \brief the icon set that is used when the button is in an "off" state

  \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons. There is
  now an \l QToolButton::iconSet property that replaces both \l
  QToolButton::onIconSet and \l QToolButton::offIconSet.

  For ease of porting, this property is a synonym for \l
  QToolButton::iconSet. You probably want to go over your application
  code and use the QIconSet On/Off mechanism.

  \sa iconSet QIconSet::State
*/
void QToolButton::setOffIconSet(const QIconSet& set)
{
    setIcon(set);
}


/*! \property QToolButton::pixmap
    \brief the pixmap of the button

    The pixmap property has no meaning for tool buttons. Use the
    iconSet property instead.
*/


/*! \overload
    \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  sets the \l iconSet property. If you relied on the \a on parameter,
  you probably want to update your code to use the QIconSet On/Off
  mechanism.

  \sa iconSet QIconSet::State
*/

void QToolButton::setIconSet(const QIconSet & set, bool /* on */)
{
    QAbstractButton::setIcon(set);
    qWarning("QToolButton::setIconSet(): 'on' parameter ignored");
}

/*! \overload
    \obsolete

  Since Qt 3.0, QIconSet contains both the On and Off icons.

  For ease of porting, this function ignores the \a on parameter and
  returns the \l iconSet property. If you relied on the \a on
  parameter, you probably want to update your code to use the QIconSet
  On/Off mechanism.
*/
QIconSet QToolButton::iconSet(bool /* on */) const
{
    return QAbstractButton::icon();
}

#endif

/*!
    Associates the popup menu \a popup with this tool button.

    The popup will be shown each time the tool button has been pressed
    down for a certain amount of time. A typical application example
    is the "back" button in some web browsers's tool bars. If the user
    clicks it, the browser simply browses back to the previous page.
    If the user presses and holds the button down for a while, the
    tool button shows a menu containing the current history list.

    Ownership of the popup menu is not transferred to the tool button.

    \sa menu()
*/
void QToolButton::setMenu(QMenu* menu)
{
    d->menu = menu;
    update();
}

/*!
    Returns the associated popup menu, or 0 if no popup menu has been
    defined.

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
    if (!d->menu || actions().count() < 2)
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
    else
        popupTimerDone();
}

void QToolButtonPrivate::popupTimerDone()
{
    popupTimer.stop();
    if ((!q->isDown() && delay > 0) || (!menu && q->actions().count() < 2))
        return;

    if(menu) {
        popupMenu = menu;
        if(q->actions().count() > 1)
            qWarning("QToolButton: menu in setMenu() overriding actions set in addAction!");
    } else {
        popupMenu = new QMenu(q);
        QList<QAction*> actions = q->actions();
        for(int i = 1; i < actions.size(); i++) //skip the first
            popupMenu->addAction(actions[i]);
    }
    repeat = q->autoRepeat();
    q->setAutoRepeat(false);
    bool horizontal = true;
#ifndef QT_NO_TOOLBAR
    QToolBar *tb = qt_cast<QToolBar*>(q->parentWidget());
    if (tb && tb->orientation() == Vertical)
        horizontal = false;
#endif
    QPoint p;
    QRect screen = qApp->desktop()->availableGeometry(q);
    if (horizontal) {
        if (QApplication::reverseLayout()) {
            if (q->mapToGlobal(QPoint(0, q->rect().bottom())).y() + popupMenu->sizeHint().height() <= screen.height()) {
                p = q->mapToGlobal(q->rect().bottomRight());
            } else {
                p = q->mapToGlobal(q->rect().topRight() - QPoint(0, popupMenu->sizeHint().height()));
            }
            p.rx() -= popupMenu->sizeHint().width();
        } else {
            if (q->mapToGlobal(QPoint(0, q->rect().bottom())).y() + popupMenu->sizeHint().height() <= screen.height()) {
                p = q->mapToGlobal(q->rect().bottomLeft());
            } else {
                p = q->mapToGlobal(q->rect().topLeft() - QPoint(0, popupMenu->sizeHint().height()));
            }
        }
    } else {
        if (QApplication::reverseLayout()) {
            if (q->mapToGlobal(QPoint(q->rect().left(), 0)).x() - popupMenu->sizeHint().width() <= screen.x()) {
                p = q->mapToGlobal(q->rect().topRight());
            } else {
                p = q->mapToGlobal(q->rect().topLeft());
                p.rx() -= popupMenu->sizeHint().width();
            }
        } else {
            if (q->mapToGlobal(QPoint(q->rect().right(), 0)).x() + popupMenu->sizeHint().width() <= screen.width()) {
                p = q->mapToGlobal(q->rect().topRight());
            } else {
                p = q->mapToGlobal(q->rect().topLeft() - QPoint(popupMenu->sizeHint().width(), 0));
            }
        }
    }
    QPointer<QToolButton> that = q;
    //we filter the menu because we do not want to replay the event when the button is
    //clicked on while the menu is up (see discardNextMouseEvent)
    popupMenu->installEventFilter(q);
    popupMenu->exec(p);
    popupMenu->removeEventFilter(q);
    if (popupMenu != menu)
        delete popupMenu;
    popupMenu = 0; //no longer a popup menu
    if (!that)
        return;

    q->setDown(false);
    if (repeat)
        q->setAutoRepeat(true);
}

/*!
    \property QToolButton::popupDelay
    \brief the time delay between pressing the button and the appearance of the associated popup menu in milliseconds.

    Usually this is around half a second. A value of 0 will add a
    special section to the toolbutton that can be used to open the
    popupmenu.

    \sa setPopup()
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

QToolButton::TextPosition QToolButton::textPosition() const
{
    return d->textPos;
}

void QToolButton::setTextPosition(TextPosition pos)
{
    d->textPos = pos;
    updateGeometry();
    update();
}

#include "moc_qtoolbutton.cpp"

#endif
