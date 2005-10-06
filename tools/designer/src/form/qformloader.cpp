
#include "qformloader.h"
#include "customwidget.h"

#include <formbuilder.h>

#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QWidget>
#include <QMap>
#include <private/qobject_p.h>

namespace QForm
{

typedef QMap<QString, bool> widget_map;
Q_GLOBAL_STATIC(widget_map, g_widgets)

class FormBuilderPrivate: public QFormInternal::QFormBuilder
{
    friend class Loader;
    friend class LoaderPrivate;

public:
    Loader *loader;

    FormBuilderPrivate(): loader(0) {}

    QWidget *defaultCreateWidget(const QString &widgetName, QWidget *parentWidget)
    {
        return QFormInternal::QFormBuilder::createWidget(widgetName, parentWidget, QString());
    }

    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
    {
        if (QWidget *widget = loader->createWidget(widgetName, parentWidget)) {
            widget->setObjectName(name);
            return widget;
        }

        return 0;
    }
};

class LoaderPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(Loader)
public:
    FormBuilderPrivate builder;

    void setupWidgetMap() const;
};

void LoaderPrivate::setupWidgetMap() const
{
    if (!g_widgets()->isEmpty())
        return;

#define DECLARE_WIDGET(a, b) g_widgets()->insert(QLatin1String(#a), true);
#define DECLARE_LAYOUT(a, b)

#include "widgets.table"

#undef DECLARE_WIDGET
#undef DECLARE_LAYOUT
}

Loader::Loader(QObject *parent)
    : QObject(*new LoaderPrivate, parent)
{
    Q_D(Loader);

    d->builder.loader = this;

    QStringList paths;
    foreach (const QString &path, QApplication::libraryPaths()) {
        paths.append(path + QDir::separator() + QLatin1String("designer"));
    }

    d->builder.setPluginPath(paths);
}

Loader::~Loader()
{
}

QWidget *Loader::load(QIODevice *device, QWidget *parentWidget)
{
    Q_D(Loader);
    return d->builder.load(device, parentWidget);
}

QStringList Loader::pluginPaths() const
{
    Q_D(const Loader);
    return d->builder.pluginPaths();
}

void Loader::clearPluginParhs()
{
    Q_D(Loader);
    d->builder.clearPluginPaths();
}

void Loader::addPluginPath(const QString &path)
{
    Q_D(Loader);
    d->builder.addPluginPath(path);
}

QWidget *Loader::createWidget(const QString &className, QWidget *parent)
{
    Q_D(Loader);
    return d->builder.defaultCreateWidget(className, parent);
}

QStringList Loader::availableWidgets() const
{
    Q_D(const Loader);

    d->setupWidgetMap();
    widget_map available = *g_widgets();

    foreach (QDesignerCustomWidgetInterface *plugin, d->builder.customWidgets()) {
        available.insert(plugin->name(), true);
    }

    return available.keys();
}

} // namespace QForm

