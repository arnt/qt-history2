/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwidgetaction.h"
#include "qdebug.h"

#ifndef QT_NO_ACTION
#include "qwidgetaction_p.h"

/*!
    \class QWidgetAction
    \since 4.2
    \brief The QWidgetAction class extends QAction by an interface
    for inserting custom widgets into action based containers, such
    as toolbars.
    
    Most actions in application are represented as items in menus or
    buttons in toolbars. However sometimes more complex widgets are
    necessary. For example a zoom action in a word processor may be
    realized using a QComboBox in a QToolBar, presenting a range
    of different zoom levels. QToolBar provides QToolBar::insertWidget()
    as convenience function for inserting a single widget.
    However if you want to implement an action that uses custom
    widgets for visualization in multiple containers then you have to
    subclass QWidgetAction.

    If a QWidgetAction is added for example to a QToolBar then
    QWidgetAction::createWidget() is called. Reimplementations of that
    function should create a new custom widget with the specified parent.
    
    If the action is removed from a container widget then QWidgetAction::deleteWidget()
    is called with the previously created custom widget as argument. The default implementation
    hides the widget and deletes it using QObject::deleteLater().

    If you have only one single custom widget then you can set it as default
    widget using setDefaultWidget(). That widget will then be used if the
    action is added to a QToolBar, or in general to an action container that
    supports QWidgetAction. If a QWidgetAction with only a default widget is
    added to two toolbars at the same time then the default widget is shown
    only in the first toolbar the action was added to. QWidgetAction takes
    over ownership of the default widget.
    
    Currently in Qt only QToolBar supports QWidgetAction.

    \ingroup application
    \mainclass
*/

/*!
    Constructs an action with \a parent.
*/
QWidgetAction::QWidgetAction(QObject* parent)
    : QAction(*(new QWidgetActionPrivate), parent)
{
}

/*!
    Destroys the object and frees allocated resources.
*/
QWidgetAction::~QWidgetAction()
{
    Q_D(QWidgetAction);
    for (int i = 0; i < d->createdWidgets.count(); ++i)
        disconnect(d->createdWidgets.at(i), SIGNAL(destroyed(QObject *)),
                   this, SLOT(_q_widgetDestroyed(QObject *)));
    QList<QWidget *> widgetsToDelete = d->createdWidgets;
    d->createdWidgets.clear();
    qDeleteAll(widgetsToDelete);
    delete d->defaultWidget;
}

/*!
    Sets the default widget. The ownership is transferred to QWidgetAction.
    Unless createWidget() is re-implemented by a subclass to return a
    new widget the default widget is used when a container widget requests
    a widget through requestWidget().
*/
void QWidgetAction::setDefaultWidget(QWidget *w)
{
    Q_D(QWidgetAction);
    if (w == d->defaultWidget || d->defaultWidgetInUse)
        return;
    delete d->defaultWidget;
    d->defaultWidget = w;
    if (!w)
        return;

    setVisible(!(w->isHidden() && w->testAttribute(Qt::WA_WState_ExplicitShowHide)));
    d->defaultWidget->hide();
    d->defaultWidget->setParent(0);
    d->defaultWidgetInUse = false;
}

/*!
    Returns the default widget.
*/
QWidget *QWidgetAction::defaultWidget() const
{
    Q_D(const QWidgetAction);
    return d->defaultWidget;
}

/*!
    Container widgets that support actions call this function to request a
    widget as visual representation of the action.
*/
QWidget *QWidgetAction::requestWidget(QWidget *parent)
{
    Q_D(QWidgetAction);

    QWidget *w = createWidget(parent);
    if (!w) {
        if (d->defaultWidgetInUse || !d->defaultWidget)
            return 0;
        d->defaultWidget->setParent(parent);
        d->defaultWidgetInUse = true;
        return d->defaultWidget;
    }

    connect(w, SIGNAL(destroyed(QObject *)),
            this, SLOT(_q_widgetDestroyed(QObject *)));
    d->createdWidgets.append(w);
    return w;
}

/*!
    \fn void QWidgetAction::releaseWidget(QWidget *widget)

    Releases the specified \a widget.

    Container widgets that support actions call this function when a widget
    action is removed.
*/
void QWidgetAction::releaseWidget(QWidget *w)
{
    Q_D(QWidgetAction);
    
    if (w == d->defaultWidget) {
        d->defaultWidget->hide();
        d->defaultWidget->setParent(0);
        d->defaultWidgetInUse = false;
        return;
    }

    if (!d->createdWidgets.contains(w))
        return;
    
    disconnect(w, SIGNAL(destroyed(QObject *)),
               this, SLOT(_q_widgetDestroyed(QObject *)));
    d->createdWidgets.removeAll(w);
    deleteWidget(w);
}

/*!
  \reimp
*/
bool QWidgetAction::event(QEvent *e)
{
    return QAction::event(e);
}

/*!
    This function is called whenever the action is added to a container widget
    that supports custom widgets. If you don't want a custom widget to be
    used as representation of the action in the specified \a parent widget then
    0 should be returned.
*/
QWidget *QWidgetAction::createWidget(QWidget *parent)
{
    Q_UNUSED(parent)
    return 0;
}

/*!
    This function is called whenever the action is removed from a container
    widget that displays the action using a custom widget previously created
    using createWidget(). The default implementation hides the widget and
    schedules it for deletion using QObject::deleteLater().
*/
void QWidgetAction::deleteWidget(QWidget *widget)
{
    widget->hide();
    widget->deleteLater();
}

/*!
    Returns the list of widgets that have been using createWidget() and
    are currently in use by widgets the action has been added to.
*/
QList<QWidget *> QWidgetAction::createdWidgets() const
{
    Q_D(const QWidgetAction);
    return d->createdWidgets;
}

#include "moc_qwidgetaction.cpp"

#endif // QT_NO_ACTION

