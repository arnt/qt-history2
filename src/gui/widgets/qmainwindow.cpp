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

#include "qdockwindow.h"
#include "qtoolbar.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qevent.h>

#include <private/qwidget_p.h>
#define d d_func()


class QMainWindowPrivate : public QWidgetPrivate
{
public:
    inline QMainWindowPrivate()
        : layout(0), iconSize(Qt::SmallIconSize), toolButtonStyle(Qt::ToolButtonIconOnly)
    { }
    QMainWindowLayout *layout;
    Qt::IconSize iconSize;
    Qt::ToolButtonStyle toolButtonStyle;
};


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
    \class QMainWindow qmainwindow.h
    \brief The QMainWindow class provides a main application window.

    \ingroup application
    \mainclass

    QMainWindow provides a main application window, with a menu bar,
    tool bars, dock windows and a status bar around a large central
    widget, such as a text edit, drawing canvas or QWorkspace (for MDI
    applications).

    \sa QDockWindow, QToolBar
*/

/*!
    \fn void QMainWindow::iconSizeChanged(Qt::IconSize iconSize)

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
    : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::WType_TopLevel)
{
    d->layout = new QMainWindowLayout(this);
}

#ifdef QT_COMPAT
/*!
    \obsolete
    Constructs a QMainWindow with the given \a parent, \a name, and
    with the specified widget \a flags.
 */
QMainWindow::QMainWindow(QWidget *parent, const char *name, Qt::WFlags flags)
    : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::WType_TopLevel)
{
    setObjectName(name);
    d->layout = new QMainWindowLayout(this);
}
#endif

/*!
    Destroys the main window.
 */
QMainWindow::~QMainWindow()
{ }

/*! \property QMainWindow::iconSize
    \brief size of toolbar icons in this mainwindow.

    The default is Qt::SmallIconSize.
*/

Qt::IconSize QMainWindow::iconSize() const
{ return d->iconSize; }

void QMainWindow::setIconSize(Qt::IconSize iconSize)
{
    if (d->iconSize == iconSize)
        return;
    d->iconSize = iconSize;
    emit iconSizeChanged(d->iconSize);
}

/*! \property QMainWindow::toolButtonStyle
    \brief style of toolbar buttons in this mainwindow.

    The default is Qt::ToolButtonIconOnly.
*/

Qt::ToolButtonStyle QMainWindow::toolButtonStyle() const
{ return d->toolButtonStyle; }

void QMainWindow::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
    if (d->toolButtonStyle == toolButtonStyle)
        return;
    d->toolButtonStyle = toolButtonStyle;
    emit toolButtonStyleChanged(d->toolButtonStyle);
}

/*!
    Returns the menu bar for the main window. This function creates
    and returns an empty menu bar if the menu bar does not exist.

    \sa setMenuBar()
*/
QMenuBar *QMainWindow::menuBar() const
{
    QMenuBar *menubar = qt_cast<QMenuBar*>(d->layout->menuBar());
    if (!menubar) {
	QMainWindow *self = const_cast<QMainWindow *>(this);
	menubar = new QMenuBar(self);
	self->setMenuBar(menubar);
    }
    return menubar;
}

/*!
    Sets the menu bar for the main window to \a menubar.

    \sa menuBar()
*/
void QMainWindow::setMenuBar(QMenuBar *menubar)
{
    Q_ASSERT_X(menubar != 0,
	       "QMainWindow::setMenuBar()", "parameter cannot be zero");
    Q_ASSERT_X(!d->layout->menuBar(),
	       "QMainWindow::setMenuBar()", "menu bar already set");
    layout()->setMenuBar(menubar);
}

/*!
    Returns the status bar for the main window. This function creates
    and returns an empty status bar if the status bar does not exist.

    \sa setStatusBar()
*/
QStatusBar *QMainWindow::statusBar() const
{
    QStatusBar *statusbar = d->layout->statusBar();
    if (!statusbar) {
	QMainWindow *self = const_cast<QMainWindow *>(this);
	statusbar = new QStatusBar(self);
	self->setStatusBar(statusbar);
    }
    return statusbar;
}

/*!
    Sets the status bar for the main window to \a statusbar.

    \sa statusBar()
*/
void QMainWindow::setStatusBar(QStatusBar *statusbar)
{
    Q_ASSERT_X(statusbar != 0,
	       "QMainWindow::setStatusBar()", "parameter cannot be zero");
    Q_ASSERT_X(!d->layout->statusBar(),
	       "QMainWindow::setStatusBar()", "status bar already set");
    d->layout->setStatusBar(statusbar);
}

/*!
    Returns the central widget for the main window.  This function
    returns zero if the central widget has not been set.

    \sa setCentralWidget()
*/
QWidget *QMainWindow::centralWidget() const
{ return d->layout->centralWidget(); }

/*!
    Sets the given \a widget to be the main window's central widget.

    \warning This function should be called at most once for each main
    window instance

    \sa centralWidget()
*/
void QMainWindow::setCentralWidget(QWidget *widget)
{ d->layout->setCentralWidget(widget); }

/*!
    Sets the given dock window \a area to occupy the specified \a
    corner.

    \sa corner()
*/
void QMainWindow::setCorner(Qt::Corner corner, Qt::DockWindowArea area)
{
    bool valid = false;
    switch (corner) {
    case Qt::TopLeftCorner:
        valid = (area == Qt::TopDockWindowArea || area == Qt::LeftDockWindowArea);
        break;
    case Qt::TopRightCorner:
        valid = (area == Qt::TopDockWindowArea || area == Qt::RightDockWindowArea);
        break;
    case Qt::BottomLeftCorner:
        valid = (area == Qt::BottomDockWindowArea || area == Qt::LeftDockWindowArea);
        break;
    case Qt::BottomRightCorner:
        valid = (area == Qt::BottomDockWindowArea || area == Qt::RightDockWindowArea);
        break;
    }
    Q_ASSERT_X(valid, "QMainWindow::setCorner", "'area' is not valid for 'corner'");
    d->layout->corners[corner] = area;
}

/*!
    Returns the dock window area that occupies the specified \a
    corner.

    \sa setCorner()
*/
Qt::DockWindowArea QMainWindow::corner(Qt::Corner corner) const
{ return d->layout->corners[corner]; }

/*!
    Adds a toolbar break to the given \a area after all the other
    objects that are present.
*/
void QMainWindow::addToolBarBreak(Qt::ToolBarArea area)
{ d->layout->addToolBarBreak(area); }

/*!
    Inserts a toolbar break before the toolbar specified by \a before.
*/
void QMainWindow::insertToolBarBreak(QToolBar *before)
{ d->layout->insertToolBarBreak(before); }

/*!
    Adds the \a toolbar into the specified \a area in this main
    window.  The \a toolbar is placed at the end of the current tool
    bar block (i.e. line).

    \sa insertToolBar() addToolBarBlock() insertToolBarBlock()
*/
void QMainWindow::addToolBar(Qt::ToolBarArea area, QToolBar *toolbar)
{
    Q_ASSERT_X(toolbar->isDockable(area),
               "QMainWIndow::addToolBar", "specified 'area' is not an allowed area");

    connect(this, SIGNAL(iconSizeChanged(Qt::IconSize)),
            toolbar, SLOT(setIconSize(Qt::IconSize)));
    connect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
            toolbar, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

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

    \sa insertToolBarBlock() addToolBar() addToolBarBlock()
*/
void QMainWindow::insertToolBar(QToolBar *before, QToolBar *toolbar)
{
    Q_ASSERT_X(toolbar->isDockable(toolBarArea(before)),
               "QMainWIndow::insertToolBar", "specified 'area' is not an allowed area");

    connect(this, SIGNAL(iconSizeChanged(Qt::IconSize)),
            toolbar, SLOT(setIconSize(Qt::IconSize)));
    connect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
            toolbar, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

    d->layout->insertToolBar(before, toolbar);

    if (isVisible())
        d->layout->relayout();
}

/*!
    Removes the \a toolbar from the main window.
*/
void QMainWindow::removeToolBar(QToolBar *toolbar)
{
    disconnect(this, SIGNAL(iconSizeChanged(Qt::IconSize)),
               toolbar, SLOT(setIconSize(Qt::IconSize)));
    disconnect(this, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
               toolbar, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

    d->layout->removeWidget(toolbar);
}

/*!
    Returns the tool bar area for \a toolbar.

    \sa addToolBar() addToolBarBlock() Qt::ToolBarArea
*/
Qt::ToolBarArea QMainWindow::toolBarArea(QToolBar *toolbar) const
{ return d->layout->toolBarArea(toolbar); }

/*!
    Adds the given \a dockwindow to the specified \a area.
*/
void QMainWindow::addDockWindow(Qt::DockWindowArea area, QDockWindow *dockwindow)
{
    Q_ASSERT_X(dockwindow->isDockable(area),
               "QMainWindow::addDockWindow", "specified 'area' is not an allowed area");
    Qt::Orientation orientation = Qt::Horizontal;
    switch (area) {
    case Qt::LeftDockWindowArea:
    case Qt::RightDockWindowArea:
        orientation = Qt::Vertical;
        break;
    default:
        break;
    }
    extendDockWindowArea(area, dockwindow, orientation);

#ifdef Q_WS_MAC     //drawer support
    extern bool qt_mac_is_macdrawer(const QWidget *); //qwidget_mac.cpp
    if (qt_mac_is_macdrawer(dockwindow)) {
        extern bool qt_mac_set_drawer_preferred_edge(QWidget *, Qt::DockWindowArea); //qwidget_mac.cpp
        qt_mac_set_drawer_preferred_edge(dockwindow, area);
        if (dockwindow->isVisible()) {
            dockwindow->hide();
            dockwindow->show();
        }
    }
#endif
}

/*!
    Extend \a dockwindow into the given \a area in the direction specified
    by the \a orientation.
*/
void QMainWindow::extendDockWindowArea(Qt::DockWindowArea area, QDockWindow *dockwindow,
                                       Qt::Orientation orientation)
{
    // add a window to an area, placing done relative to the previous
    Q_ASSERT_X(dockwindow->isDockable(area),
               "QMainWindow::extendDockWindowArea", "specified 'area' is not an allowed area");
    d->layout->extendDockWindowArea(area, dockwindow, orientation);
    if (isVisible())
        d->layout->relayout();
}

/*!
    \fn void QMainWindow::splitDockWindow(QDockWindow *first, QDockWindow *second, Qt::Orientation orientation)

    Splits the space covered by the \a first dock window into two parts,
    moves the \a first dock window into the first part, and moves the
    \a second dock window into the second part.

    The \a orientation specifies how the space is divided: A Qt::Horizontal
    split places the second dock window to the right of the first; a
    Qt::Vertical split places the second dock window below the first.

    \e Note: The Qt::LayoutDirection influences the order of the dock windows
    in the two parts of the divided area. When right-to-left layout direction
    is enabled, the placing of the dock windows will be reversed.
*/
void QMainWindow::splitDockWindow(QDockWindow *after, QDockWindow *dockwindow,
                                  Qt::Orientation orientation)
{
    Qt::DockWindowArea area = dockWindowArea(after);
    Q_UNUSED(area);
    Q_ASSERT_X(dockwindow->isDockable(area),
               "QMainWindow::splitDockWindow", "specified 'area' is not an allowed area");
    d->layout->splitDockWindow(after, dockwindow, orientation);
    if (isVisible())
        d->layout->relayout();
}

/*!
    Removes the \a dockwindow from the main window.
*/
void QMainWindow::removeDockWindow(QDockWindow *dockwindow)
{ d->layout->removeRecursive(dockwindow); }

/*!
    Returns the \c Qt::DockWindowArea for \a dockwindow.

    \sa addDockWindow() extendDockWindowArea() splitDockWindow() Qt::DockWindowArea
*/
Qt::DockWindowArea QMainWindow::dockWindowArea(QDockWindow *dockwindow) const
{ return d->layout->dockWindowArea(dockwindow); }

/*!
    Saves the current state of this mainwindow's toolbars and
    dockwindows.  The \a version number is stored as part of the data.

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
    d->layout->saveState(stream);
    return data;
}

/*!
    Restores the \a state of this mainwindow's toolbars and
    dockwindows.  The \a version number is compared with that stored
    in \a state.  If they do not match, the mainwindow's state is left
    unchanged, and this function returns false; otherwise, the state
    is restored, and this function returns true.

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
    bool restored = d->layout->restoreState(stream);
    if (isVisible())
        d->layout->relayout();
    return restored;
}

/*! \reimp */
void QMainWindow::childEvent(QChildEvent *event)
{
    if (event->polished()) {
	QMenuBar *menubar;
	QStatusBar *statusbar;
	if ((menubar = qt_cast<QMenuBar *>(event->child()))) {
	    QMenuBar *mb = qt_cast<QMenuBar*>(d->layout->menuBar());
	    Q_ASSERT(mb == 0 || mb == menubar);
	    if (!mb)
                d->layout->setMenuBar(menubar);
	} else if ((statusbar = qt_cast<QStatusBar *>(event->child()))) {
	    QStatusBar *sb = d->layout->statusBar();
	    Q_ASSERT(sb == 0 || sb == statusbar);
	    if (!sb)
                d->layout->setStatusBar(statusbar);
	}
    }
}

/*! \reimp */
bool QMainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::ToolBarChange) {
        int deltaH = 0;
        int deltaW = 0;
        QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
        for (int i = 0; i < toolbars.size(); ++i) {
            QToolBar *toolbar = toolbars.at(i);
            Qt::ToolBarArea area = toolBarArea(toolbar);
            if(toolbar->isVisible()) {
                if (area == Qt::LeftToolBarArea || area == Qt::RightToolBarArea)
                    deltaW -= toolbar->width();
                else
                    deltaH -= toolbar->height();
                toolbar->hide();
            } else {
                if (area == Qt::LeftToolBarArea || area == Qt::RightToolBarArea)
                    deltaW += toolbar->width();
                else
                    deltaH += toolbar->height();
                toolbar->show();
            }
            if (deltaH || deltaW) {
                QApplication::sendPostedEvents(this, QEvent::LayoutRequest);
                resize(width() + deltaW, height() + deltaH);
            }
        }
        return true;
    } else if (event->type() == QEvent::StatusTip) {
        if (QStatusBar *sb = d->layout->statusBar())
            sb->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
        else
            static_cast<QStatusTipEvent*>(event)->ignore();
        return true;
    }
    return QWidget::event(event);
}

/*!
    \reimp
*/
void QMainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if (QWidget *w = childAt(event->pos()))
        if (!qt_cast<QToolBar*>(w) && !qt_cast<QDockWindow*>(w))
            return;
    QMenu *popup = createPopupMenu();
    if (!popup)
	return;
    popup->exec(event->globalPos());
    delete popup;
    event->accept();
}

/*!
    This function is called to create a popup menu when the user
    right-clicks a toolbar or dock window. If you want to create a
    custom popup menu, reimplement this function and return the
    created popup menu. Ownership of the popup menu is transferred to
    the caller.
*/
QMenu *QMainWindow::createPopupMenu()
{
    QMenu *menu = 0;
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
    QList<QDockWindow *> dockwindows = qFindChildren<QDockWindow *>(this);
    if (toolbars.size() || dockwindows.size()) {
        menu = new QMenu(this);
        if (!dockwindows.isEmpty()) {
        for (int i = 0; i < dockwindows.size(); ++i)
            menu->addAction(dockwindows.at(i)->toggleViewAction());
            menu->addSeparator();
        }
        if (!toolbars.isEmpty()) {
            for (int i = 0; i < toolbars.size(); ++i)
                menu->addAction(toolbars.at(i)->toggleViewAction());
        }
    }
    return menu;
}
