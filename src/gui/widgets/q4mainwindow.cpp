#include "q4mainwindow.h"
#include "q4mainwindowlayout_p.h"

#include "q4dockwindow.h"
#include "q4toolbar.h"

#include <qmenubar.h>
#include <qstatusbar.h>

#include <private/qwidget_p.h>
#define d d_func()


class Q4MainWindowPrivate : public QWidgetPrivate
{
public:
    Q4MainWindowLayout *layout;
};


Q4MainWindow::Q4MainWindow(QWidget *parent, WFlags flags)
    : QWidget(*(new Q4MainWindowPrivate()), parent, flags)
{ d->layout = new Q4MainWindowLayout(this); }

Q4MainWindow::~Q4MainWindow()
{ }

QMenuBar *Q4MainWindow::menuBar() const
{
    QMenuBar *menubar = d->layout->menuBar();
    if (!menubar) {
	Q4MainWindow *self = const_cast<Q4MainWindow *>(this);
	menubar = new QMenuBar(self);
	self->setMenuBar(menubar);
    }
    return menubar;
}

void Q4MainWindow::setMenuBar(QMenuBar *menubar)
{
    Q_ASSERT_X(menubar != 0,
	       "Q4MainWindow::setMenuBar()", "parameter cannot be zero");
    Q_ASSERT_X(!d->layout->menuBar(),
	       "Q4MainWindow::setMenuBar()", "menu bar already set");
    Q_ASSERT_X(menubar->parentWidget() == this,
	       "Q4MainWindow::setMenuBar()", "menu bar parent must be the main window");
    layout()->setMenuBar(menubar);
}

QStatusBar *Q4MainWindow::statusBar() const
{
    QStatusBar *statusbar = d->layout->statusBar();
    if (!statusbar) {
	Q4MainWindow *self = const_cast<Q4MainWindow *>(this);
	statusbar = new QStatusBar(self);
	self->setStatusBar(statusbar);
    }
    return statusbar;
}

void Q4MainWindow::setStatusBar(QStatusBar *statusbar)
{
    Q_ASSERT_X(statusbar != 0,
	       "Q4MainWindow::setStatusBar()", "parameter cannot be zero");
    Q_ASSERT_X(!d->layout->statusBar(),
	       "Q4MainWindow::setStatusBar()", "status bar already set");
    Q_ASSERT_X(statusbar->parentWidget() == this,
	       "Q4MainWindow::setStatusBar()", "status bar parent must be the main window");
    d->layout->setStatusBar(statusbar);
}

QWidget *Q4MainWindow::centerWidget() const
{ return d->layout->centerWidget(); }

void Q4MainWindow::setCenterWidget(QWidget *widget)
{
    Q_ASSERT_X(widget != 0,
	       "Q4MainWindow::setCenterWidget()", "parameter cannot be zero");
    Q_ASSERT_X(!d->layout->centerWidget(),
	       "Q4MainWindow::setCenterWidget()", "center widget already set");
    Q_ASSERT_X(widget->parentWidget() == this,
	       "Q4MainWindow::setCenterWidget()", "center widget parent must be the main window");
    d->layout->setCenterWidget(widget);
}

void Q4MainWindow::setCorner(Corner corner, DockWindowArea area)
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
    Q_ASSERT_X(valid, "Q4MainWindow::setCorner", "'area' is not valid for 'corner'");
    d->layout->corners[corner] = area;
}

Qt::DockWindowArea Q4MainWindow::corner(Corner corner) const
{ return d->layout->corners[corner]; }

void Q4MainWindow::setDockWindowState(const QString &state)
{
    Q_ASSERT_X(false, "Q4MainWindow::setDockWindowState", "unimplemented");
}

QString Q4MainWindow::dockWindowState() const
{
    Q_ASSERT_X(false, "Q4MainWindow::dockWindowState", "unimplemented");
    return QString();
}

/*! \reimp */
void Q4MainWindow::childEvent(QChildEvent *event)
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
bool Q4MainWindow::event(QEvent *event)
{ return QWidget::event(event); }
