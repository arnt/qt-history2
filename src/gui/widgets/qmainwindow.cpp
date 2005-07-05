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

#include <private/qwidget_p.h>
#include "qtoolbar_p.h"

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
};

void QMainWindowPrivate::init()
{
    Q_Q(QMainWindow);
    layout = new QMainWindowLayout(q);
    const int metric = q->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    iconSize = QSize(metric, metric);
    explicitIconSize = false;
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
    \brief The QMainWindow class provides a main application window.

    \ingroup application
    \mainclass

    QMainWindow provides a main application window, with a menu bar,
    tool bars, dock widgets and a status bar around a large central
    widget, such as a text edit, drawing canvas or Q3Workspace (for MDI
    applications).

    Topics:

    \tableofcontents

    \section1 Saving and restoring state

    The saveState() and restoreState() functions provide a means to
    save and restore the layout of the QToolBars and QDockWidgets in
    the QMainWindow.  These functions work by storing the \link
    QObject::objectName objectName\endlink of each QToolBar and
    QDockWidget together with information about placement, size, etc.

    \section1 Behavior of Dock Widgets

    \target dock-widget-separators
    \section2 Dock Widget Separators

    QMainWindow uses separators to separate QDockWidgets from each
    other and the \l{centralWidget()}{central widget}. These
    separators let the user control the size of QDockWidgets by
    dragging the boundary between them.

    A QDockWidget can be as large or as small as the user wishes,
    between the minimumSizeHint() (or minimumSize()) and
    maximumSize() of the QDockWidget. When a QDockWidget reaches its
    minimum size, space will be taken from other QDockWidgets in the
    direction of the user's drag, if possible. Once all QDockWidgets
    have reached their minimum sizes, further dragging does nothing.
    When a QDockWidget reaches its maximium size, space will be given
    to other QDockWidgets in the opposite direction of the user's
    drag, if possible. Once all QDockWidgets have reached their
    minimum size, futher dragging does nothing.

    \target dragging-dock-widgets
    \section2 Dragging Dock Widgets

    QDockWidget displays a title bar to let the user drag the dock
    widget to a new location. A QDockWidget can be moved to any
    location provided enough space is available. (QMainWindow won't
    resize itself to a larger size in an attempt to provide more
    space.)

    A QRubberBand is shown while dragging the QDockWidget. This
    QRubberBand provides an indication to the user about where the
    QDockWidget will be placed when the mouse button is released.

    \section2 Dragging over Neighbors

    All un-nested QDockWidgets in the same dock area are considered
    neighbors. When dragging a QDockWidget over its neighbor:

    \list

    \o QMainWindow will split the neighbor perpendicularly to the
    direction of the QDockWidgets.

    \o QMainWindow will swap the position of the QDockWidget being
    dragged and its neighbor once the user has dragged the mouse past
    the center point of the neighboring QDockWidget.

    \endlist

    The following diagram depicts this behavior:

    \image dockwidget-neighbors.png Diagram

    \section2 Dragging over other QDockWidgets

    When dragging nested QDockWidgets, or when dragging to a
    different dock area, QMainWindow will split the QDockWindow under
    the mouse. Be aware that the QDockWidget under the mouse will
    only be split by the QDockWidget being dragged if both can fit in
    the space currently occupied by the QDockWidget under the mouse.

    A QDockWidget can be split horizontally or vertically, with the
    QDockWidget being dragged being placed in one of four possible
    locations, as shown in the diagram below:

    \image dockwidget-cross.png Diagram

    \omit ### When dragging a nested QDockWidget \endomit

    \section2 Dragging to a Different Qt::DockWidgetArea

    The QDockWidget::floatable property influences feedback when the
    user drags a QDockWidget over the central widget:

    \list

    \o If \l{QDockWidget::floating}{floating} is \c true,
    QMainWindow chooses a dock area based on the position of the mouse
    pointer. If the mouse is within 50 pixels of the
    central widget's edge, the adjacent dock area is chosen.
    When dragging into the corners of these 50 pixel regions, the
    current corner() configuration is used to make the decision.
    Otherwise, the QRubberBand is shown under the mouse pointer, as
    above.

    \o If \l{QDockWidget::floating}{floating} is \c false,
    QMainWindow chooses a dock area based on the distance between the
    mouse pointer and the center of the central widget. If the mouse
    comes within 50 pixels of the central widget's edge, the adjacent
    dock area is always chosen. When dragging into the corners of
    these 50 pixel regions, the current corner() configuration is
    used to make the decision.

    \endlist

    In either case, dragging the mouse over another QDockWidget causes
    QMainWindow to choose the other QDockWidget's dock area.

    When dragging outside the QMainWindow, the QDockWidget::floating
    property again controls feedback during dragging. When the
    property is \c false, dragging outside of the QMainWindow will show
    the rubberband over the QDockWidget's current location. This
    indicates that the QDockWidget cannot be moved outside of the
    QMainWindow. When the QDockWidget::floatable property is \c true,
    dragging outside of the QMainWindow will show the QRubberBand
    under the mouse pointer. This indicates that the QDockWidget will
    be floating when the mouse button is released.

    \sa QMenuBar, QToolBar, QStatusBar, QDockWidget
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

    This signal is emiited when the style used for tool buttons in the
    window is changed. The new style is passed in \a toolButtonStyle.

    You can connect this signal to other components to help maintain
    a consistent appearance for your application.

    \sa setToolButtonStyle()
*/

/*!
    Constructs a QMainWindow with the given \a parent and the specified
    widget \a flags.
 */
QMainWindow::QMainWindow(QWidget *parent, Qt::WFlags flags)
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
QMainWindow::QMainWindow(QWidget *parent, const char *name, Qt::WFlags flags)
    : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::WType_TopLevel)
{
    setObjectName(name);
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
    if (d->layout->menuBar())
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
	self->setStatusBar(statusbar);
    }
    return statusbar;
}

/*!
    Sets the status bar for the main window to \a statusbar.

    Note: QMainWindow takes ownership of the \a statusbar pointer and
    deletes it at the appropriate time.

    \sa statusBar()
*/
void QMainWindow::setStatusBar(QStatusBar *statusbar)
{
    Q_D(QMainWindow);
    if (d->layout->statusBar())
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

    \warning This function should be called at most once for each main
    window instance

    Note: QMainWindow takes ownership of the \a widget pointer and
    deletes it at the appropriate time.

    \sa centralWidget()
*/
void QMainWindow::setCentralWidget(QWidget *widget)
{ d_func()->layout->setCentralWidget(widget); }

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
    Q_ASSERT_X(valid, "QMainWindow::setCorner", "'area' is not valid for 'corner'");
    if (valid)
        d_func()->layout->corners[corner] = area;
}

/*!
    Returns the dock widget area that occupies the specified \a
    corner.

    \sa setCorner()
*/
Qt::DockWidgetArea QMainWindow::corner(Qt::Corner corner) const
{ return d_func()->layout->corners[corner]; }

#ifndef QT_NO_TOOLBAR
/*!
    Adds a toolbar break to the given \a area after all the other
    objects that are present.
*/
void QMainWindow::addToolBarBreak(Qt::ToolBarArea area)
{ d_func()->layout->addToolBarBreak(area); }

/*!
    Inserts a toolbar break before the toolbar specified by \a before.
*/
void QMainWindow::insertToolBarBreak(QToolBar *before)
{ d_func()->layout->insertToolBarBreak(before); }

/*!
    Adds the \a toolbar into the specified \a area in this main
    window. The \a toolbar is placed at the end of the current tool
    bar block (i.e. line).

    \sa insertToolBar() addToolBarBreak() insertToolBarBreak()
*/
void QMainWindow::addToolBar(Qt::ToolBarArea area, QToolBar *toolbar)
{
    Q_D(QMainWindow);
    Q_ASSERT_X(toolbar->isAreaAllowed(area),
               "QMainWIndow::addToolBar", "specified 'area' is not an allowed area");

    toolbar->d_func()->updateIconSize(d->iconSize);
    toolbar->d_func()->updateToolButtonStyle(d->toolButtonStyle);
    connect(this, SIGNAL(iconSizeChanged(QSize)),
            toolbar, SLOT(updateIconSize(QSize)));
    connect(this, SIGNAL(toolButtonStyleChanged(ToolButtonStyle)),
            toolbar, SLOT(updateToolButtonStyle(ToolButtonStyle)));

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

    toolbar->d_func()->updateIconSize(d->iconSize);
    toolbar->d_func()->updateToolButtonStyle(d->toolButtonStyle);
    connect(this, SIGNAL(iconSizeChanged(QSize)),
            toolbar, SLOT(updateIconSize(QSize)));
    connect(this, SIGNAL(toolButtonStyleChanged(ToolButtonStyle)),
            toolbar, SLOT(updateToolButtonStyle(ToolButtonStyle)));

    d->layout->insertToolBar(before, toolbar);

    if (isVisible())
        d->layout->relayout();
}

/*!
    Removes the \a toolbar from the main window.
*/
void QMainWindow::removeToolBar(QToolBar *toolbar)
{
    disconnect(this, SIGNAL(iconSizeChanged(QSize)),
               toolbar, SLOT(updateIconSize(QSize)));
    disconnect(this, SIGNAL(toolButtonStyleChanged(ToolButtonStyle)),
               toolbar, SLOT(updateToolButtonStyle(ToolButtonStyle)));

    d_func()->layout->removeWidget(toolbar);
}

/*!
    Returns the tool bar area for \a toolbar.

    \sa addToolBar() addToolBarBreak() Qt::ToolBarArea
*/
Qt::ToolBarArea QMainWindow::toolBarArea(QToolBar *toolbar) const
{ return d_func()->layout->toolBarArea(toolbar); }

#endif // QT_NO_TOOLBAR

/*!
    Adds the given \a dockwidget to the specified \a area.
*/
void QMainWindow::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
    Q_ASSERT_X(dockwidget->isAreaAllowed(area),
               "QMainWindow::addDockWidget", "specified 'area' is not an allowed area");
    Qt::Orientation orientation = Qt::Horizontal;
    switch (area) {
    case Qt::LeftDockWidgetArea:
    case Qt::RightDockWidgetArea:
        orientation = Qt::Vertical;
        break;
    default:
        break;
    }
    addDockWidget(area, dockwidget, orientation);

#ifdef Q_WS_MAC     //drawer support
    extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
    if (qt_mac_is_macdrawer(dockwidget)) {
        extern bool qt_mac_set_drawer_preferred_edge(QWidget *, Qt::DockWidgetArea); //qwidget_mac.cpp
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
    // add a window to an area, placing done relative to the previous
    Q_ASSERT_X(dockwidget->isAreaAllowed(area),
               "QMainWindow::addDockWidget", "specified 'area' is not an allowed area");
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

    \e Note: The Qt::LayoutDirection influences the order of the dock widgets
    in the two parts of the divided area. When right-to-left layout direction
    is enabled, the placing of the dock widgets will be reversed.
*/
void QMainWindow::splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
                                  Qt::Orientation orientation)
{
    Qt::DockWidgetArea area = dockWidgetArea(after);
    Q_UNUSED(area);
    Q_ASSERT_X(dockwidget->isAreaAllowed(area),
               "QMainWindow::splitDockWidget", "specified 'area' is not an allowed area");
    d_func()->layout->splitDockWidget(after, dockwidget, orientation);
    if (isVisible())
        d_func()->layout->relayout();
}

/*!
    Removes the \a dockwidget from the main window.
*/
void QMainWindow::removeDockWidget(QDockWidget *dockwidget)
{ d_func()->layout->removeRecursive(dockwidget); }

/*!
    Returns the \c Qt::DockWidgetArea for \a dockwidget.

    \sa addDockWidget() splitDockWidget() Qt::DockWidgetArea
*/
Qt::DockWidgetArea QMainWindow::dockWidgetArea(QDockWidget *dockwidget) const
{ return d_func()->layout->dockWidgetArea(dockwidget); }

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
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    int marker, v;
    stream >> marker;
    stream >> v;
    if (marker != QMainWindowLayout::VersionMarker || v != version)
        return false;
    bool restored = d_func()->layout->restoreState(stream);
    if (isVisible())
        QApplication::postEvent(this, new QResizeEvent(size(), size()));
    return restored;
}

/*! \reimp */
bool QMainWindow::event(QEvent *event)
{
    Q_D(QMainWindow);
#ifndef QT_NO_TOOLBAR
    if (event->type() == QEvent::ToolBarChange) {
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
    } else
#endif
    if (event->type() == QEvent::StatusTip) {
#ifndef QT_NO_STATUSBAR
        if (QStatusBar *sb = d->layout->statusBar())
            sb->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
        else
#endif
            static_cast<QStatusTipEvent*>(event)->ignore();
        return true;
    } else if (event->type() == QEvent::StyleChange) {
        if (!d->explicitIconSize)
            setIconSize(QSize());
    }
    return QWidget::event(event);
}

/*!
    \reimp
*/
void QMainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    // only show the context menu for direct QDockWidget and QToolBar
    // children
    QWidget *child = childAt(event->pos());
    while (child && child != this) {
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
    This function is called to create a popup menu when the user
    right-clicks on the menu bar, a toolbar or a dock widget. If you
    want to create a custom popup menu, reimplement this function and
    return the created popup menu. Ownership of the popup menu is
    transferred to the caller.
*/
QMenu *QMainWindow::createPopupMenu()
{
    QMenu *menu = 0;
    QList<QDockWidget *> dockwidgets = qFindChildren<QDockWidget *>(this);
    if (dockwidgets.size()) {
        menu = new QMenu(this);
        for (int i = 0; i < dockwidgets.size(); ++i)
            if (dockwidgets.at(i)->parentWidget() == this)
                menu->addAction(dockwidgets.at(i)->toggleViewAction());
        menu->addSeparator();
    }
#ifndef QT_NO_TOOLBAR
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
    if (toolbars.size()) {
        if (!menu)
            menu = new QMenu(this);
        for (int i = 0; i < toolbars.size(); ++i)
            if (toolbars.at(i)->parentWidget() == this)
                menu->addAction(toolbars.at(i)->toggleViewAction());
    }
#endif
    return menu;
}
#endif // QT_NO_MENU

#endif // QT_NO_MAINWINDOW
