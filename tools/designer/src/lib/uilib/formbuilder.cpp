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

#include "formbuilder.h"
#include <ui4.h>

#include <QtGui/QtGui>
#include <QtGui/QListWidget>
#include <QtGui/QComboBox>

FormBuilder::FormBuilder()
{
}

QWidget *FormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QList<DomAction*> actions = ui_widget->elementAction();
    for (int i=0; i<actions.size(); ++i) {
        QAction *action = new QAction(/*widget*/);
        applyProperties(action, actions.at(i)->elementProperty());
        m_actions.insert(actions.at(i)->attributeName(), action);
    }

    if (QWidget *w = Resource::create(ui_widget, parentWidget)) {
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


QWidget *FormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
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
        qWarning("widget `%s' not supported", widgetName.toLatin1().data());
    }

    if (qobject_cast<QDialog *>(w))
        w->setParent(parentWidget, 0);
    return w;
}

QLayout *FormBuilder::createLayout(const QString &layoutName, QObject *parent, const QString &name)
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
        qWarning("layout `%s' not supported", layoutName.toLatin1().data());
    }

    return l;
}

bool FormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return Resource::addItem(ui_item, item, layout);
}

bool FormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    return Resource::addItem(ui_widget, widget, parentWidget);
}

QWidget *FormBuilder::widgetByName(QWidget *topLevel, const QString &name)
{
    Q_ASSERT(topLevel);
    if (topLevel->objectName() == name)
        return topLevel;

    return qFindChild<QWidget*>(topLevel, name);
}

void FormBuilder::createConnections(DomConnections *ui_connections, QWidget *widget)
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

        QByteArray sig = c->elementSignal().toLatin1();
        sig.prepend("2");
        QByteArray sl = c->elementSlot().toLatin1();
        sl.prepend("1");

        QObject::connect(sender, sig, receiver, sl);
    }
}

QWidget *FormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
    return Resource::create(ui, parentWidget);
}

QLayout *FormBuilder::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    return Resource::create(ui_layout, layout, parentWidget);
}

QLayoutItem *FormBuilder::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    return Resource::create(ui_layoutItem, layout, parentWidget);
}

QAction *FormBuilder::create(DomAction *ui_action, QObject *parent)
{
    return Resource::create(ui_action, parent);
}

QActionGroup *FormBuilder::create(DomActionGroup *ui_action_group, QObject *parent)
{
    return Resource::create(ui_action_group, parent);
}

void FormBuilder::saveListWidgetExtraInfo(QListWidget *listWidget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_parentWidget);

    QList<DomItem*> ui_items = ui_widget->elementItem();

    for (int i=0; i<listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        DomItem *ui_item = new DomItem();

        QList<DomProperty*> properties;

        // text
        DomString *str = new DomString;
        str->setText(item->text());

        DomProperty *p = 0;

        p = new DomProperty;
        p->setAttributeName(QLatin1String("text"));
        p->setElementString(str);
        properties.append(p);

#if 0 // ### implement me
        p = new DomProperty;
        p->setAttributeName(QLatin1String("icon"));
        p->setElementIconSet();
        properties.append(p);
#endif

        ui_item->setElementProperty(properties);
        ui_items.append(ui_item);
    }

    ui_widget->setElementItem(ui_items);
}

void FormBuilder::saveComboBoxExtraInfo(QComboBox *comboBox, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    Q_UNUSED(ui_parentWidget);

    QList<DomItem*> ui_items = ui_widget->elementItem();

    for (int i=0; i<comboBox->count(); ++i) {
        DomItem *ui_item = new DomItem();

        QList<DomProperty*> properties;

        // text
        DomString *str = new DomString;
        str->setText(comboBox->itemText(i));

        DomProperty *p = 0;

        p = new DomProperty;
        p->setAttributeName(QLatin1String("text"));
        p->setElementString(str);
        properties.append(p);

#if 0 // ### implement me
        p = new DomProperty;
        p->setAttributeName(QLatin1String("icon"));
        p->setElementIconSet();
        properties.append(p);
#endif

        ui_item->setElementProperty(properties);
        ui_items.append(ui_item);
    }

    ui_widget->setElementItem(ui_items);
}

void FormBuilder::saveExtraInfo(QWidget *widget, DomWidget *ui_widget, DomWidget *ui_parentWidget)
{
    if (QListWidget *listWidget = qobject_cast<QListWidget*>(widget)) {
        saveListWidgetExtraInfo(listWidget, ui_widget, ui_parentWidget);
    } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
        saveComboBoxExtraInfo(comboBox, ui_widget, ui_parentWidget);
    }
}

void FormBuilder::loadListWidgetExtraInfo(DomWidget *ui_widget, QListWidget *listWidget, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    foreach (DomItem *ui_item, ui_widget->elementItem()) {
        QHash<QString, DomProperty*> properties = propertyMap(ui_item->elementProperty());
        QListWidgetItem *item = new QListWidgetItem(listWidget);

        DomProperty *p = 0;

        p = properties.value(QLatin1String("text"));
        if (p && p->kind() == DomProperty::String) {
            item->setText(p->elementString()->text());
        }

        p = properties.value(QLatin1String("icon"));
        if (p && p->kind() == DomProperty::IconSet) {
            // ### not implemented yet
        }
    }
}

void FormBuilder::loadComboBoxExtraInfo(DomWidget *ui_widget, QComboBox *comboBox, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget);

    foreach (DomItem *ui_item, ui_widget->elementItem()) {
        QHash<QString, DomProperty*> properties = propertyMap(ui_item->elementProperty());
        QString text;
        QIcon icon;

        DomProperty *p = 0;

        p = properties.value(QLatin1String("text"));
        if (p && p->kind() == DomProperty::String) {
            text = p->elementString()->text();
        }

        p = properties.value(QLatin1String("icon"));
        if (p && p->kind() == DomProperty::IconSet) {
            // ### not implemented yet
        }

        comboBox->addItem(text, icon);
    }
}


void FormBuilder::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (QListWidget *listWidget = qobject_cast<QListWidget*>(widget)) {
        loadListWidgetExtraInfo(ui_widget, listWidget, parentWidget);
    } else if (QComboBox *comboBox = qobject_cast<QComboBox*>(widget)) {
        loadComboBoxExtraInfo(ui_widget, comboBox, parentWidget);
    }
}
