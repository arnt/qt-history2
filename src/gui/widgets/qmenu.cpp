/****************************************************************************
**
** Implementation of QMenu and QMenuBar.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmenu.h"
#include "qstyle.h"
#include "qevent.h"
#include "qtimer.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qtoolbar.h"
#include "qmainwindow.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
# include "qaccessible.h"
#endif
#ifndef QT_NO_EFFECTS
# include <private/qeffects_p.h>
#endif
#ifndef QT_NO_WHATSTHIS
# include <qwhatsthis.h>
#endif

#include "qmenu_p.h"
#include "qmenubar_p.h"
#define d d_func()
#define q q_func()

/* QMenu code */
// internal class used for the torn off popup
class QTornOffMenu : public QMenu
{
    Q_OBJECT
public:
    QTornOffMenu(QMenu *p) : QMenu(0)
    {
        d->tornoff = 1;
        d->causedPopup = ((QTornOffMenu*)p)->d->causedPopup;

        setParent(p, WType_TopLevel | WStyle_Tool | WDestructiveClose | WStyle_NormalBorder);
        p->setWindowTitle(p->windowTitle());
        setEnabled(p->isEnabled());
        QObject::connect(this, SIGNAL(activated(QAction*)), p, SIGNAL(activated(QAction*)));
        QObject::connect(this, SIGNAL(highlighted(QAction*)), p, SIGNAL(highlighted(QAction*)));
        QList<QAction*> items = p->actions();
        for(int i = 0; i < items.count(); i++)
            addAction(items.at(i));
    }
    void syncWithMenu(QMenu *, QActionEvent *act)
    {
        if(act->type() == QEvent::ActionAdded)
            insertAction(act->before(), act->action());
        else if(act->type() == QEvent::ActionRemoved)
            removeAction(act->action());
    }
    void actionEvent(QActionEvent *e)
    {
        QMenu::actionEvent(e);
        resize(sizeHint());
    }
};
#include "qmenu.moc"

QList<QMenuAction*> QMenuPrivate::calcActionRects() const
{
    if(!itemsDirty)
        return actionItems;

    QList<QMenuAction*> ret;
    QList<QAction*> items = q->actions();
    int max_column_width = 0, dh = QApplication::desktop()->height(), ncols = 1, y = 0;
    const int hmargin = q->style().pixelMetric(QStyle::PM_MenuVMargin, q),
              vmargin = q->style().pixelMetric(QStyle::PM_MenuVMargin, q);

    //for compatability now - will have to refactor this away..
    tabWidth = maxIconWidth = 0;
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if(!action->isVisible())
            continue;
        QIconSet is = action->icon();
        if(!is.isNull())
            maxIconWidth = qMax(maxIconWidth, (uint)is.pixmap(QIconSet::Small, QIconSet::Normal).width() + 4);
    }

    //calculate size
    const QFontMetrics fm = q->fontMetrics();
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if(!action->isVisible())
            continue;

        QSize sz;

        //calc what I think the size is..
        if(action->isSeparator()) {
            sz = QSize(2, 2);
        } else {
            QString s = action->text();
            int t = s.indexOf('\t');
            if(t != -1) {
                tabWidth = qMax((int)tabWidth, fm.width(s.mid(t+1)));
                s = s.left(t);
            }
            int w = fm.width(s);
            w -= s.count('&') * fm.width('&');
            w += s.count("&&") * fm.width('&');
            sz.setWidth(w);
            sz.setHeight(fm.height());

            QIconSet is = action->icon();
            if(!is.isNull()) {
                QSize is_sz = is.pixmap(QIconSet::Small, QIconSet::Normal).size();
                if(is_sz.height() > sz.height())
                    sz.setHeight(is_sz.height());
            }
        }

        //let the style modify the above size..
        sz = q->style().sizeFromContents(QStyle::CT_MenuItem, q, sz, QStyleOption(action, maxIconWidth, 0));

        if(!sz.isEmpty()) {
            max_column_width = qMax(max_column_width, sz.width());
            //wrapping
            if(!scroll &&
               y+sz.height()+vmargin > dh - (q->style().pixelMetric(QStyle::PM_MenuDesktopFrameWidth, q) * 2)) {
                ncols++;
                y = vmargin;
            }
            y += sz.height();
            //append item
            QMenuAction *item = new QMenuAction;
            item->action = action;
            item->rect = QRect(0, 0, sz.width(), sz.height());
            ret.append(item);
        }
    }
    if(tabWidth)
        max_column_width += tabWidth; //finally add in the tab width

    //calculate position
    int x = hmargin;
    y = vmargin;
    for(int i = 0; i < ret.count(); i++) {
        QMenuAction *action = ret.at(i);
        if(!scroll &&
           y+action->rect.height() > dh - (q->style().pixelMetric(QStyle::PM_MenuDesktopFrameWidth, q) * 2)) {
            ncols--;
            if(ncols < 0)
                qWarning("QMenu: Column mismatch calculation. %d", ncols);
            x += max_column_width + hmargin;
            y = vmargin;
        }
        action->rect.moveBy(x, y);                        //move
        action->rect.setWidth(max_column_width); //uniform width
        y += action->rect.height();
    }
    return ret;
}

void QMenuPrivate::updateActions()
{
    if(!itemsDirty)
        return;
    for(QList<QMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it)
        delete (*it);
    sloppyAction = 0;
    actionItems = calcActionRects();
    ncols = 1;
    if(!scroll) {
        for(int i = 0, last_left = 0; i < actionItems.count(); i++) {
            int left = actionItems[i]->rect.left();
            if(left > last_left) {
                last_left = left;
                ncols++;
            }
        }
    }
    itemsDirty = 0;
}

QRect QMenuPrivate::actionRect(QMenuAction *act) const
{
    QRect ret = act->rect;
    if(scroll)
        ret.moveBy(0, scroll->scrollOffset);
    if(tearoff)
        ret.moveBy(0, q->style().pixelMetric(QStyle::PM_MenuTearoffHeight, q));
    const int fw = q->style().pixelMetric(QStyle::PM_MenuFrameWidth, q);
    ret.moveBy(fw, fw);
    return ret;
}

void QMenuPrivate::hideUpToMenuBar()
{
    if(!tornoff) {
        QWidget *caused = causedPopup;
        q->hide(); //hide after getting causedPopup
        while(caused) {
            if(QMenuBar *mb = qt_cast<QMenuBar*>(caused)) {
                mb->d->setCurrentAction(0);
                caused = 0;
            } else if(QMenu *m = qt_cast<QMenu*>(caused)) {
                caused = m->d->causedPopup;
                if(!m->d->tornoff)
                    m->hide();
                m->d->setCurrentAction(0);
            } else {
                qWarning("not possible..");
                caused = 0;
            }
        }
    }
    setCurrentAction(0);
}

void QMenuPrivate::popupAction(QMenuAction *action, int delay, bool activateFirst)
{
    if(action && action->action->menu()) {
        if(!delay) {
            q->internalDelayedPopup();
        } else {
            static QTimer *menuDelayTimer = 0;
            if(!menuDelayTimer)
                menuDelayTimer = new QTimer(qApp, "menu submenu timer");
            menuDelayTimer->disconnect(SIGNAL(timeout()));
            QObject::connect(menuDelayTimer, SIGNAL(timeout()),
                             q, SLOT(internalDelayedPopup()));
            menuDelayTimer->start(delay, true);
        }
        if(activateFirst)
            action->action->menu()->d->setFirstActionActive();
    }
}

void QMenuPrivate::setFirstActionActive()
{
    const int scrollerHeight = q->style().pixelMetric(QStyle::PM_MenuScrollerHeight, q);
    for(int i = 0, saccum = 0; i < actionItems.count(); i++) {
        QMenuAction *act = actionItems[i];
        if(scroll && scroll->scrollFlags & QMenuScroller::ScrollUp) {
            saccum -= act->rect.height();
            if(saccum > scroll->scrollOffset-scrollerHeight)
                continue;
        }
        if(!act->action->isSeparator() &&
           (q->style().styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, q) || act->action->isEnabled())) {
            setCurrentAction(act);
            break;
        }
    }
}

// popup == -1 means do not popup, 0 means immediately, others mean use a timer
void QMenuPrivate::setCurrentAction(QMenuAction *action, int popup, bool activateFirst)
{
    d->tearoffHighlighted = 0;
    if(action == currentAction)
        return;
    if(currentAction)
        q->update(actionRect(currentAction));

    d->sloppyAction = 0;
    if(!sloppyRegion.isEmpty())
        sloppyRegion = QRegion();
    currentAction = action;
    if(action && !action->action->isSeparator()) {
        activateAction(action->action, QAction::Hover);
        if(popup != -1)
            popupAction(d->currentAction, popup, activateFirst);
        q->update(actionRect(action));
    }
    if(activeMenu && (!action || !action->action->menu())) { //otherwise done in popupAction
        QMenu *menu = activeMenu;
        activeMenu = NULL;
        menu->hide();
    }
}

QMenuAction *QMenuPrivate::actionAt(QPoint p) const
{
    if(!q->rect().contains(p))     //sanity check
       return 0;

    for(int i = 0; i < actionItems.count(); i++) {
        QMenuAction *act = actionItems[i];
        if(actionRect(act).contains(p))
            return act;
    }
    return 0;
}

//actually performs the scrolling
void QMenuPrivate::scrollMenu(uint dir)
{
    if(!scroll || !(scroll->scrollFlags & dir)) //not really possible...
        return;

    //figure out how much to offset..
    int soff = 0, scrollHeight = q->style().pixelMetric(QStyle::PM_MenuScrollerHeight, q);
    if(dir == QMenuScroller::ScrollUp) {
        for(int i = 0, saccum = 0; i < actionItems.count(); i++) {
            QMenuAction *act = actionItems[i];
            saccum -= act->rect.height();
            if(saccum <= scroll->scrollOffset-scrollHeight) {
                soff = saccum - scroll->scrollOffset;
                break;
            }
        }
    } else if(dir == QMenuScroller::ScrollDown) {
        for(int i = 0, saccum = 0; i < actionItems.count(); i++) {
            QMenuAction *act = actionItems[i];
            saccum += act->rect.height();
            if(saccum >= -scroll->scrollOffset) {
                saccum = scroll->scrollOffset + saccum;
                int scrollerArea = q->height() - scrollHeight;
                for(i++ ; i < actionItems.count(); i++) {
                    act = actionItems[i];
                    saccum += act->rect.height();
                    if(saccum > scrollerArea) {
                        soff = -(scrollerArea - saccum);
                        break;
                    }
                }
                break;
            }
        }
    }

     //we can do resizing magic (ala Panther)
    int dh = QApplication::desktop()->height();
    const int desktopFrame = q->style().pixelMetric(QStyle::PM_MenuDesktopFrameWidth, q);
    if(q->height() < dh-(desktopFrame*2)) {
        QRect geom = q->geometry();
        if(dir == QMenuScroller::ScrollUp) {
            geom.setHeight(qMin(geom.height()-soff, dh-(desktopFrame*2)));
        } else if(dir == QMenuScroller::ScrollDown) {
            geom.setTop(qMax(desktopFrame, geom.top()-soff));
            if(geom != q->geometry())
                soff = 0;
        }
        q->setGeometry(geom);
    }

    //actually offset some things now (if necessary)
    if(soff) {
        scroll->scrollOffset -= soff;
        if(scroll->scrollOffset > 0)
            scroll->scrollOffset = 0;
    }

    //finally update the scroller status
    scroll->scrollFlags = QMenuScroller::ScrollNone;
    if(scroll->scrollOffset) //easy and cheap one
        scroll->scrollFlags |= QMenuScroller::ScrollUp;
    for(int i = 0; i < actionItems.count(); i++) {
        if(actionItems[i]->rect.bottom() > q->height()-scroll->scrollOffset) {
            scroll->scrollFlags |= QMenuScroller::ScrollDown;
            break;
        }
    }
    q->update();     //issue an update so we see all the new state..
}

/* This is poor-mans eventfilters. This avoids the use of
   eventFilter (which can be nasty for users of QMenuBar's). */
bool QMenuPrivate::mouseEventTaken(QMouseEvent *e)
{
    QPoint pos = q->mapFromGlobal(e->globalPos());
    if(scroll && !activeMenu) { //let the scroller "steal" the event
        bool isScroll = false;
        if(pos.x() >= 0 && pos.x() < q->width()) {
            const int scrollerHeight = q->style().pixelMetric(QStyle::PM_MenuScrollerHeight, q);
            for(int dir = QMenuScroller::ScrollUp; dir <= QMenuScroller::ScrollDown; dir = dir << 1) {
                if(scroll->scrollFlags & dir) {
                    if(dir == QMenuScroller::ScrollUp)
                        isScroll = (pos.y() <= scrollerHeight);
                    else if(dir == QMenuScroller::ScrollDown)
                        isScroll = (pos.y() >= q->height()-scrollerHeight);
                    if(isScroll) {
                        scroll->scrollDirection = dir;
                        break;
                    }
                }
            }
        }
        if(isScroll) {
            if(!scroll->scrollTimer)
                scroll->scrollTimer = new QBasicTimer;
            scroll->scrollTimer->start(50, q);
            return true;
        } else if(scroll->scrollTimer && scroll->scrollTimer->isActive()) {
            scroll->scrollTimer->stop();
        }
    }

    if(tearoff) { //let the tear off thingie "steal" the event..
        QRect tearRect(0, 0, q->width(), q->style().pixelMetric(QStyle::PM_MenuTearoffHeight, q));
        if(scroll && scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
            tearRect.moveBy(0, q->style().pixelMetric(QStyle::PM_MenuScrollerHeight, q));
        q->update(tearRect);
        if(tearRect.contains(pos)) {
            setCurrentAction(0);
            tearoffHighlighted = 1;
            if(e->type() == QEvent::MouseButtonRelease) {
                if(tornPopup) {
                    tornPopup->close();
                } else {
                    tornPopup = new QTornOffMenu(q);
                    tornPopup->setGeometry(q->geometry());
                    tornPopup->show();
                }
                hideUpToMenuBar();
            }
            return true;
        }
        tearoffHighlighted = 0;
    }

    if(q->frameGeometry().contains(e->globalPos())) //otherwise if the event is in our rect we want it..
        return false;

    for(QWidget *caused = causedPopup; caused;) {
        bool passOnEvent = false;
        QWidget *next_widget = 0;
        QPoint cpos = caused->mapFromGlobal(e->globalPos());
        if(QMenuBar *mb = qt_cast<QMenuBar*>(caused)) {
            passOnEvent = mb->rect().contains(cpos);
        } else if(QMenu *m = qt_cast<QMenu*>(caused)) {
            passOnEvent = m->d->actionAt(cpos);
            next_widget = m->d->causedPopup;
        }
        if(passOnEvent) {
            QMouseEvent new_e(e->type(), cpos, e->button(), e->state());
            QApplication::sendEvent(caused, &new_e);
            return true;
        }
        if(!next_widget)
            break;
        caused = next_widget;
    }
    return false;
}

void QMenuPrivate::activateAction(QAction *action, QAction::ActionEvent action_e)
{
    if(!action)
        return;
    action->activate(action_e);

#if defined(QT_ACCESSIBILITY_SUPPORT)
    if(action_e == QAction::Hover) {
        int actionID = indexOf(action);
        QAccessible::updateAccessibility(q, actionID, QAccessible::Focus);
        QAccessible::updateAccessibility(q, actionID, QAccessible::Selection);
    }
#endif

    for(QWidget *caused = q; caused;) {
        if(QMenuBar *mb = qt_cast<QMenuBar*>(caused)) {
            if(action_e == QAction::Trigger)
                emit mb->activated(action);
            else if(action_e == QAction::Hover)
                emit mb->highlighted(action);
            caused = 0;
        } else if(QMenu *m = qt_cast<QMenu*>(caused)) {
            caused = m->d->causedPopup;
            if(action_e == QAction::Trigger)
                emit m->activated(action);
            else if(action_e == QAction::Hover)
                emit m->highlighted(action);
        } else {
            qWarning("not possible..");
            caused = 0;
        }
    }
}

/*!
    \class QMenu qmenu.h
    \brief The QMenu class provides a menu widget for use in QMenuBar
    or context menus.

    \ingroup application
    \ingroup basic
    \mainclass

    A menu widget is a selection menu. It can be either a pull-down
    menu in a menu bar or a standalone context menu.  Pull-down menus
    are shown by the menu bar when the user clicks on the respective
    item or presses the specified shortcut key. Use
    QMenuBar::addAction() to insert a menu into a menu bar.  Show a
    context menu either asynchronously with popup() or synchronously
    with exec().

    Technically, a menu consists of a list of action items. You add
    actions with addAction(). An action is represented vertically and
    rendered by QStyle. In addition, items can have a text label, an
    optional icon drawn on the very left side, and an accelerator key
    such as "Ctrl+X".

    There are three kinds of action items: separators, action items
    that perform an action and menu items that show a
    submenu. Separators are inserted with addSeparator(). For submenus
    use addMenu(). All other items are considered action items.

    When inserting action items you usually specify a receiver and a
    slot. The receiver will be notifed whenever the item is selected.
    In addition, QMenu provides two signals, activated() and
    highlighted(), which signal the QAction that was triggered from
    the menu. 

    You clear a menu with clear() and remove single items with
    removeAction().

    A menu can display check marks for certain items when enabled with
    setCheckable(true). 

    A QMenu can also provide a tear-off menu. A tear-off menu is a
    top-level window that contains a copy of the menu. This makes it
    possible for the user to "tear off" frequently used menus and
    position them in a convenient place on the screen. If you want
    that functionality for a certain menu, insert a tear-off handle
    with setTearOffEnabled(). When using tear-off menus, bear in
    mind that the concept isn't typically used on Microsoft Windows so
    users may not be familiar with it. Consider using a QToolBar
    instead. 

    \link menu-example.html menu/menu.cpp\endlink is an example of
    QMenuBar and QMenu use.

    \important addAction removeAction clear addSeparator addMenu

    <img src=qmenu-m.png> <img src=qmenu-w.png>

    \sa QMenuBar
    \link guibooks.html#fowler GUI Design Handbook: Menu, Drop-Down and
    Pop-Up\endlink
*/


/*!
    Constructs a menu with parent \a parent.

    Although a popup menu is always a top-level widget, if a parent is
    passed the popup menu will be deleted when that parent is
    destroyed (as with any other QObject).
*/
QMenu::QMenu(QWidget *parent) : QWidget(*new QMenuPrivate, parent, WType_TopLevel|WType_Popup)
{
    setFocusPolicy(StrongFocus);
    setMouseTracking(style().styleHint(QStyle::SH_Menu_MouseTracking));
    if(style().styleHint(QStyle::SH_Menu_Scrollable, this)) {
        d->scroll = new QMenuPrivate::QMenuScroller;
        d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
#ifdef QT_COMPAT
    QObject::connect(this, SIGNAL(activated(QAction*)), this, SLOT(compatActivated(QAction*)));
    QObject::connect(this, SIGNAL(highlighted(QAction*)), this, SLOT(compatHighlighted(QAction*)));
#endif
}

/*!
    Destroys the menu.
*/
QMenu::~QMenu()
{
    if(d->sync)
        qApp->exit_loop();
    if(d->tornPopup)
        d->tornPopup->close();
}

/*!
  \overload

  Appends an action with text \a text to the list of actions.

  This convenience function will create a new QAction, setting its
  text, append it to the list of actions, and finally return the newly
  created action.

  \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QString &text)
{
    QAction *ret = new QAction(text);
    addAction(ret);
    return ret;
}

/*!
  \overload

  Appends an action with text \a text and icon \a icon to the list of
  actions.

  This convenience function will create a new QAction, setting its
  text and icon, append it to the list of actions, and finally return the
  newly created action.

  \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QIconSet &icon, const QString &text)
{
    QAction *ret = new QAction(icon, text);
    addAction(ret);
    return ret;
}

/*!
  \overload

  Appends an action with text \a text to the list of actions. This
  will automatically QObject::connect() the created QAction's
  triggered() signal to object's \a receiver function \a member.

  This convenience function will create a new QAction, setting its
  text, append it to the list of actions, and finally return the
  newly created action.

  \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QString &text, const QObject *receiver, const char* member)
{
    QAction *ret = new QAction(text, this);
    QObject::connect(ret, SIGNAL(triggered()), receiver, member);
    addAction(ret);
    return ret;
}

/*!
  \overload

  Appends an action with text \a text and icon \a icon to the list of
  actions. This will automatically QObject::connect() the created
  QAction's triggered() signal to object's \a receiver function \a
  member.

  This convenience function will create a new QAction, setting its
  text and icon, append it to the list of actions, and finally
  return the newly created action.

  \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QIconSet &icon, const QString &text, const QObject *receiver, const char* member)
{
    QAction *ret = new QAction(icon, text, this);
    QObject::connect(ret, SIGNAL(triggered()), receiver, member);
    addAction(ret);
    return ret;
}

/*!
  \overload

  Appends an action with text \a text and menu \a menu to the list of
  actions.

  This convenience function will create a new QAction, setting its
  text and menu, append it to the list of actions, and finally
  return the newly created action.

  \sa QWidget::addAction()
*/
QAction *QMenu::addMenu(const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, menu, this);
    addAction(ret);
    return ret;
}

/*!
  \overload

  Appends an action with QAction::separator() set to true.

  This convenience function will create a new QAction, append it to
  the list of actions, and finally return the newly created action.

  \sa QWidget::addAction()
*/
QAction *QMenu::addSeparator()
{
    QAction *ret = new QAction(this);
    ret->setSeparator(true);
    addAction(ret);
    return ret;
}

/*!
  \overload

  Inserts an action with text \a text and menu \a menu into the list
  of actions before \a before.

  This convenience function will create a new QAction, insert it to
  the list of actions, and finally return the newly created action.

  \sa QWidget::insertAction() addMenu()
*/
QAction *QMenu::insertMenu(QAction *before, const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, menu, this);
    insertAction(before, ret);
    return ret;
}

/*!
  \overload

  Inserts an action with QAction::separator() set to true into the
  list of actions before \a before.

  This convenience function will create a new QAction, insert it to
  the list of actions, and finally return the newly created action.

  \sa QWidget::insertAction() addSeparator()
*/
QAction *QMenu::insertSeparator(QAction *before)
{
    QAction *ret = new QAction(this);
    ret->setSeparator(true);
    insertAction(before, ret);
    return ret;
}

/*!
    \property QMenu::tearOffEnabled
    \brief whether the menu supports being torn off

    When true QMenu has a special menu item that creates a copy of the
    menu when the menu is selected. This "torn-off" copy lives in a
    separate window. It contains the same menu items as the original
    menu, with the exception of the tear-off handle.
*/
void QMenu::setTearOffEnabled(bool b)
{
    if(d->tearoff == b)
        return;
    if(!b && d->tornPopup)
        d->tornPopup->close();
    d->tearoff = b;

    d->itemsDirty = true;
    if(isVisible())
        resize(sizeHint());
}

bool QMenu::isTearOffEnabled() const
{
    return d->tearoff;
}

/*!
    \property QMenu::checkable
    \brief whether the display of check marks on menu items is enabled

    When true, the display of check marks on menu items is enabled.
    Checking is always enabled when in Windows-style.

    \sa QAction::setChecked()
*/
void QMenu::setCheckable(bool b)
{
    if(b == d->checkable)
        return;
    d->checkable = b;

    d->itemsDirty = true;
    if(isVisible())
        resize(sizeHint());
}

bool QMenu::isCheckable() const
{
    return d->checkable;
}

/*!  
  Returns the QAction that is currently highlighted. A null pointer
  will be returned if no action is currently selected.
*/
QAction *QMenu::activeAction() const
{
    return d->currentAction->action;
}

/*!
    Removes all actions.

    \sa removeAction()
*/
void QMenu::clear()
{
    QList<QAction*> acts = actions();
    for(int i = 0; i < acts.size(); i++)
        removeAction(acts[i]);
}

/*!
  \internal
  
  If a menu does not fit on the screen it lays itself out so that it
  does fit. It is style dependent what layout means (for example, on
  Windows it will use multiple columns).

  This functions returns the number of columns necessary.
*/
int QMenu::columnCount() const
{
    const_cast<QMenuPrivate*>(d)->updateActions();
    return d->ncols;
}

/*!
  \internal

  Return the item at \a pt, or 0 if there is no item there or if it is
  a separator item (and ignoreSeparator is true).
*/
QAction *QMenu::actionAtPos(const QPoint &pt, bool ignoreSeparator) const
{
    const_cast<QMenuPrivate*>(d)->updateActions();
    if(QMenuAction *ret = d->actionAt(pt)) {
        if(!ignoreSeparator || !ret->action->isSeparator())
            return ret->action;
    }
    return 0;
}

/*!
  \internal

  Returns the geometry of action \a act.
*/
QRect QMenu::actionGeometry(QAction *act) const
{
    const_cast<QMenuPrivate*>(d)->updateActions();
    for(QList<QMenuAction*>::ConstIterator it = d->actionItems.begin(); it != d->actionItems.end(); ++it) {
        if((*it)->action == act)
            return (*it)->rect;
    }
    return QRect();
}

/*!
    \reimp
*/
QSize QMenu::sizeHint() const
{
    ensurePolished();
    QSize s(0, 0);
    QList<QMenuAction*> actions = d->calcActionRects();
    if(!actions.isEmpty())
        s.setWidth(s.width()+actions[0]->rect.width()); //just take the first
    for(int i = 0; i < actions.count(); ++i) {
        QRect actionRect(actions[i]->rect);
        if(actionRect.right() > s.width())
            s.setWidth(actionRect.right());
        if(actionRect.bottom() > s.height())
            s.setHeight(actions[i]->rect.bottom());
    }
    if(d->tearoff)
        s.setHeight(s.height()+style().pixelMetric(QStyle::PM_MenuTearoffHeight, this));
    if(const int fw = q->style().pixelMetric(QStyle::PM_MenuFrameWidth, q)) {
        s.setWidth(s.width()+(fw*2));
        s.setHeight(s.height()+(fw*2));
    }
    s.setWidth(s.width()+q->style().pixelMetric(QStyle::PM_MenuHMargin, q));
    s.setHeight(s.height()+q->style().pixelMetric(QStyle::PM_MenuVMargin, q));
    return style().sizeFromContents(QStyle::CT_Menu, this, s.expandedTo(QApplication::globalStrut()));
}

/*!
    Displays the menu so that the action \a atAction will be at the
    specified \e global position \a p. To translate a widget's local
    coordinates into global coordinates, use QWidget::mapToGlobal().

    When positioning a menu with exec() or popup(), bear in mind that
    you cannot rely on the menu's current size(). For performance
    reasons, the menu adapts its size only when necessary, so in many
    cases, the size before and after the show is different. Instead,
    use sizeHint(). It calculates the proper size depending on the
    menu's current contents.

    \sa QWidget::mapToGlobal(), exec()
*/
void QMenu::popup(const QPoint &p, QAction *atAction)
{
    if(d->scroll) { //reset scroll state from last popup
        d->scroll->scrollOffset = 0;
        d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
    d->tearoffHighlighted = 0;

    d->updateActions();
    QPoint pos = p;
    QSize size = sizeHint();
    QRect screen = QApplication::desktop()->geometry();
    const int desktopFrame = style().pixelMetric(QStyle::PM_MenuDesktopFrameWidth, this);
    if(d->ncols != 1) {
        pos.setY(screen.top()+desktopFrame);
    } else if(atAction) {
        for(int i=0, above_height=0; i<(int)d->actionItems.count(); i++) {
            QMenuAction *action = d->actionItems.at(i);
            if(action->action == atAction) {
                int newY = pos.y()-above_height;
                if(d->scroll && newY < desktopFrame) {
                    d->scroll->scrollFlags |= QMenuPrivate::QMenuScroller::ScrollUp;
                    d->scroll->scrollOffset = newY;
                    newY = desktopFrame;
                }
                pos.setY(newY);

                if(!style().styleHint(QStyle::SH_Menu_FillScreenWithScroll)) {
                    int below_height = above_height + d->scroll->scrollOffset;
                    for(int i2 = i; i2 < (int)d->actionItems.count(); i2++)
                        below_height += d->actionItems.at(i2)->rect.height();
                    size.setHeight(below_height);
                }
                break;
            } else {
                above_height += action->rect.height();
            }
        }
    }
    if(d->scroll && pos.y()+size.height() > screen.height()-(desktopFrame*2)) {
        d->scroll->scrollFlags |= QMenuPrivate::QMenuScroller::ScrollDown;
        size.setHeight(screen.height()-desktopFrame-pos.y());
    }

    QPoint mouse = QCursor::pos();
    bool snapToMouse = (p == mouse);
    if(snapToMouse) {
        if(qApp->reverseLayout())
            pos.setX(pos.x()-size.width());
        if(pos.x()+size.width() > screen.right())
            pos.setX(mouse.x()-size.width());
        if(pos.y()+size.height() > screen.bottom() - (desktopFrame * 2))
            pos.setY(mouse.y()-size.height());
        if(pos.x() < screen.left())
            pos.setX(mouse.x());
        if(pos.y() < screen.top() + desktopFrame)
            pos.setY(screen.top() + desktopFrame);
    }
    setGeometry(QRect(pos, size));

#ifndef QT_NO_EFFECTS
    int hGuess = qApp->reverseLayout() ? QEffects::LeftScroll : QEffects::RightScroll;
    int vGuess = QEffects::DownScroll;
    if(qApp->reverseLayout()) {
        if((snapToMouse && (pos.x() + size.width()/2 > mouse.x())) ||
           (qt_cast<QMenu*>(d->causedPopup) && pos.x() + size.width()/2 > d->causedPopup->x()))
            hGuess = QEffects::RightScroll;
    } else {
        if((snapToMouse && (pos.x() + size.width()/2 < mouse.x())) ||
           (qt_cast<QMenu*>(d->causedPopup) && pos.x() + size.width()/2 < d->causedPopup->x()))
            hGuess = QEffects::LeftScroll;
    }

#ifndef QT_NO_MENUBAR
    if((snapToMouse && (pos.y() + size.height()/2 < mouse.y())) ||
       (qt_cast<QMenuBar*>(d->causedPopup) && pos.y() + size.width()/2 < d->causedPopup->mapToGlobal(d->causedPopup->pos()).y()))
       vGuess = QEffects::UpScroll;
#endif

    if(QApplication::isEffectEnabled(UI_AnimateMenu)) {
        if(QApplication::isEffectEnabled(UI_FadeMenu))
            qFadeEffect(this);
        else if(d->causedPopup)
            qScrollEffect(this, qt_cast<QMenu*>(d->causedPopup) ? hGuess : vGuess);
        else
            qScrollEffect(this, hGuess | vGuess);
    } else
#endif
    {
        show();
    }

#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuStart);
#endif
}

/*!
    \overload

    Executes this menu synchronously.

    This is equivalent to \c{exec(mapToGlobal(QPoint(0,0)))}. 

    This returns the selected QAction in either the popup menu or one
    of its submenus, or 0 if no item is selected (normally because the
    user pressed Esc).

    In most situations you'll want to specify the position yourself,
    for example at the current mouse position: 
    \code
      exec(QCursor::pos()); 
    \endcode 
    or aligned to a widget: 
    \code
      exec(somewidget.mapToGlobal(QPoint(0,0))); 
    \endcode
*/
QAction *QMenu::exec()
{
    return exec(mapToGlobal(QPoint(0,0)));
}


/*!
    \overload

    Executes this menu synchronously.

    Opens the menu so that the action \a action will be at the
    specified \e global position \a p. To translate a widget's local
    coordinates into global coordinates, use QWidget::mapToGlobal().

    This returns the selected QAction in either the popup menu or one
    of its submenus, or 0 if no item is selected (normally because the
    user pressed Esc).

    Note that all signals are emitted as usual. If you connect a
    QAction to a slot and call the menu's exec(), you get the result
    both via the signal-slot connection and in the return value of
    exec().

    Common usage is to position the menu at the current mouse
    position:
    \code
        exec(QCursor::pos());
    \endcode
    or aligned to a widget:
    \code
        exec(somewidget.mapToGlobal(QPoint(0, 0)));
    \endcode

    When positioning a menu with exec() or popup(), bear in mind that
    you cannot rely on the menu's current size(). For performance
    reasons, the menu adapts its size only when necessary. So in many
    cases, the size before and after the show is different. Instead,
    use sizeHint(). It calculates the proper size depending on the
    menu's current contents.

    \sa popup(), QWidget::mapToGlobal()
*/
QAction *QMenu::exec(const QPoint &p, QAction *action)
{
    d->sync = 1;
    popup(p, action);
    qApp->enter_loop();

    QAction *ret = d->syncAction;
    d->syncAction = 0;
    d->sync = 0;
    return ret;
}

/*!
    \overload

    Executes this menu synchronously.

    This returns the selected QAction in either the popup menu or one
    of its submenus, or 0 if no item is selected (normally because the
    user pressed Esc).

    This is equivelant to:
    \code
       QMenu menu;
       foreach (QAction *a, actions)
          menu->addAction(a);
       menu.exec(pos, at);
    \endcode

    \sa popup(), QWidget::mapToGlobal()
*/
QAction *QMenu::exec(QList<QAction*> actions, const QPoint &pos, QAction *at)
{
    QMenu menu;
    for(QList<QAction*>::Iterator it = actions.begin(); it != actions.end(); ++it)
        menu.addAction((*it));
    return menu.exec(pos, at);
}

/*!
  \reimp
*/
void QMenu::hideEvent(QHideEvent *)
{
    if(d->sync)
        qApp->exit_loop();
    d->setCurrentAction(0);
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuEnd);
#endif
    if(QMenuBar *mb = qt_cast<QMenuBar*>(d->causedPopup))
        mb->d->setCurrentAction(0);
    d->mouseDown = false;
    d->causedPopup = 0;
}

/*!
  \reimp
*/
void QMenu::paintEvent(QPaintEvent *e)
{
    d->updateActions();

    QPainter p(this);
    QRegion emptyArea = QRegion(rect());
    const int fw = style().pixelMetric(QStyle::PM_MenuFrameWidth, this);

    //draw the items that need updating..
    for(int i=0; i<(int)d->actionItems.count(); i++) {
        QMenuAction *action = d->actionItems.at(i);
        QRect adjustedActionRect = d->actionRect(action);
        if(!e->rect().intersects(adjustedActionRect))
           continue;

        //set the clip region to be extra safe (and adjust for the scrollers)
        QRegion adjustedActionReg(adjustedActionRect);
        emptyArea -= adjustedActionReg;
        p.setClipRegion(adjustedActionReg);

        QPalette pal = palette();
        QStyle::SFlags flags = QStyle::Style_Default;
        if(isEnabled() && action->action->isEnabled() && (!action->action->menu() || action->action->menu()->isEnabled()))
            flags |= QStyle::Style_Enabled;
        else
            pal.setCurrentColorGroup(QPalette::Disabled);
        if(d->currentAction == action)
            flags |= QStyle::Style_Active;
        if(d->mouseDown)
            flags |= QStyle::Style_Down;
        style().drawControl(QStyle::CE_MenuItem, &p, this, adjustedActionRect, pal, flags,
                            QStyleOption(action->action, d->maxIconWidth, d->tabWidth));
    }

    //draw the scroller regions..
    if(d->scroll) {
        const int scrollerHeight = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
        if(d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
            QRect topScroll(fw, fw, width()-(fw*2), scrollerHeight);
            emptyArea -= QRegion(topScroll);
            p.setClipRect(topScroll);
            QStyle::SFlags flags = QStyle::Style_Default;
            //if(d->scroll->scrollDirection & QMenuPrivate::QMenuScroller::ScrollUp)
            //flags |= QStyle::Style_Active;
            style().drawControl(QStyle::CE_MenuScroller, &p, this, topScroll, palette(), flags);
        }
        if(d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown) {
            QRect bottomScroll(fw, height()-scrollerHeight-fw, width()-(fw*2), scrollerHeight);
            emptyArea -= QRegion(bottomScroll);
            p.setClipRect(bottomScroll);
            QStyle::SFlags flags = QStyle::Style_Down;
            //if(d->scroll->scrollDirection & QMenuPrivate::QMenuScroller::ScrollDown)
            //flags |= QStyle::Style_Active;
            style().drawControl(QStyle::CE_MenuScroller, &p, this, bottomScroll, palette(), flags);
        }
    }
    //paint the tear off..
    if(d->tearoff) {
        QRect tearRect(fw, fw, width()-(fw*2), style().pixelMetric(QStyle::PM_MenuTearoffHeight, this));
        if(d->scroll && d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
            tearRect.moveBy(0, style().pixelMetric(QStyle::PM_MenuScrollerHeight, this));
        emptyArea -= QRegion(tearRect);
        p.setClipRect(tearRect);
        QStyle::SFlags flags = QStyle::Style_Default;
        if(d->tearoffHighlighted)
            flags |= QStyle::Style_Active;
        style().drawControl(QStyle::CE_MenuTearoff, &p, this, tearRect, palette(), flags);
    }
    //draw border
    if(fw) {
        QRegion borderReg;
        borderReg += QRect(0, 0, fw, height()); //left
        borderReg += QRect(width()-fw, 0, fw, height()); //right
        borderReg += QRect(0, 0, width(), fw); //top
        borderReg += QRect(0, height()-fw, width(), fw); //bottom
        p.setClipRegion(borderReg);
        emptyArea -= borderReg;
        style().drawPrimitive(QStyle::PE_MenuFrame, &p, rect(), palette(), QStyle::Style_Default);
    }

    //finally the rest of the space
    p.setClipRegion(emptyArea);
    style().drawControl(QStyle::CE_MenuEmptyArea, &p, this, rect(), palette());
}

/*!
  \reimp
*/
void QMenu::mousePressEvent(QMouseEvent *e)
{
    if(d->mouseEventTaken(e))
        return;
    if(e->button() != LeftButton)
        return;
    if(!rect().contains(e->pos())) {
        d->hideUpToMenuBar();
        return;
    }
    d->mouseDown = true;

    QMenuAction *action = d->actionAt(e->pos());
    d->setCurrentAction(action, 20);
}

/*!
  \reimp
*/
void QMenu::mouseReleaseEvent(QMouseEvent *e)
{
    if(d->mouseEventTaken(e))
        return;
    if(e->button() != LeftButton || !d->mouseDown)
        return;
    d->mouseDown = false;
    QMenuAction *action = d->actionAt(e->pos());
    if(d->sync)
        d->syncAction = action ? action->action : 0;
    if(action && action->action->isEnabled()) {
        if(action->action->menu()) {
            action->action->menu()->d->setFirstActionActive();
        } else {
            if(action)
                d->activateAction(action->action, QAction::Trigger);
            d->hideUpToMenuBar();
        }
    }
}

/*!
  \reimp
*/
void QMenu::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::StyleChange) {
        d->itemsDirty = 1;
        setMouseTracking(style().styleHint(QStyle::SH_Menu_MouseTracking));
        if(isVisible())
            resize(sizeHint());
        if(style().styleHint(QStyle::SH_Menu_Scrollable, this)) {
            delete d->scroll;
            d->scroll = 0;
        } else if(!d->scroll) {
            d->scroll = new QMenuPrivate::QMenuScroller;
        }
    } else if(e->type() == QEvent::EnabledChange) {
        if (d->tornPopup) // torn-off menu
            d->tornPopup->setEnabled(isEnabled());
    }
    QWidget::changeEvent(e);
}

/*!
  \reimp
*/
bool
QMenu::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if(ke->key() == Key_Tab || ke->key() == Key_BackTab) {
            keyPressEvent(ke);
            return true;
        }
    }
    return QWidget::event(e);
}

/*!
  \reimp
*/
void QMenu::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if(QApplication::reverseLayout()) {  // in reverse mode open/close key for submenues are reversed
        if(key == Key_Left)
            key = Key_Right;
        else if(key == Key_Right)
            key = Key_Left;
    }
    if(key == Key_Tab) //means down
        key = Key_Down;

    bool key_consumed = false;
    switch(key) {
    case Key_Up:
    case Key_Down: {
        QMenuAction *nextAction = 0;
        uint scroll_direction = QMenuPrivate::QMenuScroller::ScrollNone;
        if(!d->currentAction) {
            nextAction = d->actionItems.first();
        } else {
            for(int i=0, y=0; !nextAction && i < (int)d->actionItems.count(); i++) {
                QMenuAction *act = d->actionItems.at(i);
                if(act == d->currentAction) {
                    if(key == Key_Up) {
                        for(int next_i = i-1; true; next_i--) {
                            if(next_i == -1) {
                                if(d->scroll)
                                    break;
                                next_i = d->actionItems.count()-1;
                            }
                            QMenuAction *next = d->actionItems.at(next_i);
                            if(next == d->currentAction)
                                break;
                            if(next->action->isSeparator() ||
                               (!next->action->isEnabled() &&
                                !style().styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, this)))
                                continue;
                            nextAction = next;
                            if(d->scroll && (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)) {
                                int topVisible = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
                                if(d->tearoff)
                                    topVisible += style().pixelMetric(QStyle::PM_MenuTearoffHeight, q);
                                if(((y + d->scroll->scrollOffset) - topVisible) < act->rect.height())
                                    scroll_direction = QMenuPrivate::QMenuScroller::ScrollUp;
                            }
                            break;
                        }
                        if(!nextAction && d->tearoff)
                            d->tearoffHighlighted = 1;
                    } else {
                        for(int next_i = i+1; true; next_i++) {
                            if(next_i == d->actionItems.count()) {
                                if(d->scroll)
                                    break;
                                next_i = 0;
                            }
                            QMenuAction *next = d->actionItems.at(next_i);
                            if(next == d->currentAction)
                                break;
                            if(next->action->isSeparator() ||
                               (!next->action->isEnabled() &&
                                !style().styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, this)))
                                continue;
                            nextAction = next;
                            if(d->scroll && (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown)) {
                                const int scrollerHeight = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
                                int bottomVisible = height()-scrollerHeight;
                                if(d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
                                    bottomVisible -= scrollerHeight;
                                if(d->tearoff)
                                    bottomVisible -= style().pixelMetric(QStyle::PM_MenuTearoffHeight, q);
                                if((y + d->scroll->scrollOffset + act->rect.height()) > bottomVisible)
                                    scroll_direction = QMenuPrivate::QMenuScroller::ScrollDown;
                            }
                            break;
                        }
                    }
                    break;
                }
                y += act->rect.height();
            }
        }
        if(nextAction) {
            if(d->scroll && scroll_direction != QMenuPrivate::QMenuScroller::ScrollNone) {
                if(d->scroll->scrollTimer)
                    d->scroll->scrollTimer->stop();
                d->scrollMenu(scroll_direction);
            }
            d->setCurrentAction(nextAction);
            key_consumed = true;
        }
        break; }

    case Key_Right:
        if(d->currentAction && d->currentAction->action->isEnabled() && d->currentAction->action->menu()) {
            d->popupAction(d->currentAction, 0, true);
            key_consumed = true;
            break;
        }
        //FALL THROUGH
    case Key_Left: {
        if(key == Key_Left && d->causedPopup && qt_cast<QMenu*>(d->causedPopup)) {
            hide();
            key_consumed = true;
        } else if(d->currentAction && !d->scroll) {
            QMenuAction *nextAction = 0;
            if(key == Key_Left) {
                QRect actionR = d->actionRect(d->currentAction);
                for(int x = actionR.left()-1; !nextAction && x >= 0; x--)
                    nextAction = d->actionAt(QPoint(x, actionR.center().y()));
            } else {
                QRect actionR = d->actionRect(d->currentAction);
                for(int x = actionR.right()+1; !nextAction && x < width(); x++)
                    nextAction = d->actionAt(QPoint(x, actionR.center().y()));
            }
            if(nextAction) {
                d->setCurrentAction(nextAction);
                key_consumed = true;
            }
        }
        break; }

    case Key_Alt:
        key_consumed = true;
        break;

    case Key_Escape:
        key_consumed = true;
        if(d->tornoff) {
            close();
            return;
        }
        hide();
        break;

    case Key_Space:
        if(!style().styleHint(QStyle::SH_Menu_SpaceActivatesItem, this))
            break;
        // for motif, fall through
    case Key_Return:
    case Key_Enter: {
            if(!d->currentAction)
                break;
#ifndef QT_NO_WHATSTHIS
            bool whats_this_mode = QWhatsThis::inWhatsThisMode();
#else
            const bool whats_this_mode = false;
#endif
            if(!d->currentAction->action->isEnabled() && !whats_this_mode)
                break;
            if(d->currentAction->action->menu()) {
                d->popupAction(d->currentAction, 20, true);
            } else {
                d->activateAction(d->currentAction->action, QAction::Trigger);
                d->hideUpToMenuBar();
            }
            key_consumed = true;
            break; }

#ifndef QT_NO_WHATSTHIS
    case Key_F1:
        if(!d->currentAction || d->currentAction->action->whatsThis().isNull())
            break;
        if(!QWhatsThis::inWhatsThisMode())
            QWhatsThis::enterWhatsThisMode();
        QWhatsThis::leaveWhatsThisMode();
        //fall-through!
#endif
    default:
        key_consumed = false;
    }

    if(!key_consumed) {                                // send to menu bar
        if(QWidget *caused = d->causedPopup) {
            while(QMenu *m = qt_cast<QMenu*>(caused))
                caused = m->d->causedPopup;
            if(QMenuBar *mb = qt_cast<QMenuBar*>(caused)) {
                QMenuAction *oldAct = mb->d->currentAction;
                QApplication::sendEvent(mb, e);
                if(mb->d->currentAction != oldAct)
                    key_consumed = true;
            }
        }


        if(!key_consumed && (!e->state() || e->state() == AltButton || e->state() == ShiftButton) && e->text().length()==1) {
            int clashCount = 0;
            QMenuAction *first = 0, *currentSelected = 0, *firstAfterCurrent = 0;
            {
                QChar c = e->text()[0].toUpper();
                for(int i = 0; i < d->actionItems.size(); ++i) {
                    register QMenuAction *act = d->actionItems.at(i);
                    QString s = act->action->text();
                    if(!s.isEmpty()) {
                        int ampersand = s.indexOf('&');
                        if(ampersand >= 0) {
                            if(s[ampersand+1].toUpper() == c) {
                                clashCount++;
                                if(!first)
                                    first = act;
                                if(act == d->currentAction)
                                    currentSelected = act;
                                else if (!firstAfterCurrent && currentSelected)
                                    firstAfterCurrent = act;
                            }
                        }
                    }
                }
            }
            QMenuAction *next_action = 0;
            if(clashCount >= 1) {
                if(clashCount == 1 || !d->currentAction || (currentSelected && !firstAfterCurrent))
                    next_action = first;
                else
                    next_action = firstAfterCurrent;
            }
            if(next_action) {
                d->setCurrentAction(next_action, 20, true);
                if(!next_action->action->menu()) {
                    d->activateAction(next_action->action, QAction::Trigger);
                    d->hideUpToMenuBar();
                }
            }
        }
#ifdef Q_OS_WIN32
        if (key_consumed && (e->key() == Key_Control || e->key() == Key_Shift || e->key() == Key_Meta))
            qApp->beep();
#endif // Q_OS_WIN32
    }
}

/*!
  \reimp
*/
void QMenu::mouseMoveEvent(QMouseEvent *e)
{
    if(d->mouseEventTaken(e))
        return;

    QMenuAction *action = d->actionAt(e->pos());
    if(!action) {
        const int fw = q->style().pixelMetric(QStyle::PM_MenuFrameWidth, q);
        if(e->pos().x() <= fw || e->pos().x() >= width()-fw ||
           e->pos().y() <= fw || e->pos().y() >= height()-fw)
            return; //mouse over frame
    } else {
        d->mouseDown = e->state() & LeftButton;
    }
    if(d->sloppyRegion.contains(e->pos())) {
        static QTimer *sloppyDelayTimer = 0;
        if(!sloppyDelayTimer)
            sloppyDelayTimer = new QTimer(qApp, "menu sloppy timer");
        sloppyDelayTimer->disconnect(SIGNAL(timeout()));
        QObject::connect(sloppyDelayTimer, SIGNAL(timeout()),
                         q, SLOT(internalSetSloppyAction()));
        sloppyDelayTimer->start(style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this)*6, true);
        d->sloppyAction = action;
    } else {
        d->setCurrentAction(action, style().styleHint(QStyle::SH_Menu_SubMenuPopupDelay, this));
    }
}

/*!
  \reimp
*/
void QMenu::leaveEvent(QEvent *)
{
    d->sloppyAction = 0;
    if(!d->sloppyRegion.isEmpty())
        d->sloppyRegion = QRegion();
#if 0
    if(!d->tornoff)
        d->setCurrentAction(0);
#endif
}

/*!
  \reimp
*/
void
QMenu::timerEvent(QTimerEvent *e)
{
    if(d->scroll && d->scroll->scrollTimer && d->scroll->scrollTimer->timerId() == e->timerId()) {
        d->scrollMenu(d->scroll->scrollDirection);
        if(d->scroll->scrollFlags == QMenuPrivate::QMenuScroller::ScrollNone)
            d->scroll->scrollTimer->stop();
    }
}

/*!
  \reimp
*/
void QMenu::actionEvent(QActionEvent *e)
{
    d->itemsDirty = 1;
    if(d->tornPopup)
        d->tornPopup->syncWithMenu(this, e);
#ifdef Q_WS_MAC
    if(d->mac_menu) {
        if(e->type() == QEvent::ActionAdded)
            d->mac_menu->addAction(e->action(), d->mac_menu->findAction(e->before()));
        else if(e->type() == QEvent::ActionRemoved)
            d->mac_menu->removeAction(e->action());
        else if(e->type() == QEvent::ActionChanged)
            d->mac_menu->syncAction(e->action());
    }
#endif

    if(isVisible())
        update();
}

/*!
  \internal
*/
void QMenu::internalSetSloppyAction()
{
    if(d->sloppyAction)
        d->setCurrentAction(d->sloppyAction, 0);
}

/*!
  \internal
*/
void QMenu::internalDelayedPopup()
{
    if(!d->currentAction || !d->currentAction->action || d->currentAction->action->menu() == d->activeMenu)
        return;

    //hide the current item
    if(QMenu *menu = d->activeMenu) {
        d->activeMenu = NULL;
        menu->hide();
    }

    //setup
    QRect actionRect(d->actionRect(d->currentAction));
    QPoint pos(mapToGlobal(QPoint(width(), actionRect.top())));
    d->activeMenu = d->currentAction->action->menu();
    d->activeMenu->d->causedPopup = this;
    if(d->activeMenu->parent() != this)
        d->activeMenu->setParent(this, d->activeMenu->getWFlags());

    bool on_left = false;     //find "best" position
    const QSize menuSize(d->activeMenu->sizeHint());
    if(QApplication::reverseLayout()) {
        on_left = true;
        QMenu *caused = qt_cast<QMenu*>(d->causedPopup);
        if(caused && caused->x() < x() || x() - menuSize.width() < 0)
            on_left = false;
    } else {
        QMenu *caused = qt_cast<QMenu*>(d->causedPopup);
        if(caused && caused->x() > x() ||
           x() + width() + menuSize.width() > QApplication::desktop()->width())
            on_left = true;
    }
    if(on_left)
        pos.setX(-menuSize.width());

    //calc sloppy focus buffer
    if(style().styleHint(QStyle::SH_Q3PopupMenu_SloppySubMenus, this)) {
        QPoint cur = QCursor::pos();
        if(actionRect.contains(mapFromGlobal(cur))) {
            QPoint pts[4];
            pts[0] = QPoint(cur.x(), cur.y() - 2);
            pts[3] = QPoint(cur.x(), cur.y() + 2);
            if(pos.x() >= cur.x())        {
                pts[1] = QPoint(geometry().right(), pos.y());
                pts[2] = QPoint(geometry().right(), pos.y() + menuSize.height());
            } else {
                pts[1] = QPoint(pos.x() + menuSize.width(), pos.y());
                pts[2] = QPoint(pos.x() + menuSize.width(), pos.y() + menuSize.height());
            }
            QPointArray points(4);
            for(int i = 0; i < 4; i++)
                points.setPoint(i, mapFromGlobal(pts[i]));
            d->sloppyRegion = QRegion(points);
        }
    }

    //do the popup
    d->activeMenu->popup(pos);
}

#ifdef QT_COMPAT
int QMenu::insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                          const QKeySequence *accel, const QMenu *popup, int id, int index)
{
    QAction *act = new QAction;
    if(id != -1)
        act->setId(id);
    if(icon)
        act->setIcon(*icon);
    if(text)
        act->setText(*text);
    if(popup)
        act->setMenu(const_cast<QMenu*>(popup));
    if(accel)
        act->setAccel(*accel);
    if(receiver && member)
        QObject::connect(act, SIGNAL(triggered()), receiver, member);
    if(index == -1)
        addAction(act);
    else
        insertAction(act, actions().value(index+1));
    return act->id();
}

int QMenu::insertSeparator(int index)
{
    QAction *act = new QAction;
    act->setSeparator(true);
    if(index == -1)
        addAction(act);
    else
        insertAction(act, actions().value(index+1));
    return act->id();
}

QAction *QMenu::findActionForId(int id) const
{
    QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *a = list.at(i);
        if (a->id() == id)
            return a;
    }
    return 0;
}

int QMenu::frameWidth() const
{
    return style().pixelMetric(QStyle::PM_MenuFrameWidth, this);
}

void QMenu::compatActivated(QAction *act)
{
    emit activated(act->id());
}

void QMenu::compatHighlighted(QAction *act)
{
    emit highlighted(act->id());
}
#endif
