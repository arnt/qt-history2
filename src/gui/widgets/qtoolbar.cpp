#include "qtoolbar.h"

#include "qmainwindow.h"
#include "qmainwindowlayout_p.h"
#include "qtoolbarbutton_p.h"

#include <qaction.h>
#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>

#include <private/qframe_p.h>
#define d d_func()
#define q q_func()

static inline QCOORD pick(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Horizontal ? p.x() : p.y(); }

static inline QCOORD pick(Qt::Orientation o, const QSize &s)
{ return o == Qt::Horizontal ? s.width() : s.height(); }

// ## this should be styled and not inherit from QFrame
class QToolBarSeparator : public QFrame
{
public:
    QToolBarSeparator(QToolBar *parent, Qt::Orientation new_ori) : QFrame(parent)
    {
	setOrientation(new_ori);
    }

    inline QSize sizeHint() const
    {
	int ext = ori == Qt::Horizontal ? QFrame::sizeHint().width() : QFrame::sizeHint().height();
	return QSize(ext, ext);
    }
    inline QSize minimumSize() const
    { return sizeHint(); }
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void setOrientation(Qt::Orientation new_ori) {
	if (new_ori == Qt::Horizontal)
	    setFrameStyle(QFrame::VLine | QFrame::Sunken);
	else
	    setFrameStyle(QFrame::HLine | QFrame::Sunken);
	ori = new_ori;
    }
    Qt::Orientation orientation() { return ori; }

private:
    Qt::Orientation ori;
};

class QToolBarHandle : public QWidget
{
public:
    QToolBarHandle(QToolBar *parent) : QWidget(parent), state(0)
    {
	setCursor(Qt::SizeAllCursor);
    }

    QSize sizeHint() const {
	const int ext = QApplication::style().pixelMetric(QStyle::PM_DockWindowHandleExtent);
	return QSize(ext, ext);
    }
    inline QSize minimumSize() const
    { return sizeHint(); }
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    Qt::Orientation orientation();

    struct DragState {
	QPoint offset;
	bool canDrop;
    };
    DragState *state;
};


void QToolBarHandle::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QStyleOptionDockWindow opt(0);
    opt.state = QStyle::Style_Default;
    if (isEnabled())
	opt.state |= QStyle::Style_Enabled;
    QBoxLayout *box = qt_cast<QBoxLayout *>(parentWidget()->layout());
    if (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft)
	opt.state |= QStyle::Style_Horizontal;

    opt.rect = QStyle::visualRect(rect(), this);
    opt.palette = palette();
    style().drawPrimitive(QStyle::PE_DockWindowHandle, &opt, &p, this);
    QWidget::paintEvent(e);
}

void QToolBarHandle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    Q_ASSERT(parentWidget());
    Q_ASSERT(!state);
    state = new DragState;

    state->offset = mapTo(parentWidget(), event->pos());
}

void QToolBarHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    delete state;
    state = 0;
}

void QToolBarHandle::mouseMoveEvent(QMouseEvent *event)
{
    Q_ASSERT(state != 0);

    // see if there is a main window under us, and ask them to place
    // the tool bar accordingly
    QWidget *widget = QApplication::widgetAt(event->globalPos());
    if (widget) {
	while (widget && (!qt_cast<QMainWindow *>(widget)))
	    widget = widget->parentWidget();

	if (widget) {
 	    QMainWindowLayout *layout = qt_cast<QMainWindowLayout *>(widget->layout());
	    Q_ASSERT_X(layout != 0, "QMainWindow", "must have a layout");
	    QPoint p = parentWidget()->mapFromGlobal(event->globalPos()) - state->offset;

	    // ### the offset is measured from the widget origin
	    if (orientation() == Qt::Vertical)
		p.setX(state->offset.x() + p.x());
	    else
		p.setY(state->offset.y() + p.y());

	    // try re-positioning toolbar
	    layout->dropToolBar(qt_cast<QToolBar *>(parentWidget()), event->globalPos(), p);
	}
    }
}

Qt::Orientation QToolBarHandle::orientation()
{
    QBoxLayout *box = qt_cast<QBoxLayout *>(parentWidget()->layout());
    if (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft)
	return Qt::Horizontal;
    return Qt::Vertical;
}


// ### move this into the style code and make the extension stylable
static const char * const arrow_v_xpm[] = {
    "7 9 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    ".+++++.",
    "..+++..",
    "+..+..+",
    "++...++",
    ".++.++.",
    "..+++..",
    "+..+..+",
    "++...++",
    "+++.+++"};

static const char * const arrow_h_xpm[] = {
    "9 7 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    "..++..+++",
    "+..++..++",
    "++..++..+",
    "+++..++..",
    "++..++..+",
    "+..++..++",
    "..++..+++"};

class QToolBarExtension : public QToolButton
{
public:
    QToolBarExtension(QWidget *parent);

    void setOrientation(Qt::Orientation o);
    QSize sizeHint() const{ return QSize(14, 14); }
    QSize minimumSize() const { return sizeHint(); }
    QSize minimumSizeHint() const { return sizeHint(); }

private:
    Qt::Orientation orientation;
};

QToolBarExtension::QToolBarExtension(QWidget *parent)
    : QToolButton(parent)
{
    setAutoRaise(true);
    setOrientation(Qt::Horizontal);
}

void QToolBarExtension::setOrientation(Qt::Orientation o)
{
    if (o == Qt::Horizontal) {
        setIcon(QPixmap((const char **)arrow_h_xpm));
    } else {
        setIcon(QPixmap((const char **)arrow_v_xpm));
   }
}

class QToolBarPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QToolBar);
public:
    inline QToolBarPrivate()
        : movable(true), allowedAreas(Qt::AllToolBarAreas), area(Qt::ToolBarAreaTop),
          handle(0), extension(0)
    { }

    void actionTriggered();

    bool movable;
    QSize old_size;
    Qt::ToolBarAreaFlags allowedAreas;
    Qt::ToolBarArea area;
    QToolBarHandle *handle;
    QToolBarExtension *extension;
    void init();
};

void QToolBarPrivate::actionTriggered()
{
    QAction *action = qt_cast<QAction *>(q->sender());
    Q_ASSERT_X(action != 0, "QToolBar::actionTriggered", "internal error");
    emit q->actionTriggered(action);
}

void QToolBarPrivate::init()
{
    q->setFrameStyle(QFrame::Panel | QFrame::Raised);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, q);
    layout->setSpacing(2);
    handle = new QToolBarHandle(q);
    extension = new QToolBarExtension(q);
    extension->hide();
    layout->removeWidget(d->extension);
    q->setArea(Qt::ToolBarAreaTop);
}

QToolBar::QToolBar(QMainWindow *parent)
    : QFrame(*new QToolBarPrivate, parent)
{
    Q_ASSERT_X(parent != 0, "QToolBar", "parent cannot be zero");
    d->init();
}

#ifdef QT_COMPAT
QToolBar::QToolBar(QMainWindow *parent, const char *name)
    : QFrame(*new QToolBarPrivate, parent)
{
    Q_ASSERT_X(parent != 0, "QToolBar", "parent cannot be zero");
    d->init();
    setObjectName(name);
}
#endif

QToolBar::~QToolBar()
{
}

void QToolBar::setParent(QMainWindow *parent)
{ QFrame::setParent(parent); }

QMainWindow *QToolBar::mainWindow() const
{ return qt_cast<QMainWindow *>(parentWidget()); }

void QToolBar::setMovable(bool movable)
{
    d->movable = movable;
    d->handle->setShown(d->movable);
}

bool QToolBar::isMovable() const
{ return d->movable; }

void QToolBar::setAllowedAreas(Qt::ToolBarAreaFlags areas)
{ d->allowedAreas = (areas & Qt::ToolBarAreaMask); }

Qt::ToolBarAreaFlags QToolBar::allowedAreas() const
{ return d->allowedAreas; }

void QToolBar::setArea(Qt::ToolBarArea area, bool linebreak)
{
    Q_ASSERT_X(((d->allowedAreas & area) == area),
               "QToolBar::setArea", "specified 'area' is not an allowed area");

    Q_ASSERT(parentWidget() && qt_cast<QMainWindowLayout *>(parentWidget()->layout()));
    QMainWindowLayout *mainwin_layout = qt_cast<QMainWindowLayout *>(parentWidget()->layout());
    mainwin_layout->add(this, area, linebreak);

    int pos;
    switch (area) {
    case Qt::ToolBarAreaLeft:   pos = 0; break;
    case Qt::ToolBarAreaRight:  pos = 1; break;
    case Qt::ToolBarAreaTop:    pos = 2; break;
    case Qt::ToolBarAreaBottom: pos = 3; break;
    default:
        Q_ASSERT(false);
        break;
    }

    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
    Q_ASSERT_X(box != 0, "QToolBar::setArea", "internal error");

    switch (pos) {
    case 0: // Left
    case 1: // Right
	box->setDirection(QBoxLayout::TopToBottom);
	box->setAlignment(Qt::AlignTop);
	d->extension->setOrientation(Qt::Vertical);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	break;

    case 2: // Top
    case 3: // Bottom
	box->setDirection(QBoxLayout::LeftToRight);
	box->setAlignment(Qt::AlignLeft);
	d->extension->setOrientation(Qt::Horizontal);
	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	break;

    default:
	Q_ASSERT_X(false, "QToolBar::setArea", "internal error");
    }

    // change the orientation of any separators
    QLayoutItem *item = 0;
    int i = 0;
    while ((item = box->itemAt(i++))) {
	QToolBarSeparator *sep = qt_cast<QToolBarSeparator *>(item->widget());
	if (sep) {
	    if (box->direction() == QBoxLayout::LeftToRight
		|| box->direction() == QBoxLayout::RightToLeft)
		sep->setOrientation(Qt::Horizontal);
	    else
		sep->setOrientation(Qt::Vertical);
	}
    }

    // if we're dragging - swap the offset coords around as well
    if (d->handle->state) {
	QPoint p = d->handle->state->offset;
	d->handle->state->offset = QPoint(p.y(), p.x());
    }
    d->area = area;
}

Qt::ToolBarArea QToolBar::area() const
{ return d->area; }

QAction *QToolBar::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addAction(action);
    return action;
}

QAction *QToolBar::addAction(const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

QAction *QToolBar::addAction(const QString &text,
                              const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

QAction *QToolBar::addAction(const QIconSet &icon, const QString &text,
                              const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

QAction *QToolBar::insertAction(QAction *before, const QString &text)
{
    QAction *action = new QAction(text, this);
    insertAction(before, action);
    return action;
}

QAction *QToolBar::insertAction(QAction *before, const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    insertAction(before, action);
    return action;
}

QAction *QToolBar::insertAction(QAction *before, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(before, action);
    return action;
}

QAction *QToolBar::insertAction(QAction *before, const QIconSet &icon, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(before, action);
    return action;
}

QAction *QToolBar::addSeparator()
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

QAction *QToolBar::insertSeparator(QAction *before)
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
    return action;
}

void QToolBar::childEvent(QChildEvent *event)
{
    QWidget *widget = qt_cast<QWidget *>(event->child());
    if (!widget) return;
    if (event->added()) layout()->addWidget(widget);
    else if (event->removed()) layout()->removeWidget(widget);
}

void QToolBar::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
    case QEvent::ActionAdded:
	if (action->isSeparator()) {
	    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
	    if (box->direction() == QBoxLayout::LeftToRight
		|| box->direction() == QBoxLayout::RightToLeft)
		(void) new QToolBarSeparator(this, Qt::Horizontal);
	    else
		(void) new QToolBarSeparator(this, Qt::Vertical);
	} else {
	    QToolBarButton *button = new QToolBarButton(this);
	    button->addAction(action);

            QObject::connect(action, SIGNAL(triggered()),
                             this, SLOT(actionTriggered()));
            if (action->menu()) {
                QObject::connect(action->menu(), SIGNAL(activated(QAction *)),
                                 this, SIGNAL(actionTriggered(QAction *)));
            }
	}
	break;

    case QEvent::ActionChanged:
        action->disconnect(this, SLOT(actionTriggered()));
        QObject::connect(action, SIGNAL(triggered()),
                         this, SLOT(actionTriggered()));
        if (action->menu()) {
            action->menu()->disconnect(this, SIGNAL(actionTriggered(QAction *)));
            QObject::connect(action->menu(), SIGNAL(activated(QAction *)),
                             this, SIGNAL(actionTriggered(QAction *)));
        }
	break;

    case QEvent::ActionRemoved:
	break;

    default:
	Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
    }
}

void QToolBar::resizeEvent(QResizeEvent *)
{
    QList<int> hidden;
    Qt::Orientation o;
    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
    if (box->direction() == QBoxLayout::LeftToRight
	|| box->direction() == QBoxLayout::RightToLeft)
	o = Qt::Horizontal;
    else
	o = Qt::Vertical;

    // calculate the real size hint for the toolbar - even including
    // hidden buttons/tb items
    int i = 0;
    QSize real_sh(0, 0);
    while (layout()->itemAt(i))
	real_sh += layout()->itemAt(i++)->widget()->sizeHint();
    real_sh += QSize(layout()->spacing()*i + layout()->margin()*2,
		     layout()->spacing()*i + layout()->margin()*2);

    i = 1;  // tb handle is always the first item in the layout

    // only consider the size of the extension if the tb is shrinking
    int action_idx = 0;
    bool use_extension = (pick(o, size()) < pick(o, d->old_size))
			 || (pick(o, size()) < pick(o, real_sh));
    while (layout()->itemAt(i)) {
	QWidget *w = layout()->itemAt(i)->widget();
	if (pick(o, w->pos()) + pick(o, w->size())
	    >= (pick(o, size()) - ((use_extension && d->extension->isShown())
				   ? pick(o, d->extension->size()) : 0)))
	{
	    w->hide();
	    if (w->actions().size() > 0)
		hidden.append(action_idx);
	} else {
	    w->show();
	}
	if (w->actions().size() > 0)
	    ++action_idx;
	++i;
    }

    if (hidden.size() > 0) {
	if (o == Qt::Horizontal)
 	    d->extension->setGeometry(width() - d->extension->sizeHint().width() - frameWidth(),
				      frameWidth(),
				      d->extension->sizeHint().width() - frameWidth()*2,
				      height() - frameWidth()*2);
 	else
 	    d->extension->setGeometry(frameWidth(),
				      height() - d->extension->sizeHint().height() - frameWidth()*2,
				      width() - frameWidth()*2,
				      d->extension->sizeHint().height());

	QMenu *pop = d->extension->menu();
	if (!pop) {
	    pop = new QMenu(this);
	    d->extension->setMenu(pop);
	    d->extension->setPopupDelay(-1);
	    box->removeWidget(pop); // auto-added to the layout - remove it
	}
	pop->clear();
	for(int i = 0; i < hidden.size(); ++i) {
	    // ### needs special handling of custom widgets and
	    // ### e.g. combo boxes - only actions are supported in
	    // ### the preview
	    pop->addAction(actions().at(hidden.at(i)));
	}
	d->extension->show();
    } else if (d->extension->isShown() && hidden.size() == 0) {
	if (d->extension->menu())
	    d->extension->menu()->clear();
	d->extension->hide();
    }
    d->old_size = size();
}

#include "moc_qtoolbar.cpp"
