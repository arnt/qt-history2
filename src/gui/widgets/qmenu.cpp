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
#include "qdebug.h"
#include <private/qaction_p.h>
#ifdef QT3_SUPPORT
#include <qmenudata.h>

#endif // QT3_SUPPORT

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

        setParent(p, Qt::Window | Qt::Tool);
	setAttribute(Qt::WA_DeleteOnClose, true);
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
        if (act->type() == QEvent::ActionAdded) {
            insertAction(act->before(), act->action());
        } else if (act->type() == QEvent::ActionRemoved)
            removeAction(act->action());
    }
    void actionEvent(QActionEvent *e)
    {
        QMenu::actionEvent(e);
        resize(sizeHint()+contentsMarginSize());
    }
};
#include "qmenu.moc"

void QMenuPrivate::calcActionRects(QMap<QAction*, QRect> &actionRects, QList<QAction*> &actionList) const
{
    if (!itemsDirty) {
        actionRects = d->actionRects;
        actionList = d->actionList;
        return;
    }

    actionRects.clear();
    actionList.clear();
    QList<QAction*> items = q->actions();
    int max_column_width = 0, dh = QApplication::desktop()->availableGeometry(q).height(), ncols = 1, y = 0;
    const int hmargin = q->style()->pixelMetric(QStyle::PM_MenuHMargin, 0, q),
              vmargin = q->style()->pixelMetric(QStyle::PM_MenuVMargin, 0, q),
              icone = q->style()->pixelMetric(QStyle::PM_SmallIconSize, 0, q);

    //for compatability now - will have to refactor this away..
    tabWidth = 0;
    maxIconWidth = 0;
    hasCheckableItems = false;
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if (!action->isVisible())
            continue;
        hasCheckableItems |= action->isCheckable();
        QIcon is = action->icon();
        if (!is.isNull()) {
            uint miw = maxIconWidth;
            maxIconWidth = qMax<uint>(miw, icone + 4);
        }
    }

    //calculate size
    QFontMetrics qfm = q->fontMetrics();
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if (!action->isVisible())
            continue;

        QFontMetrics fm(action->font().resolve(q->font()));
        QSize sz;

        //calc what I think the size is..
        if (action->isSeparator()) {
            sz = QSize(2, 2);
        } else {
            QString s = action->text();
            int t = s.indexOf('\t');
            if (t != -1) {
                tabWidth = qMax(int(tabWidth), qfm.width(s.mid(t+1)));
                s = s.left(t);
            } else {
                QKeySequence seq = action->shortcut();
                if (!seq.isEmpty())
                    tabWidth = qMax(int(tabWidth), fm.width(seq));
            }
            int w = fm.width(s);
            w -= s.count('&') * fm.width('&');
            w += s.count("&&") * fm.width('&');
            sz.setWidth(w);
            sz.setHeight(qMax(fm.height(), qfm.height()));

            QIcon is = action->icon();
            if (!is.isNull()) {
                QSize is_sz = QSize(icone, icone);
                if (is_sz.height() > sz.height())
                    sz.setHeight(is_sz.height());
            }
        }

        //let the style modify the above size..
        QStyleOptionMenuItem opt = getStyleOption(action);
        opt.rect = q->rect();
        sz = q->style()->sizeFromContents(QStyle::CT_MenuItem, &opt, sz, q);

        if (!sz.isEmpty()) {
            max_column_width = qMax(max_column_width, sz.width());
            //wrapping
            if (!scroll &&
               y+sz.height()+vmargin > dh - (q->style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, q) * 2)) {
                ncols++;
                y = vmargin;
            }
            y += sz.height();
            //append item
            actionRects.insert(action, QRect(0, 0, sz.width(), sz.height()));
            actionList.append(action);
        }
    }
    if (tabWidth)
        max_column_width += tabWidth; //finally add in the tab width

    //calculate position
    int x = hmargin;
    y = vmargin;
    for(int i = 0; i < actionList.count(); i++) {
        QAction *action = actionList.at(i);
        QRect &rect = actionRects[action];
        if (!scroll &&
           y+rect.height() > dh - (q->style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, q) * 2)) {
            ncols--;
            if (ncols < 0)
                qWarning("QMenu: Column mismatch calculation. %d", ncols);
            x += max_column_width + hmargin;
            y = vmargin;
        }
        rect.translate(x, y);                        //move
        rect.setWidth(max_column_width); //uniform width
        y += rect.height();
    }
}

void QMenuPrivate::updateActions()
{
    if (!itemsDirty)
        return;
    sloppyAction = 0;
    calcActionRects(actionRects, actionList);
    ncols = 1;
    int last_left = q->style()->pixelMetric(QStyle::PM_MenuVMargin, 0, q);
    if (!scroll) {
        for(int i = 0; i < actionList.count(); i++) {
            int left = actionRects.value(actionList.at(i)).left();
            if (left > last_left) {
                last_left = left;
                ncols++;
            }
        }
    }
    itemsDirty = 0;
}

QRect QMenuPrivate::actionRect(QAction *act) const
{
    QRect ret = actionRects.value(act);
    if (scroll)
        ret.translate(0, scroll->scrollOffset);
    if (tearoff)
        ret.translate(0, q->style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, q));
    const int fw = q->style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, q);
    ret.translate(fw+leftmargin, fw+topmargin);
    return ret;
}

void QMenuPrivate::hideUpToMenuBar()
{
    if (!tornoff) {
        QWidget *caused = causedPopup;
        q->hide(); //hide after getting causedPopup
        while(caused) {
            if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
                mb->d->setCurrentAction(0);
                caused = 0;
            } else if (QMenu *m = qobject_cast<QMenu*>(caused)) {
                caused = m->d->causedPopup;
                if (!m->d->tornoff)
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

void QMenuPrivate::popupAction(QAction *action, int delay, bool activateFirst)
{
    if (action && action->menu() && action->isEnabled() && action->menu()->isEnabled()) {
        if (!delay) {
            q->internalDelayedPopup();
        } else {
            static QTimer *menuDelayTimer = 0;
            if (!menuDelayTimer)
                menuDelayTimer = new QTimer(qApp);
            menuDelayTimer->disconnect(SIGNAL(timeout()));
            QObject::connect(menuDelayTimer, SIGNAL(timeout()),
                             q, SLOT(internalDelayedPopup()));
            menuDelayTimer->setSingleShot(true);
            menuDelayTimer->start(delay);
        }
        if (activateFirst)
            action->menu()->d->setFirstActionActive();
    }
}

void QMenuPrivate::setFirstActionActive()
{
    const int scrollerHeight = q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q);
    for(int i = 0, saccum = 0; i < actionList.count(); i++) {
        QAction *act = actionList[i];
        if (scroll && scroll->scrollFlags & QMenuScroller::ScrollUp) {
            saccum -= actionRects.value(act).height();
            if (saccum > scroll->scrollOffset-scrollerHeight)
                continue;
        }
        if (!act->isSeparator() &&
           (q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)
            || act->isEnabled())) {
            setCurrentAction(act);
            break;
        }
    }
}

// popup == -1 means do not popup, 0 means immediately, others mean use a timer
void QMenuPrivate::setCurrentAction(QAction *action, int popup, bool activateFirst)
{
    d->tearoffHighlighted = 0;
    if (action == currentAction)
        return;
    if (currentAction)
        q->update(actionRect(currentAction));

    d->sloppyAction = 0;
    if (!sloppyRegion.isEmpty())
        sloppyRegion = QRegion();
    currentAction = action;
    if (action && !action->isSeparator()) {
        activateAction(action, QAction::Hover);
        if (popup != -1)
            popupAction(d->currentAction, popup, activateFirst);
        q->update(actionRect(action));
    }
    if (activeMenu && (!action || !action->menu())) { //otherwise done in popupAction
        QMenu *menu = activeMenu;
        activeMenu = 0;
        menu->hide();
    }
}

QAction *QMenuPrivate::actionAt(QPoint p) const
{
    if (!q->rect().contains(p))     //sanity check
       return 0;

    for(int i = 0; i < actionList.count(); i++) {
        QAction *act = actionList[i];
        if (actionRect(act).contains(p))
            return act;
    }
    return 0;
}


/*!
    Returns the action associated with this menu.
*/
QAction *QMenu::menuAction() const
{
    return d->menuAction;
}

/*!
  \property QMenu::title

  \brief The title of the menu

  This is equivalent to the QAction::text property of the menuAction().
*/
QString QMenu::title() const
{
    return d->menuAction->text();
}

void QMenu::setTitle(const QString &text)
{
    d->menuAction->setText(text);
}

/*!
  \property QMenu::icon

  \brief The icon of the menu

  This is equivalent to the QAction::icon property of the menuAction().
*/
QIcon QMenu::icon() const
{
    return d->menuAction->icon();
}

void QMenu::setIcon(const QIcon &icon)
{
    d->menuAction->setIcon(icon);
}


//actually performs the scrolling
void QMenuPrivate::scrollMenu(QAction *action, QMenuScroller::ScrollLocation location, bool active)
{
    int newOffset = 0;
    const int scrollHeight = q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q);
    const int topScroll = (scroll->scrollFlags & QMenuScroller::ScrollUp)   ? scrollHeight : 0;
    const int botScroll = (scroll->scrollFlags & QMenuScroller::ScrollDown) ? scrollHeight : 0;
    if (location == QMenuScroller::ScrollTop) {
        for(int i = 0, saccum = 0; i < actionList.count(); i++) {
            QAction *act = actionList.at(i);
            if (act == action) {
                newOffset = topScroll - saccum;
                break;
            }
            saccum += actionRects.value(act).height();
        }
    } else {
        for(int i = 0, saccum = 0; i < actionList.count(); i++) {
            QAction *act = actionList.at(i);
            saccum += actionRects.value(act).height();
            if (act == action) {
                if (location == QMenuScroller::ScrollCenter)
                    newOffset = ((q->height() / 2) - botScroll) - saccum;
                else
                    newOffset = (q->height() - botScroll) - saccum;
                break;
            }
        }
    }

    //figure out which scroll flags
    uint newScrollFlags = QMenuScroller::ScrollNone;
    if (newOffset < 0) //easy and cheap one
        newScrollFlags |= QMenuScroller::ScrollUp;
    for(int i = 0, saccum=newOffset+topScroll; i < actionList.count(); i++) {
        saccum += actionRects.value(actionList.at(i)).height();
        if (saccum > q->height()) {
            newScrollFlags |= QMenuScroller::ScrollDown;
            break;
        }
    }

    if (location == QMenuScroller::ScrollTop && ((newScrollFlags & QMenuScroller::ScrollUp)
                                                 || !(scroll->scrollFlags & QMenuScroller::ScrollUp)))
        newOffset += scrollHeight;
    else if (location == QMenuScroller::ScrollBottom && ((newScrollFlags & QMenuScroller::ScrollDown)
                                                         || !(scroll->scrollFlags & QMenuScroller::ScrollDown)))
        newOffset -= scrollHeight;

    QRect screen = QApplication::desktop()->availableGeometry(q);
    const int desktopFrame = q->style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, q);
    if (q->height() < screen.height()-(desktopFrame*2)-1) {
        QRect geom = q->geometry();
        if (newOffset > d->scroll->scrollOffset) { //scroll up
            geom.setHeight(geom.height()-(newOffset-scroll->scrollOffset));
        } else {
            geom.setTop(geom.top() + (newOffset-scroll->scrollOffset));
            if (geom != q->geometry()) {
                newOffset = 0;
                newScrollFlags &= ~QMenuScroller::ScrollUp;
            }
        }
        if (geom.bottom() > screen.bottom() - desktopFrame)
            geom.setBottom(screen.bottom() - desktopFrame);
        if (geom.top() < desktopFrame+screen.top())
            geom.setTop(desktopFrame+screen.top());
        if (geom != q->geometry()) {
#if 0
            if (newScrollFlags & QMenuScroller::ScrollDown &&
               q->geometry().top() - geom.top() >= -newOffset)
                newScrollFlags &= ~QMenuScroller::ScrollDown;
#endif
            q->setGeometry(geom);
        }
    }

    //actually update flags
    scroll->scrollOffset = newOffset;
    if (scroll->scrollOffset > 0)
        scroll->scrollOffset = 0;
    scroll->scrollFlags = newScrollFlags;
    if (active)
        setCurrentAction(action);

    q->update();     //issue an update so we see all the new state..
}

//only directional
void QMenuPrivate::scrollMenu(QMenuScroller::ScrollDirection direction, bool page, bool active)
{
    if (!scroll || !(scroll->scrollFlags & direction)) //not really possible...
        return;
    const int scrollHeight = q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q);
    const int topScroll = (scroll->scrollFlags & QMenuScroller::ScrollUp)   ? scrollHeight : 0;
    const int botScroll = (scroll->scrollFlags & QMenuScroller::ScrollDown) ? scrollHeight : 0;
    if (direction == QMenuScroller::ScrollUp) {
        for(int i = 0, saccum = 0; i < actionList.count(); i++) {
            QAction *act = actionList.at(i);
            const int iHeight = actionRects.value(act).height();
            saccum -= iHeight;
            if (saccum <= scroll->scrollOffset-topScroll) {
                scrollMenu(act, page ? QMenuScroller::ScrollBottom : QMenuScroller::ScrollTop, active);
                break;
            }
        }
    } else if (direction == QMenuScroller::ScrollDown) {
        for(int i = 0, saccum = 0; i < actionList.count(); i++) {
            QAction *act = actionList.at(i);
            const int iHeight = actionRects.value(act).height();
            if (saccum <= scroll->scrollOffset-topScroll) {
                const int scrollerArea = q->height() - botScroll;
                int visible = -(((scroll->scrollOffset-topScroll) - saccum) - iHeight);
                for(i++ ; i < actionList.count(); i++) {
                    act = actionList.at(i);
                    const int iHeight = actionRects.value(act).height();
                    visible += iHeight;
                    if (visible > scrollerArea-topScroll) {
                        scrollMenu(act, page ? QMenuScroller::ScrollTop : QMenuScroller::ScrollBottom, active);
                        break;
                    }
                }
                break;
            }
            saccum -= iHeight;
        }
    }
}

/* This is poor-mans eventfilters. This avoids the use of
   eventFilter (which can be nasty for users of QMenuBar's). */
bool QMenuPrivate::mouseEventTaken(QMouseEvent *e)
{
    QPoint pos = q->mapFromGlobal(e->globalPos());
    if (scroll && !activeMenu) { //let the scroller "steal" the event
        bool isScroll = false;
        if (pos.x() >= 0 && pos.x() < q->width()) {
            const int scrollerHeight = q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q);
            for(int dir = QMenuScroller::ScrollUp; dir <= QMenuScroller::ScrollDown; dir = dir << 1) {
                if (scroll->scrollFlags & dir) {
                    if (dir == QMenuScroller::ScrollUp)
                        isScroll = (pos.y() <= scrollerHeight);
                    else if (dir == QMenuScroller::ScrollDown)
                        isScroll = (pos.y() >= q->height()-scrollerHeight);
                    if (isScroll) {
                        scroll->scrollDirection = dir;
                        break;
                    }
                }
            }
        }
        if (isScroll) {
            if (!scroll->scrollTimer)
                scroll->scrollTimer = new QBasicTimer;
            scroll->scrollTimer->start(50, q);
            return true;
        } else if (scroll->scrollTimer && scroll->scrollTimer->isActive()) {
            scroll->scrollTimer->stop();
        }
    }

    if (tearoff) { //let the tear off thingie "steal" the event..
        QRect tearRect(0, 0, q->width(), q->style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, q));
        if (scroll && scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
            tearRect.translate(0, q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q));
        q->update(tearRect);
        if (tearRect.contains(pos)) {
            setCurrentAction(0);
            tearoffHighlighted = 1;
            if (e->type() == QEvent::MouseButtonRelease) {
                if (tornPopup) {
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

    if (q->frameGeometry().contains(e->globalPos())) //otherwise if the event is in our rect we want it..
        return false;

    for(QWidget *caused = causedPopup; caused;) {
        bool passOnEvent = false;
        QWidget *next_widget = 0;
        QPoint cpos = caused->mapFromGlobal(e->globalPos());
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
            passOnEvent = mb->rect().contains(cpos);
        } else if (QMenu *m = qobject_cast<QMenu*>(caused)) {
            passOnEvent = m->d->actionAt(cpos);
            next_widget = m->d->causedPopup;
        }
        if (passOnEvent) {
            QMouseEvent new_e(e->type(), cpos, e->button(), e->buttons(), e->modifiers());
            QApplication::sendEvent(caused, &new_e);
            return true;
        }
        if (!next_widget)
            break;
        caused = next_widget;
    }
    return false;
}

void QMenuPrivate::activateAction(QAction *action, QAction::ActionEvent action_e)
{
    bool inWhatsThisMode = QWhatsThis::inWhatsThisMode();
    if (!action || !q->isEnabled()
        || (action_e == QAction::Trigger && !inWhatsThisMode && !action->isEnabled()))
        return;

    /* I have to save the caused stack here because it will be undone after popup execution (ie in the hide).
       Then I iterate over the list to actually send the events. --Sam
    */
    QList<QPointer<QWidget> > causedStack;
    for(QWidget *widget = causedPopup; widget; ) {
        causedStack.append(widget);
        if (QMenu *qmenu = ::qobject_cast<QMenu*>(widget))
            widget = qmenu->d->causedPopup;
        else
            break;
    }
    if (action_e == QAction::Trigger) {
        hideUpToMenuBar();
        if (inWhatsThisMode) {
            QString s = action->whatsThis();
            if (s.isEmpty())
                s = whatsThis;
            QWhatsThis::showText(q->mapToGlobal(actionRect(action).center()), s, q);
            return;
        }
    }

    action->activate(action_e);

    for(int i = 0; i < causedStack.size(); ++i) {
        QPointer<QWidget> widget = causedStack.at(i);
        if (!widget)
            continue;
        //fire
        if (QMenu *qmenu = ::qobject_cast<QMenu*>(widget)) {
            widget = qmenu->d->causedPopup;
            if (action_e == QAction::Trigger) {
                emit qmenu->triggered(action);
#ifdef QT3_SUPPORT
                emit qmenu->activated(qmenu->findIdForAction(action));
#endif
            } else if (action_e == QAction::Hover) {
                emit qmenu->hovered(action);
#ifdef QT3_SUPPORT
                emit qmenu->highlighted(qmenu->findIdForAction(action));
#endif
            }
        } else if (QMenuBar *qmenubar = ::qobject_cast<QMenuBar*>(widget)) {
            if (action_e == QAction::Trigger) {
                emit qmenubar->triggered(action);
#ifdef QT3_SUPPORT
                emit qmenubar->activated(qmenu->findIdForAction(action));
#endif
            } else if (action_e == QAction::Hover) {
                emit qmenubar->hovered(action);
#ifdef QT3_SUPPORT
                emit qmenubar->highlighted(qmenu->findIdForAction(action));
#endif
            }
            break; //nothing more..
        }
    }

    if (action_e == QAction::Hover) {
#ifndef QT_NO_ACCESSIBILITY
        int actionID = indexOf(action);
        QAccessible::updateAccessibility(q, actionID, QAccessible::Focus);
        QAccessible::updateAccessibility(q, actionID, QAccessible::Selection);
#endif
        QWidget *w = d->causedPopup;
        while (QMenu *m = qobject_cast<QMenu*>(w))
            w = m->d->causedPopup;
        action->showStatusText(w);
    }
}

void QMenuPrivate::actionTriggered()
{
    if (QAction *action = qobject_cast<QAction *>(q->sender())) {
        emit q->triggered(action);
#ifdef QT3_SUPPORT
        emit q->activated(q->findIdForAction(action));
#endif
    }
}

void QMenuPrivate::actionHovered()
{
    if (QAction *action = qobject_cast<QAction *>(q->sender())) {
        emit q->hovered(action);
#ifdef QT3_SUPPORT
        emit q->highlighted(q->findIdForAction(action));
#endif
    }
}

QStyleOptionMenuItem QMenuPrivate::getStyleOption(const QAction *action) const
{
    QStyleOptionMenuItem opt;
    opt.palette = q->palette();
    opt.state = QStyle::State_None;

    if (q->window()->isActiveWindow())
        opt.state |= QStyle::State_Active;
    if (q->isEnabled() && action->isEnabled()
            && (!action->menu() || action->menu()->isEnabled()))
        opt.state |= QStyle::State_Enabled;
    else
        opt.palette.setCurrentColorGroup(QPalette::Disabled);

    opt.font = action->font();

    if (currentAction && currentAction == action)
        opt.state |= QStyle::State_Selected;
    if (mouseDown)
        opt.state |= QStyle::State_Down;
    opt.menuHasCheckableItems = hasCheckableItems;
    if (!action->isCheckable()) {
        opt.checkType = QStyleOptionMenuItem::NotCheckable;
    } else {
        opt.checkType = (action->actionGroup() && action->actionGroup()->isExclusive())
                            ? QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
        opt.checked = action->isChecked();
    }
    if (action->menu())
        opt.menuItemType = QStyleOptionMenuItem::SubMenu;
    else if (action->isSeparator())
        opt.menuItemType = QStyleOptionMenuItem::Separator;
    else if (defaultAction == action)
	    opt.menuItemType = QStyleOptionMenuItem::DefaultItem;
    else
        opt.menuItemType = QStyleOptionMenuItem::Normal;
    opt.icon = action->icon();
    QString textAndAccel = action->text();
    if (textAndAccel.indexOf('\t') == -1) {
        QKeySequence seq = action->shortcut();
        if (!seq.isEmpty())
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

    \sa QMenuBar \link guibooks.html#fowler GUI Design Handbook: Menu, Drop-Down and
    Pop-Up\endlink
*/


/*!
    Constructs a menu with parent \a parent.

    Although a popup menu is always a top-level widget, if a parent is
    passed the popup menu will be deleted when that parent is
    destroyed (as with any other QObject).
*/
QMenu::QMenu(QWidget *parent)
    : QWidget(*new QMenuPrivate, parent, Qt::Popup)
{
    setAttribute(Qt::WA_CustomWhatsThis);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(style()->styleHint(QStyle::SH_Menu_MouseTracking, 0, this));
    if (style()->styleHint(QStyle::SH_Menu_Scrollable, 0, this)) {
        d->scroll = new QMenuPrivate::QMenuScroller;
        d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
    d->menuAction = new QAction(this);
    d->menuAction->d->menu = this;
}

/*!
    Constructs a menu with a \a title and a \a parent.

    Although a popup menu is always a top-level widget, if a parent is
    passed the popup menu will be deleted when that parent is
    destroyed (as with any other QObject).

    \sa title
*/
QMenu::QMenu(const QString &title, QWidget *parent)
    : QWidget(*new QMenuPrivate, parent, Qt::Popup)
{
    setAttribute(Qt::WA_CustomWhatsThis);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(style()->styleHint(QStyle::SH_Menu_MouseTracking));
    if (style()->styleHint(QStyle::SH_Menu_Scrollable, 0, this)) {
        d->scroll = new QMenuPrivate::QMenuScroller;
        d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
    d->menuAction = new QAction(title, this);
    d->menuAction->d->menu = this;
}

/*!
    Destroys the menu.
*/
QMenu::~QMenu()
{
    if (d->eventLoop)
        d->eventLoop->exit();
    if (d->tornPopup)
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
QAction *QMenu::addAction(const QIcon &icon, const QString &text)
{
    QAction *ret = new QAction(icon, text, this);
    addAction(ret);
    return ret;
}

/*!
    \overload

    This convenience function creates a new action with the text \a
    text and an optional shortcut \a shortcut. The action's
    triggered() signal is connected to the \a receiver's \a member
    slot. The function adds the newly created action to the menu's
    list of actions and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QString &text, const QObject *receiver, const char* member, const QKeySequence &shortcut)
{
    QAction *action = new QAction(text, this);
    action->setShortcut(shortcut);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

/*!
    \overload

    This convenience function creates a new action with an \a icon and
    some \a text and an optional shortcut \a shortcut. The action's
    triggered() signal is connected to the \a member slot of the \a
    receiver object. The function adds the newly created action to the
    menu's list of actions, and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QIcon &icon, const QString &text, const QObject *receiver,
                          const char* member, const QKeySequence &shortcut)
{
    QAction *action = new QAction(icon, text, this);
    action->setShortcut(shortcut);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

/*!
    This convenience function adds \a menu as a submenu to this menu.
    It returns the menus menuAction().
    \sa QWidget::addAction() QMenu::menuAction()
*/
QAction *QMenu::addMenu(QMenu *menu)
{
    QAction *action = menu->menuAction();
    addAction(action);
    return action;
}

/*!
  Appends a new QMenu with \a title to the menu. The menu
  takes ownership of the menu. Returns the new menu.

  \sa QWidget::addAction() QMenu::menuAction()
*/
QMenu *QMenu::addMenu(const QString &title)
{
    QMenu *menu = new QMenu(title, this);
    addAction(menu->menuAction());
    return menu;
}

/*!
  Appends a new QMenu with \a icon and \a title to the menu. The menu
  takes ownership of the menu. Returns the new menu.

  \sa QWidget::addAction() QMenu::menuAction()
*/
QMenu *QMenu::addMenu(const QIcon &icon, const QString &title)
{
    QMenu *menu = new QMenu(title, this);
    menu->setIcon(icon);
    addAction(menu->menuAction());
    return menu;
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
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

/*!
    This convenience function inserts \a menu before action \a before
    and returns the menus menuAction().

    \sa QWidget::insertAction() addMenu()
*/
QAction *QMenu::insertMenu(QAction *before, QMenu *menu)
{
    QAction *action = menu->menuAction();
    insertAction(before, action);
    return action;
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
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
    return action;
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
    if (d->tearoff == b)
        return;
    if (!b && d->tornPopup)
        d->tornPopup->close();
    d->tearoff = b;

    d->itemsDirty = true;
    if (isVisible())
        resize(sizeHint()+contentsMarginSize());
}

bool QMenu::isTearOffEnabled() const
{
    return d->tearoff;
}

/*!
  When a menu is torn off a second menu is shown to display the menu
  contents in a new window. When the menu is in this mode and the menu
  is visible returns true; otherwise false.

  \sa hideTearOffMenu() isTearOffEnabled()
*/
bool QMenu::isTearOffMenuVisible() const
{
    if (d->tornPopup)
        return d->tornPopup->isVisible();
    return false;
}

/*!
   This function will forcibly hide the torn off menu making it
   disappear from the users desktop.

   \sa isTearOffMenuVisible() isTearOffEnabled()
*/
void QMenu::hideTearOffMenu()
{
    if (d->tornPopup)
        d->tornPopup->close();
}


/*!
  Sets the currently highlighte daction to \a act.
*/
void QMenu::setActiveAction(QAction *act)
{
    d->setCurrentAction(act);
    if (d->scroll)
        d->scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollCenter);
}


/*!
    Returns the currently highlighted action, or 0 if no
    action is currently highlighted.
*/
QAction *QMenu::activeAction() const
{
    return d->currentAction;
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
    return d->ncols;
}

/*!
  \internal

  Returns the item at \a pt; returns 0 if there is no item there.
*/
QAction *QMenu::actionAt(const QPoint &pt) const
{
    if (QAction *ret = d->actionAt(pt))
        return ret;
    return 0;
}

/*!
  \internal

  Returns the geometry of action \a act.
*/
QRect QMenu::actionGeometry(QAction *act) const
{
    return d->actionRect(act);
}

/*!
    \reimp
*/
QSize QMenu::sizeHint() const
{
    ensurePolished();
    QMap<QAction*, QRect> actionRects;
    QList<QAction*> actionList;
    d->calcActionRects(actionRects, actionList);

    QSize s;
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    for (QMap<QAction*, QRect>::const_iterator i = actionRects.begin();
         i != actionRects.constEnd(); ++i) {
        if (i.value().bottom() > s.height())
            s.setHeight(i.value().y()+i.value().height());
        if (i.value().right() > s.width())
            s.setWidth(i.value().right());
    }
    if (d->tearoff)
        s.rheight() += style()->pixelMetric(QStyle::PM_MenuTearoffHeight, &opt, this);
    if (const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, &opt, this)) {
        s.rwidth() += fw*2;
        s.rheight() += fw*2;
    }
    s.rwidth() += 2 * style()->pixelMetric(QStyle::PM_MenuHMargin, &opt, this);
    s.rheight() += 2 * style()->pixelMetric(QStyle::PM_MenuVMargin, &opt, this);

    return style()->sizeFromContents(QStyle::CT_Menu, &opt,
                                    s.expandedTo(QApplication::globalStrut()), this);
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
    if (d->scroll) { //reset scroll state from last popup
        d->scroll->scrollOffset = 0;
        d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
    d->tearoffHighlighted = 0;
    d->motions = 0;

    d->updateActions();
    QPoint pos = p;
    QSize size = sizeHint() + contentsMarginSize();
    QRect screen = QApplication::desktop()->availableGeometry(p);
    const int desktopFrame = style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, this);
    if (d->ncols != 1) {
        pos.setY(screen.top()+desktopFrame);
    } else if (atAction) {
        for(int i=0, above_height=0; i<(int)d->actionList.count(); i++) {
            QAction *action = d->actionList.at(i);
            if (action == atAction) {
                int newY = pos.y()-above_height;
                if (d->scroll && newY < desktopFrame) {
                    d->scroll->scrollFlags = d->scroll->scrollFlags
                                             | QMenuPrivate::QMenuScroller::ScrollUp;
                    d->scroll->scrollOffset = newY;
                    newY = desktopFrame;
                }
                pos.setY(newY);

                if (!style()->styleHint(QStyle::SH_Menu_FillScreenWithScroll, 0, this)) {
                    int below_height = above_height + d->scroll->scrollOffset;
                    for(int i2 = i; i2 < (int)d->actionList.count(); i2++)
                        below_height += d->actionRects.value(d->actionList.at(i2)).height();
                    size.setHeight(below_height);
                }
                break;
            } else {
                above_height += d->actionRects.value(action).height();
            }
        }
    }

    QPoint mouse = QCursor::pos();
    bool snapToMouse = (p == mouse);
    if (snapToMouse) {
        if (qApp->layoutDirection() == Qt::RightToLeft)
            pos.setX(pos.x()-size.width());
        if (pos.x()+size.width() > screen.right())
            pos.setX(mouse.x()-size.width());
        if (pos.y()+size.height() > screen.bottom() - (desktopFrame * 2))
            pos.setY(mouse.y()-size.height());
        if (pos.x() < screen.left())
            pos.setX(mouse.x());
        if (pos.y() < screen.top() + desktopFrame)
            pos.setY(screen.top() + desktopFrame);
    }
    if (d->scroll && pos.y()+size.height() > screen.height()-(desktopFrame*2)) {
        d->scroll->scrollFlags |= uint(QMenuPrivate::QMenuScroller::ScrollDown);
        size.setHeight(screen.bottom()-desktopFrame-pos.y());
    } else {
        if(pos.y()+size.height() > screen.height())
            pos.setY(desktopFrame+screen.top()+(screen.height()-desktopFrame-size.height()));
        else if(pos.y() < screen.top())
            pos.setY(screen.top());
    }
    setGeometry(QRect(pos, size));

#ifndef QT_NO_EFFECTS
    int hGuess = qApp->layoutDirection() == Qt::RightToLeft ? QEffects::LeftScroll : QEffects::RightScroll;
    int vGuess = QEffects::DownScroll;
    if (qApp->layoutDirection() == Qt::RightToLeft) {
        if ((snapToMouse && (pos.x() + size.width()/2 > mouse.x())) ||
           (qobject_cast<QMenu*>(d->causedPopup) && pos.x() + size.width()/2 > d->causedPopup->x()))
            hGuess = QEffects::RightScroll;
    } else {
        if ((snapToMouse && (pos.x() + size.width()/2 < mouse.x())) ||
           (qobject_cast<QMenu*>(d->causedPopup) && pos.x() + size.width()/2 < d->causedPopup->x()))
            hGuess = QEffects::LeftScroll;
    }

#ifndef QT_NO_MENUBAR
    if ((snapToMouse && (pos.y() + size.height()/2 < mouse.y())) ||
       (qobject_cast<QMenuBar*>(d->causedPopup) && pos.y() + size.width()/2 < d->causedPopup->mapToGlobal(d->causedPopup->pos()).y()))
       vGuess = QEffects::UpScroll;
#endif

    if (QApplication::isEffectEnabled(Qt::UI_AnimateMenu)) {
        emit aboutToShow();
        if (QApplication::isEffectEnabled(Qt::UI_FadeMenu))
            qFadeEffect(this);
        else if (d->causedPopup)
            qScrollEffect(this, qobject_cast<QMenu*>(d->causedPopup) ? hGuess : vGuess);
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

    This is equivalent to \c{exec(pos())}.

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
    or in reaction to a QMouseEvent *e:
    \code
      exec(e->globalPos());
    \endcode
*/
QAction *QMenu::exec()
{
    return exec(pos());
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
    or in reaction to a QMouseEvent *e:
    \code
      exec(e->globalPos());
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
    QEventLoop eventLoop;
    d->eventLoop = &eventLoop;
    popup(p, action);
    (void) eventLoop.exec();

    action = d->syncAction;
    d->syncAction = 0;
    d->eventLoop = 0;
    return action;
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
#ifdef QT3_SUPPORT
    emit aboutToHide();
#endif
    if (d->eventLoop)
        d->eventLoop->exit();
    d->setCurrentAction(0);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuEnd);
#endif
    if (QMenuBar *mb = qobject_cast<QMenuBar*>(d->causedPopup))
        mb->d->setCurrentAction(0);
    d->mouseDown = false;
    d->hasHadMouse = false;
    d->causedPopup = 0;
}

/*!
  \reimp
*/
void QMenu::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QRegion emptyArea = QRegion(rect());

    //draw the items that need updating..
    for (int i = 0; i < d->actionList.count(); ++i) {
        QAction *action = d->actionList.at(i);
        QRect adjustedActionRect = d->actionRect(action);
        if (!e->rect().intersects(adjustedActionRect))
           continue;
        //set the clip region to be extra safe (and adjust for the scrollers)
        QRegion adjustedActionReg(adjustedActionRect);
        emptyArea -= adjustedActionReg;
        p.setClipRegion(adjustedActionReg);

        QStyleOptionMenuItem opt = d->getStyleOption(action);
        opt.rect = adjustedActionRect;
        style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
    }

    const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
    QStyleOptionMenuItem menuOpt;
    menuOpt.palette = palette();
    menuOpt.state = QStyle::State_None;
    menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
    menuOpt.menuRect = rect();
    menuOpt.maxIconWidth = 0;
    menuOpt.tabWidth = 0;
    //draw the scroller regions..
    if (d->scroll) {
        const int scrollerHeight = style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, this);
        menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
        if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp) {
            menuOpt.rect.setRect(fw, fw, width() - (fw * 2), scrollerHeight);
            emptyArea -= QRegion(menuOpt.rect);
            p.setClipRect(menuOpt.rect);
            style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
        }
        if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown) {
            menuOpt.rect.setRect(fw, height() - scrollerHeight - fw, width() - (fw * 2),
                                     scrollerHeight);
            emptyArea -= QRegion(menuOpt.rect);
            menuOpt.state = QStyle::State_Down;
            p.setClipRect(menuOpt.rect);
            style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p, this);
        }
    }
    //paint the tear off..
    if (d->tearoff) {
        menuOpt.menuItemType = QStyleOptionMenuItem::TearOff;
        menuOpt.rect.setRect(fw, fw, width() - (fw * 2),
                             style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this));
        if (d->scroll && d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
            menuOpt.rect.translate(0, style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, this));
        emptyArea -= QRegion(menuOpt.rect);
        p.setClipRect(menuOpt.rect);
        menuOpt.state = QStyle::State_None;
        if (d->tearoffHighlighted)
            menuOpt.state |= QStyle::State_Selected;
        style()->drawControl(QStyle::CE_MenuTearoff, &menuOpt, &p, this);
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
        QStyleOptionFrame frame;
        frame.rect = rect();
        frame.palette = palette();
        frame.state = QStyle::State_None;
        frame.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        frame.midLineWidth = 0;
        style()->drawPrimitive(QStyle::PE_FrameMenu, &frame, &p, this);
    }

    //finally the rest of the space
    p.setClipRegion(emptyArea);
    menuOpt.state = QStyle::State_None;
    menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
    menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
    menuOpt.rect = rect();
    menuOpt.menuRect = rect();
    style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
}

#ifndef QT_NO_WHEELEVENT
/*!
  \reimp
*/
void QMenu::wheelEvent(QWheelEvent *e)
{
    if (d->scroll && rect().contains(e->pos()))
        d->scrollMenu(e->delta() > 0 ?
                      QMenuPrivate::QMenuScroller::ScrollUp : QMenuPrivate::QMenuScroller::ScrollDown);
}
#endif

/*!
  \reimp
*/
void QMenu::mousePressEvent(QMouseEvent *e)
{
    if (d->mouseEventTaken(e))
        return;
    if (!rect().contains(e->pos())) {
         if (d->noReplayFor
             && QRect(d->noReplayFor->mapToGlobal(QPoint()), d->noReplayFor->size()).contains(e->globalPos()))
             setAttribute(Qt::WA_NoMouseReplay);
        d->hideUpToMenuBar();
        return;
    }
    d->mouseDown = true;

    QAction *action = d->actionAt(e->pos());
    d->setCurrentAction(action, 20);
}

/*!
  \reimp
*/
void QMenu::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->mouseEventTaken(e))
        return;

    d->mouseDown = false;
    QAction *action = d->actionAt(e->pos());
    for(QWidget *caused = this; caused;) {
        if (QMenu *m = qobject_cast<QMenu*>(caused)) {
            caused = m->d->causedPopup;
            if (m->d->eventLoop && (!action || action->isEnabled())) // synchronous operation
                m->d->syncAction = action;
        } else {
            break;
        }
    }
    if (action && action == d->currentAction) {
        if (action->menu())
            action->menu()->d->setFirstActionActive();
        else
            d->activateAction(action, QAction::Trigger);
    } else if (d->motions > 6) {
        d->hideUpToMenuBar();
    }
}

/*!
  \reimp
*/
void QMenu::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange) {
        d->itemsDirty = 1;
        setMouseTracking(style()->styleHint(QStyle::SH_Menu_MouseTracking, 0, this));
        if (isVisible())
            resize(sizeHint() + contentsMarginSize());
        if (!style()->styleHint(QStyle::SH_Menu_Scrollable, 0, this)) {
            delete d->scroll;
            d->scroll = 0;
        } else if (!d->scroll) {
            d->scroll = new QMenuPrivate::QMenuScroller;
            d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
        }
    } else if (e->type() == QEvent::EnabledChange) {
        if (d->tornPopup) // torn-off menu
            d->tornPopup->setEnabled(isEnabled());
        d->menuAction->setEnabled(isEnabled());
    }
    QWidget::changeEvent(e);
}


/*!
  \reimp
*/
bool
QMenu::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            keyPressEvent(ke);
            return true;
        }
    } break;
    case QEvent::Resize:
        d->itemsDirty = 1;
        d->updateActions();
        break;
    case QEvent::Show:
        d->updateActions();
        break;
    case QEvent::QueryWhatsThis:
        e->setAccepted(d->whatsThis.size());
        if (QAction *action = d->actionAt(static_cast<QHelpEvent*>(e)->pos())) {
            if (action->whatsThis().size() || action->menu())
                e->accept();
        }
        return true;
    default:
        break;
    }
    return QWidget::event(e);
}

/*!
  \reimp
*/
void QMenu::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if (isRightToLeft()) {  // in reverse mode open/close key for submenues are reversed
        if (key == Qt::Key_Left)
            key = Qt::Key_Right;
        else if (key == Qt::Key_Right)
            key = Qt::Key_Left;
    }
    if (key == Qt::Key_Tab) //means down
        key = Qt::Key_Down;

    bool key_consumed = false;
    switch(key) {
    case Qt::Key_Home:
        key_consumed = true;
        if (d->currentAction && d->scroll)
            d->scrollMenu(d->actionList.first(), QMenuPrivate::QMenuScroller::ScrollTop, true);
        break;
    case Qt::Key_End:
        key_consumed = true;
        if (d->currentAction && d->scroll)
            d->scrollMenu(d->actionList.last(), QMenuPrivate::QMenuScroller::ScrollBottom, true);
        break;
    case Qt::Key_PageUp:
        key_consumed = true;
        if (d->currentAction && d->scroll)
            d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollUp, true, true);
        break;
    case Qt::Key_PageDown:
        key_consumed = true;
        if (d->currentAction && d->scroll)
            d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollDown, true, true);
        break;
    case Qt::Key_Up:
    case Qt::Key_Down: {
        key_consumed = true;
        QAction *nextAction = 0;
        QMenuPrivate::QMenuScroller::ScrollLocation scroll_loc = QMenuPrivate::QMenuScroller::ScrollStay;
        if (!d->currentAction) {
            nextAction = d->actionList.first();
        } else {
            for(int i=0, y=0; !nextAction && i < (int)d->actionList.count(); i++) {
                QAction *act = d->actionList.at(i);
                if (act == d->currentAction) {
                    if (key == Qt::Key_Up) {
                        for(int next_i = i-1; true; next_i--) {
                            if (next_i == -1) {
                                if (d->scroll)
                                    break;
                                next_i = d->actionList.count()-1;
                            }
                            QAction *next = d->actionList.at(next_i);
                            if (next == d->currentAction)
                                break;
                            if (next->isSeparator() ||
                               (!next->isEnabled() &&
                                !style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this)))
                                continue;
                            nextAction = next;
                            if (d->scroll && (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)) {
                                int topVisible = style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, this);
                                if (d->tearoff)
                                    topVisible += style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this);
                                if (((y + d->scroll->scrollOffset) - topVisible) <= d->actionRects.value(nextAction).height())
                                    scroll_loc = QMenuPrivate::QMenuScroller::ScrollTop;
                            }
                            break;
                        }
                        if (!nextAction && d->tearoff)
                            d->tearoffHighlighted = 1;
                    } else {
                        y += d->actionRects.value(act).height();
                        for(int next_i = i+1; true; next_i++) {
                            if (next_i == d->actionList.count()) {
                                if (d->scroll)
                                    break;
                                next_i = 0;
                            }
                            QAction *next = d->actionList.at(next_i);
                            if (next == d->currentAction)
                                break;
                            if (next->isSeparator() ||
                               (!next->isEnabled() &&
                                !style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this)))
                                continue;
                            nextAction = next;
                            if (d->scroll && (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown)) {
                                const int scrollerHeight = style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, this);
                                int bottomVisible = height()-scrollerHeight;
                                if (d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
                                    bottomVisible -= scrollerHeight;
                                if (d->tearoff)
                                    bottomVisible -= style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, q);
                                if ((y + d->scroll->scrollOffset + d->actionRects.value(nextAction).height()) > bottomVisible)
                                    scroll_loc = QMenuPrivate::QMenuScroller::ScrollBottom;
                            }
                            break;
                        }
                    }
                    break;
                }
                y += d->actionRects.value(act).height();
            }
        }
        if (nextAction) {
            if (d->scroll && scroll_loc != QMenuPrivate::QMenuScroller::ScrollStay) {
                if (d->scroll->scrollTimer)
                    d->scroll->scrollTimer->stop();
                d->scrollMenu(nextAction, scroll_loc);
            }
            d->setCurrentAction(nextAction);
        }
        break; }

    case Qt::Key_Right:
        if (d->currentAction && d->currentAction->isEnabled() && d->currentAction->menu()) {
            d->popupAction(d->currentAction, 0, true);
            key_consumed = true;
            break;
        }
        //FALL THROUGH
    case Qt::Key_Left: {
        if (d->currentAction && !d->scroll) {
            QAction *nextAction = 0;
            if (key == Qt::Key_Left) {
                QRect actionR = d->actionRect(d->currentAction);
                for(int x = actionR.left()-1; !nextAction && x >= 0; x--)
                    nextAction = d->actionAt(QPoint(x, actionR.center().y()));
            } else {
                QRect actionR = d->actionRect(d->currentAction);
                for(int x = actionR.right()+1; !nextAction && x < width(); x++)
                    nextAction = d->actionAt(QPoint(x, actionR.center().y()));
            }
            if (nextAction) {
                d->setCurrentAction(nextAction);
                key_consumed = true;
            }
        }
        if (!key_consumed && key == Qt::Key_Left && d->causedPopup && qobject_cast<QMenu*>(d->causedPopup)) {
            QPointer<QWidget> caused = d->causedPopup;
            hide();
            if (caused)
                caused->setFocus();
            key_consumed = true;
        }
        break; }

    case Qt::Key_Alt:
        key_consumed = true;
        break;

    case Qt::Key_Escape:
        key_consumed = true;
        if (d->tornoff) {
            close();
            return;
        }
        {
            QPointer<QWidget> caused = d->causedPopup;
            hide(); //hide after getting causedPopup
            if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
                mb->d->setCurrentAction(d->menuAction);
                mb->d->setKeyboardMode(true);
            }
        }
        break;

    case Qt::Key_Space:
        if (!style()->styleHint(QStyle::SH_Menu_SpaceActivatesItem, 0, this))
            break;
        // for motif, fall through
    case Qt::Key_Return:
    case Qt::Key_Enter: {
            if (!d->currentAction)
                break;
            if (d->currentAction->menu())
                d->popupAction(d->currentAction, 20, true);
            else
                d->activateAction(d->currentAction, QAction::Trigger);
            key_consumed = true;
            break; }

#ifndef QT_NO_WHATSTHIS
    case Qt::Key_F1:
        if (!d->currentAction || d->currentAction->whatsThis().isNull())
            break;
        QWhatsThis::enterWhatsThisMode();
        d->activateAction(d->currentAction, QAction::Trigger);
        return;
#endif
    default:
        key_consumed = false;
    }

    if (!key_consumed) {                                // send to menu bar
        if ((!e->modifiers() || e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ShiftModifier) &&
           e->text().length()==1) {
            int clashCount = 0;
            QAction *first = 0, *currentSelected = 0, *firstAfterCurrent = 0;
            {
                QChar c = e->text().at(0).toUpper();
                for(int i = 0; i < d->actionList.size(); ++i) {
                    register QAction *act = d->actionList.at(i);
                    QKeySequence sequence = QKeySequence::mnemonic(act->text());
                    int key = sequence[0] & 0xffff;
                    if (key == c.unicode()) {
                        clashCount++;
                        if (!first)
                            first = act;
                        if (act == d->currentAction)
                            currentSelected = act;
                        else if (!firstAfterCurrent && currentSelected)
                            firstAfterCurrent = act;
                    }
                }
            }
            QAction *next_action = 0;
            if (clashCount >= 1) {
                if (clashCount == 1 || !d->currentAction || (currentSelected && !firstAfterCurrent))
                    next_action = first;
                else
                    next_action = firstAfterCurrent;
            }
            if (next_action) {
                d->setCurrentAction(next_action, 20, true);
                if (!next_action->menu()) {
                    key_consumed = true;
                    d->activateAction(next_action, QAction::Trigger);
                }
            }
        }
        if (!key_consumed) {
            if (QWidget *caused = d->causedPopup) {
                while(QMenu *m = qobject_cast<QMenu*>(caused))
                    caused = m->d->causedPopup;
                if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
                    QAction *oldAct = mb->d->currentAction;
                    QApplication::sendEvent(mb, e);
                    if (mb->d->currentAction != oldAct)
                        key_consumed = true;
                }
            }
        }

#ifdef Q_OS_WIN32
        if (key_consumed && (e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift || e->key() == Qt::Key_Meta))
            qApp->beep();
#endif // Q_OS_WIN32
    }
    if (key_consumed)
        e->accept();
    else
        e->ignore();
}

/*!
  \reimp
*/
void QMenu::mouseMoveEvent(QMouseEvent *e)
{
    if (!isVisible() || d->mouseEventTaken(e))
        return;
    d->motions++;
    if (d->motions == 0) // ignore first mouse move event (see enterEvent())
        return;
    d->hasHadMouse |= rect().contains(e->pos());

    QAction *action = d->actionAt(e->pos());
    if (!action) {
        if (d->hasHadMouse && !rect().contains(e->pos()))
            d->setCurrentAction(0);
        return;
    } else {
        d->mouseDown = e->buttons() & Qt::LeftButton;
    }
    if (d->sloppyRegion.contains(e->pos())) {
        static QTimer *sloppyDelayTimer = 0;
        if (!sloppyDelayTimer)
            sloppyDelayTimer = new QTimer(qApp);
        sloppyDelayTimer->disconnect(SIGNAL(timeout()));
        QObject::connect(sloppyDelayTimer, SIGNAL(timeout()),
                         q, SLOT(internalSetSloppyAction()));
        sloppyDelayTimer->setSingleShot(true);
        sloppyDelayTimer->start(style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, 0, this)*6);
        d->sloppyAction = action;
    } else {
        d->setCurrentAction(action, style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, 0, this));
    }
}

/*!
  \reimp
*/
void QMenu::enterEvent(QEvent *)
{
    d->motions = -1; // force us to ignore the generate mouse move in mouseMoveEvent()
}

/*!
  \reimp
*/
void QMenu::leaveEvent(QEvent *)
{
    d->sloppyAction = 0;
    if (!d->sloppyRegion.isEmpty())
        d->sloppyRegion = QRegion();
}

/*!
  \reimp
*/
void
QMenu::timerEvent(QTimerEvent *e)
{
    if (d->scroll && d->scroll->scrollTimer && d->scroll->scrollTimer->timerId() == e->timerId()) {
        d->scrollMenu((QMenuPrivate::QMenuScroller::ScrollDirection)d->scroll->scrollDirection);
        if (d->scroll->scrollFlags == QMenuPrivate::QMenuScroller::ScrollNone)
            d->scroll->scrollTimer->stop();
    }
}

/*!
  \reimp
*/
void QMenu::actionEvent(QActionEvent *e)
{
    d->itemsDirty = 1;
    setAttribute(Qt::WA_Resized, false);
    if (d->tornPopup)
        d->tornPopup->syncWithMenu(this, e);
#ifdef Q_WS_MAC
    if (d->mac_menu) {
        if (e->type() == QEvent::ActionAdded)
            d->mac_menu->addAction(e->action(), d->mac_menu->findAction(e->before()));
        else if (e->type() == QEvent::ActionRemoved)
            d->mac_menu->removeAction(e->action());
        else if (e->type() == QEvent::ActionChanged)
            d->mac_menu->syncAction(e->action());
    }
#endif
    if (e->type() == QEvent::ActionAdded) {
        connect(e->action(), SIGNAL(triggered()), this, SLOT(actionTriggered()));
        connect(e->action(), SIGNAL(hovered()), this, SLOT(actionHovered()));
    } else if (e->type() == QEvent::ActionRemoved) {
        e->action()->disconnect(this);
    }

    if (isVisible()) {
        d->updateActions();
        update();
    }
}

/*!
  \internal
*/
void QMenu::internalSetSloppyAction()
{
    if (d->sloppyAction)
        d->setCurrentAction(d->sloppyAction, 0);
}

/*!
  \internal
*/
void QMenu::internalDelayedPopup()
{
    if (!d->currentAction || !d->currentAction || d->currentAction->menu() == d->activeMenu)
        return;

    //hide the current item
    if (QMenu *menu = d->activeMenu) {
        d->activeMenu = 0;
        menu->hide();
    }

    //setup
    QRect actionRect(d->actionRect(d->currentAction));
    QPoint pos(mapToGlobal(QPoint(actionRect.right(), actionRect.top())));
    d->activeMenu = d->currentAction->menu();
    d->activeMenu->d->causedPopup = this;

    bool on_left = false;     //find "best" position
    const QSize menuSize(d->activeMenu->sizeHint() + d->activeMenu->contentsMarginSize());
    if (isRightToLeft()) {
        on_left = true;
        QMenu *caused = qobject_cast<QMenu*>(d->causedPopup);
        if (caused && caused->x() < x() || x() - menuSize.width() < 0)
            on_left = false;
    } else {
        QMenu *caused = qobject_cast<QMenu*>(d->causedPopup);
        if (caused && caused->x() > x() ||
            x() + width() + menuSize.width() > QApplication::desktop()->width())
            on_left = true;
    }
    if (on_left)
        pos.rx() = mapToGlobal(QPoint(actionRect.left() - menuSize.width(), 0)).x();

    //calc sloppy focus buffer
    if (style()->styleHint(QStyle::SH_Menu_SloppySubMenus, 0, this)) {
        QPoint cur = QCursor::pos();
        if (actionRect.contains(mapFromGlobal(cur))) {
            QPoint pts[4];
            pts[0] = QPoint(cur.x(), cur.y() - 2);
            pts[3] = QPoint(cur.x(), cur.y() + 2);
            if (pos.x() >= cur.x())        {
                pts[1] = QPoint(geometry().right(), pos.y());
                pts[2] = QPoint(geometry().right(), pos.y() + menuSize.height());
            } else {
                pts[1] = QPoint(pos.x() + menuSize.width(), pos.y());
                pts[2] = QPoint(pos.x() + menuSize.width(), pos.y() + menuSize.height());
            }
            QPolygon points(4);
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
    \fn void QMenu::triggered(QAction *action)

    This signal is emitted when a menu action is triggered; \a action
    is the action that caused the signal to be emitted.

    Normally, you connect each menu action's triggered() signal to its
    own custom slot, but sometimes you will want to connect several
    actions to a single slot, for example, when you have a group of
    closely related actions, such as "left justify", "center", "right
    justify".

    \sa hovered(), QAction::triggered()
*/

/*!
    \fn void QMenu::hovered(QAction *action)

    This signal is emitted when a menu action is highlighted; \a action
    is the action that caused the signal to be emitted.

    Often this is used to update status information.

    \sa triggered(), QAction::hovered()
*/


/*!\internal
*/
void QMenu::setNoReplayFor(QWidget *noReplayFor)
{
    d->noReplayFor = noReplayFor;
}

#ifdef QT3_SUPPORT

int QMenu::insertAny(const QIcon *icon, const QString *text, const QObject *receiver, const char *member,
                          const QKeySequence *shortcut, const QMenu *popup, int id, int index)
{
    QAction *act = popup ? popup->menuAction() : new QAction(this);
    if (id != -1)
        static_cast<QMenuItem*>(act)->setId(id);
    if (icon)
        act->setIcon(*icon);
    if (text)
        act->setText(*text);
    if (shortcut)
        act->setShortcut(*shortcut);
    if (receiver && member)
        QObject::connect(act, SIGNAL(activated(int)), receiver, member);
    if (index == -1)
        addAction(act);
    else
        insertAction(actions().value(index+1), act);
    return findIdForAction(act);
}

/*!
    Use the insertSeparator() overload that takes a QAction *
    parameter instead.
*/
int QMenu::insertSeparator(int index)
{
    QAction *act = new QAction(this);
    act->setSeparator(true);
    if (index == -1)
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

/*!
###
*/
QMenuItem *QMenu::findPopup( QMenu *popup, int *index )
{
   QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *act = list.at(i);
        if (act->menu() == popup) {
            QMenuItem *item = static_cast<QMenuItem *>(act);
            if (index)
                *index = act->d->id;
            return item;
        }
    }
    return 0;
}


/*!
###
*/
bool QMenu::setItemParameter(int id, int param)
{
    if (QAction *act = findActionForId(id)) {
        act->d->param = param;
        return true;
    }
    return false;
}

/*!
###
*/
int QMenu::itemParameter(int id) const
{
    if (QAction *act = findActionForId(id))
        return act->d->param;
    return id;
}

/*!
    Use style()->pixelMetric(QStyle::PM_MenuPanelWidth, this) instead.
*/
int QMenu::frameWidth() const
{
    return style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
}

int QMenu::findIdForAction(QAction *act) const
{
    if (!act)
        return -1;
    return act->d->id;
}
#endif // QT3_SUPPORT

/*!
    \fn uint QMenu::count() const

    Use actions().count() instead.
*/

/*!
    \fn int QMenu::insertItem(const QString &text, const QObject *receiver, const char* member, const QKeySequence& shortcut, int id, int index)

    Use insertAction() or one of the addAction() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QIcon& icon, const QString &text, const QObject *receiver, const char* member, const QKeySequence& shortcut, int id, int index)

    Use insertAction() or one of the addAction() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QPixmap &pixmap, const QObject *receiver, const char* member, const QKeySequence& shortcut, int id, int index)

    Use insertAction() or one of the addAction() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QString &text, int id, int index)

    Use insertAction() or one of the addAction() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QIcon& icon, const QString &text, int id, int index)

    Use insertAction() or one of the addAction() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QString &text, QMenu *popup, int id, int index)

    Use insertMenu() or one of the addMenu() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QIcon& icon, const QString &text, QMenu *popup, int id, int index)

    Use insertMenu() or one of the addMenu() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QPixmap &pixmap, int id, int index)

    Use insertAction() or one of the addAction() overloads instead.
*/

/*!
    \fn int QMenu::insertItem(const QPixmap &pixmap, QMenu *popup, int id, int index)

    Use insertMenu() or one of the addMenu() overloads instead.
*/

/*!
    \fn void QMenu::removeItem(int id)

    Use removeAction() instead.
*/

/*!
    \fn void QMenu::removeItemAt(int index)

    Use removeAction() instead.
*/

/*!
    \fn QKeySequence QMenu::accel(int id) const

    Use shortcut() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::setAccel(const QKeySequence& key, int id)

    Use setShortcut() on the relevant QAction instead.
*/

/*!
    \fn QIcon QMenu::iconSet(int id) const

    Use icon() on the relevant QAction instead.
*/

/*!
    \fn QString QMenu::text(int id) const

    Use text() on the relevant QAction instead.
*/

/*!
    \fn QPixmap QMenu::pixmap(int id) const

    Use QPixmap(icon()) on the relevant QAction instead.
*/

/*!
    \fn void QMenu::setWhatsThis(int id, const QString &w)

    Use setWhatsThis() on the relevant QAction instead.
*/

/*!
    \fn QString QMenu::whatsThis(int id) const

    Use whatsThis() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::changeItem(int id, const QString &text)

    Use setText() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::changeItem(int id, const QPixmap &pixmap)

    Use setText() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::changeItem(int id, const QIcon &icon, const QString &text)

    Use setIcon() and setText() on the relevant QAction instead.
*/

/*!
    \fn bool QMenu::isItemActive(int id) const

    Use activeAction() instead.
*/

/*!
    \fn bool QMenu::isItemEnabled(int id) const

    Use isEnabled() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::setItemEnabled(int id, bool enable)

    Use setEnabled() on the relevant QAction instead.
*/

/*!
    \fn bool QMenu::isItemChecked(int id) const

    Use isChecked() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::setItemChecked(int id, bool check)

    Use setChecked() on the relevant QAction instead.
*/

/*!
    \fn bool QMenu::isItemVisible(int id) const

    Use isVisible() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::setItemVisible(int id, bool visible)

    Use setVisible() on the relevant QAction instead.
*/

/*!
    \fn QRect QMenu::itemGeometry(int index)

    Use actionGeometry() on the relevant QAction instead.
*/

/*!
    \fn QFont QMenu::itemFont(int id) const

    Use font() on the relevant QAction instead.
*/

/*!
    \fn void QMenu::setItemFont(int id, const QFont &font)

    Use setFont() on the relevant QAction instead.
*/

/*!
    \fn int QMenu::indexOf(int id) const

    Use actions().indexOf(action) on the relevant QAction instead.
*/

/*!
    \fn int QMenu::idAt(int index) const
###
*/

/*!
    \fn void QMenu::activateItemAt(int index)

    Use activate() on the relevant QAction instead.
*/

/*!
    \fn bool QMenu::connectItem(int id, const QObject *receiver, const char* member)

    Use connect() on the relevant QAction instead.
*/

/*!
    \fn bool QMenu::disconnectItem(int id,const QObject *receiver, const char* member)
    Use disconnect() on the relevant QAction instead.

*/

/*!
    \fn QMenuItem *QMenu::findItem(int id) const
###
*/

/*!
    \fn void QMenu::popup(const QPoint & pos, int indexAtPoint)

    Use popup() on the relevant QAction instead.
*/

/*!
    \fn int QMenu::insertTearOffHandle(int a, int b)

    Use setTearOffEnabled() instead.
*/

/*!
    \fn int QMenu::itemAtPos(const QPoint &p, bool ignoreSeparator)
###
*/

/*!
    \fn int QMenu::columns() const

    Use columnCount() instead.
*/

/*!
    \fn int QMenu::itemHeight(int index)

    Use actionGeometry(actions().value(index)).height() instead.
*/

/*!
    \fn int QMenu::itemHeight(QMenuItem *mi)

###
*/

/*!
    \fn void QMenu::aboutToHide();

    Invert the logic and use aboutToShow() instead.
*/

/*!
    \fn void QMenu::activated(int itemId);

    Use triggered() instead.
*/

/*!
    \fn void QMenu::highlighted(int itemId);

    Use hovered() instead.
*/

/*!
    \fn void QMenu::setCheckable(bool checkable)

    Not necessary anymore. The \a checkable parameter is ignored.
*/

/*!
    \fn bool QMenu::isCheckable() const

    Not necessary anymore. Always returns true.
*/

#include "moc_qmenu.cpp"
