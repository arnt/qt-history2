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

#include "qdockwidget.h"
#include "qtoolbar.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qevent.h>
#include <qstyle.h>

#include <private/qwidget_p.h>
#define d d_func()


class QMainWindowPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMainWindow)
public:
    inline QMainWindowPrivate()
        : layout(0), toolButtonStyle(Qt::ToolButtonIconOnly)
    { }
    QMainWindowLayout *layout;
    QSize iconSize;
    Qt::ToolButtonStyle toolButtonStyle;
    void init();
};

void QMainWindowPrivate::init()
{
    Q_Q(QMainWindow);
    int e = q->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    iconSize = QSize(e, e);
    layout = new QMainWindowLayout(q);
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
    \class QMainWindow qmainwindow.h
    \brief The QMainWindow class provides a main application window.

    \ingroup application
    \mainclass

    QMainWindow provides a main application window, with a menu bar,
    tool bars, dock windows and a status bar around a large central
    widget, such as a text edit, drawing canvas or QWorkspace (for MDI
    applications).

    \tableofcontents

    \section1 Behavior of Dock Widgets

    \target dock-widget-separators
    \section2 Dock Widget Separators

    QMainWindow uses separators to separate QDockWidgets from each
    other and the centralWidget(). These separators let the user
    control the size of QDockWidgets by dragging the boundary between
    them.

    QDockWidgets can be as large or as small as the user wishes,
    between the \l minimumSizeHint() (or \l minimumSize()) and \l
    maximumSize() of each QDockWidget.  When a QDockWidget reaches its
    minimum size, space will be taken from other QDockWidgets in the
    direction of the user's drag, if possible.  Once all QDockWidgets
    have reached their minimum sizes, further dragging does nothing.
    When a QDockWidget reaches it maximium size, space will be given
    to other QDockWidgets in the opposite direction of the user's
    drag, if possible.  Once all QDockWidgets have reached their
    minimum size, futher dragging does nothing.

    \target dragging-dock-widgets
    \section2 Dragging Dock Widgets

    QDockWidget displays a titlebar to let the user drag the dock
    widget to a new location.  A QDockWidget can be moved to any
    location provided enough space is available.  QMainWindow will \e
    not resize itself to a larger size in an attempt to provide more
    space.

    A QRubberBand is shown while dragging the QDockWidget.  This
    QRubberBand provides an indication to the user about where the
    QDockWidget will be placed when the mouse button is released.

    \section2 Dragging over Neighbors

    All un-nested QDockWidgets in the same Qt::DockWidgetArea are
    considered neighbors.  When dragging a QDockWidget over its
    neighbor

    \list

    \i QMainWindow will split the neighbor perpendicularly to the
    direction of the QDockWidgets.

    \i QMainWindow will swap the position of the QDockWidget being
    dragged and its neighbor once the user has dragged the mouse past
    the center point of the neighboring QDockWidget.

    \endlist

    \section2 Dragging over other QDockWidgets

    When dragging nested QDockWidgets, or when dragging to a different
    Qt::DockWidgetArea, QMainWindow will split the QDockWindow under
    the mouse. \e Note: the QDockWidget under the mouse will only be
    split by the QDockWidget being dragged if both can fit in the
    space currently occupied by the QDockWidget under the mouse.

    A QDockWidget can be split horizontally or vertically, with the
    QDockWidget being dragged being placed in one of four possible
    locations.

    ### QDockWidget X diagram goes here

    When dragging a nested QDockWidget

    \section2 Dragging to a Different Qt::DockWidgetArea

    The QDockWidget::floatable property controls feedback during
    dragging:

    \list

    \i \c true - When dragging over the centralWidget(), QMainWindow
    choose a Qt::DockWindowArea based on the position of the mouse
    pointer.  If the mouse is within 50 pixels of the
    centralWidget()'s edge, the adjacent Qt::DockWindowArea is chosen.
    When dragging into the corners of these 50 pixel regions, the
    current corner() configuration is used to make the decision.
    Otherwise, the QRubberBand is shown under the mouse pointer, as
    above.

    \i \c false - When dragging over the centralWidget(), QMainWindow
    chooses a Qt::DockWindowArea based on the distance between the
    mouse pointer and the center of the centralWidget().  If the mouse
    comes within 50 pixels of the centralWidget()'s edge, the adjacent
    Qt::DockWindowArea is always chosen.  When dragging into the
    corners of these 50 pixel regions, the current corner()
    configuration is used to make the decision.

    \endlist

    In either case, dragging the mouse over another QDockWidget causes
    QMainWindow to choose the other QDockWidget's Qt::DockWindowArea.

    ### QMainWindow X diagram goes here

    \section2 Dragging outside the QMainWindow

    Again, the QDockWidget::floatable property controls feedback during
    dragging. When the QDockWidget::floatable property is \c false,
    dragging outside of the QMainWindow will show the rubberband over
    the QDockWidget's current location.  This indicates that the
    QDockWidget cannot be moved outside of the QMainWindow. When the
    QDockWidget::floatable property is \c true, dragging outside of
    the QMainWindow will show the QRubberBand under the mouse pointer.
    This indicates that the QDockWidget will be floating when the
    mouse button is released.

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
    : QWidget(*(new QMainWindowPrivate()), parent, flags | Qt::WType_TopLevel)
{
    d->init();
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
    d->init();
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
{ return d->iconSize; }

void QMainWindow::setIconSize(const QSize &iconSize)
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
    QMenuBar *menubar = qobject_cast<QMenuBar *>(d->layout->menuBar());
    if (!menubar) {
	QMainWindow *self = const_cast<QMainWindow *>(this);
	menubar = new QMenuBar(self);
	self->setMenuBar(menubar);
    }
    return menubar;
}

/*!
    Sets the menu bar for the main window to \a menubar.

    Note: QMainWindow takes ownership of the \a menubar pointer and
    deletes it at the appropriate time.

    \sa menuBar()
*/
void QMainWindow::setMenuBar(QMenuBar *menubar)
{
    if (d->layout->menuBar())
        delete d->layout->menuBar();
    d->layout->setMenuBar(menubar);
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

    Note: QMainWindow takes ownership of the \a statusbar pointer and
    deletes it at the appropriate time.

    \sa statusBar()
*/
void QMainWindow::setStatusBar(QStatusBar *statusbar)
{
    if (d->layout->statusBar())
        delete d->layout->statusBar();
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

    Note: QMainWindow takes ownership of the \a widget pointer and
    deletes it at the appropriate time.

    \sa centralWidget()
*/
void QMainWindow::setCentralWidget(QWidget *widget)
{ d->layout->setCentralWidget(widget); }

/*!
    Sets the given dock window \a area to occupy the specified \a
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
        d->layout->corners[corner] = area;
}

/*!
    Returns the dock window area that occupies the specified \a
    corner.

    \sa setCorner()
*/
Qt::DockWidgetArea QMainWindow::corner(Qt::Corner corner) const
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
    Q_ASSERT_X(toolbar->isAreaAllowed(area),
               "QMainWIndow::addToolBar", "specified 'area' is not an allowed area");

    toolbar->setIconSize(d->iconSize);
    connect(this, SIGNAL(iconSizeChanged(QSize)),
            toolbar, SLOT(setIconSize(QSize)));
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
    Q_ASSERT_X(toolbar->isAreaAllowed(toolBarArea(before)),
               "QMainWIndow::insertToolBar", "specified 'area' is not an allowed area");

    toolbar->setIconSize(d->iconSize);
    connect(this, SIGNAL(iconSizeChanged(QSize)),
            toolbar, SLOT(setIconSize(QSize)));
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
    disconnect(this, SIGNAL(iconSizeChanged(QSize)),
               toolbar, SLOT(setIconSize(QSize)));
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
    d->layout->addDockWidget(area, dockwidget, orientation);
    if (isVisible())
        d->layout->relayout();
}

/*!
    \fn void QMainWindow::splitDockWidget(QDockWidget *first, QDockWidget *second, Qt::Orientation orientation)

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
void QMainWindow::splitDockWidget(QDockWidget *after, QDockWidget *dockwidget,
                                  Qt::Orientation orientation)
{
    Qt::DockWidgetArea area = dockWidgetArea(after);
    Q_UNUSED(area);
    Q_ASSERT_X(dockwidget->isAreaAllowed(area),
               "QMainWindow::splitDockWidget", "specified 'area' is not an allowed area");
    d->layout->splitDockWidget(after, dockwidget, orientation);
    if (isVisible())
        d->layout->relayout();
}

/*!
    Removes the \a dockwidget from the main window.
*/
void QMainWindow::removeDockWidget(QDockWidget *dockwidget)
{ d->layout->removeRecursive(dockwidget); }

/*!
    Returns the \c Qt::DockWidgetArea for \a dockwidget.

    \sa addDockWidget() splitDockWidget() Qt::DockWidgetArea
*/
Qt::DockWidgetArea QMainWindow::dockWidgetArea(QDockWidget *dockwidget) const
{ return d->layout->dockWidgetArea(dockwidget); }

/*!
    Saves the current state of this mainwindow's toolbars and
    dockwidgets.  The \a version number is stored as part of the data.

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
    dockwidgets.  The \a version number is compared with that stored
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
    QMenu *popup = createPopupMenu();
    if (!popup)
	return;
    popup->exec(event->globalPos());
    delete popup;
    event->accept();
}

/*!
    This function is called to create a popup menu when the user
    right-clicks on the menubar, a toolbar or a dock window. If you
    want to create a custom popup menu, reimplement this function and
    return the created popup menu. Ownership of the popup menu is
    transferred to the caller.
*/
QMenu *QMainWindow::createPopupMenu()
{
    QMenu *menu = 0;
    QList<QToolBar *> toolbars = qFindChildren<QToolBar *>(this);
    QList<QDockWidget *> dockwidgets = qFindChildren<QDockWidget *>(this);
    if (toolbars.size() || dockwidgets.size()) {
        menu = new QMenu(this);
        if (!dockwidgets.isEmpty()) {
        for (int i = 0; i < dockwidgets.size(); ++i)
            menu->addAction(dockwidgets.at(i)->toggleViewAction());
            menu->addSeparator();
        }
        if (!toolbars.isEmpty()) {
            for (int i = 0; i < toolbars.size(); ++i)
                menu->addAction(toolbars.at(i)->toggleViewAction());
        }
    }
    return menu;
}
