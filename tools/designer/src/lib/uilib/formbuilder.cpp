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

#include "customwidget.h"
#include "formbuilder.h"
#include "ui4.h"

#include <QtGui/QtGui>

#ifdef QFORMINTERNAL_NAMESPACE
using namespace QFormInternal;
#endif


/*!
    \class QFormBuilder

    \brief The QFormBuilder class is used to dynamically construct
    user interfaces from .ui files at run-time.

    \inmodule QtDesigner

    The QFormBuilder class provides a mechanism for dynamically
    creating user interfaces at run-time, based on \c{.ui} files
    created with \QD. For example:

    \code
        MyForm::MyForm(QWidget *parent)
            : QWidget(parent)
        {
            QFormBuilder builder;
            QFile file(":/forms/myWidget.ui");
            file.open(QFile::ReadOnly);
            QWidget *myWidget = builder.load(&file, this);
            file.close();

            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(myWidget);
            setLayout(layout);
        }
    \endcode

    By including the user interface in the example's resources (\c
    myForm.grc), we ensure that it will be present when the example is
    run:

    \code
    <!DOCTYPE RCC><RCC version="1.0">
    <qresource prefix="/forms">
       <file>mywidget.ui</file>
    </qresource>
    </RCC>
    \endcode

    QFormBuilder extends the QAbstractFormBuilder base class with a
    number of functions that are used to support custom widget
    plugins:

    \list
    \o pluginPaths() returns the list of paths that the form builder
       searches when loading custom widget plugins.
    \o addPluginPath() allows additional paths to be registered with
       the form builder.
    \o setPluginPath() is used to replace the existing list of paths
       with a list obtained from some other source.
    \o clearPluginPaths() removes all paths registered with the form
       builder.
    \o customWidgets() returns a list of interfaces to plugins that
       can be used to create new instances of registered custom widgets.
    \endlist

    For a complete example using QFormBuilder, see the \l
    {designer/calculatorbuilder}{Calculator Builder example} which
    shows how to create a user interface from a \QD form at runtime,
    using the QFormBuilder class.

    \sa QAbstractFormBuilder
*/

/*!
    \fn QFormBuilder::QFormBuilder()

    Constructs a new form builder.
*/

QFormBuilder::QFormBuilder()
{
}

/*!
    \internal
*/
QWidget *QFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui_widget, parentWidget);
}


/*!
    \internal
*/
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
        qWarning("QFormBuilder: Cannot create widget of class %s.", widgetName.toLatin1().constData());
        return 0;
    }

    w->setObjectName(name);

    if (qobject_cast<QDialog *>(w))
        w->setParent(parentWidget, 0);

    return w;
}

/*!
    \internal
*/
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

/*!
    \internal
*/
bool QFormBuilder::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    return QAbstractFormBuilder::addItem(ui_item, item, layout);
}

/*!
    \internal
*/
bool QFormBuilder::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    return QAbstractFormBuilder::addItem(ui_widget, widget, parentWidget);
}

/*!
    \internal
*/
QWidget *QFormBuilder::widgetByName(QWidget *topLevel, const QString &name)
{
    Q_ASSERT(topLevel);
    if (topLevel->objectName() == name)
        return topLevel;

    return qFindChild<QWidget*>(topLevel, name);
}

/*!
    \internal
*/
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

/*!
    \internal
*/
QWidget *QFormBuilder::create(DomUI *ui, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui, parentWidget);
}

/*!
    \internal
*/
QLayout *QFormBuilder::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui_layout, layout, parentWidget);
}

/*!
    \internal
*/
QLayoutItem *QFormBuilder::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    return QAbstractFormBuilder::create(ui_layoutItem, layout, parentWidget);
}

/*!
    \internal
*/
QAction *QFormBuilder::create(DomAction *ui_action, QObject *parent)
{
    return QAbstractFormBuilder::create(ui_action, parent);
}

/*!
    \internal
*/
QActionGroup *QFormBuilder::create(DomActionGroup *ui_action_group, QObject *parent)
{
    return QAbstractFormBuilder::create(ui_action_group, parent);
}

/*!
    Returns the list of paths the form builder searches for plugins.

    \sa addPluginPath(), clearPluginPaths(), setPluginPath()*/
QStringList QFormBuilder::pluginPaths() const
{
    return m_pluginPaths;
}

/*!
    Clears the list of paths that the form builder uses to search for custom widget plugins.

    \sa pluginPaths(), addPluginPath(), setPluginPath()*/
void QFormBuilder::clearPluginPaths()
{
    m_pluginPaths.clear();
    updateCustomWidgets();
}

/*!
    Adds a new plugin path specified by \a pluginPath to the list of paths that will be
    searched by the form builder when loading a custom widget plugin.

    \sa setPluginPath(), clearPluginPaths()*/
void QFormBuilder::addPluginPath(const QString &pluginPath)
{
    m_pluginPaths.append(pluginPath);
    updateCustomWidgets();
}

/*!
    Sets the list of plugin paths to the list specified by \a pluginPaths.

    \sa addPluginPath(), clearPluginPaths(), pluginPaths()*/
void QFormBuilder::setPluginPath(const QStringList &pluginPaths)
{
    m_pluginPaths = pluginPaths;
    updateCustomWidgets();
}

/*!
    \internal
*/
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

    Returns the list of interfaces to the custom widgets registered with the form builder.
*/
QList<QDesignerCustomWidgetInterface*> QFormBuilder::customWidgets() const
{
    return m_customWidgets.values();
}
