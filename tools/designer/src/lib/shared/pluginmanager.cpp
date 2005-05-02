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

#include "pluginmanager.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QPluginLoader>
#include <QtCore/QLibraryInfo>
#include <QtCore/qdebug.h>


namespace qdesigner_internal {

static QStringList unique(const QStringList &lst)
{
    QSet<QString> s = QSet<QString>::fromList(lst);
    return s.toList();
}

}

PluginManager::PluginManager(QDesignerFormEditorInterface *core)
    : QObject(core), m_core(core)
{
    QSettings settings;

    settings.beginGroup(QLatin1String("PluginManager"));

    if (!settings.contains(QLatin1String("PluginPaths"))) {
        // first time designer is run - set some defaults
        QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath)
                            + QDir::separator() + QLatin1String("designer");
        settings.setValue(QLatin1String("PluginPaths"), QStringList() << path);
    }

    m_pluginPaths = qdesigner_internal::unique(settings.value(QLatin1String("PluginPaths")).toStringList());
    m_disabledPlugins = qdesigner_internal::unique(settings.value(QLatin1String("DisabledPlugins")).toStringList());
    updateRegisteredPlugins();

    settings.endGroup();
}

PluginManager::~PluginManager()
{
    syncSettings();
}

QDesignerFormEditorInterface *PluginManager::core() const
{
    return m_core;
}

QStringList PluginManager::findPlugins(const QString &path)
{
    QStringList result;

    QDir dir(path);
    if (!dir.exists())
        return result;

    QStringList filters;
#if defined(Q_OS_WIN32)
    filters << QLatin1String("*.dll");
#elif defined(Q_OS_DARWIN)
    filters << QLatin1String("*.dylib") << QLatin1String("*.so") << QLatin1String("*.bundle");
#elif defined(Q_OS_HPUX)
    filters << QLatin1String("*.sl");
#elif defined(Q_OS_UNIX)
    filters << QLatin1String("*.so");
#endif

    QStringList candidates = dir.entryList(filters);
    foreach (QString plugin, candidates) {
        QString fileName = dir.absoluteFilePath(plugin);
        result.append(fileName);
    }

    return result;
}

void PluginManager::setDisabledPlugins(const QStringList &disabled_plugins)
{
    m_disabledPlugins = disabled_plugins;
    updateRegisteredPlugins();
}

void PluginManager::setPluginPaths(const QStringList &plugin_paths)
{
    m_pluginPaths = plugin_paths;
    updateRegisteredPlugins();
}

QStringList PluginManager::disabledPlugins() const
{
    return m_disabledPlugins;
}

QStringList PluginManager::registeredPlugins() const
{
    return m_registeredPlugins;
}

QStringList PluginManager::pluginPaths() const
{
    return m_pluginPaths;
}

QObject *PluginManager::instance(const QString &plugin) const
{
    if (m_disabledPlugins.contains(plugin))
        return 0;

    QPluginLoader loader(plugin);
    if (loader.load())
        return loader.instance();

    return 0;
}

void PluginManager::updateRegisteredPlugins()
{
    m_registeredPlugins.clear();
    foreach (QString path,  m_pluginPaths)
        registerPath(path);
}

void PluginManager::registerPath(const QString &path)
{
    QStringList candidates = findPlugins(path);

    foreach (QString plugin, candidates)
        registerPlugin(plugin);
}

void PluginManager::registerPlugin(const QString &plugin)
{
    if (m_disabledPlugins.contains(plugin))
        return;
    if (m_registeredPlugins.contains(plugin))
        return;

    QPluginLoader loader(plugin);
    if (loader.load())
        m_registeredPlugins += plugin;
}

bool PluginManager::syncSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("PluginManager"));
    settings.setValue(QLatin1String("PluginPaths"), m_pluginPaths);
    settings.setValue(QLatin1String("DisabledPlugins"), m_disabledPlugins);
    settings.endGroup();
    return settings.status() == QSettings::NoError;
}

void PluginManager::ensureInitialized()
{
    QStringList plugins = registeredPlugins();

    m_customWidgets.clear();
    foreach (QString plugin, plugins) {
        QObject *o = instance(plugin);

        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            m_customWidgets.append(c);
        } else if (QDesignerCustomWidgetCollectionInterface *coll = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(o)) {
            m_customWidgets += coll->customWidgets();
        }
    }

    foreach (QDesignerCustomWidgetInterface *c, m_customWidgets) {
        if (!c->isInitialized())
            c->initialize(core());
    }
}

QList<QDesignerCustomWidgetInterface*> PluginManager::registeredCustomWidgets() const
{
    const_cast<PluginManager*>(this)->ensureInitialized();
    return m_customWidgets;
}

