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

#include <QtDesigner/formbuilder.h>
#include <QtDesigner/ui4.h>

#include <QtGui/QtGui>

QFormBuilder::QFormBuilder()
{
}

QWidget *QFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QList<DomAction*> actions = ui_widget->elementAction();
    for (int i=0; i<actions.size(); ++i) {
        QAction *action = new QAction(/*widget*/);
        applyProperties(action, actions.at(i)->elementProperty());
        m_actions.insert(actions.at(i)->attributeName(), action);
    }

    if (QWidget *w = QAbstractFormBuilder::create(ui_widget, parentWidget)) {
        //if (QMenu *menu = qobject_cast<QMenu*>(w)) {
            QList<DomActionRef*> refs = ui_widget->elementAddAction();
            for (int i=0; i<refs.size(); ++i) {
                if (QAction *a = m_actions.value(refs.at(i)->attributeName()))
                    w->addAction(a);
            }
        //}
        return w;
    }

    return 0;
}


QWidget *QFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    QWidget *w = 0;

    if (qobject_cast<QTabWidget*>(parentWidget)
            || qobject_cast<QStackedWidget*>(parentWidget)
            || qobject_cast<QToolBox*>(parentWidget))
        parentWidget = 0;

    if (widgetName == QLatin1String("Line"))
        w = new QFrame(parentWidget);
    else if (widgetName == QLatin1String("QToolBar"))
        w = new QToolBar(qobject_cast<QMainWindow*>(parentWidget));

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) /*DECLARE_WIDGET(W, C)*/
#define DECLARE_WIDGET(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET

    if (w) {
        w->setObjectName(name);
    } else {
        qWarning("widget `%s' not supported", widgetName.toUtf8().data());
    }

    if (qobject_cast<QDialog *>(w))
        w->setParent(parentWidget, 0);
    return w;
}

QLayout *QFormBuilder::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    QLayout *l = 0;

    QWidget *parentWidget = qobject_cast<QWidget*>(parent);
    QLayout *parentLayout = qobject_cast<QLayout*>(parent);

    Q_ASSERT(parentWidget || parentLayout);

#define DECLARE_WIDGET(W, C)
#define DECLARE_COMPAT_WIDGET(W, C)

#define DECLARE_LAYOUT(L, C) \
    if (layoutName == QLatin1String(#L)) { \
        Q_ASSERT(l == 0); \
        l = parentLayout \
            ? new L() \
            : new L(parentWidget); \
    }

#include "widgets.table"

#undef DECLARE_LAYOUT
#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_WIDGET

    if (l) {
        l->setObjectName(name);
    } else {
        qWarning("layout `%s' not supported", layoutName.toUtf8().data());
    }

    return l;
}

bool QFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return QAbstractFormBuilder::addItem(ui_item, item, layout);
}

bool QFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    return QAbstractFormBuilder::addItem(ui_widget, widget, parentWidget);
}

QWidget *QFormBuilder::widgetByName(QWidget *topLevel, const QString &name)
{
    Q_ASSERT(topLevel);
    if (topLevel->objectName() == name)
        return topLevel;

    return qFindChild<QWidget*>(topLevel, name);
}

void QFormBuilder::createConnections(DomConnections *ui_connections, QWidget *widget)
{
    Q_ASSERT(widget != 0);

    if (ui_connections == 0)
        return;

    QList<DomConnection*> connections = ui_connections->elementConnection();
    foreach (DomConnection *c, connections) {
        QWidget *sender = widgetByName(widget, c->elementSender());
        QWidget *receiver = widgetByName(widget, c->elementReceiver());
        if (!sender || !receiver)
            continue;

        QByteArray sig = c->elementSignal().toUtf8();
        sig.prepend("2");
        QByteArray sl = c->elementSlot().toUtf8();
        sl.prepend("1");

        QObject::connect(sender, sig, receiver, sl);
    }
}

QWidget *QFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui, parentWidget);
}

QLayout *QFormBuilder::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui_layout, layout, parentWidget);
}

QLayoutItem *QFormBuilder::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui_layoutItem, layout, parentWidget);
}

QAction *QFormBuilder::create(DomAction *ui_action, QObject *parent)
{
    return QAbstractFormBuilder::create(ui_action, parent);
}

QActionGroup *QFormBuilder::create(DomActionGroup *ui_action_group, QObject *parent)
{
    return QAbstractFormBuilder::create(ui_action_group, parent);
}

