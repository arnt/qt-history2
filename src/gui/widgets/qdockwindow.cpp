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
#include <qtoolbutton.h>

#include <private/qframe_p.h>
#include "qmainwindowlayout_p.h"
#include "qdockwindowlayout_p.h"
#define d d_func()
#define q q_func()




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
    void toggleFloated();
};

QDockWindowTitleButton::QDockWindowTitleButton(QDockWindowTitle *title)
    : QAbstractButton(title)
{ }

QSize QDockWindowTitleButton::sizeHint() const
{
    ensurePolished();

    int dim = 0;
    if (!icon().isNull()) {
        const QPixmap pm = icon().pixmap(QIconSet::Small, QIconSet::Normal);
        dim = qMax(pm.width(), pm.height());
    }

    return QSize(dim + 4, dim + 4);
}

void QDockWindowTitleButton::enterEvent(QEvent *event)
{
    update();
    QAbstractButton::enterEvent(event);
}

void QDockWindowTitleButton::leaveEvent(QEvent *event)
{
    update();
    QAbstractButton::leaveEvent(event);
}

void QDockWindowTitleButton::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    QRect r = rect();

    QStyle::SFlags flags = QStyle::Style_AutoRaise;
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (isChecked())
        flags |= QStyle::Style_On;
    if (isDown())
        flags |= QStyle::Style_Down;
    if (underMouse()) {
        flags |= QStyle::Style_MouseOver;
        if (!isChecked() && !isDown())
            flags |= QStyle::Style_Raised;
    }
    style().drawPrimitive(QStyle::PE_ButtonTool, &p, r, palette(), flags);

    r.addCoords(2, 2, -2, -2);
    const QPixmap pm = icon().pixmap(QIconSet::Small, QIconSet::Normal);
    p.drawPixmap(r.x() + (r.width() - pm.width()) / 2, r.y() + (r.height() - pm.height()) / 2, pm);
}

QDockWindowTitle::QDockWindowTitle(QDockWindow *tw)
    : QWidget(tw), dockwindow(tw), floatButton(0), closeButton(0), state(0)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

    box = new QBoxLayout(QBoxLayout::LeftToRight, this);
    box->setMargin(style().pixelMetric(QStyle::PM_DockWindowFrameWidth));
    box->setSpacing(0);
    box->addItem(spacer);

    updateButtons();
    updateWindowTitle();
}

void QDockWindowTitle::styleChange(QStyle &)
{
    box->setMargin(style().pixelMetric(QStyle::PM_DockWindowFrameWidth));

    updateWindowTitle();

    if (floatButton)
        floatButton->setIcon(style().stylePixmap(QStyle::SP_TitleBarMaxButton));

    if (closeButton)
        closeButton->setIcon(style().stylePixmap(QStyle::SP_TitleBarCloseButton));
}

void QDockWindowTitle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    Q_ASSERT(!state);

    // check if the tool window is movable... do nothing if it is not
    if (!dockwindow->isMovable())
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
    state->floating = dockwindow->isFloated()
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
        if (dockwindow->isFloatable()) {
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

    if (!dropped && dockwindow->isFloatable()) {
        target = state->floating;
        target.moveTopLeft(event->globalPos() - state->offset);

        if (!dockwindow->isFloated()) {
            dockwindow->hide();
            dockwindow->setFloated();
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
    // ### this should be done through the style

    QPainter p(this);
    const QPalette &pal = palette();

    if (dockwindow->isMovable()) {
        p.setPen(pal.color(QPalette::Dark));
        // p.setBrush(pal.background());
        p.drawRect(rect());
    }

    if (!dockwindow->windowTitle().isEmpty()) {
        QRect r = spacer->geometry();
        const int indent = p.fontMetrics().descent();
        r.addCoords(indent, 0, -indent, 0);

	p.setPen(pal.color(QPalette::Foreground));
	p.drawText(r, Qt::AlignLeft, dockwindow->windowTitle());
    }
}

void QDockWindowTitle::updateButtons()
{
    if (dockwindow->isFloatable()) {
        if (!floatButton) {
            floatButton = new QDockWindowTitleButton(this);
            floatButton->setIcon(style().stylePixmap(QStyle::SP_TitleBarMaxButton));
            connect(floatButton, SIGNAL(clicked()), SLOT(toggleFloated()));

            box->insertWidget(1, floatButton);
        }
    } else {
        delete floatButton;
        floatButton = 0;
    }

    if (dockwindow->isClosable()) {
        if (!closeButton) {
            closeButton = new QDockWindowTitleButton(this);
            closeButton->setIcon(style().stylePixmap(QStyle::SP_TitleBarCloseButton));
            connect(closeButton, SIGNAL(clicked()), dockwindow, SLOT(close()));

            box->insertWidget(2, closeButton);
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

void QDockWindowTitle::toggleFloated()
{
    dockwindow->setFloated(!dockwindow->isFloated(),
                           dockwindow->mapToGlobal(QPoint(height(), height())));
}





/*
  Private class
*/

class QDockWindowPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QDockWindow);

    public:
    inline QDockWindowPrivate(QMainWindow *parent)
	: QFramePrivate(), mainWindow(parent), closable(true), movable(true), floatable(true),
          currentArea(Qt::DockWindowAreaLeft), allowedAreas(~0u & Qt::DockWindowAreaMask),
          grid(0), box(0), title(0)
    { }

    void init();
    void place(Qt::DockWindowArea area, Qt::Orientation direction, bool extend);

    QMainWindow *mainWindow;

    bool closable;
    bool movable;
    bool floatable;
    Qt::DockWindowArea currentArea;
    Qt::DockWindowAreaFlags allowedAreas;

    QGridLayout *grid;
    QBoxLayout *box;
    QDockWindowTitle *title;
};

void QDockWindowPrivate::init() {
    q->setFrameStyle(QFrame::Panel | QFrame::Raised);

    d->grid = new QGridLayout(q, 2, 1);
    d->grid->setMargin(2);
    d->grid->setSpacing(2);

    d->title = new QDockWindowTitle(q);
    d->grid->addWidget(d->title, 0, 0);

    d->box = new QVBoxLayout(d->grid);
}

void QDockWindowPrivate::place(Qt::DockWindowArea area, Qt::Orientation direction, bool extend)
{
    QMainWindowLayout *mwl = qt_cast<QMainWindowLayout *>(d->mainWindow->layout());
    Q_ASSERT(mwl != 0);
    QDockWindowLayout *dwl = mwl->layoutForArea(area);
    Q_ASSERT(dwl != 0);

    if (extend)
        dwl->extend(q, direction);
    else
        dwl->split(q, direction);
}




/*
  Tool window
 */

QDockWindow::QDockWindow(QMainWindow *parent, Qt::WFlags flags)
    : QFrame(*(new QDockWindowPrivate(parent)), parent,
             flags | Qt::WStyle_Customize | Qt::WStyle_NoBorder)
{
    d->init();
}

QDockWindow::QDockWindow(QMainWindow *parent, Qt::DockWindowArea area, Qt::WFlags flags)
    : QFrame(*(new QDockWindowPrivate(parent)), parent,
             flags | Qt::WStyle_Customize | Qt::WStyle_NoBorder)
{
    d->init();
    setCurrentArea(area);
}

QDockWindow::~QDockWindow()
{ }

void QDockWindow::setParent(QMainWindow *parent)
{ QFrame::setParent(parent); }

QMainWindow *QDockWindow::mainWindow() const
{ return d->mainWindow; }

void QDockWindow::setClosable(bool closable)
{
    d->closable = closable;
    d->title->updateButtons();
}

bool QDockWindow::isClosable() const
{ return d->closable; }

void QDockWindow::setMovable(bool movable)
{
    d->movable = movable;
    d->title->updateButtons();
}

bool QDockWindow::isMovable() const
{ return d->movable; }

void QDockWindow::setFloatable(bool floatable)
{
    d->floatable = floatable;
    d->title->updateButtons();
}

bool QDockWindow::isFloatable() const
{ return d->movable && d->floatable; }

void QDockWindow::setFloated(bool floated, const QPoint &pos)
{
    bool visible = isVisible();

    setParent(floated ? 0 : d->mainWindow);

    if (floated) {
        if (!pos.isNull())
            move(pos);
    } else {
        setCurrentArea(d->currentArea);
    }

    if (visible)
        show();
}

bool QDockWindow::isFloated() const
{ return isTopLevel(); }

void QDockWindow::setAllowedAreas(Qt::DockWindowAreaFlags areas)
{ d->allowedAreas = (areas & Qt::DockWindowAreaMask); }

Qt::DockWindowAreaFlags QDockWindow::allowedAreas() const
{ return d->allowedAreas; }

Qt::DockWindowArea QDockWindow::currentArea() const
{ return d->currentArea; }

// add a window to an area using a hueristic to determine direction
void QDockWindow::setCurrentArea(Qt::DockWindowArea area)
{
    Q_ASSERT_X(((d->allowedAreas & area) == area),
               "QDockWindow::setCurrentArea", "specified 'area' is not an allowed area");

#ifdef Q_WS_MAC
    extern bool qt_mac_is_macdrawer(QWidget *); //qwidget_mac.cpp
    if (qt_mac_is_macdrawer(this)) {
        Qt::Dock x;
        switch (area) {
        case Qt::DockWindowAreaLeft:
            x = Qt::DockLeft;
            break;
        case Qt::DockWindowAreaRight:
            x = Qt::DockRight;
            break;
        case Qt::DockWindowAreaTop:
            x = Qt::DockTop;
            break;
        case Qt::DockWindowAreaBottom:
            x = Qt::DockBottom;
            break;
        default:
            Q_ASSERT(false);
        }
        // from qwidget_mac.cpp
        extern bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::Dock edge);
        qt_mac_set_drawer_preferred_edge(this, x);
        return;
    }
#endif

    d->currentArea = area;

    if (!isFloated()) {
        Qt::Orientation direction;
        switch (area) {
        case Qt::DockWindowAreaLeft:
        case Qt::DockWindowAreaRight:
            direction = Qt::Vertical;
            break;
        case Qt::DockWindowAreaTop:
        case Qt::DockWindowAreaBottom:
            direction = Qt::Horizontal;
            break;
        default:
            Q_ASSERT(false);
        }
        d->place(area, direction, true);
    }
}

// add a window to an area, placing done relative to the previous
void QDockWindow::setCurrentArea(Qt::DockWindowArea area, Qt::Orientation direction, bool extend)
{
    Q_ASSERT_X(((d->allowedAreas & area) == area),
               "QDockWindow::setCurrentArea", "specified 'area' is not an allowed area");

    d->currentArea = area;

    if (!isFloated())
        d->place(area, direction, extend);
}

// splits the specified dockwindow
void QDockWindow::setCurrentArea(QDockWindow *after, Qt::Orientation direction)
{
    Qt::DockWindowArea area = after->currentArea();
    Q_ASSERT_X(((d->allowedAreas & area) == area),
               "QDockWindow::setCurrentArea", "specified 'area' is not an allowed area");

    d->currentArea = area;

    Q_ASSERT_X(false, "QDockWindow::place", "place after specified dock window is unimplemented");
}

void QDockWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        d->title->updateWindowTitle();
        break;
    default:
        break;
    }
}

void QDockWindow::childEvent(QChildEvent *event)
{
    QWidget *child = qt_cast<QWidget *>(event->child());
    if (!child || child->isTopLevel()) return;

    if (event->added()) {
	if (d->box)
	    d->box->addWidget(child);
	else
	    ; // the title was added
    }
}

bool QDockWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Reparent:
	{
	    QWidget *parent = parentWidget();
            QMainWindow *mainWindow = qt_cast<QMainWindow *>(parent);

	    Q_ASSERT_X(!parent || mainWindow, "QDockWindow::setParent",
                       "QDockWindow must have QMainWindow as a parent");

            if (mainWindow)
                d->mainWindow = mainWindow;

            break;
        }
    default:
        break;
    }
    return QFrame::event(event);
}

#include "qdockwindow.moc"
