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
#include "qtoolbarextension_p.h"
#include "qtoolbarhandle_p.h"
#include "qtoolbarseparator_p.h"

static QStyleOptionFrame getStyleOption(QToolBar *tb)
{
    QStyleOptionFrame opt;
    opt.init(tb);
    if (tb->orientation() == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    opt.lineWidth = tb->style()->pixelMetric(QStyle::PM_ToolBarFrameWidth);
    return opt;
}

/*
    QToolBarPrivate
*/

void QToolBarPrivate::init()
{
    Q_Q(QToolBar);
    movable = true;
    q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));


    QStyleOptionFrame opt = getStyleOption(q);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, q);
    QStyle *style = q->style();

    int e = style->pixelMetric(QStyle::PM_ToolBarIconSize);
    iconSize = QSize(e, e);

    layout->setAlignment(Qt::AlignLeft);
    layout->setMargin(style->pixelMetric(QStyle::PM_ToolBarFrameWidth, &opt, q)
                      + style->pixelMetric(QStyle::PM_ToolBarItemMargin, &opt, q));
    layout->setSpacing(style->pixelMetric(QStyle::PM_ToolBarItemSpacing, &opt, q));

    handle = new QToolBarHandle(q);
    QObject::connect(q, SIGNAL(orientationChanged(Orientation)),
                     handle, SLOT(setOrientation(Orientation)));
    layout->addWidget(handle);
    handle->setVisible(movable && (qobject_cast<QMainWindow *>(q->parentWidget()) != 0));

    extension = new QToolBarExtension(q);
    QObject::connect(q, SIGNAL(orientationChanged(Orientation)),
                     extension, SLOT(setOrientation(Orientation)));
    extension->setFocusPolicy(Qt::NoFocus);
    extension->hide();

#ifdef Q_WS_MAC
    if (q->parentWidget()) {
        // Make sure that the window has the "toolbar" button.
        extern WindowPtr qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
        ChangeWindowAttributes(qt_mac_window_for(q->parentWidget()), kWindowToolbarButtonAttribute,
                               kWindowNoAttributes);
    }
#endif

    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(toggleView(bool)));
}

void QToolBarPrivate::toggleView(bool b)
{
    Q_Q(QToolBar);
    if (b == q->isHidden()) {
        if (b)
            q->show();
        else
            q->close();
    }
}

void QToolBarPrivate::updateIconSize(const QSize &sz)
{
    Q_Q(QToolBar);
    if (!explicitIconSize) {
        // iconSize not explicitly set
        q->setIconSize(sz);
        explicitIconSize = false;
    }
}

void QToolBarPrivate::updateToolButtonStyle(Qt::ToolButtonStyle style)
{
    Q_Q(QToolBar);
    if (!explicitToolButtonStyle) {
        q->setToolButtonStyle(style);
        explicitToolButtonStyle = false;
    }
}

QToolBarItem QToolBarPrivate::createItem(QAction *action)
{
    Q_Q(QToolBar);
    QToolBarItem item;
    item.action = action;
    item.hidden = false;

    QToolBarWidgetAction *widgetAction = qobject_cast<QToolBarWidgetAction *>(action);
    if (widgetAction) {
        item.widget = widgetAction->widget();
    } else if (action->isSeparator()) {
        item.widget = new QToolBarSeparator(q);
        QObject::connect(q, SIGNAL(orientationChanged(Orientation)),
                         item.widget, SLOT(setOrientation(Orientation)));
    } else {
        QToolButton *button = new QToolButton(q);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIconSize(iconSize);
        button->setToolButtonStyle(toolButtonStyle);
        QObject::connect(q, SIGNAL(iconSizeChanged(QSize)),
                         button, SLOT(setIconSize(QSize)));
        QObject::connect(q, SIGNAL(toolButtonStyleChanged(ToolButtonStyle)),
                         button, SLOT(setToolButtonStyle(ToolButtonStyle)));
        button->setDefaultAction(action);
        QObject::connect(button, SIGNAL(triggered(QAction*)), q, SIGNAL(actionTriggered(QAction*)));
        if (action->menu())
            button->setPopupMode(QToolButton::MenuButtonPopup);
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
    for (int i = 0; i < items.size(); ++i) {
        const QToolBarItem &item = items.at(i);
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

    A toolbar can be fixed in place in a particular area() (e.g. at
    the top of the window), or it can be movable() between toolbar
    areas: see \l{allowedAreas} and isAreaAllowed().

*/

/*!
    \fn bool QToolBar::isAreaAllowed(Qt::ToolBarArea area) const

    Returns true if this toolbar is dockable in the given \a area;
    otherwise returns false.
*/

/*!
    \fn void QToolBar::actionTriggered(QAction *action)

    This signal is emitted when a toolbar button is pressed. The
    parameter holds the toolbar button's associated \a action.
*/

/*!
    \fn void QToolBar::allowedAreasChanged(Qt::ToolBarAreas allowedAreas)

    This signal is emitted when the collection of allowed areas for the
    toolbar is changed. The new areas in which the toolbar can be positioned
    are specified by \a allowedAreas.

    \sa QToolBar::allowedAreas
*/

/*!
    \fn void QToolBar::iconSizeChanged(const QSize &iconSize)

    This signal is emitted when the icon size is changed.  The \a
    iconSize parameter holds the toolbar's new icon size.

    \sa QToolBar::iconSize QMainWindow::iconSize
*/

/*!
    \fn void QToolBar::movableChanged(bool movable)

    This signal is emitted when the toolbar becomes movable or fixed.
    If the toolbar can be moved, \a movable is true; otherwise it is
    false.

    \sa QToolBar::movable
*/

/*!
    \fn void QToolBar::orientationChanged(Qt::Orientation orientation)

    This signal is emitted when the orientation of the toolbar changes.
    The new orientation is specified by the \a orientation given.

    \sa QToolBar::orientation
*/

/*!
    \fn void QToolBar::toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle)

    This signal is emitted when the tool button style is changed. The
    \a toolButtonStyle parameter holds the toolbar's new tool button
    style.

    \sa QToolBar::toolButtonStyle QMainWindow::toolButtonStyle
*/

/*!
    Constructs a QToolBar with the given \a parent.
*/
QToolBar::QToolBar(QWidget *parent)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
}

/*!
    Constructs a QToolBar with the given \a parent.

    The window title is set to \a title. This title is used when the
    toolbar is floating as an independent window. It is also shown
    in the context menu provided by QMainWindow.

    \sa setWindowTitle()
*/
QToolBar::QToolBar(const QString &title, QWidget *parent)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
    setWindowTitle(title);
}

#ifdef QT3_SUPPORT
/*! \obsolete
    Constructs a QToolBar with the given \a parent and \a name.
*/
QToolBar::QToolBar(QWidget *parent, const char *name)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
    setObjectName(name);
}
#endif

/*!
    Destroys the toolbar.
*/
QToolBar::~QToolBar()
{
    // Remove the toolbar button if there is nothing left.
    QMainWindow *mainwindow = qobject_cast<QMainWindow *>(parentWidget());
    if (mainwindow) {
        QMainWindowLayout *mainwin_layout = qobject_cast<QMainWindowLayout *>(mainwindow->layout());
        mainwin_layout->removeToolBarInfo(this);
        mainwin_layout->relayout();
#ifdef Q_WS_MAC
        if (mainwin_layout && mainwin_layout->tb_layout_info.isEmpty())
            ChangeWindowAttributes(qt_mac_window_for(mainwindow), kWindowNoAttributes,
                                   kWindowToolbarButtonAttribute);
#endif
    }
}

/*! \property QToolBar::movable
    \brief whether the user can move the toolbar within the toolbar area,
    or between toolbar areas

    By default, this property is true.

    This property only makes sense if the toolbar is in a
    QMainWindow.

    \sa allowedAreas
*/

void QToolBar::setMovable(bool movable)
{
    Q_D(QToolBar);
    if (!movable == !d->movable)
        return;
    d->movable = movable;
    d->handle->setVisible(d->movable && (qobject_cast<QMainWindow *>(parentWidget()) != 0));
    emit movableChanged(d->movable);
}

bool QToolBar::isMovable() const
{ Q_D(const QToolBar); return d->movable; }

/*!
    \property QToolBar::allowedAreas
    \brief areas where the toolbar may be placed

    The default is \c Qt::AllToolBarAreas.

    This property only makes sense if the toolbar is in a
    QMainWindow.

    \sa movable
*/

void QToolBar::setAllowedAreas(Qt::ToolBarAreas areas)
{
    Q_D(QToolBar);
    areas &= Qt::ToolBarArea_Mask;
    if (areas == d->allowedAreas)
        return;
    d->allowedAreas = areas;
    emit allowedAreasChanged(d->allowedAreas);
}

Qt::ToolBarAreas QToolBar::allowedAreas() const
{ Q_D(const QToolBar); return d->allowedAreas; }

/*! \property QToolBar::orientation
    \brief orientation of the toolbar

    The default is \c Qt::Horizontal.

    The orientation is updated automatically when the toolbar is
    managed by QMainWindow.
*/

void QToolBar::setOrientation(Qt::Orientation orientation)
{
    Q_D(QToolBar);
    if (orientation == d->orientation)
        return;

    d->orientation = orientation;

    QBoxLayout *box = qobject_cast<QBoxLayout *>(layout());
    Q_ASSERT_X(box != 0, "QToolBar::setOrientation", "internal error");

    switch (d->orientation) {
    case Qt::Vertical:
	box->setDirection(QBoxLayout::TopToBottom);
        box->setAlignment(Qt::AlignTop);
 	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	break;

    case Qt::Horizontal:
	box->setDirection(QBoxLayout::LeftToRight);
        box->setAlignment(Qt::AlignLeft);
 	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	break;
    }

    emit orientationChanged(d->orientation);
}

Qt::Orientation QToolBar::orientation() const
{ Q_D(const QToolBar); return d->orientation; }

/*! \property QToolBar::iconSize
    \brief size of icons in the toolbar.

    The default is Qt::AutomaticIconSize.
*/

QSize QToolBar::iconSize() const
{ Q_D(const QToolBar); return d->iconSize; }

void QToolBar::setIconSize(const QSize &iconSize)
{
    Q_D(QToolBar);
    QSize sz = iconSize;
    if (!sz.isValid()) {
        QMainWindow *mw = qobject_cast<QMainWindow *>(parentWidget());
        if (mw && mw->layout()) {
            QLayout *layout = mw->layout();
            int i = 0;
            QLayoutItem *item = 0;
            do {
                item = layout->itemAt(i++);
                if (item->widget() == this)
                    sz = mw->iconSize();
            } while (!sz.isValid() && item != 0);
        }
    }
    if (!sz.isValid()) {
        const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        sz = QSize(metric, metric);
    }
    if (d->iconSize != sz) {
        d->iconSize = sz;
        emit iconSizeChanged(d->iconSize);
    }
    d->explicitIconSize = iconSize.isValid();
}

/*! \property QToolBar::toolButtonStyle
    \brief style of toolbar buttons.

    The defaults is Qt::ToolButtonIconOnly.
*/

Qt::ToolButtonStyle QToolBar::toolButtonStyle() const
{ Q_D(const QToolBar); return d->toolButtonStyle; }

void QToolBar::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
    Q_D(QToolBar);
    d->explicitToolButtonStyle = true;
    if (d->toolButtonStyle == toolButtonStyle)
        return;
    d->toolButtonStyle = toolButtonStyle;
    emit toolButtonStyleChanged(d->toolButtonStyle);
    layout()->invalidate(); // ### See 72944
}

/*!
    Removes all actions from the toolbar.

    \sa removeAction()
*/
void QToolBar::clear()
{
    QList<QAction *> actions = this->actions();
    for(int i = 0; i < actions.size(); i++)
        removeAction(actions.at(i));
}

/*! \fn void QToolBar::addAction(QAction *action)

    Adds \a action to the end of the toolbar.

    \sa addAction()
*/

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar.
*/
QAction *QToolBar::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a icon and \a text. This
    action is added to the end of the toolbar.
*/
QAction *QToolBar::addAction(const QIcon &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar. The action's \link QAction::triggered()
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

/*!
    \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is added to the end of the toolbar. The action's \link
    QAction::triggered() triggered()\endlink signal is connected to \a
    member in \a receiver.
*/
QAction *QToolBar::addAction(const QIcon &icon, const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
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
    Inserts a separator into the toolbar in front of the toolbar
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
    Adds the given \a widget to the toolbar as the toolbar's last
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
    Inserts the given \a widget in front of the toolbar item
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

    Returns the geometry of the toolbar item associated with the given
    \a action, or an invalid QRect if no matching item is found.
*/
QRect QToolBar::actionGeometry(QAction *action) const
{
    Q_D(const QToolBar);
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.action == action)
            return item.widget->geometry();
    }
    return QRect();
}

/*!
    Returns the action at point \a p. This function returns zero if no
    action was found.

    \sa QWidget::childAt()
*/
QAction *QToolBar::actionAt(const QPoint &p) const
{
    Q_D(const QToolBar);
    QWidget *widget = childAt(p);
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.widget == widget)
            return item.action;
    }
    return 0;
}

/*! \fn QAction *QToolBar::actionAt(int x, int y) const
    \overload

    Returns the action at the point \a x, \a y. This function returns
    zero if no action was found.
*/

/*! \reimp */
void QToolBar::actionEvent(QActionEvent *event)
{
    Q_D(QToolBar);
    QAction *action = event->action();
    QToolBarWidgetAction *widgetAction = qobject_cast<QToolBarWidgetAction *>(action);

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
                qobject_cast<QBoxLayout *>(layout())->insertWidget(index + 1, item.widget);
            } else {
                d->items.append(item);
                qobject_cast<QBoxLayout *>(layout())->insertWidget(d->items.size(), item.widget);
            }
            item.widget->setVisible(item.action->isVisible());
            break;
        }

    case QEvent::ActionChanged:
        {
            int index = d->indexOf(action);
            Q_ASSERT_X(index >= 0 && index < d->items.size(),
                       "QToolBar::actionEvent", "internal error");
            const QToolBarItem &item = d->items.at(index);
            if (!item.hidden)
                item.widget->setVisible(item.action->isVisible());

            break;
        }

    case QEvent::ActionRemoved:
        {
            int index = d->indexOf(action);
            Q_ASSERT_X(index >= 0 && index < d->items.size(),
                       "QToolBar::removeAction", "internal error");
            QToolBarItem item = d->items.takeAt(index);
            layout()->removeWidget(item.widget);
            if (!widgetAction) {
                // destroy the QToolButton/QToolBarSeparator
                delete item.widget;
            } else {
                if (!isHidden())
                    item.widget->hide();
            }
            break;
        }

    default:
	Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
    }
}

/*! \reimp */
void QToolBar::changeEvent(QEvent *event)
{
    Q_D(QToolBar);
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        d->toggleViewAction->setText(windowTitle());
        break;
    case QEvent::StyleChange:
        {
            QStyleOptionFrame opt = getStyleOption(this);
            d->layout->setMargin(style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, &opt, this)
                                 + style()->pixelMetric(QStyle::PM_ToolBarItemMargin, &opt, this));
            d->layout->setSpacing(style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, &opt, this));
            break;
        }
    default:
        break;
    }
}

/*! \reimp */
void QToolBar::childEvent(QChildEvent *event)
{
    Q_D(QToolBar);
    QWidget *widget = qobject_cast<QWidget *>(event->child());
    if (widget) {
#if !defined(QT_NO_DEBUG)
        if (!widget->isWindow() && event->type() == QEvent::ChildPolished) {
            bool found = (d->handle == widget || d->extension == widget);
            for (int i = 0; !found && i < d->items.size(); ++i) {
                const QToolBarItem &item = d->items.at(i);
                if (item.widget == widget)
                    found = true;
            }
            if (!found)
                qWarning("QToolBar: child widget '%s::%s' not added, use QToolBar::addWidget()",
                         widget->objectName().toLocal8Bit().constData(), widget->metaObject()->className());
        } else
#endif
        if (event->type() == QEvent::ChildRemoved) {
            for (int i = 0; i < d->items.size(); ++i) {
                const QToolBarItem &item = d->items.at(i);
                QToolBarWidgetAction *widgetAction = 0;
                if (item.widget == widget
                    && (widgetAction = qobject_cast<QToolBarWidgetAction *>(item.action))) {
                    removeAction(widgetAction);
                    // ### should we delete the action, or is it the programmers reponsibility?
                    // delete widgetAction;
                }
            }
        }
    }
    QWidget::childEvent(event);
}

/*! \reimp */
void QToolBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    QStyleOptionFrame opt = getStyleOption(this);
    style()->drawPrimitive(QStyle::PE_PanelToolBar, &opt, &p, this);
}

/*! \reimp */
void QToolBar::resizeEvent(QResizeEvent *event)
{
    Q_D(QToolBar);
    QBoxLayout *box = qobject_cast<QBoxLayout *>(layout());
    Qt::Orientation orientation = (box->direction() == QBoxLayout::LeftToRight
                                   || box->direction() == QBoxLayout::RightToLeft)
                                  ? Qt::Horizontal
                                  : Qt::Vertical;
    const int margin = box->margin();
    int i = 0;
    int extension_size = 0;
    int hidden_count = 0;
    int max_item_extent = 0;
    i = d->items.size(); // note: the toolbar handle is not counted
    while (i > 0) {
	QWidget *w = box->itemAt(i)->widget();
	bool hide = false;
	if (QApplication::layoutDirection() == Qt::RightToLeft && orientation == Qt::Horizontal) {
            if (w->isHidden()) {
                if (box->itemAt(i-1) && !box->itemAt(i-1)->widget()->isHidden()) {
                    QWidget *pw = box->itemAt(i-1)->widget();
                    hide = pw->pos().x() < (extension_size + w->size().width() + margin + box->spacing());
                } else {
                    // calculate the pos of the hidden item
                    int pos = 0;
                    for (int k = 1; k < i; ++k) { // idx 0 == handle
                        QWidget * pw = box->itemAt(k)->widget();
                        if (pw == w)
                            break;
                        pos = pw->isHidden()
                              ? pos - pw->size().width() - box->spacing()
                              : pw->pos().x();
                    }
                    pos = pos - w->size().width() - box->spacing();
                    hide = pos < extension_size + margin;
                }
            } else {
                hide = w->pos().x() < extension_size + margin;
            }
	} else {
	    hide = pick(orientation, w->pos()) + pick(orientation, w->size())
		   >= pick(orientation, size()) - extension_size;
	}
	if (hide && i > 1) { // never hide the first item in the tb
	    w->hide();
	    d->items[i - 1].hidden = true;
	    ++hidden_count;
	    // the size of the extension menu button needs to be
	    // considered when buttons in the toolbar are hidden
	    extension_size = pick(orientation, d->extension->sizeHint());
	} else {
            w->setVisible(d->items[i - 1].action->isVisible());
            d->items[i - 1].hidden = false;
            if (orientation == Qt::Horizontal)
                max_item_extent = qMax(max_item_extent, w->sizeHint().height());
            else
                max_item_extent = qMax(max_item_extent, w->sizeHint().width());
	}
	--i;
    }

    if (orientation == Qt::Horizontal) {
        setMinimumSize(d->handle->sizeHint().width() + box->spacing()*2 + extension_size + margin*2
                       + (d->items.isEmpty()
                          ? d->iconSize.width()
                          : d->items[0].widget->sizeHint().width()),
                       max_item_extent + margin*2);
    } else {
        setMinimumSize(max_item_extent + margin*2,
                       d->handle->sizeHint().height() + box->spacing()*2 + extension_size
                       + margin*2 + (d->items.isEmpty()
                                     ? d->iconSize.height()
                                     : d->items[0].widget->sizeHint().height()));
    }

    if (hidden_count > 0) {
	if (orientation == Qt::Horizontal) {
	    int x = QApplication::layoutDirection() == Qt::RightToLeft
		    ? margin
		    : width() - d->extension->sizeHint().width() - margin;
	    d->extension->setGeometry(x, margin, d->extension->sizeHint().width(), max_item_extent);
        } else {
	    d->extension->setGeometry(margin,
				      height() - d->extension->sizeHint().height() - margin,
				      max_item_extent,
				      d->extension->sizeHint().height());
        }

	QMenu *pop = d->extension->menu();
	if (!pop) {
	    pop = new QMenu(this);
	    d->extension->setMenu(pop);
	}
	pop->clear();
	for(int i = 0; i < d->items.size(); ++i) {
            const QToolBarItem &item = d->items.at(i);
            if (!item.hidden) continue;

            if (!qobject_cast<QToolBarWidgetAction *>(item.action)) {
                pop->addAction(item.action);
            } else {
                // ### needs special handling of custom widgets and
                // ### e.g. combo boxes - only actions are supported in
                // ### the preview
            }
        }
        if (pop->actions().size() > 0) {
            d->extension->show();
            d->extension->setEnabled(true);
        } else {
            // show a disabled ext btn in the case where widgets in
            // the toolbar are hidden but not put into the ext menu -
            // this indicates that some items in the tb is hidden
            d->extension->show();
            d->extension->setEnabled(false);
        }
    } else if (!d->extension->isHidden()) {
	if (d->extension->menu())
	    d->extension->menu()->clear();
	d->extension->hide();
    }
    QWidget::resizeEvent(event);
}

/*! \reimp */
bool QToolBar::event(QEvent *event)
{
    Q_D(QToolBar);
    switch (event->type()) {
    case QEvent::Hide:
        if (!isHidden())
            break;
        // fallthrough intended
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
    case QEvent::ParentChange:
        d->handle->setVisible(d->movable && (qobject_cast<QMainWindow *>(parentWidget()) != 0));
        break;
    case QEvent::StyleChange:
        if (!d->explicitIconSize)
            setIconSize(QSize());
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

/*!
    Returns a checkable action that can be used to show or hide this
    toolbar.

    The action's text is set to the toolbar's window title.

    \sa QAction::text QWidget::windowTitle
*/
QAction *QToolBar::toggleViewAction() const
{ Q_D(const QToolBar); return d->toggleViewAction; }

/*!
    \fn void QToolBar::setLabel(const QString &label)

    Use setWindowTitle() instead.
*/

/*!
    \fn QString QToolBar::label() const

    Use windowTitle() instead.
*/


#include "moc_qtoolbar.cpp"
