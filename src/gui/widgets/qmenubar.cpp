#include <qmenubar.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qapplication.h>
#include "qdesktopwidget.h"
#ifndef QT_NO_ACCESSIBILITY
# include "qaccessible.h"
#endif
#include <qpainter.h>
#include <qevent.h>
#include <qmainwindow.h>
#include <qtoolbar.h>

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
#if 0
    if(itemsWidth == q_width && q_start == itemsStart)
        return;
#endif

#ifdef Q_WS_MAC
    if(d->mac_menubar) {//nothing to see here folks, move along..
        itemsDirty = 0;
        return;
    }
#endif
    actionItems = calcActionRects(q_width, q_start);
    itemsWidth = q_width;
    itemsStart = q_start;
    if(itemsDirty) {
        for(int j = 0; j < shortcutIndexMap.size(); ++j)
            q->releaseShortcut(shortcutIndexMap.value(j));
        shortcutIndexMap.resize(0); // faster than clear
        for(int i = 0; i < actionItems.count(); i++)
            shortcutIndexMap.append(
                q->grabShortcut(QKeySequence::mnemonic(actionItems.at(i)->action->text())));
    }
    itemsDirty = 0;

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
    return QStyle::visualRect(ret, q);
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
        const int off = pos.x()+popup_size.width() - dh.right();
        if(off > 0)
            pos.setX(qMax(0, pos.x()-off));
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
            if(q->style().styleHint(QStyle::SH_GUIStyle, q) == Qt::MotifStyle)
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
        QStyleOptionMenuItem opt = getStyleOption(action);
        sz = q->style().sizeFromContents(QStyle::CT_MenuBarItem, &opt, sz, fm, q);

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

QStyleOptionMenuItem QMenuBarPrivate::getStyleOption(const QAction *action) const
{
    QStyleOptionMenuItem opt(0);
    opt.palette = q->palette();
    opt.state = QStyle::Style_Default;
    if (q->isEnabled() && action->isEnabled())
        opt.state |= QStyle::Style_Enabled;
    else
        opt.palette.setCurrentColorGroup(QPalette::Disabled);
    if (currentAction && currentAction->action == action) {
        opt.state |= QStyle::Style_Active;
        if (popupState && !closePopupMode)
            opt.state |= QStyle::Style_Down;
    }
    if (q->hasFocus() || currentAction)
        opt.state |= QStyle::Style_HasFocus;
    opt.menuRect = q->rect();
    opt.menuItemType = QStyleOptionMenuItem::Normal;
    opt.checkState = QStyleOptionMenuItem::NotCheckable;
    opt.text = action->text();
    opt.icon = action->icon();
    return opt;
}

/*!
    \class QMenuBar
    \brief The QMenuBar class provides a horizontal menu bar.

    \ingroup application
    \mainclass

    A menu bar consists of a list of pull-down menu items. You add
    menu items with addMenu(). For example, asuming that \c menubar
    is a pointer to a QMenuBar and \c filemenu is a pointer to a
    QMenu, the following statement inserts the menu into the menu bar:
    \code
      menubar->addMenu("&File", filemenu);
    \endcode

    The ampersand in the menu item's text sets Alt+F as a shortcut for
    this menu. (You can use "\&\&" to get a real ampersand in the menu
    bar.)

    There is no need to lay out a menu bar. It automatically sets its
    own geometry to the top of the parent widget and changes it
    appropriately whenever the parent is resized.

    Example of creating a menu bar with menu items (from \l menu/menu.cpp):
    \quotefile menu/menu.cpp
    \skipto file = new QMenu
    \printline
    \skipto Qt::Key_O
    \printline
    \printline
    \skipto new QMenuBar
    \printline
    \skipto addMenu
    \printline

    In most main window style applications you would use the menuBar()
    provided in QMainWindow, adding \l{QMenu}s to the menu bar and
    adding \l{QAction}s to the popup menus.

    Example (from \l action/application.cpp):
    \quotefile action/application.cpp
    \skipto file = new QMenu
    \printuntil fileNewAction

    Menu items may be removed with removeAction().

    \inlineimage qmenubar-m.png Screenshot in Motif style
    \inlineimage qmenubar-w.png Screenshot in Windows style

    \section1 QMenuBar on Qt/Mac

    QMenuBar on Qt/Mac is a wrapper for using the system-wide menubar.
    If you have multiple menubars in one dialog the outermost menubar
    (normally inside a widget with widget flag \c Qt::WType_TopLevel) will
    be used for the system-wide menubar.

    Qt/Mac also provides a menubar merging feature to make QMenuBar
    conform more closely to accepted Mac OS X menubar layout. The
    merging functionality is based on string matching the title of a
    QMenu entry. These strings are translated (using QObject::tr()) in
    the "QMenuBar" context. If an entry is moved its slots will still
    fire as if it was in the original place. The table below outlines
    the strings looked for and where the entry is placed if matched:

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
    QMenuBar and QMenu use.

    \sa QMenu QShortcut QAction \link http://developer.apple.com/techpubs/macosx/Carbon/HumanInterfaceToolbox/Aqua/aqua.html Aqua Style Guidelines \endlink \link guibooks.html#fowler GUI Design Handbook: Menu Bar \endlink
*/


void QMenuBarPrivate::init()
{
    QWidget *parent = q->parentWidget();
#ifdef Q_WS_MAC
    macCreateMenuBar(parent);
#endif
    q->setBackgroundRole(QPalette::Button);
    if(parent) {
        q->topLevelWidget()->installEventFilter(q); //grab shortcuts (and maybe resizes)
        if(!parent->isTopLevel())
            parent->installEventFilter(q); //handle resizes
    }
    q->setMouseTracking(q->style().styleHint(QStyle::SH_MenuBar_MouseTracking));
#ifdef QT_COMPAT
    QObject::connect(q, SIGNAL(activated(QAction*)), q, SLOT(compatActivated(QAction*)));
    QObject::connect(q, SIGNAL(highlighted(QAction*)), q, SLOT(compatHighlighted(QAction*)));
#endif
}

/*!
    Constructs a menu bar with parent \a parent.
*/
QMenuBar::QMenuBar(QWidget *parent) : QWidget(*new QMenuBarPrivate, parent, 0)
{
    d->init();
}

#ifdef QT_COMPAT
QMenuBar::QMenuBar(QWidget *parent, const char *name) : QWidget(*new QMenuBarPrivate, parent, 0)
{
    d->init();
    setObjectName(name);
}
#endif

/*!
    Destroys the menu bar.
*/
QMenuBar::~QMenuBar()
{
#ifdef Q_WS_MAC
    d->macDestroyMenuBar();
#endif
}

/*!
    \overload

    This convenience function creates a new action with \a text.
    The function adds the newly created action to the menu's
    list of actions, and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenuBar::addAction(const QString &text)
{
    QAction *ret = new QAction(text);
    addAction(ret);
    return ret;
}

/*!
    \overload

    This convenience function creates a new action with the text \a
    text and a keyboard shortcut of \a shortcut. The action's
    triggered() signal is connected to the \a receiver's \a member
    slot. The function adds the newly created action to the menu's
    list of actions and returns it.

    \sa QWidget::addAction()
*/
QAction *QMenuBar::addAction(const QString &text, const QObject *receiver, const char* member)
{
    QAction *ret = new QAction(text, this);
    QObject::connect(ret, SIGNAL(triggered()), receiver, member);
    addAction(ret);
    return ret;
}

/*!
  Appends an action with text \a text and menu \a menu to the list of
  actions.

  This convenience function will create a new QAction, setting its
  text and menu, append it to the list of actions, and finally
  return the newly created action.

  \sa QWidget::addAction()
*/
QAction *QMenuBar::addMenu(const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, this);
    ret->setMenu(menu);
    addAction(ret);
    return ret;
}

/*!
  Appends a separator to the menu.
*/
QAction *QMenuBar::addSeparator()
{
    QAction *ret = new QAction;
    ret->setSeparator(true);
    addAction(ret);
    return ret;
}

/*!
  Inserts an action with text \a text and menu \a menu into the list
  of actions before \a before.

  This convenience function will create a new QAction, insert it to
  the list of actions, and finally return the newly created action.

  \sa QWidget::insertAction() addMenu()
*/
QAction *QMenuBar::insertMenu(QAction *before, const QString &text, QMenu *menu)
{
    QAction *ret = new QAction(text, this);
    ret->setMenu(menu);
    insertAction(before, ret);
    return ret;
}

/*!
  Returns the QAction that is currently highlighted. A null pointer
  will be returned if no action is currently selected.
*/
QAction *QMenuBar::activeAction() const
{
    return d->currentAction->action;
}

/*!
    Removes all the actions from the menu bar.

    \sa removeAction()
*/
void QMenuBar::clear()
{
    QList<QAction*> acts = actions();
    for(int i = 0; i < acts.size(); i++)
        removeAction(acts[i]);
}

/*!
    \property QMenuBar::defaultUp
    \brief the popup orientation

    The default popup orientation. By default, menus pop "down" the
    screen. By setting the property to true, the menu will pop "up".
    You might call this for menus that are \e below the document to
    which they refer.

    If the menu would not fit on the screen, the other direction is
    used automatically.
*/
void QMenuBar::setDefaultUp(bool b)
{
    d->defaultPopDown = !b;
}

bool QMenuBar::isDefaultUp() const
{
    return !d->defaultPopDown;
}

/*!
  \reimp
*/
void QMenuBar::resizeEvent(QResizeEvent *)
{
    d->itemsDirty = 1;
}

/*!
  \reimp
*/
void QMenuBar::paintEvent(QPaintEvent *e)
{
    d->updateActions();

    QPainter p(this);
    QRegion emptyArea(rect());

    //draw the items
    for (int i = 0; i < d->actionItems.count(); ++i) {
        QMenuAction *action = d->actionItems.at(i);
        QRect adjustedActionRect = d->actionRect(action);
        if(!e->rect().intersects(adjustedActionRect))
           continue;

        emptyArea -= adjustedActionRect;
        QStyleOptionMenuItem opt = d->getStyleOption(action->action);
        opt.rect = adjustedActionRect;
        p.setClipRect(adjustedActionRect);
        style().drawControl(QStyle::CE_MenuBarItem, &opt, &p, this);
    }
     //draw border
    if(int fw = style().pixelMetric(QStyle::PM_MenuBarFrameWidth, this)) {
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
        frame.lineWidth = style().pixelMetric(QStyle::PM_MenuBarFrameWidth);
        frame.midLineWidth = 0;
        style().drawPrimitive(QStyle::PE_MenuBarFrame, &frame, &p, this);
    }
    p.setClipRegion(emptyArea);
    QStyleOptionMenuItem menuOpt(0);
    menuOpt.palette = palette();
    menuOpt.state = QStyle::Style_Default;
    menuOpt.menuItemType = QStyleOptionMenuItem::EmptyArea;
    menuOpt.checkState = QStyleOptionMenuItem::NotCheckable;
    menuOpt.rect = rect();
    menuOpt.menuRect = rect();
    style().drawControl(QStyle::CE_MenuBarEmptyArea, &menuOpt, &p, this);
}

/*!
  \reimp
*/
void QMenuBar::mousePressEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton)
        return;
    QMenuAction *action = d->actionAt(e->pos());
    if (!action) {
        d->setCurrentAction(0);
        return;
    }

    d->mouseDown = true;

    if(d->currentAction == action && d->popupState) {
        if((d->closePopupMode = (style().styleHint(QStyle::SH_GUIStyle) == Qt::WindowsStyle)))
            q->update(d->actionRect(action));
    } else {
        d->setCurrentAction(action, true);
    }
}

/*!
  \reimp
*/
void QMenuBar::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton || !d->mouseDown)
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

/*!
  \reimp
*/
void QMenuBar::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if(QApplication::reverseLayout()) {  // in reverse mode open/close key for submenues are reversed
        if(key == Qt::Key_Left)
            key = Qt::Key_Right;
        else if(key == Qt::Key_Right)
            key = Qt::Key_Left;
    }
    if(key == Qt::Key_Tab) //means right
        key = Qt::Key_Right;

    bool key_consumed = false;
    switch(key) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Enter:
    case Qt::Key_Space:
    case Qt::Key_Return: {
        if(!style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this) || !d->currentAction)
           break;
        if(d->currentAction->action->menu()) {
            d->popupAction(d->currentAction, true);
        } else if(key == Qt::Key_Enter || key == Qt::Key_Return || key == Qt::Key_Space) {
            d->activateAction(d->currentAction->action, QAction::Trigger);
            d->setCurrentAction(d->currentAction, false);
        }
        key_consumed = true;
        break; }

    case Qt::Key_Right:
    case Qt::Key_Left: {
        if(d->currentAction) {
            QMenuAction *nextAction = 0;
            for(int i=0; i<(int)d->actionItems.count(); i++) {
                if(d->actionItems.at(i) == d->currentAction) {
                    if(key == Qt::Key_Left) {
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
                if(key == Qt::Key_Left)
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

    case Qt::Key_Escape:
        d->setCurrentAction(0);
        d->setKeyboardMode(false);
        key_consumed = true;
        break;

    default:
        key_consumed = false;
    }

    if(!key_consumed &&
       (!e->state() || (e->state()&(Qt::MetaButton|Qt::AltButton))) && e->text().length()==1 && !d->popupState) {
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
            key_consumed = true;
            d->setCurrentAction(next_action, true, true);
        }
    }
    if(key_consumed)
        e->accept();
}

/*!
  \reimp
*/
void QMenuBar::mouseMoveEvent(QMouseEvent *e)
{
    d->mouseDown = e->state() & Qt::LeftButton;
    QMenuAction *action = d->actionAt(e->pos());
    bool popupState = d->popupState || d->mouseDown;
    if(action || !popupState)
        d->setCurrentAction(action, popupState);
}

/*!
  \reimp
*/
void QMenuBar::leaveEvent(QEvent *)
{
    if(!hasFocus() && !d->popupState)
        d->setCurrentAction(0);
}

/*!
  \reimp
*/
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
    Q_UNUSED(e);
#endif
    if(isVisible())
        update();
}

/*!
  \reimp
*/
void QMenuBar::focusInEvent(QFocusEvent *)
{
    if(!d->currentAction && !d->actionItems.isEmpty())
        d->setCurrentAction(d->actionItems.first());
}

/*!
  \reimp
*/
void QMenuBar::focusOutEvent(QFocusEvent *)
{
    if(!d->popupState) {
        d->setCurrentAction(0);
        d->setKeyboardMode(false);
    }
}

/*!
  \reimp
*/
void QMenuBar::changeEvent(QEvent *e)
{
    if(e->type() == QEvent::StyleChange) {
        d->itemsDirty = 1;
        setMouseTracking(style().styleHint(QStyle::SH_MenuBar_MouseTracking));
        if(parentWidget())
            resize(parentWidget()->width(), heightForWidth(parentWidget()->width()));
    }
}

/*!
  \reimp
*/
void QMenuBar::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
}

/*!
  \reimp
*/
bool QMenuBar::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
#if 0
        if(!d->keyboardState) { //all keypresses..
            d->setCurrentAction(0);
            return ;
        }
#endif
        if(ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_BackTab) {
            keyPressEvent(ke);
            return true;
        }

    } else if(e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        int shortcutId = se->shortcutId();
        for(int j = 0; j < d->shortcutIndexMap.size(); ++j) {
            if (shortcutId == d->shortcutIndexMap.value(j))
                internalShortcutActivated(j);
        }
    }
    return QWidget::event(e);
}

/*!
  \reimp
*/
bool
QMenuBar::eventFilter(QObject *object, QEvent *event)
{
    if (object == parent() && object
#ifndef QT_NO_TOOLBAR
        && !qt_cast<QToolBar*>(object)
        && !object->inherits("Q3ToolBar")
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
        // some window systems/managers use alt/meta as shortcut keys
        // for switching between windows/desktops/etc.  If the focus
        // widget gets unfocused, then we need to stop waiting for alt release
        d->altPressed = false;
        QWidget *f = ((QWidget *)object)->focusWidget();
        if(f && !f->isTopLevel())
            f->removeEventFilter(this);
        return false;
    } else if(!(event->type() == QEvent::ShortcutOverride ||
                event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) ||
              !style().styleHint(QStyle::SH_MenuBar_AltKeyNavigation, this)) {
        return false;
    }

    QWidget *widget = (QWidget *)object;
    QKeyEvent *ke = (QKeyEvent *)event;
#ifndef QT_NO_SHORTCUT
    // look for Alt press and Alt-anything press
    if(event->type() == QEvent::Shortcut || event->type() == QEvent::KeyPress) {
        QWidget *f = widget->focusWidget();
        // ### this thinks alt and meta are the same
        if(ke->key() == Qt::Key_Alt || ke->key() == Qt::Key_Meta) {
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
            } else if(ke->stateAfter() == Qt::AltButton) {  // Start waiting for Alt release on focus widget
                d->altPressed = true;
                if(f && f != object)
                    f->installEventFilter(this);
            }
        } else if(ke->key() == Qt::Key_Control || ke->key() == Qt::Key_Shift) {        // Other modifiers kills focus on menubar
            d->setKeyboardMode(false);
        } else {         // Got other key, no need to wait for Alt release
            d->altPressed = false;
        }
        d->setCurrentAction(0);
        return false;
    }
#endif
    if(((QWidget*)object)->focusWidget() == object || (object->parent() == 0 && ((QWidget*)object)->focusWidget() == 0)) {
        if(d->altPressed && event->type() == QEvent::KeyRelease && (ke->key() == Qt::Key_Alt || ke->key() == Qt::Key_Meta)) {    //alt release
            d->setKeyboardMode(true);
            if(!widget->isTopLevel())
                object->removeEventFilter(this);
            return true;
        } else if(!hasFocus() && (event->type() == QEvent::ShortcutOverride) &&
                  !(((QKeyEvent *)event)->key() == Qt::Key_Alt || ((QKeyEvent *)event)->key() == Qt::Key_Meta)) {         // Cancel if next keypress is NOT Alt/Meta,
            if(!widget->isTopLevel())
                object->removeEventFilter(this);
            d->setKeyboardMode(false);
        }
    }
    return false;
}

/*!
  \internal

  Return the item at \a pt, or 0 if there is no item there or if it is
  a separator item.
*/
QAction *QMenuBar::actionAtPos(const QPoint &pt) const
{
    const_cast<QMenuBarPrivate*>(d)->updateActions();
    if(QMenuAction *ret = d->actionAt(pt))
        return ret->action;
    return 0;
}

/*!
  \internal

  Returns the geometry of action \a act.
*/
QRect QMenuBar::actionGeometry(QAction *act) const
{
    const_cast<QMenuBarPrivate*>(d)->updateActions();
    for(QList<QMenuAction*>::ConstIterator it = d->actionItems.begin(); it != d->actionItems.end(); ++it) {
        if((*it)->action == act)
            return d->actionRect((*it));
    }
    return QRect();
}

/*!
  \reimp
*/
QSize QMenuBar::minimumSizeHint() const
{
    return sizeHint();
}

/*!
  \reimp
*/
QSize QMenuBar::sizeHint() const
{
#ifdef Q_WS_MAC
    const bool as_gui_menubar = !d->mac_menubar;
#else
    const bool as_gui_menubar = true;
#endif

    ensurePolished();
    QSize ret(0, 0);
    if(as_gui_menubar) {
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
        ret.setWidth(ret.width()+q->style().pixelMetric(QStyle::PM_MenuBarHMargin, q));
        ret.setHeight(ret.height()+q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q));
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
    if(as_gui_menubar) {
        QStyleOptionMenuItem opt(0);
        opt.rect = rect();
        opt.menuRect = rect();
        opt.state = QStyle::Style_Default;
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.checkState = QStyleOptionMenuItem::NotCheckable;
        opt.palette = palette();
        return (style().sizeFromContents(QStyle::CT_MenuBar, &opt,
                                         ret.expandedTo(QApplication::globalStrut()), fontMetrics(),
                                         this));
    }
    return ret;
}

/*!
  \reimp
*/
int QMenuBar::heightForWidth(int max_width) const
{
#ifdef Q_WS_MAC
    const bool as_gui_menubar = !d->mac_menubar;
#else
    const bool as_gui_menubar = true;
#endif

    int height = 0;
    if(as_gui_menubar) {
        QList<QMenuAction*> actions = d->calcActionRects(max_width, 0);
        for(int i = 0; i < actions.count(); ++i)
            height = qMax(height, actions[i]->rect.bottom());
        height += (q->style().pixelMetric(QStyle::PM_MenuBarFrameWidth, q)*2) + q->style().pixelMetric(QStyle::PM_MenuBarVMargin, q);
    }
    if(d->leftWidget)
        height = qMax(d->leftWidget->sizeHint().height(), height);
    if(d->rightWidget)
        height = qMax(d->rightWidget->sizeHint().height(), height);
    if(as_gui_menubar) {
        QStyleOptionMenuItem opt(0);
        opt.rect = rect();
        opt.menuRect = rect();
        opt.state = QStyle::Style_Default;
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.checkState = QStyleOptionMenuItem::NotCheckable;
        opt.palette = palette();
        return style().sizeFromContents(QStyle::CT_MenuBar, &opt, QSize(0, height), fontMetrics(),
                                        this).height(); //not pretty..
    }
    return height;
}

/*!
  \internal
*/
void QMenuBar::internalShortcutActivated(int id)
{
    QMenuAction *act = d->actionItems.at(id);
    d->setCurrentAction(act, true);
}

/*!
  \internal

  This sets widget \a w to be shown directly on the left of the first
  menu item.

  \sa setRightWidget
*/
void QMenuBar::setLeftWidget(QWidget *w)
{
    d->itemsDirty = true;
    if((d->leftWidget = w))
        d->leftWidget->setParent(this);
    d->updateActions();
}

/*!
  \internal

  This sets widget \a w to be shown directly on the right of the last
  menu item.

  \sa setLeftWidget
*/
void QMenuBar::setRightWidget(QWidget *w)
{
    d->itemsDirty = true;
    if((d->rightWidget = w))
        d->rightWidget->setParent(this);
    d->updateActions();
}

/*!
    \fn void QMenuBar::activated(QAction *action)

    This signal is emitted when a menu action is selected; \a action
    is the action that caused the event to be sent.

    Normally, you connect each menu action to a single slot using
    QAction::triggered(), but sometimes you will want to connect
    several items to a single slot (most often if the user selects
    from an array). This signal is useful in such cases.

    \sa highlighted(), QAction::triggered()
*/

/*!
    \fn void QMenuBar::highlighted(QAction *action)

    This signal is emitted when a menu action is highlighted; \a action
    is the action that caused the event to be sent.

    Often this is used to update status information.

    \sa activated(), QAction::hovered()
*/


#ifdef QT_COMPAT
#include "qmenudata.h"
int QMenuBar::frameWidth() const
{
    return style().pixelMetric(QStyle::PM_MenuBarFrameWidth, this);
}

int QMenuBar::insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                        const QKeySequence *shortcut, const QMenu *popup, int id, int index)
{
    QAction *act = new QAction;
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
        insertAction(act, actions().value(index+1));
    return findIdForAction(act);
}

int QMenuBar::insertSeparator(int index)
{
    QAction *act = new QAction;
    act->setSeparator(true);
    if(index == -1)
        addAction(act);
    else
        insertAction(act, actions().value(index+1));
    return findIdForAction(act);
}

bool QMenuBar::setItemParameter(int id, int param)
{
    if(QAction *act = findActionForId(id)) {
        static_cast<QMenuItem*>(act)->setSignalValue(param);
        return true;
    }
    return false;
}

int QMenuBar::itemParameter(int id) const
{
    if(QAction *act = findActionForId(id))
        return static_cast<QMenuItem*>(act)->signalValue();
    return id;
}

QAction *QMenuBar::findActionForId(int id) const
{
    QList<QAction *> list = actions();
    for (int i = 0; i < list.size(); ++i) {
        QAction *act = list.at(i);
        if (findIdForAction(act) == id)
            return act;
    }
    return 0;
}

void QMenuBar::compatActivated(QAction *act)
{
    emit activated(findIdForAction(act));
}

void QMenuBar::compatHighlighted(QAction *act)
{
    emit highlighted(findIdForAction(act));
}

int QMenuBar::findIdForAction(QAction *act) const
{
    return static_cast<QMenuItem*>(act)->id();
}
#endif
