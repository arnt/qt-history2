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

#include "qtoolbar.h"

#ifndef QT_NO_TOOLBAR

#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qrubberband.h>
#include <qsignalmapper.h>
#include <qstylepainter.h>
#include <qtoolbutton.h>
#include <qwidgetaction.h>
#include <qtimer.h>
#include <private/qwidgetaction_p.h>
#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
#endif

#include <private/qmainwindowlayout_p.h>

#include "qtoolbar_p.h"
#include "qtoolbarseparator_p.h"
#include "qtoolbarlayout_p.h"

#include  "qdebug.h"

/******************************************************************************
** QToolBarPrivate
*/

void QToolBarPrivate::init()
{
    Q_Q(QToolBar);

    waitForPopupTimer = new QTimer(q);
    waitForPopupTimer->setSingleShot(false);
    waitForPopupTimer->setInterval(500);
    QObject::connect(waitForPopupTimer, SIGNAL(timeout()), q, SLOT(_q_waitForPopup()));

    floatable = true;
    movable = true;
    q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    q->setBackgroundRole(QPalette::Button);
    q->setMouseTracking(true);

    QStyle *style = q->style();
    int e = style->pixelMetric(QStyle::PM_ToolBarIconSize, 0, q);
    iconSize = QSize(e, e);

    layout = new QToolBarLayout(q);

#ifdef Q_WS_MAC
    if (q->parentWidget() && q->parentWidget()->isWindow()) {
        // Make sure that the window has the "toolbar" button.
        reinterpret_cast<QToolBar *>(q->parentWidget())->d_func()->createWinId(); // Please let me create your winId...
        extern WindowPtr qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
        ChangeWindowAttributes(qt_mac_window_for(q->parentWidget()), kWindowToolbarButtonAttribute,
                               kWindowNoAttributes);
    }
#endif

    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(_q_toggleView(bool)));
}

void QToolBarPrivate::_q_toggleView(bool b)
{
    Q_Q(QToolBar);
    if (b == q->isHidden()) {
        if (b)
            q->show();
        else
            q->close();
    }
}

void QToolBarPrivate::_q_updateIconSize(const QSize &sz)
{
    Q_Q(QToolBar);
    if (!explicitIconSize) {
        // iconSize not explicitly set
        q->setIconSize(sz);
        explicitIconSize = false;
    }
}

void QToolBarPrivate::_q_updateToolButtonStyle(Qt::ToolButtonStyle style)
{
    Q_Q(QToolBar);
    if (!explicitToolButtonStyle) {
        q->setToolButtonStyle(style);
        explicitToolButtonStyle = false;
    }
}

void QToolBarPrivate::setWindowState(bool floating, bool unplug, const QRect &rect)
{
    Q_Q(QToolBar);
    bool visible = !q->isHidden();

    q->hide();

    Qt::WindowFlags flags = floating ? Qt::Tool : Qt::Widget;

    flags |= Qt::FramelessWindowHint;

    if (unplug)
        flags |= Qt::X11BypassWindowManagerHint;

    q->setWindowFlags(flags);

    if (!rect.isNull())
        q->setGeometry(rect);

    if (visible)
        q->show();
}

void QToolBarPrivate::initDrag(const QPoint &pos)
{
    Q_Q(QToolBar);

    if (state != 0)
        return;

    QMainWindow *win = qobject_cast<QMainWindow*>(q->parentWidget());
    Q_ASSERT(win != 0);
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(win->layout());
    Q_ASSERT(layout != 0);
    if (layout->pluggingWidget != 0) // the main window is animating a docking operation
        return;

    state = new DragState;
    state->pressPos = pos;
    state->dragging = false;
    state->widgetItem = 0;
}

void QToolBarPrivate::startDrag()
{
    Q_Q(QToolBar);

    Q_ASSERT(state != 0);
    if (state->dragging)
        return;

    QMainWindow *win = qobject_cast<QMainWindow*>(q->parentWidget());
    Q_ASSERT(win != 0);
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(win->layout());
    Q_ASSERT(layout != 0);

    int w = q->width();

    state->widgetItem = layout->unplug(q);
    Q_ASSERT(state->widgetItem != 0);
    state->dragging = true;

    if (q->layoutDirection() == Qt::RightToLeft)
        state->pressPos = QPoint(w - state->pressPos.x(), state->pressPos.y());
}

void QToolBarPrivate::endDrag()
{
    Q_Q(QToolBar);
    Q_ASSERT(state != 0);

    q->releaseMouse();

    if (state->dragging) {
        QMainWindowLayout *layout =
            qobject_cast<QMainWindowLayout *>(q->parentWidget()->layout());
        Q_ASSERT(layout != 0);

        if (!layout->plug(state->widgetItem)) {
            if (q->isFloatable()) {
                layout->restore();
#ifdef Q_WS_X11
                setWindowState(true); // gets rid of the X11BypassWindowManager window flag
                                      // and activates the resizer
#endif
                q->activateWindow();
            } else {
                layout->revert(state->widgetItem);
            }
        }
    }

    delete state;
    state = 0;
}

void QToolBarPrivate::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (!layout->movable())
        return;

    initDrag(event->pos());
}

void QToolBarPrivate::mouseReleaseEvent(QMouseEvent*)
{
    endDrag();
}

void QToolBarPrivate::mouseMoveEvent(QMouseEvent *event)
{
    Q_Q(QToolBar);

    if (!state)
        return;

    QMainWindow *win = qobject_cast<QMainWindow*>(q->parentWidget());
    if (win == 0)
        return;

    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout*>(win->layout());
    Q_ASSERT(layout != 0);

    if (!state->dragging
        && layout->pluggingWidget == 0
        && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {
            startDrag();
#ifdef Q_OS_WIN
            grabMouseWhileInWindow();
#else
            q->grabMouse();
#endif
    }

    if (state->dragging) {
        QPoint pos = event->globalPos();
        // if we are right-to-left, we move so as to keep the right edge the same distance
        // from the mouse
        if (q->layoutDirection() == Qt::LeftToRight)
            pos -= state->pressPos;
        else
            pos += QPoint(state->pressPos.x() - q->width(), -state->pressPos.y());
        q->move(pos);
        layout->hover(state->widgetItem, event->globalPos());
    }
}

void QToolBarPrivate::unplug(const QRect &_r)
{
    Q_Q(QToolBar);

    QRect r = _r;
    r.moveTopLeft(q->mapToGlobal(QPoint(0, 0)));
    setWindowState(true, true, r);
}

void QToolBarPrivate::plug(const QRect &r)
{
    setWindowState(false, false, r);
}

/******************************************************************************
** QToolBar
*/

/*!
    \class QToolBar

    \brief The QToolBar class provides a movable panel that contains a
    set of controls.

    \ingroup application
    \mainclass

    Toolbar buttons are added by adding \e actions, using addAction()
    or insertAction(). Groups of buttons can be separated using
    addSeparator() or insertSeparator(). If a toolbar button is not
    appropriate, a widget can be inserted instead using addWidget() or
    insertWidget(); examples of suitable widgets are QSpinBox,
    QDoubleSpinBox, and QComboBox. When a toolbar button is pressed it
    emits the actionTriggered() signal.

    A toolbar can be fixed in place in a particular area (e.g. at the
    top of the window), or it can be movable (isMovable()) between
    toolbar areas; see allowedAreas() and isAreaAllowed().

    When a toolbar is resized in such a way that it is too small to
    show all the items it contains, an extension button will appear as
    the last item in the toolbar. Pressing the extension button will
    pop up a menu containing the items that does not currently fit in
    the toolbar.

    When a QToolBar is not a child of a QMainWindow, it looses the ability
    to populate the extension pop up with widgets added to the toolbar using
    addWidget(). Please use widget actions created by inheriting QWidgetAction
    and implementing QWidgetAction::createWidget() instead. This is a known
    issue which will be fixed in a future release.

    \sa QToolButton, QMenu, QAction, {Application Example}
*/

/*!
    \fn bool QToolBar::isAreaAllowed(Qt::ToolBarArea area) const

    Returns true if this toolbar is dockable in the given \a area;
    otherwise returns false.
*/

/*!
    \fn void QToolBar::actionTriggered(QAction *action)

    This signal is emitted when a toolbar button is pressed. The
    parameter holds the toolbar button's associated \a action.
*/

/*!
    \fn void QToolBar::allowedAreasChanged(Qt::ToolBarAreas allowedAreas)

    This signal is emitted when the collection of allowed areas for the
    toolbar is changed. The new areas in which the toolbar can be positioned
    are specified by \a allowedAreas.

    \sa allowedAreas
*/

/*!
    \fn void QToolBar::iconSizeChanged(const QSize &iconSize)

    This signal is emitted when the icon size is changed.  The \a
    iconSize parameter holds the toolbar's new icon size.

    \sa iconSize QMainWindow::iconSize
*/

/*!
    \fn void QToolBar::movableChanged(bool movable)

    This signal is emitted when the toolbar becomes movable or fixed.
    If the toolbar can be moved, \a movable is true; otherwise it is
    false.

    \sa movable
*/

/*!
    \fn void QToolBar::orientationChanged(Qt::Orientation orientation)

    This signal is emitted when the orientation of the toolbar changes.
    The new orientation is specified by the \a orientation given.

    \sa orientation
*/

/*!
    \fn void QToolBar::toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle)

    This signal is emitted when the tool button style is changed. The
    \a toolButtonStyle parameter holds the toolbar's new tool button
    style.

    \sa toolButtonStyle QMainWindow::toolButtonStyle
*/

/*!
    Constructs a QToolBar with the given \a parent.
*/
QToolBar::QToolBar(QWidget *parent)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
}

/*!
    Constructs a QToolBar with the given \a parent.

    The given window \a title identifies the toolbar and is shown in
    the context menu provided by QMainWindow.

    \sa setWindowTitle()
*/
QToolBar::QToolBar(const QString &title, QWidget *parent)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
    setWindowTitle(title);
}

#ifdef QT3_SUPPORT
/*! \obsolete
    Constructs a QToolBar with the given \a parent and \a name.
*/
QToolBar::QToolBar(QWidget *parent, const char *name)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
    setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the toolbar.
*/
QToolBar::~QToolBar()
{
    // Remove the toolbar button if there is nothing left.
    QMainWindow *mainwindow = qobject_cast<QMainWindow *>(parentWidget());
    if (mainwindow) {
#ifdef Q_WS_MAC
        QMainWindowLayout *mainwin_layout = qobject_cast<QMainWindowLayout *>(mainwindow->layout());
        if (mainwin_layout && mainwin_layout->layoutState.toolBarAreaLayout.isEmpty()
                && mainwindow->testAttribute(Qt::WA_WState_Created))
            ChangeWindowAttributes(qt_mac_window_for(mainwindow), kWindowNoAttributes,
                                   kWindowToolbarButtonAttribute);
#endif
    }
}

/*! \property QToolBar::movable
    \brief whether the user can move the toolbar within the toolbar area,
    or between toolbar areas

    By default, this property is true.

    This property only makes sense if the toolbar is in a
    QMainWindow.

    \sa allowedAreas
*/

void QToolBar::setMovable(bool movable)
{
    Q_D(QToolBar);
    if (!movable == !d->movable)
        return;
    d->movable = movable;
    d->layout->invalidate();
    emit movableChanged(d->movable);
}

bool QToolBar::isMovable() const
{
    Q_D(const QToolBar);
    return d->movable;
}

bool QToolBar::isFloatable() const
{
    Q_D(const QToolBar);
    return d->floatable;
}

void QToolBar::setFloatable(bool floatable)
{
    Q_D(QToolBar);
    d->floatable = floatable;
}

bool QToolBar::isFloating() const
{
    return isWindow();
}

/*!
    \property QToolBar::allowedAreas
    \brief areas where the toolbar may be placed

    The default is Qt::AllToolBarAreas.

    This property only makes sense if the toolbar is in a
    QMainWindow.

    \sa movable
*/

void QToolBar::setAllowedAreas(Qt::ToolBarAreas areas)
{
    Q_D(QToolBar);
    areas &= Qt::ToolBarArea_Mask;
    if (areas == d->allowedAreas)
        return;
    d->allowedAreas = areas;
    emit allowedAreasChanged(d->allowedAreas);
}

Qt::ToolBarAreas QToolBar::allowedAreas() const
{
    Q_D(const QToolBar);
#ifdef Q_WS_MAC
    if (QMainWindow *window = qobject_cast<QMainWindow *>(parentWidget())) {
        if (window->unifiedTitleAndToolBarOnMac()) // Don't allow drags to the top (for now).
            return (d->allowedAreas & ~Qt::TopToolBarArea);
    }
#endif
    return d->allowedAreas;
}

/*! \property QToolBar::orientation
    \brief orientation of the toolbar

    The default is Qt::Horizontal.

    This function should not be used when the toolbar is managed
    by QMainWindow. You can use QMainWindow::addToolBar() or
    QMainWindow::insertToolBar() if you wish to move a toolbar (that
    is already added to a main window) to another Qt::ToolBarArea.
*/

void QToolBar::setOrientation(Qt::Orientation orientation)
{
    Q_D(QToolBar);
    if (orientation == d->orientation)
        return;

    d->orientation = orientation;

    if (orientation == Qt::Vertical)
 	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    else
 	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    d->layout->invalidate();
    d->layout->activate();

    emit orientationChanged(d->orientation);
}

Qt::Orientation QToolBar::orientation() const
{ Q_D(const QToolBar); return d->orientation; }

/*!
    \property QToolBar::iconSize
    \brief size of icons in the toolbar.

    The default size is determined by the application's style and is
    derived from the QStyle::PM_ToolBarIconSize pixel metric. It is
    the maximum size an icon can have. Icons of smaller size will not
    be scaled up.
*/

QSize QToolBar::iconSize() const
{ Q_D(const QToolBar); return d->iconSize; }

void QToolBar::setIconSize(const QSize &iconSize)
{
    Q_D(QToolBar);
    QSize sz = iconSize;
    if (!sz.isValid()) {
        QMainWindow *mw = qobject_cast<QMainWindow *>(parentWidget());
        if (mw && mw->layout()) {
            QLayout *layout = mw->layout();
            int i = 0;
            QLayoutItem *item = 0;
            do {
                item = layout->itemAt(i++);
                if (item && (item->widget() == this))
                    sz = mw->iconSize();
            } while (!sz.isValid() && item != 0);
        }
    }
    if (!sz.isValid()) {
        const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize, 0, this);
        sz = QSize(metric, metric);
    }
    if (d->iconSize != sz) {
        d->iconSize = sz;
        setMinimumSize(0, 0);
        emit iconSizeChanged(d->iconSize);
    }
    d->explicitIconSize = iconSize.isValid();

    d->layout->invalidate();
}

/*!
    \property QToolBar::toolButtonStyle
    \brief the style of toolbar buttons

    The default is Qt::ToolButtonIconOnly.
*/

Qt::ToolButtonStyle QToolBar::toolButtonStyle() const
{ Q_D(const QToolBar); return d->toolButtonStyle; }

void QToolBar::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
    Q_D(QToolBar);
    d->explicitToolButtonStyle = true;
    if (d->toolButtonStyle == toolButtonStyle)
        return;
    d->toolButtonStyle = toolButtonStyle;
    setMinimumSize(0, 0);
    emit toolButtonStyleChanged(d->toolButtonStyle);
}

/*!
    Removes all actions from the toolbar.

    \sa removeAction()
*/
void QToolBar::clear()
{
    QList<QAction *> actions = this->actions();
    for(int i = 0; i < actions.size(); i++)
        removeAction(actions.at(i));
}

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar.
*/
QAction *QToolBar::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a icon and \a text. This
    action is added to the end of the toolbar.
*/
QAction *QToolBar::addAction(const QIcon &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar. The action's \link QAction::triggered()
    triggered()\endlink signal is connected to \a member in \a
    receiver.
*/
QAction *QToolBar::addAction(const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is added to the end of the toolbar. The action's \link
    QAction::triggered() triggered()\endlink signal is connected to \a
    member in \a receiver.
*/
QAction *QToolBar::addAction(const QIcon &icon, const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
     Adds a separator to the end of the toolbar.

     \sa insertSeparator()
*/
QAction *QToolBar::addSeparator()
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

/*!
    Inserts a separator into the toolbar in front of the toolbar
    item associated with the \a before action.

    \sa addSeparator()
*/
QAction *QToolBar::insertSeparator(QAction *before)
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
    return action;
}

/*!
    Adds the given \a widget to the toolbar as the toolbar's last
    item.

    Note: You should use QAction::setVisible() to change the
    visibility of the widget. Using QWidget::setVisible(),
    QWidget::show() and QWidget::hide() does not work.

    \sa insertWidget()
*/
QAction *QToolBar::addWidget(QWidget *widget)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(widget);
    action->d_func()->autoCreated = true;
    addAction(action);
    return action;
}

/*!
    Inserts the given \a widget in front of the toolbar item
    associated with the \a before action.

    Note: You should use QAction::setVisible() to change the
    visibility of the widget. Using QWidget::setVisible(),
    QWidget::show() and QWidget::hide() does not work.

    \sa addWidget()
*/
QAction *QToolBar::insertWidget(QAction *before, QWidget *widget)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(widget);
    action->d_func()->autoCreated = true;
    insertAction(before, action);
    return action;
}

/*!
    \internal

    Returns the geometry of the toolbar item associated with the given
    \a action, or an invalid QRect if no matching item is found.
*/
QRect QToolBar::actionGeometry(QAction *action) const
{
    Q_D(const QToolBar);

    int index = d->layout->indexOf(action);
    if (index == -1)
        return QRect();
    return d->layout->itemAt(index)->widget()->geometry();
}

/*!
    Returns the action at point \a p. This function returns zero if no
    action was found.

    \sa QWidget::childAt()
*/
QAction *QToolBar::actionAt(const QPoint &p) const
{
    Q_D(const QToolBar);
    QWidget *widget = childAt(p);
    int index = d->layout->indexOf(widget);
    if (index == -1)
        return 0;
    QLayoutItem *item = d->layout->itemAt(index);
    return static_cast<QToolBarItem*>(item)->action;
}

/*! \fn QAction *QToolBar::actionAt(int x, int y) const
    \overload

    Returns the action at the point \a x, \a y. This function returns
    zero if no action was found.
*/

/*! \reimp */
void QToolBar::actionEvent(QActionEvent *event)
{
    Q_D(QToolBar);
    QAction *action = event->action();
    QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action);

    switch (event->type()) {
        case QEvent::ActionAdded: {
            Q_ASSERT_X(widgetAction == 0 || d->layout->indexOf(widgetAction) == -1,
                        "QToolBar", "widgets cannot be inserted multiple times");

            // reparent the action to this toolbar if it has been created
            // using the addAction(text) etc. convenience functions, to
            // preserve Qt 4.1.x behavior. The widget is already
            // reparented to us due to the createWidget call inside
            // createItem()
            if (widgetAction != 0 && widgetAction->d_func()->autoCreated)
                widgetAction->setParent(this);

            int index = d->layout->count();
            if (event->before()) {
                index = d->layout->indexOf(event->before());
                Q_ASSERT_X(index != -1, "QToolBar::insertAction", "internal error");
            }
            d->layout->insertAction(index, action);
            break;
        }

        case QEvent::ActionChanged:
            d->layout->invalidate();
            break;

        case QEvent::ActionRemoved: {
            int index = d->layout->indexOf(action);
            if (index != -1) {
                delete d->layout->takeAt(index);
            }
            break;
        }

        default:
            Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
    }
}

/*! \reimp */
void QToolBar::changeEvent(QEvent *event)
{
    Q_D(QToolBar);
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        d->toggleViewAction->setText(windowTitle());
        break;
    case QEvent::StyleChange:
        d->layout->invalidate();
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

/*! \reimp */
void QToolBar::paintEvent(QPaintEvent *)
{
    Q_D(QToolBar);

    QPainter p(this);
    QStyle *style = this->style();
    QStyleOptionToolBar opt;
    initStyleOption(&opt);

    if (d->layout->expanded || isWindow()) {
        p.fillRect(opt.rect, palette().background());
        style->drawPrimitive(QStyle::PE_FrameMenu, &opt, &p, this);
    } else {
        style->drawControl(QStyle::CE_ToolBar, &opt, &p, this);
    }

    opt.rect = d->layout->handleRect();
    if (opt.rect.isValid())
        style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

/*
    Checks if an expanded toolbar has to wait for this popup to close before
    the toolbar collapses. This is true if
    1) the popup has the toolbar in its parent chain,
    2) the popup is a menu whose menuAction is somewhere in the toolbar.
*/
static bool waitForPopup(QToolBar *tb, QWidget *popup)
{
    if (popup == 0 || popup->isHidden())
        return false;

    QWidget *w = popup;
    while (w != 0) {
        if (w == tb)
            return true;
        w = w->parentWidget();
    }

    QMenu *menu = qobject_cast<QMenu*>(popup);
    if (menu == 0)
        return false;

    QAction *action = menu->menuAction();
    QList<QWidget*> widgets = action->associatedWidgets();
    for (int i = 0; i < widgets.count(); ++i) {
        if (waitForPopup(tb, widgets.at(i)))
            return true;
    }

    return false;
}

/*! \reimp */
bool QToolBar::event(QEvent *event)
{
    Q_D(QToolBar);

    switch (event->type()) {
    case QEvent::Hide:
        if (!isHidden())
            break;
        // fallthrough intended
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
    case QEvent::ParentChange:
        d->layout->setUsePopupMenu(qobject_cast<QMainWindow*>(parentWidget()) == 0);
        break;
    case QEvent::StyleChange:
        if (!d->explicitIconSize)
            setIconSize(QSize());
        break;
    case QEvent::LayoutDirectionChange:
        d->layout->invalidate();
        break;
    case QEvent::MouseButtonPress: {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if (d->layout->handleRect().contains(e->pos())) {
            d->mousePressEvent(e);
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease:
        if (d->state != 0) {
            d->mouseReleaseEvent(static_cast<QMouseEvent*>(event));
            return true;
        }
        break;
    case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);

#ifndef QT_NO_CURSOR
            if (d->layout->handleRect().contains(e->pos()))
                setCursor(Qt::SizeAllCursor);
            else
                unsetCursor();
#endif

            if (d->state != 0) {
                d->mouseMoveEvent(e);
                return true;
            }
        }
        break;
    case QEvent::Leave:
        if (d->state != 0 && d->state->dragging) {
#ifdef Q_OS_WIN
            // The cursor has left the toolbar while it is being dragged,
            // we end the drag.
            d->endDrag();
#endif
        } else {
            if (!d->layout->expanded || d->layout->collapsing)
                break;

            QWidget *w = qApp->activePopupWidget();
            if (waitForPopup(this, w)) {
                d->waitForPopupTimer->start();
                break;
            }

            d->waitForPopupTimer->stop();
            d->layout->setExpanded(false);
            break;
        }
    default:
        break;
    }
    return QWidget::event(event);
}

void QToolBarPrivate::_q_waitForPopup()
{
    Q_Q(QToolBar);

    QWidget *w = qApp->activePopupWidget();
    if (!waitForPopup(q, w)) {
        waitForPopupTimer->stop();
        if (!q->underMouse())
            layout->setExpanded(false);
    }
}

/*!
    Returns a checkable action that can be used to show or hide this
    toolbar.

    The action's text is set to the toolbar's window title.

    \sa QAction::text QWidget::windowTitle
*/
QAction *QToolBar::toggleViewAction() const
{ Q_D(const QToolBar); return d->toggleViewAction; }

/*!
    \fn void QToolBar::setLabel(const QString &label)

    Use setWindowTitle() instead.
*/

/*!
    \fn QString QToolBar::label() const

    Use windowTitle() instead.
*/

/*!
    \since 4.2

    Returns the widget associated with the specified \a action.

    \sa addWidget()
*/
QWidget *QToolBar::widgetForAction(QAction *action) const
{
    Q_D(const QToolBar);

    int index = d->layout->indexOf(action);
    if (index == -1)
        return 0;

    return d->layout->itemAt(index)->widget();
}

/*!
    \internal
*/
void QToolBar::initStyleOption(QStyleOptionToolBar *option) const
{
    Q_D(const QToolBar);

    if (!option)
        return;

    option->initFrom(this);
    if (orientation() == Qt::Horizontal)
        option->state |= QStyle::State_Horizontal;
    option->lineWidth = style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, 0, this);
    option->features = d->layout->movable()
                        ? QStyleOptionToolBar::Movable
                        : QStyleOptionToolBar::None;

    // Add more styleoptions if the toolbar has been added to a mainwindow.
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());

    if (!mainWindow)
        return;

    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
    Q_ASSERT_X(layout != 0, "QToolBar::initStyleOption()",
               "QMainWindow->layout() != QMainWindowLayout");

    layout->getStyleOptionInfo(option, const_cast<QToolBar *>(this));
}

/*!
    \reimp
*/
void QToolBar::childEvent(QChildEvent *event) // ### remove me in 5.0
{
    QWidget::childEvent(event);
}

/*!
    \reimp
*/
void QToolBar::resizeEvent(QResizeEvent *event) // ### remove me in 5.0
{
    QWidget::resizeEvent(event);
}


#include "moc_qtoolbar.cpp"
#endif // QT_NO_TOOLBAR
