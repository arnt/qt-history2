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


QMainWindow::QMainWindow(QWidget *parent, WFlags flags)
    : QWidget(*(new QMainWindowPrivate()), parent, flags)
{ d->layout = new QMainWindowLayout(this); }

QMainWindow::~QMainWindow()
{ }

QMenuBar *QMainWindow::menuBar() const
{
    QMenuBar *menubar = d->layout->menuBar();
    if (!menubar) {
	QMainWindow *self = const_cast<QMainWindow *>(this);
	menubar = new QMenuBar(self);
	self->setMenuBar(menubar);
    }
    return menubar;
}

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

QWidget *QMainWindow::centerWidget() const
{ return d->layout->centerWidget(); }

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

void QMainWindow::setCorner(Corner corner, DockWindowArea area)
{
    bool valid = false;
    switch (corner) {
    case TopLeft:
        valid = (area == DockWindowAreaTop || area == DockWindowAreaLeft);
        break;
    case TopRight:
        valid = (area == DockWindowAreaTop || area == DockWindowAreaRight);
        break;
    case BottomLeft:
        valid = (area == DockWindowAreaBottom || area == DockWindowAreaLeft);
        break;
    case BottomRight:
        valid = (area == DockWindowAreaBottom || area == DockWindowAreaRight);
        break;
    }
    Q_ASSERT_X(valid, "QMainWindow::setCorner", "'area' is not valid for 'corner'");
    d->layout->corners[corner] = area;
}

Qt::DockWindowArea QMainWindow::corner(Corner corner) const
{ return d->layout->corners[corner]; }

void QMainWindow::setDockWindowState(const QString &state)
{
    Q_ASSERT_X(false, "QMainWindow::setDockWindowState", "unimplemented");
}

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
	    QMenuBar *mb = d->layout->menuBar();
	    Q_ASSERT(mb == 0 || mb == menubar);
	    if (!mb) d->layout->setMenuBar(menubar);
	} else if ((statusbar = qt_cast<QStatusBar *>(event->child()))) {
	    QStatusBar *sb = d->layout->statusBar();
	    Q_ASSERT(sb == 0 || sb == statusbar);
	    if (!sb) d->layout->setStatusBar(statusbar);
	}
    }
}

/*! \reimp */
bool QMainWindow::event(QEvent *event)
{ return QWidget::event(event); }
