/****************************************************************************
**
** Implementation of Q3MenuBar class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_MENUBAR
#include "q3menubar.h"
#include "q3popupmenu.h"
#include "qaccel.h"
#include "qapplication.h"
#include "qcleanuphandler.h"
#include "qdesktopwidget.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qlayout.h"
#include "q3mainwindow.h"
#include "qpainter.h"
#include "qpointer.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtimer.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include "qsignal.h"

class Q3MenuDataData {
    // attention: also defined in q3menudata.cpp
public:
    Q3MenuDataData();
    QPointer<QWidget> aWidget;
    int aInt;
};

#ifndef QT_NO_ACCESSIBILITY
static bool inMenu = false;
#endif

#if defined(Q_WS_X11)
extern int qt_xfocusout_grab_counter; // defined in qapplication_x11.cpp
#endif

/*!
    \class Q3MenuBar q3menubar.h
    \brief The Q3MenuBar class provides a horizontal menu bar.

    \ingroup application
    \mainclass

    A menu bar consists of a list of pull-down menu items. You add
    menu items with \link Q3MenuData::insertItem()
    insertItem()\endlink. For example, asuming that \c menubar is a
    pointer to a Q3MenuBar and \c filemenu is a pointer to a
    Q3PopupMenu, the following statement inserts the menu into the menu
    bar:
    \code
    menubar->insertItem("&File", filemenu);
    \endcode
    The ampersand in the menu item's text sets Alt+F as a shortcut for
    this menu. (You can use "\&\&" to get a real ampersand in the menu
    bar.)

    Items are either enabled or disabled. You toggle their state with
    setItemEnabled().

    There is no need to lay out a menu bar. It automatically sets its
    own geometry to the top of the parent widget and changes it
    appropriately whenever the parent is resized.

    \important insertItem removeItem clear insertSeparator setItemEnabled isItemEnabled setItemVisible isItemVisible

    Example of creating a menu bar with menu items (from \l menu/menu.cpp):
    \quotefile menu/menu.cpp
    \skipto file = new Q3PopupMenu
    \printline
    \skipto Qt::Key_O
    \printline
    \printline
    \skipto new Q3MenuBar
    \printline
    \skipto insertItem
    \printline

    In most main window style applications you would use the menuBar()
    provided in Q3MainWindow, adding \l{Q3PopupMenu}s to the menu bar
    and adding \l{QAction}s to the popup menus.

    Example (from \l action/application.cpp):
    \quotefile action/application.cpp
    \skipto file = new Q3PopupMenu
    \printuntil fileNewAction

    Menu items can have text and pixmaps (or iconsets), see the
    various \link Q3MenuData::insertItem() insertItem()\endlink
    overloads, as well as separators, see \link
    Q3MenuData::insertSeparator() insertSeparator()\endlink. You can
    also add custom menu items that are derived from
    \l{Q3CustomMenuItem}.

    Menu items may be removed with removeItem() and enabled or
    disabled with \link Q3MenuData::setItemEnabled()
    setItemEnabled()\endlink.

    <img src=q3menubar-m.png> <img src=q3menubar-w.png>

    \section1 Q3MenuBar on Qt/Mac

    Q3MenuBar on Qt/Mac is a wrapper for using the system-wide menubar.
    If you have multiple menubars in one dialog the outermost menubar
    (normally inside a widget with widget flag \c Qt::WType_TopLevel) will
    be used for the system-wide menubar.

    Note that arbitrary Qt widgets \e cannot be inserted into a
    Q3MenuBar on the Mac because Qt uses Mac's native menus which don't
    support this functionality. This limitation does not apply to
    stand-alone Q3PopupMenus.

    Qt/Mac also provides a menubar merging feature to make Q3MenuBar
    conform more closely to accepted Mac OS X menubar layout. The
    merging functionality is based on string matching the title of a
    Q3PopupMenu entry. These strings are translated (using
    QObject::tr()) in the "Q3MenuBar" context. If an entry is moved its
    slots will still fire as if it was in the original place. The
    table below outlines the strings looked for and where the entry is
    placed if matched:

    \table
    \header \i String matches \i Placement \i Notes
    \row \i about.*
         \i Application Menu | About <application name>
         \i If this entry is not found no About item will appear in
            the Application Menu
    \row \i config, options, setup, settings or preferences
         \i Application Menu | Preferences
         \i If this entry is not found the Settings item will be disabled
    \row \i quit or exit
         \i Application Menu | Quit <application name>
         \i If this entry is not found a default Quit item will be
            created to call QApplication::quit()
    \endtable

    \link menu-example.html menu/menu.cpp\endlink is an example of
    Q3MenuBar and Q3PopupMenu use.

    \sa Q3PopupMenu QAccel QAction \link http://developer.apple.com/techpubs/macosx/Carbon/HumanInterfaceToolbox/Aqua/aqua.html Aqua Style Guidelines \endlink \link guibooks.html#fowler GUI Design Handbook: Menu Bar \endlink
*/


/*!
    \enum Q3MenuBar::Separator

    This enum type is used to decide whether Q3MenuBar should draw a
    separator line at its bottom.

    \value Never In many applications there is already a separator,
    and having two looks wrong.

    \value InWindowsStyle In some other applications a separator looks
    good in Windows style, but nowhere else.
*/

/*!
    \fn void Q3MenuBar::activated(int id)

    This signal is emitted when a menu item is selected; \a id is the
    id of the selected item.

    Normally you will connect each menu item to a single slot using
    Q3MenuData::insertItem(), but sometimes you will want to connect
    several items to a single slot (most often if the user selects
    from an array). This signal is useful in such cases.

    \sa highlighted(), Q3MenuData::insertItem()
*/

/*!
    \fn void Q3MenuBar::highlighted(int id)

    This signal is emitted when a menu item is highlighted; \a id is
    the id of the highlighted item.

    Normally, you will connect each menu item to a single slot using
    Q3MenuData::insertItem(), but sometimes you will want to connect
    several items to a single slot (most often if the user selects
    from an array). This signal is useful in such cases.

    \sa activated(), Q3MenuData::insertItem()
*/


// Motif style parameters

static const int motifBarHMargin        = 2;        // menu bar hor margin to item
static const int motifBarVMargin        = 1;        // menu bar ver margin to item
static const int motifItemFrame                = 2;        // menu item frame width
static const int motifItemHMargin        = 5;        // menu item hor text margin
static const int motifItemVMargin        = 4;        // menu item ver text margin

/*

+-----------------------------
|      BarFrame
|   +-------------------------
|   |           V  BarMargin
|   |        +---------------------
|   | H |      ItemFrame
|   |        |  +-----------------
|   |        |  |                           \
|   |        |  |  ^         T E X T   ^            | ItemVMargin
|   |        |  |  |                   |           /
|   |        |      ItemHMargin
|   |
|

*/


/*****************************************************************************
  Q3MenuBar member functions
 *****************************************************************************/


/*!
    Constructs a menu bar called \a name with parent \a parent.
*/
Q3MenuBar::Q3MenuBar(QWidget *parent, const char *name)
    : Q3Frame(parent, name)
{
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    mac_eaten_menubar = false;
    mac_d = 0;
    macCreateNativeMenubar();
#endif
    isMenuBar = true;
#ifndef QT_NO_ACCEL
    autoaccel = 0;
#endif
    irects    = 0;
    rightSide = 0; // Right of here is rigth-aligned content
    mseparator = 0;
    waitforalt = 0;
    popupvisible = 0;
    hasmouse = 0;
    defaultup = 0;
    toggleclose = 0;
    pendingDelayedContentsChanges = 0;
    pendingDelayedStateChanges = 0;
    if (parent) {
        // filter parent events for resizing
        parent->installEventFilter(this);

        // filter top-level-widget events for accelerators
        QWidget *tlw = topLevelWidget();
        if (tlw != parent)
            tlw->installEventFilter(this);
    }
    installEventFilter(this);

    setBackgroundRole(QPalette::Button);
    setFrameStyle(Q3Frame::MenuBarPanel | Q3Frame::Raised);

    QFontMetrics fm = fontMetrics();
    int h = 2*motifBarVMargin + fm.height() + motifItemVMargin + 2*frameWidth() + 2*motifItemFrame;

    setGeometry(0, 0, width(), h);

    setMouseTracking(style().styleHint(QStyle::SH_MenuBar_MouseTracking));
    setAttribute(Qt::WA_CustomWhatsThis);
}



/*! \reimp */
void Q3MenuBar::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        setMouseTracking(style().styleHint(QStyle::SH_MenuBar_MouseTracking));
    } else if(ev->type() == QEvent::FontChange) {
        badSize = true;
        updateGeometry();
        if (isVisible())
            calculateRects();
    } else if(ev->type() == QEvent::LanguageChange) {
        menuContentsChanged();
    }
    Q3Frame::changeEvent(ev);
}



/*!
    Destroys the menu bar.
*/

Q3MenuBar::~Q3MenuBar()
{
#ifndef QT_NO_ACCEL
    delete autoaccel;
#endif
#if defined(Q_WS_MAC) && !defined(QMAC_QM3ENUBAR_NO_NATIVE)
    macRemoveNativeMenubar();
#endif
    if (irects)                // Avoid purify complaint.
        delete [] irects;
}

/*!
    \internal

    Repaints the menu item with id \a id; does nothing if there is no
    such menu item.
*/
void Q3MenuBar::updateItem(int id)
{
    int i = indexOf(id);
    if (i >= 0 && irects)
        repaint(irects[i]);
}

#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
static bool fromFrameChange = false;
#endif

/*!
    Recomputes the menu bar's display data according to the new
    contents.

    You should never need to call this; it is called automatically by
    Q3MenuData whenever it needs to be called.
*/

void Q3MenuBar::menuContentsChanged()
{
    // here the part that can't be delayed
    Q3MenuData::menuContentsChanged();
    badSize = true;                                // might change the size
    if(pendingDelayedContentsChanges)
        return;
    pendingDelayedContentsChanges = 1;
    if(!pendingDelayedStateChanges)// if the timer hasn't been started yet
        QTimer::singleShot(0, this, SLOT(performDelayedChanges()));
}

void Q3MenuBar::performDelayedContentsChanged()
{
    pendingDelayedContentsChanges = 0;
    // here the part the can be delayed
#ifndef QT_NO_ACCEL
    // if performDelayedStateChanged() will be called too,
    // it will call setupAccelerators() too, no need to do it twice
    if(!pendingDelayedStateChanges)
        setupAccelerators();
#endif
    calculateRects();
    if (isVisible()) {
        update();
#ifndef QT_NO_MAINWINDOW
        Q3MainWindow *mw = qt_cast<Q3MainWindow*>(parent());
        if (mw) {
            mw->triggerLayout();
            mw->update();
        }
#endif
#ifndef QT_NO_LAYOUT
        if (parentWidget() && parentWidget()->layout())
            parentWidget()->layout()->activate();
#endif
    }
}

/*!
    Recomputes the menu bar's display data according to the new state.

    You should never need to call this; it is called automatically by
    Q3MenuData whenever it needs to be called.
*/

void Q3MenuBar::menuStateChanged()
{
    if(pendingDelayedStateChanges)
        return;
    pendingDelayedStateChanges = 1;
    if(!pendingDelayedContentsChanges) // if the timer hasn't been started yet
        QTimer::singleShot(0, this, SLOT(performDelayedChanges()));
}

void Q3MenuBar::performDelayedStateChanged()
{
    pendingDelayedStateChanges = 0;
    // here the part that can be delayed
#ifndef QT_NO_ACCEL
    setupAccelerators(); // ### when we have a good solution for the accel vs. focus
                         // widget problem, remove that. That is only a workaround
                         // if you remove this, see performDelayedContentsChanged()
#endif
    update();
}


void Q3MenuBar::performDelayedChanges()
{
#if defined(Q_WS_MAC) && !defined(QMAC_MENUBAR_NO_NATIVE)
    // I must do this here as the values change in the function below.
    bool needMacUpdate = (pendingDelayedContentsChanges || pendingDelayedStateChanges);
#endif
    if(pendingDelayedContentsChanges)
        performDelayedContentsChanged();
    if(pendingDelayedStateChanges)
        performDelayedStateChanged();
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    if(mac_eaten_menubar && needMacUpdate) {
        macDirtyNativeMenubar();

        bool all_hidden = true;
        if(irects) {
            for(int i = 0; all_hidden && i < (int)mitems->count(); i++)
                all_hidden = irects[i].isEmpty();
        }
        if(all_hidden) {
            if(!isHidden())
                hide();
        } else {
            if(!isShown() && !fromFrameChange)
                show();
        }
    }
#endif
}


void Q3MenuBar::menuInsPopup(Q3PopupMenu *popup)
{
    connect(popup, SIGNAL(activatedRedirect(int)),
             SLOT(subActivated(int)));
    connect(popup, SIGNAL(highlightedRedirect(int)),
             SLOT(subHighlighted(int)));
    connect(popup, SIGNAL(destroyed(QObject*)),
             this, SLOT(popupDestroyed(QObject*)));
}

void Q3MenuBar::menuDelPopup(Q3PopupMenu *popup)
{
    popup->disconnect(SIGNAL(activatedRedirect(int)));
    popup->disconnect(SIGNAL(highlightedRedirect(int)));
    disconnect(popup, SIGNAL(destroyed(QObject*)),
                this, SLOT(popupDestroyed(QObject*)));
}

void Q3MenuBar::frameChanged()
{
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    fromFrameChange = true;
#endif
    menuContentsChanged();
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    fromFrameChange = false;
#endif
}

/*!
    \internal

    This function is used to adjust the menu bar's geometry to the
    parent widget's geometry. Note that this is \e not part of the
    public interface - the function is \c public only because
    QObject::eventFilter() is.

    Resizes the menu bar to fit in the parent widget when the parent
    receives a resize event.
*/

bool Q3MenuBar::eventFilter(QObject *object, QEvent *event)
{
    if (object == parent() && object
#ifndef QT_NO_TOOLBAR
         && !qt_cast<Q3ToolBar*>(object)
#endif
         && event->type() == QEvent::Resize) {
        QResizeEvent *e = (QResizeEvent *)event;
        int w = e->size().width();
        setGeometry(0, y(), w, heightForWidth(w));
        return false;
    }

    if (!isVisible() || !object->isWidgetType())
        return false;

    if (object == this && event->type() == QEvent::LanguageChange) {
        badSize = true;
        calculateRects();
        return false;
    } else if (event->type() == QEvent::MouseButtonPress ||
                event->type() == QEvent::MouseButtonRelease) {
        waitforalt = 0;
        return false;
    } else if (waitforalt && event->type() == QEvent::FocusOut) {
        // some window systems/managers use alt/meta as accelerator keys
        // for switching between windows/desktops/etc.  If the focus
        // widget gets unfocused, then we need to stop waiting for alt
        // NOTE: this event came from the real focus widget, so we don't
        // need to touch the event filters
        waitforalt = 0;
    // although the comment above said not to remove the event filter, it is
    // incorrect. We need to remove our self fom the focused widget as normally
    // this happens in the key release but it does not happen in this case
    QWidget * f = ((QWidget *)object)->focusWidget();
    if (f)
        f->removeEventFilter(this);
        return false;
    } else if (!(event->type() == QEvent::Accel ||
                event->type() == QEvent::AccelOverride ||
                event->type() == QEvent::KeyPress ||
                event->type() == QEvent::KeyRelease) ||
                !style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this)) {
        return false;
    }

    QKeyEvent * ke = (QKeyEvent *) event;
#ifndef QT_NO_ACCEL
    // look for Alt press and Alt-anything press
    if (event->type() == QEvent::Accel) {
        QWidget * f = ((QWidget *)object)->focusWidget();
        // ### this thinks alt and meta are the same
        if (ke->key() == Qt::Key_Alt || ke->key() == Qt::Key_Meta) {
            // A new Alt press and we wait for release, eat
            // this key and don't wait for Alt on this widget
            if (waitforalt) {
                waitforalt = 0;
                if (object->parent())
                    object->removeEventFilter(this);
                ke->accept();
                return true;
            // Menu has focus, send focus back
            } else if (hasFocus()) {
                setAltMode(false);
                ke->accept();
                return true;
            // Start waiting for Alt release on focus widget
            } else if (ke->stateAfter() == Qt::AltButton) {
                waitforalt = 1;
#if defined(Q_WS_X11)
                Q3MenuData::d->aInt = qt_xfocusout_grab_counter;
#endif
                if (f && f != object)
                    f->installEventFilter(this);
            }
        // Other modifiers kills focus on menubar
        } else if (ke->key() == Qt::Key_Control || ke->key() == Qt::Key_Shift) {
            setAltMode(false);
        // Got other key, no need to wait for Alt release
        } else {
            waitforalt = 0;
        }
        // ### ! block all accelerator events when the menu bar is active
        if (qApp && qApp->focusWidget() == this) {
            return true;
        }

        return false;
    }
#endif
    // look for Alt release
    if (((QWidget*)object)->focusWidget() == object ||
         (object->parent() == 0 && ((QWidget*)object)->focusWidget() == 0)) {
        if (waitforalt && event->type() == QEvent::KeyRelease &&
            (ke->key() == Qt::Key_Alt || ke->key() == Qt::Key_Meta)
#if defined(Q_WS_X11)
                && Q3MenuData::d->aInt == qt_xfocusout_grab_counter
#endif
           ) {
            setAltMode(true);
            if (object->parent())
                object->removeEventFilter(this);
            QWidget * tlw = ((QWidget *)object)->topLevelWidget();
            if (tlw) {
                // ### !
                // make sure to be the first event filter, so we can kill
                // accelerator events before the accelerators get to them.
                tlw->removeEventFilter(this);
                tlw->installEventFilter(this);
            }
            return true;
        // Cancel if next keypress is NOT Alt/Meta,
        } else if (!hasFocus() && (event->type() == QEvent::AccelOverride) &&
                    !(((QKeyEvent *)event)->key() == Qt::Key_Alt ||
                      ((QKeyEvent *)event)->key() == Qt::Key_Meta)) {
            if (object->parent())
                object->removeEventFilter(this);
            setAltMode(false);
        }
    }

    return false;                                // don't stop event
}



/*!
  \internal
  Receives signals from menu items.
*/

void Q3MenuBar::subActivated(int id)
{
    emit activated(id);
}

/*!
  \internal
  Receives signals from menu items.
*/

void Q3MenuBar::subHighlighted(int id)
{
    emit highlighted(id);
}

/*!
  \internal
  Receives signals from menu accelerator.
*/
#ifndef QT_NO_ACCEL
void Q3MenuBar::accelActivated(int id)
{
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    if(mac_eaten_menubar)
        return;
#endif
    if (!isEnabled())                                // the menu bar is disabled
        return;
    setAltMode(true);
    setActiveItem(indexOf(id));
}
#endif

/*!
  \internal
  This slot receives signals from menu accelerator when it is about to be
  destroyed.
*/
#ifndef QT_NO_ACCEL
void Q3MenuBar::accelDestroyed()
{
    autoaccel = 0;                                // don't delete it twice!
}
#endif

void Q3MenuBar::popupDestroyed(QObject *o)
{
    removePopup((Q3PopupMenu*)o);
}

bool Q3MenuBar::tryMouseEvent(Q3PopupMenu *, QMouseEvent *e)
{
    QPoint pos = mapFromGlobal(e->globalPos());
    if (!rect().contains(pos))                // outside
        return false;
    int item = itemAtPos(pos);
    if (item == -1 && (e->type() == QEvent::MouseButtonPress ||
                        e->type() == QEvent::MouseButtonRelease)) {
        hidePopups();
        goodbye();
        return false;
    }
    QMouseEvent ee(e->type(), pos, e->globalPos(), e->button(), e->state());
    event(&ee);
    return true;
}


void Q3MenuBar::tryKeyEvent(Q3PopupMenu *, QKeyEvent *e)
{
    event(e);
}


void Q3MenuBar::goodbye(bool cancelled)
{
    mouseBtDn = false;
    popupvisible = 0;
    setAltMode(cancelled && style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this));
}


void Q3MenuBar::openActPopup()
{
#ifndef QT_NO_ACCESSIBILITY
    if (!inMenu) {
        QAccessible::updateAccessibility(this, 0, QAccessible::MenuStart);
        inMenu = true;
    }
#endif

    if (actItem < 0)
        return;
    Q3PopupMenu *popup = mitems->at(actItem)->popup();
    if (!popup || !popup->isEnabled())
        return;

    QRect  r = itemRect(actItem);
    bool reverse = QApplication::reverseLayout();
    const int yoffset = 1; //(style().styleHint(QStyle::SH_GUIStyle) == QStyle::WindowsStyle) ? 4 : 1; ### this breaks designer mainwindow editing
    QPoint pos = r.bottomLeft() + QPoint(0,yoffset);
    if(reverse) {
        pos = r.bottomRight() + QPoint(0,yoffset);
        pos.rx() -= popup->sizeHint().width();
    }

    int ph = popup->sizeHint().height();
    pos = mapToGlobal(pos);
    int sh = QApplication::desktop()->height();
    if (defaultup || (pos.y() + ph > sh)) {
        QPoint t = mapToGlobal(r.topLeft());
        if(reverse) {
            t = mapToGlobal(r.topRight());
            t.rx() -= popup->sizeHint().width();
        }
        t.ry() -= (QCOORD)ph;
        if (!defaultup || t.y() >= 0)
            pos = t;
    }

    //avoid circularity
    if (popup->isVisible())
        return;

    Q_ASSERT(popup->parentMenu == 0);
    popup->parentMenu = this;                        // set parent menu

    popup->snapToMouse = false;
    popup->popup(pos);
    popup->snapToMouse = true;
}

/*!
  \internal
  Hides all popup menu items.
*/

void Q3MenuBar::hidePopups()
{
#ifndef QT_NO_ACCESSIBILITY
    bool anyVisible = false;
#endif
    for (int i = 0; i < mitems->size(); ++i) {
        register Q3MenuItem *mi = mitems->at(i);
        if (mi->popup() && mi->popup()->isVisible()) {
#ifndef QT_NO_ACCESSIBILITY
            anyVisible = true;
#endif
            mi->popup()->hide();
        }
    }
#ifndef QT_NO_ACCESSIBILITY
    if (!popupvisible && anyVisible && inMenu) {
        QAccessible::updateAccessibility(this, 0, QAccessible::MenuEnd);
        inMenu = false;
    }
#endif
}


/*!
    Reimplements QWidget::show() in order to set up the correct
    keyboard accelerators and to raise itself to the top of the widget
    stack.
*/

void Q3MenuBar::show()
{
#ifndef QT_NO_ACCEL
    setupAccelerators();
#endif

    if (parentWidget())
        resize(parentWidget()->width(), height());

    QApplication::sendPostedEvents(this, QEvent::Resize);
    performDelayedChanges();
    calculateRects();

#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    if(mac_eaten_menubar) {
        //If all elements are invisible no reason for me to be visible either
        bool all_hidden = true;
        if(irects) {
            for(int i = 0; all_hidden && i < (int)mitems->count(); i++)
                all_hidden = irects[i].isEmpty();
        }
        if(all_hidden)
            QWidget::hide();
        else
            QWidget::show();
    } else {
        QWidget::show();
    }
#else
    QWidget::show();
#endif

#ifndef QT_NO_MAINWINDOW
    Q3MainWindow *mw = qt_cast<Q3MainWindow*>(parent());
    if (mw) //### ugly workaround
        mw->triggerLayout();
#endif
    raise();
}

/*!
    Reimplements QWidget::hide() in order to deselect any selected
    item, and calls setUpLayout() for the main window.
*/

void Q3MenuBar::hide()
{
    QWidget::hide();
    setAltMode(false);
    hidePopups();
#ifndef QT_NO_MAINWINDOW
    Q3MainWindow *mw = qt_cast<Q3MainWindow*>(parent());
    if (mw) //### ugly workaround
        mw->triggerLayout();
#endif
}

/*****************************************************************************
  Item geometry functions
 *****************************************************************************/

/*
    This function serves two different purposes. If the parameter is
    negative, it updates the irects member for the current width and
    resizes. Otherwise, it does the same calculations for the GIVEN
    width and returns the height to which it WOULD have resized. A bit
    tricky, but both operations require almost identical steps.
*/
int Q3MenuBar::calculateRects(int max_width)
{
    ensurePolished();
    bool update = (max_width < 0);

    if (update) {
        rightSide = 0;
        if (!badSize)                                // size was not changed
            return 0;
        delete [] irects;
        int i = mitems->count();
        if (i == 0) {
            irects = 0;
        } else {
            irects = new QRect[i];
        }
        max_width = width();
    }
    QFontMetrics fm = fontMetrics();
    int max_height = 0;
    int max_item_height = 0;
    int nlitems = 0;                                // number on items on cur line
    int gs = style().styleHint(QStyle::SH_GUIStyle);
    bool reverse = QApplication::reverseLayout();
    int x = frameWidth();
    int y = frameWidth();
    if (gs == Qt::MotifStyle) {
        x += motifBarHMargin;
        y += motifBarVMargin;
    } else if (gs == Qt::WindowsStyle) {
        x += 2;
        y += 2;
    }
    if (reverse)
        x = max_width - x;

    int i = 0;
    int separator = -1;
    const int itemSpacing = style().pixelMetric(QStyle::PM_MenuBarItemSpacing);
    const int lastItem = reverse ? 0 : mitems->count() - 1;

    while (i < (int)mitems->count()) {        // for each menu item...
        Q3MenuItem *mi = mitems->at(i);

        int w=0, h=0;
        if (!mi->isVisible()
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
                                  ||  (mac_eaten_menubar && !mi->custom() && !mi->widget())
#endif
            ) {
            ; // empty rectangle
        } else if (mi->widget()) {
            if (mi->widget()->parentWidget() != this)
                mi->widget()->setParent(this);
            w = mi->widget()->sizeHint().expandedTo(QApplication::globalStrut()).width()+2;
            h = mi->widget()->sizeHint().expandedTo(QApplication::globalStrut()).height()+2;
            if (i && separator < 0)
                separator = i;
        } else if (mi->pixmap()) {                        // pixmap item
            w = qMax(mi->pixmap()->width() + 4, QApplication::globalStrut().width());
            h = qMax(mi->pixmap()->height() + 4, QApplication::globalStrut().height());
        } else if (!mi->text().isNull()) {        // text item
            QString s = mi->text();
            w = fm.boundingRect(s).width()
                + 2*motifItemHMargin;
            w -= s.count('&')*fm.width('&');
            w += s.count("&&")*fm.width('&');
            w = qMax(w, QApplication::globalStrut().width());
            h = qMax(fm.height() + motifItemVMargin, QApplication::globalStrut().height());
        } else if (mi->isSeparator()) {        // separator item
            if (style().styleHint(QStyle::SH_GUIStyle) == Qt::MotifStyle)
                separator = i; //### only motif?
        }
        if (!mi->isSeparator() || mi->widget()) {
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
            if (!mac_eaten_menubar) {
#endif
                if (gs == Qt::MotifStyle) {
                    w += 2*motifItemFrame;
                    h += 2*motifItemFrame;
                }
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
            }
#endif

            if (((!reverse && x + w + frameWidth() - max_width > 0) ||
                 (reverse && x - w - itemSpacing - frameWidth() < 0))
                 && nlitems > 0) {
                nlitems = 0;
                x = frameWidth();
                y += h;
                if (gs == Qt::MotifStyle) {
                    x += motifBarHMargin;
                    y += motifBarVMargin;
                }
                if (reverse)
                    x = max_width - x + itemSpacing;
                if (style().styleHint(QStyle::SH_GUIStyle) == Qt::MotifStyle)
                    separator = -1;
            }
            if (y + h + 2*frameWidth() > max_height)
                max_height = y + h + 2*frameWidth();
            if (h > max_item_height)
                max_item_height = h;
        }

        const bool isLast = (i == lastItem);

        if(reverse) {
            x -= w;
            if (!isLast && !mi->isSeparator())
                x -= itemSpacing;
        }
        if (update) {
            irects[i].setRect(x, y, w, h);
        }
        if (!reverse) {
            x += w;
            if (!isLast && !mi->isSeparator())
                x += itemSpacing;
        }
        nlitems++;
        i++;
    }
    if (gs == Qt::WindowsStyle) {
        max_height += 2;
        max_width += 2;
    }

    if (update) {
        if (separator >= 0) {
            int moveBy = reverse ? - x - frameWidth() : max_width - x - frameWidth();
            rightSide = x;
            while(--i >= separator) {
                irects[i].moveBy(moveBy, 0);
            }
        } else {
            rightSide = width()-frameWidth();
        }
        if (max_height != height())
            resize(width(), max_height);
        for (i = 0; i < (int)mitems->count(); i++) {
            irects[i].setHeight(max_item_height );
            Q3MenuItem *mi = mitems->at(i);
            if (mi->widget()) {
                QRect r (QPoint(0,0), mi->widget()->sizeHint());
                r.moveCenter(irects[i].center());
                mi->widget()->setGeometry(r);
                if(mi->widget()->isHidden())
                    mi->widget()->show();
            }
        }
        badSize = false;
    }

    return max_height;
}

/*!
    Returns the height that the menu would resize itself to if its
    parent (and hence itself) resized to the given \a max_width. This
    can be useful for simple layout tasks in which the height of the
    menu bar is needed after items have been inserted. See \l
    showimg/showimg.cpp for an example of the usage.
*/
int Q3MenuBar::heightForWidth(int max_width) const
{
    // Okay to cast away const, as we are not updating.
    if (max_width < 0) max_width = 0;
    return ((Q3MenuBar*)this)->calculateRects(max_width);
}

/*!
  \internal
  Return the bounding rectangle for the menu item at position \a index.
*/

QRect Q3MenuBar::itemRect(int index)
{
    calculateRects();
    return irects ? irects[index] : QRect(0,0,0,0);
}

/*!
  \internal
  Return the item at \a pos, or -1 if there is no item there or if
  it is a separator item.
*/

int Q3MenuBar::itemAtPos(const QPoint &pos_)
{
    calculateRects();
    if (!irects)
        return -1;
    int i = 0;
    QPoint pos = pos_;
    // Fitts' Law for edges - compensate for the extra margin
    // added in calculateRects()
    const int margin = 2;
    pos.setX(qMax(margin, qMin(width() - margin, pos.x())));
    pos.setY(qMax(margin, qMin(height() - margin, pos.y())));
    while (i < (int)mitems->count()) {
        if (!irects[i].isEmpty() && irects[i].contains(pos)) {
            Q3MenuItem *mi = mitems->at(i);
            return mi->isSeparator() ? -1 : i;
        }
        ++i;
    }
    return -1;                                        // no match
}


/*!
  \property Q3MenuBar::separator
  \brief in which cases a menubar sparator is drawn

  \obsolete
*/
void Q3MenuBar::setSeparator(Separator when)
{
    mseparator = when;
}

Q3MenuBar::Separator Q3MenuBar::separator() const
{
    return mseparator ? InWindowsStyle : Never;
}

/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!
    Called from Q3Frame::paintEvent(). Draws the menu bar contents
    using painter \a p.
*/
void Q3MenuBar::drawContents(QPainter *p)
{
    performDelayedChanges();
    QRegion reg(contentsRect());
    QPalette pal = palette();

    // this shouldn't happen
    if (!irects)
        return;

    QStyleOptionMenuItem opt(0);
    for (int i = 0; i < mitems->count(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (!mi->text().isEmpty() || mi->pixmap()) {
            QRect r = irects[i];
            if(r.isEmpty() || !mi->isVisible())
                continue;
            reg = reg.subtract(r);
            opt.rect = r;
            opt.state = QStyle::Style_Default;
            opt.palette = pal;
            opt.menuRect = rect();
            opt.text = mi->text();
            if (mi->pixmap())
                opt.icon = *mi->pixmap();
            opt.checkState = QStyleOptionMenuItem::NotCheckable;
            opt.menuItemType = QStyleOptionMenuItem::Normal;
            opt.maxIconWidth = 0;
            opt.tabWidth = 0;
            bool eav = mi->isEnabledAndVisible();
            if (!eav)
                opt.palette.setCurrentColorGroup(QPalette::Disabled);
            if (isEnabled() && eav)
                opt.state |= QStyle::Style_Enabled;
            if (i == actItem)
                opt.state |= QStyle::Style_Active;
            if (actItemDown)
                opt.state |= QStyle::Style_Down;
            if (hasFocus() || hasmouse || popupvisible)
                opt.state |= QStyle::Style_HasFocus;
            style().drawControl(QStyle::CE_MenuBarItem, &opt, p, this);
        }
    }

    p->save();
    p->setClipRegion(reg);
    opt.state = QStyle::Style_Default;
    opt.rect = contentsRect();
    opt.text = "";
    opt.icon = QIconSet();
    style().drawControl(QStyle::CE_MenuBarEmptyArea, &opt, p, this);
    p->restore();

#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    if (!mac_eaten_menubar)
#endif
    {
        Qt::GUIStyle gs = (Qt::GUIStyle) style().styleHint(QStyle::SH_GUIStyle);
        if (mseparator == InWindowsStyle && gs == Qt::WindowsStyle) {
            p->setPen(pal.light());
            p->drawLine(0, height()-1, width()-1, height()-1);
            p->setPen(pal.dark());
            p->drawLine(0, height()-2, width()-1, height()-2);
        }
    }
}


/*!
    \reimp
*/
void Q3MenuBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    mouseBtDn = true;                                // mouse button down
    int item = itemAtPos(e->pos());
    if (item == actItem && popupvisible)
        toggleclose = 1;
    if (item >= 0) {
        QFocusEvent::Reason oldReason = QFocusEvent::reason();
        Q3MenuItem *mi = findItem(idAt(item));
        // we know that a popup will open, so set the reason to avoid
        // itemviews to redraw their selections
        if (mi && mi->popup())
            QFocusEvent::setReason(QFocusEvent::Popup);
        setAltMode(true);
        QFocusEvent::setReason(oldReason);
    }
    setActiveItem(item, true, false);
}


/*!
    \reimp
*/
void Q3MenuBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    if (!mouseBtDn)
        return;
    mouseBtDn = false;                                // mouse button up
    int item = itemAtPos(e->pos());
    if (item >= 0 && !mitems->at(item)->isEnabledAndVisible() ||
         actItem >= 0 && !mitems->at(actItem)->isEnabledAndVisible()) {
        hidePopups();
        setActiveItem(-1);
        return;
    }
    bool showMenu = true;
    if (toggleclose &&
         // pressing an item twice closes in windows, but not in motif :/
         style().styleHint(QStyle::SH_GUIStyle) == Qt::WindowsStyle &&
         actItem == item) {
        showMenu = false;
        setAltMode(false);
    }
    setActiveItem(item, showMenu, !hasMouseTracking());
    toggleclose = 0;
}


/*!
    \reimp
*/
void Q3MenuBar::mouseMoveEvent(QMouseEvent *e)
{
    int item = itemAtPos(e->pos());
    if (!mouseBtDn && !popupvisible) {
        if (item >= 0) {
            if (!hasmouse) {
                hasmouse = 1;
                if (actItem == item)
                    actItem = -1; // trigger update
            }
        }
        setActiveItem(item, false, false);
        return;
    }
    if (item != actItem && item >= 0  && (popupvisible || mouseBtDn))
        setActiveItem(item, true, false);
}


/*!
    \reimp
*/
void Q3MenuBar::leaveEvent(QEvent * e)
{
    hasmouse = 0;
    int actId = idAt(actItem);
    if (!hasFocus() && !popupvisible)
        actItem = -1;
    updateItem(actId);
    Q3Frame::leaveEvent(e);
}


/*!
    \reimp
*/
void Q3MenuBar::keyPressEvent(QKeyEvent *e)
{
    if (actItem < 0)
        return;

    Q3MenuItem  *mi = 0;
    int dx = 0;

    if (e->state() & Qt::ControlButton &&
         (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab))
    {
        e->ignore();
        return;
    }

    switch (e->key()) {
     case Qt::Key_Left:
        dx = QApplication::reverseLayout() ? 1 : -1;
        break;

    case Qt::Key_Right:
    case Qt::Key_Tab:
        dx = QApplication::reverseLayout() ? -1 : 1;
        break;

    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation))
            setActiveItem(actItem);
        break;

    case Qt::Key_Escape:
        setAltMode(false);
        break;
    }

    if (dx) {                                        // highlight next/prev
        register int i = actItem;
        int c = mitems->count();
        int n = c;
        while (n--) {
            i = i + dx;
            if (i == c)
                i = 0;
            else if (i < 0)
                i = c - 1;
            mi = mitems->at(i);
            // ### fix windows-style traversal - currently broken due to
            // Q3MenuBar's reliance on Q3PopupMenu
            if (/* (style() == Qt::WindowsStyle || */ mi->isEnabledAndVisible() /*) */
                 && !mi->isSeparator())
                break;
        }
        setActiveItem(i, popupvisible        );
    } else if ((!e->state() || (e->state()&(Qt::MetaButton|Qt::AltButton))) && e->text().length()==1 && !popupvisible) {
        QChar c = e->text()[0].toUpper();

        Q3MenuItem* first = 0;
        Q3MenuItem* currentSelected = 0;
        Q3MenuItem* firstAfterCurrent = 0;

        int indx = 0;
        int clashCount = 0;
        for (int i = 0; i < mitems->size(); ++i) {
            register Q3MenuItem *m = mitems->at(i);
            QString s = m->text();
            if (!s.isEmpty()) {
                int i = s.indexOf('&');
                if (i >= 0)
                {
                    if (s[i+1].toUpper() == c) {
                        clashCount++;
                        if (!first)
                            first = m;
                        if (indx == actItem)
                            currentSelected = m;
                        else if (!firstAfterCurrent && currentSelected)
                            firstAfterCurrent = m;
                    }
                }
            }
            indx++;
        }
        if (0 == clashCount) {
            return;
        } else if (1 == clashCount) {
            indx = indexOf(first->id());
        } else {
            // If there's clashes and no one is selected, use first one
            // or if there is no clashes _after_ current, use first one
            if (!currentSelected || (currentSelected && !firstAfterCurrent))
                indx = indexOf(first->id());
            else
                indx = indexOf(firstAfterCurrent->id());
        }

        setActiveItem(indx);
    }
}


/*!
    \reimp
*/
void Q3MenuBar::resizeEvent(QResizeEvent *e)
{
    Q3Frame::resizeEvent(e);
    if (badSize)
        return;
    badSize = true;
    calculateRects();
}

/*
    Sets actItem to \a i and calls repaint for the changed things.

    Takes care to optimize the repainting. Assumes that
    calculateRects() has been called as appropriate.
*/

void Q3MenuBar::setActiveItem(int i, bool show, bool activate_first_item)
{
    if (i == actItem && (uint)show == popupvisible)
        return;

    Q3MenuItem* mi = 0;
    if (i >= 0)
        mi = mitems->at(i);
    if (mi && !mi->isEnabledAndVisible())
        return;

    popupvisible = i >= 0 ? (show) : 0;
    actItemDown = popupvisible;

    if (i < 0 || actItem < 0) {
        // just one item needs repainting
        int n = qMax(actItem, i);
        actItem = i;
        if (irects && n >= 0)
            repaint(irects[n]);
    } else if (QABS(i-actItem) == 1) {
        // two neighbouring items need repainting
        int o = actItem;
        actItem = i;
        if (irects)
            repaint(irects[i].unite(irects[o]));
    } else {
        // two non-neighbouring items need repainting
        int o = actItem;
        actItem = i;
        if (irects) {
            repaint(irects[o]);
            repaint(irects[i]);
        }
    }

    hidePopups();

    if (!popupvisible && actItem >= 0 && irects) {
        QRect mfrect = irects[actItem];
        setMicroFocusHint(mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), false);
    }

#ifndef QT_NO_ACCESSIBILITY
    if (mi)
        QAccessible::updateAccessibility(this, indexOf(mi->id())+1, QAccessible::Focus);
#endif

    if (actItem < 0 || !popupvisible || !mi )
        return;

    Q3PopupMenu *popup = mi->popup();
    if (popup) {
        emit highlighted(mi->id());
        openActPopup();
        if (activate_first_item)
            popup->setFirstItemActive();
    } else {                                // not a popup
        goodbye(false);
        if (mi->signal()) {
            int value = mi->signalValue();
            mi->signal()->activate(&value);
        }
        emit activated(mi->id());
    }
}


void Q3MenuBar::setAltMode(bool enable)
{
#ifndef QT_NO_ACCESSIBILITY
    if (inMenu && !enable) {
        QAccessible::updateAccessibility(this, 0, QAccessible::MenuEnd);
        inMenu = false;
    } else if (!inMenu && enable) {
        QAccessible::updateAccessibility(this, 0, QAccessible::MenuStart);
        inMenu = true;
    }
#endif

    waitforalt = 0;
    actItemDown = false;
    if (enable) {
        if (!Q3MenuData::d->aWidget)
            Q3MenuData::d->aWidget = qApp->focusWidget();
        setFocus();
        updateItem(idAt(actItem));
    } else {
        // set the focus back to the previous widget if
        // we still have the focus.
        if (qApp->focusWidget() == this) {
            if (Q3MenuData::d->aWidget)
                Q3MenuData::d->aWidget->setFocus();
            else
                clearFocus();
        }
        int actId = idAt(actItem);
        actItem = -1;
        updateItem(actId);
        Q3MenuData::d->aWidget = 0;
    }
}

/*!
    Sets up keyboard accelerators for the menu bar.
*/
#ifndef QT_NO_ACCEL

void Q3MenuBar::setupAccelerators()
{
    delete autoaccel;
    autoaccel = 0;

    for (int i = 0; i < mitems->size(); ++i) {
        register Q3MenuItem *mi = mitems->at(i);
        if (!mi->isEnabledAndVisible()) // ### when we have a good solution for the accel vs. focus widget problem, remove that. That is only a workaround
            continue;
        QString s = mi->text();
        if (!s.isEmpty()) {
            int i = QAccel::shortcutKey(s);
            if (i) {
                if (!autoaccel) {
                    autoaccel = new QAccel(this);
                    autoaccel->setIgnoreWhatsThis(true);
                    connect(autoaccel, SIGNAL(activated(int)),
                             SLOT(accelActivated(int)));
                    connect(autoaccel, SIGNAL(activatedAmbiguously(int)),
                             SLOT(accelActivated(int)));
                    connect(autoaccel, SIGNAL(destroyed()),
                             SLOT(accelDestroyed()));
                }
                autoaccel->insertItem(i, mi->id());
            }
        }
        if (mi->popup()) {
            Q3PopupMenu* popup = mi->popup();
            popup->updateAccel(this);
            if (!popup->isEnabled())
                popup->enableAccel(false);
        }
    }
}
#endif

/*!
    \reimp
 */
void Q3MenuBar::focusInEvent(QFocusEvent *)
{
    if (actItem < 0) {
        int i = -1;
        while (actItem < 0 && ++i < (int) mitems->count()) {
            Q3MenuItem* mi = mitems->at(i);
            if (mi && mi->isEnabledAndVisible() && !mi->isSeparator())
                setActiveItem(i, false);
        }
    } else if (!popupvisible) {
        updateItem(idAt(actItem));
    }
}

/*!
    \reimp
 */
void Q3MenuBar::focusOutEvent(QFocusEvent *)
{
    updateItem(idAt(actItem));
    if (!popupvisible)
        setAltMode(false);
}

/*!
    \reimp
*/

QSize Q3MenuBar::sizeHint() const
{
    int h = height();
    if (badSize)
        h = ((Q3MenuBar*)this)->calculateRects();
    QSize s(2*frameWidth(),0);
    if (irects) {
        for (int i = 0; i < (int)mitems->count(); ++i)
            s.setWidth(s.width() + irects[i].width() + 2);
    }
    s.setHeight(h);
    QStyleOptionMenuItem opt(0);
    opt.rect = rect();
    opt.menuRect = rect();
    opt.state = QStyle::Style_Default;
    opt.menuItemType = QStyleOptionMenuItem::Normal;
    opt.checkState = QStyleOptionMenuItem::NotCheckable;
    opt.palette = palette();
    return (style().sizeFromContents(QStyle::CT_MenuBar, &opt,
                                     s.expandedTo(QApplication::globalStrut()),
                                     fontMetrics(), this));
}

/*!
    \reimp
*/

QSize Q3MenuBar::minimumSize() const
{
#ifndef QT_NO_TOOLBAR
    Q3ToolBar *tb = qt_cast<Q3ToolBar*>(parent());
    if (tb)
        return sizeHint();
#endif
    return Q3Frame::minimumSize();
}

/*!
    \reimp
*/

QSize Q3MenuBar::minimumSizeHint() const
{
    return minimumSize();
}

/*!
    \property Q3MenuBar::defaultUp
    \brief the popup orientation

    The default popup orientation. By default, menus pop "down" the
    screen. By setting the property to true, the menu will pop "up".
    You might call this for menus that are \e below the document to
    which they refer.

    If the menu would not fit on the screen, the other direction is
    used automatically.
*/
void Q3MenuBar::setDefaultUp(bool on)
{
    defaultup = on;
}

bool Q3MenuBar::isDefaultUp() const
{
    return defaultup;
}


/*!
    \reimp
 */
void Q3MenuBar::activateItemAt(int index)
{
    if (index >= 0 && index < (int) mitems->count())
        setActiveItem(index);
    else
        goodbye(false);
}

#endif // QT_NO_MENUBAR
