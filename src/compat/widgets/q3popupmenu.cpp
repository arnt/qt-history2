/****************************************************************************
**
** Implementation of Q3PopupMenu class.
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

#include "q3popupmenu.h"
#ifndef QT_NO_POPUPMENU
#include "q3menubar.h"
#include "qaccel.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qdesktopwidget.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qpointer.h"
#include "qsignal.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtimer.h"
#include "qwhatsthis.h"
#include <private/qeffects_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

//#define ANIMATED_POPUP
//#define BLEND_POPUP

// Motif style parameters

static const int motifArrowHMargin        = 6;        // arrow horizontal margin
static const int motifArrowVMargin        = 2;        // arrow vertical margin


/*

+-----------------------------
|      PopupFrame
|   +-------------------------
|   |           ItemFrame
|   |        +---------------------
|   |        |
|   |        |                           \
|   |        |   ^        T E X T          ^            | ItemVMargin
|   |        |   |                  |           /
|   |              ItemHMargin
|

*/

#if 0
# define DEBUG_SLOPPY_SUBMENU
#endif

// used for internal communication
static Q3PopupMenu * syncMenu = 0;
static int syncMenuId = 0;

// Used to detect motion prior to mouse-release
static int motion;

// used to provide ONE single-shot timer
static QTimer * singleSingleShot = 0;

static bool supressAboutToShow = false;

static void cleanup()
{
    delete singleSingleShot;
    singleSingleShot = 0;
}

static void popupSubMenuLater(int msec, Q3PopupMenu * receiver) {
    if (!singleSingleShot) {
        singleSingleShot = new QTimer(qApp, "popup submenu timer");
        qAddPostRoutine(cleanup);
    }

    singleSingleShot->disconnect(SIGNAL(timeout()));
    QObject::connect(singleSingleShot, SIGNAL(timeout()),
                      receiver, SLOT(subMenuTimer()));
    singleSingleShot->start(msec, true);
}

static bool preventAnimation = false;

static QStyleOptionMenuItem getStyleOption(const Q3PopupMenu *pop, const Q3MenuItem *mi)
{
    QStyleOptionMenuItem opt(0);
    opt.palette = pop->palette();
    opt.state = QStyle::Style_Default;
    opt.menuRect = pop->rect();
    if (mi->isSeparator()) {
        opt.menuItemType = QStyleOptionMenuItem::Separator;
    } else if (mi->popup()) {
        opt.menuItemType = QStyleOptionMenuItem::SubMenu;
    } else if (mi->custom()) {
        opt.menuItemType = QStyleOptionMenuItem::Q3Custom;
        opt.q3CustomItemSizeHint = mi->custom()->sizeHint();
        opt.q3CustomItemFullSpan = mi->custom()->fullSpan();
    } else {
        opt.menuItemType = QStyleOptionMenuItem::Normal;
    }
    if (pop->isCheckable()) {
        if (mi->isChecked())
            opt.checkState = QStyleOptionMenuItem::Checked;
        else
            opt.checkState = QStyleOptionMenuItem::Unchecked;
    } else {
        opt.checkState = QStyleOptionMenuItem::NotCheckable;
    }
    opt.text = mi->text();
    if (mi->iconSet())
        opt.icon = *mi->iconSet();
    else if (mi->pixmap())
        opt.icon = *mi->pixmap();
    if (pop->isEnabled() && mi->isEnabledAndVisible())
        opt.state |= QStyle::Style_Enabled;
    return opt;
}


/*!
    \class Q3PopupMenu q3popupmenu.h
    \brief The Q3PopupMenu class provides a popup menu widget.

    \ingroup application
    \ingroup basic
    \mainclass

    A popup menu widget is a selection menu. It can be either a
    pull-down menu in a menu bar or a standalone context (popup) menu.
    Pull-down menus are shown by the menu bar when the user clicks on
    the respective item or presses the specified shortcut key. Use
    Q3MenuBar::insertItem() to insert a popup menu into a menu bar.
    Show a context menu either asynchronously with popup() or
    synchronously with exec().

    Technically, a popup menu consists of a list of menu items. You
    add items with insertItem(). An item is either a string, a pixmap
    or a custom item that provides its own drawing function (see
    Q3CustomMenuItem). In addition, items can have an optional icon
    drawn on the very left side and an accelerator key such as
    "Ctrl+X".

    There are three kinds of menu items: separators, menu items that
    perform an action and menu items that show a submenu. Separators
    are inserted with insertSeparator(). For submenus, you pass a
    pointer to a Q3PopupMenu in your call to insertItem(). All other
    items are considered action items.

    When inserting action items you usually specify a receiver and a
    slot. The receiver will be notifed whenever the item is selected.
    In addition, Q3PopupMenu provides two signals, activated() and
    highlighted(), which signal the identifier of the respective menu
    item. It is sometimes practical to connect several items to one
    slot. To distinguish between them, specify a slot that takes an
    integer argument and use setItemParameter() to associate a unique
    value with each item.

    You clear a popup menu with clear() and remove single items with
    removeItem() or removeItemAt().

    A popup menu can display check marks for certain items when
    enabled with setCheckable(true). You check or uncheck items with
    setItemChecked().

    Items are either enabled or disabled. You toggle their state with
    setItemEnabled(). Just before a popup menu becomes visible, it
    emits the aboutToShow() signal. You can use this signal to set the
    correct enabled/disabled states of all menu items before the user
    sees it. The corresponding aboutToHide() signal is emitted when
    the menu hides again.

    You can provide What's This? help for single menu items with
    setWhatsThis(). See QWhatsThis for general information about this
    kind of lightweight online help.

    For ultimate flexibility, you can also add entire widgets as items
    into a popup menu (for example, a color selector).

    A Q3PopupMenu can also provide a tear-off menu. A tear-off menu is
    a top-level window that contains a copy of the menu. This makes it
    possible for the user to "tear off" frequently used menus and
    position them in a convenient place on the screen. If you want
    that functionality for a certain menu, insert a tear-off handle
    with insertTearOffHandle(). When using tear-off menus, bear in
    mind that the concept isn't typically used on Microsoft Windows so
    users may not be familiar with it. Consider using a QToolBar
    instead. Tear-off menus cannot contain custom widgets; if the
    original menu contains a custom widget item, this item is omitted.

    \link menu-example.html menu/menu.cpp\endlink is an example of
    Q3MenuBar and Q3PopupMenu use.

    \important insertItem removeItem removeItemAt clear text pixmap iconSet insertSeparator changeItem whatsThis setWhatsThis accel setAccel setItemEnabled isItemEnabled setItemVisible isItemVisible setItemChecked isItemChecked connectItem disconnectItem setItemParameter itemParameter

    \sa Q3MenuBar
    \link guibooks.html#fowler GUI Design Handbook: Menu, Drop-Down and
    Pop-Up\endlink
*/


/*!
    \fn void Q3PopupMenu::aboutToShow()

    This signal is emitted just before the popup menu is displayed.
    You can connect it to any slot that sets up the menu contents
    (e.g. to ensure that the right items are enabled).

    \sa aboutToHide(), setItemEnabled(), setItemChecked(), insertItem(), removeItem()
*/

/*!
    \fn void Q3PopupMenu::aboutToHide()

    This signal is emitted just before the popup menu is hidden after
    it has been displayed.

    \warning Do not open a widget in a slot connected to this signal.

    \sa aboutToShow(), setItemEnabled(), setItemChecked(), insertItem(), removeItem()
*/

/*!
    \fn void Q3PopupMenu::setWhatsThis(int id, const QString& s)

    Sets the menu text for menu item \a id to the string \a s.
*/

/*!
    \fn QString Q3PopupMenu::whatsThis(int id) const

    Returns the menu text for menu item \a id.
*/


/*****************************************************************************
  Q3PopupMenu member functions
 *****************************************************************************/

class Q3MenuDataData {
    // attention: also defined in q3menudata.cpp
public:
    Q3MenuDataData();
    QPointer<Q3PopupMenu> aPopup;
    int aInt;
};

class Q3PopupMenuPrivate {
public:
    struct Scroll {
        enum { ScrollNone=0, ScrollUp=0x01, ScrollDown=0x02 };
        uint scrollable : 2;
        uint direction : 1;
        int topScrollableIndex, scrollableSize;
        QTime lastScroll;
        QTimer *scrolltimer;
    } scroll;
    QSize calcSize;
    QRegion mouseMoveBuffer;
};

static Q3PopupMenu* active_popup_menu = 0;

/*!
    Constructs a popup menu called \a name with parent \a parent.

    Although a popup menu is always a top-level widget, if a parent is
    passed the popup menu will be deleted when that parent is
    destroyed (as with any other QObject).
*/

Q3PopupMenu::Q3PopupMenu(QWidget *parent, const char *name)
    : Q3Frame(parent, name, Qt::WType_Popup)
{
    d = new Q3PopupMenuPrivate;
    d->scroll.scrollableSize = d->scroll.topScrollableIndex = 0;
    d->scroll.scrollable = Q3PopupMenuPrivate::Scroll::ScrollNone;
    d->scroll.scrolltimer = 0;
    isPopupMenu          = true;
#ifndef QT_NO_ACCEL
    autoaccel          = 0;
    accelDisabled = false;
#endif
    popupActive          = -1;
    snapToMouse          = true;
    tab = 0;
    checkable = 0;
    tornOff = 0;
    pendingDelayedContentsChanges = 0;
    pendingDelayedStateChanges = 0;
    maxPMWidth = 0;

    tab = 0;
    ncols = 1;
    setFrameStyle(Q3Frame::PopupPanel | Q3Frame::Raised);
    setMouseTracking(style().styleHint(QStyle::SH_Menu_MouseTracking, this));
    setBackgroundRole(QPalette::Button);
    connectModalRecursionSafety = 0;

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_CustomWhatsThis);
}

/*!
    Destroys the popup menu.
*/

Q3PopupMenu::~Q3PopupMenu()
{
    if (syncMenu == this && qApp) {
        qApp->exit_loop();
        syncMenu = 0;
    }

    if(d->scroll.scrolltimer)
        delete d->scroll.scrolltimer;

    if (isVisible()) {
        parentMenu = 0;
        hidePopups();
    }

    delete (Q3PopupMenu*) Q3MenuData::d->aPopup;  // tear-off menu

    preventAnimation = false;
    delete d;
}


/*!
    Updates the item with identity \a id.
*/
void Q3PopupMenu::updateItem(int id)                // update popup menu item
{
    updateRow(indexOf(id));
}


void Q3PopupMenu::setCheckable(bool enable)
{
    if (isCheckable() != enable) {
        checkable = enable;
        badSize = true;
        if (Q3MenuData::d->aPopup)
            Q3MenuData::d->aPopup->setCheckable(enable);
    }
}

/*!
    \property Q3PopupMenu::checkable
    \brief whether the display of check marks on menu items is enabled

    When true, the display of check marks on menu items is enabled.
    Checking is always enabled when in Windows-style.

    \sa Q3MenuData::setItemChecked()
*/

bool Q3PopupMenu::isCheckable() const
{
    return checkable;
}

void Q3PopupMenu::menuContentsChanged()
{
    // here the part that can't be delayed
    Q3MenuData::menuContentsChanged();
    badSize = true;                                // might change the size
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    mac_dirty_popup = 1;
#endif
    if(pendingDelayedContentsChanges)
        return;
    pendingDelayedContentsChanges = 1;
    if(!pendingDelayedStateChanges) // if the timer hasn't been started yet
        QTimer::singleShot(0, this, SLOT(performDelayedChanges()));
}

void Q3PopupMenu::performDelayedContentsChanged()
{
    pendingDelayedContentsChanges = 0;
    // here the part the can be delayed
#ifndef QT_NO_ACCEL
    // if performDelayedStateChanged() will be called too,
    // it will call updateAccel() too, no need to do it twice
    if(!pendingDelayedStateChanges)
        updateAccel(0);
#endif
    if (isVisible()) {
        if (tornOff)
            return;
        updateSize(true);
        update();
    }
    Q3PopupMenu* p = Q3MenuData::d->aPopup;
    if (p && p->isVisible()) {
        if (p->mitemsAutoDelete) {
            while (!p->mitems->isEmpty())
                delete p->mitems->takeFirst();
        } else {
            p->mitems->clear();
        }
        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *mi = mitems->at(i);
            if (mi->id() != Q3MenuData::d->aInt && !mi->widget())
                p->mitems->append(mi);
        }
        p->updateSize(true);
        p->update();
    }
#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    mac_dirty_popup = 1;
#endif
}


void Q3PopupMenu::menuStateChanged()
{
    // here the part that can't be delayed
    if(pendingDelayedStateChanges)
        return;
    pendingDelayedStateChanges = 1;
    if(!pendingDelayedContentsChanges) // if the timer hasn't been started yet
        QTimer::singleShot(0, this, SLOT(performDelayedChanges()));
}

void Q3PopupMenu::performDelayedStateChanged()
{
    pendingDelayedStateChanges = 0;
    // here the part that can be delayed
#ifndef QT_NO_ACCEL
    updateAccel(0); // ### when we have a good solution for the accel vs. focus widget problem, remove that. That is only a workaround
    // if you remove this, see performDelayedContentsChanged()
#endif
    update();
    if (Q3MenuData::d->aPopup)
        Q3MenuData::d->aPopup->update();
}

void Q3PopupMenu::performDelayedChanges()
{
    if(pendingDelayedContentsChanges)
        performDelayedContentsChanged();
    if(pendingDelayedStateChanges)
        performDelayedStateChanged();
}

void Q3PopupMenu::menuInsPopup(Q3PopupMenu *popup)
{
    connect(popup, SIGNAL(activatedRedirect(int)),
             SLOT(subActivated(int)));
    connect(popup, SIGNAL(highlightedRedirect(int)),
             SLOT(subHighlighted(int)));
    connect(popup, SIGNAL(destroyed(QObject*)),
             this, SLOT(popupDestroyed(QObject*)));
}

void Q3PopupMenu::menuDelPopup(Q3PopupMenu *popup)
{
    popup->disconnect(SIGNAL(activatedRedirect(int)));
    popup->disconnect(SIGNAL(highlightedRedirect(int)));
    disconnect(popup, SIGNAL(destroyed(QObject*)),
                this, SLOT(popupDestroyed(QObject*)));
}


void Q3PopupMenu::frameChanged()
{
    menuContentsChanged();
}

/*!
    Displays the popup menu so that the item number \a indexAtPoint
    will be at the specified \e global position \a pos. To translate a
    widget's local coordinates into global coordinates, use
    QWidget::mapToGlobal().

    When positioning a popup with exec() or popup(), bear in mind that
    you cannot rely on the popup menu's current size(). For
    performance reasons, the popup adapts its size only when
    necessary, so in many cases, the size before and after the show is
    different. Instead, use sizeHint(). It calculates the proper size
    depending on the menu's current contents.
*/

void Q3PopupMenu::popup(const QPoint &pos, int indexAtPoint)
{
    if (!isPopup() && isVisible())
        hide();

    //avoid circularity
    if (isVisible() || !isEnabled())
        return;

#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    if(macPopupMenu(pos, indexAtPoint))
        return;
#endif

    if(d->scroll.scrollable) {
        d->scroll.scrollable = Q3PopupMenuPrivate::Scroll::ScrollNone;
        d->scroll.topScrollableIndex = d->scroll.scrollableSize = 0;
        badSize = true;
    }
    updateSize();

    QPoint mouse = QCursor::pos();
    snapToMouse = pos == mouse;

    // have to emit here as a menu might be setup in a slot connected
    // to aboutToShow which will change the size of the menu
    bool s = supressAboutToShow;
    supressAboutToShow = true;
    if (!s) {
        emit aboutToShow();
        updateSize(true);
    }

    int screen_num;
    if (QApplication::desktop()->isVirtualDesktop())
        screen_num =
            QApplication::desktop()->screenNumber(QApplication::reverseLayout() ?
                                                   pos+QPoint(width(),0) : pos);
    else
        screen_num = QApplication::desktop()->screenNumber(this);
#ifdef Q_WS_MAC
    QRect screen = QApplication::desktop()->availableGeometry(screen_num);
#else
    QRect screen = QApplication::desktop()->screenGeometry(screen_num);
#endif
    int sw = screen.width();                        // screen width
    int sh = screen.height();                        // screen height
    int sx = screen.x();                        // screen pos
    int sy = screen.y();
    int x  = pos.x();
    int y  = pos.y();
    if (indexAtPoint >= 0)                        // don't subtract when < 0
        y -= itemGeometry(indexAtPoint).y();                // (would subtract 2 pixels!)
    int w  = width();
    int h  = height();

    if (snapToMouse) {
        if (qApp->reverseLayout())
            x -= w;
        if (x+w > sx+sw)
            x = mouse.x()-w;
        if (y+h > sy+sh)
            y = mouse.y()-h;
        if (x < sx)
            x = mouse.x();
        if (y < sy)
            y = sy;
    }

    if(style().styleHint(QStyle::SH_Menu_Scrollable, this)) {
        int off_top = 0, off_bottom = 0;
        if(y+h > sy+sh)
            off_bottom = (y+h) - (sy+sh);
        if(y < sy)
            off_top = sy - y;
        if(off_bottom || off_top) {
            int ch = updateSize().height(); //store the old height, before setting scrollable --Sam
            const int vextra = style().pixelMetric(QStyle::PM_MenuVMargin, this);
            d->scroll.scrollableSize = h - off_top - off_bottom - 2*vextra;
            if(off_top) {
                move(x, y = sy);
                d->scroll.scrollable = d->scroll.scrollable | Q3PopupMenuPrivate::Scroll::ScrollUp;
            }
            if(off_bottom)
                d->scroll.scrollable = d->scroll.scrollable | Q3PopupMenuPrivate::Scroll::ScrollDown;
            if(off_top != off_bottom && indexAtPoint >= 0) {
                ch -= (vextra * 2);
                if(ch > sh) //no bigger than the screen!
                    ch = sh;
                if(ch > d->scroll.scrollableSize)
                    d->scroll.scrollableSize = ch;
            }

            updateSize(true); //now set the size using the scrollable/scrollableSize as above
            w = width();
            h = height();
            if(off_top && indexAtPoint >= 0) { //scroll to it
                int tmp_y = 0;
                for (int i = 0; i < mitems->size(); ++i) {
                    if (tmp_y >= off_top)
                        break;
                    Q3MenuItem *mi = mitems->at(i);
                    QStyleOptionMenuItem opt = getStyleOption(this, mi);
                    opt.rect = rect();
                    opt.tabWidth = 0;
                    opt.maxIconWidth = maxPMWidth;
                    QSize sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt,
                                                        QSize(0, itemHeight(mi)), fontMetrics(),
                                                        this);
                    tmp_y += sz.height();
                    d->scroll.topScrollableIndex++;
                }
            }
        }
    }
    if (x+w > sx+sw)                                // the complete widget must
        x = sx+sw - w;                                //   be visible
    if (y+h > sy+sh)
        y = sy+sh - h;
    if (x < sx)
        x = sx;
    if (y < sy)
        y = sy;
    move(x, y);
    motion=0;
    actItem = -1;

#ifndef QT_NO_EFFECTS
    int hGuess = qApp->reverseLayout() ? QEffects::LeftScroll : QEffects::RightScroll;
    int vGuess = QEffects::DownScroll;
    if (qApp->reverseLayout()) {
        if (snapToMouse && (x + w/2 > mouse.x()) ||
             (parentMenu && parentMenu->isPopupMenu &&
               (x + w/2 > ((Q3PopupMenu*)parentMenu)->x())))
            hGuess = QEffects::RightScroll;
    } else {
        if (snapToMouse && (x + w/2 < mouse.x()) ||
             (parentMenu && parentMenu->isPopupMenu &&
               (x + w/2 < ((Q3PopupMenu*)parentMenu)->x())))
            hGuess = QEffects::LeftScroll;
    }

#ifndef QT_NO_MENUBAR
    if (snapToMouse && (y + h/2 < mouse.y()) ||
         (parentMenu && parentMenu->isMenuBar &&
           (y + h/2 < ((Q3MenuBar*)parentMenu)->mapToGlobal(((Q3MenuBar*)parentMenu)->pos()).y())))
        vGuess = QEffects::UpScroll;
#endif

    if (QApplication::isEffectEnabled(Qt::UI_AnimateMenu) &&
         preventAnimation == false) {
        if (QApplication::isEffectEnabled(Qt::UI_FadeMenu))
            qFadeEffect(this);
        else if (parentMenu)
            qScrollEffect(this, parentMenu->isPopupMenu ? hGuess : vGuess);
        else
            qScrollEffect(this, hGuess | vGuess);
    } else
#endif
        {
            show();
        }
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuStart);
#endif
}

/*!
    \fn void Q3PopupMenu::activated(int id)

    This signal is emitted when a menu item is selected; \a id is the
    id of the selected item.

    Normally, you connect each menu item to a single slot using
    Q3MenuData::insertItem(), but sometimes you will want to connect
    several items to a single slot (most often if the user selects
    from an array). This signal is useful in such cases.

    \sa highlighted(), Q3MenuData::insertItem()
*/

/*!
    \fn void Q3PopupMenu::highlighted(int id)

    This signal is emitted when a menu item is highlighted; \a id is
    the id of the highlighted item.

    \sa activated(), Q3MenuData::insertItem()
*/

/*! \fn void Q3PopupMenu::highlightedRedirect(int id)
  \internal
  Used internally to connect submenus to their parents.
*/

/*! \fn void Q3PopupMenu::activatedRedirect(int id)
  \internal
  Used internally to connect submenus to their parents.
*/

void Q3PopupMenu::subActivated(int id)
{
    emit activatedRedirect(id);
}

void Q3PopupMenu::subHighlighted(int id)
{
    emit highlightedRedirect(id);
}

static bool fromAccel = false;

#ifndef QT_NO_ACCEL
void Q3PopupMenu::accelActivated(int id)
{
    Q3MenuItem *mi = findItem(id);
    if (mi && mi->isEnabledAndVisible()) {
        QPointer<QSignalEmitter> signal = mi->signal();
        int value = mi->signalValue();
        fromAccel = true;
        actSig(mi->id());
        fromAccel = false;
        if (signal)
            signal->activate(&value);
    }
}

void Q3PopupMenu::accelDestroyed()                // accel about to be deleted
{
    autoaccel = 0;                                // don't delete it twice!
}
#endif //QT_NO_ACCEL

void Q3PopupMenu::popupDestroyed(QObject *o)
{
    removePopup((Q3PopupMenu*)o);
}

void Q3PopupMenu::actSig(int id, bool inwhatsthis)
{
    if (!inwhatsthis) {
        emit activated(id);
#ifndef QT_NO_ACCESSIBILITY
        if (!fromAccel)
            QAccessible::updateAccessibility(this, indexOf(id)+1, QAccessible::MenuCommand);
#endif
    } else {
#ifndef QT_NO_WHATSTHIS
        QRect r(itemGeometry(indexOf(id)));
        QPoint p(r.center().x(), r.bottom());
        QHelpEvent e(QEvent::WhatsThis, p, mapToGlobal(p));
        QApplication::sendEvent(this, &e);
#endif
    }

    emit activatedRedirect(id);
}

void Q3PopupMenu::hilitSig(int id)
{
    emit highlighted(id);
    emit highlightedRedirect(id);

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, indexOf(id)+1, QAccessible::Focus);
    QAccessible::updateAccessibility(this, indexOf(id)+1, QAccessible::Selection);
#endif
}

void Q3PopupMenu::setFirstItemActive()
{
    int ai = 0;
    if(d->scroll.scrollable)
        ai = d->scroll.topScrollableIndex;
    for (int i = ai; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (!mi->isSeparator() && mi->id() != Q3MenuData::d->aInt &&
           (style().styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, this) || mi->isEnabledAndVisible())) {
            setActiveItem(ai);
            return;
        }
        ai++;
    }
    actItem = -1;
}

/*!
  \internal
  Hides all popup menus (in this menu tree) that are currently open.
*/

void Q3PopupMenu::hideAllPopups()
{
    register Q3MenuData *top = this;                // find top level popup
    if (!preventAnimation)
        QTimer::singleShot(10, this, SLOT(allowAnimation()));
    preventAnimation = true;

    if (!isPopup())
        return; // nothing to do

    while (top->parentMenu && top->parentMenu->isPopupMenu
            && ((Q3PopupMenu*)top->parentMenu)->isPopup())
        top = top->parentMenu;
    ((Q3PopupMenu*)top)->hide();                        // cascade from top level

}

/*!
  \internal
  Hides all popup sub-menus.
*/

void Q3PopupMenu::hidePopups()
{
    if (!preventAnimation)
        QTimer::singleShot(10, this, SLOT(allowAnimation()));
    preventAnimation = true;

    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (mi->popup() && mi->popup()->parentMenu == this) //avoid circularity
            mi->popup()->hide();
    }
    popupActive = -1;                                // no active sub menu
    if(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this))
        d->mouseMoveBuffer = QRegion();

    QRect mfrect = itemGeometry(actItem);
    setMicroFocusHint(mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), false);
}


/*!
  \internal
  Sends the event to the menu bar.
*/

bool Q3PopupMenu::tryMenuBar(QMouseEvent *e)
{
    register Q3MenuData *top = this;                // find top level
    while (top->parentMenu)
        top = top->parentMenu;
#ifndef QT_NO_MENUBAR
    return top->isMenuBar ?
        ((Q3MenuBar *)top)->tryMouseEvent(this, e) :
                              ((Q3PopupMenu*)top)->tryMouseEvent(this, e);
#else
    return ((Q3PopupMenu*)top)->tryMouseEvent(this, e);
#endif
}


/*!
  \internal
*/
bool Q3PopupMenu::tryMouseEvent(Q3PopupMenu *p, QMouseEvent * e)
{
    if (p == this)
        return false;
    QPoint pos = mapFromGlobal(e->globalPos());
    if (!rect().contains(pos))                // outside
        return false;
    QMouseEvent ee(e->type(), pos, e->globalPos(), e->button(), e->state());
    event(&ee);
    return true;
}

/*!
  \internal
  Tells the menu bar to go back to idle state.
*/

void Q3PopupMenu::byeMenuBar()
{
#ifndef QT_NO_MENUBAR
    register Q3MenuData *top = this;                // find top level
    while (top->parentMenu)
        top = top->parentMenu;
#endif
    hideAllPopups();
#ifndef QT_NO_MENUBAR
    if (top->isMenuBar)
        ((Q3MenuBar *)top)->goodbye();
#endif
}


/*!
  \internal
  Return the item at \a pos, or -1 if there is no item there or if
  it is a separator item.
*/

int Q3PopupMenu::itemAtPos(const QPoint &pos, bool ignoreSeparator) const
{
    if (!contentsRect().contains(pos))
        return -1;

    int row = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    if(d->scroll.scrollable) {
        if(d->scroll.topScrollableIndex) {
            if (d->scroll.topScrollableIndex < mitems->size())
                row = d->scroll.topScrollableIndex;
            y += style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
        }
    }
    int itemw = contentsRect().width() / ncols;
    QSize sz;
    Q3MenuItem *mi = 0;
    for (; row < mitems->size(); ++row) {
        mi = mitems->at(row);
        if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
           y >= contentsRect().height() - style().pixelMetric(QStyle::PM_MenuScrollerHeight, this))
            return -1;
        if (!mi->isVisible())
            continue;
        int itemh = itemHeight(mi);
        QStyleOptionMenuItem opt = getStyleOption(this, mi);
        opt.rect = rect();
        opt.maxIconWidth = maxPMWidth;
        opt.tabWidth = 0;
        sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt, QSize(0, itemh),
                                      fontMetrics(), this);
        sz = sz.expandedTo(QSize(itemw, sz.height()));
        itemw = sz.width();
        itemh = sz.height();

        if (ncols > 1 && y + itemh > contentsRect().bottom()) {
            y = contentsRect().y();
            x +=itemw;
        }
        if (QRect(x, y, itemw, itemh).contains(pos))
            break;
        y += itemh;
    }

    if (mi && (!ignoreSeparator || !mi->isSeparator()))
        return row;
    return -1;
}

/*!
  \internal
  Returns the geometry of item number \a index.
*/

QRect Q3PopupMenu::itemGeometry(int index)
{
    QSize sz;
    int row = 0, scrollh = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp) {
        scrollh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
        y += scrollh;
        if(d->scroll.topScrollableIndex < mitems->size())
            row = d->scroll.topScrollableIndex;
    }
    int itemw = contentsRect().width() / ncols;
    for (; row < mitems->size(); ++row) {
        Q3MenuItem *mi = mitems->at(row);
        if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
           y >= contentsRect().height() - scrollh)
            break;
        if (!mi->isVisible())
            continue;
        int itemh = itemHeight(mi);

        QStyleOptionMenuItem opt = getStyleOption(this, mi);
        opt.rect = rect();
        opt.maxIconWidth = maxPMWidth;
        opt.tabWidth = 0;
        sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt, QSize(0, itemh),
                                      fontMetrics(), this);
        sz = sz.expandedTo(QSize(itemw, sz.height()));
        itemw = sz.width();
        itemh = sz.height();
        if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
           (y + itemh > contentsRect().height() - scrollh))
            itemh -= (y + itemh) - (contentsRect().height() - scrollh);
        if (ncols > 1 && y + itemh > contentsRect().bottom()) {
            y = contentsRect().y();
            x +=itemw;
        }
        if (row == index)
            return QRect(x,y,itemw,itemh);
        y += itemh;
    }

    return QRect(0,0,0,0);
}


/*!
  \internal
  Calculates and sets the size of the popup menu, based on the size
  of the items.
*/

QSize Q3PopupMenu::updateSize(bool force_update, bool do_resize)
{
    ensurePolished();
    if (count() == 0) {
        QSize ret = QSize(50, 8);
        if(do_resize)
            setFixedSize(ret);
        badSize = true;
        return ret;
    }

    int scrheight = 0;
    if(d->scroll.scrollableSize) {
        if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp)
            scrheight += style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
        if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown)
            scrheight += style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
    }

    if(badSize || force_update) {
#ifndef QT_NO_ACCEL
        updateAccel(0);
#endif
        int height = 0;
        int max_width = 0, max_height = 0;
        QFontMetrics fm = fontMetrics();
        maxPMWidth = 0;
        int maxWidgetWidth = 0;
        tab = 0;

        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *mi = mitems->at(i);
            QWidget *miw = mi->widget();
            if (miw) {
                if (miw->parentWidget() != this) {
                    miw->setParent(this);
                    miw->show();
                }
                // widget items musn't propgate mouse events
                ((Q3PopupMenu*)miw)->setWFlags(Qt::WNoMousePropagation);
            }
            if (mi->custom())
                mi->custom()->setFont(font());
            if (mi->iconSet() != 0)
                maxPMWidth = qMax(maxPMWidth,
                                   mi->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal).width() + 4);
        }

        int dh = QApplication::desktop()->height();
        ncols = 1;
        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *mi = mitems->at(i);
            if (!mi->isVisible())
                continue;
            int w = 0;
            int itemHeight = Q3PopupMenu::itemHeight(mi);

            if (mi->widget()) {
                QSize s(mi->widget()->sizeHint());
                s = s.expandedTo(mi->widget()->minimumSize());
                mi->widget()->resize(s);
                if (s.width()  > maxWidgetWidth)
                    maxWidgetWidth = s.width();
                itemHeight = s.height();
            } else {
                if(! mi->isSeparator()) {
                    if (mi->custom()) {
                        if (mi->custom()->fullSpan()) {
                            maxWidgetWidth = qMax(maxWidgetWidth,
                                                   mi->custom()->sizeHint().width());
                        } else {
                            QSize s (mi->custom()->sizeHint());
                            w += s.width();
                        }
                    }

                    w += maxPMWidth;

                    if (! mi->text().isNull()) {
                        QString s = mi->text();
                        int t;
                        // ##### search for similar places and replace by regular fm.width(... , Qt::AlignLeft, ....)
                        if ((t = s.indexOf('\t')) >= 0) { // string contains tab
                            w += fm.width(s, t);
                            w -= s.count('&') * fm.width('&');
                            w += s.count("&&") * fm.width('&');
                            int tw = fm.width(s.mid(t + 1));
                            if (tw > tab)
                                tab = tw;
                        } else {
                            w += fm.width(s);
                            w -= s.count('&') * fm.width('&');
                            w += s.count("&&") * fm.width('&');
                        }
                    } else if (mi->pixmap())
                        w += mi->pixmap()->width();
                } else {
                    if (mi->custom()) {
                        QSize s (mi->custom()->sizeHint());
                        w += s.width();
                    } else {
                        w = itemHeight = 2;
                    }
                }

                QStyleOptionMenuItem opt = getStyleOption(this, mi);
                opt.rect = rect();
                opt.maxIconWidth = maxPMWidth;
                opt.tabWidth = 0;
                QSize sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt,
                                                    QSize(w, itemHeight), fontMetrics(), this);

                w = sz.width();
                itemHeight = sz.height();

                if (mi->text().isNull() && !mi->pixmap() && !mi->iconSet() &&
                     !mi->isSeparator() && !mi->widget() && !mi->custom())
                    qWarning("Q3PopupMenu: (%s) Popup has invalid menu item",
                              objectName().local8Bit());
            }
            height += itemHeight;
            if(style().styleHint(QStyle::SH_Menu_Scrollable, this)) {
                if(scrheight && height >= d->scroll.scrollableSize)
                    break;
            } else if(height + 2*frameWidth() >= dh) {
                ncols++;
                max_height = qMax(max_height, height - itemHeight);
                height = 0;
            }
            if (w > max_width)
                max_width = w;
        }
        if(ncols == 1 && !max_height)
            max_height = height;

        if(style().styleHint(QStyle::SH_Menu_Scrollable, this)) {
            height += scrheight;
            setMouseTracking(true);
        }

        if (tab)
            tab -= fontMetrics().minRightBearing();
        else
            max_width -= fontMetrics().minRightBearing();

        if (max_width + tab < maxWidgetWidth)
            max_width = maxWidgetWidth - tab;

        const int fw = frameWidth();
        int extra_width = (fw+style().pixelMetric(QStyle::PM_MenuHMargin, this)) * 2,
           extra_height = (fw+style().pixelMetric(QStyle::PM_MenuVMargin, this)) * 2;
        if (ncols == 1)
            d->calcSize = QSize(qMax(minimumWidth(), max_width + tab + extra_width),
                              qMax(minimumHeight() , height + extra_height));
        else
            d->calcSize = QSize(qMax(minimumWidth(), (ncols*(max_width + tab)) + extra_width),
                              qMax(minimumHeight(), qMin(max_height + extra_height + 1, dh)));
        badSize = false;
    }

    if(do_resize) {
        setMaximumSize(d->calcSize);
        resize(d->calcSize);

        bool hasWidgetItems = false;
        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *mi = mitems->at(i);
            if(mi->widget()) {
                hasWidgetItems = true;
                break;
            }
        }
        if (hasWidgetItems) {
            // Position the widget items. It could be done in drawContents
            // but this way we get less flicker.
            QSize sz;
            int x = contentsRect().x();
            int y = contentsRect().y();
            int itemw = contentsRect().width() / ncols;
            for (int i = 0; i < mitems->size(); ++i) {
                Q3MenuItem *mi = mitems->at(i);
                if (!mi->isVisible())
                    continue;

                int itemh = itemHeight(mi);

                QStyleOptionMenuItem opt = getStyleOption(this, mi);
                opt.rect = rect();
                opt.maxIconWidth = maxPMWidth;
                opt.tabWidth = 0;
                sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt, QSize(0, itemh),
                                              fontMetrics(), this);
                sz = sz.expandedTo(QSize(itemw, sz.height()));
                itemw = sz.width();
                itemh = sz.height();

                if (ncols > 1 && y + itemh > contentsRect().bottom()) {
                    y = contentsRect().y();
                    x +=itemw;
                }
                if (mi->widget())
                    mi->widget()->setGeometry(x, y, itemw, mi->widget()->height());
                y += itemh;
            }
        }
    }
    return d->calcSize;
}


#ifndef QT_NO_ACCEL
/*!
  \internal
  The \a parent is 0 when it is updated when a menu item has
  changed a state, or it is something else if called from the menu bar.
*/

void Q3PopupMenu::updateAccel(QWidget *parent)
{
    if (parent) {
        delete autoaccel;
        autoaccel = 0;
    } else if (!autoaccel) {
        // we have no parent. Rather than ignoring any accelerators we try to find this popup's main window
        if (tornOff) {
            parent = this;
        } else {
            QWidget *w = (QWidget *) this;
            parent = w->parentWidget();
            while ((!w->testWFlags(Qt::WType_TopLevel) || !w->testWFlags(Qt::WType_Popup)) && parent) {
                w = parent;
                parent = parent->parentWidget();
            }
        }
    }

    if (parent == 0 && autoaccel == 0)
        return;

    if (autoaccel)                                // build it from scratch
        autoaccel->clear();
    else {
        // create an autoaccel in any case, even if we might not use
        // it immediately. Maybe the user needs it later.
        autoaccel = new QAccel(parent, this);
        connect(autoaccel, SIGNAL(activated(int)),
                 SLOT(accelActivated(int)));
        connect(autoaccel, SIGNAL(activatedAmbiguously(int)),
                 SLOT(accelActivated(int)));
        connect(autoaccel, SIGNAL(destroyed()),
                 SLOT(accelDestroyed()));
        if (accelDisabled)
            autoaccel->setEnabled(false);
    }
    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        QKeySequence k = mi->key();
        if ((int)k) {
            int id = autoaccel->insertItem(k, mi->id());
#ifndef QT_NO_WHATSTHIS
            autoaccel->setWhatsThis(id, mi->whatsThis());
#endif
        }
        if (!mi->text().isNull() || mi->custom()) {
            QString s = mi->text();
            int i = s.indexOf('\t');

            // Note: Only looking at the first key in the sequence!
            if ((int)k && (int)k != Qt::Key_unknown) {
                QString t = (QString)mi->key();
                if (i >= 0)
                    s.replace(i+1, s.length()-i, t);
                else {
                    s += '\t';
                    s += t;
                }
            } else if (!k) {
                if (i >= 0)
                    s.truncate(i);
            }
            if (s != mi->text()) {
                mi->setText(s);
                badSize = true;
            }
        }
        if (mi->popup() && parent) {                // call recursively
            // reuse
            Q3PopupMenu* popup = mi->popup();
            if (!popup->avoid_circularity) {
                popup->avoid_circularity = 1;
                popup->updateAccel(parent);
                popup->avoid_circularity = 0;
            }
        }
    }
}

/*!
  \internal
  It would be better to check in the slot.
*/

void Q3PopupMenu::enableAccel(bool enable)
{
    if (autoaccel)
        autoaccel->setEnabled(enable);
    accelDisabled = !enable;                // rememeber when updateAccel
    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (mi->popup())                        // call recursively
            mi->popup()->enableAccel(enable);
    }
}
#endif

/*!
    \reimp
*/
void Q3PopupMenu::show()
{
    if (!isPopup() && isVisible())
        hide();

    if (isVisible()) {
        supressAboutToShow = false;
        QWidget::show();
        return;
    }
    if (!supressAboutToShow)
        emit aboutToShow();
    else
        supressAboutToShow = false;
    performDelayedChanges();
    updateSize(true);
    QWidget::show();
    popupActive = -1;
    if(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this))
        d->mouseMoveBuffer = QRegion();
}

/*!
    \reimp
*/

void Q3PopupMenu::hide()
{
    if (syncMenu == this && qApp) {
        qApp->exit_loop();
        syncMenu = 0;
    }

    if (!isVisible()) {
        QWidget::hide();
        return;
    }
    emit aboutToHide();

    actItem = popupActive = -1;
    if(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this))
        d->mouseMoveBuffer = QRegion();
    mouseBtDn = false;                                // mouse button up
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuEnd);
#endif
    parentMenu = 0;
    hidePopups();
    QWidget::hide();
}


/*!
    Calculates the height in pixels of the item in row \a row.
*/
int Q3PopupMenu::itemHeight(int row) const
{
    return itemHeight(mitems->at(row));
}

/*!
    \overload

    Calculates the height in pixels of the menu item \a mi.
*/
int Q3PopupMenu::itemHeight(Q3MenuItem *mi) const
{
    if  (mi->widget())
        return mi->widget()->height();
    if (mi->custom() && mi->custom()->fullSpan())
        return mi->custom()->sizeHint().height();

    QFontMetrics fm(fontMetrics());
    int h = 0;
    if (mi->isSeparator()) // separator height
        h = 2;
    else if (mi->pixmap()) // pixmap height
        h = mi->pixmap()->height();
    else                     // text height
        h = fm.height();

    if (!mi->isSeparator() && mi->iconSet() != 0)
        h = qMax(h, mi->iconSet()->pixmap(QIconSet::Small,
                                           QIconSet::Normal).height());
    if (mi->custom())
        h = qMax(h, mi->custom()->sizeHint().height());

    return h;
}


/*!
    Draws menu item \a mi in the area \a x, \a y, \a w, \a h, using
    painter \a p. The item is drawn active if \a act is true or drawn
    inactive if \a act is false. The rightmost \a tab_ pixels are used
    for accelerator text.

    \sa QStyle::drawControl()
*/
void Q3PopupMenu::drawItem(QPainter* p, int tab_, Q3MenuItem* mi,
                           bool act, int x, int y, int w, int h)
{
    // This call is a waste, but we want people to not use this class, right :)
    QStyleOptionMenuItem menuOpt = getStyleOption(this, mi);
    menuOpt.state = QStyle::Style_Default;
    menuOpt.maxIconWidth = maxPMWidth;
    menuOpt.tabWidth = tab_;
    if (isEnabled() && mi->isEnabledAndVisible() && (!mi->popup() || mi->popup()->isEnabled()))
        menuOpt.state |= QStyle::Style_Enabled;
    if (act)
        menuOpt.state |= QStyle::Style_Active;
    if (mouseBtDn)
        menuOpt.state |= QStyle::Style_Down;

    if (!(menuOpt.state & QStyle::Style_Enabled))
        menuOpt.palette.setCurrentColorGroup(QPalette::Disabled);

    menuOpt.rect.setRect(x, y, w, h);
    if (mi->custom() && mi->custom()->fullSpan()) {
        // the custom item will paint it all, so just do the background.
        menuOpt.menuItemType = QStyleOptionMenuItem::Normal;
        menuOpt.text = "";
        menuOpt.icon = QIconSet();
        style().drawControl(QStyle::CE_MenuItem, &menuOpt, p, this);
        mi->custom()->paint(p, palette(), act, menuOpt.state & QStyle::Style_Enabled, x, y, w, h);
    } else {
        style().drawControl(QStyle::CE_MenuItem, &menuOpt, p, this);
    }
}

/*!
    Draws all menu items using painter \a p.
*/
void Q3PopupMenu::drawContents(QPainter* p)
{
    int row = 0;
    int x = contentsRect().x();
    int y = contentsRect().y();
    QStyleOptionMenuItem opt(0);
    opt.menuRect = rect();
    opt.state = QStyle::Style_Default;
    opt.palette = palette();
    opt.checkState = QStyleOptionMenuItem::NotCheckable;
    opt.menuItemType = QStyleOptionMenuItem::Scroller;
    opt.tabWidth = 0;
    opt.maxIconWidth = maxPMWidth;
    if (d->scroll.scrollable) {
        if (d->scroll.topScrollableIndex < mitems->size())
            row = d->scroll.topScrollableIndex;

        if (d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp) {
            opt.rect.setRect(x, y, contentsRect().width(),
                             style().pixelMetric(QStyle::PM_MenuScrollerHeight, this));
            if (!p->hasClipping() || p->clipRegion().contains(opt.rect)) {
                opt.state = QStyle::Style_Up;
                if (isEnabled())
                    opt.state |= QStyle::Style_Enabled;
                style().drawControl(QStyle::CE_MenuScroller, &opt, p, this);
            }
            y += opt.rect.height();
        }
    }

    int itemw = contentsRect().width() / ncols;
    QSize sz;
    for (; row < mitems->size(); ++row) {
        Q3MenuItem *mi = mitems->at(row);
        if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
           y >= contentsRect().height() - style().pixelMetric(QStyle::PM_MenuScrollerHeight, this))
            break;
        if (!mi->isVisible())
            continue;
        QStyleOptionMenuItem menuOpt = getStyleOption(this, mi);
        menuOpt.tabWidth = 0;
        menuOpt.maxIconWidth = maxPMWidth;
        int itemh = itemHeight(mi);
        sz = style().sizeFromContents(QStyle::CT_MenuItem, &menuOpt,
                                      QSize(0, itemh), fontMetrics(), this);
        sz = sz.expandedTo(QSize(itemw, sz.height()));
        itemw = sz.width();
        itemh = sz.height();

        if (ncols > 1 && y + itemh > contentsRect().bottom()) {
            if (y < contentsRect().bottom()) {
                menuOpt.rect.setRect(x, y, itemw, contentsRect().bottom() - y);
                if(!p->hasClipping() || p->clipRegion().contains(menuOpt.rect)) {
                    menuOpt.state = QStyle::Style_Default;
                    if (isEnabled() && mi->isEnabledAndVisible())
                        menuOpt.state |= QStyle::Style_Enabled;
                    style().drawControl(QStyle::CE_MenuItem, &menuOpt, p, this);
                }
            }
            y = contentsRect().y();
            x +=itemw;
        }
        if (!mi->widget() && (!p->hasClipping() || p->clipRegion().contains(QRect(x, y, itemw, itemh))))
            drawItem(p, tab, mi, row == actItem, x, y, itemw, itemh);
        y += itemh;
    }
    if (y < contentsRect().bottom()) {
        opt.rect.setRect(x, y, itemw, contentsRect().bottom() - y);
        if(!p->hasClipping() || p->clipRegion().contains(opt.rect)) {
            opt.state = QStyle::Style_Default;
            if (isEnabled())
                opt.state |= QStyle::Style_Enabled;
            opt.icon = QIconSet();
            opt.text = "";
            opt.menuItemType = QStyleOptionMenuItem::Normal;
            style().drawControl(QStyle::CE_MenuItem, &opt, p, this);
        }
    }
    if (d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown) {
        int sh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
        opt.icon = QIconSet();
        opt.text = "";
        opt.menuItemType = QStyleOptionMenuItem::Scroller;
        opt.rect.setRect(x, contentsRect().height() - sh, contentsRect().width(), sh);
        if (!p->hasClipping() || p->clipRegion().contains(opt.rect)) {
            opt.state = QStyle::Style_Down;
            if (isEnabled())
                opt.state |= QStyle::Style_Enabled;
            style().drawControl(QStyle::CE_MenuScroller, &opt, p, this);
        }
    }
#if defined(DEBUG_SLOPPY_SUBMENU)
    if (style().styleHint(QStyle::SH_Menu_SloppySubMenus, this)) {
        p->setClipRegion(d->mouseMoveBuffer);
        p->fillRect(d->mouseMoveBuffer.boundingRect(), palette().brush(QPalette::Highlight));
    }
#endif
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!
    \reimp
*/

void Q3PopupMenu::paintEvent(QPaintEvent *e)
{
    Q3Frame::paintEvent(e);
}

/*!
    \reimp
*/
bool Q3PopupMenu::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WhatsThis: {
        Q3MenuItem *mi = mitems->value(itemAtPos(static_cast<QHelpEvent *>(e)->pos()), 0);
        if (mi && mi->whatsThis().size()) {
            QWhatsThis::showText(static_cast<QHelpEvent*>(e)->globalPos(),
                                 mi->whatsThis(), this);
            return true;
        }
    break; }
#ifndef QT_NO_ACCESSIBILITY
    case QEvent::AccessibilityHelp: {
        QAccessibleEvent *ev = static_cast<QAccessibleEvent *>(e);
        if (ev->child() > 0 && ev->textType() == QAccessibleEvent::Help) {
            ev->setValue(whatsThis(idAt(ev->child())));
            return true;
        }
    break; }
#endif
    default:
        break;
    }
    return Q3Frame::event(e);
}

/*!
    \reimp
*/
void Q3PopupMenu::closeEvent(QCloseEvent * e) {
    e->accept();
    byeMenuBar();
}

/*!
    \reimp
*/

void Q3PopupMenu::mousePressEvent(QMouseEvent *e)
{
    int sh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
    if (rect().contains(e->pos()) &&
        ((d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp && e->pos().y() <= sh) || //up
         (d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
             e->pos().y() >= contentsRect().height() - sh))) //down
        return;

    mouseBtDn = true;                                // mouse button down
    int item = itemAtPos(e->pos());
    if (item == -1) {
        if (!rect().contains(e->pos()) && !tryMenuBar(e)) {
            byeMenuBar();
        }
        return;
    }
    register Q3MenuItem *mi = mitems->at(item);
    if (item != actItem)                        // new item activated
        setActiveItem(item);

    Q3PopupMenu *popup = mi->popup();
    if (popup) {
        if (popup->isVisible()) {                // sub menu already open
            int pactItem = popup->actItem;
            popup->actItem = -1;
            popup->hidePopups();
            popup->updateRow(pactItem);
        } else {                                // open sub menu
            hidePopups();
            popupSubMenuLater(20, this);
        }
    } else {
        hidePopups();
    }
}

/*!
    \reimp
*/

void Q3PopupMenu::mouseReleaseEvent(QMouseEvent *e)
{
    // do not hide a standalone context menu on press-release, unless
    // the user moved the mouse significantly
    if (!parentMenu && !mouseBtDn && actItem < 0 && motion < 6)
        return;

    mouseBtDn = false;

    // if the user released the mouse outside the menu, pass control
    // to the menubar or our parent menu
    int sh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
    if (!rect().contains(e->pos()) && tryMenuBar(e))
        return;
    else if((d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp && e->pos().y() <= sh) || //up
            (d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
             e->pos().y() >= contentsRect().height() - sh)) //down
        return;

    if (actItem < 0) { // we do not have an active item
        // if the release is inside without motion (happens with
        // oversized popup menus on small screens), ignore it
        if (rect().contains(e->pos()) && motion < 6)
            return;
        else
            byeMenuBar();
    } else {        // selected menu item!
        register Q3MenuItem *mi = mitems->at(actItem);
        if (mi ->widget()) {
            QWidget* widgetAt = QApplication::widgetAt(e->globalPos());
            if (widgetAt && widgetAt != this) {
                QMouseEvent me(e->type(), widgetAt->mapFromGlobal(e->globalPos()),
                                e->globalPos(), e->button(), e->state());
                QApplication::sendEvent(widgetAt, &me);
            }
        }
        Q3PopupMenu *popup = mi->popup();
#ifndef QT_NO_WHATSTHIS
            bool b = QWhatsThis::inWhatsThisMode();
#else
            const bool b = false;
#endif
        if (!mi->isEnabledAndVisible()) {
#ifndef QT_NO_WHATSTHIS
            if (b) {
                actItem = -1;
                updateItem(mi->id());
                byeMenuBar();
                actSig(mi->id(), b);
            }
#endif
        } else         if (popup) {
            popup->setFirstItemActive();
        } else {                                // normal menu item
            byeMenuBar();                        // deactivate menu bar
            if (mi->isEnabledAndVisible()) {
                actItem = -1;
                updateItem(mi->id());
                active_popup_menu = this;
                QPointer<QSignalEmitter> signal = mi->signal();
                int value = mi->signalValue();
                actSig(mi->id(), b);
                if (signal && !b)
                    signal->activate(&value);
                active_popup_menu = 0;
            }
        }
    }
}

/*!
    \reimp
*/

void Q3PopupMenu::mouseMoveEvent(QMouseEvent *e)
{
    motion++;

    if (parentMenu && parentMenu->isPopupMenu) {
        Q3PopupMenu* p = (Q3PopupMenu*)parentMenu;
        int myIndex;

        p->findPopup(this, &myIndex);
        QPoint pPos = p->mapFromParent(e->globalPos());
        if (p->actItem != myIndex && !p->rect().contains(pPos))
            p->setActiveItem(myIndex);

        if (style().styleHint(QStyle::SH_Menu_SloppySubMenus, this)) {
            p->d->mouseMoveBuffer = QRegion();
#ifdef DEBUG_SLOPPY_SUBMENU
            p->repaint();
#endif
        }
    }

    if ((e->state() & Qt::MouseButtonMask) == 0 &&
         !hasMouseTracking())
        return;

    int         item = itemAtPos(e->pos());
    if (item == -1) {                                // no valid item
        int lastActItem = actItem;
        actItem = -1;
        if (lastActItem >= 0)
            updateRow(lastActItem);
        if(d->scroll.scrollable &&
           e->pos().x() >= rect().x() && e->pos().x() <= rect().width()) {
            if(!d->scroll.scrolltimer) {
                d->scroll.scrolltimer = new QTimer(this, "popup scroll timer");
                QObject::connect(d->scroll.scrolltimer, SIGNAL(timeout()),
                                  this, SLOT(subScrollTimer()));
            }
            if(!d->scroll.scrolltimer->isActive())
                d->scroll.scrolltimer->start(40);
        } else if (lastActItem > 0 ||
                    (!rect().contains(e->pos()) && !tryMenuBar(e))) {
            popupSubMenuLater(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay,
                                                this), this);
        }
    } else {                                        // mouse on valid item
        // but did not register mouse press
        if ((e->state() & Qt::MouseButtonMask) && !mouseBtDn)
            mouseBtDn = true; // so mouseReleaseEvent will pop down

        register Q3MenuItem *mi = mitems->at(item);

        if (mi->widget()) {
            QWidget* widgetAt = QApplication::widgetAt(e->globalPos());
            if (widgetAt && widgetAt != this) {
                QMouseEvent me(e->type(), widgetAt->mapFromGlobal(e->globalPos()),
                                e->globalPos(), e->button(), e->state());
                QApplication::sendEvent(widgetAt, &me);
            }
        }

        if (actItem == item)
            return;

        if (style().styleHint(QStyle::SH_Menu_SloppySubMenus, this) &&
             d->mouseMoveBuffer.contains(e->pos())) {
            actItem = item;
            popupSubMenuLater(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this) * 6,
                               this);
            return;
        }

        if (mi->popup() || (popupActive >= 0 && popupActive != item))
            popupSubMenuLater(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this),
                               this);
        else if (singleSingleShot)
            singleSingleShot->stop();

        if (item != actItem)
            setActiveItem(item);
    }
}


/*!
    \reimp
*/

void Q3PopupMenu::keyPressEvent(QKeyEvent *e)
{
    /*
      I get nothing but complaints about this.  -Brad

      - if (mouseBtDn && actItem >= 0) {
      -        if (e->key() == Qt::Key_Shift ||
      -            e->key() == Qt::Key_Control ||
      -            e->key() == Qt::Key_Alt)
      -            return;
      -
      -        Q3MenuItem *mi = mitems->at(actItem);
      -        int modifier = (((e->state() & Qt::ShiftButton) ? Qt::SHIFT : 0) |
      -                        ((e->state() & Qt::ControlButton) ? Qt::CTRL : 0) |
      -                        ((e->state() & Qt::AltButton) ? Qt::ALT : 0));
      -
      - #ifndef QT_NO_ACCEL
      -        if (mi)
      -            setAccel(modifier + e->key(), mi->id());
      - #endif
      - return;
      - }
    */

    Q3MenuItem  *mi = 0;
    Q3PopupMenu *popup;
    int dy = 0;
    bool ok_key = true;

    int key = e->key();
    if (QApplication::reverseLayout()) {
        // in reverse mode opening and closing keys for submenues are reversed
        if (key == Qt::Key_Left)
            key = Qt::Key_Right;
        else if (key == Qt::Key_Right)
            key = Qt::Key_Left;
    }

    switch (key) {
    case Qt::Key_Tab:
        // ignore tab, otherwise it will be passed to the menubar
        break;

    case Qt::Key_Up:
        dy = -1;
        break;

    case Qt::Key_Down:
        dy = 1;
        break;

    case Qt::Key_Alt:
        if (style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this))
            byeMenuBar();
        break;

    case Qt::Key_Escape:
        if (tornOff) {
            close();
            return;
        }
        // just hide one
        {
            Q3MenuData* p = parentMenu;
            hide();
#ifndef QT_NO_MENUBAR
            if (p && p->isMenuBar)
                ((Q3MenuBar*) p)->goodbye(true);
#endif
        }
        break;

    case Qt::Key_Left:
        if (ncols > 1 && actItem >= 0) {
            QRect r(itemGeometry(actItem));
            int newActItem = itemAtPos(QPoint(r.left() - 1, r.center().y()));
            if (newActItem >= 0) {
                setActiveItem(newActItem);
                break;
            }
        }
        if (parentMenu && parentMenu->isPopupMenu) {
            ((Q3PopupMenu *)parentMenu)->hidePopups();
            if (singleSingleShot)
                singleSingleShot->stop();
            break;
        }

        ok_key = false;
        break;

    case Qt::Key_Right:
        if (actItem >= 0 && (mi=mitems->at(actItem))->isEnabledAndVisible() && (popup=mi->popup())) {
            hidePopups();
            if (singleSingleShot)
                singleSingleShot->stop();
            // ### The next two lines were switched to fix the problem with the first item of the
            // submenu not being highlighted...any reason why they should have been the other way??
            subMenuTimer();
            popup->setFirstItemActive();
            break;
        } else if (actItem == -1 && (parentMenu && !parentMenu->isMenuBar)) {
            dy = 1;
            break;
        }
        if (ncols > 1 && actItem >= 0) {
            QRect r(itemGeometry(actItem));
            int newActItem = itemAtPos(QPoint(r.right() + 1, r.center().y()));
            if (newActItem >= 0) {
                setActiveItem(newActItem);
                break;
            }
        }
        ok_key = false;
        break;

    case Qt::Key_Space:
        if (! style().styleHint(QStyle::SH_Menu_SpaceActivatesItem, this))
            break;
        // for motif, fall through

    case Qt::Key_Return:
    case Qt::Key_Enter:
        {
            if (actItem < 0)
                break;
#ifndef QT_NO_WHATSTHIS
            bool b = QWhatsThis::inWhatsThisMode();
#else
            const bool b = false;
#endif
            mi = mitems->at(actItem);
            if (!mi->isEnabled() && !b)
                break;
            popup = mi->popup();
            if (popup) {
                hidePopups();
                popupSubMenuLater(20, this);
                popup->setFirstItemActive();
            } else {
                actItem = -1;
                updateItem(mi->id());
                byeMenuBar();
                if (mi->isEnabledAndVisible() || b) {
                    active_popup_menu = this;
                    QPointer<QSignalEmitter> signal = mi->signal();
                    int value = mi->signalValue();
                    actSig(mi->id(), b);
                    if (signal && !b)
                        signal->activate(&value);
                    active_popup_menu = 0;
                }
            }
        }
        break;
#ifndef QT_NO_WHATSTHIS
    case Qt::Key_F1:
        if (actItem < 0 || e->state() != Qt::ShiftButton)
            break;
        {
            qDebug("send event");
            QRect r(itemGeometry(actItem));
            QPoint p(r.center().x(), r.bottom());
            QHelpEvent e(QEvent::WhatsThis, p, mapToGlobal(p));
            QApplication::sendEvent(this, &e);
        }
        //fall-through!
#endif
    default:
        ok_key = false;

    }
    if (!ok_key &&
         (!e->state() || e->state() == Qt::AltButton || e->state() == Qt::ShiftButton) &&
         e->text().length()==1) {
        QChar c = e->text()[0].toUpper();

        Q3MenuItem* first = 0;
        Q3MenuItem* currentSelected = 0;
        Q3MenuItem* firstAfterCurrent = 0;

        mi = 0;
        int indx = 0;
        int clashCount = 0;
        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *m = mitems->at(i);
            QString s = m->text();
            if (!s.isEmpty()) {
                int i = s.indexOf('&');
                while (i >= 0 && i < (int)s.length() - 1) {
                    if (s[i+1].toUpper() == c) {
                        ok_key = true;
                        clashCount++;
                        if (!first)
                            first = m;
                        if (indx == actItem)
                            currentSelected = m;
                        else if (!firstAfterCurrent && currentSelected)
                            firstAfterCurrent = m;
                        break;
                    } else if (s[i+1] == '&') {
                        i = s.indexOf('&', i+2);
                    } else {
                        break;
                    }
                }
            }
            indx++;
        }

        if (1 == clashCount) { // No clashes, continue with selection
            mi = first;
            popup = mi->popup();
            if (popup) {
                setActiveItem(indexOf(mi->id()));
                hidePopups();
                popupSubMenuLater(20, this);
                popup->setFirstItemActive();
            } else {
                byeMenuBar();
#ifndef QT_NO_WHATSTHIS
                bool b = QWhatsThis::inWhatsThisMode();
#else
                const bool b = false;
#endif
                if (mi->isEnabledAndVisible() || b) {
                    active_popup_menu = this;
                    QPointer<QSignalEmitter> signal = mi->signal();
                    int value = mi->signalValue();
                    actSig(mi->id(), b);
                    if (signal && !b )
                        signal->activate(&value);
                    active_popup_menu = 0;
                }
            }
        } else if (clashCount > 1) { // Clashes, highlight next...
            // If there's clashes and no one is selected, use first one
            // or if there is no clashes _after_ current, use first one
            if (!currentSelected || (currentSelected && !firstAfterCurrent))
                dy = indexOf(first->id()) - actItem;
            else
                dy = indexOf(firstAfterCurrent->id()) - actItem;
        }
    }
#ifndef QT_NO_MENUBAR
    if (!ok_key) {                                // send to menu bar
        register Q3MenuData *top = this;                // find top level
        while (top->parentMenu)
            top = top->parentMenu;
        if (top->isMenuBar) {
            int beforeId = top->actItem;
            ((Q3MenuBar*)top)->tryKeyEvent(this, e);
            if (beforeId != top->actItem)
                ok_key = true;
        }
    }
#endif
    if (actItem < 0) {
        if (dy > 0) {
            setFirstItemActive();
        } else if (dy < 0) {
            for (int i = mitems->size()-1; i >= 0; --i) {
                Q3MenuItem *mi = mitems->at(i);
                if (!mi->isSeparator() && mi->id() != Q3MenuData::d->aInt) {
                    setActiveItem(i);
                    return;
                }
            }
            actItem = -1;
        }
        return;
    }

    if (dy) {                                // highlight next/prev
        register int i = actItem;
        int c = mitems->count();
        for(int n = c; n; n--) {
            i = i + dy;
            if(d->scroll.scrollable) {
                if(d->scroll.scrolltimer)
                    d->scroll.scrolltimer->stop();
                if(i < 0)
                    i = 0;
                else if(i >= c)
                    i  = c - 1;
            } else {
                if (i == c)
                    i = 0;
                else if (i < 0)
                    i = c - 1;
            }
            mi = mitems->at(i);
            if (!mi || !mi->isVisible())
                continue;

            if (!mi->isSeparator() &&
                 (style().styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, this)
                   || mi->isEnabledAndVisible()))
                break;
        }
        if (i != actItem)
            setActiveItem(i);
        if(d->scroll.scrollable) { //need to scroll to make it visible?
            QRect r = itemGeometry(actItem);
            if(r.isNull() || r.height() < itemHeight(mitems->at(actItem))) {
                bool refresh = false;
                if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp && dy == -1) { //up
                    if(d->scroll.topScrollableIndex >= 0) {
                        d->scroll.topScrollableIndex--;
                        refresh = true;
                    }
                } else if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown) { //down
                    int sh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
                    int y = (d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp) ? sh : 0;
                    for (int i = 0; i < mitems->size(); ++i) {
                        Q3MenuItem *mi = mitems->at(i);
                        if(i >= d->scroll.topScrollableIndex) {
                            int itemh = itemHeight(mi);
                            QStyleOptionMenuItem opt(0);
                            opt.palette = palette();
                            opt.rect = rect();
                            opt.menuRect = rect();
                            opt.state = QStyle::Style_Default;
                            opt.checkState = isCheckable() ? QStyleOptionMenuItem::Unchecked
                                                           : QStyleOptionMenuItem::NotCheckable;
                            opt.menuItemType = QStyleOptionMenuItem::Normal;
                            opt.text = mi->text();
                            opt.tabWidth = 0;
                            opt.maxIconWidth = maxPMWidth;
                            QSize sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt,
                                                                QSize(0, itemh), fontMetrics(),
                                                                this);
                            y += sz.height();
                            if(y > (contentsRect().height()-sh)) {
                                if(sz.height() > sh || i < mitems->size()-1)
                                    d->scroll.topScrollableIndex++;
                                refresh = true;
                                break;
                            }
                        }
                    }
                }
                if(refresh) {
                    updateScrollerState();
                    update();
                }
            }
        }
    }

#ifdef Q_OS_WIN32
    if (!ok_key &&
        !(e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift || e->key() == Qt::Key_Meta))
        qApp->beep();
#endif // Q_OS_WIN32
}


/*!
    \reimp
*/

void Q3PopupMenu::timerEvent(QTimerEvent *e)
{
    Q3Frame::timerEvent(e);
}

/*!
    \reimp
*/
void Q3PopupMenu::leaveEvent(QEvent *)
{
    if (testWFlags(Qt::WStyle_Tool) && style().styleHint(QStyle::SH_Menu_MouseTracking, this)) {
        int lastActItem = actItem;
        actItem = -1;
        if (lastActItem >= 0)
            updateRow(lastActItem);
    }
}

/*!
    \reimp
*/
void Q3PopupMenu::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        setMouseTracking(style().styleHint(QStyle::SH_Menu_MouseTracking, this));
        updateSize(true);
    } else if(ev->type() == QEvent::EnabledChange) {
        if (Q3MenuData::d->aPopup) // torn-off menu
            Q3MenuData::d->aPopup->setEnabled(isEnabled());
    }
    Q3Frame::changeEvent(ev);

    if (ev->type() == QEvent::FontChange) {
        badSize = true;
        if (isVisible()) {
            updateSize();
            update();
        }
    }
}


/*!
    If a popup menu does not fit on the screen it lays itself out so
    that it does fit. It is style dependent what layout means (for
    example, on Windows it will use multiple columns).

    This functions returns the number of columns necessary.
*/
int Q3PopupMenu::columns() const
{
    return ncols;
}

/* This private slot handles the scrolling popupmenu */
void Q3PopupMenu::subScrollTimer() {
    QPoint pos = QCursor::pos();
    if(!d->scroll.scrollable || !isVisible()) {
        if(d->scroll.scrolltimer)
            d->scroll.scrolltimer->stop();
        return;
    } else if(pos.x() > x() + width() || pos.x() < x()) {
        return;
    }
    int sh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
    if(!d->scroll.lastScroll.isValid()) {
        d->scroll.lastScroll = QTime::currentTime();
    } else {
        int factor=0;
        if(pos.y() < y())
            factor = y() - pos.y();
        else if(pos.y() > y() + height())
            factor = pos.y() - (y() + height());
        int msecs = 250 - ((factor / 10) * 40);
        if(d->scroll.lastScroll.msecsTo(QTime::currentTime()) < qMax(0, msecs))
            return;
        d->scroll.lastScroll = QTime::currentTime();
    }
    if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp && pos.y() <= y() + sh) { //up
        if(d->scroll.topScrollableIndex > 0) {
            d->scroll.topScrollableIndex--;
            updateScrollerState();
            update(contentsRect());
        }
    } else if(d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollDown &&
              pos.y() >= (y() + contentsRect().height()) - sh) { //down
        int y = contentsRect().y() + sh;
        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *mi = mitems->at(i);
            if(i >= d->scroll.topScrollableIndex) {
                int itemh = itemHeight(mi);
                QStyleOptionMenuItem opt = getStyleOption(this, mi);
                opt.tabWidth = 0;
                opt.maxIconWidth = maxPMWidth;
                QSize sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt, QSize(0, itemh),
                                                    fontMetrics(), this);
                y += sz.height();
                if(y > contentsRect().height() - sh) {
                    d->scroll.topScrollableIndex++;
                    updateScrollerState();
                    update(contentsRect());
                    break;
                }
            }
        }
    }
}

/* This private slot handles the delayed submenu effects */

void Q3PopupMenu::subMenuTimer() {

    if (!isVisible() || (actItem < 0 && popupActive < 0) || actItem == popupActive)
        return;

    if (popupActive >= 0) {
        hidePopups();
        popupActive = -1;
    }

    // hidePopups() may change actItem etc.
    if (!isVisible() || actItem < 0 || actItem == popupActive)
        return;

    Q3MenuItem *mi = mitems->at(actItem);
    if (!mi || !mi->isEnabledAndVisible())
        return;

    Q3PopupMenu *popup = mi->popup();
    if (!popup || !popup->isEnabled())
        return;

    //avoid circularity
    if (popup->isVisible())
        return;

    Q_ASSERT(popup->parentMenu == 0);
    popup->parentMenu = this;                        // set parent menu

    emit popup->aboutToShow();
    supressAboutToShow = true;


    QRect r(itemGeometry(actItem));
    QPoint p;
    QSize ps = popup->sizeHint();
    if(QApplication::reverseLayout()) {
        p = QPoint(r.left() + motifArrowHMargin - ps.width(), r.top() + motifArrowVMargin);
        p = mapToGlobal(p);

        bool right = false;
        if ((parentMenu && parentMenu->isPopupMenu &&
               ((Q3PopupMenu*)parentMenu)->geometry().x() < geometry().x()) ||
             p.x() < 0)
            right = true;
        if (right && (ps.width() > QApplication::desktop()->width() - mapToGlobal(r.topRight()).x()))
            right = false;
        if (right)
            p.setX(mapToGlobal(r.topRight()).x());
    } else {
        p = QPoint(r.right() - motifArrowHMargin, r.top() + motifArrowVMargin);
        p = mapToGlobal(p);

        bool left = false;
        if ((parentMenu && parentMenu->isPopupMenu &&
               ((Q3PopupMenu*)parentMenu)->geometry().x() > geometry().x()) ||
             p.x() + ps.width() > QApplication::desktop()->width())
            left = true;
        if (left && (ps.width() > mapToGlobal(r.topLeft()).x()))
            left = false;
        if (left)
            p.setX(mapToGlobal(r.topLeft()).x() - ps.width());
    }
    QRect pr = popup->itemGeometry(popup->count() - 1);
    if (p.y() + ps.height() > QApplication::desktop()->height() &&
        p.y() - ps.height() + (QCOORD) pr.height() >= 0)
        p.setY(p.y() - ps.height() + (QCOORD) pr.height());

    if (style().styleHint(QStyle::SH_Menu_SloppySubMenus, this)) {
         QPoint cur = QCursor::pos();
         if (r.contains(mapFromGlobal(cur))) {
             QPoint pts[4];
             pts[0] = QPoint(cur.x(), cur.y() - 2);
             pts[3] = QPoint(cur.x(), cur.y() + 2);
             if (p.x() >= cur.x())        {
                 pts[1] = QPoint(geometry().right(), p.y());
                 pts[2] = QPoint(geometry().right(), p.y() + ps.height());
             } else {
                 pts[1] = QPoint(p.x() + ps.width(), p.y());
                 pts[2] = QPoint(p.x() + ps.width(), p.y() + ps.height());
             }
             QPointArray points(4);
             for(int i = 0; i < 4; i++)
                 points.setPoint(i, mapFromGlobal(pts[i]));
             d->mouseMoveBuffer = QRegion(points);
             repaint();
         }
    }

    popupActive = actItem;
    popup->popup(p);
}

void Q3PopupMenu::allowAnimation()
{
    preventAnimation = false;
}

void Q3PopupMenu::updateRow(int row)
{
    if (!isVisible())
        return;

    if (badSize) {
        updateSize();
        update();
        return;
    }
    updateSize();
    QRect r = itemGeometry(row);
    if (!r.isNull()) // can happen via the scroller
        repaint(r);
}


/*!
    \overload

    Executes this popup synchronously.

    Opens the popup menu so that the item number \a indexAtPoint will
    be at the specified \e global position \a pos. To translate a
    widget's local coordinates into global coordinates, use
    QWidget::mapToGlobal().

    The return code is the id of the selected item in either the popup
    menu or one of its submenus, or -1 if no item is selected
    (normally because the user pressed Esc).

    Note that all signals are emitted as usual. If you connect a menu
    item to a slot and call the menu's exec(), you get the result both
    via the signal-slot connection and in the return value of exec().

    Common usage is to position the popup at the current mouse
    position:
    \code
        exec(QCursor::pos());
    \endcode
    or aligned to a widget:
    \code
        exec(somewidget.mapToGlobal(QPoint(0, 0)));
    \endcode

    When positioning a popup with exec() or popup(), bear in mind that
    you cannot rely on the popup menu's current size(). For
    performance reasons, the popup adapts its size only when
    necessary. So in many cases, the size before and after the show is
    different. Instead, use sizeHint(). It calculates the proper size
    depending on the menu's current contents.

    \sa popup()
*/

int Q3PopupMenu::exec(const QPoint & pos, int indexAtPoint)
{
    snapToMouse = true;
    if (!qApp)
        return -1;

    Q3PopupMenu* priorSyncMenu = syncMenu;

    syncMenu = this;
    syncMenuId = -1;

    QPointer<Q3PopupMenu> that = this;
    connectModal(that, true);
    popup(pos, indexAtPoint);
    qApp->enter_loop();
    connectModal(that, false);

    syncMenu = priorSyncMenu;
    return syncMenuId;
}



/*
  Connect the popup and all its submenus to modalActivation() if
  \a doConnect is true, otherwise disconnect.
 */
void Q3PopupMenu::connectModal(Q3PopupMenu* receiver, bool doConnect)
{
    if (!receiver)
        return;

    connectModalRecursionSafety = doConnect;

    if (doConnect)
        connect(this, SIGNAL(activated(int)),
                 receiver, SLOT(modalActivation(int)));
    else
        disconnect(this, SIGNAL(activated(int)),
                    receiver, SLOT(modalActivation(int)));

    for (int i = 0; i < mitems->size(); ++i) {
        Q3MenuItem *mi = mitems->at(i);
        if (mi->popup() && mi->popup() != receiver
             && (bool)(mi->popup()->connectModalRecursionSafety) != doConnect)
            mi->popup()->connectModal(receiver, doConnect); //avoid circular
    }
}


/*!
    Executes this popup synchronously.

    This is equivalent to \c{exec(mapToGlobal(QPoint(0,0)))}. In most
    situations you'll want to specify the position yourself, for
    example at the current mouse position:
    \code
        exec(QCursor::pos());
    \endcode
    or aligned to a widget:
    \code
        exec(somewidget.mapToGlobal(QPoint(0,0)));
    \endcode
*/

int Q3PopupMenu::exec()
{
    return exec(mapToGlobal(QPoint(0,0)));
}


/*  Internal slot used for exec(). */

void Q3PopupMenu::modalActivation(int id)
{
    syncMenuId = id;
}


/*!
    Sets the currently active item to index \a i and repaints as necessary.
*/

void Q3PopupMenu::setActiveItem(int i)
{
    int lastActItem = actItem;
    actItem = i;
    if (lastActItem >= 0)
        updateRow(lastActItem);
    if (i >= 0 && i != lastActItem)
        updateRow(i);
    Q3MenuItem *mi = mitems->at(actItem);
    if (!mi)
        return;

    if (mi->widget() && mi->widget()->isFocusEnabled()) {
        mi->widget()->setFocus();
    } else {
        setFocus();
        QRect mfrect = itemGeometry(actItem);
        setMicroFocusHint(mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), false);
    }
    if (mi->id() != -1)
        hilitSig(mi->id());
}


/*!
    \reimp
*/
QSize Q3PopupMenu::sizeHint() const
{
    ensurePolished();
    if(style().styleHint(QStyle::SH_Menu_Scrollable, this))
        return minimumSize(); //can be any size..

    Q3PopupMenu* that = (Q3PopupMenu*) this;
    //We do not need a resize here, just the sizeHint..
    return that->updateSize(false, false).expandedTo(QApplication::globalStrut());
}


/*!
    \overload

    Returns the id of the item at \a pos, or -1 if there is no item
    there or if it is a separator.
*/
int Q3PopupMenu::idAt(const QPoint& pos) const
{
    return idAt(itemAtPos(pos));
}


/*!
    \fn int Q3PopupMenu::idAt(int index) const

    Returns the identifier of the menu item at position \a index in
    the internal list, or -1 if \a index is out of range.

    \sa Q3MenuData::setId(), Q3MenuData::indexOf()
*/


/*!
    \reimp
 */
bool Q3PopupMenu::focusNextPrevChild(bool next)
{
    register Q3MenuItem *mi;
    int dy = next? 1 : -1;
    if (dy && actItem < 0) {
        setFirstItemActive();
    } else if (dy) {                                // highlight next/prev
        register int i = actItem;
        int c = mitems->count();
        int n = c;
        while (n--) {
            i = i + dy;
            if (i == c)
                i = 0;
            else if (i < 0)
                i = c - 1;
            mi = mitems->at(i);
            if (mi && !mi->isSeparator() &&
                 (style().styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, this)
                   || mi->isEnabledAndVisible()))
                break;
        }
        if (i != actItem)
            setActiveItem(i);
    }
    return true;
}


/*!
    \reimp
 */
void Q3PopupMenu::focusInEvent(QFocusEvent *)
{
}

/*!
    \reimp
 */
void Q3PopupMenu::focusOutEvent(QFocusEvent *)
{
}


class QTearOffMenuItem : public Q3CustomMenuItem
{
public:
    QTearOffMenuItem()
    {
    }
    ~QTearOffMenuItem()
    {
    }
    void paint(QPainter* p, const QPalette& pal, bool /* act*/,
                bool /*enabled*/, int x, int y, int w, int h)
    {
        p->setPen(QPen(pal.dark(), 1, Qt::DashLine));
        p->drawLine(x+2, y+h/2-1, x+w-4, y+h/2-1);
        p->setPen(QPen(pal.light(), 1, Qt::DashLine));
        p->drawLine(x+2, y+h/2, x+w-4, y+h/2);
    }
    bool fullSpan() const
    {
        return true;
    }

    QSize sizeHint()
    {
        return QSize(20, 6);
    }
};



/*!
    Inserts a tear-off handle into the menu. A tear-off handle is a
    special menu item that creates a copy of the menu when the menu is
    selected. This "torn-off" copy lives in a separate window. It
    contains the same menu items as the original menu, with the
    exception of the tear-off handle.

    The handle item is assigned the identifier \a id or an
    automatically generated identifier if \a id is < 0. The generated
    identifiers (negative integers) are guaranteed to be unique within
    the entire application.

    The \a index specifies the position in the menu. The tear-off
    handle is appended at the end of the list if \a index is negative.
*/
int Q3PopupMenu::insertTearOffHandle(int id, int index)
{
    int myid = insertItem(new QTearOffMenuItem, id, index);
    connectItem(myid, this, SLOT(toggleTearOff()));
    Q3MenuData::d->aInt = myid;
    return myid;
}


/*!\internal

  implements tear-off menus
 */
void Q3PopupMenu::toggleTearOff()
{
    if (active_popup_menu && active_popup_menu->tornOff) {
        active_popup_menu->close();
    } else  if (Q3MenuData::d->aPopup) {
        delete (Q3PopupMenu *)Q3MenuData::d->aPopup; // delete the old one
    } else {
        // create a tear off menu
        Q3PopupMenu* p = new Q3PopupMenu(parentWidget(), "tear off menu");
        connect(p, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
#ifndef QT_NO_WIDGET_TOPEXTRA
        p->setWindowTitle(windowTitle());
#endif
        p->setCheckable(isCheckable());
        QPoint geo = geometry().topLeft();
        p->setParent(parentWidget(), Qt::WType_TopLevel | Qt::WStyle_Tool | Qt::WDestructiveClose);
        p->move(geo);
        p->mitemsAutoDelete = false;
        p->tornOff = true;
        for (int i = 0; i < mitems->size(); ++i) {
            Q3MenuItem *mi = mitems->at(i);
            if (mi->id() != Q3MenuData::d->aInt && !mi->widget())
                p->mitems->append(mi);
        }
        p->show();
        Q3MenuData::d->aPopup = p;
    }
}

/*!
    \reimp
 */
void Q3PopupMenu::activateItemAt(int index)
{
    if (index >= 0 && index < (int) mitems->count()) {
        Q3MenuItem *mi = mitems->at(index);
        if (index != actItem)                        // new item activated
            setActiveItem(index);
        Q3PopupMenu *popup = mi->popup();
        if (popup) {
            if (popup->isVisible()) {                // sub menu already open
                int pactItem = popup->actItem;
                popup->actItem = -1;
                popup->hidePopups();
                popup->updateRow(pactItem);
            } else {                                // open sub menu
                hidePopups();
                actItem = index;
                subMenuTimer();
                popup->setFirstItemActive();
            }
        } else {
            byeMenuBar();                        // deactivate menu bar

#ifndef QT_NO_WHATSTHIS
            bool b = QWhatsThis::inWhatsThisMode();
#else
            const bool b = false;
#endif
            if (!mi->isEnabledAndVisible()) {
#ifndef QT_NO_WHATSTHIS
                if (b) {
                    actItem = -1;
                    updateItem(mi->id());
                    byeMenuBar();
                    actSig(mi->id(), b);
                }
#endif
            } else {
                byeMenuBar();                        // deactivate menu bar
                if (mi->isEnabledAndVisible()) {
                    actItem = -1;
                    updateItem(mi->id());
                    active_popup_menu = this;
                    QPointer<QSignalEmitter> signal = mi->signal();
                    int value = mi->signalValue();
                    actSig(mi->id(), b);
                    if (signal && !b)
                        signal->activate(&value);
                    active_popup_menu = 0;
                }
            }
        }
    } else {
        if (tornOff) {
            close();
        } else {
            Q3MenuData* p = parentMenu;
            hide();
#ifndef QT_NO_MENUBAR
            if (p && p->isMenuBar)
                ((Q3MenuBar*) p)->goodbye(true);
#endif
        }
    }

}

/*! \internal
  This private function is to update the scroll states in styles that support scrolling. */
void
Q3PopupMenu::updateScrollerState()
{
    uint old_scrollable = d->scroll.scrollable;
    d->scroll.scrollable = Q3PopupMenuPrivate::Scroll::ScrollNone;
    if(!style().styleHint(QStyle::SH_Menu_Scrollable, this))
        return;

    int row = 0;
    if(d->scroll.topScrollableIndex < mitems->size())
        row = d->scroll.topScrollableIndex;

    int y = 0, sh = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
    if(row) {
        // can't use |= because of a bug/feature in IBM xlC 5.0.2
        d->scroll.scrollable = d->scroll.scrollable | Q3PopupMenuPrivate::Scroll::ScrollUp;
        y += sh;
    }
    for (; row < mitems->size(); ++row) {
        Q3MenuItem *mi = mitems->at(row);

        int myheight = contentsRect().height();
        QStyleOptionMenuItem opt = getStyleOption(this, mi);
        opt.rect = rect();
        opt.tabWidth = 0;
        opt.maxIconWidth = maxPMWidth;
        QSize sz = style().sizeFromContents(QStyle::CT_MenuItem, &opt, QSize(0, itemHeight(mi)),
                                            fontMetrics(), this);
        if(y + sz.height() >= myheight) {
            d->scroll.scrollable = d->scroll.scrollable | Q3PopupMenuPrivate::Scroll::ScrollDown;
            break;
        }
        y += sz.height();
    }
    if((d->scroll.scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp) &&
       !(old_scrollable & Q3PopupMenuPrivate::Scroll::ScrollUp))
        d->scroll.topScrollableIndex++;
}

#endif // QT_NO_POPUPMENU

