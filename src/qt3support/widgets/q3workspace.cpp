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

#include "q3workspace.h"
#ifndef QT_NO_WORKSPACE
#include "qapplication.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qicon.h"
#include "qimage.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qmenubar.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpointer.h"
#include "qscrollbar.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include "qvboxwidget.h"
#include "qdebug.h"
#include <private/q3titlebar_p.h>
#include <private/qwidget_p.h>
#include <private/qwidgetresizehandler_p.h>

#define d d_func()
#define q q_func()

/*!
    \class Q3Workspace
    \brief The Q3Workspace widget provides a workspace window that be
    used in an MDI application.

    \compat

    Multiple Document Interface (MDI) applications are typically
    composed of a main window containing a menu bar, a toolbar, and
    a central Q3Workspace widget. The workspace itself is used to display
    a number of child windows, each of which is a widget.

    The workspace itself is an ordinary Qt widget. It has a standard
    constructor that takes a parent widget and an object name.
    Workspaces can be placed in any layout, but are typically given
    as the central widget in a QMainWindow:

    \code
        MainWindow::MainWindow()
        {
            workspace = new Q3Workspace(this);
            setCentralWidget(workspace);
            ...
        }
    \endcode

    Child windows (MDI windows) are standard Qt widgets that are
    inserted into the workspace with addWindow(). As with top-level
    widgets, you can call functions such as show(), hide(),
    showMaximized(), and setWindowTitle() on a child window to change
    its appearance within the workspace. You can also provide widget
    flags to determine the layout of the decoration or the behavior of
    the widget itself.

    To change or retrieve the geometry of a child window, you must
    operate on its parentWidget(). The parentWidget() provides
    access to the decorated frame that contains the child window
    widget. When a child window is maximised, its decorated frame
    is hidden. If the top-level widget contains a menu bar, it will display
    the maximised window's operations menu to the left of the menu
    entries, and the window's controls to the right.

    A child window becomes active when it gets the keyboard focus,
    or when setFocus() is called. The user can activate a window by moving
    focus in the usual ways, for example by clicking a window or by pressing
    Tab. The workspace emits a signal windowActivated() when the active
    window changes, and the function activeWindow() returns a pointer to the
    active child window, or 0 if no window is active.

    The convenience function windowList() returns a list of all
    child windows. This information could be used in a
    popup menu containing a list of windows, for example.
    This feature is also available as part of the
    \link http://www.trolltech.com/products/solutions/catalog/Widgets/qtwindowlistmenu/
    Window Menu \endlink Qt Solution.

    Q3Workspace provides two built-in layout strategies for child
    windows: cascade() and tile(). Both are slots so you can easily
    connect menu entries to them.

    \img qworkspace-arrange.png

    If you want your users to be able to work with child windows
    larger than the visible workspace area, set the scrollBarsEnabled
    property to true.
*/

static bool inTitleChange = false;

class Q3WorkspaceChild : public QWidget
{
    Q_OBJECT

    friend class Q3WorkspacePrivate;
    friend class Q3Workspace;
    friend class Q3TitleBar;

public:
    Q3WorkspaceChild(QWidget* window, Q3Workspace* parent=0, Qt::WFlags flags = 0);
    ~Q3WorkspaceChild();

    void setActive(bool);
    bool isActive() const;

    void adjustToFullscreen();

    QWidget* windowWidget() const;
    QWidget* iconWidget() const;

    void doResize();
    void doMove();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QSize baseSize() const;

    int frameWidth() const;

    void show();

signals:
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);

public slots:
    void activate();
    void showMinimized();
    void showMaximized();
    void showNormal();
    void showShaded();
    void setWindowTitle(const QString&);
    void internalRaise();
    void titleBarDoubleClicked();

    void move(int x, int y);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void childEvent(QChildEvent*);
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    bool eventFilter(QObject *, QEvent *);

    bool focusNextPrevChild(bool);

    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);

private:
    Q_DISABLE_COPY(Q3WorkspaceChild)

    QWidget *childWidget;
    QWidget *backgroundWidget;
    QWidgetResizeHandler *widgetResizeHandler;
    Q3TitleBar *titlebar;
    QPointer<Q3TitleBar> iconw;
    QSize windowSize;
    QSize shadeRestore;
    QSize shadeRestoreMin;
    bool act                  :1;
    bool shademode            :1;
    bool snappedRight         :1;
    bool snappedDown          :1;
};

int Q3WorkspaceChild::frameWidth() const
{
    return contentsRect().left();
}



class Q3WorkspacePrivate : public QWidgetPrivate {
    Q_DECLARE_PUBLIC(Q3Workspace)
public:
    Q3WorkspaceChild* active;
    QList<Q3WorkspaceChild *> windows;
    QList<Q3WorkspaceChild *> focus;
    QList<QWidget *> icons;
    Q3WorkspaceChild* maxWindow;
    QRect maxRestore;
    QPointer<QFrame> maxcontrols;
    QPointer<QMenuBar> maxmenubar;
    QHash<int, const char*> shortcutMap;

    int px;
    int py;
    QWidget *becomeActive;
    QPointer<QLabel> maxtools;
    QString topTitle;

    QMenu *popup, *toolPopup;
    enum WSActs { RestoreAct, MoveAct, ResizeAct, MinimizeAct, MaximizeAct, CloseAct, StaysOnTopAct, ShadeAct, NCountAct };
    QAction *actions[NCountAct];

    QScrollBar *vbar, *hbar;
    QWidget *corner;
    int yoffset, xoffset;
    QBrush background;

    void init();
    void insertIcon(QWidget* w);
    void removeIcon(QWidget* w);
    void place(QWidget*);

    Q3WorkspaceChild* findChild(QWidget* w);
    void showMaximizeControls();
    void hideMaximizeControls();
    void activateWindow(QWidget* w, bool change_focus = true);
    void hideChild(Q3WorkspaceChild *c);
    void showWindow(QWidget* w);
    void maximizeWindow(QWidget* w);
    void minimizeWindow(QWidget* w);
    void normalizeWindow(QWidget* w);

    QRect updateWorkspace();

private:
    void normalizeActiveWindow();
    void minimizeActiveWindow();
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);
    void operationMenuActivated(QAction *);
    void scrollBarChanged();
    void updateActions();
};

static bool isChildOf(QWidget * child, QWidget * parent)
{
    if (!parent || !child)
        return false;
    QWidget * w = child;
    while(w && w != parent)
        w = w->parentWidget();
    return w != 0;
}

/*!
    Constructs a workspace with the given \a parent.
*/
Q3Workspace::Q3Workspace(QWidget *parent)
    : QWidget(*new Q3WorkspacePrivate, parent, 0)
{
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
Q3Workspace::Q3Workspace(QWidget *parent, const char *name)
    : QWidget(*new Q3WorkspacePrivate, parent, 0)
{
    setObjectName(name);
    d->init();
}

/*!
    \internal
*/
void
Q3WorkspacePrivate::init()
{
    d->maxcontrols = 0;
    d->active = 0;
    d->maxWindow = 0;
    d->maxtools = 0;
    d->px = 0;
    d->py = 0;
    d->becomeActive = 0;
    d->popup = new QMenu(q);
    d->toolPopup = new QMenu(q);
    d->popup->setObjectName("qt_internal_mdi_popup");
    d->toolPopup->setObjectName("qt_internal_mdi_tool_popup");

    d->actions[Q3WorkspacePrivate::RestoreAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarNormalButton)),
                                                            q->tr("&Restore"), q);
    d->actions[Q3WorkspacePrivate::MoveAct] = new QAction(q->tr("&Move"), q);
    d->actions[Q3WorkspacePrivate::ResizeAct] = new QAction(q->tr("&Size"), q);
    d->actions[Q3WorkspacePrivate::MinimizeAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarMinButton)),
                                                             q->tr("Mi&nimize"), q);
    d->actions[Q3WorkspacePrivate::MaximizeAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarMaxButton)),
                                                             q->tr("Ma&ximize"), q);
    d->actions[Q3WorkspacePrivate::CloseAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarCloseButton)),
                                                          q->tr("&Close")
                                                          +"\t"+(QString)QKeySequence(Qt::CTRL+Qt::Key_F4)
                                                          ,q);
    QObject::connect(d->actions[Q3WorkspacePrivate::CloseAct], SIGNAL(triggered()), q, SLOT(closeActiveWindow()));
    d->actions[Q3WorkspacePrivate::StaysOnTopAct] = new QAction(q->tr("Stay on &Top"), q);
    d->actions[Q3WorkspacePrivate::StaysOnTopAct]->setChecked(true);
    d->actions[Q3WorkspacePrivate::ShadeAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarShadeButton)),
                                                          q->tr("Sh&ade"), q);

    QObject::connect(d->popup, SIGNAL(aboutToShow()), q, SLOT(updateActions()));
    QObject::connect(d->popup, SIGNAL(triggered(QAction*)), q, SLOT(operationMenuActivated(QAction*)));
    d->popup->addAction(d->actions[Q3WorkspacePrivate::RestoreAct]);
    d->popup->addAction(d->actions[Q3WorkspacePrivate::MoveAct]);
    d->popup->addAction(d->actions[Q3WorkspacePrivate::ResizeAct]);
    d->popup->addAction(d->actions[Q3WorkspacePrivate::MinimizeAct]);
    d->popup->addAction(d->actions[Q3WorkspacePrivate::MaximizeAct]);
    d->popup->addSeparator();
    d->popup->addAction(d->actions[Q3WorkspacePrivate::CloseAct]);

    QObject::connect(d->toolPopup, SIGNAL(aboutToShow()), q, SLOT(updateActions()));
    QObject::connect(d->toolPopup, SIGNAL(triggered(QAction*)), q, SLOT(operationMenuActivated(QAction*)));
    d->toolPopup->addAction(d->actions[Q3WorkspacePrivate::MoveAct]);
    d->toolPopup->addAction(d->actions[Q3WorkspacePrivate::ResizeAct]);
    d->toolPopup->addAction(d->actions[Q3WorkspacePrivate::StaysOnTopAct]);
    d->toolPopup->addSeparator();
    d->toolPopup->addAction(d->actions[Q3WorkspacePrivate::ShadeAct]);
    d->toolPopup->addAction(d->actions[Q3WorkspacePrivate::CloseAct]);

    // Set up shortcut bindings (id -> slot), most used first
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::Key_Tab), "activateNextWindow");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab), "activatePreviousWindow");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::Key_F4), "closeActiveWindow");
    shortcutMap.insert(q->grabShortcut(Qt::ALT + Qt::Key_Minus), "showOperationMenu");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::Key_F6), "activateNextWindow");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_F6), "activatePreviousWindow");
    shortcutMap.insert(q->grabShortcut(Qt::Key_Forward), "activateNextWindow");
    shortcutMap.insert(q->grabShortcut(Qt::Key_Back), "activatePreviousWindow");

    background = q->palette().dark();
    q->setAttribute(Qt::WA_NoBackground, true);
    q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->topTitle = q->window()->windowTitle();
    d->hbar = d->vbar = 0;
    d->corner = 0;
    d->xoffset = d->yoffset = 0;

    updateWorkspace();

    q->window()->installEventFilter(q);
}

/*!
    Destroys the workspace and frees any allocated resources.
*/

Q3Workspace::~Q3Workspace()
{
}

/*! \reimp */
QSize Q3Workspace::sizeHint() const
{
    QSize s(QApplication::desktop()->size());
    return QSize(s.width()*2/3, s.height()*2/3);
}

/*!
    \overload

    Sets the palette background color to \a c.

    \sa setPaletteBackgroundPixmap()
*/
void Q3Workspace::setPaletteBackgroundColor(const QColor & c)
{
    d->background.setColor(c);
    update();
}


/*!
    \overload

    Sets the palette background pixmap to \a pm.

    \sa setPaletteBackgroundColor()
*/
void Q3Workspace::setPaletteBackgroundPixmap(const QPixmap & pm)
{
    d->background.setPixmap(pm);
    update();
}



/*!
  Adds widget \a w as new sub window to the workspace.  If \a flags
  are non-zero, they will override the flags set on the widget.

  Returns the window frame.

*/
QWidget * Q3Workspace::addWindow(QWidget *w, Qt::WFlags flags)
{
    if (!w)
        return 0;

    if ((flags & Qt::WindowType_Mask)  == Qt::Tool) {
        bool customize =  (flags & (Qt::WindowTitleHint
                                    | Qt::WindowSystemMenuHint
                                    | Qt::WindowMinimizeButtonHint
                                    | Qt::WindowMaximizeButtonHint
                                    | Qt::WindowContextHelpButtonHint));
        if (!customize)
            flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
    }

    bool wasMaximized = w->isMaximized();
    bool wasMinimized = w->isMinimized();
    bool hasBeenHidden = w->isExplicitlyHidden();
    bool hasSize = w->testAttribute(Qt::WA_Resized);
    int x = w->x();
    int y = w->y();
    bool hasPos = w->testAttribute(Qt::WA_Moved);
    QSize s = w->size().expandedTo(w->minimumSizeHint());
    if (!hasSize && w->sizeHint().isValid())
        w->adjustSize();

    Q3WorkspaceChild* child = new Q3WorkspaceChild(w, this, flags);
    child->setObjectName("qt_workspacechild");
    child->installEventFilter(this);

    connect(child, SIGNAL(popupOperationMenu(QPoint)),
            this, SLOT(popupOperationMenu(QPoint)));
    connect(child, SIGNAL(showOperationMenu()),
            this, SLOT(showOperationMenu()));
    d->windows.append(child);
    if (child->isVisibleTo(this))
        d->focus.append(child);
    child->internalRaise();

    if (!hasPos)
        d->place(child);
    if (hasSize)
        child->resize(s + child->baseSize());
    else
        child->adjustSize();
    if (hasPos)
        child->move(x, y);

    w->setHidden(hasBeenHidden);
    return child;

    if (wasMaximized)
        w->showMaximized();
    else if (wasMinimized)
        w->showMinimized();
    else if (!hasBeenHidden)
        d->activateWindow(w);

    d->updateWorkspace();
    return child;
}

/*! \reimp */
void Q3Workspace::childEvent(QChildEvent * e)
{
    if (e->type() == QEvent::ChildInserted && e->child()->isWidgetType()) {
        QWidget* w = static_cast<QWidget*>(e->child());
        if (!w || qobject_cast<Q3WorkspaceChild*>(w)
            || !(w->windowType() == Qt::Widget || w->windowType() == Qt::Dialog || w->windowType() == Qt::Tool || w->windowType() == Qt::Window)
            || d->icons.contains(w) || w == d->vbar || w == d->hbar || w == d->corner)
            return;
        addWindow(w, w->windowFlags());
    } else
        if (e->removed()) {
            if (d->windows.removeAll(static_cast<Q3WorkspaceChild*>(e->child()))) {
                d->focus.removeAll(static_cast<Q3WorkspaceChild*>(e->child()));
                if (d->maxWindow == e->child())
                    d->maxWindow = 0;
                d->updateWorkspace();
            }
        }
}

/*! \reimp */
#ifndef QT_NO_WHEELEVENT
void Q3Workspace::wheelEvent(QWheelEvent *e)
{
    if (!scrollBarsEnabled())
        return;
    if (d->vbar && d->vbar->isVisible() && !(e->modifiers() & Qt::AltModifier))
        QApplication::sendEvent(d->vbar, e);
    else if (d->hbar && d->hbar->isVisible())
        QApplication::sendEvent(d->hbar, e);
}
#endif

void Q3WorkspacePrivate::activateWindow(QWidget* w, bool change_focus)
{
    if (!w) {
        d->active = 0;
        emit q->windowActivated(0);
        return;
    }
    if (!q->isVisible()) {
        d->becomeActive = w;
        return;
    }

    if (d->active && d->active->windowWidget() == w) {
        if (!isChildOf(q->focusWidget(), w)) // child window does not have focus
            d->active->setActive(true);
        return;
    }

    d->active = 0;
    // First deactivate all other workspace clients
    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        Q3WorkspaceChild* c = *it;
        ++it;
        if (c->windowWidget() == w)
            d->active = c;
        else
            c->setActive(false);
    }

    if (!d->active)
        return;

    // Then activate the new one, so the focus is stored correctly
    d->active->setActive(true);

    if (!d->active)
        return;

    if (d->maxWindow && d->maxWindow != d->active && d->active->windowWidget() &&
        (d->active->windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint)) {
        d->active->showMaximized();
        if (d->maxtools) {
            QIcon icon = w->windowIcon();
            if (!icon.isNull()) {
                int iconSize = d->maxtools->size().height();
                QPixmap pm(icon.pixmap(QSize(iconSize, iconSize)));
                d->maxtools->setPixmap(pm);
            } else
            {
                QPixmap pm(14,14);
                pm.fill(Qt::color1);
                pm.setMask(pm.createHeuristicMask());
                d->maxtools->setPixmap(pm);
            }
        }
    }

    d->active->internalRaise();

    if (change_focus) {
	int from = d->focus.indexOf(d->active);
        if (from >= 0)
            d->focus.move(from, d->focus.size() - 1);
    }

    updateWorkspace();
    emit q->windowActivated(w);
}


/*!
    Returns a pointer to the widget corresponding to the active child
    window, or 0 if no window is active.
*/
QWidget* Q3Workspace::activeWindow() const
{
    return d->active?d->active->windowWidget():0;
}


void Q3WorkspacePrivate::place(QWidget *w)
{
    QList<QWidget *> widgets;
    for (QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin()); it != d->windows.end(); ++it)
        if (*it != w)
            widgets.append(*it);

    int overlap, minOverlap = 0;
    int possible;

    QRect r1(0, 0, 0, 0);
    QRect r2(0, 0, 0, 0);
    QRect maxRect = q->rect();
    int x = maxRect.left(), y = maxRect.top();
    QPoint wpos(maxRect.left(), maxRect.top());

    bool firstPass = true;

    do {
        if (y + w->height() > maxRect.bottom()) {
            overlap = -1;
        } else if(x + w->width() > maxRect.right()) {
            overlap = -2;
        } else {
            overlap = 0;

            r1.setRect(x, y, w->width(), w->height());

            QWidget *l;
            QList<QWidget *>::Iterator it(widgets.begin());
            while (it != widgets.end()) {
                l = *it;
                ++it;

                if (d->maxWindow == l)
                    r2 = d->maxRestore;
                else
                    r2.setRect(l->x(), l->y(), l->width(), l->height());

                if (r2.intersects(r1)) {
                    r2.setCoords(qMax(r1.left(), r2.left()),
                                 qMax(r1.top(), r2.top()),
                                 qMin(r1.right(), r2.right()),
                                 qMin(r1.bottom(), r2.bottom())
                                );

                    overlap += (r2.right() - r2.left()) *
                               (r2.bottom() - r2.top());
                }
            }
        }

        if (overlap == 0) {
            wpos = QPoint(x, y);
            break;
        }

        if (firstPass) {
            firstPass = false;
            minOverlap = overlap;
        } else if (overlap >= 0 && overlap < minOverlap) {
            minOverlap = overlap;
            wpos = QPoint(x, y);
        }

        if (overlap > 0) {
            possible = maxRect.right();
            if (possible - w->width() > x) possible -= w->width();

            QWidget *l;
            QList<QWidget *>::Iterator it(widgets.begin());
            while (it != widgets.end()) {
                l = *it;
                ++it;
                if (d->maxWindow == l)
                    r2 = d->maxRestore;
                else
                    r2.setRect(l->x(), l->y(), l->width(), l->height());

                if((y < r2.bottom()) && (r2.top() < w->height() + y)) {
                    if(r2.right() > x)
                        possible = possible < r2.right() ?
                                   possible : r2.right();

                    if(r2.left() - w->width() > x)
                        possible = possible < r2.left() - w->width() ?
                                   possible : r2.left() - w->width();
                }
            }

            x = possible;
        } else if (overlap == -2) {
            x = maxRect.left();
            possible = maxRect.bottom();

            if (possible - w->height() > y) possible -= w->height();

            QWidget *l;
            QList<QWidget *>::Iterator it(widgets.begin());
            while (it != widgets.end()) {
                l = *it;
                ++it;
                if (d->maxWindow == l)
                    r2 = d->maxRestore;
                else
                    r2.setRect(l->x(), l->y(), l->width(), l->height());

                if(r2.bottom() > y)
                    possible = possible < r2.bottom() ?
                               possible : r2.bottom();

                if(r2.top() - w->height() > y)
                    possible = possible < r2.top() - w->height() ?
                               possible : r2.top() - w->height();
            }

            y = possible;
        }
    }
    while(overlap != 0 && overlap != -1);

    w->move(wpos);
    updateWorkspace();
}


void Q3WorkspacePrivate::insertIcon(QWidget* w)
{
    if (!w || d->icons.contains(w))
        return;
    d->icons.append(w);
    if (w->parentWidget() != q) {
        w->setParent(q, 0);
        w->move(0,0);
    }
    QRect cr = updateWorkspace();
    int x = 0;
    int y = cr.height() - w->height();

    QList<QWidget *>::Iterator it(d->icons.begin());
    while (it != d->icons.end()) {
        QWidget* i = *it;
        ++it;
        if (x > 0 && x + i->width() > cr.width()){
            x = 0;
            y -= i->height();
        }

        if (i != w &&
            i->geometry().intersects(QRect(x, y, w->width(), w->height())))
            x += i->width();
    }
    w->move(x, y);

    if (q->isVisibleTo(q->parentWidget())) {
        w->show();
        w->lower();
    }
    updateWorkspace();
}


void Q3WorkspacePrivate::removeIcon(QWidget* w)
{
    if (d->icons.removeAll(w))
        w->hide();
}


/*! \reimp  */
void Q3Workspace::resizeEvent(QResizeEvent *)
{
    if (d->maxWindow) {
        d->maxWindow->adjustToFullscreen();
        if (d->maxWindow->windowWidget())
            d->maxWindow->windowWidget()->overrideWindowState(Qt::WindowMaximized);
    }

    QRect cr = d->updateWorkspace();

    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        Q3WorkspaceChild* c = *it;
        ++it;
        if (c->windowWidget() && !(c->windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint))
            continue;

        int x = c->x();
        int y = c->y();
        if (c->snappedDown)
            y =  cr.height() - c->height();
        if (c->snappedRight)
            x =  cr.width() - c->width();

        if (x != c->x() || y != c->y())
            c->move(x, y);
    }

}

/*! \reimp */
void Q3Workspace::showEvent(QShowEvent *e)
{
    if (d->maxWindow && !style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this))
        d->showMaximizeControls();
    QWidget::showEvent(e);
    if (d->becomeActive) {
        d->activateWindow(d->becomeActive);
        d->becomeActive = 0;
    } else if (d->windows.count() > 0 && !d->active) {
        d->activateWindow(d->windows.first()->windowWidget());
    }

    // force a frame repaint - this is a workaround for what seems to be a bug
    // introduced when changing the QWidget::show() implementation. Might be
    // a windows bug as well though.
    for (int i = 0; i < d->windows.count(); ++i) {
	Q3WorkspaceChild* c = d->windows.at(i);
        c->update(c->rect());
    }

    d->updateWorkspace();
}

/*! \reimp */
void Q3Workspace::hideEvent(QHideEvent *)
{
    if (!isVisible() && !style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this))
        d->hideMaximizeControls();
}

/*! \reimp */
void Q3Workspace::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), d->background);
}

void Q3WorkspacePrivate::minimizeWindow(QWidget* w)
{
    Q3WorkspaceChild* c = findChild(w);

    if (!w || !(w->windowFlags() & Qt::WindowMinimizeButtonHint))
        return;

    if (c) {
        bool wasMax = false;
        if (c == d->maxWindow) {
            wasMax = true;
            d->maxWindow = 0;
            inTitleChange = true;
            if (d->topTitle.size())
                q->window()->setWindowTitle(d->topTitle);
            inTitleChange = false;
            if (!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q))
                hideMaximizeControls();
            for (QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin()); it != d->windows.end(); ++it) {
                Q3WorkspaceChild* c = *it;
                if (c->titlebar)
                    c->titlebar->setMovable(true);
                c->widgetResizeHandler->setActive(true);
            }
        }
        c->hide();
        if (wasMax)
            c->setGeometry(d->maxRestore);
        if (!d->focus.contains(c))
            d->focus.append(c);
        insertIcon(c->iconWidget());

        if (!d->maxWindow)
            activateWindow(w);

        updateWorkspace();

        w->overrideWindowState(Qt::WindowMinimized);
        c->overrideWindowState(Qt::WindowMinimized);
    }
}

void Q3WorkspacePrivate::normalizeWindow(QWidget* w)
{
    Q3WorkspaceChild* c = findChild(w);
    if (!w)
        return;
    if (c) {
        w->overrideWindowState(Qt::WindowNoState);
        if (!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q)
            && d->maxWindow) {
            hideMaximizeControls();
        } else {
            if (w->minimumSize() != w->maximumSize())
                c->widgetResizeHandler->setActive(true);
            if (c->titlebar)
                c->titlebar->setMovable(true);
        }
        w->overrideWindowState(Qt::WindowNoState);
        c->overrideWindowState(Qt::WindowNoState);

        if (c == d->maxWindow) {
            c->setGeometry(d->maxRestore);
            d->maxWindow = 0;
            inTitleChange = true;
            if (d->topTitle.size())
                q->window()->setWindowTitle(d->topTitle);
            inTitleChange = false;
        } else {
            if (c->iconw)
                removeIcon(c->iconw->parentWidget());
            c->show();
        }

        if (!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q))
            hideMaximizeControls();
        for (QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin()); it != d->windows.end(); ++it) {
            Q3WorkspaceChild* c = *it;
            if (c->titlebar)
                c->titlebar->setMovable(true);
            if (c->childWidget && c->childWidget->minimumSize() != c->childWidget->maximumSize())
                c->widgetResizeHandler->setActive(true);
        }
        activateWindow(w, true);
        updateWorkspace();
    }
}

void Q3WorkspacePrivate::maximizeWindow(QWidget* w)
{
    Q3WorkspaceChild* c = findChild(w);

    if (!w || !(w->windowFlags() & Qt::WindowMaximizeButtonHint))
        return;

    if (!c || c == d->maxWindow)
        return;

    bool updatesEnabled = q->isUpdatesEnabled();
    q->setUpdatesEnabled(false);

    if (c->iconw && d->icons.contains(c->iconw->parentWidget()))
        normalizeWindow(w);
    QRect r(c->geometry());
    Q3WorkspaceChild *oldMaxWindow = d->maxWindow;
    d->maxWindow = c;
    c->adjustToFullscreen();
    c->show();
    c->internalRaise();
    if (oldMaxWindow != c) {
        if (oldMaxWindow)
            oldMaxWindow->setGeometry(d->maxRestore);
        d->maxRestore = r;
    }

    activateWindow(w);
    if(!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q)) {
        showMaximizeControls();
    } else {
        if (!d->active && d->becomeActive) {
            d->active = (Q3WorkspaceChild*)d->becomeActive->parentWidget();
            d->active->setActive(true);
            d->becomeActive = 0;
            emit q->windowActivated(d->active->windowWidget());
        }
        c->widgetResizeHandler->setActive(false);
        if (c->titlebar)
            c->titlebar->setMovable(false);
    }
    inTitleChange = true;
    if (d->topTitle.size())
        q->window()->setWindowTitle(q->tr("%1 - [%2]")
                                            .arg(d->topTitle).arg(c->windowTitle()));
    inTitleChange = false;

    updateWorkspace();

    w->overrideWindowState(Qt::WindowMaximized);
    c->overrideWindowState(Qt::WindowMaximized);
    q->setUpdatesEnabled(updatesEnabled);
}

void Q3WorkspacePrivate::showWindow(QWidget* w)
{
    if (w->isMinimized() && (w->windowFlags() & Qt::WStyle_Minimize))
        minimizeWindow(w);
    else if ((d->maxWindow || w->isMaximized()) && w->windowFlags() & Qt::WindowMaximizeButtonHint)
        maximizeWindow(w);
    else if (w->windowFlags() & Qt::WindowMaximizeButtonHint)
        normalizeWindow(w);
    else
        w->parentWidget()->show();
    if (d->maxWindow)
        d->maxWindow->internalRaise();
    updateWorkspace();
}


Q3WorkspaceChild* Q3WorkspacePrivate::findChild(QWidget* w)
{
    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        Q3WorkspaceChild* c = *it;
        ++it;
        if (c->windowWidget() == w)
            return c;
    }
    return 0;
}

/*!
    Returns a list of all child windows. If \a order is CreationOrder
    (the default), the windows are listed in the order in which they
    were inserted into the workspace. If \a order is StackingOrder,
    the windows are listed in their stacking order, with the topmost
    window as the last item in the list.
*/
QWidgetList Q3Workspace::windowList(WindowOrder order) const
{
    QWidgetList windows;
    if (order == StackingOrder) {
        QObjectList cl = children();
        for (int i = 0; i < cl.size(); ++i) {
            Q3WorkspaceChild *c = qobject_cast<Q3WorkspaceChild*>(cl.at(i));
            if (c && c->windowWidget())
                windows.append(c->windowWidget());
        }
    } else {
        QList<Q3WorkspaceChild *>::ConstIterator it(d->windows.begin());
        while (it != d->windows.end()) {
            Q3WorkspaceChild* c = *it;
            ++it;
            if (c->windowWidget())
                windows.append(c->windowWidget());
        }
    }
    return windows;
}

/*! \reimp */
bool Q3Workspace::eventFilter(QObject *o, QEvent * e)
{
    static QTime* t = 0;
    static Q3Workspace* tc = 0;
    if (o == d->maxtools) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            {
                QMenuBar* b = (QMenuBar*)o->parent();
                if (!t)
                    t = new QTime;
                if (tc != this || t->elapsed() > QApplication::doubleClickInterval()) {
                    if (QApplication::reverseLayout()) {
                        QPoint p = b->mapToGlobal(QPoint(b->x() + b->width(), b->y() + b->height()));
                        p.rx() -= d->popup->sizeHint().width();
                        d->popupOperationMenu(p);
                    } else {
                        d->popupOperationMenu(b->mapToGlobal(QPoint(b->x(), b->y() + b->height())));
                    }
                    t->start();
                    tc = this;
                } else {
                    tc = 0;
                    closeActiveWindow();
                }
                return true;
            }
        default:
            break;
        }
        return QWidget::eventFilter(o, e);
    }
    switch (e->type()) {
    case QEvent::HideToParent:
        break;
    case QEvent::ShowToParent:
        if (Q3WorkspaceChild *c = qobject_cast<Q3WorkspaceChild*>(o))
            if (!d->focus.contains(c))
                d->focus.append(c);
        d->updateWorkspace();
        break;
    case QEvent::WindowTitleChange:
        if (inTitleChange)
            break;

        inTitleChange = true;
        if (o == window()) {
            QWidget *tlw = (QWidget*)o;
            if (!d->maxWindow
                || tlw->windowTitle() != tr("%1 - [%2]").arg(d->topTitle).arg(d->maxWindow->windowTitle()))
                d->topTitle = tlw->windowTitle();
        }

        if (d->maxWindow && d->topTitle.size())
            window()->setWindowTitle(tr("%1 - [%2]")
                .arg(d->topTitle).arg(d->maxWindow->windowTitle()));
        inTitleChange = false;

        break;
    case QEvent::Close:
        if (o == window())
        {
            QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
            while (it != d->windows.end()) {
                Q3WorkspaceChild* c = *it;
                ++it;
                if (c->shademode)
                    c->showShaded();
            }
        } else if (qobject_cast<Q3WorkspaceChild*>(o)) {
            d->popup->hide();
        }
        d->updateWorkspace();
        break;
    case QEvent::Shortcut:
        {
            QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
            const char *theSlot = d->shortcutMap.value(se->shortcutId(), 0);
            if (theSlot)
                QMetaObject::invokeMember(this, theSlot);
        }
        break;
    default:
        break;
    }
    return QWidget::eventFilter(o, e);
}

void Q3WorkspacePrivate::showMaximizeControls()
{
    Q_ASSERT(d->maxWindow);
    QMenuBar* b = 0;

    // Do a breadth-first search first on every parent,
    QWidget* w = q->parentWidget();
    QList<QMenuBar*> l;
    while (l.isEmpty() && w) {
        l = qFindChildren<QMenuBar*>(w);
        w = w->parentWidget();
    }

    // and query recursively if nothing is found.
    if (!l.size())
        l = qFindChildren<QMenuBar*>(q->window());
    if (l.size())
        b = l.at(0);

    if (!b)
        return;

    if (!d->maxcontrols) {
        d->maxmenubar = b;
        d->maxcontrols = new QFrame(q->window());
        d->maxcontrols->setObjectName("qt_maxcontrols");
        QHBoxLayout* l = new QHBoxLayout(d->maxcontrols);
        l->setMargin(d->maxcontrols->frameWidth());
        l->setSpacing(0);
        if (d->maxWindow->windowWidget() &&
            (d->maxWindow->windowWidget()->windowFlags() & Qt::WStyle_Minimize)) {
            QToolButton* iconB = new QToolButton(d->maxcontrols);
            iconB->setObjectName("iconify");
            iconB->setToolTip(q->tr("Minimize"));
            l->addWidget(iconB);
            iconB->setFocusPolicy(Qt::NoFocus);
            QPixmap pm = q->style()->standardPixmap(QStyle::SP_TitleBarMinButton);
            iconB->setIcon(pm);
            iconB->setIconSize(pm.size());
            QObject::connect(iconB, SIGNAL(clicked()),
                             q, SLOT(minimizeActiveWindow()));
        }

        QToolButton* restoreB = new QToolButton(d->maxcontrols);
        restoreB->setObjectName("restore");
        restoreB->setToolTip(q->tr("Restore Down"));
        l->addWidget(restoreB);
        restoreB->setFocusPolicy(Qt::NoFocus);
        QPixmap pm = q->style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
        restoreB->setIcon(pm);
        restoreB->setIconSize(pm.size());
        QObject::connect(restoreB, SIGNAL(clicked()),
                         q, SLOT(normalizeActiveWindow()));

        l->addSpacing(2);
        QToolButton* closeB = new QToolButton(d->maxcontrols);
        closeB->setObjectName("close");
        closeB->setToolTip(q->tr("Close"));
        l->addWidget(closeB);
        closeB->setFocusPolicy(Qt::NoFocus);
        pm = q->style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
        closeB->setIcon(pm);
        closeB->setIconSize(pm.size());
        QObject::connect(closeB, SIGNAL(clicked()),
                         q, SLOT(closeActiveWindow()));

        d->maxcontrols->setFixedSize(d->maxcontrols->minimumSizeHint());
    }

    b->setCornerWidget(d->maxcontrols);
    d->maxcontrols->show();
    if (!d->active && d->becomeActive) {
        d->active = (Q3WorkspaceChild*)d->becomeActive->parentWidget();
        d->active->setActive(true);
        d->becomeActive = 0;
        emit q->windowActivated(d->active->windowWidget());
    }
    if (d->active) {
        if (!d->maxtools) {
            d->maxtools = new QLabel(q->window());
            d->maxtools->setObjectName("qt_maxtools");
            d->maxtools->installEventFilter(q);
        }
        if (d->active->windowWidget() && !d->active->windowWidget()->windowIcon().isNull()) {
            QIcon icon = d->active->windowWidget()->windowIcon();
            int iconSize = d->maxcontrols->size().height();
            d->maxtools->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));
        } else {
            QPixmap pm = q->style()->standardPixmap(QStyle::SP_TitleBarMenuButton);
            if (pm.isNull()) {
                pm = QPixmap(14,14);
                pm.fill(Qt::black);
            }
            d->maxtools->setPixmap(pm);
        }
        b->setCornerWidget(d->maxtools, Qt::TopLeftCorner);
        d->maxtools->show();
    }
}


void Q3WorkspacePrivate::hideMaximizeControls()
{
    if (d->maxmenubar) {
        d->maxmenubar->setCornerWidget(0, Qt::TopLeftCorner);
        d->maxmenubar->setCornerWidget(0, Qt::TopRightCorner);
    }
    delete d->maxcontrols;
    d->maxcontrols = 0;
    delete d->maxtools;
    d->maxtools = 0;
}

/*!
    Closes the child window that is currently active.

    \sa closeAllWindows()
*/
void Q3Workspace::closeActiveWindow()
{
    if (d->maxWindow && d->maxWindow->windowWidget())
        d->maxWindow->windowWidget()->close();
    else if (d->active && d->active->windowWidget())
        d->active->windowWidget()->close();
    d->updateWorkspace();
}

/*!
    Closes all child windows.

    If any child window fails to accept the close event, the remaining windows
    will remain open.

    \sa closeActiveWindow()
*/
void Q3Workspace::closeAllWindows()
{
    bool did_close = true;
    QList<Q3WorkspaceChild *>::const_iterator it = d->windows.constBegin();
    while (it != d->windows.constEnd() && did_close) {
        Q3WorkspaceChild *c = *it;
        ++it;
        if (c->windowWidget() && !c->windowWidget()->isExplicitlyHidden())
            did_close = c->windowWidget()->close();
    }
}

void Q3WorkspacePrivate::normalizeActiveWindow()
{
    if (d->maxWindow)
        d->maxWindow->showNormal();
    else if (d->active)
        d->active->showNormal();
}

void Q3WorkspacePrivate::minimizeActiveWindow()
{
    if (d->maxWindow)
        d->maxWindow->showMinimized();
    else if (d->active)
        d->active->showMinimized();
}

void Q3WorkspacePrivate::showOperationMenu()
{
    if  (!d->active || !d->active->windowWidget())
        return;
    Q_ASSERT((d->active->windowWidget()->windowFlags() & Qt::WindowSystemMenuHint));
    QPoint p;
    QMenu *popup = d->active->titlebar->isTool() ? d->toolPopup : d->popup;
    if (QApplication::reverseLayout()) {
        p = QPoint(d->active->windowWidget()->mapToGlobal(QPoint(d->active->windowWidget()->width(),0)));
        p.rx() -= popup->sizeHint().width();
    } else {
        p = QPoint(d->active->windowWidget()->mapToGlobal(QPoint(0,0)));
    }
    if (!d->active->isVisible()) {
        p = d->active->iconWidget()->mapToGlobal(QPoint(0,0));
        p.ry() -= popup->sizeHint().height();
    }
    popupOperationMenu(p);
}

void Q3WorkspacePrivate::popupOperationMenu(const QPoint&  p)
{
    if (!d->active || !d->active->windowWidget() || !(d->active->windowWidget()->windowFlags() & Qt::WindowSystemMenuHint))
        return;
    if ((d->active->titlebar->isTool()))
        d->toolPopup->popup(p);
    else
        d->popup->popup(p);
}

void Q3WorkspacePrivate::updateActions()
{
    for (int i = 1; i < NCountAct-1; i++) {
        bool enable = d->active != 0;
        d->actions[i]->setEnabled(enable);
    }

    if (!d->active || !d->active->windowWidget())
        return;

    QWidget *windowWidget = d->active->windowWidget();
    bool canResize = windowWidget->maximumSize() != windowWidget->minimumSize();
    d->actions[Q3WorkspacePrivate::ResizeAct]->setEnabled(canResize);
    d->actions[Q3WorkspacePrivate::MinimizeAct]->setEnabled((windowWidget->windowFlags() & Qt::WStyle_Minimize));
    d->actions[Q3WorkspacePrivate::MaximizeAct]->setEnabled((windowWidget->windowFlags() & Qt::WindowMaximizeButtonHint) && canResize);

    if (d->active == d->maxWindow) {
        d->actions[Q3WorkspacePrivate::MoveAct]->setEnabled(false);
        d->actions[Q3WorkspacePrivate::ResizeAct]->setEnabled(false);
        d->actions[Q3WorkspacePrivate::MaximizeAct]->setEnabled(false);
    } else if (d->active->isVisible()){
        d->actions[Q3WorkspacePrivate::RestoreAct]->setEnabled(false);
    } else {
        d->actions[Q3WorkspacePrivate::MoveAct]->setEnabled(false);
        d->actions[Q3WorkspacePrivate::ResizeAct]->setEnabled(false);
        d->actions[Q3WorkspacePrivate::MinimizeAct]->setEnabled(false);
    }
    if (d->active->shademode) {
        d->actions[Q3WorkspacePrivate::ShadeAct]->setIcon(
            QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarUnshadeButton)));
        d->actions[Q3WorkspacePrivate::ShadeAct]->setText(q->tr("&Unshade"));
    } else {
        d->actions[Q3WorkspacePrivate::ShadeAct]->setIcon(
            QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarShadeButton)));
        d->actions[Q3WorkspacePrivate::ShadeAct]->setText(q->tr("Sh&ade"));
    }
    d->actions[Q3WorkspacePrivate::StaysOnTopAct]->setEnabled(!d->active->shademode && canResize);
    d->actions[Q3WorkspacePrivate::StaysOnTopAct]->setChecked(
        (d->active->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint));
}

void Q3WorkspacePrivate::operationMenuActivated(QAction *action)
{
    if (!d->active)
        return;
    if(action == d->actions[Q3WorkspacePrivate::RestoreAct]) {
        d->active->showNormal();
    } else if(action == d->actions[Q3WorkspacePrivate::MoveAct]) {
        d->active->doMove();
    } else if(action == d->actions[Q3WorkspacePrivate::ResizeAct]) {
        if (d->active->shademode)
            d->active->showShaded();
        d->active->doResize();
    } else if(action == d->actions[Q3WorkspacePrivate::MinimizeAct]) {
        d->active->showMinimized();
    } else if(action == d->actions[Q3WorkspacePrivate::MaximizeAct]) {
        d->active->showMaximized();
    } else if(action == d->actions[Q3WorkspacePrivate::ShadeAct]) {
        d->active->showShaded();
    } else if(action == d->actions[Q3WorkspacePrivate::StaysOnTopAct]) {
        if(QWidget* w = d->active->windowWidget()) {
            if ((w->windowFlags() & Qt::WindowStaysOnTopHint)) {
                w->overrideWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
            } else {
                w->overrideWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
                w->parentWidget()->raise();
            }
        }
    }
}


void Q3WorkspacePrivate::hideChild(Q3WorkspaceChild *c)
{
    bool updatesEnabled = q->isUpdatesEnabled();
    q->setUpdatesEnabled(false);
    focus.removeAll(c);
    QRect restore;
    if (d->maxWindow == c)
        restore = maxRestore;
    if (active == c) {
        q->setFocus();
        q->activatePreviousWindow();
    }
    if (active == c)
        activateWindow(0);
    if (maxWindow == c) {
        hideMaximizeControls();
        maxWindow = 0;
    }
    c->hide();
    if (!restore.isEmpty())
        c->setGeometry(restore);
    q->setUpdatesEnabled(updatesEnabled);
}

/*!
    Gives the input focus to the next window in the list of child
    windows.

    \sa activatePreviousWindow()
*/
void Q3Workspace::activateNextWindow()
{
    if (d->focus.isEmpty())
        return;
    if (!d->active) {
        if (d->focus.first())
            d->activateWindow(d->focus.first()->windowWidget(), false);
        return;
    }

    int a = d->focus.indexOf(d->active) + 1;

    a = a % d->focus.count();

    if (d->focus.at(a))
        d->activateWindow(d->focus.at(a)->windowWidget(), false);
    else
        d->activateWindow(0);
}

/*!
    Gives the input focus to the previous window in the list of child
    windows.

    \sa activateNextWindow()
*/
void Q3Workspace::activatePreviousWindow()
{
    if (d->focus.isEmpty())
        return;
    if (!d->active) {
        if (d->focus.last())
            d->activateWindow(d->focus.first()->windowWidget(), false);
        else
            d->activateWindow(0);
        return;
    }

    int a = d->focus.indexOf(d->active) - 1;
    if (a < 0)
        a = d->focus.count()-1;

    if (d->focus.at(a))
        d->activateWindow(d->focus.at(a)->windowWidget(), false);
    else
        d->activateWindow(0);
}


/*!
    \fn void Q3Workspace::windowActivated(QWidget* w)

    This signal is emitted when the child window \a w becomes active.
    Note that \a w can be 0, and that more than one signal may be
    emitted for a single activation event.

    \sa activeWindow(), windowList()
*/

/*!
    Arranges all the child windows in a cascade pattern.

    \sa tile()
*/
void Q3Workspace::cascade()
{
    blockSignals(true);
    if  (d->maxWindow)
        d->maxWindow->showNormal();

    if (d->vbar) {
        d->vbar->blockSignals(true);
        d->vbar->setValue(0);
        d->vbar->blockSignals(false);
        d->hbar->blockSignals(true);
        d->hbar->setValue(0);
        d->hbar->blockSignals(false);
        d->scrollBarChanged();
    }

    const int xoffset = 13;
    const int yoffset = 20;

    // make a list of all relevant mdi clients
    QList<Q3WorkspaceChild *> widgets;
    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    Q3WorkspaceChild* wc = 0;

    for (it = d->focus.begin(); it != d->focus.end(); ++it) {
        wc = *it;
        if (wc->windowWidget()->isVisibleTo(this) && !(wc->titlebar->isTool()))
            widgets.append(wc);
    }

    int x = 0;
    int y = 0;

    it = widgets.begin();
    while (it != widgets.end()) {
        Q3WorkspaceChild *child = *it;
        ++it;

        QSize prefSize = child->windowWidget()->sizeHint().expandedTo(child->windowWidget()->minimumSizeHint());
        if (!prefSize.isValid())
            prefSize = child->windowWidget()->size();
        prefSize = prefSize.expandedTo(child->windowWidget()->minimumSize()).boundedTo(child->windowWidget()->maximumSize());
        if (prefSize.isValid())
            prefSize += QSize(child->baseSize().width(), child->baseSize().height());

        int w = prefSize.width();
        int h = prefSize.height();

        child->showNormal();
        if (y + h > height())
            y = 0;
        if (x + w > width())
            x = 0;
        child->setGeometry(x, y, w, h);
        x += xoffset;
        y += yoffset;
        child->internalRaise();
    }
    d->updateWorkspace();
    blockSignals(false);
}

/*!
    Arranges all child windows in a tile pattern.

    \sa cascade()
*/
void Q3Workspace::tile()
{
    blockSignals(true);
    QWidget *oldActive = d->active ? d->active->windowWidget() : 0;
    if  (d->maxWindow)
        d->maxWindow->showNormal();

    if (d->vbar) {
        d->vbar->blockSignals(true);
        d->vbar->setValue(0);
        d->vbar->blockSignals(false);
        d->hbar->blockSignals(true);
        d->hbar->setValue(0);
        d->hbar->blockSignals(false);
        d->scrollBarChanged();
    }

    int rows = 1;
    int cols = 1;
    int n = 0;
    Q3WorkspaceChild* c;

    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        c = *it;
        ++it;
        if (!c->windowWidget()->isExplicitlyHidden()
            && !(c->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)
            && !c->iconw)
            n++;
    }

    while (rows * cols < n) {
        if (cols <= rows)
            cols++;
        else
            rows++;
    }
    int add = cols * rows - n;
    bool* used = new bool[cols*rows];
    for (int i = 0; i < rows*cols; i++)
        used[i] = false;

    int row = 0;
    int col = 0;
    int w = width() / cols;
    int h = height() / rows;

    it = d->windows.begin();
    while (it != d->windows.end()) {
        c = *it;
        ++it;
        if (c->iconw || c->windowWidget()->isExplicitlyHidden() || (c->titlebar->isTool()))
            continue;
        if (!row && !col) {
            w -= c->baseSize().width();
            h -= c->baseSize().height();
        }
        if ((c->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)) {
            QPoint p = c->pos();
            if (p.x()+c->width() < 0)
                p.setX(0);
            if (p.x() > width())
                p.setX(width() - c->width());
            if (p.y() + 10 < 0)
                p.setY(0);
            if (p.y() > height())
                p.setY(height() - c->height());

            if (p != c->pos())
                c->QWidget::move(p);
        } else {
            c->showNormal();
            used[row*cols+col] = true;
            QSize sz(w, h);
            QSize bsize(c->baseSize());
            sz = sz.expandedTo(c->windowWidget()->minimumSize()).boundedTo(c->windowWidget()->maximumSize());
            sz += bsize;

	    if ( add ) {
                if (sz.height() == h + bsize.height()) // no relevant constrains
                    sz.rheight() *= 2;
		used[(row+1)*cols+col] = true;
		add--;
	    }

            c->setGeometry(col*w + col*bsize.width(), row*h + row*bsize.height(), sz.width(), sz.height());

            while(row < rows && col < cols && used[row*cols+col]) {
                col++;
                if (col == cols) {
                    col = 0;
                    row++;
                }
            }
        }
    }
    delete [] used;

    d->activateWindow(oldActive);
    d->updateWorkspace();
    blockSignals(false);
}

Q3WorkspaceChild::Q3WorkspaceChild(QWidget* window, Q3Workspace *parent, Qt::WFlags flags)
    : QWidget(parent,
             Qt::WStyle_NoBorder | Qt::WStyle_Customize
             | Qt::WNoMousePropagation | Qt::SubWindow)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setMouseTracking(true);
    act = false;
    iconw = 0;
    shademode = false;
    titlebar = 0;
    snappedRight = false;
    snappedDown = false;

    backgroundWidget = new QWidget(this);
    backgroundWidget->setAttribute(Qt::WA_NoSystemBackground);
    if (window) {
        if (flags)
            window->setParent(this, flags & ~Qt::WindowType_Mask);
        else
            window->setParent(this);
        switch (window->focusPolicy()) {
        case Qt::NoFocus:
            window->setFocusPolicy(Qt::ClickFocus);
            break;
        case Qt::TabFocus:
            window->setFocusPolicy(Qt::StrongFocus);
            break;
        default:
            break;
        }
    }

    if (window && (flags & Qt::WStyle_Title)) {
        titlebar = new Q3TitleBar(window, this, flags);
        connect(titlebar, SIGNAL(doActivate()),
                 this, SLOT(activate()));
        connect(titlebar, SIGNAL(doClose()),
                 window, SLOT(close()));
        connect(titlebar, SIGNAL(doMinimize()),
                 this, SLOT(showMinimized()));
        connect(titlebar, SIGNAL(doNormal()),
                 this, SLOT(showNormal()));
        connect(titlebar, SIGNAL(doMaximize()),
                 this, SLOT(showMaximized()));
        connect(titlebar, SIGNAL(popupOperationMenu(QPoint)),
                 this, SIGNAL(popupOperationMenu(QPoint)));
        connect(titlebar, SIGNAL(showOperationMenu()),
                 this, SIGNAL(showOperationMenu()));
        connect(titlebar, SIGNAL(doShade()),
                 this, SLOT(showShaded()));
        connect(titlebar, SIGNAL(doubleClicked()),
                 this, SLOT(titleBarDoubleClicked()));
    }

    setMinimumSize(128, 0);
    int fw =  style()->pixelMetric(QStyle::PM_MDIFrameWidth, 0, this);
    setContentsMargins(fw, fw, fw, fw);

    childWidget = window;
    if (!childWidget)
        return;

    setWindowTitle(childWidget->windowTitle());

    QPoint p;
    QSize s;
    QSize cs;

    bool hasBeenResized = childWidget->testAttribute(Qt::WA_Resized);

    if (!hasBeenResized)
        cs = childWidget->sizeHint().expandedTo(childWidget->minimumSizeHint());
    else
        cs = childWidget->size();

    int th = titlebar ? titlebar->sizeHint().height() : 0;
    if (titlebar) {
        if (!childWidget->windowIcon().isNull())
            titlebar->setWindowIcon(childWidget->windowIcon());
        if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            th += frameWidth();
        else
            th -= contentsRect().y();

        p = QPoint(contentsRect().x(),
                    th + contentsRect().y());
        s = QSize(cs.width() + 2*frameWidth(),
                   cs.height() + 2*frameWidth() + th);
    } else {
        p = QPoint(contentsRect().x(), contentsRect().y());
        s = QSize(cs.width() + 2*frameWidth(),
                   cs.height() + 2*frameWidth());
    }

    childWidget->move(p);
    resize(s);

    childWidget->installEventFilter(this);

    widgetResizeHandler = new QWidgetResizeHandler(this, window);
    widgetResizeHandler->setSizeProtection(!parent->scrollBarsEnabled());
    widgetResizeHandler->setFrameWidth(frameWidth());
    connect(widgetResizeHandler, SIGNAL(activate()),
             this, SLOT(activate()));
    if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
        widgetResizeHandler->setExtraHeight(th + contentsRect().y() - 2*frameWidth());
    else
        widgetResizeHandler->setExtraHeight(th + contentsRect().y() - frameWidth());
    if (childWidget->minimumSize() == childWidget->maximumSize())
        widgetResizeHandler->setActive(QWidgetResizeHandler::Resize, false);
    setBaseSize(baseSize());
}

Q3WorkspaceChild::~Q3WorkspaceChild()
{
    if (iconw)
        delete iconw->parentWidget();

    Q3Workspace *workspace = qobject_cast<Q3Workspace*>(parentWidget());
    if (workspace) {
        workspace->d->focus.removeAll(this);
        if (workspace->d->active == this)
            workspace->activatePreviousWindow();
        if (workspace->d->active == this)
            workspace->d->activateWindow(0);
        if (workspace->d->maxWindow == this) {
            workspace->d->hideMaximizeControls();
            workspace->d->maxWindow = 0;
        }
    }
}

void Q3WorkspaceChild::moveEvent(QMoveEvent *)
{
    ((Q3Workspace*)parentWidget())->d->updateWorkspace();
}

void Q3WorkspaceChild::resizeEvent(QResizeEvent *)
{
    bool wasMax = isMaximized();
    QRect r = contentsRect();
    QRect cr;

    if (titlebar) {
        int th = titlebar->sizeHint().height();
        QRect tbrect(0, 0, width(), th);
        if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            tbrect = QRect(r.x(), r.y(), r.width(), th);
        titlebar->setGeometry(tbrect);

        if (style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            th -= frameWidth();
        cr = QRect(r.x(), r.y() + th + (shademode ? (frameWidth() * 3) : 0),
                    r.width(), r.height() - th);
    } else {
        cr = r;
    }

    if (!childWidget)
        return;

    windowSize = cr.size();
    backgroundWidget->setGeometry(cr);
    childWidget->setGeometry(cr);
    ((Q3Workspace*)parentWidget())->d->updateWorkspace();

    if (wasMax) {
        overrideWindowState(Qt::WindowMaximized);
        childWidget->overrideWindowState(Qt::WindowMaximized);
    }
}

QSize Q3WorkspaceChild::baseSize() const
{
    int th = titlebar ? titlebar->sizeHint().height() : 0;
    if (style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
        th -= frameWidth();
    return QSize(2*frameWidth(), 2*frameWidth() + th);
}

QSize Q3WorkspaceChild::sizeHint() const
{
    if (!childWidget)
        return QWidget::sizeHint() + baseSize();

    QSize prefSize = windowWidget()->sizeHint().expandedTo(windowWidget()->minimumSizeHint());
    prefSize = prefSize.expandedTo(windowWidget()->minimumSize()).boundedTo(windowWidget()->maximumSize());
    prefSize += baseSize();

    return prefSize;
}

QSize Q3WorkspaceChild::minimumSizeHint() const
{
    if (!childWidget)
        return QWidget::minimumSizeHint() + baseSize();
    QSize s = childWidget->minimumSize();
    if (s.isEmpty())
        s = childWidget->minimumSizeHint();
    return s + baseSize();
}

void Q3WorkspaceChild::activate()
{
    ((Q3Workspace*)parentWidget())->d->activateWindow(windowWidget());
}

bool Q3WorkspaceChild::eventFilter(QObject * o, QEvent * e)
{
    if (!isActive() && (e->type() == QEvent::MouseButtonPress ||
        e->type() == QEvent::FocusIn)) {
        if (iconw) {
            ((Q3Workspace*)parentWidget())->d->normalizeWindow(windowWidget());
            if (iconw) {
                ((Q3Workspace*)parentWidget())->d->removeIcon(iconw->parentWidget());
                delete iconw->parentWidget();
                iconw = 0;
            }
        }
        activate();
    }

    // for all widgets except the window, that's the only thing we
    // process, and if we have no childWidget we skip totally
    if (o != childWidget || childWidget == 0)
        return false;

    switch (e->type()) {
    case QEvent::ShowToParent:
        if (((Q3Workspace*)parentWidget())->d->focus.indexOf(this) < 0)
            ((Q3Workspace*)parentWidget())->d->focus.append(this);

        if (windowWidget() && (windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)) {
            internalRaise();
            show();
            backgroundWidget->lower();
        }
        ((Q3Workspace*)parentWidget())->d->showWindow(windowWidget());
        break;
    case QEvent::WindowStateChange: {
        Qt::WindowStates state = windowWidget()->windowState();

        if (state & Qt::WindowMaximized) {
            if (windowWidget()->maximumSize().isValid() &&
                (windowWidget()->maximumWidth() < parentWidget()->width() ||
                 windowWidget()->maximumHeight() < parentWidget()->height())) {
                windowWidget()->resize(windowWidget()->maximumSize());
                windowWidget()->overrideWindowState(Qt::WindowNoState);
                if (titlebar)
                    titlebar->repaint();
                break;
            }
            if ((windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint))
                ((Q3Workspace*)parentWidget())->d->maximizeWindow(windowWidget());
            else
                ((Q3Workspace*)parentWidget())->d->normalizeWindow(windowWidget());
        } else if (state & Qt::WindowMinimized) {
            ((Q3Workspace*)parentWidget())->d->minimizeWindow(windowWidget());
        } else {
            ((Q3Workspace*)parentWidget())->d->normalizeWindow(windowWidget());
            if (iconw) {
                ((Q3Workspace*)parentWidget())->d->removeIcon(iconw->parentWidget());
                delete iconw->parentWidget();
            }
        }
    } break;
    case QEvent::HideToParent:
    {
        QWidget * w = iconw;
        if (w && (w = w->parentWidget())) {
            ((Q3Workspace*)parentWidget())->d->removeIcon(w);
            delete w;
        }
        ((Q3Workspace*)parentWidget())->d->hideChild(this);
    } break;
    case QEvent::WindowTitleChange:
        setWindowTitle(childWidget->windowTitle());
        if (iconw)
            iconw->setWindowTitle(childWidget->windowTitle());
        break;
    case QEvent::WindowIconChange:
        {
            Q3Workspace* ws = (Q3Workspace*)parentWidget();
            if (!titlebar)
                break;
            int iconSize = titlebar->size().height();
            QIcon icon = childWidget->windowIcon();

            titlebar->setWindowIcon(icon);
            if (iconw)
                iconw->setWindowIcon(icon);

            if (ws->d->maxWindow != this)
                break;

            if (ws->d->maxtools)
                ws->d->maxtools->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));
        }
        break;
    case QEvent::Resize:
        {
            QResizeEvent* re = (QResizeEvent*)e;
            if (re->size() != windowSize && !shademode)
                resize(re->size() + baseSize());
        }
        break;

    case QEvent::WindowDeactivate:
        if (titlebar)
            titlebar->setActive(false);
        repaint();
        break;

    case QEvent::WindowActivate:
        if (titlebar)
            titlebar->setActive(act);
        repaint();
        break;

    default:
        break;
    }

    return QWidget::eventFilter(o, e);
}

bool Q3WorkspaceChild::focusNextPrevChild(bool next)
{
    extern Q_GUI_EXPORT bool qt_tab_all_widgets;
    uint focus_flag = qt_tab_all_widgets ? Qt::TabFocus : Qt::StrongFocus;

    QWidget *f = focusWidget();
    if (!f)
        f = this;

    QWidget *w = f;
    QWidget *test = f->nextInFocusChain();
    while (test != f) {
        if ((test->focusPolicy() & focus_flag) == focus_flag
            && !(test->focusProxy()) && test->isVisibleTo(this)
            && test->isEnabled() && isAncestorOf(w)) {
            w = test;
            if (next)
                break;
        }
        test = test->nextInFocusChain();
    }
    if (w == f)
        return false;
    w->setFocus();
    return true;
}

void Q3WorkspaceChild::childEvent(QChildEvent* e)
{
    if (e->type() == QEvent::ChildRemoved && e->child() == childWidget) {
        childWidget = 0;
        if (iconw) {
            ((Q3Workspace*)parentWidget())->d->removeIcon(iconw->parentWidget());
            delete iconw->parentWidget();
        }
        close();
    }
}


void Q3WorkspaceChild::doResize()
{
    widgetResizeHandler->doResize();
}

void Q3WorkspaceChild::doMove()
{
    widgetResizeHandler->doMove();
}

void Q3WorkspaceChild::enterEvent(QEvent *)
{
}

void Q3WorkspaceChild::leaveEvent(QEvent *)
{
#ifndef QT_NO_CURSOR
    if (!widgetResizeHandler->isButtonDown())
        setCursor(Qt::ArrowCursor);
#endif
}

void Q3WorkspaceChild::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    opt.lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
    opt.midLineWidth = 1;

    if (titlebar && titlebar->isActive())
        opt.state |= QStyle::State_Active;

    style()->drawPrimitive(QStyle::PE_FrameWindow, &opt, &p, this);
}

void Q3WorkspaceChild::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        resizeEvent(0);
        if (iconw) {
            QVBoxWidget *vbox = qobject_cast<QVBoxWidget*>(iconw->parentWidget());
            Q_ASSERT(vbox);
            if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar)) {
                vbox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
                vbox->resize(196+2*vbox->frameWidth(), 20 + 2*vbox->frameWidth());
            } else {
                vbox->resize(196, 20);
            }
        }
    }
    QWidget::changeEvent(ev);
}

void Q3WorkspaceChild::setActive(bool b)
{
    if (!childWidget)
        return;

    bool hasFocus = isChildOf(window()->focusWidget(), this);
    if (act == b && hasFocus)
        return;

    act = b;

    if (titlebar)
        titlebar->setActive(act);
    if (iconw)
        iconw->setActive(act);
    update();

    QList<QWidget*> wl = qFindChildren<QWidget*>(childWidget);
    if (act) {
        for (int i = 0; i < wl.size(); ++i) {
            QWidget *w = wl.at(i);
            w->removeEventFilter(this);
        }
        if (!hasFocus) {
            QWidget *lastfocusw = childWidget->focusWidget();
            if (lastfocusw && lastfocusw->focusPolicy() != Qt::NoFocus) {
                lastfocusw->setFocus();
            } else if (childWidget->focusPolicy() != Qt::NoFocus) {
                childWidget->setFocus();
            } else {
                // find something, anything, that accepts focus, and use that.
                for (int i = 0; i < wl.size(); ++i) {
                    QWidget *w = wl.at(i);
                    if(w->focusPolicy() != Qt::NoFocus) {
                        w->setFocus();
                        break;
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < wl.size(); ++i) {
            QWidget *w = wl.at(i);
            w->removeEventFilter(this);
            w->installEventFilter(this);
        }
    }
}

bool Q3WorkspaceChild::isActive() const
{
    return act;
}

QWidget* Q3WorkspaceChild::windowWidget() const
{
    return childWidget;
}


QWidget* Q3WorkspaceChild::iconWidget() const
{
    if (!iconw) {
        Q3WorkspaceChild* that = (Q3WorkspaceChild*) this;

        QVBoxWidget* vbox = new QVBoxWidget(that, Qt::Window);
        vbox->setObjectName("qt_vbox");
        Q3TitleBar *tb = new Q3TitleBar(windowWidget(), vbox);
        tb->setObjectName("_workspacechild_icon_");
        QStyleOptionTitleBar opt = tb->getStyleOption();
        int th = style()->pixelMetric(QStyle::PM_TitleBarHeight, &opt, tb);
        int iconSize = style()->pixelMetric(QStyle::PM_MDIMinimizedWidth, 0, this);
        if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar)) {
            vbox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
            vbox->resize(iconSize+2*vbox->frameWidth(), th+2*vbox->frameWidth());
        } else {
            vbox->resize(iconSize, th);
        }
        that->iconw = tb;
        iconw->setActive(isActive());

        connect(iconw, SIGNAL(doActivate()),
                 this, SLOT(activate()));
        connect(iconw, SIGNAL(doClose()),
                 windowWidget(), SLOT(close()));
        connect(iconw, SIGNAL(doNormal()),
                 this, SLOT(showNormal()));
        connect(iconw, SIGNAL(doMaximize()),
                 this, SLOT(showMaximized()));
        connect(iconw, SIGNAL(popupOperationMenu(QPoint)),
                 this, SIGNAL(popupOperationMenu(QPoint)));
        connect(iconw, SIGNAL(showOperationMenu()),
                 this, SIGNAL(showOperationMenu()));
        connect(iconw, SIGNAL(doubleClicked()),
                 this, SLOT(titleBarDoubleClicked()));
    }
    if (windowWidget()) {
        iconw->setWindowTitle(windowWidget()->windowTitle());
    }
    return iconw->parentWidget();
}

void Q3WorkspaceChild::showMinimized()
{
    windowWidget()->setWindowState(Qt::WindowMinimized | (windowWidget()->windowState() & ~Qt::WindowMaximized));
}

void Q3WorkspaceChild::showMaximized()
{
    windowWidget()->setWindowState(Qt::WindowMaximized | (windowWidget()->windowState() & ~Qt::WindowMinimized));
}

void Q3WorkspaceChild::showNormal()
{
    windowWidget()->setWindowState(windowWidget()->windowState() & ~(Qt::WindowMinimized|Qt::WindowMaximized));
}

void Q3WorkspaceChild::showShaded()
{
    if (!titlebar)
        return;
    ((Q3Workspace*)parentWidget())->d->activateWindow(windowWidget());
    QWidget* w = windowWidget();
    if (shademode) {
        w->overrideWindowState(Qt::WindowNoState);
        overrideWindowState(Qt::WindowNoState);

        shademode = false;
        resize(shadeRestore.expandedTo(minimumSizeHint()));
        setMinimumSize(shadeRestoreMin);
        style()->polish(this);
    } else {
        shadeRestore = size();
        shadeRestoreMin = minimumSize();
        setMinimumHeight(0);
        shademode = true;
        w->overrideWindowState(Qt::WindowMinimized);
        overrideWindowState(Qt::WindowMinimized);

        if (style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            resize(width(), titlebar->height());
        else
            resize(width(), titlebar->height() + 2*frameWidth() + 1);
        style()->polish(this);
    }
    titlebar->update();
}

void Q3WorkspaceChild::titleBarDoubleClicked()
{
    if (!windowWidget())
        return;
    if (iconw)
        showNormal();
    else if (windowWidget()->windowFlags() & Qt::WindowShadeButtonHint)
            showShaded();
    else if (windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint)
        showMaximized();
}

void Q3WorkspaceChild::adjustToFullscreen()
{
    if (!childWidget)
        return;

    if(style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this)) {
        setGeometry(parentWidget()->rect());
    } else {
        int fw =  style()->pixelMetric(QStyle::PM_MDIFrameWidth, 0, this);
        int th = titlebar ? titlebar->sizeHint().height() : 0;
        int w = parentWidget()->width() + 2*fw;
        int h = parentWidget()->height() + 2*fw + th;
        w = qMax(w, childWidget->minimumWidth());
        h = qMax(h, childWidget->minimumHeight());
        setGeometry(-fw, -fw - th, w, h);
    }
    childWidget->overrideWindowState(Qt::WindowMaximized);
    overrideWindowState(Qt::WindowMaximized);
}

void Q3WorkspaceChild::setWindowTitle(const QString& cap)
{
    if (titlebar)
        titlebar->setWindowTitle(cap);
    QWidget::setWindowTitle(cap);
}

void Q3WorkspaceChild::internalRaise()
{

    QWidget *stackUnderWidget = 0;
    if (!windowWidget() || (windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint) == 0) {

        QList<Q3WorkspaceChild *>::Iterator it(((Q3Workspace*)parent())->d->windows.begin());
        while (it != ((Q3Workspace*)parent())->d->windows.end()) {
            Q3WorkspaceChild* c = *it;
            ++it;
            if (c->windowWidget() &&
                !c->windowWidget()->isExplicitlyHidden() &&
                (c->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)) {
                if (stackUnderWidget)
                    c->stackUnder(stackUnderWidget);
                else
                    c->raise();
                stackUnderWidget = c;
            }
        }
    }

    if (stackUnderWidget) {
        if (iconw)
            iconw->parentWidget()->stackUnder(stackUnderWidget);
        stackUnder(stackUnderWidget);
    } else {
        if (iconw)
            iconw->parentWidget()->raise();
        raise();
    }

}

void Q3WorkspaceChild::show()
{
    if (childWidget && childWidget->isExplicitlyHidden())
        childWidget->show();
    QWidget::show();
}

/*
  ## Does not work anymore because QWidgetResizeHandler calls move() and move is no longer virtual
 */
void Q3WorkspaceChild::move(int x, int y)
{
    int nx = x;
    int ny = y;

    if (titlebar->isTool()) {
        int dx = 10;
        int dy = 10;

        if (QABS(x) < dx)
            nx = 0;
        if (QABS(y) < dy)
            ny = 0;
        if (QABS(x + width() - parentWidget()->width()) < dx) {
            nx = parentWidget()->width() - width();
            snappedRight = true;
        } else
            snappedRight = false;

        if (QABS(y + height() - parentWidget()->height()) < dy) {
            ny = parentWidget()->height() - height();
            snappedDown = true;
        } else
            snappedDown = false;
    }
    QWidget::move(nx, ny);
}

bool Q3Workspace::scrollBarsEnabled() const
{
    return d->vbar != 0;
}

/*!
    \property Q3Workspace::scrollBarsEnabled
    \brief whether the workspace provides scrollbars

    If this property is true, the workspace will provide scrollbars if any
    of the child windows extend beyond the edges of the visible
    workspace. The workspace area will automatically increase to
    contain child windows if they are resized beyond the right or
    bottom edges of the visible area.

    If this property is false (the default), resizing child windows
    out of the visible area of the workspace is not permitted, although
    it is still possible to position them partially outside the visible area.
*/
void Q3Workspace::setScrollBarsEnabled(bool enable)
{
    if ((d->vbar != 0) == enable)
        return;

    d->xoffset = d->yoffset = 0;
    if (enable) {
        d->vbar = new QScrollBar(Qt::Vertical, this);
        d->vbar->setObjectName("vertical scrollbar");
        connect(d->vbar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarChanged()));
        d->hbar = new QScrollBar(Qt::Horizontal, this);
        d->hbar->setObjectName("horizontal scrollbar");
        connect(d->hbar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarChanged()));
        d->corner = new QWidget(this);
        d->corner->setObjectName("qt_corner");
        d->updateWorkspace();
    } else {
        delete d->vbar;
        delete d->hbar;
        delete d->corner;
        d->vbar = d->hbar = 0;
        d->corner = 0;
    }

    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        Q3WorkspaceChild *child = *it;
        ++it;
        child->widgetResizeHandler->setSizeProtection(!enable);
    }
}

QRect Q3WorkspacePrivate::updateWorkspace()
{
    QRect cr(q->rect());

    if (q->scrollBarsEnabled() && !d->maxWindow) {
        d->corner->raise();
        d->vbar->raise();
        d->hbar->raise();
        if (d->maxWindow)
            d->maxWindow->internalRaise();

        QRect r(0, 0, 0, 0);
        QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
        while (it != d->windows.end()) {
            Q3WorkspaceChild *child = *it;
            ++it;
            if (!child->isExplicitlyHidden())
                r = r.unite(child->geometry());
        }
        d->vbar->blockSignals(true);
        d->hbar->blockSignals(true);

        int hsbExt = d->hbar->sizeHint().height();
        int vsbExt = d->vbar->sizeHint().width();


        bool showv = d->yoffset || d->yoffset + r.bottom() - q->height() + 1 > 0 || d->yoffset + r.top() < 0;
        bool showh = d->xoffset || d->xoffset + r.right() - q->width() + 1 > 0 || d->xoffset + r.left() < 0;

        if (showh && !showv)
            showv = d->yoffset + r.bottom() - q->height() + hsbExt + 1 > 0;
        if (showv && !showh)
            showh = d->xoffset + r.right() - q->width() + vsbExt  + 1 > 0;

        if (!showh)
            hsbExt = 0;
        if (!showv)
            vsbExt = 0;

        if (showv) {
            d->vbar->setSingleStep(qMax(q->height() / 12, 30));
            d->vbar->setPageStep(q->height() - hsbExt);
            d->vbar->setMinimum(qMin(0, d->yoffset + qMin(0, r.top())));
            d->vbar->setMaximum(qMax(0, d->yoffset + qMax(0, r.bottom() - q->height() + hsbExt + 1)));
            d->vbar->setGeometry(q->width() - vsbExt, 0, vsbExt, q->height() - hsbExt);
            d->vbar->setValue(d->yoffset);
            d->vbar->show();
        } else {
            d->vbar->hide();
        }

        if (showh) {
            d->hbar->setSingleStep(qMax(q->width() / 12, 30));
            d->hbar->setPageStep(q->width() - vsbExt);
            d->hbar->setMinimum(qMin(0, d->xoffset + qMin(0, r.left())));
            d->hbar->setMaximum(qMax(0, d->xoffset + qMax(0, r.right() - q->width() + vsbExt  + 1)));
            d->hbar->setGeometry(0, q->height() - hsbExt, q->width() - vsbExt, hsbExt);
            d->hbar->setValue(d->xoffset);
            d->hbar->show();
        } else {
            d->hbar->hide();
        }

        if (showh && showv) {
            d->corner->setGeometry(q->width() - vsbExt, q->height() - hsbExt, vsbExt, hsbExt);
            d->corner->show();
        } else {
            d->corner->hide();
        }

        d->vbar->blockSignals(false);
        d->hbar->blockSignals(false);

        cr.setRect(0, 0, q->width() - vsbExt, q->height() - hsbExt);
    }

    QList<QWidget *>::Iterator ii(d->icons.begin());
    while (ii != d->icons.end()) {
        QWidget* w = *ii;
        ++ii;
        int x = w->x();
        int y = w->y();
        bool m = false;
        if (x+w->width() > cr.width()) {
            m = true;
            x =  cr.width() - w->width();
        }
        if (y+w->height() >  cr.height()) {
            y =  cr.height() - w->height();
            m = true;
        }
        if (m) {
            if (Q3WorkspaceChild *child = qobject_cast<Q3WorkspaceChild*>(w))
                child->move(x, y);
            else
                w->move(x, y);
        }
    }

    return cr;

}

void Q3WorkspacePrivate::scrollBarChanged()
{
    int ver = d->yoffset - d->vbar->value();
    int hor = d->xoffset - d->hbar->value();
    d->yoffset = d->vbar->value();
    d->xoffset = d->hbar->value();

    QList<Q3WorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        Q3WorkspaceChild *child = *it;
        ++it;
        // we do not use move() due to the reimplementation in Q3WorkspaceChild
        child->setGeometry(child->x() + hor, child->y() + ver, child->width(), child->height());
    }
    d->updateWorkspace();
}

/*!
    \enum Q3Workspace::WindowOrder

    Specifies the order in which child windows are returned from windowList().

    \value CreationOrder The windows are returned in the order of their creation
    \value StackingOrder The windows are returned in the order of their stacking
*/

/*!\reimp */
void Q3Workspace::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        if (isVisible() && d->maxWindow) {
            if(style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this))
                d->hideMaximizeControls();
            else
                d->showMaximizeControls();
        }
    }
    QWidget::changeEvent(ev);
}

#endif // QT_NO_WORKSPACE

#include "moc_q3workspace.cpp"
#include "q3workspace.moc"
