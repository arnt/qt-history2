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

#include "qmainwindow.h"
#include "qmainwindowlayout_p.h"

#ifndef QT_NO_MAINWINDOW

#include "qdockwidget.h"
#include "qtoolbar.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qevent.h>
#include <qstyle.h>
#include <qdebug.h>
#include <qpainter.h>

#include <private/qwidget_p.h>
#include "qtoolbar_p.h"
#include "qwidgetanimator_p.h"

class QMainWindowPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMainWindow)
public:
    inline QMainWindowPrivate()
        : layout(0), toolButtonStyle(Qt::ToolButtonIconOnly)
    { }
    QMainWindowLayout *layout;
    QSize iconSize;
    bool explicitIconSize;
    Qt::ToolButtonStyle toolButtonStyle;
    void init();
    QList<int> hoverSeparator;
    QPoint hoverPos;

#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
    QCursor separatorCursor(const QList<int> &path) const;
    void adjustCursor(const QPoint &pos);
#endif
};

void QMainWindowPrivate::init()
{
    Q_Q(QMainWindow);
    layout = new QMainWindowLayout(q);
    const int metric = q->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    iconSize = QSize(metric, metric);
    explicitIconSize = false;

    q->setAttribute(Qt::WA_Hover);
}

/*
    The Main Window:

    +----------------------------------------------------------+
    | Menu Bar                                                 |
    +----------------------------------------------------------+
    | Tool Bar Area                                            |
    |   +--------------------------------------------------+   |
    |   | Dock Window Area                                 |   |
    |   |   +------------------------------------------+   |   |
    |   |   |                                          |   |   |
    |   |   | Central Widget                           |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   |                                          |   |   |
    |   |   +------------------------------------------+   |   |
    |   |                                                  |   |
    |   +--------------------------------------------------+   |
    |                                                          |
    +----------------------------------------------------------+
    | Status Bar                                               |
    +----------------------------------------------------------+

*/

/*!
    \class QMainWindow
    \brief The QMainWindow class implements a main application
    window.

    \ingroup application
    \mainclass

    A main window provides a framework for building an
    application's user interface. Qt has QMainWindow and its \l{Main
    Window and Related Classes}{related classes} for main window
    management. QMainWindow has its own layout to which you can add
    toolbars, dock windows, a menu bar, and a status bar. The layout
    has a center area that can be occupied by any kind of widget. You
    can see an image of the layout below.
    
    (bilde av komponenter main-window related classes/framework. layout)
    
    \tableofcontents
    
    \section1 Creating MainWindow Components
    
    A central widget will typically be a standard Qt widget such
    as a QTextEdit or a QGraphicsView. Custom widgets can also be
    used for advanced applications. You set the central widget with \c
    setCentralWidget(). 
    
    Main windows have either a single (SDI) or multiple (MDI)
    document interface. You create MDI applications in Qt by using a
    QWorkspace as the central widget.
    
    We will now examine each of the other widgets that can be
    added to a main window. We give examples on how to create and add
    them.
    
    \section2 Creating Menus
    
    Qt implements menus in QMenu and QMainWindow keeps them in a
    QMenuBar. \l{QAction}{QActions} are added to the menus, which
    display them as menu items.
    
    You can add new menus to the main window's menu bar by calling
    \c menuBar(), which returns the QMenuBar for the window, and then
    add a menu with QMenuBar::addMenu().
    
    QMainWindow comes with a default menu bar, but you can also
    set one yourself with \c setMenuBar(). If you wish to implement a
    custom menu bar, i.e., not use the QMenuBar widget, you can set it
    with \c setMenuWidget().

    An example of how to create menus follows:
    
    \quotefromfile application/mainwindow.cpp
    \skipuntil /::createMenus/
    \printuntil /saveAct/
    
    The \c createPopupMenu() function creates popup menus when the
    main window receives context menu events.  The default
    implementation generates a menu with the checkable actions from
    the dock widgets and toolbars. You can reimplement \c
    createPopupMenu() for a custom menu.
    
    \section2 Creating Toolbars
    
    Toolbars are implemented in the QToolBar class.  You add a
    toolbar to a main window with \c addToolBar().
    
    You control the initial position of toolbars by assigning them
    to a specific Qt::ToolBarArea. You can split an area by inserting
    a toolbar break--think of this as a line break in text
    editing--with \c addToolBarBreak() or \c insertToolBarBreak(). You
    can also restrict placement by the user with
    QToolBar::setAllowedAreas() and QToolbar::setMovable(). 
    
    The size of toolbar icons can be retrieved with \c iconSize().
    The sizes are platform dependent; you can set a fixed size with \c
    setIconSize(). You can alter the appearance of all tool buttons in
    the toolbars with \c setToolButtonStyle(). 
    
    An example of toolbar creation follows:
    
    \quotefromfile mainwindow/application/mainwindow.cpp
    \skipto /::createToolBars/
    \printuntil /fileToolBar->addAction/
    
    \section2 Creating Dock Widgets
    
    Dock widgets are implemented in the QDockWidget class. A dock
    widget is a window that can be docked into the main window.  You
    add dock widgets to a main window with \c addDockWidget().
    
    There are four dock widget areas as given by the
    Qt::DockWidgetArea enum: left, right, top, and bottom. You can
    specify which dock widget area that should occupy the corners
    where the areas overlap with \c setDockWidgetCorner(). By default
    each area can only contain one row (vertical or horizontal) of
    dock widgets, but if you enable nesting with \c
    setDockNestingEnabled(), dock widgets can be added in either
    direction.
    
    Two dock widgets may also be stacked on top of each other. A
    \l{QTabBar}{tab bar} is then used to select which widget that
    should be displayed.
    
    We give an example of how to create and add dock widgets to a
    main window:
    
    \quotefromfile snippets/mainwindowsnippet.cpp
    \skipto /QDockWidget \*dockWidget/
    \printuntil /addDockWidget/

    \section2 The Status Bar    

    You can set a status bar with \c setStatusBar(), but one is
    created the first time \c statusBar(), which returns the main
    window's status bar, is called. See QStatusBar for information on
    how to use it.
    
    \section1 Storing State
    
    QMainWindow can store the state of its layout with \c
    saveState(); it can later be retrieved with \c restoreState(). It
    is the position and size (relative to the size of the main window)
    of the toolbars and dock widgets that are stored.
    
    \sa QMenuBar, QToolBar, QStatusBar, QDockWidget, {Application
    Example}, {Dock Widgets Example}, {MDI Example}, {SDI Example},
    {Menus Example}
*/

/*!
    \fn void QMainWindow::iconSizeChanged(const QSize &iconSize)

    This signal is emitted when the size of the icons used in the
    window is changed. The new icon size is passed in \a iconSize.

    You can connect this signal to other components to help maintain
    a consistent appearance for your application.

    \sa setIconSize()
*/

/*!
    \fn void QMainWindow::toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle)

    This signal is emitted when the style used for tool buttons in the
    window is changed. The new style is passed in \a toolButtonStyle.

    You can connect this signal to other components to help maintain
    a consistent appearance for your application.

    \sa setToolButtonStyle()
*/

/*!
    Constructs a QMainWindow with the given \a parent and the specified
    widget \a flags.
 */
QMainWindow::QMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::Window)
{
    d_func()->init();
}

#ifdef QT3_SUPPORT
/*!
    \obsolete
    Constructs a QMainWindow with the given \a parent, \a name, and
    with the specified widget \a flags.
 */
QMainWindow::QMainWindow(QWidget *parent, const char *name, Qt::WindowFlags flags)
    : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::WType_TopLevel)
{
    setObjectName(QString::fromAscii(name));
    d_func()->init();
}
#endif

/*!
    Destroys the main window.
 */
QMainWindow::~QMainWindow()
{ }

/*! \property QMainWindow::iconSize
    \brief size of toolbar icons in this mainwindow.

    The default is the default tool bar icon size of the GUI style.
*/

QSize QMainWindow::iconSize() const
{ return d_func()->iconSize; }

void QMainWindow::setIconSize(const QSize &iconSize)
{
    Q_D(QMainWindow);
    QSize sz = iconSize;
    if (!sz.isValid()) {
        const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        sz = QSize(metric, metric);
    }
    if (d->iconSize != sz) {
        d->iconSize = sz;
        emit iconSizeChanged(d->iconSize);
    }
    d->explicitIconSize = iconSize.isValid();
}

/*! \property QMainWindow::toolButtonStyle
    \brief style of toolbar buttons in this mainwindow.

    The default is Qt::ToolButtonIconOnly.
*/

Qt::ToolButtonStyle QMainWindow::toolButtonStyle() const
{ return d_func()->toolButtonStyle; }

void QMainWindow::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
    Q_D(QMainWindow);
    if (d->toolButtonStyle == toolButtonStyle)
        return;
    d->toolButtonStyle = toolButtonStyle;
    emit toolButtonStyleChanged(d->toolButtonStyle);
}

#ifndef QT_NO_MENUBAR
/*!
    Returns the menu bar for the main window. This function creates
    and returns an empty menu bar if the menu bar does not exist.

    \sa setMenuBar()
*/
QMenuBar *QMainWindow::menuBar() const
{
    QMenuBar *menuBar = qobject_cast<QMenuBar *>(d_func()->layout->menuBar());
    if (!menuBar) {
	QMainWindow *self = const_cast<QMainWindow *>(this);
	menuBar = new QMenuBar(self);
	self->setMenuBar(menuBar);
    }
    return menuBar;
}

/*!
    Sets the menu bar for the main window to \a menuBar.

    Note: QMainWindow takes ownership of the \a menuBar pointer and
    deletes it at the appropriate time.

    \sa menuBar()
*/
void QMainWindow::setMenuBar(QMenuBar *menuBar)
{
    Q_D(QMainWindow);
    if (d->layout->menuBar() && d->layout->menuBar() != menuBar)
        delete d->layout->menuBar();
    d->layout->setMenuBar(menuBar);
}

/*!
    \since 4.2

    Returns the menu bar for the main window. This function returns
    null if a menubar hasn't been constructed yet.
*/
QWidget *QMainWindow::menuWidget() const
{
    QWidget *menuBar = d_func()->layout->menuBar();
    return menuBar;
}

/*!
    \since 4.2

    Sets the menu bar for the main window to \a menuBar.

    QMainWindow takes ownership of the \a menuBar pointer and
    deletes it at the appropriate time.
*/
void QMainWindow::setMenuWidget(QWidget *menuBar)
{
    Q_D(QMainWindow);
    if (d->layout->menuBar() && d->layout->menuBar() != menuBar)
        delete d->layout->menuBar();
    d->layout->setMenuBar(menuBar);
}
#endif // QT_NO_MENUBAR

#ifndef QT_NO_STATUSBAR
/*!
    Returns the status bar for the main window. This function creates
    and returns an empty status bar if the status bar does not exist.

    \sa setStatusBar()
*/
QStatusBar *QMainWindow::statusBar() const
{
    QStatusBar *statusbar = d_func()->layout->statusBar();
    if (!statusbar) {
	QMainWindow *self = const_cast<QMainWindow *>(this);
	statusbar = new QStatusBar(self);
        statusbar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	self->setStatusBar(statusbar);
    }
    return statusbar;
}

/*!
    Sets the status bar for the main window to \a statusbar.

    Setting the status bar to 0 will remove it from the main window.
    Note that QMainWindow takes ownership of the \a statusbar pointer
    and deletes it at the appropriate time.

    \sa statusBar()
*/
void QMainWindow::setStatusBar(QStatusBar *statusbar)
{
    Q_D(QMainWindow);
    if (d->layout->statusBar() && d->layout->statusBar() != statusbar)
        delete d->layout->statusBar();
    d->layout->setStatusBar(statusbar);
}
#endif // QT_NO_STATUSBAR

/*!
    Returns the central widget for the main window. This function
    returns zero if the central widget has not been set.

    \sa setCentralWidget()
*/
QWidget *QMainWindow::centralWidget() const
{ return d_func()->layout->centralWidget(); }

/*!
    Sets the given \a widget to be the main window's central widget.

    Note: QMainWindow takes ownership of the \a widget pointer and
    deletes it at the appropriate time.

    \sa centralWidget()
*/
void QMainWindow::setCentralWidget(QWidget *widget)
{
    Q_D(QMainWindow);
    if (d->layout->centralWidget() && d->layout->centralWidget() != widget)
        delete d->layout->centralWidget();
    d->layout->setCentralWidget(widget);
}

#ifndef QT_NO_DOCKWIDGET
/*!
    Sets the given dock widget \a area to occupy the specified \a
    corner.

    \sa corner()
*/
void QMainWindow::setCorner(Qt::Corner corner, Qt::DockWidgetArea area)
{
    bool valid = false;
    switch (corner) {
    case Qt::TopLeftCorner:
        valid = (area == Qt::TopDockWidgetArea || area == Qt::LeftDockWidgetArea);
        break;
    case Qt::TopRightCorner:
        valid = (area == Qt::TopDockWidgetArea || area == Qt::RightDockWidgetArea);
        break;
    case Qt::BottomLeftCorner:
        valid = (area == Qt::BottomDockWidgetArea || area == Qt::LeftDockWidgetArea);
        break;
    case Qt::BottomRightCorner:
        valid = (area == Qt::BottomDockWidgetArea || area == Qt::RightDockWidgetArea);
        break;
    }
    if (!valid)
        qWarning("QMainWindow::setCorner(): 'area' is not valid for 'corner'");
    else
        d_func()->layout->setCorner(corner, area);
}

/*!
    Returns the dock widget area that occupies the specified \a
    corner.

    \sa setCorner()
*/
Qt::DockWidgetArea QMainWindow::corner(Qt::Corner corner) const
{ return d_func()->layout->corner(corner); }
#endif

#ifndef QT_NO_TOOLBAR

static bool checkToolBarArea(Qt::ToolBarArea area, const char *where)
{
    switch (area) {
    case Qt::LeftToolBarArea:
    case Qt::RightToolBarArea:
    case Qt::TopToolBarArea:
    case Qt::BottomToolBarArea:
        return true;
    default:
        break;
    }
    qWarning("%s: invalid 'area' argument", where);
    return false;
}

/*!
    Adds a toolbar break to the given \a area after all the other
    objects that are present.
*/
void QMainWindow::addToolBarBreak(Qt::ToolBarArea area)
{
    if (!checkToolBarArea(area, "QMainWindow::addToolBarBreak"))
        return;
    d_func()->layout->addToolBarBreak(area);
}

/*!
    Inserts a toolbar break before the toolbar specified by \a before.
*/
void QMainWindow::insertToolBarBreak(QToolBar *before)
{ d_func()->layout->insertToolBarBreak(before); }

/*!
    Removes a toolbar break previously inserted before the toolbar specified by \a before.
*/

void QMainWindow::removeToolBarBreak(QToolBar *before)
{
    Q_D(QMainWindow);
    d->layout->removeToolBarBreak(before);
}

/*!
    Adds the \a toolbar into the specified \a area in this main
    window. The \a toolbar is placed at the end of the current tool
    bar block (i.e. line). If the main window already manages \a toolbar
    then it will only move the toolbar to \a area.

    \sa insertToolBar() addToolBarBreak() insertToolBarBreak()
*/
void QMainWindow::addToolBar(Qt::ToolBarArea area, QToolBar *toolbar)
{
    if (!checkToolBarArea(area, "QMainWindow::addToolBar"))
        return;

    Q_D(QMainWindow);

    if (!toolbar->isAreaAllowed(area))
        qWarning("QMainWIndow::addToolBar(): specified 'area' is not an allowed for this toolbar");

    disconnect(this, SIGNAL(iconSizeChanged(QSize)),
               toolbar, SLOT(_q_updateIconSize(QSize)));
    disconnect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
               toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

    d->layout->removeWidget(toolbar);

    toolbar->d_func()->_q_updateIconSize(d->iconSize);
    toolbar->d_func()->_q_updateToolButtonStyle(d->toolButtonStyle);
    connect(this, SIGNAL(iconSizeChanged(QSize)),
            toolbar, SLOT(_q_updateIconSize(QSize)));
    connect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
            toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

    d->layout->addToolBar(area, toolbar);

    if (isVisible())
        d->layout->relayout();
}

/*! \overload
    Equivalent of calling addToolBar(Qt::TopToolBarArea, \a toolbar)
*/
void QMainWindow::addToolBar(QToolBar *toolbar)
{ addToolBar(Qt::TopToolBarArea, toolbar); }

/*!
    \overload

    Creates a QToolBar object, setting its window title to \a title,
    and inserts it into the top toolbar area.

    \sa setWindowTitle()
*/
QToolBar *QMainWindow::addToolBar(const QString &title)
{
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setWindowTitle(title);
    addToolBar(toolBar);
    return toolBar;
}

/* Removes the toolbar from the mainwindow so that it can be added again. Does not
   explicitly hide the toolbar. */
static void qt_remove_toolbar_from_layout(QToolBar *toolbar, QMainWindowPrivate *d)
{
    if (toolbar) {
        QObject::disconnect(d->q_ptr, SIGNAL(iconSizeChanged(QSize)),
                   toolbar, SLOT(_q_updateIconSize(QSize)));
        QObject::disconnect(d->q_ptr, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

        d->layout->removeWidget(toolbar);
    }
}

/*!
    Inserts the \a toolbar into the area occupied by the \a before toolbar
    so that it appears before it. For example, in normal left-to-right
    layout operation, this means that \a toolbar will appear to the left
    of the toolbar specified by \a before in a horizontal toolbar area.

    \sa insertToolBarBreak() addToolBar() addToolBarBreak()
*/
void QMainWindow::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
    Q_D(QMainWindow);
    Q_ASSERT_X(toolbar->isAreaAllowed(toolBarArea(before)),
               "QMainWIndow::insertToolBar", "specified 'area' is not an allowed area");

    qt_remove_toolbar_from_layout(toolbar, d);

    toolbar->d_func()->_q_updateIconSize(d->iconSize);
    toolbar->d_func()->_q_updateToolButtonStyle(d->toolButtonStyle);
    connect(this, SIGNAL(iconSizeChanged(QSize)),
            toolbar, SLOT(_q_updateIconSize(QSize)));
    connect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
            toolbar, SLOT(_q_updateToolButtonStyle(Qt::ToolButtonStyle)));

    d->layout->insertToolBar(before, toolbar);

    if (isVisible())
        d->layout->relayout();
}

/*!
    Removes the \a toolbar from the main window layout and hides
    it. Note that the \a toolbar is \e not deleted.
*/
void QMainWindow::removeToolBar(QToolBar *toolbar)
{
    if (toolbar) {
        qt_remove_toolbar_from_layout(toolbar, d_func());
        toolbar->hide();
    }
}

/*!
    Returns the Qt::ToolBarArea for \a toolbar. If \a toolbar has not
    been added to the main window, this function returns \c
    Qt::NoToolBarArea.

    \sa addToolBar() addToolBarBreak() Qt::ToolBarArea
*/
Qt::ToolBarArea QMainWindow::toolBarArea(QToolBar *toolbar) const
{ return d_func()->layout->toolBarArea(toolbar); }

/*!

    Returns whether there is a toolbar
    break before the \a toolbar.

    \sa  addToolBarBreak(), insertToolBarBreak()
*/
bool QMainWindow::toolBarBreak(QToolBar *toolbar) const
{
    return d_func()->layout->toolBarBreak(toolbar);
}

#endif // QT_NO_TOOLBAR

#ifndef QT_NO_DOCKWIDGET

/*! \property QMainWindow::animated
    \brief whether manipulating dock widgets is animated
    \since 4.2

    When a dock widget is dragged over the main window, other dock widgets in the
    window will adjust themselves to make space for the dragged widget. If this
    property is set to true, their movement will be animated. The default value
    is true.
*/

bool QMainWindow::isAnimated() const
{
    return d_func()->layout->animationEnabled;
}

void QMainWindow::setAnimated(bool enabled)
{
    d_func()->layout->animationEnabled = enabled;
}

/*! \property QMainWindow::dockNestingEnabled
    \brief whether docks can be nested
    \since 4.2

    If this property is set to false, dock areas can only contain a single row
    (horizontal or vertical) of dock widgets. If this property is set to true,
    the area occupied by a dock widget can be split in either direction to contain
    more dock widgets.

    Dock nesting is only necessary in applications that contain a lot of
    dock widgets. It gives the user greater freedom in organizing their main window.
    However, dock nesting leads to more complex (and less intuitive) behavior when
    a dock widget is dragged over the main window, since there are more ways in which
    a dropped dock widget may be placed in the dock area.

    This property should be set before any dock widgets are added to the main window.
*/

bool QMainWindow::isDockNestingEnabled() const
{
    return d_func()->layout->dockNestingEnabled;
}

void QMainWindow::setDockNestingEnabled(bool enabled)
{
    d_func()->layout->dockNestingEnabled = enabled;
}

/*! \property QMainWindow::verticalTabsEnabled
    \brief whether left and right dock areas use vertical tabs
    \since 4.2

    If this property is set to false, dock areas containing tabbed dock widgets
    display horizontal tabs, simmilar to Visual Studio.

    If this property is set to true, then the right and left dock areas display vertical
    tabs, simmilar to KDevelop.

    This property should be set before any dock widgets are added to the main window.
*/

bool QMainWindow::verticalTabsEnabled() const
{
    return d_func()->layout->verticalTabsEnabled();
}

void QMainWindow::setVerticalTabsEnabled(bool enabled)
{
    d_func()->layout->setVerticalTabsEnabled(enabled);
}

static bool checkDockWidgetArea(Qt::DockWidgetArea area, const char *where)
{
    switch (area) {
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea:
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        return true;
    default:
        break;
    }
    qWarning("%s: invalid 'area' argument", where);
    return false;
}

/*!
    Adds the given \a dockwidget to the specified \a area.
*/
void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
    if (!checkDockWidgetArea(area, "QMainWindow::addDockWidget"))
        return;

    if (!dockwidget->isAreaAllowed(area))
        qWarning("QMainWindow::addDockWidget(): specified 'area' is not an allowed for this widget");

    Qt::Orientation orientation = Qt::Vertical;
    switch (area) {
    case Qt::TopDockWidgetArea:
    case Qt::BottomDockWidgetArea:
        orientation = Qt::Horizontal;
        break;
    default:
        break;
    }
    d_func()->layout->removeWidget(dockwidget); // in case it was already in here
    addDockWidget(area, dockwidget, orientation);

#ifdef Q_WS_MAC     //drawer support
    extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
    if (qt_mac_is_macdrawer(dockwidget)) {
        extern bool qt_mac_set_drawer_preferred_edge(QWidget *, Qt::DockWidgetArea); //qwidget_mac.cpp
        window()->createWinId();
        dockwidget->window()->createWinId();
        qt_mac_set_drawer_preferred_edge(dockwidget, area);
        if (dockwidget->isVisible()) {
            dockwidget->hide();
            dockwidget->show();
        }
    }
#endif
}

/*!
    Adds \a dockwidget into the given \a area in the direction
    specified by the \a orientation.
*/
void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget,
                                Qt::Orientation orientation)
{
    if (!checkDockWidgetArea(area, "QMainWindow::addDockWidget"))
        return;

    if (!dockwidget->isAreaAllowed(area))
        qWarning("QMainWindow::addDockWidget(): specified 'area' is not an allowed for this widget");

    // add a window to an area, placing done relative to the previous
    d_func()->layout->addDockWidget(area, dockwidget, orientation);
    if (isVisible())
        d_func()->layout->relayout();
}

/*!
    \fn void QMainWindow::splitDockWidget(QDockWidget *first, QDockWidget *second, Qt::Orientation orientation)

    Splits the space covered by the \a first dock widget into two parts,
    moves the \a first dock widget into the first part, and moves the
    \a second dock widget into the second part.

    The \a orientation specifies how the space is divided: A Qt::Horizontal
    split places the second dock widget to the right of the first; a
    Qt::Vertical split places the second dock widget below the first.

    \e Note: if \a first is currently in a tabbed docked area, \a second will
    be added as a new tab, not as a neighbor of \a first. This is because a
    single tab can contain only one dock widget.

    \e Note: The Qt::LayoutDirection influences the order of the dock widgets
    in the two parts of the divided area. When right-to-left layout direction
    is enabled, the placing of the dock widgets will be reversed.

    \sa tabifyDockWidget(), addDockWidget(), removeDockWidget()
*/
void QMainWindow::splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
                                  Qt::Orientation orientation)
{
    if (!dockwidget->isAreaAllowed(dockWidgetArea(after)))
        qWarning("QMainWindow::splitDockWidget(): specified 'area' is not an allowed for this widget");
    d_func()->layout->splitDockWidget(after, dockwidget, orientation);
    if (isVisible())
        d_func()->layout->relayout();
}

/*!
    \fn void QMainWindow::tabifyDockWidget(QDockWidget *first, QDockWidget *second)

    Moves \a second dock widget on top of \a first dock widget, creating a tabbed
    docked area in the main window.
*/
void QMainWindow::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
    if (!second->isAreaAllowed(dockWidgetArea(first)))
        qWarning("QMainWindow::splitDockWidget(): specified 'area' is not an allowed for this widget");
    d_func()->layout->tabifyDockWidget(first, second);
    if (isVisible())
        d_func()->layout->relayout();
}

/*!
    Removes the \a dockwidget from the main window layout and hides
    it. Note that the \a dockwidget is \e not deleted.
*/
void QMainWindow::removeDockWidget(QDockWidget *dockwidget)
{
    if (dockwidget) {
        d_func()->layout->removeWidget(dockwidget);
        dockwidget->hide();
    }
}

/*!
    Returns the Qt::DockWidgetArea for \a dockwidget. If \a dockwidget
    has not been added to the main window, this function returns \c
    Qt::NoDockWidgetArea.

    \sa addDockWidget() splitDockWidget() Qt::DockWidgetArea
*/
Qt::DockWidgetArea QMainWindow::dockWidgetArea(QDockWidget *dockwidget) const
{ return d_func()->layout->dockWidgetArea(dockwidget); }

#endif // QT_NO_DOCKWIDGET

/*!
    Saves the current state of this mainwindow's toolbars and
    dockwidgets. The \a version number is stored as part of the data.

    The \link QObject::objectName objectName\endlink property is used
    to identify each QToolBar and QDockWidget.  You should make sure
    that this property is unique for each QToolBar and QDockWidget you
    add to the QMainWindow

    To restore the saved state, pass the return value and \a version
    number to restoreState().

    \sa restoreState()
*/
QByteArray QMainWindow::saveState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QMainWindowLayout::VersionMarker;
    stream << version;
    d_func()->layout->saveState(stream);
    return data;
}

/*!
    Restores the \a state of this mainwindow's toolbars and
    dockwidgets. The \a version number is compared with that stored
    in \a state. If they do not match, the mainwindow's state is left
    unchanged, and this function returns \c false; otherwise, the state
    is restored, and this function returns \c true.

    \sa saveState()
*/
bool QMainWindow::restoreState(const QByteArray &state, int version)
{
    if (state.isEmpty())
        return false;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    int marker, v;
    stream >> marker;
    stream >> v;
    if (stream.status() != QDataStream::Ok || marker != QMainWindowLayout::VersionMarker || v != version)
        return false;
    bool restored = d_func()->layout->restoreState(stream);
    layout()->setGeometry(rect());
    return restored;
}

#if !defined(QT_NO_DOCKWIDGET) && !defined(QT_NO_CURSOR)
QCursor QMainWindowPrivate::separatorCursor(const QList<int> &path) const
{
    QDockAreaLayoutInfo *info = layout->dockAreaLayout.info(path);
    Q_ASSERT(info != 0);
    if (path.size() == 1) {
        return info->o == Qt::Horizontal
                ? Qt::SplitVCursor : Qt::SplitHCursor;
    }

    return info->o == Qt::Horizontal
            ? Qt::SplitHCursor : Qt::SplitVCursor;
}

void QMainWindowPrivate::adjustCursor(const QPoint &pos)
{
    Q_Q(QMainWindow);

    hoverPos = pos;

    if (pos == QPoint(0, 0)) {
        if (!hoverSeparator.isEmpty())
            q->update(layout->dockAreaLayout.separatorRect(hoverSeparator));
        hoverSeparator.clear();
        q->unsetCursor();
    } else {
        QList<int> pathToSeparator
            = layout->dockAreaLayout.findSeparator(pos);

        if (pathToSeparator != hoverSeparator) {
            if (!hoverSeparator.isEmpty())
                q->update(layout->dockAreaLayout.separatorRect(hoverSeparator));

            hoverSeparator = pathToSeparator;

            if (hoverSeparator.isEmpty()) {
                q->unsetCursor();
            } else {
                q->update(layout->dockAreaLayout.separatorRect(hoverSeparator));
                QCursor cursor = separatorCursor(hoverSeparator);
                q->setCursor(cursor);
            }
        }
    }
}
#endif

/*! \reimp */
bool QMainWindow::event(QEvent *event)
{
    Q_D(QMainWindow);
    switch (event->type()) {

#ifndef QT_NO_DOCKWIDGET
        case QEvent::Paint: {
            QPainter p(this);
            QRegion r = static_cast<QPaintEvent*>(event)->region();
            d->layout->dockAreaLayout.paintSeparators(&p, this, r, d->hoverPos);
            break;
        }

#ifndef QT_NO_CURSOR
        case QEvent::HoverMove:  {
            d->adjustCursor(static_cast<QHoverEvent*>(event)->pos());
            break;
        }

        case QEvent::HoverLeave:
            d->adjustCursor(QPoint(0, 0));
            break;
#endif // QT_NO_CURSOR

        case QEvent::MouseButtonPress: {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (e->button() == Qt::LeftButton && d->layout->startSeparatorMove(e->pos())) {
                // The click was on a separator, eat this event
                e->accept();
                return true;
            }
            break;
        }

        case QEvent::MouseMove: {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);

#ifndef QT_NO_CURSOR
            d->adjustCursor(e->pos());
#endif
            if (e->buttons() & Qt::LeftButton) {
                if (d->layout->separatorMove(e->pos())) {
                    // We're moving a separator, eat this event
                    e->accept();
                    return true;
                }
            }

            break;
        }

        case QEvent::MouseButtonRelease: {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            if (d->layout->endSeparatorMove(e->pos())) {
                // We've released a separator, eat this event
                e->accept();
                return true;
            }
            break;
        }

#endif

#ifndef QT_NO_TOOLBAR
        case QEvent::ToolBarChange: {
            QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
            QSize minimumSize = d->layout->minimumSize();
            for (int i = 0; i < toolbars.size(); ++i) {
                QToolBar *toolbar = toolbars.at(i);
                toolbar->setVisible(!toolbar->isVisible());
            }
            QApplication::sendPostedEvents(this, QEvent::LayoutRequest);
            QSize newMinimumSize = d->layout->minimumSize();
            QSize delta = newMinimumSize - minimumSize;
            resize(size() + delta);
            return true;
        }
#endif

#ifndef QT_NO_STATUSTIP
        case QEvent::StatusTip:
#ifndef QT_NO_STATUSBAR
            if (QStatusBar *sb = d->layout->statusBar())
                sb->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
            else
#endif
                static_cast<QStatusTipEvent*>(event)->ignore();
            return true;
#endif // QT_NO_STATUSTIP

        case QEvent::StyleChange:
            if (!d->explicitIconSize)
                setIconSize(QSize());
            break;

        default:
            break;
    }

    return QWidget::event(event);
}

/*!
    \internal
*/
bool QMainWindow::isSeparator(const QPoint &pos) const
{
    Q_D(const QMainWindow);
#ifndef QT_NO_DOCKWIDGET
    return !d->layout->dockAreaLayout.findSeparator(pos).isEmpty();
#else
    return false;
#endif
}

/*!
    \reimp
*/
void QMainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    // only show the context menu for direct QDockWidget and QToolBar
    // children and for the menubar as well
    QWidget *child = childAt(event->pos());
    while (child && child != this) {
        if (QMenuBar *mb = qobject_cast<QMenuBar *>(child)) {
            if (mb->parentWidget() != this)
                return;
            break;
        }

#ifndef QT_NO_DOCKWIDGET
        if (QDockWidget *dw = qobject_cast<QDockWidget *>(child)) {
            if (dw->parentWidget() != this)
                return;
            if (dw->widget()
                && dw->widget()->geometry().contains(child->mapFrom(this, event->pos()))) {
                // ignore the event if the mouse is over the QDockWidget contents
                return;
            }
            break;
        }
#endif // QT_NO_DOCKWIDGET
#ifndef QT_NO_TOOLBAR
        if (QToolBar *tb = qobject_cast<QToolBar *>(child)) {
            if (tb->parentWidget() != this)
                return;
            break;
        }
#endif
        child = child->parentWidget();
    }
    if (child == this)
        return;

#ifndef QT_NO_MENU
    QMenu *popup = createPopupMenu();
    if (!popup)
	return;
    popup->exec(event->globalPos());
    delete popup;
    event->accept();
#endif
}

#ifndef QT_NO_MENU
/*!
    Returns a popup menu containing checkable entries for the toolbars and
    dock widgets present in the main window.

    By default, this function is called by the main window when the user
    activates a context menu, typically by right-clicking on a toolbar or a dock
    widget.

    If you want to create a custom popup menu, reimplement this function and
    return a newly-created popup menu. Ownership of the popup menu is transferred
    to the caller.

    \sa addDockWidget(), addToolBar(), menuBar()
*/
QMenu *QMainWindow::createPopupMenu()
{
    Q_D(QMainWindow);
    QMenu *menu = 0;
#ifndef QT_NO_DOCKWIDGET
    QList<QDockWidget *> dockwidgets = qFindChildren<QDockWidget *>(this);
    if (dockwidgets.size()) {
        menu = new QMenu(this);
        for (int i = 0; i < dockwidgets.size(); ++i) {
            QDockWidget *dockWidget = dockwidgets.at(i);
            if (dockWidget->parentWidget() == this
                && d->layout->contains(dockWidget)) {
                menu->addAction(dockwidgets.at(i)->toggleViewAction());
            }
        }
        menu->addSeparator();
    }
#endif // QT_NO_DOCKWIDGET
#ifndef QT_NO_TOOLBAR
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
    if (toolbars.size()) {
        if (!menu)
            menu = new QMenu(this);
        for (int i = 0; i < toolbars.size(); ++i) {
            QToolBar *toolBar = toolbars.at(i);
            if (toolBar->parentWidget() == this
                && d->layout->contains(toolBar)) {
                menu->addAction(toolbars.at(i)->toggleViewAction());
            }
        }
    }
#endif
    Q_UNUSED(d);
    return menu;
}
#endif // QT_NO_MENU

#endif // QT_NO_MAINWINDOW
