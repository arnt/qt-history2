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
#include "qapplication.h"
#include "qdesktopwidget.h"
#ifndef QT_NO_ACCESSIBILITY
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

        setParent(p, Qt::WType_TopLevel | Qt::WStyle_Tool | Qt::WDestructiveClose | Qt::WStyle_NormalBorder);
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
        resize(sizeHint()+contentsMarginSize());
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
    const int hmargin = q->style().pixelMetric(QStyle::PM_MenuHMargin, q),
              vmargin = q->style().pixelMetric(QStyle::PM_MenuVMargin, q);

    //for compatability now - will have to refactor this away..
    const_cast<QMenuPrivate *>(this)->tabWidth = 0;
    const_cast<QMenuPrivate *>(this)->maxIconWidth = 0;
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if(!action->isVisible())
            continue;
        QIconSet is = action->icon();
        if(!is.isNull())
            const_cast<QMenuPrivate *>(this)->maxIconWidth =
                qMax(maxIconWidth,
                     (uint)is.pixmap(QIconSet::Small, QIconSet::Normal).width() + 4);
    }

    //calculate size
    QFontMetrics qfm = q->fontMetrics();
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if(!action->isVisible())
            continue;

        QFontMetrics fm(action->font().resolve(q->font()));
        QSize sz;

        //calc what I think the size is..
        if(action->isSeparator()) {
            sz = QSize(2, 2);
        } else {
            QString s = action->menuText();
            int t = s.indexOf('\t');
            if(t != -1) {
                const_cast<QMenuPrivate *>(this)->tabWidth =
                    qMax((int)tabWidth, qfm.width(s.mid(t+1)));
                s = s.left(t);
            } else {
                QKeySequence seq = action->shortcut();
                if (!seq.isEmpty())
                    const_cast<QMenuPrivate *>(this)->tabWidth =
                        qMax((int)tabWidth, fm.width(seq));
            }
            int w = fm.width(s);
            w -= s.count('&') * fm.width('&');
            w += s.count("&&") * fm.width('&');
            sz.setWidth(w);
            sz.setHeight(qMax(fm.height(), qfm.height()));

            QIconSet is = action->icon();
            if(!is.isNull()) {
                QSize is_sz = is.pixmap(QIconSet::Small, QIconSet::Normal).size();
                if(is_sz.height() > sz.height())
                    sz.setHeight(is_sz.height());
            }
        }

        //let the style modify the above size..
        QStyleOptionMenuItem opt = getStyleOption(action);
        opt.rect = q->rect();
        sz = q->style().sizeFromContents(QStyle::CT_MenuItem, &opt, sz, q->fontMetrics(), q);

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
    ret.moveBy(fw+leftmargin, fw+topmargin);
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
                menuDelayTimer = new QTimer(qApp);
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
        scroll->scrollFlags = scroll->scrollFlags | QMenuScroller::ScrollUp;
    for(int i = 0; i < actionItems.count(); i++) {
        if(actionItems[i]->rect.bottom() > q->height()-scroll->scrollOffset) {
            scroll->scrollFlags = scroll->scrollFlags | QMenuScroller::ScrollDown;
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

    if(action_e == QAction::Hover) {
#ifndef QT_NO_ACCESSIBILITY
        int actionID = indexOf(action);
        QAccessible::updateAccessibility(q, actionID, QAccessible::Focus);
        QAccessible::updateAccessibility(q, actionID, QAccessible::Selection);
#endif
        action->showStatusText(q);
    } else if(action_e == QAction::Trigger) {
        hideUpToMenuBar();
    }

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

QStyleOptionMenuItem QMenuPrivate::getStyleOption(const QAction *action) const
{
    QStyleOptionMenuItem opt(0);
    opt.palette = q->palette();
    opt.state = QStyle::Style_Default;

    if (q->isEnabled() && action->isEnabled()
            && (!action->menu() || action->menu()->isEnabled()))
        opt.state |= QStyle::Style_Enabled;
    else
        opt.palette.setCurrentColorGroup(QPalette::Disabled);

    opt.font = action->font();

    if (defaultAction == action)
        opt.state |= QStyle::Style_ButtonDefault; //probably should be something else
    if (currentAction && currentAction->action == action)
        opt.state |= QStyle::Style_Active;
    if (mouseDown)
        opt.state |= QStyle::Style_Down;
    if (!checkable)
        opt.checkState = QStyleOptionMenuItem::NotCheckable;
    else
        opt.checkState = action->isChecked() ? QStyleOptionMenuItem::Checked
                                             : QStyleOptionMenuItem::Unchecked;
    if (action->menu())
        opt.menuItemType = QStyleOptionMenuItem::SubMenu;
    else if (action->isSeparator())
        opt.menuItemType = QStyleOptionMenuItem::Separator;
    else
        opt.menuItemType = QStyleOptionMenuItem::Normal;
    opt.icon = action->icon();
    QString textAndAccel = action->text();
    if (textAndAccel.indexOf('\t') == -1) {
        QKeySequence seq = action->shortcut();
        if(!seq.isEmpty())
            textAndAccel += '\t' + QString(seq);
    }
    opt.text = textAndAccel;
    opt.tabWidth = tabWidth;
    opt.maxIconWidth = maxIconWidth;
    opt.menuRect = q->rect();
    return opt;
}

/*!
    \class QMenu
    \brief The QMenu class provides a menu widget for use in menu
    bars, context menus, and other popup menus.

    \ingroup application
    \ingroup basic
    \mainclass

    A menu widget is a selection menu. It can be either a pull-down
    menu in a menu bar or a standalone context menu. Pull-down menus
    are shown by the menu bar when the user clicks on the respective
    item or presses the specified shortcut key. Use
    QMenuBar::addAction() to insert a menu into a menu bar. Context
    menus are usually invoked by some special keyboard key or by
    right-clicking. They can be executed either asynchronously with
    popup() or synchronously with exec(). Menus can also be invoked in
    response to button presses; these are just like context menus
    except for how they are invoked.

    A menu consists of a list of action items. Actions are added with
    addAction(). An action is represented vertically and rendered by
    QStyle. In addition, actions can have a text label, an optional
    icon drawn on the very left side, and shortcut key sequence such
    as "Ctrl+X".

    There are three kinds of action items: separators, actions that
    show a submenu, and actions that perform an action. Separators are
    inserted with addSeparator(). For submenus use addMenu(). All
    other items are considered action items.

    When inserting action items you usually specify a receiver and a
    slot. The receiver will be notifed whenever the item is
    triggered(). In addition, QMenu provides two signals, activated()
    and highlighted(), which signal the QAction that was triggered
    from the menu.

    You clear a menu with clear() and remove individual action items
    with removeAction().

    If isCheckable() is true (which is the default for Windows), any
    of the menu's action items can be checked.

    A QMenu can also provide a tear-off menu. A tear-off menu is a
    top-level window that contains a copy of the menu. This makes it
    possible for the user to "tear off" frequently used menus and
    position them in a convenient place on the screen. If you want
    this functionality for a particular menu, insert a tear-off handle
    with setTearOffEnabled(). When using tear-off menus, bear in mind
    that the concept isn't typically used on Microsoft Windows so
    some users may not be familiar with it. Consider using a QToolBar
    instead.

    \link menu-example.html menu/menu.cpp\endlink is an example of
    QMenuBar and QMenu use.

    Important inherited functions: addAction(), removeAction(), clear(),
    addSeparator(), and addMenu().

    \inlineimage qmenu-m.png Screenshot in Motif style
    \inlineimage qmenu-w.png Screenshot in Windows style

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
QMenu::QMenu(QWidget *parent) : QWidget(*new QMenuPrivate, parent, Qt::WType_TopLevel|Qt::WType_Popup)
{
    setFocusPolicy(Qt::StrongFocus);
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

    This convenience function creates a new action with \a text.
    The function adds the newly created action to the menu's
    list of actions, and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QString &text)
{
    QAction *ret = new QAction(text, this);
    addAction(ret);
    return ret;
}

/*!
    \overload

    This convenience function creates a new action with an \a icon
    and some \a text. The function adds the newly created action to
    the menu's list of actions, and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QIconSet &icon, const QString &text)
{
    QAction *ret = new QAction(icon, text, this);
    addAction(ret);
    return ret;
}

/*!
    \overload

    This convenience function creates a new action with the text \a
    text. The action's triggered() signal is connected to the \a
    receiver's \a member slot. The function adds the newly created
    action to the menu's list of actions and returns it.

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

    This convenience function creates a new action with an \a icon and
    some \a text.  The action's triggered() signal is connected to the
    \a member slot of the \a receiver object. The function adds the
    newly created action to the menu's list of actions, and returns
    it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QIconSet &icon, const QString &text, const QObject *receiver,
                          const char* member)
{
    QAction *ret = new QAction(icon, text, this);
    QObject::connect(ret, SIGNAL(triggered()), receiver, member);
    addAction(ret);
    return ret;
}

/*!
    This convenience function creates a new action with some \a
    text, and a submenu specified by \a menu. The function adds the newly
    created action to the menu's list of actions, and returns it.
     \sa QWidget::addAction()
*/
QAction *QMenu::addMenu(const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, this);
    ret->setMenu(menu);
    addAction(ret);
    return ret;
}

/*!
    \overload

    This convenience function creates a new action with an \a icon,
    some \a text, and a submenu specified by \a menu. The function adds
    the newly created action to the menu's list of actions, and
    returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addMenu(const QIconSet &icon, const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, this);
    ret->setMenu(menu);
    ret->setIcon(icon);
    addAction(ret);
    return ret;
}

/*!
    This convenience function creates a new separator action, i.e. an
    action with QAction::separator() set to true, and adds the new
    action to this menu's list of actions. It returns the newly
    created action.

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
    This convenience function creates a new action with the text \a
    text, and submenu \a menu. The function inserts the newly created
    action into this menu's list of actions before action \a before
    and returns it.

    \sa QWidget::insertAction() addMenu()
*/
QAction *QMenu::insertMenu(QAction *before, const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, this);
    ret->setMenu(menu);
    insertAction(before, ret);
    return ret;
}

/*!
    \overload

    This convenience function creates a new action with an icon \a icon, the text \a
    text, and submenu \a menu. The function inserts the newly created
    action into this menu's list of actions before action \a before
    and returns it.

    \sa QWidget::insertAction() addMenu()
*/
QAction *QMenu::insertMenu(QAction *before, const QIconSet &icon, const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, this);
    ret->setMenu(menu);
    ret->setIcon(icon);
    insertAction(before, ret);
    return ret;
}

/*!
    This convenience function creates a new separator action, i.e. an
    action with QAction::separator() set to true. The function inserts
    the newly created action into this menu's list of actions before
    action \a before and returns it.

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
  This will set the default action to \a act. The default action may
  have a visual queue depending on the current QStyle. A default
  action is usually meant to indicate what will defaultly happen on a
  drop, as shown in a context menu.

  \sa defaultAction()
*/
void QMenu::setDefaultAction(QAction *act)
{
    d->defaultAction = act;
}

/*!
  Returns the current default action.

  \sa setDefaultAction()
*/
QAction *QMenu::defaultAction() const
{
    return d->defaultAction;
}

/*!
    \property QMenu::tearOffEnabled
    \brief whether the menu supports being torn off

    When true, QMenu has a special menu item (often shown as a dashed
    line at the top of the menu) that creates a copy of the menu when
    the tear-off menu item is triggered. This "torn-off" copy lives in
    a separate window. It contains the same menu items as the original
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
        resize(sizeHint()+contentsMarginSize());
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
        resize(sizeHint()+contentsMarginSize());
}

bool QMenu::isCheckable() const
{
    return d->checkable;
}

/*!
    Returns the currently highlighted action, or 0 if no
    action is currently highlighted.
*/
QAction *QMenu::activeAction() const
{
    return d->currentAction ? d->currentAction->action : 0;
}

/*!
    Removes all the menu's actions.

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

  Returns the item at \a pt; returns 0 if there is no item there, or if it is
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
            return d->actionRect((*it));
    }
    return QRect();
}

/*!
    \reimp
*/
QSize QMenu::sizeHint() const
{
    ensurePolished();
    QList<QMenuAction*> actions = d->calcActionRects();
    QRect r;
    for(int i = 0; i < actions.count(); ++i)
        r = r.unite(actions[i]->rect);
    QSize s = r.size();
    if(d->tearoff)
        s.rheight() += style().pixelMetric(QStyle::PM_MenuTearoffHeight, this);
    if(const int fw = q->style().pixelMetric(QStyle::PM_MenuFrameWidth, q)) {
        s.rwidth() += fw*2;
        s.rheight() += fw*2;
    }
    s.rwidth() +=2*q->style().pixelMetric(QStyle::PM_MenuHMargin, q);
    s.rheight() += 2*q->style().pixelMetric(QStyle::PM_MenuVMargin, q);
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::Style_Default;
    return style().sizeFromContents(QStyle::CT_Menu, &opt,
                                    s.expandedTo(QApplication::globalStrut()), fontMetrics(), this);
}

/*!
    Displays the menu so that the action \a atAction will be at the
    specified \e global position \a p. To translate a widget's local
    coordinates into global coordinates, use QWidget::mapToGlobal().

    When positioning a menu with exec() or popup(), bear in mind that
    you cannot rely on the menu's current size(). For performance
    reasons, the menu adapts its size only when necessary, so in many
    cases, the size before and after the show is different. Instead,
    use sizeHint() which calculates the proper size depending on the
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
    QSize size = sizeHint() + contentsMarginSize();;
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
                    d->scroll->scrollFlags = d->scroll->scrollFlags
                                             | QMenuPrivate::QMenuScroller::ScrollUp;
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
        d->scroll->scrollFlags = d->scroll->scrollFlags | QMenuPrivate::QMenuScroller::ScrollDown;
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

    if(QApplication::isEffectEnabled(Qt::UI_AnimateMenu)) {
        if(QApplication::isEffectEnabled(Qt::UI_FadeMenu))
            qFadeEffect(this);
        else if(d->causedPopup)
            qScrollEffect(this, qt_cast<QMenu*>(d->causedPopup) ? hGuess : vGuess);
        else
            qScrollEffect(this, hGuess | vGuess);
    } else
#endif
    {
        emit aboutToShow();
        show();
    }

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuStart);
#endif
}

/*!
    Executes this menu synchronously.

    This is equivalent to \c{exec(mapToGlobal(QPoint(0,0)))}.

    This returns the triggered QAction in either the popup menu or one
    of its submenus, or 0 if no item was triggered (normally because
    the user pressed Esc).

    In most situations you'll want to specify the position yourself,
    for example, the current mouse position:
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

    Pops up the menu so that the action \a action will be at the
    specified \e global position \a p. To translate a widget's local
    coordinates into global coordinates, use QWidget::mapToGlobal().

    This returns the triggered QAction in either the popup menu or one
    of its submenus, or 0 if no item was triggered (normally because
    the user pressed Esc).

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
    use sizeHint() which calculates the proper size depending on the
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

    The menu's actions are specified by the list of \a actions. The menu
    will pop up so that the specified action, \a at, appears at global
    position \a pos. If \a at is not specified then the menu appears
    at position \a pos.

    The function returns the triggered QAction in either the popup
    menu or one of its submenus, or 0 if no item was triggered
    (normally because the user pressed Esc).

    This is equivalent to:
    \code
       QMenu menu;
       QAction *at = actions[0]; // Assumes actions is not empty
       foreach (QAction *a, actions)
          menu.addAction(a);
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
#ifdef QT_COMPAT
    emit aboutToHide();
#endif
    if(d->sync)
        qApp->exit_loop();
    d->setCurrentAction(0);
#ifndef QT_NO_ACCESSIBILITY
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
    for (int i = 0; i < d->actionItems.count(); ++i) {
        QMenuAction *action = d->actionItems.at(i);
        QRect adjustedActionRect = d->actionRect(action);
        if(!e->rect().intersects(adjustedActionRect))
           continue;
        //set the clip region to be extra safe (and adjust for the scrollers)
        QRegion adjustedActionReg(adjustedActionRect);
        emptyArea -= adjustedActionReg;
        p.setClipRegion(adjustedActionReg);

        QStyleOptionMenuItem opt = d->getStyleOption(action->action);
        opt.rect = adjustedActionRect;
        style().drawControl(QStyle::CE_MenuItem, &opt, &p, this);
    }

    QStyleOptionMenuItem menuOpt(0);
    menuOpt.palette = palette();
    menuOpt.state = QStyle::Style_Default;
    menuOpt.checkState = QStyleOptionMenuItem::NotCheckable;
    menuOpt.menuRect = rect();
    menuOpt.maxIconWidth = 0;
    menuOpt.tabWidth = 0;
    //draw the scroller regions..
    if (d->scroll) {
        const int scrollerHeight = style().pixelMetric(QStyle::PM_MenuScrollerHeight, this);
        menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
        if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
            menuOpt.rect.setRect(fw, fw, width() - (fw * 2), scrollerHeight);
            emptyArea -= QRegion(menuOpt.rect);
            p.setClipRect(menuOpt.rect);
            style().drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
        }
        if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown) {
            menuOpt.rect.setRect(fw, height() - scrollerHeight - fw, width() - (fw * 2),
                                     scrollerHeight);
            emptyArea -= QRegion(menuOpt.rect);
            menuOpt.state = QStyle::Style_Down;
            p.setClipRect(menuOpt.rect);
            style().drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
        }
    }
    //paint the tear off..
    if (d->tearoff) {
        menuOpt.menuItemType = QStyleOptionMenuItem::TearOff;
        menuOpt.rect.setRect(fw, fw, width() - (fw * 2),
                             style().pixelMetric(QStyle::PM_MenuTearoffHeight, this));
        if (d->scroll && d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
            menuOpt.rect.moveBy(0, style().pixelMetric(QStyle::PM_MenuScrollerHeight, this));
        emptyArea -= QRegion(menuOpt.rect);
        p.setClipRect(menuOpt.rect);
        menuOpt.state = QStyle::Style_Default;
        if (d->tearoffHighlighted)
            menuOpt.state |= QStyle::Style_Active;
        style().drawControl(QStyle::CE_MenuTearoff, &menuOpt, &p, this);
    }
    //draw border
    if (fw) {
        QRegion borderReg;
        borderReg += QRect(0, 0, fw, height()); //left
        borderReg += QRect(width()-fw, 0, fw, height()); //right
        borderReg += QRect(0, 0, width(), fw); //top
        borderReg += QRect(0, height()-fw, width(), fw); //bottom
        p.setClipRegion(borderReg);
        emptyArea -= borderReg;
        QStyleOptionFrame frame(0);
        frame.rect = rect();
        frame.palette = palette();
        frame.state = QStyle::Style_Default;
        frame.lineWidth = style().pixelMetric(QStyle::PM_DefaultFrameWidth);
        frame.midLineWidth = 0;
        style().drawPrimitive(QStyle::PE_MenuFrame, &frame, &p, this);
    }

    //finally the rest of the space
    p.setClipRegion(emptyArea);
    menuOpt.state = QStyle::Style_Default;
    menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
    menuOpt.checkState = QStyleOptionMenuItem::NotCheckable;
    menuOpt.rect = rect();
    menuOpt.menuRect = rect();
    style().drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
}

/*!
  \reimp
*/
void QMenu::mousePressEvent(QMouseEvent *e)
{
    if(d->mouseEventTaken(e))
        return;
    if(e->button() != Qt::LeftButton)
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
    if(e->button() != Qt::LeftButton || !d->mouseDown)
        return;
    d->mouseDown = false;
    QMenuAction *action = d->actionAt(e->pos());
    for(QWidget *caused = this; caused;) {
        if(QMenu *m = qt_cast<QMenu*>(caused)) {
            caused = m->d->causedPopup;
            if(m->d->sync)
                m->d->syncAction = action ? action->action : 0;
        } else {
            break;
        }
    }
    if(action && action->action->isEnabled()) {
        if(action->action->menu()) {
            action->action->menu()->d->setFirstActionActive();
        } else {
            d->hideUpToMenuBar();
            if(action)
                d->activateAction(action->action, QAction::Trigger);
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
            resize(sizeHint() + contentsMarginSize());
        if(!style().styleHint(QStyle::SH_Menu_Scrollable, this)) {
            delete d->scroll;
            d->scroll = 0;
        } else if(!d->scroll) {
            d->scroll = new QMenuPrivate::QMenuScroller;
            d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
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
void QMenu::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
}

/*!
  \reimp
*/
bool
QMenu::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if(ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_BackTab) {
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
        if(key == Qt::Key_Left)
            key = Qt::Key_Right;
        else if(key == Qt::Key_Right)
            key = Qt::Key_Left;
    }
    if(key == Qt::Key_Tab) //means down
        key = Qt::Key_Down;

    bool key_consumed = false;
    switch(key) {
    case Qt::Key_Up:
    case Qt::Key_Down: {
        QMenuAction *nextAction = 0;
        uint scroll_direction = QMenuPrivate::QMenuScroller::ScrollNone;
        if(!d->currentAction) {
            nextAction = d->actionItems.first();
        } else {
            for(int i=0, y=0; !nextAction && i < (int)d->actionItems.count(); i++) {
                QMenuAction *act = d->actionItems.at(i);
                if(act == d->currentAction) {
                    if(key == Qt::Key_Up) {
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

    case Qt::Key_Right:
        if(d->currentAction && d->currentAction->action->isEnabled() && d->currentAction->action->menu()) {
            d->popupAction(d->currentAction, 0, true);
            key_consumed = true;
            break;
        }
        //FALL THROUGH
    case Qt::Key_Left: {
        if(d->currentAction && !d->scroll) {
            QMenuAction *nextAction = 0;
            if(key == Qt::Key_Left) {
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
        if(!key_consumed && key == Qt::Key_Left && d->causedPopup && qt_cast<QMenu*>(d->causedPopup)) {
            hide();
            key_consumed = true;
        }
        break; }

    case Qt::Key_Alt:
        key_consumed = true;
        break;

    case Qt::Key_Escape:
        key_consumed = true;
        if(d->tornoff) {
            close();
            return;
        }
        hide();
        break;

    case Qt::Key_Space:
        if(!style().styleHint(QStyle::SH_Menu_SpaceActivatesItem, this))
            break;
        // for motif, fall through
    case Qt::Key_Return:
    case Qt::Key_Enter: {
            if(!d->currentAction)
                break;
#ifndef QT_NO_WHATSTHIS
            bool whats_this_mode = QWhatsThis::inWhatsThisMode();
#else
            const bool whats_this_mode = false;
#endif
            if(!d->currentAction->action->isEnabled() && !whats_this_mode)
                break;
            if(d->currentAction->action->menu())
                d->popupAction(d->currentAction, 20, true);
            else
                d->activateAction(d->currentAction->action, QAction::Trigger);
            key_consumed = true;
            break; }

#ifndef QT_NO_WHATSTHIS
    case Qt::Key_F1:
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
        if((!e->state() || e->state() == Qt::AltButton || e->state() == Qt::ShiftButton) && e->text().length()==1) {
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
                    key_consumed = true;
                    d->activateAction(next_action->action, QAction::Trigger);
                }
            }
        }
        if(!key_consumed) {
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
        }

#ifdef Q_OS_WIN32
        if (key_consumed && (e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift || e->key() == Qt::Key_Meta))
            qApp->beep();
#endif // Q_OS_WIN32
    }
    if(key_consumed)
        e->accept();
}

/*!
  \reimp
*/
void QMenu::mouseMoveEvent(QMouseEvent *e)
{
    if(!isVisible() || d->mouseEventTaken(e))
        return;

    QMenuAction *action = d->actionAt(e->pos());
    if(!action) {
        const int fw = q->style().pixelMetric(QStyle::PM_MenuFrameWidth, q);
        if(e->pos().x() <= fw || e->pos().x() >= width()-fw ||
           e->pos().y() <= fw || e->pos().y() >= height()-fw)
            return; //mouse over frame
    } else {
        d->mouseDown = e->state() & Qt::LeftButton;
    }
    if(d->sloppyRegion.contains(e->pos())) {
        static QTimer *sloppyDelayTimer = 0;
        if(!sloppyDelayTimer)
            sloppyDelayTimer = new QTimer(qApp);
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
    const QSize menuSize(d->activeMenu->sizeHint() + d->activeMenu->contentsMarginSize());
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
        pos.rx() = x() - menuSize.width();

    //calc sloppy focus buffer
    if(style().styleHint(QStyle::SH_Menu_SloppySubMenus, this)) {
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

/*!
    \fn void QMenu::aboutToShow()

    This signal is emitted just before the menu is shown to the user.
*/

/*!
    \fn void QMenu::activated(QAction *action)

    This signal is emitted when a menu action is triggered; \a action
    is the action that caused the signal to be emitted.

    Normally, you connect each menu action's triggered() signal to its
    own custom  slot, but sometimes you will want to connect several
    actions to a single slot, for example, when you have a group of
    closely related actions, such as "left justify", "center", "right
    justify".

    \sa highlighted(), QAction::triggered()
*/

/*!
    \fn void QMenu::highlighted(QAction *action)

    This signal is emitted when a menu action is highlighted; \a action
    is the action that caused the signal to be emitted.

    Often this is used to update status information.

    \sa activated(), QAction::hovered()
*/

#ifdef QT_COMPAT
#include "qmenudata.h"
int QMenu::insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                          const QKeySequence *shortcut, const QMenu *popup, int id, int index)
{
    QAction *act = new QAction(this);
    if(id != -1)
        static_cast<QMenuItem*>(act)->setId(id);
    if(icon)
        act->setIcon(*icon);
    if(text)
        act->setText(*text);
    if(popup)
        act->setMenu(const_cast<QMenu*>(popup));
    if(shortcut)
        act->setShortcut(*shortcut);
    if(receiver && member)
        QObject::connect(act, SIGNAL(triggered()), receiver, member);
    if(index == -1)
        addAction(act);
    else
        insertAction(actions().value(index+1), act);
    return findIdForAction(act);
}

int QMenu::insertSeparator(int index)
{
    QAction *act = new QAction(this);
    act->setSeparator(true);
    if(index == -1)
        addAction(act);
    else
        insertAction(actions().value(index+1), act);
    return findIdForAction(act);
}

QAction *QMenu::findActionForId(int id) const
{
    QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *act = list.at(i);
        if (findIdForAction(act)== id)
            return act;
    }
    return 0;
}

QMenuItem *QMenu::findPopup( QMenu *popup, int *index )
{
   QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *act = list.at(i);
        if (act->menu() == popup) {
            if (index)
                *index = static_cast<QMenuItem*>(act)->id();
            return static_cast<QMenuItem*>(act);
        }
    }
    return 0;
}


bool QMenu::setItemParameter(int id, int param)
{
    if(QAction *act = findActionForId(id)) {
        static_cast<QMenuItem*>(act)->setSignalValue(param);
        return true;
    }
    return false;
}

int QMenu::itemParameter(int id) const
{
    if(QAction *act = findActionForId(id))
        return static_cast<QMenuItem*>(act)->signalValue();
    return id;
}

int QMenu::frameWidth() const
{
    return style().pixelMetric(QStyle::PM_MenuFrameWidth, this);
}

void QMenu::compatActivated(QAction *act)
{
    emit activated(findIdForAction(act));
}

void QMenu::compatHighlighted(QAction *act)
{
    emit highlighted(findIdForAction(act));
}

int QMenu::findIdForAction(QAction *act) const
{
    if(!act)
        return -1;
    return static_cast<QMenuItem*>(act)->id();
}
#endif
