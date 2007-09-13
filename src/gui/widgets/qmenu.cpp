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

#include "qmenu.h"

#ifndef QT_NO_MENU

#include "qdebug.h"
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
#include "qwidgetaction.h"
#include "qtoolbutton.h"
#include <private/qaction_p.h>
#ifdef QT3_SUPPORT
#include <qmenudata.h>
#endif // QT3_SUPPORT

#ifdef Q_WS_X11
#   include <private/qt_x11_p.h>
#endif

#if defined(Q_WS_MAC) && !defined(QT_NO_EFFECTS)
#   include <private/qcore_mac_p.h>
#endif


QT_BEGIN_NAMESPACE

QPointer<QMenu> QMenuPrivate::mouseDown;
QBasicTimer QMenuPrivate::menuDelayTimer;
QBasicTimer QMenuPrivate::sloppyDelayTimer;

/* QMenu code */
// internal class used for the torn off popup
class QTornOffMenu : public QMenu
{
    Q_OBJECT
    class QTornOffMenuPrivate : public QMenuPrivate
    {
        Q_DECLARE_PUBLIC(QMenu)
    public:
        QTornOffMenuPrivate(QMenu *p) : causedMenu(p) {
            tornoff = 1;
            causedPopup.widget = 0;
            causedPopup.action = ((QTornOffMenu*)p)->d_func()->causedPopup.action;
            causedStack = ((QTornOffMenu*)p)->d_func()->calcCausedStack();
        }
        QList<QPointer<QWidget> > calcCausedStack() const { return causedStack; }
        QPointer<QMenu> causedMenu;
        QList<QPointer<QWidget> > causedStack;
    };
public:
    QTornOffMenu(QMenu *p) : QMenu(*(new QTornOffMenuPrivate(p)))
    {
        // make the torn-off menu a sibling of p (instead of a child)
        QWidget *parentWidget = p->parentWidget();
        if (!parentWidget)
            parentWidget = p;
        setParent(parentWidget, Qt::Window | Qt::Tool);
	setAttribute(Qt::WA_DeleteOnClose, true);
        setWindowTitle(p->windowTitle());
        setEnabled(p->isEnabled());
        //QObject::connect(this, SIGNAL(triggered(QAction*)), this, SLOT(onTrigger(QAction*)));
        //QObject::connect(this, SIGNAL(hovered(QAction*)), this, SLOT(onHovered(QAction*)));
        QList<QAction*> items = p->actions();
        for(int i = 0; i < items.count(); i++)
            addAction(items.at(i));
    }
    void syncWithMenu(QMenu *menu, QActionEvent *act)
    {
        Q_D(QTornOffMenu);
        if(menu != d->causedMenu)
            return;
        if (act->type() == QEvent::ActionAdded) {
            insertAction(act->before(), act->action());
        } else if (act->type() == QEvent::ActionRemoved)
            removeAction(act->action());
    }
    void actionEvent(QActionEvent *e)
    {
        QMenu::actionEvent(e);
        setFixedSize(sizeHint());
    }
public slots:
    void onTrigger(QAction *action) { d_func()->activateAction(action, QAction::Trigger, false); }
    void onHovered(QAction *action) { d_func()->activateAction(action, QAction::Hover, false); }
private:
    Q_DECLARE_PRIVATE(QTornOffMenu)
    friend class QMenuPrivate;
};

void QMenuPrivate::init()
{
    Q_Q(QMenu);
#ifndef QT_NO_WHATSTHIS
    q->setAttribute(Qt::WA_CustomWhatsThis);
#endif
    q->setMouseTracking(q->style()->styleHint(QStyle::SH_Menu_MouseTracking, 0, q));
    if (q->style()->styleHint(QStyle::SH_Menu_Scrollable, 0, q)) {
        scroll = new QMenuPrivate::QMenuScroller;
        scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
    defaultMenuAction = menuAction = new QAction(q);
    menuAction->d_func()->menu = q;
}

//Windows and KDE allows menus to cover the taskbar, while GNOME and Mac don't
QRect QMenuPrivate::popupGeometry(int screen) const
{
#ifdef Q_WS_WIN
    return QApplication::desktop()->screenGeometry(screen);
#elif defined Q_WS_X11
    if (X11->desktopEnvironment == DE_KDE)
        return QApplication::desktop()->screenGeometry(screen);
    else
        return QApplication::desktop()->availableGeometry(screen);
#else
        return QApplication::desktop()->availableGeometry(screen);
#endif
}

QList<QPointer<QWidget> > QMenuPrivate::calcCausedStack() const
{
    QList<QPointer<QWidget> > ret;
    for(QWidget *widget = causedPopup.widget; widget; ) {
        ret.append(widget);
        if (QTornOffMenu *qtmenu = qobject_cast<QTornOffMenu*>(widget))
            ret += qtmenu->d_func()->causedStack;
        if (QMenu *qmenu = qobject_cast<QMenu*>(widget))
            widget = qmenu->d_func()->causedPopup.widget;
        else
            break;
    }
    return ret;
}

void QMenuPrivate::calcActionRects(QMap<QAction*, QRect> &actionRects, QList<QAction*> &actionList) const
{
    Q_Q(const QMenu);
    if (!itemsDirty) {
        actionRects = this->actionRects;
        actionList = this->actionList;
        return;
    }

    actionRects.clear();
    actionList.clear();
    QList<QAction*> items = filterActions(q->actions());
    int max_column_width = 0,
        dh = popupGeometry(QApplication::desktop()->screenNumber(q)).height(),
        ncols = 1,
        y = 0;
    const int hmargin = q->style()->pixelMetric(QStyle::PM_MenuHMargin, 0, q),
              vmargin = q->style()->pixelMetric(QStyle::PM_MenuVMargin, 0, q),
              icone = q->style()->pixelMetric(QStyle::PM_SmallIconSize, 0, q);

    //for compatability now - will have to refactor this away..
    tabWidth = 0;
    maxIconWidth = 0;
    hasCheckableItems = false;
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if (widgetItems.value(action))
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

        QFontMetrics fm(action->font().resolve(q->font()));
        QSize sz;

        //let the style modify the above size..
        QStyleOptionMenuItem opt;
        q->initStyleOption(&opt, action);
        opt.rect = q->rect();

        if (QWidget *w = widgetItems.value(action)) {
          sz=w->sizeHint().expandedTo(w->minimumSize()).expandedTo(w->minimumSizeHint()).boundedTo(w->maximumSize());
        } else {
            //calc what I think the size is..
            if (action->isSeparator()) {
                sz = QSize(2, 2);
            } else {
                QString s = action->text();
                int t = s.indexOf(QLatin1Char('\t'));
                if (t != -1) {
                    tabWidth = qMax(int(tabWidth), qfm.width(s.mid(t+1)));
                    s = s.left(t);
    #ifndef QT_NO_SHORTCUT
                } else {
                    QKeySequence seq = action->shortcut();
                    if (!seq.isEmpty())
                        tabWidth = qMax(int(tabWidth), qfm.width(seq));
    #endif
                }
                int w = fm.width(s);
                w -= s.count(QLatin1Char('&')) * fm.width(QLatin1Char('&'));
                w += s.count(QLatin1String("&&")) * fm.width(QLatin1Char('&'));
                sz.setWidth(w);
                sz.setHeight(qMax(fm.height(), qfm.height()));

                QIcon is = action->icon();
                if (!is.isNull()) {
                    QSize is_sz = QSize(icone, icone);
                    if (is_sz.height() > sz.height())
                        sz.setHeight(is_sz.height());
                }
            }
          sz = q->style()->sizeFromContents(QStyle::CT_MenuItem, &opt, sz, q);
        }


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
        if (rect.isNull())
            continue;
        if (!scroll &&
           y+rect.height() > dh - (q->style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, q) * 2)) {
            ncols--;
            if (ncols < 0)
                qWarning("QMenu: Column calculation mismatch (%d)", ncols);
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
    Q_Q(const QMenu);
    if (!itemsDirty)
        return;
    sloppyAction = 0;
    calcActionRects(actionRects, actionList);
    for (QHash<QAction *, QWidget *>::ConstIterator item = widgetItems.constBegin(),
         end = widgetItems.constEnd(); item != end; ++item) {
        QAction *action = item.key();
        QWidget *widget = item.value();
        widget->setGeometry(actionRect(action));
        widget->setVisible(action->isVisible());
    }
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

QList<QAction *> QMenuPrivate::filterActions(const QList<QAction *> &actions) const
{
    QList<QAction *> visibleActions;
    int i = 0;
    while (i < actions.count()) {
        QAction *action = actions.at(i);
        if (!action->isVisible()) {
            ++i;
            continue;
        }
        if (!action->isSeparator() || !collapsibleSeparators) {
            visibleActions.append(action);
            ++i;
            continue;
        }

        // no leading separators
        if (!visibleActions.isEmpty())
            visibleActions.append(action);

        // skip double/tripple/etc. separators
        while (i < actions.count()
               && (!actions.at(i)->isVisible() || actions.at(i)->isSeparator()))
            ++i;
    }

    if (collapsibleSeparators) {
        // remove trailing separators
        while (!visibleActions.isEmpty() && visibleActions.last()->isSeparator())
            visibleActions.removeLast();
    }

    return visibleActions;
}

QRect QMenuPrivate::actionRect(QAction *act) const
{
    Q_Q(const QMenu);
    QRect ret = actionRects.value(act);
    if (ret.isNull())
        return ret;
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
    Q_Q(QMenu);
    if (!tornoff) {
        QWidget *caused = causedPopup.widget;
        hideMenu(q); //hide after getting causedPopup
        while(caused) {
#ifndef QT_NO_MENUBAR
            if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
                mb->d_func()->setCurrentAction(0);
                caused = 0;
            } else
#endif
            if (QMenu *m = qobject_cast<QMenu*>(caused)) {
                caused = m->d_func()->causedPopup.widget;
                if (!m->d_func()->tornoff)
                    hideMenu(m);
                m->d_func()->setCurrentAction(0);
            } else {
#ifndef QT_NO_TOOLBUTTON
                if (qobject_cast<QToolButton*>(caused) == 0)
#endif
                    qWarning("QMenu: Internal error");
                caused = 0;
            }
        }
    }
    setCurrentAction(0);
}

void QMenuPrivate::hideMenu(QMenu *menu)
{
    if (!menu)
        return;

#if !defined(QT_NO_EFFECTS)
    // Flash item which is about to trigger (if any).
    if (menu->style()->styleHint(QStyle::SH_Menu_FlashTriggeredItem)
        && currentAction && currentAction == actionAboutToTrigger) {

        QEventLoop eventLoop;
        QAction *activeAction = currentAction;

        // Deselect and wait 60 ms.
        menu->setActiveAction(0);
        QTimer::singleShot(60, &eventLoop, SLOT(quit()));
        eventLoop.exec();

        // Select and wait 20 ms.
        menu->setActiveAction(activeAction);
        QTimer::singleShot(20, &eventLoop, SLOT(quit()));
        eventLoop.exec();
    }

    // Fade out.
    if (menu->style()->styleHint(QStyle::SH_Menu_FadeOutOnHide)) {
        // ### Qt4.4
        // Should be something like: q->transitionWindow(Qt::FadeOutTransition, 150);
        // Hopefully we'll integrate qt/research/windowtransitions into main before 4.4.
        // Talk to Richard, Trenton or Bjoern.
#if defined(Q_WS_MAC)
        TransitionWindowOptions options = {0, 0.15, 0, 0};
        TransitionWindowWithOptions(qt_mac_window_for(menu), kWindowFadeTransitionEffect,
                                    kWindowHideTransitionAction, 0, 1, &options);

        // Wait for the transition to complete.
        QEventLoop eventLoop;
        QTimer::singleShot(150, &eventLoop, SLOT(quit()));
        eventLoop.exec();
#endif // Q_WS_MAC
    }
#endif // QT_NO_EFFECTS
    menu->hide();
}

void QMenuPrivate::popupAction(QAction *action, int delay, bool activateFirst)
{
    Q_Q(QMenu);
    if (action && action->isEnabled()) {
        if (!delay)
            q->internalDelayedPopup();
        else
            QMenuPrivate::menuDelayTimer.start(delay, q);
        if (activateFirst && action->menu())
            action->menu()->d_func()->setFirstActionActive();
    } else if (QMenu *menu = activeMenu) {  //hide the current item
        activeMenu = 0;
        hideMenu(menu);
    }
}

void QMenuPrivate::setSyncAction()
{
    Q_Q(QMenu);
    QAction *current = currentAction;
    if(current && (!current->isEnabled() || current->menu() || current->isSeparator()))
        current = 0;
    for(QWidget *caused = q; caused;) {
        if (QMenu *m = qobject_cast<QMenu*>(caused)) {
            caused = m->d_func()->causedPopup.widget;
            if (m->d_func()->eventLoop)
                m->d_func()->syncAction = current; // synchronous operation
        } else {
            break;
        }
    }
}


void QMenuPrivate::setFirstActionActive()
{
    Q_Q(QMenu);
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
void QMenuPrivate::setCurrentAction(QAction *action, int popup, SelectionReason reason, bool activateFirst)
{
    Q_Q(QMenu);
    tearoffHighlighted = 0;
    if (action == currentAction && !(action && action->menu() && action->menu() != activeMenu)) {
        if(QMenu *menu = qobject_cast<QMenu*>(causedPopup.widget)) {
            if(causedPopup.action && menu->d_func()->activeMenu == q)
                menu->d_func()->setCurrentAction(causedPopup.action, 0, reason, false);
        }
        return;
    }
    if (currentAction)
        q->update(actionRect(currentAction));

    sloppyAction = 0;
    if (!sloppyRegion.isEmpty())
        sloppyRegion = QRegion();
    QMenu *hideActiveMenu = activeMenu;
#ifndef QT_NO_STATUSTIP
    QAction *previousAction = currentAction;
#endif
    currentAction = action;
    if (action) {
        if (!action->isSeparator()) {
            activateAction(action, QAction::Hover);
            if (popup != -1) {
                hideActiveMenu = 0; //will be done "later"
                popupAction(currentAction, popup, activateFirst);
            }
            q->update(actionRect(action));
            QWidget *widget = widgetItems.value(action);

            if (reason == SelectedFromKeyboard) {
                if (widget) {
                    if (widget->focusPolicy() != Qt::NoFocus)
                        widget->setFocus(Qt::TabFocusReason);
                } else {
                    //when the action has no QWidget, the QMenu itself should
                    // get the focus
                    // Since the menu is a pop-up, it uses the popup reason.
                    q->setFocus(Qt::PopupFocusReason);
                }
            }
        } else { //action is a separator
            if (popup != -1)
                hideActiveMenu = 0; //will be done "later"
        }
#ifndef QT_NO_STATUSTIP
    }  else if (previousAction) {
        QWidget *w = causedPopup.widget;
        while (QMenu *m = qobject_cast<QMenu*>(w))
            w = m->d_func()->causedPopup.widget;
        if (w) {
            QString empty;
            QStatusTipEvent tip(empty);
            QApplication::sendEvent(w, &tip);
        }
#endif
    }
    if (hideActiveMenu) {
        activeMenu = 0;
#ifndef QT_NO_EFFECTS
        // kill any running effect
        qFadeEffect(0);
        qScrollEffect(0);
#endif
        hideMenu(hideActiveMenu);
    }
}

QAction *QMenuPrivate::actionAt(QPoint p) const
{
    if (!q_func()->rect().contains(p))     //sanity check
       return 0;

    for(int i = 0; i < actionList.count(); i++) {
        QAction *act = actionList[i];
        if (actionRect(act).contains(p))
            return act;
    }
    return 0;
}

void QMenuPrivate::setOverrideMenuAction(QAction *a)
{
    Q_Q(QMenu);
    QObject::disconnect(menuAction, SIGNAL(destroyed()), q, SLOT(_q_overrideMenuActionDestroyed()));
    if (a) {
        menuAction = a;
        QObject::connect(a, SIGNAL(destroyed()), q, SLOT(_q_overrideMenuActionDestroyed()));
    } else { //we revert back to the default action created by the QMenu itself
        menuAction = defaultMenuAction;
    }
}

void QMenuPrivate::_q_overrideMenuActionDestroyed()
{
    menuAction=defaultMenuAction;
}

/*!
    Returns the action associated with this menu.
*/
QAction *QMenu::menuAction() const
{
    return d_func()->menuAction;
}

/*!
  \property QMenu::title

  \brief The title of the menu

  This is equivalent to the QAction::text property of the menuAction().
*/
QString QMenu::title() const
{
    return d_func()->menuAction->text();
}

void QMenu::setTitle(const QString &text)
{
    d_func()->menuAction->setText(text);
}

/*!
  \property QMenu::icon

  \brief The icon of the menu

  This is equivalent to the QAction::icon property of the menuAction().
*/
QIcon QMenu::icon() const
{
    return d_func()->menuAction->icon();
}

void QMenu::setIcon(const QIcon &icon)
{
    d_func()->menuAction->setIcon(icon);
}


//actually performs the scrolling
void QMenuPrivate::scrollMenu(QAction *action, QMenuScroller::ScrollLocation location, bool active)
{
    Q_Q(QMenu);
    if (!scroll || !scroll->scrollFlags)
        return;
    int newOffset = 0;
    const int scrollHeight = q->style()->pixelMetric(QStyle::PM_MenuScrollerHeight, 0, q);
    const int topScroll = (scroll->scrollFlags & QMenuScroller::ScrollUp)   ? scrollHeight : 0;
    const int botScroll = (scroll->scrollFlags & QMenuScroller::ScrollDown) ? scrollHeight : 0;
    const int vmargin = q->style()->pixelMetric(QStyle::PM_MenuVMargin, 0, q);

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
                    newOffset = ((q->height() / 2) - botScroll) - (saccum - topScroll);
                else
                    newOffset = (q->height() - botScroll) - (saccum - topScroll);
                break;
            }
        }
        if(newOffset)
            newOffset -= vmargin*2;
    }

    //figure out which scroll flags
    uint newScrollFlags = QMenuScroller::ScrollNone;
    if (newOffset < 0) //easy and cheap one
        newScrollFlags |= QMenuScroller::ScrollUp;
    for(int i = 0, saccum=newOffset; i < actionList.count(); i++) {
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

    QRect screen = popupGeometry(QApplication::desktop()->screenNumber(q));
    const int desktopFrame = q->style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, q);
    if (q->height() < screen.height()-(desktopFrame*2)-1) {
        QRect geom = q->geometry();
        if (newOffset > scroll->scrollOffset && (scroll->scrollFlags & newScrollFlags & QMenuScroller::ScrollUp)) { //scroll up
            const int newHeight = geom.height()-(newOffset-scroll->scrollOffset);
            if(newHeight > geom.height())
                geom.setHeight(newHeight);
        } else if(scroll->scrollFlags & newScrollFlags & QMenuScroller::ScrollDown) {
            int newTop = geom.top() + (newOffset-scroll->scrollOffset);
            if (newTop < desktopFrame+screen.top())
                newTop = desktopFrame+screen.top();
            if (newTop < geom.top()) {
                geom.setTop(newTop);
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

void QMenuPrivate::scrollMenu(QMenuScroller::ScrollLocation location, bool active)
{
    Q_Q(QMenu);
    if(location == QMenuScroller::ScrollBottom) {
        for(int i = actionList.size()-1; i >= 0; --i) {
            QAction *act = actionList.at(i);
            if (!act->isSeparator() &&
                (q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)
                 || act->isEnabled())) {
                if(scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown)
                    scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollBottom, active);
                else if(active)
                    setCurrentAction(act, /*popup*/-1, QMenuPrivate::SelectedFromKeyboard);
                break;
            }
        }
    } else if(location == QMenuScroller::ScrollTop) {
        for(int i = 0; i < actionList.size(); ++i) {
            QAction *act = actionList.at(i);
            if (!act->isSeparator() &&
                (q->style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, q)
                 || act->isEnabled())) {
                if(scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
                    scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollTop, active);
                else if(active)
                    setCurrentAction(act, /*popup*/-1, QMenuPrivate::SelectedFromKeyboard);
                break;
            }
        }
    }
}

//only directional
void QMenuPrivate::scrollMenu(QMenuScroller::ScrollDirection direction, bool page, bool active)
{
    Q_Q(QMenu);
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
        bool scrolled = false;
        for(int i = 0, saccum = topScroll; i < actionList.count(); i++) { // saccum = topScroll
            QAction *act = actionList.at(i);
            const int iHeight = actionRects.value(act).height();
            saccum -= iHeight;
            if (saccum <= scroll->scrollOffset-topScroll) {
                const int scrollerArea = q->height() - botScroll;
                int visible = -(((scroll->scrollOffset-topScroll) - saccum) - iHeight);
                for(i++ ; i < actionList.count(); i++) {
                    act = actionList.at(i);
                    const int iHeight = actionRects.value(act).height();
                    visible += iHeight;
                    if (visible > scrollerArea-topScroll) {
                        scrolled = true;
                        scrollMenu(act, page ? QMenuScroller::ScrollTop : QMenuScroller::ScrollBottom, active);
                        break;
                    }
                }
                break;
            }
        }
        if(!scrolled) {
            scroll->scrollFlags &= ~QMenuScroller::ScrollDown;
            q->update();
        }
    }
}

/* This is poor-mans eventfilters. This avoids the use of
   eventFilter (which can be nasty for users of QMenuBar's). */
bool QMenuPrivate::mouseEventTaken(QMouseEvent *e)
{
    Q_Q(QMenu);
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
        if (tearRect.contains(pos) && hasMouseMoved(e->globalPos())) {
            setCurrentAction(0);
            tearoffHighlighted = 1;
            if (e->type() == QEvent::MouseButtonRelease) {
                if (!tornPopup)
                    tornPopup = new QTornOffMenu(q);
                tornPopup->setGeometry(q->geometry());
                tornPopup->show();
                hideUpToMenuBar();
            }
            return true;
        }
        tearoffHighlighted = 0;
    }

    if (q->frameGeometry().contains(e->globalPos())) //otherwise if the event is in our rect we want it..
        return false;

    for(QWidget *caused = causedPopup.widget; caused;) {
        bool passOnEvent = false;
        QWidget *next_widget = 0;
        QPoint cpos = caused->mapFromGlobal(e->globalPos());
#ifndef QT_NO_MENUBAR
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
            passOnEvent = mb->rect().contains(cpos);
        } else
#endif
        if (QMenu *m = qobject_cast<QMenu*>(caused)) {
            passOnEvent = m->d_func()->actionAt(cpos);
            next_widget = m->d_func()->causedPopup.widget;
        }
        if (passOnEvent) {
            if(e->type() != QEvent::MouseButtonRelease || mouseDown == caused) {
            QMouseEvent new_e(e->type(), cpos, e->button(), e->buttons(), e->modifiers());
            QApplication::sendEvent(caused, &new_e);
            return true;
        }
        }
        if (!next_widget)
            break;
        caused = next_widget;
    }
    return false;
}

void QMenuPrivate::activateAction(QAction *action, QAction::ActionEvent action_e, bool self)
{
    Q_Q(QMenu);
#ifndef QT_NO_WHATSTHIS
    bool inWhatsThisMode = QWhatsThis::inWhatsThisMode();
#endif
    if (!action || !q->isEnabled()
        || (action_e == QAction::Trigger
#ifndef QT_NO_WHATSTHIS
            && !inWhatsThisMode
#endif
            && (action->isSeparator() ||!action->isEnabled())))
        return;

    /* I have to save the caused stack here because it will be undone after popup execution (ie in the hide).
       Then I iterate over the list to actually send the events. --Sam
    */
    const QList<QPointer<QWidget> > causedStack = calcCausedStack();
    if (action_e == QAction::Trigger) {
        if (!inWhatsThisMode)
            actionAboutToTrigger = action;
        for(QWidget *widget = qApp->activePopupWidget(); widget; ) {
            if (QMenu *qmenu = qobject_cast<QMenu*>(widget)) {
                if(qmenu == q)
                    hideUpToMenuBar();
                widget = qmenu->d_func()->causedPopup.widget;
            } else {
                break;
            }
        }
#ifndef QT_NO_WHATSTHIS
        if (inWhatsThisMode) {
            QString s = action->whatsThis();
            if (s.isEmpty())
                s = whatsThis;
            QWhatsThis::showText(q->mapToGlobal(actionRect(action).center()), s, q);
            return;
        }
#endif
    }

#ifdef QT3_SUPPORT
    const int actionId = q->findIdForAction(action);
#endif
    if(self)
        action->activate(action_e);

    for(int i = 0; i < causedStack.size(); ++i) {
        QPointer<QWidget> widget = causedStack.at(i);
        if (!widget)
            continue;
        //fire
        if (QMenu *qmenu = qobject_cast<QMenu*>(widget)) {
            widget = qmenu->d_func()->causedPopup.widget;
            if (action_e == QAction::Trigger) {
                emit qmenu->triggered(action);
#ifdef QT3_SUPPORT
                emit qmenu->activated(actionId);
#endif
            } else if (action_e == QAction::Hover) {
                emit qmenu->hovered(action);
#ifdef QT3_SUPPORT
                emit qmenu->highlighted(actionId);
#endif
            }
#ifndef QT_NO_MENUBAR
        } else if (QMenuBar *qmenubar = qobject_cast<QMenuBar*>(widget)) {
            if (action_e == QAction::Trigger) {
                emit qmenubar->triggered(action);
#ifdef QT3_SUPPORT
                emit qmenubar->activated(actionId);
#endif
            } else if (action_e == QAction::Hover) {
                emit qmenubar->hovered(action);
#ifdef QT3_SUPPORT
                emit qmenubar->highlighted(actionId);
#endif
            }
            break; //nothing more..
#endif
        }
    }

    if (action_e == QAction::Hover) {
#ifndef QT_NO_ACCESSIBILITY
        if (QAccessible::isActive()) {
            int actionIndex = indexOf(action) + 1;
            QAccessible::updateAccessibility(q, actionIndex, QAccessible::Focus);
            QAccessible::updateAccessibility(q, actionIndex, QAccessible::Selection);
        }
#endif
        QWidget *w = causedPopup.widget;
        while (QMenu *m = qobject_cast<QMenu*>(w))
            w = m->d_func()->causedPopup.widget;
        action->showStatusText(w);
    } else {
        actionAboutToTrigger = 0;
    }
}

void QMenuPrivate::_q_actionTriggered()
{
    Q_Q(QMenu);
    if (QAction *action = qobject_cast<QAction *>(q->sender())) {
#ifdef QT3_SUPPORT
        //we store it here because the action might be deleted/changed by connected slots 
        const int id = q->findIdForAction(action);
#endif
        emit q->triggered(action);
#ifdef QT3_SUPPORT
        emit q->activated(id);
#endif
    }
}

void QMenuPrivate::_q_actionHovered()
{
    Q_Q(QMenu);
    if (QAction *action = qobject_cast<QAction *>(q->sender())) {
#ifdef QT3_SUPPORT
        //we store it here because the action might be deleted/changed by connected slots 
        const int id = q->findIdForAction(action);
#endif
        emit q->hovered(action);
#ifdef QT3_SUPPORT
        emit q->highlighted(id);
#endif
    }
}

bool QMenuPrivate::hasMouseMoved(const QPoint &globalPos)
{
    //determines if the mouse has moved (ie its intial position has
    //changed by more than QApplication::startDragDistance()
    //or if there were at least 6 mouse motions)
    return motions > 6 ||
        QApplication::startDragDistance() < (mousePopupPos - globalPos).manhattanLength();
}


/*!
    Initialize \a option with the values from this menu and information from \a action. This method
    is useful for subclasses when they need a QStyleOptionMenuItem, but don't want
    to fill in all the information themselves.

    \sa QStyleOption::initFrom() QMenuBar::initStyleOption()
*/
void QMenu::initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const
{
    if (!option || !action)
        return;

    Q_D(const QMenu);
    option->initFrom(this);
    option->palette = palette();
    option->state = QStyle::State_None;

    if (window()->isActiveWindow())
        option->state |= QStyle::State_Active;
    if (isEnabled() && action->isEnabled()
            && (!action->menu() || action->menu()->isEnabled()))
        option->state |= QStyle::State_Enabled;
    else
        option->palette.setCurrentColorGroup(QPalette::Disabled);

    option->font = action->font();

    if (d->currentAction && d->currentAction == action && !d->currentAction->isSeparator()) {
        option->state |= QStyle::State_Selected
                     | (d->mouseDown ? QStyle::State_Sunken : QStyle::State_None);
    }

    option->menuHasCheckableItems = d->hasCheckableItems;
    if (!action->isCheckable()) {
        option->checkType = QStyleOptionMenuItem::NotCheckable;
    } else {
        option->checkType = (action->actionGroup() && action->actionGroup()->isExclusive())
                            ? QStyleOptionMenuItem::Exclusive : QStyleOptionMenuItem::NonExclusive;
        option->checked = action->isChecked();
    }
    if (action->menu())
        option->menuItemType = QStyleOptionMenuItem::SubMenu;
    else if (action->isSeparator())
        option->menuItemType = QStyleOptionMenuItem::Separator;
    else if (d->defaultAction == action)
	    option->menuItemType = QStyleOptionMenuItem::DefaultItem;
    else
        option->menuItemType = QStyleOptionMenuItem::Normal;
    if (action->isIconVisibleInMenu())
        option->icon = action->icon();
    QString textAndAccel = action->text();
#ifndef QT_NO_SHORTCUT
    if (textAndAccel.indexOf(QLatin1Char('\t')) == -1) {
        QKeySequence seq = action->shortcut();
        if (!seq.isEmpty())
            textAndAccel += QLatin1Char('\t') + QString(seq);
    }
#endif
    option->text = textAndAccel;
    option->tabWidth = d->tabWidth;
    option->maxIconWidth = d->maxIconWidth;
    option->menuRect = rect();
}

/*!
    \class QMenu
    \brief The QMenu class provides a menu widget for use in menu
    bars, context menus, and other popup menus.

    \ingroup application
    \ingroup basicwidgets
    \mainclass

    A menu widget is a selection menu. It can be either a pull-down
    menu in a menu bar or a standalone context menu. Pull-down menus
    are shown by the menu bar when the user clicks on the respective
    item or presses the specified shortcut key. Use
    QMenuBar::addMenu() to insert a menu into a menu bar. Context
    menus are usually invoked by some special keyboard key or by
    right-clicking. They can be executed either asynchronously with
    popup() or synchronously with exec(). Menus can also be invoked in
    response to button presses; these are just like context menus
    except for how they are invoked.

    \raw HTML
    <table align="center" cellpadding="0">
    <tr>
        <td>
            \endraw
            \inlineimage plastique-menu.png
            \raw HTML
        </td>
        <td>
            \endraw
            \inlineimage windowsxp-menu.png
            \raw HTML
        </td>
        <td>
            \endraw
            \inlineimage macintosh-menu.png
            \raw HTML
        </td>

    </tr>
    <tr>
        <td colspan="3">
           \endraw
           A menu shown in \l{Plastique Style Widget Gallery}{Plastique widget style},
           \l{Windows XP Style Widget Gallery}{Windows XP widget style},
           and \l{Macintosh Style Widget Gallery}{Macintosh widget style}.
           \raw HTML
        </td>
    </tr>
    </table>
    \endraw

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
    \l{QAction::triggered()}{triggered()}. In addition, QMenu provides
    two signals, activated() and highlighted(), which signal the
    QAction that was triggered from the menu.

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

    See the \l{mainwindows/menus}{Menus} example for an example of how
    to use QMenuBar and QMenu in your application.

    Important inherited functions: addAction(), removeAction(), clear(),
    addSeparator(), and addMenu().

    \sa QMenuBar, {fowler}{GUI Design Handbook: Menu, Drop-Down and Pop-Up},
        {Application Example}, {Menus Example}, {Recent Files Example}
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
    Q_D(QMenu);
    d->init();
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
    Q_D(QMenu);
    d->init();
    d->menuAction->setText(title);
}

/*! \internal
 */
QMenu::QMenu(QMenuPrivate &dd, QWidget *parent)
    : QWidget(dd, parent, Qt::Popup)
{
    Q_D(QMenu);
    d->init();
}

/*!
    Destroys the menu.
*/
QMenu::~QMenu()
{
    Q_D(QMenu);
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
    \l{QAction::triggered()}{triggered()} signal is connected to the
    \a receiver's \a member slot. The function adds the newly created
    action to the menu's list of actions and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QString &text, const QObject *receiver, const char* member, const QKeySequence &shortcut)
{
    QAction *action = new QAction(text, this);
#ifdef QT_NO_SHORTCUT
    Q_UNUSED(shortcut);
#else
    action->setShortcut(shortcut);
#endif
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    \overload

    This convenience function creates a new action with an \a icon and
    some \a text and an optional shortcut \a shortcut. The action's
    \l{QAction::triggered()}{triggered()} signal is connected to the
    \a member slot of the \a receiver object. The function adds the
    newly created action to the menu's list of actions, and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenu::addAction(const QIcon &icon, const QString &text, const QObject *receiver,
                          const char* member, const QKeySequence &shortcut)
{
    QAction *action = new QAction(icon, text, this);
#ifdef QT_NO_SHORTCUT
    Q_UNUSED(shortcut);
#else
    action->setShortcut(shortcut);
#endif
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
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
    action with QAction::isSeparator() returning true, and adds the new
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

    \sa QWidget::insertAction(), addMenu()
*/
QAction *QMenu::insertMenu(QAction *before, QMenu *menu)
{
    QAction *action = menu->menuAction();
    insertAction(before, action);
    return action;
}

/*!
    This convenience function creates a new separator action, i.e. an
    action with QAction::isSeparator() returning true. The function inserts
    the newly created action into this menu's list of actions before
    action \a before and returns it.

    \sa QWidget::insertAction(), addSeparator()
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
    d_func()->defaultAction = act;
}

/*!
  Returns the current default action.

  \sa setDefaultAction()
*/
QAction *QMenu::defaultAction() const
{
    return d_func()->defaultAction;
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
    Q_D(QMenu);
    if (d->tearoff == b)
        return;
    if (!b && d->tornPopup)
        d->tornPopup->close();
    d->tearoff = b;

    d->itemsDirty = true;
    if (isVisible())
        resize(sizeHint());
}

bool QMenu::isTearOffEnabled() const
{
    return d_func()->tearoff;
}

/*!
  When a menu is torn off a second menu is shown to display the menu
  contents in a new window. When the menu is in this mode and the menu
  is visible returns true; otherwise false.

  \sa hideTearOffMenu() isTearOffEnabled()
*/
bool QMenu::isTearOffMenuVisible() const
{
    if (d_func()->tornPopup)
        return d_func()->tornPopup->isVisible();
    return false;
}

/*!
   This function will forcibly hide the torn off menu making it
   disappear from the users desktop.

   \sa isTearOffMenuVisible() isTearOffEnabled()
*/
void QMenu::hideTearOffMenu()
{
    if (d_func()->tornPopup)
        d_func()->tornPopup->close();
}


/*!
  Sets the currently highlighted action to \a act.
*/
void QMenu::setActiveAction(QAction *act)
{
    Q_D(QMenu);
    d->setCurrentAction(act, 0);
    if (d->scroll)
        d->scrollMenu(act, QMenuPrivate::QMenuScroller::ScrollCenter);
}


/*!
    Returns the currently highlighted action, or 0 if no
    action is currently highlighted.
*/
QAction *QMenu::activeAction() const
{
    return d_func()->currentAction;
}

/*!
    \since 4.2

    Returns true if there are no actions inserted into the menu, false
    otherwise.

    \sa QWidget::actions()
*/

bool QMenu::isEmpty() const
{
    return actions().isEmpty();
}

/*!
    Removes all the menu's actions. Actions owned by the menu and not
    shown in any other widget are deleted.

    \sa removeAction()
*/
void QMenu::clear()
{
    QList<QAction*> acts = actions();
    for(int i = 0; i < acts.size(); i++) {
        removeAction(acts[i]);
        if (acts[i]->parent() == this && acts[i]->d_func()->widgets.isEmpty())
            delete acts[i];
    }
}

/*!
  If a menu does not fit on the screen it lays itself out so that it
  does fit. It is style dependent what layout means (for example, on
  Windows it will use multiple columns).

  This functions returns the number of columns necessary.
*/
int QMenu::columnCount() const
{
    return d_func()->ncols;
}

/*!
  Returns the item at \a pt; returns 0 if there is no item there.
*/
QAction *QMenu::actionAt(const QPoint &pt) const
{
    if (QAction *ret = d_func()->actionAt(pt))
        return ret;
    return 0;
}

/*!
  Returns the geometry of action \a act.
*/
QRect QMenu::actionGeometry(QAction *act) const
{
    return d_func()->actionRect(act);
}

/*!
    \reimp
*/
QSize QMenu::sizeHint() const
{
    Q_D(const QMenu);
    ensurePolished();
    QMap<QAction*, QRect> actionRects;
    QList<QAction*> actionList;
    d->calcActionRects(actionRects, actionList);

    QSize s;
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    for (QMap<QAction*, QRect>::const_iterator i = actionRects.constBegin();
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

    s += QSize(d->leftmargin + d->rightmargin, d->topmargin + d->bottommargin);

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
    Q_D(QMenu);
    if (d->scroll) { //reset scroll state from last popup
        d->scroll->scrollOffset = 0;
        d->scroll->scrollFlags = QMenuPrivate::QMenuScroller::ScrollNone;
    }
    d->tearoffHighlighted = 0;
    d->motions = 0;
    d->doChildEffects = true;

    ensurePolished(); // Get the right font
    emit aboutToShow();
    d->updateActions();
    QPoint pos = p;
    QSize size = sizeHint();
    QRect screen = d->popupGeometry(QApplication::desktop()->screenNumber(p));
    const int desktopFrame = style()->pixelMetric(QStyle::PM_MenuDesktopFrameWidth, 0, this);
    if (d->ncols > 1) {
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

                if (d->scroll && d->scroll->scrollFlags != QMenuPrivate::QMenuScroller::ScrollNone
                    && !style()->styleHint(QStyle::SH_Menu_FillScreenWithScroll, 0, this)) {
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
    d->mousePopupPos = mouse;
    const bool snapToMouse = (QRect(p.x()-3, p.y()-3, 6, 6).contains(mouse));

    //handle popup falling "off screen"
    if (qApp->layoutDirection() == Qt::RightToLeft) {
        if(snapToMouse) //position flowing left from the mouse
            pos.setX(mouse.x()-size.width());

        if (pos.x() < screen.left()+desktopFrame)
            pos.setX(qMax(p.x(), screen.left()+desktopFrame));
        if (pos.x()+size.width() > screen.right()-desktopFrame)
            pos.setX(qMax(p.x()-size.width(), screen.right()-desktopFrame-size.width()));
    } else {
        if (pos.x()+size.width() > screen.right()-desktopFrame) {
            pos.setX(qMin(p.x()-size.width(), screen.right()-desktopFrame-size.width()));
        }
        if (pos.x() < screen.left()+desktopFrame)
            pos.setX(qMax(p.x(), screen.left() + desktopFrame));
    }
    if (pos.y() + size.height() - 1 > screen.bottom() - desktopFrame) {
        if(snapToMouse)
            pos.setY(qMin(mouse.y() - (size.height() + desktopFrame), screen.bottom()-desktopFrame-size.height()+1));
        else
            pos.setY(qMax(p.y() - (size.height() + desktopFrame), screen.bottom()-desktopFrame-size.height()+1));
    } else if (pos.y() < screen.top() + desktopFrame) {
        pos.setY(screen.top() + desktopFrame);
    }

    if (pos.y() < screen.top() + desktopFrame)
        pos.setY(screen.top() + desktopFrame);
    if (pos.y()+size.height()-1 > screen.bottom() - desktopFrame) {
        if (d->scroll) {
            d->scroll->scrollFlags |= uint(QMenuPrivate::QMenuScroller::ScrollDown);
            int y = qMax(screen.y(),pos.y());
            size.setHeight(screen.bottom()-(desktopFrame*2)-y);
        } else {
            // Too big for screen, bias to see bottom of menu (for some reason)
            pos.setY(screen.bottom()-size.height()+1);
        }
    }
    setGeometry(QRect(pos, size));

#ifndef QT_NO_EFFECTS
    int hGuess = qApp->layoutDirection() == Qt::RightToLeft ? QEffects::LeftScroll : QEffects::RightScroll;
    int vGuess = QEffects::DownScroll;
    if (qApp->layoutDirection() == Qt::RightToLeft) {
        if ((snapToMouse && (pos.x() + size.width()/2 > mouse.x())) ||
           (qobject_cast<QMenu*>(d->causedPopup.widget) && pos.x() + size.width()/2 > d->causedPopup.widget->x()))
            hGuess = QEffects::RightScroll;
    } else {
        if ((snapToMouse && (pos.x() + size.width()/2 < mouse.x())) ||
           (qobject_cast<QMenu*>(d->causedPopup.widget) && pos.x() + size.width()/2 < d->causedPopup.widget->x()))
            hGuess = QEffects::LeftScroll;
    }

#ifndef QT_NO_MENUBAR
    if ((snapToMouse && (pos.y() + size.height()/2 < mouse.y())) ||
       (qobject_cast<QMenuBar*>(d->causedPopup.widget) &&
        pos.y() + size.width()/2 < d->causedPopup.widget->mapToGlobal(d->causedPopup.widget->pos()).y()))
       vGuess = QEffects::UpScroll;
#endif
    if (QApplication::isEffectEnabled(Qt::UI_AnimateMenu)) {
        bool doChildEffects = true;
#ifndef QT_NO_MENUBAR
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(d->causedPopup.widget)) {
            doChildEffects = mb->d_func()->doChildEffects;
            mb->d_func()->doChildEffects = false;
        } else
#endif
        if (QMenu *m = qobject_cast<QMenu*>(d->causedPopup.widget)) {
            doChildEffects = m->d_func()->doChildEffects;
            m->d_func()->doChildEffects = false;
        }

        if (doChildEffects) {
            if (QApplication::isEffectEnabled(Qt::UI_FadeMenu))
                qFadeEffect(this);
            else if (d->causedPopup.widget)
                qScrollEffect(this, qobject_cast<QMenu*>(d->causedPopup.widget) ? hGuess : vGuess);
            else
                qScrollEffect(this, hGuess | vGuess);
        } else {
            // kill any running effect
            qFadeEffect(0);
            qScrollEffect(0);

            show();
        }
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
    Q_D(QMenu);
    createWinId();
    QEventLoop eventLoop;
    d->eventLoop = &eventLoop;
    popup(p, action);

    QPointer<QObject> guard = this;
    (void) eventLoop.exec();
    if (guard.isNull())
        return 0;

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
    for(QList<QAction*>::ConstIterator it = actions.constBegin(); it != actions.constEnd(); ++it)
        menu.addAction((*it));
    return menu.exec(pos, at);
}

/*!
  \reimp
*/
void QMenu::hideEvent(QHideEvent *)
{
    Q_D(QMenu);
    emit aboutToHide();
    if (d->eventLoop)
        d->eventLoop->exit();
    d->setCurrentAction(0);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::PopupMenuEnd);
#endif
#ifndef QT_NO_MENUBAR
    if (QMenuBar *mb = qobject_cast<QMenuBar*>(d->causedPopup.widget))
        mb->d_func()->setCurrentAction(0);
#endif
    d->mouseDown = 0;
    d->hasHadMouse = false;
    d->causedPopup.widget = 0;
    d->causedPopup.action = 0;
}

/*!
  \reimp
*/
void QMenu::paintEvent(QPaintEvent *e)
{
    Q_D(QMenu);
    QPainter p(this);
    QRegion emptyArea = QRegion(rect());

    //draw the items that need updating..
    for (int i = 0; i < d->actionList.count(); ++i) {
        QAction *action = d->actionList.at(i);
        QRect adjustedActionRect = d->actionRect(action);
        if (!e->rect().intersects(adjustedActionRect)
            || d->widgetItems.value(action))
           continue;
        //set the clip region to be extra safe (and adjust for the scrollers)
        QRegion adjustedActionReg(adjustedActionRect);
        emptyArea -= adjustedActionReg;
        p.setClipRegion(adjustedActionReg);

        QStyleOptionMenuItem opt;
        initStyleOption(&opt, action);
        opt.rect = adjustedActionRect;
        style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
    }

    const int fw = style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this);
    QStyleOptionMenuItem menuOpt;
    menuOpt.initFrom(this);
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
        menuOpt.state |= QStyle::State_Enabled;
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
            menuOpt.state |= QStyle::State_DownArrow;
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
        frame.lineWidth = style()->pixelMetric(QStyle::PM_MenuPanelWidth);
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
    Q_D(QMenu);
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
    Q_D(QMenu);
    if (d->mouseEventTaken(e))
        return;
    if (!rect().contains(e->pos())) {
         if (d->noReplayFor
             && QRect(d->noReplayFor->mapToGlobal(QPoint()), d->noReplayFor->size()).contains(e->globalPos()))
             setAttribute(Qt::WA_NoMouseReplay);
         if (d->eventLoop) // synchronous operation
             d->syncAction = 0;
        d->hideUpToMenuBar();
        return;
    }
    d->mouseDown = this;

    QAction *action = d->actionAt(e->pos());
    d->setCurrentAction(action, 20);
    update();
}

/*!
  \reimp
*/
void QMenu::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QMenu);
    if (d->mouseEventTaken(e))
        return;
    if(d->mouseDown != this) {
        d->mouseDown = 0;
        return;
    }

    d->mouseDown = 0;
    d->setSyncAction();
    QAction *action = d->actionAt(e->pos());

    if (action && action == d->currentAction) {
        if (action->menu())
            action->menu()->d_func()->setFirstActionActive();
        else {
#ifdef Q_WS_WIN
#ifndef QT_NO_MENUBAR
            //On Windows only context menus can be activated with the right button
            bool isContextMenu = true;
            const QWidget *parent = parentWidget();
            while (parent) {
                if (qobject_cast<const QMenuBar *>(parent)) {
                    isContextMenu = false;
                    break;
                }
                parent = parent->parentWidget();
            }
            if (e->button() == Qt::LeftButton || (e->button() == Qt::RightButton && isContextMenu))
#endif
#endif
                d->activateAction(action, QAction::Trigger);
        }
    } else if (d->hasMouseMoved(e->globalPos())) {
        d->hideUpToMenuBar();
    }
}

/*!
  \reimp
*/
void QMenu::changeEvent(QEvent *e)
{
    Q_D(QMenu);
    if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange ||
        e->type() == QEvent::LayoutDirectionChange) {
        d->itemsDirty = 1;
        setMouseTracking(style()->styleHint(QStyle::SH_Menu_MouseTracking, 0, this));
        if (isVisible())
            resize(sizeHint());
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
#ifdef Q_WS_MAC
        d->setMacMenuEnabled(isEnabled());
#endif
    }
    QWidget::changeEvent(e);
}


/*!
  \reimp
*/
bool
QMenu::event(QEvent *e)
{
    Q_D(QMenu);
    switch (e->type()) {
    case QEvent::ShortcutOverride: {
            QKeyEvent *kev = static_cast<QKeyEvent*>(e);
            if (kev->key() == Qt::Key_Up || kev->key() == Qt::Key_Down
                || kev->key() == Qt::Key_Left || kev->key() == Qt::Key_Right
                || kev->key() == Qt::Key_Enter || kev->key() == Qt::Key_Return
                || kev->key() == Qt::Key_Escape) {
                e->accept();
                return true;
            }
        }
        break;
    case QEvent::KeyPress: {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            keyPressEvent(ke);
            return true;
        }
    } break;
    case QEvent::ContextMenu:
        if(QMenuPrivate::menuDelayTimer.isActive()) {
            QMenuPrivate::menuDelayTimer.stop();
            internalDelayedPopup();
        }
        break;
    case QEvent::Resize:
        d->itemsDirty = 1;
        d->updateActions();
        break;
    case QEvent::Show:
        d->mouseDown = 0;
        d->updateActions();
        break;
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis:
        e->setAccepted(d->whatsThis.size());
        if (QAction *action = d->actionAt(static_cast<QHelpEvent*>(e)->pos())) {
            if (action->whatsThis().size() || action->menu())
                e->accept();
        }
        return true;
#endif
    default:
        break;
    }
    return QWidget::event(e);
}

/*!
    \reimp
*/
bool QMenu::focusNextPrevChild(bool next)
{
    setFocus();
    QKeyEvent ev(QEvent::KeyPress, next ? Qt::Key_Tab : Qt::Key_Backtab, Qt::NoModifier);
    keyPressEvent(&ev);
    return true;
}

/*!
  \reimp
*/
void QMenu::keyPressEvent(QKeyEvent *e)
{
    Q_D(QMenu);
    int key = e->key();
    if (isRightToLeft()) {  // in reverse mode open/close key for submenues are reversed
        if (key == Qt::Key_Left)
            key = Qt::Key_Right;
        else if (key == Qt::Key_Right)
            key = Qt::Key_Left;
    }
#ifndef Q_WS_MAC
    if (key == Qt::Key_Tab) //means down
        key = Qt::Key_Down;
    if (key == Qt::Key_Backtab) //means up
        key = Qt::Key_Up;
#endif

    bool key_consumed = false;
    switch(key) {
    case Qt::Key_Home:
        key_consumed = true;
        if (d->scroll)
            d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollTop, true);
        break;
    case Qt::Key_End:
        key_consumed = true;
        if (d->scroll)
            d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollBottom, true);
        break;
    case Qt::Key_PageUp:
        key_consumed = true;
        if (d->currentAction && d->scroll) {
            if(d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollUp)
                d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollUp, true, true);
            else
                d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollTop, true);
        }
        break;
    case Qt::Key_PageDown:
        key_consumed = true;
        if (d->currentAction && d->scroll) {
            if(d->scroll->scrollFlags & QMenuPrivate::QMenuScroller::ScrollDown)
                d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollDown, true, true);
            else
                d->scrollMenu(QMenuPrivate::QMenuScroller::ScrollBottom, true);
        }
        break;
    case Qt::Key_Up:
    case Qt::Key_Down: {
        key_consumed = true;
        QAction *nextAction = 0;
        QMenuPrivate::QMenuScroller::ScrollLocation scroll_loc = QMenuPrivate::QMenuScroller::ScrollStay;
        if (!d->currentAction) {
            if(key == Qt::Key_Down) {
                for(int i = 0; i < d->actionList.size(); ++i) {
                    QAction *act = d->actionList.at(i);
                    if (!act->isSeparator() &&
                        (style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this)
                         || act->isEnabled())) {
                        nextAction = act;
                        break;
                    }
                }
            } else {
                for(int i = d->actionList.size()-1; i >= 0; --i) {
                    QAction *act = d->actionList.at(i);
                    if (!act->isSeparator() &&
                        (style()->styleHint(QStyle::SH_Menu_AllowActiveAndDisabled, 0, this)
                         || act->isEnabled())) {
                        nextAction = act;
                        break;
                    }
                }
            }
        } else {
            for(int i=0, y=0; !nextAction && i < (int)d->actionList.count(); i++) {
                QAction *act = d->actionList.at(i);
                if (act == d->currentAction) {
                    if (key == Qt::Key_Up) {
                        for(int next_i = i-1; true; next_i--) {
                            if (next_i == -1) {
                                if(!style()->styleHint(QStyle::SH_Menu_SelectionWrap, 0, this))
                                    break;
                                if (d->scroll)
                                    scroll_loc = QMenuPrivate::QMenuScroller::ScrollBottom;
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
                                if(!style()->styleHint(QStyle::SH_Menu_SelectionWrap, 0, this))
                                    break;
                                if (d->scroll)
                                    scroll_loc = QMenuPrivate::QMenuScroller::ScrollTop;
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
                                    bottomVisible -= style()->pixelMetric(QStyle::PM_MenuTearoffHeight, 0, this);
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
            d->setCurrentAction(nextAction, /*popup*/-1, QMenuPrivate::SelectedFromKeyboard);
        }
        break; }

    case Qt::Key_Right:
        if (!d->currentAction) {
            d->setFirstActionActive();
            key_consumed = true;
            break;
        }
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
                d->setCurrentAction(nextAction, /*popup*/-1, QMenuPrivate::SelectedFromKeyboard);
                key_consumed = true;
            }
        }
        if (!key_consumed && key == Qt::Key_Left && d->causedPopup.widget &&
            qobject_cast<QMenu*>(d->causedPopup.widget)) {
            QPointer<QWidget> caused = d->causedPopup.widget;
            d->hideMenu(this);
            if (caused)
                caused->setFocus();
            key_consumed = true;
        }
        break; }

    case Qt::Key_Alt:
        if (d->tornoff)
            break;

        key_consumed = true;
        if (style()->styleHint(QStyle::SH_MenuBar_AltKeyNavigation, 0, this))
        {
            d->hideMenu(this);
#ifndef QT_NO_MENUBAR
            if (QMenuBar *mb = qobject_cast<QMenuBar*>(qApp->focusWidget())) {
                mb->d_func()->setKeyboardMode(false);
            }
#endif
        }
        break;

    case Qt::Key_Escape:
#ifdef QT_KEYPAD_NAVIGATION
    case Qt::Key_Back:
#endif
        key_consumed = true;
        if (d->tornoff) {
            close();
            return;
        }
        {
            QPointer<QWidget> caused = d->causedPopup.widget;
            d->hideMenu(this); // hide after getting causedPopup
#ifndef QT_NO_MENUBAR
            if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
                mb->d_func()->setCurrentAction(d->menuAction);
                mb->d_func()->setKeyboardMode(true);
            }
#endif
        }
        break;

    case Qt::Key_Space:
        if (!style()->styleHint(QStyle::SH_Menu_SpaceActivatesItem, 0, this))
            break;
        // for motif, fall through
#ifdef QT_KEYPAD_NAVIGATION
    case Qt::Key_Select:
#endif
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        if (!d->currentAction) {
            d->setFirstActionActive();
            key_consumed = true;
            break;
        }

        d->setSyncAction();

        if (d->currentAction->menu())
            d->popupAction(d->currentAction, 0, true);
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
            bool activateAction = false;
            QAction *nextAction = 0;
            if (style()->styleHint(QStyle::SH_Menu_KeyboardSearch, 0, this) && !e->modifiers()) {
                int best_match_count = 0;
                d->searchBufferTimer.start(2000, this);
                d->searchBuffer += e->text();
                for(int i = 0; i < d->actionList.size(); ++i) {
                    int match_count = 0;
                    register QAction *act = d->actionList.at(i);
                    const QString act_text = act->text();
                    for(int c = 0; c < d->searchBuffer.size(); ++c) {
                        if(act_text.indexOf(d->searchBuffer.at(c), 0, Qt::CaseInsensitive) != -1)
                            ++match_count;
                    }
                    if(match_count > best_match_count) {
                        best_match_count = match_count;
                        nextAction = act;
                    }
                }
            }
#ifndef QT_NO_SHORTCUT
            else {
                int clashCount = 0;
                QAction *first = 0, *currentSelected = 0, *firstAfterCurrent = 0;
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
                if (clashCount == 1)
                    activateAction = true;
                if (clashCount >= 1) {
                    if (clashCount == 1 || !currentSelected || !firstAfterCurrent)
                        nextAction = first;
                    else
                        nextAction = firstAfterCurrent;
                }
            }
#endif
            if (nextAction) {
                key_consumed = true;
                if(d->scroll)
                    d->scrollMenu(nextAction, QMenuPrivate::QMenuScroller::ScrollCenter, false);
                d->setCurrentAction(nextAction, 20, QMenuPrivate::SelectedFromElsewhere, true);
                if (!nextAction->menu() && activateAction) {
                    d->setSyncAction();
                    d->activateAction(nextAction, QAction::Trigger);
                }
            }
        }
        if (!key_consumed) {
            if (QWidget *caused = d->causedPopup.widget) {
                while(QMenu *m = qobject_cast<QMenu*>(caused))
                    caused = m->d_func()->causedPopup.widget;
#ifndef QT_NO_MENUBAR
                if (QMenuBar *mb = qobject_cast<QMenuBar*>(caused)) {
                    QAction *oldAct = mb->d_func()->currentAction;
                    QApplication::sendEvent(mb, e);
                    if (mb->d_func()->currentAction != oldAct)
                        key_consumed = true;
                }
#endif
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
    Q_D(QMenu);
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
    } else if(e->buttons() & (Qt::LeftButton | Qt::RightButton)) {
        d->mouseDown = this;
    }
    if (d->sloppyRegion.contains(e->pos())) {
        d->sloppyAction = action;
        QMenuPrivate::sloppyDelayTimer.start(style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, 0, this)*6, this);
    } else {
        d->setCurrentAction(action, style()->styleHint(QStyle::SH_Menu_SubMenuPopupDelay, 0, this));
    }
}

/*!
  \reimp
*/
void QMenu::enterEvent(QEvent *)
{
    d_func()->motions = -1; // force us to ignore the generate mouse move in mouseMoveEvent()
}

/*!
  \reimp
*/
void QMenu::leaveEvent(QEvent *)
{
    Q_D(QMenu);
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
    Q_D(QMenu);
    if (d->scroll && d->scroll->scrollTimer && d->scroll->scrollTimer->timerId() == e->timerId()) {
        d->scrollMenu((QMenuPrivate::QMenuScroller::ScrollDirection)d->scroll->scrollDirection);
        if (d->scroll->scrollFlags == QMenuPrivate::QMenuScroller::ScrollNone)
            d->scroll->scrollTimer->stop();
    } else if(QMenuPrivate::menuDelayTimer.timerId() == e->timerId()) {
        QMenuPrivate::menuDelayTimer.stop();
        internalDelayedPopup();
    } else if(QMenuPrivate::sloppyDelayTimer.timerId() == e->timerId()) {
        QMenuPrivate::sloppyDelayTimer.stop();
        internalSetSloppyAction();
    } else if(d->searchBufferTimer.timerId() == e->timerId()) {
        d->searchBuffer.clear();
    }
}

/*!
  \reimp
*/
void QMenu::actionEvent(QActionEvent *e)
{
    Q_D(QMenu);
    d->itemsDirty = 1;
    setAttribute(Qt::WA_Resized, false);
    if (d->tornPopup)
        d->tornPopup->syncWithMenu(this, e);
    if (e->type() == QEvent::ActionAdded) {
        if(!d->tornoff) {
            connect(e->action(), SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
            connect(e->action(), SIGNAL(hovered()), this, SLOT(_q_actionHovered()));
        }

        if (QWidgetAction *wa = qobject_cast<QWidgetAction *>(e->action())) {
            QWidget *widget = wa->requestWidget(this);
            if (widget)
                d->widgetItems.insert(wa, widget);
        }
    } else if (e->type() == QEvent::ActionRemoved) {
        d->actionRects.clear();
        d->actionList.clear();
        e->action()->disconnect(this);
        if (e->action() == d->currentAction)
            d->currentAction = 0;
        if (QWidgetAction *wa = qobject_cast<QWidgetAction *>(e->action())) {
            QWidget *widget = d->widgetItems.take(wa);
            if (widget)
                wa->releaseWidget(widget);
        }
    }

#ifdef Q_WS_MAC
    if (d->mac_menu) {
        if (e->type() == QEvent::ActionAdded)
            d->mac_menu->addAction(e->action(), d->mac_menu->findAction(e->before()), d);
        else if (e->type() == QEvent::ActionRemoved)
            d->mac_menu->removeAction(e->action());
        else if (e->type() == QEvent::ActionChanged)
            d->mac_menu->syncAction(e->action());
    }
#endif

    if (isVisible()) {
        d->updateActions();
	resize(sizeHint());
        update();
    }
}

/*!
  \internal
*/
void QMenu::internalSetSloppyAction()
{
    if (d_func()->sloppyAction)
        d_func()->setCurrentAction(d_func()->sloppyAction, 0);
}

/*!
  \internal
*/
void QMenu::internalDelayedPopup()
{
    Q_D(QMenu);

    //hide the current item
    if (QMenu *menu = d->activeMenu) {
        d->activeMenu = 0;
        d->hideMenu(menu);
    }

    if (!d->currentAction || !d->currentAction->isEnabled() || !d->currentAction->menu() ||
        !d->currentAction->menu()->isEnabled() || d->currentAction->menu()->isVisible())
        return;

    //setup
    d->activeMenu = d->currentAction->menu();
    d->activeMenu->d_func()->causedPopup.widget = this;
    d->activeMenu->d_func()->causedPopup.action = d->currentAction;

    const QRect actionRect(d->actionRect(d->currentAction));
    const QSize menuSize(d->activeMenu->sizeHint());
    const QPoint rightPos(mapToGlobal(QPoint(actionRect.right(), actionRect.top())));
    const QPoint leftPos(mapToGlobal(QPoint(actionRect.left() - menuSize.width(), actionRect.top())));

    QPoint pos(rightPos);
    QMenu *caused = qobject_cast<QMenu*>(d->activeMenu->d_func()->causedPopup.widget);

    const QRect availGeometry(d->popupGeometry(QApplication::desktop()->screenNumber(caused)));
    if (isRightToLeft()) {
        pos = leftPos;
        if (caused && caused->x() < x() || pos.x() < availGeometry.left()) {
            if(rightPos.x() + menuSize.width() < availGeometry.right())
                pos = rightPos;
            else
                pos.rx() = availGeometry.left();
        }
    } else {
        if (caused && caused->x() > x() || pos.x() + menuSize.width() > availGeometry.right()) {
            if(leftPos.x() < availGeometry.left())
                pos.rx() = availGeometry.right() - menuSize.width();
            else
                pos = leftPos;
        }
    }

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
    \fn void QMenu::addAction(QAction *action)
    \overload

    Appends the action \a action to the menu's list of actions.

    \sa QMenuBar::addAction(), QWidget::addAction()
*/

/*!
    \fn void QMenu::aboutToHide()
    \since 4.2

    This signal is emitted just before the menu is hidden from the user.

    \sa aboutToShow(), hide()
*/

/*!
    \fn void QMenu::aboutToShow()

    This signal is emitted just before the menu is shown to the user.

    \sa aboutToHide(), show()
*/

/*!
    \fn void QMenu::triggered(QAction *action)

    This signal is emitted when an action in this menu is triggered; \a action
    is the action that caused the signal to be emitted.

    Normally, you connect each menu action's \l{QAction::}{triggered()} signal to its
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
#ifdef Q_WS_WIN
    d_func()->noReplayFor = noReplayFor;
#else
    Q_UNUSED(noReplayFor);
#endif
}

/*!
  \property QMenu::separatorsCollapsible
  \since 4.2

  \brief whether consecutive separators should be collapsed

  This property specifies whether consecutive separators in the menu
  should be visually collapsed to a single one. Separators at the
  beginning or the end of the menu are also hidden.
*/
bool QMenu::separatorsCollapsible() const
{
    Q_D(const QMenu);
    return d->collapsibleSeparators;
}

void QMenu::setSeparatorsCollapsible(bool collapse)
{
    Q_D(QMenu);
    d->collapsibleSeparators = collapse;
    d->itemsDirty = 1;
    if (isVisible()) {
        d->updateActions();
        update();
    }
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
    if (index == -1 || index >= actions().count())
        addAction(act);
    else
        insertAction(actions().value(index), act);
    return findIdForAction(act);
}

/*!
    Use insertAction() or one of the addAction() overloads instead.
*/
int QMenu::insertItem(QMenuItem *item, int id, int index)
{
    if (index == -1 || index >= actions().count())
        addAction(item);
    else
        insertAction(actions().value(index), item);
    if (id > -1)
        item->d_func()->id = id;
    return findIdForAction(item);
}

/*!
    Use the insertSeparator() overload that takes a QAction *
    parameter instead.
*/
int QMenu::insertSeparator(int index)
{
    QAction *act = new QAction(this);
    act->setSeparator(true);
    if (index == -1 || index >= actions().count())
        addAction(act);
    else
        insertAction(actions().value(index), act);
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
    Use QAction and actions() instead.
*/
QMenuItem *QMenu::findPopup( QMenu *popup, int *index )
{
   QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *act = list.at(i);
        if (act->menu() == popup) {
            QMenuItem *item = static_cast<QMenuItem *>(act);
            if (index)
                *index = act->d_func()->id;
            return item;
        }
    }
    return 0;
}


/*!
    Use QAction::setData() instead.
*/
bool QMenu::setItemParameter(int id, int param)
{
    if (QAction *act = findActionForId(id)) {
        act->d_func()->param = param;
        return true;
    }
    return false;
}

/*!
    Use QAction::data() instead.
*/
int QMenu::itemParameter(int id) const
{
    if (QAction *act = findActionForId(id))
        return act->d_func()->param;
    return id;
}

/*!
    Use actions instead.
*/
void QMenu::setId(int index, int id)
{
    if(QAction *act = actions().value(index))
        act->d_func()->id = id;
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
    return act->d_func()->id;
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

    Use actions instead.
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

    Use actions instead.
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

    Use actions instead.
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

    Use actionGeometry() instead.
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

/*!
    \fn void QMenu::setActiveItem(int id)

    Use setActiveAction() instead.
*/

QT_END_NAMESPACE

// for private slots
#include "moc_qmenu.cpp"
#include "qmenu.moc"

#endif // QT_NO_MENU
