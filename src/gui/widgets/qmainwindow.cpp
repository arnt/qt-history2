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
    QMainWindowLayout *layout;
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
        valid = (area == Qt::DockWindowAreaTop || area == Qt::DockWindowAreaLeft);
        break;
    case Qt::TopRightCorner:
        valid = (area == Qt::DockWindowAreaTop || area == Qt::DockWindowAreaRight);
        break;
    case Qt::BottomLeftCorner:
        valid = (area == Qt::DockWindowAreaBottom || area == Qt::DockWindowAreaLeft);
        break;
    case Qt::BottomRightCorner:
        valid = (area == Qt::DockWindowAreaBottom || area == Qt::DockWindowAreaRight);
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
    Adds the \a toolbar into the specified \a area in this main
    window.  The \a toolbar is placed at the end of the current tool
    bar block (i.e. line).

    \sa insertToolBar() addToolBarBlock() insertToolBarBlock()
*/
void QMainWindow::addToolBar(QToolBar *toolbar, Qt::ToolBarArea area)
{
    Q_ASSERT_X(toolbar->isDockable(area),
               "QMainWIndow::addToolBar", "specified 'area' is not in 'allowedAreas'");

    d->layout->addToolBar(toolbar, area);

    if (isVisible())
        d->layout->relayout();
}

/*!
    Inserts the \a toolbar into the specified \a area in this main
    window.  The \a toolbar is placed before the toolbar \a before.

    \sa insertToolBarBlock() addToolBar() addToolBarBlock()
*/
void QMainWindow::insertToolBar(QToolBar *before, QToolBar *toolbar, Qt::ToolBarArea area)
{
    Q_ASSERT_X(false, "QMainWindow::insertToolBar", "unimplemented");
}

/*!
    Starts a new block (i.e. a new line) of tool bars in the specified
    \a area in this main window.  The \a toolbar is placed at the
    beginning of the new line.

    \sa addToolBar() insertToolBarBlock() insertToolBar()
*/
void QMainWindow::addToolBarBlock(QToolBar *toolbar, Qt::ToolBarArea area)
{
    Q_ASSERT_X(toolbar->isDockable(area),
               "QMainWIndow::addToolBar", "specified 'area' is not in 'allowedAreas'");

    d->layout->addToolBarBlock(toolbar, area);

    if (isVisible())
        d->layout->relayout();
}

/*!
    Starts a new block (i.e. a new line) of tool bars in the specified
    \a area in this main window.  The \a toolbar is placed before the
    toolbar \a before.

    \sa insertToolBar() addToolBarBlock() addToolBar()
 */
void QMainWindow::insertToolBarBlock(QToolBar *before, QToolBar *toolbar, Qt::ToolBarArea area)
{
    Q_ASSERT_X(false, "QMainWindow::insertToolBarBlock", "unimplemented");
}

/*!
    Removes the \a toolbar from the main window.
*/
void QMainWindow::removeToolBar(QToolBar *toolbar)
{ d->layout->removeWidget(toolbar); }

/*!
    Returns the tool bar area for \a toolbar.

    \sa addToolBar() addToolBarBlock() Qt::ToolBarArea
*/
Qt::ToolBarArea QMainWindow::toolBarArea(QToolBar *toolbar) const
{ return d->layout->toolBarArea(toolbar); }

/*!
    \internal
    Unimplemented: it should set the \a state for the dock window.
*/
void QMainWindow::setDockWindowState(const QString &/*state*/)
{
    Q_ASSERT_X(false, "QMainWindow::setDockWindowState", "unimplemented");
}

/*!
    \internal

    Unimplemented.
*/
QString QMainWindow::dockWindowState() const
{
    Q_ASSERT_X(false, "QMainWindow::dockWindowState", "unimplemented");
    return QString();
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
                if (area == Qt::ToolBarAreaLeft || area == Qt::ToolBarAreaRight)
                    deltaW -= toolbar->width();
                else
                    deltaH -= toolbar->height();
                toolbar->hide();
            } else {
                if (area == Qt::ToolBarAreaLeft || area == Qt::ToolBarAreaRight)
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
            sb->message(static_cast<QStatusTipEvent*>(event)->tip());
        else
            static_cast<QStatusTipEvent*>(event)->ignore();
        return true;
    }
    return QWidget::event(event);
}
