#include <qmenubar.h>
#include <qstyle.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include <qapplication.h>
#include "qdesktopwidget.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
# include "qaccessible.h"
#endif
#include <qpainter.h>
#include <qevent.h>

#include "qmenu_p.h"
#include "qmenubar_p.h"
#define d d_func()
#define q q_func()

QMenuAction *QMenuBarPrivate::actionAt(QPoint p) const
{
    for(int i = 0; i < actionItems.count(); i++) {
        QMenuAction *act = actionItems[i];
        if(actionRect(act).contains(p))
            return act;
    }
    return 0;
}

void QMenuBarPrivate::updateActions()
{
    if(!itemsDirty)
        return;
    int q_width = q->width()-(q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q)*2),
        q_start = -1;
    if(d->leftWidget || d->rightWidget) {
        int vmargin = q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q) + q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q),
            hmargin = q->style().pixelMetric(QStyle::PM_MenuBarHMargin, q);
        if(d->leftWidget && d->leftWidget->isVisible()) {
            QSize sz = d->leftWidget->sizeHint();
            q_width -= sz.width();
            d->leftWidget->setGeometry(QRect(QPoint(hmargin, vmargin), sz));
        }
        if(d->rightWidget && d->rightWidget->isVisible()) {
            QSize sz = d->rightWidget->sizeHint();
            q_width -= sz.width();
            q_start = sz.width();
            d->rightWidget->setGeometry(QRect(QPoint(q->width()-sz.width()-hmargin, vmargin), sz));
        }
    }
    if(itemsWidth == q_width && q_start == itemsStart)
        return;

#ifdef Q_WS_MAC
    if(d->mac_menubar) {//nothing to see here folks, move along..
        itemsDirty = 0;
        return;
    }
#endif
    actionItems = calcActionRects(q_width, q_start);
    itemsWidth = q_width;
    itemsStart = q_start;
#ifndef QT_NO_ACCEL
    if(itemsDirty) {
        delete shortcuts;
        shortcuts = new QAccel(q);
        QObject::connect(shortcuts, SIGNAL(activated(int)), q, SLOT(internalShortcutActivated(int)));
        for(int i = 0; i < actionItems.count(); i++) {
            QKeySequence key = QAccel::shortcutKey(actionItems.at(i)->action->text());
            if(!key.isEmpty())
                shortcuts->insertItem(key);
        }
    }
#endif
    itemsDirty = 0;

#ifndef QT_NO_MAINWINDOW
    QMainWindow *mw = qt_cast<QMainWindow*>(q->parent());
    if(mw) {
        mw->triggerLayout();
        mw->update();
    }
#endif
#ifndef QT_NO_LAYOUT
    if(q->parentWidget() && q->parentWidget()->layout())
        q->parentWidget()->layout()->activate();
#endif
}

QRect QMenuBarPrivate::actionRect(QMenuAction *act) const
{
    QRect ret = act->rect;
    const int fw = q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q);
    ret.moveBy(fw, fw);
    return ret;
}

void QMenuBarPrivate::setKeyboardMode(bool b)
{
    d->altPressed = false;
    d->keyboardState = b;
    if(b) {
        d->keyboardFocusWidget = qApp->focusWidget();
        q->setFocus();
    } else {
        if(d->keyboardFocusWidget) {
            d->keyboardFocusWidget->setFocus();
            d->keyboardFocusWidget = 0;
        }
    }
}

void QMenuBarPrivate::popupAction(QMenuAction *action, bool activateFirst)
{
    popupState = true;
    if(action && action->action->menu()) {
        closePopupMode = 0;
        activeMenu = action->action->menu();
        activeMenu->d->causedPopup = q;
        if(activeMenu->parent() != q)
            activeMenu->setParent(q, activeMenu->getWFlags());

        QRect dh = QApplication::desktop()->availableGeometry();
        QRect adjustedActionRect = actionRect(action);
        QPoint pos(q->mapToGlobal(QPoint(adjustedActionRect.left(), adjustedActionRect.bottom())));
        QSize popup_size = activeMenu->sizeHint();
        if(QApplication::reverseLayout()) {
            pos.setX((pos.x()+adjustedActionRect.width())-popup_size.width());
            if(pos.x() < 0)
                pos.setX(0);
        } else {
            const int off = pos.x()+popup_size.width() - dh.right();
            if(off > 0)
                pos.setX(qMax(0, pos.x()-off));
        }
        if(!defaultPopDown || (pos.y() + popup_size.height() > dh.bottom()))
            pos.setY(qMax(dh.y(), q->mapToGlobal(QPoint(0, adjustedActionRect.top()-popup_size.height())).y()));
        activeMenu->popup(pos);
        if(activateFirst)
            activeMenu->d->setFirstActionActive();
        q->update(actionRect(action));
    }
}

void QMenuBarPrivate::setCurrentAction(QMenuAction *action, bool popup, bool activateFirst)
{
    if(currentAction == action && popup == popupState)
        return;
    if(activeMenu) {
        QMenu *menu = activeMenu;
        activeMenu = NULL;
        menu->hide();
    }
    if(currentAction)
        q->update(actionRect(currentAction));

    popupState = popup;
    currentAction = action;
    if(action) {
        activateAction(action->action, QAction::Hover);
        if(popup)
            popupAction(action, activateFirst);
        q->update(actionRect(action));
    }
}

QList<QMenuAction*> QMenuBarPrivate::calcActionRects(int max_width, int start) const
{
    if(!itemsDirty && itemsWidth == max_width && itemsStart == start)
        return actionItems;
    QList<QMenuAction*> ret;
    int max_item_height = 0, separator = -1, separator_start = 0, separator_len = 0;
    QList<QAction*> items = q->actions();

    //calculate size
    const QFontMetrics fm = q->fontMetrics();
    for(int i = 0; i < items.count(); i++) {
        QAction *action = items.at(i);
        if(!action->isVisible())
            continue;

        QSize sz;

        //calc what I think the size is..
        if(action->isSeparator()) {
            if(q->style().styleHint(QStyle::SH_GUIStyle, q) == MotifStyle)
                separator = ret.count();
            continue; //we don't really position these!
        } else {
            QString s = action->text();
            int w = fm.width(s);
            w -= s.count('&') * fm.width('&');
            w += s.count("&&") * fm.width('&');
            sz.setWidth(w);
            sz.setHeight(fm.height());
        }

        //let the style modify the above size..
        sz = q->style().sizeFromContents(QStyle::CT_MenuBarItem, q, sz, QStyleOption(action));

        if(!sz.isEmpty()) {
            if(separator == -1)
                separator_start += sz.width();
            else
                separator_len += sz.width();
            max_item_height = qMax(max_item_height, sz.height());
            //append
            QMenuAction *item = new QMenuAction;
            item->action = action;
            item->rect = QRect(0, 0, sz.width(), sz.height());
            ret.append(item);
        }
    }

    //calculate position
    const int hmargin = q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q),
              vmargin = q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q);
    const int itemSpacing = q->style().pixelMetric(QStyle::PM_MenuBarItemSpacing, q);
    const bool reverse = QApplication::reverseLayout();
    int x = start == -1 ? hmargin : start + itemSpacing, y = vmargin;
    for(int i = 0; i != ret.count(); i++) {
        QMenuAction *item = ret.at(i);
        //resize
        item->rect.setHeight(max_item_height);

        //move
        if(separator != -1 && i >= separator) { //after the separator
            int left = (max_width - separator_len - hmargin) + (x - separator_start);
            if(left < separator_start) { //wrap
                separator_start = x = hmargin;
                y += max_item_height;
            }
            item->rect.moveLeft(left);
        } else {
            if(x+item->rect.width() >= max_width-vmargin) { //wrap
                y += max_item_height;
                separator_start -= x;
                x = hmargin;
            } else {
                item->rect.moveLeft(x);
            }
        }
        if(reverse)
            item->rect.moveLeft(max_width - item->rect.right());
        item->rect.moveTop(y);

        //keep moving along..
        x += item->rect.width() + itemSpacing;
    }
    return ret;
}

void QMenuBarPrivate::activateAction(QAction *action, QAction::ActionEvent action_e)
{
    if(!action)
        return;
    action->activate(action_e);
    if(action_e == QAction::Trigger)
        emit q->activated(action);
    else if(action_e == QAction::Hover)
        emit q->highlighted(action);
}

QMenuBar::QMenuBar(QWidget *parent) : QWidget(*new QMenuBarPrivate, parent, 0)
{
#ifdef Q_WS_MAC
    d->macCreateMenuBar(parent);
#endif
    setBackgroundRole(QPalette::Button);
    if(parent) {
        topLevelWidget()->installEventFilter(this); //grab accels (and maybe resizes)
        if(!parent->isTopLevel())
            parent->installEventFilter(this); //handle resizes
    }
    setMouseTracking(style().styleHint(QStyle::SH_MenuBar_MouseTracking));
}

QMenuBar::~QMenuBar()
{
#ifdef Q_WS_MAC
    d->macDestroyMenuBar();
#endif
}

QAction *QMenuBar::addMenu(const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, menu);
    addAction(ret);
    return ret;
}

QAction *QMenuBar::insertMenu(QAction *before, const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, menu);
    insertAction(before, ret);
    return ret;
}

QAction *QMenuBar::activeAction() const
{
    return d->currentAction->action;
}

void QMenuBar::clear()
{
    QList<QAction*> acts = actions();
    for(int i = 0; i < acts.size(); i++)
        removeAction(acts[i]);
}

void QMenuBar::setDefaultUp(bool b)
{
    d->defaultPopDown = !b;
}

bool QMenuBar::isDefaultUp() const
{
    return !d->defaultPopDown;
}

void QMenuBar::resizeEvent(QResizeEvent *)
{
    d->itemsDirty = 1;
}

void QMenuBar::paintEvent(QPaintEvent *e)
{
    d->updateActions();

    QPainter p(this);
    QRegion emptyArea(rect());

    //draw the items
    for(int i=0; i<(int)d->actionItems.count(); i++) {
        QMenuAction *action = d->actionItems.at(i);
        QRect adjustedActionRect = d->actionRect(action);
        if(!e->rect().intersects(adjustedActionRect))
           continue;

        QPalette pal = palette();
        QStyle::SFlags flags = QStyle::Style_Default;
        if(isEnabled() && action->action->isEnabled())
            flags |= QStyle::Style_Enabled;
        else
            pal.setCurrentColorGroup(QPalette::Disabled);
        if(d->currentAction == action) {
            flags |= QStyle::Style_Active;
            if(d->popupState && !d->closePopupMode)
                flags |= QStyle::Style_Down;
        }
        if(hasFocus() || d->currentAction)
            flags |= QStyle::Style_HasFocus;
        emptyArea -= adjustedActionRect;
        p.setClipRect(adjustedActionRect);
        style().drawControl(QStyle::CE_MenuBarItem, &p, this,
                            adjustedActionRect, pal, flags, QStyleOption(action->action));
    }
     //draw border
    if(int fw = q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q)) {
        QRegion borderReg;
        borderReg += QRect(0, 0, fw, height()); //left
        borderReg += QRect(width()-fw, 0, fw, height()); //right
        borderReg += QRect(0, 0, width(), fw); //top
        borderReg += QRect(0, height()-fw, width(), fw); //bottom
        p.setClipRegion(borderReg);
        emptyArea -= borderReg;
        style().drawPrimitive(QStyle::PE_MenuBarFrame, &p, rect(), palette(), QStyle::Style_Default);
    }
    p.setClipRegion(emptyArea);
    style().drawControl(QStyle::CE_MenuBarEmptyArea, &p, this, rect(), palette());
}

void QMenuBar::mousePressEvent(QMouseEvent *e)
{
    if(e->button() != LeftButton)
        return;
    d->mouseDown = true;
    QMenuAction *action = d->actionAt(e->pos());
    if(d->currentAction == action && d->popupState) {
        if((d->closePopupMode = (style().styleHint(QStyle::SH_GUIStyle) == WindowsStyle)))
            q->update(d->actionRect(action));
    } else {
        d->setCurrentAction(action, true);
    }
}

void QMenuBar::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() != LeftButton || !d->mouseDown)
        return;
    d->mouseDown = false;
    QMenuAction *action = d->actionAt(e->pos());
    if((d->closePopupMode && action == d->currentAction) || !action || !action->action->menu()) {
        if(action)
            d->activateAction(action->action, QAction::Trigger);
        d->setCurrentAction(action, false);
    }
    d->closePopupMode = 0;
}

void QMenuBar::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if(QApplication::reverseLayout()) {  // in reverse mode open/close key for submenues are reversed
        if(key == Key_Left)
            key = Key_Right;
        else if(key == Key_Right)
            key = Key_Left;
    }
    if(key == Key_Tab) //means right
        key = Key_Right;

    bool key_consumed = false;
    switch(key) {
    case Key_Up:
    case Key_Down:
    case Key_Enter:
    case Key_Space:
    case Key_Return: {
        if(!style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this) || !d->currentAction)
           break;
        if(d->currentAction->action->menu()) {
            d->popupAction(d->currentAction, true);
        } else if(key == Key_Enter || key == Key_Return || key == Key_Space) {
            d->activateAction(d->currentAction->action, QAction::Trigger);
            d->setCurrentAction(d->currentAction, false);
        }
        key_consumed = true;
        break; }

    case Key_Right:
    case Key_Left: {
        if(d->currentAction) {
            QMenuAction *nextAction = 0;
            for(int i=0; i<(int)d->actionItems.count(); i++) {
                if(d->actionItems.at(i) == d->currentAction) {
                    if(key == Key_Left) {
                        if(i > 0)
                            nextAction = d->actionItems.at(i-1);
                    } else {
                        if(i < d->actionItems.count()-1)
                            nextAction = d->actionItems.at(i+1);
                    }
                    break;
                }
            }
            if(!nextAction) {
                if(key == Key_Left)
                    nextAction = d->actionItems.last();
                else
                    nextAction = d->actionItems.first();
            }
            if(nextAction) {
                d->setCurrentAction(nextAction, d->popupState, true);
                key_consumed = true;
            }
        }
        break; }

    case Key_Escape:
        d->setCurrentAction(0);
        d->setKeyboardMode(false);
        key_consumed = true;
        break;

    default:
        key_consumed = false;
    }

    if(!key_consumed &&
       (!e->state() || (e->state()&(MetaButton|AltButton))) && e->text().length()==1 && !d->popupState) {
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
        if(next_action)
            d->setCurrentAction(next_action, true, true);
    }
}

void QMenuBar::mouseMoveEvent(QMouseEvent *e)
{
    d->mouseDown = e->state() & LeftButton;
    QMenuAction *action = d->actionAt(e->pos());
    bool popupState = d->popupState || d->mouseDown;
    if(action || !popupState)
        d->setCurrentAction(action, popupState);
}

void QMenuBar::leaveEvent(QEvent *)
{
    if(!hasFocus() && !d->popupState)
        d->setCurrentAction(0);
}

void QMenuBar::actionEvent(QActionEvent *e)
{
    d->itemsDirty = 1;
#ifdef Q_WS_MAC
    if(d->mac_menubar) {
        if(e->type() == QEvent::ActionAdded)
            d->mac_menubar->addAction(e->action(), d->mac_menubar->findAction(e->before()));
        else if(e->type() == QEvent::ActionRemoved)
            d->mac_menubar->removeAction(e->action());
        else if(e->type() == QEvent::ActionChanged)
            d->mac_menubar->syncAction(e->action());
    }
#else
    (void) e;
#endif
    if(isVisible())
        update();
}

void
QMenuBar::focusInEvent(QFocusEvent *)
{
    if(!d->currentAction && !d->actionItems.isEmpty())
        d->setCurrentAction(d->actionItems.first());
}

void
QMenuBar::focusOutEvent(QFocusEvent *)
{
    if(!d->popupState) {
        d->setCurrentAction(0);
        d->setKeyboardMode(false);
    }
}

void QMenuBar::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::StyleChange) {
        d->itemsDirty = 1;
        setMouseTracking(style().styleHint(QStyle::SH_MenuBar_MouseTracking));
        if(parentWidget())
            resize(parentWidget()->width(), heightForWidth(parentWidget()->width()));
    }
}

void QMenuBar::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
}

bool
QMenuBar::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
#if 0
        if(!d->keyboardState) { //all keypresses..
            d->setCurrentAction(0);
            return ;
        }
#endif
        if(ke->key() == Key_Tab || ke->key() == Key_BackTab) {
            keyPressEvent(ke);
            return true;
        }

    }
    return QWidget::event(e);
}

bool
QMenuBar::eventFilter(QObject *object, QEvent *event)
{
    if (object == parent() && object
#ifndef QT_NO_TOOLBAR
         && !qt_cast<QToolBar*>(object)
#endif
         && event->type() == QEvent::Resize) {
        QResizeEvent *e = (QResizeEvent *)event;
        int w = e->size().width();
        setGeometry(0, y(), w, heightForWidth(w));
        return false;
    }

    if(!isVisible() || !object->isWidgetType())
        return false;

    if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        d->altPressed = false;
        return false;
    } else if(d->altPressed && event->type() == QEvent::FocusOut) {
        // some window systems/managers use alt/meta as accelerator keys
        // for switching between windows/desktops/etc.  If the focus
        // widget gets unfocused, then we need to stop waiting for alt release
        d->altPressed = false;
        QWidget *f = ((QWidget *)object)->focusWidget();
        if(f && !f->isTopLevel())
            f->removeEventFilter(this);
        return false;
    } else if(!(event->type() == QEvent::AccelOverride ||
                event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) ||
              !style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this)) {
        return false;
    }

    QWidget *widget = (QWidget *)object;
    QKeyEvent *ke = (QKeyEvent *)event;
#ifndef QT_NO_ACCEL
    // look for Alt press and Alt-anything press
    if(event->type() == QEvent::Accel || event->type() == QEvent::KeyPress) {
        QWidget *f = widget->focusWidget();
        // ### this thinks alt and meta are the same
        if(ke->key() == Key_Alt || ke->key() == Key_Meta) {
            if(d->altPressed) { //eat first alt
                d->altPressed = false;
                if(!widget->isTopLevel())
                    object->removeEventFilter(this);
                ke->accept();
                return true;
            } else if(hasFocus()) {             // Menu has focus, send focus back
                d->setKeyboardMode(false);
                ke->accept();
                return true;
            } else if(ke->stateAfter() == AltButton) {  // Start waiting for Alt release on focus widget
                d->altPressed = true;
                if(f && f != object)
                    f->installEventFilter(this);
            }
        } else if(ke->key() == Key_Control || ke->key() == Key_Shift) {        // Other modifiers kills focus on menubar
            d->setKeyboardMode(false);
        } else {         // Got other key, no need to wait for Alt release
            d->altPressed = false;
        }
        d->setCurrentAction(0);
        return false;
    }
#endif
    if(((QWidget*)object)->focusWidget() == object || (object->parent() == 0 && ((QWidget*)object)->focusWidget() == 0)) {
        if(d->altPressed && event->type() == QEvent::KeyRelease && (ke->key() == Key_Alt || ke->key() == Key_Meta)) {    //alt release
            d->setKeyboardMode(true);
            if(!widget->isTopLevel())
                object->removeEventFilter(this);
            return true;
        } else if(!hasFocus() && (event->type() == QEvent::AccelOverride) &&
                  !(((QKeyEvent *)event)->key() == Key_Alt || ((QKeyEvent *)event)->key() == Key_Meta)) {         // Cancel if next keypress is NOT Alt/Meta,
            if(!widget->isTopLevel())
                object->removeEventFilter(this);
            d->setKeyboardMode(false);
        }
    }
    return false;
}

QAction *QMenuBar::actionAtPos(const QPoint &pt) const
{
    const_cast<QMenuBarPrivate*>(d)->updateActions();
    if(QMenuAction *ret = d->actionAt(pt))
        return ret->action;
    return 0;
}

QRect QMenuBar::actionGeometry(QAction *act) const
{
    const_cast<QMenuBarPrivate*>(d)->updateActions();
    for(QList<QMenuAction*>::ConstIterator it = d->actionItems.begin(); it != d->actionItems.end(); ++it) {
        if((*it)->action == act)
            return (*it)->rect;
    }
    return QRect();
}

QSize QMenuBar::minimumSizeHint() const
{
    return sizeHint();
}

QSize QMenuBar::sizeHint() const
{
    ensurePolished();
    QSize ret(0, 0);
    QList<QMenuAction*> actions = d->calcActionRects(width()-(style().pixelMetric(QStyle::PM_MenuBarFrameWidth, this)*2), 0);
    for(int i = 0; i < actions.count(); ++i) {
        QRect actionRect(actions[i]->rect);
        if(actionRect.right() > ret.width())
            ret.setWidth(actionRect.right());
        if(actionRect.bottom() > ret.height())
            ret.setHeight(actionRect.bottom());
    }
    if(const int fw = style().pixelMetric(QStyle::PM_MenuFrameWidth, this)) {
        ret.setWidth(ret.width()+(fw*2));
        ret.setHeight(ret.height()+(fw*2));
    }
    if(d->leftWidget) {
        QSize sz = d->leftWidget->sizeHint();
        ret.setWidth(ret.width() + sz.width());
        if(sz.height() > ret.height())
            ret.setHeight(sz.height());
    }
    if(d->rightWidget) {
        QSize sz = d->rightWidget->sizeHint();
        ret.setWidth(ret.width() + sz.width());
        if(sz.height() > ret.height())
            ret.setHeight(sz.height());
    }
    ret.setWidth(ret.width()+q->style().pixelMetric(QStyle::PM_MenuBarHMargin, q));
    ret.setHeight(ret.height()+q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q));
    return (style().sizeFromContents(QStyle::CT_MenuBar, this, ret.expandedTo(QApplication::globalStrut())));
}

int QMenuBar::heightForWidth(int max_width) const
{
    QList<QMenuAction*> actions = d->calcActionRects(max_width, 0);
    int height = 0;
    for(int i = 0; i < actions.count(); ++i)
        height = qMax(height, actions[i]->rect.bottom());
    if(d->leftWidget)
        height = qMax(d->leftWidget->sizeHint().height(), height);
    if(d->rightWidget)
        height = qMax(d->rightWidget->sizeHint().height(), height);
    height += (q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q)*2) + q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q);
    return style().sizeFromContents(QStyle::CT_MenuBar, this, QSize(0, height)).height(); //not pretty..
}

void QMenuBar::internalShortcutActivated(int id)
{
#ifndef QT_NO_ACCEL
    QKeySequence key = d->shortcuts->key(id);
    for(int i = 0; i < d->actionItems.count(); i++) {
        QMenuAction *act = d->actionItems.at(i);
        if(QAccel::shortcutKey(act->action->text()) == key) {
            d->setCurrentAction(act, true);
            break;
        }
    }
#endif
}

void QMenuBar::setLeftWidget(QWidget *w)
{
    d->itemsDirty = true;
    if((d->leftWidget = w))
        d->leftWidget->setParent(this);
    d->updateActions();
}

void QMenuBar::setRightWidget(QWidget *w)
{
    d->itemsDirty = true;
    if((d->rightWidget = w))
        d->rightWidget->setParent(this);
    d->updateActions();
}

#ifdef QT_COMPAT
int QMenuBar::frameWidth() const
{
    return style().pixelMetric(QStyle::PM_MenuBarFrameWidth, this);
}

static int get_id()
{
    static int sid = -1;
    return --sid;
}

int QMenuBar::insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                        const QKeySequence *accel, const QMenu *popup, int id, int index)
{
    if(id == -1)
        id = get_id();
    QAction *act = new QAction;
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
    return id;
}


int QMenuBar::insertSeparator(int index)
{
    int id = get_id();
    QAction *act = new QAction;
    act->setSeparator(true);
    act->setId(id);
    if(index == -1)
        addAction(act);
    else
        insertAction(act, actions().value(index+1));
    return id;
}


QAction *QMenuBar::findActionForId(int id) const
{
    QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *a = list.at(i);
        if (a->id() == id)
            return a;
    }
    return 0;
}
#endif
