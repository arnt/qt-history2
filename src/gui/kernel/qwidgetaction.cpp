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

#include "qwidgetaction.h"
#include "qdebug.h"

#ifndef QT_NO_ACTION
#include "qwidgetaction_p.h"

/*!
    \class QWidgetAction
    \brief The QWidgetAction class extends QAction by an interface
    for inserting custom widgets into action container widgets.
    
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
    QWidgetAction::doCreateWidget() is called. Reimplementations of that
    function should create a new custom widget with the specified parent.
    
    If the action is removed from a container widget then QWidgetAction::doRemoveWidget()
    is called with the previously created custom widget as argument. The default implementation
    hides the widget and deletes it using QObject::deleteLater().

    If you have only one single custom widget then you can use the convenience
    constructor of QWidgetAction that takes a QWidget as first argument. That
    widget will then be used if the action is added to a QToolBar, or in general
    to an action container that supports QWidgetAction. If such a constructed
    QWidgetAction is added to two toolbars at the same time then the specified
    widget is shown only in the first toolbar the action was added to.
    QWidgetAction takes over ownership of the specified widget.

    \ingroup application
    \mainclass
*/

/*!
    Constructs an action with \a parent. This constructor can only be called
    by subclasses which reimplement doCreateWidget().
    
    \sa doCreateWidget()
*/
QWidgetAction::QWidgetAction(QObject* parent)
    : QAction(*(new QWidgetActionPrivate), parent)
{
}

/*!
    Convenience constructor that constructs an action with \a parent and uses
    the specified \a widget when the action is added to a container widget that
    supports custom widgets, such as QToolBar. The ownership of \a widget is
    transferred to QWidgetAction.
*/
QWidgetAction::QWidgetAction(QWidget *widget, QObject* parent)
    : QAction(*(new QWidgetActionPrivate), parent)
{
    Q_D(QWidgetAction);
    setVisible(!(widget->isHidden() && widget->testAttribute(Qt::WA_WState_ExplicitShowHide)));
    d->widget = widget;
    d->widget->hide();
    d->widget->setParent(0);
}

/*!
    Destroys the object and frees allocated resources.
*/
QWidgetAction::~QWidgetAction()
{
    Q_D(QWidgetAction);
    for (int i = 0; i < d->allWidgets.count(); ++i)
        disconnect(d->allWidgets.at(i), SIGNAL(destroyed(QObject *)),
                   this, SLOT(_q_widgetDestroyed(QObject *)));
    QList<QWidget *> widgetsToDelete = d->allWidgets;
    d->allWidgets.clear();
    qDeleteAll(widgetsToDelete);
    delete d->widget;
}

/*!
  \reimp
*/
bool QWidgetAction::event(QEvent *e)
{
    return QAction::event(e);
}

QWidget *QWidgetAction::createWidget(QWidget *parent)
{
    Q_D(QWidgetAction);

    QWidget *w = doCreateWidget(parent);
    if (!w) return w;

    connect(w, SIGNAL(destroyed(QObject *)),
            this, SLOT(_q_widgetDestroyed(QObject *)));
    d->allWidgets.append(w);
    return w;
}

void QWidgetAction::removeWidget(QWidget *w)
{
    Q_D(QWidgetAction);
    if (!d->allWidgets.contains(w))
        return;
    disconnect(w, SIGNAL(destroyed(QObject *)),
            this, SLOT(_q_widgetDestroyed(QObject *)));
    d->allWidgets.removeAll(w);
    doRemoveWidget(w);
}

QList<QWidget *> QWidgetAction::widgets() const
{
    Q_D(const QWidgetAction);
    return d->allWidgets;
}

/*!
    This function is called whenever the action is added to a container widget
    that supports custom widgets. If you don't want a custom widget to be
    used as representation of the action in the specified \a parent widget then
    0 should be returned.
*/
QWidget *QWidgetAction::doCreateWidget(QWidget *parent)
{
    Q_D(QWidgetAction);
    if (d->widgetInUse || !d->widget)
        return 0;
    d->widget->setParent(parent);
    d->widgetInUse = true;
    return d->widget;
}

/*!
    This function is called whenever the action is removed from a container
    widget that displays the action using a custom widget previously created
    using doCreateWidget(). The default implementation hides and deletes the
    widget, unless the action was constructed using the QActionWidget
    convenience constructor that takes an instance of a existing custom widget.
*/
void QWidgetAction::doRemoveWidget(QWidget *widget)
{
    Q_D(QWidgetAction);
    if (widget == d->widget) {
        d->widget->hide();
        d->widget->setParent(0);
        d->widgetInUse = false;
    } else {
        widget->hide();
        widget->deleteLater();
    }
}

#include "moc_qwidgetaction.cpp"

#endif // QT_NO_ACTION

