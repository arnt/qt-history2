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

#include "qtoolbar.h"

#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>
#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
#endif

#include <private/qmainwindowlayout_p.h>

#include "qtoolbar_p.h"
#include "qtoolbarbutton_p.h"
#include "qtoolbarextension_p.h"
#include "qtoolbarhandle_p.h"
#include "qtoolbarseparator_p.h"

#define d d_func()
#define q q_func()

/*
    QToolBarPrivate
*/

void QToolBarPrivate::init()
{
    q->setFrameStyle(QFrame::ToolBarPanel | QFrame::Raised);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, q);
    layout->setMargin(0);
    layout->setSpacing(q->style().pixelMetric(QStyle::PM_ToolBarItemSpacing, q));

    handle = new QToolBarHandle(q);
    layout->addWidget(handle);

    extension = new QToolBarExtension(q);
    extension->hide();

    q->setArea(Qt::ToolBarAreaTop);
#ifdef Q_WS_MAC
    // Make sure that the window has the "toolbar" button.
    extern WindowPtr qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
    ChangeWindowAttributes(qt_mac_window_for(q->parentWidget()), kWindowToolbarButtonAttribute,
                           kWindowNoAttributes);
#endif
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
    item.hidden = false;

    QBoxLayout *box = qt_cast<QBoxLayout *>(q->layout());

    QToolBarWidgetAction *widgetAction = qt_cast<QToolBarWidgetAction *>(action);
    if (widgetAction) {
        item.widget = widgetAction->widget();
    } else if (action->isSeparator()) {
        Qt::Orientation orientation = (box->direction() == QBoxLayout::LeftToRight
                                       || box->direction() == QBoxLayout::RightToLeft)
                                      ? Qt::Horizontal
                                      : Qt::Vertical;
        item.widget = new QToolBarSeparator(orientation, q);
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

/*
    Returns the position of \a action. This function returns -1 if \a
    action is not found.
*/
int QToolBarPrivate::indexOf(QAction *action) const
{
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.action == action)
            return i;
    }
    return -1;
}





/*!
    \class QToolBar qtoolbar.h

    \brief The QToolBar class provides a movable panel that contains a
    set of controls.

    \ingroup application
    \mainclass

    Toolbar buttons are added by adding \e actions, using addAction()
    or insertAction(). Groups of buttons can be separated using
    addSeparator() or insertSeparator(). If a toolbar button is not
    appropriate, a widget can be inserted instead using addWidget() or
    insertWidget(); examples of suitable widgets are QSpinBox,
    QDoubleSpinBox, and QComboBox. When a toolbar button is pressed it
    emits the actionTriggered() signal. Toolbars may only be added to
    QMainWindow and QMainWindow subclasses.

    A toolbar can be fixed in place in a particular area() (e.g. at the
    top of the window), or it can be movable() between toolbar areas, or
    it can be floatable: see \l{allowedAreas} and isDockable().

*/

/*!
    \fn bool QToolBar::isDockable(Qt::ToolBarArea area)

    Returns true if this tool bar is dockable in the given \a area;
    otherwise returns false.
*/

/*!
    \fn void QToolBar::actionTriggered(QAction *action)

    This signal is emitted when a tool bar button is pressed. The
    parameter holds the tool bar button's associated \a action.
*/

/*!
    Constructs a QToolBar with the given \a parent.
*/
QToolBar::QToolBar(QMainWindow *parent)
    : QFrame(*new QToolBarPrivate, parent)
{
    Q_ASSERT_X(parent != 0, "QToolBar", "parent cannot be zero");
    d->init();
}

#ifdef QT_COMPAT
/*! \obsolete
    Constructs a QToolBar with the given \a parent and \a name.
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

    QMainWindow *mainwindow = qt_cast<QMainWindow *>(parentWidget());
    Q_ASSERT(mainwindow != 0);
    QMainWindowLayout *mainwin_layout = qt_cast<QMainWindowLayout *>(mainwindow->layout());
    Q_ASSERT(mainwin_layout != 0);
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

    if (mainwindow->isVisible())
        mainwin_layout->relayout();
}

Qt::ToolBarArea QToolBar::area() const
{ return d->area; }

/*!
    Removes all actions from the tool bar.

    \sa removeAction()
*/
void QToolBar::clear()
{
    QList<QAction *> actions = this->actions();
    for(int i = 0; i < actions.size(); i++)
        removeAction(actions.at(i));
}

/*! \fn void QToolBar::addAction(QAction *action)

    Adds \a action to the end of the tool bar.

    \sa addAction()
*/

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

/*! \fn void QToolBar::insertAction(QAction *before, QAction *action)

    Inserts the given \a action into the tool bar in front of the tool
    bar item associated with the \a before action.

    \sa addAction()
*/

/*! \overload

    Creates a new action with the given \a text. This action is
    inserted into the tool bar in front of the tool bar item
    associated with the \a before action.
*/
QAction *QToolBar::insertAction(QAction *before, const QString &text)
{
    QAction *action = new QAction(text, this);
    insertAction(before, action);
    return action;
}

/*! \overload

    Creates a new action with the given \a icon and \a text. This
    action is inserted into the tool bar in front of the tool bar item
    associated with the \a before action.
*/
QAction *QToolBar::insertAction(QAction *before, const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    insertAction(before, action);
    return action;
}

/*! \overload

    Creates a new action with the given \a text. This action is inserted
    into the tool bar in front of the tool bar item associated with
    the \a before action. The action's \link QAction::triggered()
    triggered()\endlink signal is connected to the \a member in \a
    receiver.
*/
QAction *QToolBar::insertAction(QAction *before, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(before, action);
    return action;
}

/*! \overload

    Creates a new action with the given \a icon and \a text. This
    action is inserted into the tool bar in front of the tool bar item
    associated with the \a before action. The action's \link
    QAction::triggered() triggered()\endlink signal is connected to
    the \a member in the \a receiver.
*/
QAction *QToolBar::insertAction(QAction *before, const QIconSet &icon, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(before, action);
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
    Inserts a separator into the tool bar in front of the tool bar
    item associated with the \a before action.

    \sa addSeparator()
*/
QAction *QToolBar::insertSeparator(QAction *before)
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
    return action;
}

/*!
    Adds the given \a widget to the tool bar as the tool bar's last
    item.

    \sa insertWidget()
*/
QAction *QToolBar::addWidget(QWidget *widget)
{
    QToolBarWidgetAction *action = new QToolBarWidgetAction(widget, this);
    addAction(action);
    return action;
}

/*!
    Inserts the given \a widget in front of the tool bar item
    associated with the \a before action.

    \sa addWidget()
*/
QAction *QToolBar::insertWidget(QAction *before, QWidget *widget)
{
    QToolBarWidgetAction *action = new QToolBarWidgetAction(widget, this);
    insertAction(before, action);
    return action;
}

/*!
    \internal

    Returns the geometry of the tool bar item associated with the given
    \a action, or an invalid QRect if no matching item is found.
*/
QRect QToolBar::actionGeometry(QAction *action) const
{
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.action == action)
            return item.widget->geometry();
    }
    return QRect();
}

/*!
    \fn QAction *QToolBar::actionAt(const QPoint &p) const

    \overload

    Returns the action at point \a p.
*/

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

/*! \reimp */
void QToolBar::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();
    QToolBarWidgetAction *widgetAction = qt_cast<QToolBarWidgetAction *>(action);

    switch (event->type()) {
    case QEvent::ActionAdded:
        {
            if (d->ignoreActionAddedEvent)
                break;

            Q_ASSERT_X(!widgetAction || d->indexOf(widgetAction) == -1,
                       "QToolBar", "widgets cannot be inserted multiple times");

            QToolBarItem item = d->createItem(action);
            if (event->before()) {
                int index = d->indexOf(event->before());
                Q_ASSERT_X(index >= 0 && index < d->items.size(), "QToolBar::insertAction",
                           "internal error");
                d->items.insert(index, item);
                qt_cast<QBoxLayout *>(layout())->insertWidget(index + 1, item.widget);
            } else {
                d->items.append(item);
                qt_cast<QBoxLayout *>(layout())->insertWidget(d->items.size(), item.widget);
            }
            if (isShown())
                item.widget->show();
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
            int index = d->indexOf(action);
            Q_ASSERT_X(index >= 0 && index < d->items.size(),
                       "QToolBar::removeAction", "internal error");
            QToolBarItem item = d->items.takeAt(index);
            layout()->removeWidget(item.widget);
            if (!widgetAction) {
                // destroy the QToolBarButton/QToolBarSeparator
                delete item.widget;
            } else {
                if (isShown())
                    item.widget->hide();
            }
            break;
        }

    default:
	Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
    }
}

/*! \reimp */
void QToolBar::childEvent(QChildEvent *event)
{
    if (event->type() == QEvent::ChildRemoved) {
        QWidget *widget = qt_cast<QWidget *>(event->child());
        if (widget) {
            for (int i = 0; i < d->items.size(); ++i) {
                const QToolBarItem &item = d->items.at(i);
                QToolBarWidgetAction *widgetAction = 0;
                if (item.widget == widget
                    && (widgetAction = qt_cast<QToolBarWidgetAction *>(item.action))) {
                    removeAction(widgetAction);
                    // ### should we delete the action, or is it the programmers reponsibility?
                    // delete widgetAction;
                }
            }
        }
    }
    QFrame::childEvent(event);
}

/*! \reimp */
void QToolBar::resizeEvent(QResizeEvent *event)
{
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

            if (!qt_cast<QToolBarWidgetAction *>(item.action)) {
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

    QFrame::resizeEvent(event);
}

#include "moc_qtoolbar.cpp"
