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

#include <qmenubar.h>
#include <qstatusbar.h>

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
    |   |   | Center Widget                            |   |   |
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
    Q_ASSERT_X(menubar->parentWidget() == this,
	       "QMainWindow::setMenuBar()", "menu bar parent must be the main window");
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
    Q_ASSERT_X(statusbar->parentWidget() == this,
	       "QMainWindow::setStatusBar()", "status bar parent must be the main window");
    d->layout->setStatusBar(statusbar);
}

/*!
    Returns the center widget for the main window.  This function
    returns zero if the center widget has not been set.

    \sa setCenterWidget()
*/
QWidget *QMainWindow::centerWidget() const
{ return d->layout->centerWidget(); }

/*!
    Sets the given \a widget to be the main window's center widget.

    \warning This function should be called at most once for each main
    window instance, and the widget passed must be a child of the main
    window.

    \sa centerWidget()
*/
void QMainWindow::setCenterWidget(QWidget *widget)
{
    Q_ASSERT_X(widget != 0,
	       "QMainWindow::setCenterWidget()", "parameter cannot be zero");
    Q_ASSERT_X(!d->layout->centerWidget(),
	       "QMainWindow::setCenterWidget()", "center widget already set");
    Q_ASSERT_X(widget->parentWidget() == this,
	       "QMainWindow::setCenterWidget()", "center widget parent must be the main window");
    d->layout->setCenterWidget(widget);
}

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
{ return QWidget::event(event); }
