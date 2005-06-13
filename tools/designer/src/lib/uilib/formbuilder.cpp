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

#include <QtDesigner/QtDesigner>
#include <QtGui/QtGui>

/*!
    \class QFormBuilder
    \inmodule QtDesigner
*/

/*!
    \fn QFormBuilder::QFormBuilder()
*/

QFormBuilder::QFormBuilder()
{
}

QWidget *QFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui_widget, parentWidget);
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

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) /*DECLARE_WIDGET(W, C)*/
#define DECLARE_WIDGET(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET

    if (w == 0) { // try with a registered custom widget
        QDesignerCustomWidgetInterface *factory = m_customWidgets.value(widgetName);
        if (factory != 0)
            w = factory->createWidget(parentWidget);
    }

    if (w == 0) { // nothing to do
        return 0;
    }

    w->setObjectName(name);

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

/*!
*/
QStringList QFormBuilder::pluginPaths() const
{
    return m_pluginPaths;
}

/*!
*/
void QFormBuilder::clearPluginPaths()
{
    m_pluginPaths.clear();
    updateCustomWidgets();
}

/*!
*/
void QFormBuilder::addPluginPath(const QString &pluginPath)
{
    m_pluginPaths.append(pluginPath);
    updateCustomWidgets();
}

/*!
*/
void QFormBuilder::setPluginPath(const QStringList &pluginPaths)
{
    m_pluginPaths = pluginPaths;
    updateCustomWidgets();
}

void QFormBuilder::updateCustomWidgets()
{
    m_customWidgets.clear();

    foreach (QString path, m_pluginPaths) {
        QDir dir(path);
        QStringList candidates = dir.entryList(QDir::Files);

        foreach (QString plugin, candidates) {
            if (!QLibrary::isLibrary(plugin))
                continue;

            QPluginLoader loader(path + QLatin1String("/") + plugin);
            if (loader.load()) {
                // step 1) try with a normal plugin
                QDesignerCustomWidgetInterface *iface = 0;
                iface = qobject_cast<QDesignerCustomWidgetInterface *>(loader.instance());
                if (iface != 0) {
                    m_customWidgets.insert(iface->name(), iface);
                    continue;
                }

                // step 2) try with a collection of plugins
                QDesignerCustomWidgetCollectionInterface *c = 0;
                c = qobject_cast<QDesignerCustomWidgetCollectionInterface *>(loader.instance());
                if (c != 0) {
                    foreach (QDesignerCustomWidgetInterface *iface, c->customWidgets()) {
                        m_customWidgets.insert(iface->name(), iface);
                    }
                }
            }
        }
    }
}

/*!
    \fn QList<QDesignerCustomWidgetInterface*> QFormBuilder::customWidgets() const
*/
QList<QDesignerCustomWidgetInterface*> QFormBuilder::customWidgets() const
{
    return m_customWidgets.values();
}
