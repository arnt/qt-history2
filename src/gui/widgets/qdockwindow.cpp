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

#include "qdockwindow.h"
#include "qmainwindow.h"

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>
#include <qaction.h>

#include <private/qwidgetresizehandler_p.h>

#include "qdockwindow_p.h"
#include "qmainwindowlayout_p.h"
#include "qdockwindowlayout_p.h"

#define d d_func()
#define q q_func()

/*
    A Dock Window:

    [+] is the float button
    [X] is the close button

    +-------------------------------+
    | Dock Window Title       [+][X]|
    +-------------------------------+
    |                               |
    | place to put the single       |
    | QDockWindow child (this space |
    | does not yet have a name)     |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    +-------------------------------+

*/




/*
  Tool window title
*/

class QDockWindowTitle;

class QDockWindowTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    QDockWindowTitleButton(QDockWindowTitle *title);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};

class QDockWindowTitle : public QWidget
{
    Q_OBJECT

public:
    QDockWindowTitle(QDockWindow *tw);

    void styleChange(QStyle &);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    void updateButtons();
    void updateWindowTitle();

    QDockWindow *dockwindow;

    QSpacerItem *spacer;
    QDockWindowTitleButton *floatButton;
    QDockWindowTitleButton *closeButton;

    QBoxLayout *box;

    struct DragState {
	QRubberBand *rubberband;
        QRect origin;   // starting position
	QRect current;  // current size of the dockwindow (can be either placed or floating)
	QRect floating; // size of the floating dockwindow
        QPoint offset;
	bool canDrop;
    };
    DragState *state;

public slots:
    void toggleTopLevel();
};

QDockWindowTitleButton::QDockWindowTitleButton(QDockWindowTitle *title)
    : QAbstractButton(title)
{ }

QSize QDockWindowTitleButton::sizeHint() const
{
    ensurePolished();

    int dim = 0;
    if (!icon().isNull()) {
        const QPixmap pm = icon().pixmap(Qt::AutomaticIconSize, QIcon::Normal);
        dim = qMax(pm.width(), pm.height());
    }

    return QSize(dim + 4, dim + 4);
}

void QDockWindowTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void QDockWindowTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void QDockWindowTitleButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r = rect();
    QStyleOption opt(0);
    opt.rect = r;
    opt.palette = palette();
    opt.state = QStyle::Style_AutoRaise;
    if (isEnabled()) {
        opt.state |= QStyle::Style_Enabled;
        if (underMouse()) {
            opt.state |= QStyle::Style_MouseOver;
            if (!isChecked() && !isDown())
                opt.state |= QStyle::Style_Raised;
        }
    }
    if (isChecked())
        opt.state |= QStyle::Style_On;
    if (isDown())
        opt.state |= QStyle::Style_Down;
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

    r.addCoords(2, 2, -2, -2);
    const QPixmap pm =
        icon().pixmap(Qt::AutomaticIconSize, isEnabled() ? QIcon::Normal : QIcon::Disabled);
    style()->drawItem(&p, r, Qt::AlignCenter, palette(), isEnabled(), pm);
}

QDockWindowTitle::QDockWindowTitle(QDockWindow *tw)
    : QWidget(tw), dockwindow(tw), floatButton(0), closeButton(0), state(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

    box = new QBoxLayout(QBoxLayout::LeftToRight, this);
    box->setMargin(style()->pixelMetric(QStyle::PM_DockWindowFrameWidth));
    box->setSpacing(0);
    box->addItem(spacer);

    updateButtons();
    updateWindowTitle();
}

void QDockWindowTitle::styleChange(QStyle &)
{
    box->setMargin(style()->pixelMetric(QStyle::PM_DockWindowFrameWidth));

    updateWindowTitle();

    if (floatButton)
        floatButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarMaxButton));

    if (closeButton)
        closeButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
}

void QDockWindowTitle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    Q_ASSERT(!state);

    // check if the tool window is movable... do nothing if it is not
    if (!dockwindow->hasFeature(QDockWindow::DockWindowMovable))
        return;

    state = new DragState;

    QMainWindowLayout *layout =
        qt_cast<QMainWindowLayout *>(dockwindow->mainWindow()->layout());
    Q_ASSERT(layout != 0);
    layout->saveLayoutInfo();

    const int screen_number = QApplication::desktop()->screenNumber(topLevelWidget());
    state->rubberband = new QRubberBand(QRubberBand::Rectangle,
                                        QApplication::desktop()->screen(screen_number));

    // the current location of the tool window in global coordinates
    state->origin = QRect(dockwindow->mapToGlobal(QPoint(0, 0)), dockwindow->size());
    state->current = state->origin;

    // like the above, except using the tool window's size hint
    state->floating = dockwindow->isTopLevel()
                      ? state->current
                      : QRect(state->current.topLeft(), dockwindow->sizeHint());

    // the offset is the middle of the titlebar in a window of floating.size()
    state->offset = mapFrom(dockwindow, QPoint(state->floating.width() / 2, 0));
    state->offset = mapTo(dockwindow, QPoint(state->offset.x(), height() / 2));
    state->canDrop = true;

    state->rubberband->setGeometry(state->current);
    state->rubberband->show();
}

void QDockWindowTitle::mouseMoveEvent(QMouseEvent *event)
{
    if (!state) return;

    QRect target;

    // see if there is a main window under us, and ask it to place the tool window
    QWidget *widget = QApplication::widgetAt(event->globalPos());
    if (widget) {
	while (widget && !qt_cast<QMainWindow *>(widget)) {
	    if (widget->isTopLevel()) {
		widget = 0;
		break;
	    }
	    widget = widget->parentWidget();
	}

	if (widget) {

            QMainWindow *mainwindow = qt_cast<QMainWindow *>(widget);
            if (mainwindow && mainwindow == dockwindow->mainWindow()) {
		QMainWindowLayout *layout =
                    qt_cast<QMainWindowLayout *>(dockwindow->mainWindow()->layout());
                Q_ASSERT(layout != 0);
                QRect request = state->origin;
                request.moveTopLeft(event->globalPos() - state->offset);
		target = layout->placeDockWindow(dockwindow, request, event->globalPos());
                layout->resetLayoutInfo();
	    }
	}
    }

    state->canDrop = target.isValid();
    if (!state->canDrop) {
        if (dockwindow->hasFeature(QDockWindow::DockWindowFloatable)) {
            /*
              main window refused to accept the tool window,
              recalculate absolute position as if the tool window
              was to be dropped to toplevel
            */
            target = state->floating;
            target.moveTopLeft(event->globalPos() - state->offset);
        } else {
            /*
              cannot float the window, so put it back into it's
              original position
            */
            target = state->origin;
        }
    }

    if (state->current == target) return;

    state->rubberband->setGeometry(target);
    state->current = target;
}

void QDockWindowTitle::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    if (!state) return;

    delete state->rubberband;

    QMainWindowLayout *layout =
        qt_cast<QMainWindowLayout *>(dockwindow->mainWindow()->layout());
    Q_ASSERT(layout != 0);
    layout->discardLayoutInfo();

    // calculate absolute position if the tool window was to be
    // dropped to toplevel
    QRect target;

    QWidget *focus = qApp->focusWidget();

    // see if there is a main window under us, and ask it to drop the tool window
    QWidget *widget = QApplication::widgetAt(event->globalPos());
    bool dropped = false;
    if (state->canDrop && widget) {
        while (widget && !qt_cast<QMainWindow *>(widget)) {
            if (widget->isTopLevel()) {
                widget = 0;
                break;
            }
            widget = widget->parentWidget();
        }

        if (widget) {
            QMainWindow *mainwindow = qt_cast<QMainWindow *>(widget);
            if (mainwindow && mainwindow == dockwindow->mainWindow()) {
                QMainWindowLayout *layout =
                    qt_cast<QMainWindowLayout *>(dockwindow->mainWindow()->layout());
                Q_ASSERT(layout != 0);
                QRect request = state->origin;
                request.moveTopLeft(event->globalPos() - state->offset);
                layout->dropDockWindow(dockwindow, request, event->globalPos());
                dropped = true;
            }
        }
    }

    if (!dropped && dockwindow->hasFeature(QDockWindow::DockWindowFloatable)) {
        target = state->floating;
        target.moveTopLeft(event->globalPos() - state->offset);

        if (!dockwindow->isTopLevel()) {
            dockwindow->hide();
            dockwindow->setTopLevel();
            dockwindow->setGeometry(target);
            dockwindow->show();
        } else {
            // move to new location
            dockwindow->setGeometry(target);
        }
    }

    // restore focus
    if (focus) focus->setFocus();

    delete state;
    state = 0;
}

void QDockWindowTitle::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QStyleOptionDockWindow opt;
    opt.rect = rect();
    opt.palette = palette();
    if (isEnabled()) {
        opt.state |= QStyle::Style_Enabled;
        if (underMouse())
            opt.state |= QStyle::Style_MouseOver;
    }
    opt.title = dockwindow->windowTitle();
    opt.closable = dockwindow->hasFeature(QDockWindow::DockWindowClosable);
    opt.moveable = dockwindow->hasFeature(QDockWindow::DockWindowMovable);
    opt.floatable = dockwindow->hasFeature(QDockWindow::DockWindowFloatable);
    style()->drawControl(QStyle::CE_DockWindowTitle, &opt, &p, this);
}

void QDockWindowTitle::updateButtons()
{
    if (dockwindow->hasFeature(QDockWindow::DockWindowFloatable)) {
        if (!floatButton) {
            floatButton = new QDockWindowTitleButton(this);
            floatButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarMaxButton));
            connect(floatButton, SIGNAL(clicked()), SLOT(toggleTopLevel()));

            box->insertWidget(1, floatButton);

            if (dockwindow->isShown())
                floatButton->show();
        }
    } else {
        delete floatButton;
        floatButton = 0;
    }

    if (dockwindow->hasFeature(QDockWindow::DockWindowClosable)) {
        if (!closeButton) {
            closeButton = new QDockWindowTitleButton(this);
            closeButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
            connect(closeButton, SIGNAL(clicked()), dockwindow, SLOT(close()));

            box->insertWidget(2, closeButton);

            if (dockwindow->isShown())
                closeButton->show();
        }
    } else {
        delete closeButton;
        closeButton = 0;
    }
}

void QDockWindowTitle::updateWindowTitle()
{
    const QFontMetrics fm(font());
    spacer->changeSize(fm.width(dockwindow->windowTitle()) + box->margin() * 2, fm.lineSpacing(),
                       QSizePolicy::Expanding, QSizePolicy::Fixed);

    update();
}

void QDockWindowTitle::toggleTopLevel()
{
    dockwindow->setTopLevel(!dockwindow->isTopLevel(),
                           dockwindow->mapToGlobal(QPoint(height(), height())));
}





/*
  Private class
*/

void QDockWindowPrivate::init() {
    q->setFrameStyle(QFrame::Panel | QFrame::Raised);

    d->top = new QVBoxLayout(q);
    d->top->setMargin(2);
    d->top->setSpacing(2);

    d->title = new QDockWindowTitle(q);
    d->top->insertWidget(0, d->title);

    d->box = new QVBoxLayout(d->top);

    d->resizer = new QWidgetResizeHandler(q);
    d->resizer->setMovingEnabled(false);
    d->resizer->setActive(false);

    d->toggleViewAction = new QAction(q);
    d->toggleViewAction->setCheckable(true);
    d->toggleViewAction->setText(q->windowTitle());
    QObject::connect(d->toggleViewAction, SIGNAL(checked(bool)), q, SLOT(toggleView(bool)));
}

void QDockWindowPrivate::toggleView(bool b)
{
    if (b != q->isShown()) {
        if (b)
            q->show();
        else
            q->close();
    }
}



/*!
    \class QDockWindow

    \brief The QDockWindow class provides a widget that can be docked
    inside a QMainWindow or floated as a top-level window on the
    desktop.

    \ingroup application

    QDockWindow provides the concept of dock windows, also know as
    tool palettes or utility windows.  Dock windows are secondary
    windows placed in the \e {dock window area} around the \link
    QMainWindow::centerWidget() center widget\endlink in a
    QMainWindow.

    ### \e {mainwindow diagram showing the various areas of the
    mainwindow should go here - see below}

    Dock windows can be moved inside their current area, moved into
    new areas and floated (e.g. undocked) by the end-user.  The
    QDockWindow API allows the programmer to restrict the dock windows
    ability to move, float and close, as well as the areas in which
    they can be placed.

    \section1 Appearance

    A QDockWindow consists of a title bar and the content area.  The
    titlebar displays the dock windows \link QWidget::windowTitle()
    window title\endlink, a \e float button and a \e close button.
    Depending on the state of the QDockWindow, the \e float and \e
    close buttons may be either disabled or not shown at all.

    The visual appearance of the title bar and buttons is \link QStyle
    style \endlink dependent.

    ### \e {screenshot of QDockWindow in a few styles should go here}

    \section1 Behavior

    \list

    \i behaviour of QDockWindow while docked in QMainWindow:

    \i behaviour of QDockWindow while floated:

    \i behaviour while dragging QDockWindow:

        \list

        \i to an unoccupied dock window area:

        \i to an occupied dock window area:

        \i to top-level:

        \endlist

    \endlist

    \sa QMainWindow
*/

/*!
    \enum QDockWindow::DockWindowFeature

    \value DockWindowClosable   The dock window can be closed.
    \value DockWindowMovable    The dock window can be moved between docks
                                by the user.
    \value DockWindowFloatable  The dock window can be detached from the
                                main window, and floated as an independent
                                window.

    \value AllDockWindowFeatures  The dock window can be closed, moved,
                                  and floated.
    \value NoDockWindowFeatures   The dock window cannot be closed, moved,
                                  or floated.

    \omitvalue DockWindowFeatureMask
*/

/*!
    Constructs a QDockWindow with parent \a parent and widget flags \a
    flags.  The dock window will be placed in the left dock window
    area.
*/
QDockWindow::QDockWindow(QMainWindow *parent, Qt::WFlags flags)
    : QFrame(*(new QDockWindowPrivate(parent)), parent,
             flags | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool)
{
    Q_ASSERT_X(parent != 0, "QDockWindow", "parent cannot be zero");
    d->init();
}

/*!
    Destroys the dock window.
*/
QDockWindow::~QDockWindow()
{ }

/*!
    Returns the main window for this dock window.

    \sa setParent()
*/
QMainWindow *QDockWindow::mainWindow() const
{ return d->mainWindow; }

/*!
    Sets the main window for this dock window to \a parent.

    \sa mainWindow()
*/
void QDockWindow::setParent(QMainWindow *parent)
{ QFrame::setParent(parent); }

/*!
    Returns the widget for the dock window. This function returns zero
    if the widget has not been set.

    \sa setWidget()
*/
QWidget *QDockWindow::widget() const
{ return d->widget; }

/*!
    Sets the widget for the dock window to \a widget.

    \sa widget()
*/
void QDockWindow::setWidget(QWidget *widget)
{
    Q_ASSERT_X(widget != 0, "QDockWindow::setWidget", "parameter cannot be zero");
    Q_ASSERT_X(d->widget == 0, "QDockWindow::setWidget", "widget already set");
    d->widget = widget;
    d->box->insertWidget(1, widget);
}

/*!
    \property QDockWindow::features
    \brief whether the dock window is movable, closable, and floatable

    \sa DockWindowFeature setFeature() hasFeature()
*/
void QDockWindow::setFeatures(QDockWindow::DockWindowFeatures features)
{
    features &= DockWindowFeatureMask;
    if (d->features == features)
        return;
    d->features = features;
    d->title->updateButtons();
    d->title->update();
}

/*!
    Switches on the given \a feature if \a on is true; otherwise
    switches it off.

    \sa hasFeature() features
*/
void QDockWindow::setFeature(DockWindowFeature feature, bool on)
{ setFeatures(on ? d->features | feature : d->features & ~feature); }


QDockWindow::DockWindowFeatures QDockWindow::features() const
{ return d->features; }

/*!
    Returns true if the dock window has the given \a feature;
    otherwise returns false.

    \sa setFeature() features
*/
bool QDockWindow::hasFeature(DockWindowFeature feature) const
{ return d->features & feature; }

/*!
    \internal

    Sets the doc window to be top level. If \a floated is true it has
    no parent and floats free at position \a pos; if \a floated is
    false it is reparented to the main window.
*/
void QDockWindow::setTopLevel(bool floated, const QPoint &pos)
{
    bool visible = isVisible();

    QFrame::setParent(d->mainWindow, (floated
                                      ? (getWFlags() | Qt::WType_TopLevel)
                                      : (getWFlags() & ~Qt::WType_TopLevel)));

    if (floated) {
        if (!pos.isNull())
            move(pos);
        d->mainWindow->layout()->invalidate();
    }

    d->resizer->setActive(floated);

    if (visible)
        show();
}

/*!
    \property QDockWindow::allowedAreas
    \brief areas where the dock window may be placed

    The default is \c Qt::AllDockWindowAreas.

    \sa QDockWindow::area
*/

void QDockWindow::setAllowedAreas(Qt::DockWindowAreas areas)
{ d->allowedAreas = (areas & Qt::DockWindowArea_Mask); }

Qt::DockWindowAreas QDockWindow::allowedAreas() const
{ return d->allowedAreas; }

/*!
    \fn bool QDockWindow::isDockable(Qt::DockWindowArea area)

    Returns true if this dock window can be placed in the given \a area;
    otherwise returns false.
*/

/*! \reimp */
void QDockWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        d->title->updateWindowTitle();
        d->toggleViewAction->setText(windowTitle());
        break;
    default:
        break;
    }
    QFrame::changeEvent(event);
}

/*! \reimp */
void QDockWindow::closeEvent(QCloseEvent *event)
{
    if (!(d->features & DockWindowClosable))
        event->ignore();
}

/*! \reimp */
bool QDockWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ParentChange:
	{
	    QWidget *parent = parentWidget();
            QMainWindow *mainWindow = qt_cast<QMainWindow *>(parent);

	    Q_ASSERT_X(!parent || mainWindow, "QDockWindow::setParent",
                       "QDockWindow must have QMainWindow as a parent");

            if (mainWindow)
                d->mainWindow = mainWindow;

            break;
        }
    case QEvent::Show:
    case QEvent::Hide:
        if (!event->spontaneous())
            d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
    default:
        break;
    }
    return QFrame::event(event);
}

/*!
  Returns a checkable action that can be used to show or close this
  dock window.

  The action's text is set to the dock window's window title.

  \sa QAction::text QWidget::windowTitle
 */
QAction * QDockWindow::toggleViewAction() const
{
    return d->toggleViewAction;
}

#include "qdockwindow.moc"
#include "moc_qdockwindow.cpp"
