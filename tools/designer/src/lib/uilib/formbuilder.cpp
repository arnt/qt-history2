/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "customwidget.h"
#include "formbuilder.h"
#include "formbuilderextra_p.h"
#include "ui4_p.h"

#include <QtGui/QtGui>

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal {
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

    The QFormBuilder class is typically used by custom components and
    applications that embed \QD. Standalone applications that need to
    dynamically generate user interfaces at run-time use the
    QUiLoader class, found in the QtUiTools module.

    \sa QAbstractFormBuilder, {QtUiTools Module}
*/

/*!
    \fn QFormBuilder::QFormBuilder()

    Constructs a new form builder.
*/

QFormBuilder::QFormBuilder() : QAbstractFormBuilder()
{
}

/*!
    Destroys the form builder.
*/
QFormBuilder::~QFormBuilder()
{
}

/*!
    \internal
*/
QWidget *QFormBuilder::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    QFormBuilderExtra::instance(this)->setProcessingLayoutWidget(false);
    if (ui_widget->attributeClass() == QLatin1String("QWidget") && !ui_widget->hasAttributeNative()
            && parentWidget
#ifndef QT_NO_MAINWINDOW
            && !qobject_cast<QMainWindow *>(parentWidget)
#endif
#ifndef QT_NO_TOOLBOX
            && !qobject_cast<QToolBox *>(parentWidget)
#endif
#ifndef QT_NO_STACKEDWIDGET
            && !qobject_cast<QStackedWidget *>(parentWidget)
#endif
#ifndef QT_NO_STACKEDWIDGET
            && !qobject_cast<QTabWidget *>(parentWidget)
#endif
            )
        QFormBuilderExtra::instance(this)->setProcessingLayoutWidget(true);
    return QAbstractFormBuilder::create(ui_widget, parentWidget);
}


/*!
    \internal
*/
QWidget *QFormBuilder::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    QWidget *w = 0;

#ifndef QT_NO_TABWIDGET
    if (qobject_cast<QTabWidget*>(parentWidget))
        parentWidget = 0;
#endif
#ifndef QT_NO_STACKEDWIDGET
    if (qobject_cast<QStackedWidget*>(parentWidget))
        parentWidget = 0;
#endif
#ifndef QT_NO_TOOLBOX
    if (qobject_cast<QToolBox*>(parentWidget))
        parentWidget = 0;
#endif

    // ### special-casing for Line (QFrame) -- fix for 4.2
    if (widgetName == QLatin1String("Line")) {
        w = new QFrame(parentWidget);
        static_cast<QFrame*>(w)->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    }

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C)
#define DECLARE_WIDGET(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(parentWidget); }
#define DECLARE_WIDGET_1(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(0, parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

    if (w == 0) { // try with a registered custom widget
        QDesignerCustomWidgetInterface *factory = m_customWidgets.value(widgetName);
        if (factory != 0)
            w = factory->createWidget(parentWidget);
    }

    if (w == 0) { // nothing to do
        qWarning() << QObject::tr("QFormBuilder was unable to create a widget of the class '%1'.").arg(widgetName);
        return 0;
    }

    w->setObjectName(name);

    if (qobject_cast<QDialog *>(w))
        w->setParent(parentWidget);

    QFormBuilderExtra *fb = QFormBuilderExtra::instance(this);
    if (!fb->rootWidget())
        fb->setRootWidget(w);

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
        if (parentLayout) {
            QWidget *w = qobject_cast<QWidget *>(parentLayout->parent());
            if (w && w->inherits("Q3GroupBox")) {
                l->setContentsMargins(w->style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                    w->style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                    w->style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                    w->style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
                QGridLayout *grid = qobject_cast<QGridLayout *>(l);
                if (grid) {
                    grid->setHorizontalSpacing(-1);
                    grid->setVerticalSpacing(-1);
                } else {
                    l->setSpacing(-1);
                }
                l->setAlignment(Qt::AlignTop);
            }
        }
    } else {
        qWarning() << QObject::tr("The layout type `%1' is not supported.").arg(layoutName);
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

static QObject *objectByName(QWidget *topLevel, const QString &name)
{
    Q_ASSERT(topLevel);
    if (topLevel->objectName() == name)
        return topLevel;

    return qFindChild<QObject*>(topLevel, name);
}

/*!
    \internal
*/
void QFormBuilder::createConnections(DomConnections *ui_connections, QWidget *widget)
{
    typedef QList<DomConnection*> DomConnectionList;
    Q_ASSERT(widget != 0);

    if (ui_connections == 0)
        return;

    const DomConnectionList connections = ui_connections->elementConnection();
    if (!connections.empty()) {
        const DomConnectionList::const_iterator cend = connections.constEnd();
        for (DomConnectionList::const_iterator it = connections.constBegin(); it != cend; ++it) {

            QObject *sender = objectByName(widget, (*it)->elementSender());
            QObject *receiver = objectByName(widget, (*it)->elementReceiver());
            if (!sender || !receiver)
                continue;

            QByteArray sig = (*it)->elementSignal().toUtf8();
            sig.prepend("2");
            QByteArray sl = (*it)->elementSlot().toUtf8();
            sl.prepend("1");
            QObject::connect(sender, sig, receiver, sl);
        }
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
    bool layoutWidget = QFormBuilderExtra::instance(this)->processingLayoutWidget();
    QLayout *l = QAbstractFormBuilder::create(ui_layout, layout, parentWidget);
    if (layoutWidget) {
        int left, top, right, bottom;
        left = top = right = bottom = 0;
        const DomPropertyHash properties = propertyMap(ui_layout->elementProperty());

        if (DomProperty *prop = properties.value(QLatin1String("leftMargin")))
            left = prop->elementNumber();

        if (DomProperty *prop = properties.value(QLatin1String("topMargin")))
            top = prop->elementNumber();

        if (DomProperty *prop = properties.value(QLatin1String("rightMargin")))
            right = prop->elementNumber();

        if (DomProperty *prop = properties.value(QLatin1String("bottomMargin")))
            bottom = prop->elementNumber();

        l->setContentsMargins(left, top, right, bottom);
        QFormBuilderExtra::instance(this)->setProcessingLayoutWidget(false);
    }
    return l;
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

    \sa addPluginPath()
*/
QStringList QFormBuilder::pluginPaths() const
{
    return m_pluginPaths;
}

/*!
    Clears the list of paths that the form builder uses to search for
    custom widget plugins.

    \sa pluginPaths()
*/
void QFormBuilder::clearPluginPaths()
{
    m_pluginPaths.clear();
    updateCustomWidgets();
}

/*!
    Adds a new plugin path specified by \a pluginPath to the list of
    paths that will be searched by the form builder when loading a
    custom widget plugin.

    \sa setPluginPath(), clearPluginPaths()
*/
void QFormBuilder::addPluginPath(const QString &pluginPath)
{
    m_pluginPaths.append(pluginPath);
    updateCustomWidgets();
}

/*!
    Sets the list of plugin paths to the list specified by \a pluginPaths.

    \sa addPluginPath()
*/
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
        const QDir dir(path);
        const QStringList candidates = dir.entryList(QDir::Files);

        foreach (QString plugin, candidates) {
            if (!QLibrary::isLibrary(plugin))
                continue;

            QString loaderPath = path;
            loaderPath += QLatin1Char('/');
            loaderPath += plugin;

            QPluginLoader loader(loaderPath);
            if (loader.load()) {
                // step 1) try with a normal plugin
                QDesignerCustomWidgetInterface *iface = qobject_cast<QDesignerCustomWidgetInterface *>(loader.instance());
                if (iface != 0) {
                    m_customWidgets.insert(iface->name(), iface);
                    continue;
                }

                // step 2) try with a collection of plugins
                QDesignerCustomWidgetCollectionInterface *c = qobject_cast<QDesignerCustomWidgetCollectionInterface *>(loader.instance());
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

    Returns a list of the available plugins.
*/
QList<QDesignerCustomWidgetInterface*> QFormBuilder::customWidgets() const
{
    return m_customWidgets.values();
}

/*!
    \internal
*/
void QFormBuilder::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    typedef QList<DomProperty*> DomPropertyList;

    if (properties.empty())
        return;

    QFormBuilderExtra *fb = QFormBuilderExtra::instance(this);

    const DomPropertyList::const_iterator cend = properties.constEnd();
    for (DomPropertyList::const_iterator it = properties.constBegin(); it != cend; ++it) {
        const QVariant v = toVariant(o->metaObject(), *it);
        if (v.isNull())
            continue;

        const QString attributeName = (*it)->attributeName();
        if (o == fb->rootWidget() && attributeName == QLatin1String("geometry")) {
            // apply only the size for the rootWidget
            fb->rootWidget()->resize(qvariant_cast<QRect>(v).size());
        } else if (fb->applyPropertyInternally(o, attributeName, v)) {
        } else if (!qstrcmp("QFrame", o->metaObject()->className ()) && attributeName == QLatin1String("orientation")) {
            // ### special-casing for Line (QFrame) -- try to fix me
            o->setProperty("frameShape", v); // v is of QFrame::Shape enum
        } else {
            o->setProperty(attributeName.toUtf8(), v);
        }
    }
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif
