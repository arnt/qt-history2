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

struct QToolBarItem {
    QAction *action;
    QWidget *widget;
    bool hidden;

    inline QToolBarItem()
        : action(0), widget(0), hidden(false)
    { }
};

class QToolBarPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QToolBar);
public:
    inline QToolBarPrivate()
        : movable(true), allowedAreas(Qt::AllToolBarAreas), area(Qt::ToolBarAreaTop),
          handle(0), extension(0), ignoreActionEvent(false)
    { }

    void init();
    void actionTriggered();
    QToolBarItem createItem(QAction *action);

    bool movable;
    QSize old_size;
    Qt::ToolBarAreas allowedAreas;
    Qt::ToolBarArea area;
    QToolBarHandle *handle;
    QToolBarExtension *extension;

    QList<QToolBarItem> items;
    bool ignoreActionEvent;

};

void QToolBarPrivate::init()
{
    q->setFrameStyle(QFrame::ToolBarPanel | QFrame::Raised);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, q);
    layout->setSpacing(q->style().pixelMetric(QStyle::PM_ToolBarItemSpacing, q));

    handle = new QToolBarHandle(q);
    layout->addWidget(handle);

    extension = new QToolBarExtension(q);
    extension->hide();

    q->setArea(Qt::ToolBarAreaTop);
}

void QToolBarPrivate::actionTriggered()
{
    QAction *action = qt_cast<QAction *>(q->sender());
    Q_ASSERT_X(action != 0, "QToolBar::actionTriggered", "internal error");
    emit q->actionTriggered(action);
}

QToolBarItem QToolBarPrivate::createItem(QAction *action)
{
    QToolBarItem item;
    item.action = action;

    QBoxLayout *box = qt_cast<QBoxLayout *>(q->layout());
    if (action->isSeparator()) {
        item.widget =
            new QToolBarSeparator(q,
                                  (box->direction() == QBoxLayout::LeftToRight
                                   || box->direction() == QBoxLayout::RightToLeft)
                                  ? Qt::Horizontal
                                  : Qt::Vertical);
    } else {
        QToolBarButton *button = new QToolBarButton(q);
        button->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), q, SLOT(actionTriggered()));
        if (action->menu()) {
            QObject::connect(action->menu(), SIGNAL(activated(QAction *)),
                             q, SIGNAL(actionTriggered(QAction *)));
        }
        item.widget = button;
    }

    return item;
}

/*!
    \class QToolBar qtoolbar.h

    \brief The QToolBar class provides a movable panel that contains a
    set of controls.

    \ingroup application
    \mainclass

    \sa QMainWindow
*/

/*!
    Constructs a QToolBar with parent \a parent.
*/
QToolBar::QToolBar(QMainWindow *parent)
    : QFrame(*new QToolBarPrivate, parent)
{
    Q_ASSERT_X(parent != 0, "QToolBar", "parent cannot be zero");
    d->init();
}

#ifdef QT_COMPAT
/*! \obsolete
    Constructs a QToolBar with parent \a parent, named \a name.
*/
QToolBar::QToolBar(QMainWindow *parent, const char *name)
    : QFrame(*new QToolBarPrivate, parent)
{
    Q_ASSERT_X(parent != 0, "QToolBar", "parent cannot be zero");
    d->init();
    setObjectName(name);
}
#endif

/*!
    Destroys the tool bar.
*/
QToolBar::~QToolBar()
{
}

/*!
    Sets the main window for the tool bar to \a parent.

    \sa mainWindow()
 */
void QToolBar::setParent(QMainWindow *parent)
{ QFrame::setParent(parent); }

/*!
    Returns the main window for the tool bar.

    \sa setParent()
 */
QMainWindow *QToolBar::mainWindow() const
{ return qt_cast<QMainWindow *>(parentWidget()); }

/*! \property QToolBar::movable
    \brief whether the user can move the tool bar either within the
    tool bar area or to another tool bar area.

    This property is true by default.

    \sa QToolBar::allowedAreas
*/

void QToolBar::setMovable(bool movable)
{
    d->movable = movable;
    d->handle->setShown(d->movable);
}

bool QToolBar::isMovable() const
{ return d->movable; }

/*! \property QToolBar::allowedAreas
    \brief areas where the tool bar may be placed.

    The default is \c Qt::AllToolBarAreas.
*/

void QToolBar::setAllowedAreas(Qt::ToolBarAreas areas)
{ d->allowedAreas = (areas & Qt::ToolBarAreaMask); }

Qt::ToolBarAreas QToolBar::allowedAreas() const
{ return d->allowedAreas; }

/*! \property QToolBar::area
    \brief area where the tool bar is currently placed.

    The default is \c Qt::ToolBarAreaTop.
*/

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

/*!
    Adds \a action to the end of the tool bar.

    \sa addAction()
*/
void QToolBar::addAction(QAction *action)
{
    QToolBarItem item = d->createItem(action);
    d->items.append(item);
    qt_cast<QBoxLayout *>(layout())->insertWidget(d->items.size(), item.widget);

    if (!d->ignoreActionEvent) {
        d->ignoreActionEvent = true;

        QWidget::addAction(action);

        d->ignoreActionEvent = false;
    }
}

/*! \overload

    Creates a new action with text \a text. This action is added to
    the end of the tool bar.
*/
QAction *QToolBar::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addAction(action);
    return action;
}

/*! \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is added to the end of the tool bar.
*/
QAction *QToolBar::addAction(const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

/*! \overload

    Creates a new action with text \a text. This action is added to
    the end of the tool bar. The action's \link QAction::triggered()
    triggered()\endlink signal is connected to \a member in \a
    receiver.
*/
QAction *QToolBar::addAction(const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

/*! \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is added to the end of the tool bar. The action's \link
    QAction::triggered() triggered()\endlink signal is connected to \a
    member in \a receiver.
*/
QAction *QToolBar::addAction(const QIconSet &icon, const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

/*!
    Inserts \a action into the tool bar at position \a index.

    \sa addAction()
*/
void QToolBar::insertAction(int index, QAction *action)
{
    if (index < 0) {
        addAction(action);
        return;
    }

    QToolBarItem item = d->createItem(action);
    d->items.insert(index, item);
    qt_cast<QBoxLayout *>(layout())->insertWidget(index + 1, item.widget);

    if (!d->ignoreActionEvent) {
        d->ignoreActionEvent = true;

        // find the action before which we are inserted
        QAction *before = 0;
        for (int i = index + 1; !before && i < d->items.size(); ++i) {
            const QToolBarItem &item = d->items.at(i);
            if (item.action)
                before = item.action;
        }
        QWidget::insertAction(before, action);

        d->ignoreActionEvent = false;
    }

}

/*! \overload

    Creates a new action with text \a text. This action is inserted
    into the tool bar at position \a index.
*/
QAction *QToolBar::insertAction(int index, const QString &text)
{
    QAction *action = new QAction(text, this);
    insertAction(index, action);
    return action;
}

/*! \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is inserted into the tool bar at position \a index.
*/
QAction *QToolBar::insertAction(int index, const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    insertAction(index, action);
    return action;
}

/*! \overload

    Creates a new action with text \a text. This action is inserted
    into the tool bar at position \a index. The action's \link
    QAction::triggered() triggered()\endlink signal is connected to \a
    member in \a receiver.
*/
QAction *QToolBar::insertAction(int index, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(index, action);
    return action;
}

/*! \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is inserted into the tool bar at position \a index. The
    action's \link QAction::triggered() triggered()\endlink signal is
    connected to \a member in \a receiver.
*/
QAction *QToolBar::insertAction(int index, const QIconSet &icon, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(index, action);
    return action;
}

/*!
     Adds a separator to the end of the toolbar.

     \sa insertSeparator()
*/
QAction *QToolBar::addSeparator()
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

/*!
    Inserts a separator into the tool bar at position \a index.

    \sa addSeparator()
*/
QAction *QToolBar::insertSeparator(int index)
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(index, action);
    return action;
}

/*!
    Returns the action at position \a index. This function returns
    zero if there is no action at position \a index.

    \sa widget() addAction() insertAction()
*/
QAction *QToolBar::action(int index) const
{ return d->items.at(index).action; }

/*!
    Returns the position of \a action. This function returns -1 if \a
    action is not found.

    \sa addAction() insertAction()
*/
int QToolBar::indexOf(QAction *action) const
{
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.action == action)
            return i;
    }
    return -1;
}

/*!
    Returns the action at the point \a x, \a y. This function returns
    zero if no action was found.

    \sa QWidget::childAt()
*/
QAction *QToolBar::actionAt(int x, int y) const
{
    QWidget *widget = childAt(x, y);
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.widget == widget)
            return item.action;
    }
    return 0;
}

/*!
    Adds \a widget to the end of the tool bar.

    \sa insertWidget()
*/
void QToolBar::addWidget(QWidget *widget)
{
    QToolBarItem item;
    item.widget = widget;

    d->items.append(item);
    qt_cast<QBoxLayout *>(layout())->insertWidget(d->items.size(), item.widget);
}

/*!
    Inserts \a widget at position \a index.

    \sa addWidget()
*/
void QToolBar::insertWidget(int index, QWidget *widget)
{
    QToolBarItem item;
    item.widget = widget;

    d->items.insert(index, item);
    qt_cast<QBoxLayout *>(layout())->insertWidget(index + 1, item.widget);
}

/*!
    Removes \a widget from the tool bar.

    Note: the widget is not destroyed.

    \sa addWidget() insertWidget()
*/
void QToolBar::removeWidget(QWidget *widget)
{
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (!item.action && item.widget == widget) {
            d->items.removeAt(i);
            layout()->removeWidget(widget);
            return;
        }
    }
}

/*!
    Returns the widget at position \a index. This function returns
    zero if there is no widget at position \a index.

    \sa action() addWidget() insertWidget()
*/
QWidget *QToolBar::widget(int index) const
{
    const QToolBarItem &item = d->items.at(index);
    if (item.action)
        return 0;
    return item.widget;
}

/*!
    Returns the position of \a widget in the toolbar. This function
    returns -1 if \a widget was not found.

    \sa addWidget() insertWidget()
*/
int QToolBar::indexOf(QWidget *widget) const
{
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (!item.action && item.widget == widget)
            return i;
    }
    return -1;
}

/*! \reimp */
void QToolBar::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
    case QEvent::ActionAdded:
        {
            if (d->ignoreActionEvent)
                break;
            if (event->before()) {
                int index = indexOf(event->before());
                Q_ASSERT_X(index >= 0 && index < d->items.size(), "QToolBar::insertAction",
                           "internal error");
                insertAction(index, action);
            } else {
                addAction(action);
            }
            break;
        }

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
        {
            int index = indexOf(action);
            Q_ASSERT_X(index >= 0 && index < d->items.size(),
                       "QToolBar::removeAction", "internal error");

            QToolBarItem item = d->items.takeAt(index);

            // destroy the QToolBarButton/QToolBarSeparator
            delete item.widget;
            item.widget = 0;

            break;
        }

    default:
	Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
    }
}

/*! \reimp */
void QToolBar::childEvent(QChildEvent *event)
{
    if (event->type() != QEvent::ChildRemoved)
        return;
    QWidget *widget = qt_cast<QWidget *>(event->child());
    if (widget)
        removeWidget(widget);
}

/*! \reimp */
void QToolBar::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
    Qt::Orientation orientation = (box->direction() == QBoxLayout::LeftToRight
                                   || box->direction() == QBoxLayout::RightToLeft)
                                  ? Qt::Horizontal
                                  : Qt::Vertical;

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
    bool use_extension = (pick(orientation, size()) < pick(orientation, d->old_size))
			 || (pick(orientation, size()) < pick(orientation, real_sh));
    int hidden_count = 0;
    while (layout()->itemAt(i)) {
	QWidget *w = layout()->itemAt(i)->widget();
	if (pick(orientation, w->pos()) + pick(orientation, w->size())
	    >= (pick(orientation, size()) - ((use_extension && d->extension->isShown())
                                             ? pick(orientation, d->extension->size()) : 0))) {
            w->hide();
            d->items[i - 1].hidden = true;
            ++hidden_count;
        } else {
	    w->show();
            d->items[i - 1].hidden = false;
	}
	++i;
    }

    if (hidden_count > 0) {
	if (orientation == Qt::Horizontal) {
 	    d->extension->setGeometry(width() - d->extension->sizeHint().width() - frameWidth(),
				      frameWidth(),
				      d->extension->sizeHint().width() - frameWidth()*2,
				      height() - frameWidth()*2);
        } else {
 	    d->extension->setGeometry(frameWidth(),
				      height() - d->extension->sizeHint().height() - frameWidth()*2,
				      width() - frameWidth()*2,
				      d->extension->sizeHint().height());
        }

	QMenu *pop = d->extension->menu();
	if (!pop) {
	    pop = new QMenu(this);
	    d->extension->setMenu(pop);
	    d->extension->setPopupDelay(-1);
	}
	pop->clear();
	for(int i = 0; i < d->items.size(); ++i) {
            const QToolBarItem &item = d->items.at(i);
            if (!item.hidden) continue;

            if (item.action) {
                pop->addAction(item.action);
            } else {
                // ### needs special handling of custom widgets and
                // ### e.g. combo boxes - only actions are supported in
                // ### the preview
            }
        }
	d->extension->show();
    } else if (d->extension->isShown()) {
	if (d->extension->menu())
	    d->extension->menu()->clear();
	d->extension->hide();
    }
    d->old_size = size();
}

#include "moc_qtoolbar.cpp"
