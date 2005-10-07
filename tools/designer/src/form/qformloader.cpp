
#include "qformloader.h"
#include "customwidget.h"

#include <formbuilder.h>

#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QWidget>
#include <QMap>
#include <private/qobject_p.h>

/*!
    \namespace QForm
    \inmodule QtForm
    \brief The QForm namespace contains classes that are used to handle forms created with
    Qt Designer.
*/

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

/*!
    \class QForm::Loader
    \inmodule QtForm
    \brief The Loader class is used to load form files created with Qt Designer
    and use the information within them to build user interfaces.

    \sa QForm
*/

/*!
  \fn QForm::Loader::Loader(QObject *parent)

  Creates a form loader with the given \a parent.
*/
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

/*!
  \fn QForm::Loader::~Loader()

  Destroys the loader.
*/
Loader::~Loader()
{
}

/*!
  \fn QWidget *QForm::Loader::load(QIODevice *device, QWidget *parentWidget)

  Loads a form from the given \a device and creates a new widget with the given
  \a parent to hold its contents.
*/
QWidget *Loader::load(QIODevice *device, QWidget *parentWidget)
{
    Q_D(Loader);
    return d->builder.load(device, parentWidget);
}

/*!
  \fn QStringList QForm::Loader::pluginPaths() const

  Returns a list of paths the loader searches to locate custom widget plugins.

  \sa addPluginPath(), clearPluginPaths()
*/
QStringList Loader::pluginPaths() const
{
    Q_D(const Loader);
    return d->builder.pluginPaths();
}

/*!
  \fn void QForm::Loader::clearPluginPaths()

  Clears the list of paths used to locate plugins.

  \sa addPluginPath(), pluginPaths()
*/
void Loader::clearPluginPaths()
{
    Q_D(Loader);
    d->builder.clearPluginPaths();
}

/*!
  \fn void QForm::Loader::addPluginPath(const QString &path)

  Adds the given \a path to the list of paths used to locate plugins.

  \sa pluginPaths(), clearPluginPaths()
*/
void Loader::addPluginPath(const QString &path)
{
    Q_D(Loader);
    d->builder.addPluginPath(path);
}

/*!
  \fn QWidget *QForm::Loader::createWidget(const QString &className, QWidget *parent)

  Creates a new widget using the class specified by \a className with the given
  \a parent widget.

  \sa availableWidgets()*/
QWidget *Loader::createWidget(const QString &className, QWidget *parent)
{
    Q_D(Loader);
    return d->builder.defaultCreateWidget(className, parent);
}

/*!
  \fn QStringList QForm::Loader::availableWidgets() const

  Returns a list of available widgets that can be built using the loader.
*/
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

