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

#include <QDir>
#include <QFile>
#include <QMap>
#include <QPluginLoader>
#include <qdebug.h>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent), 
      settings(Qt::UserScope, "Trolltech/Designer")
{
    settings.beginGroup("PluginManager");
}

PluginManager::~PluginManager()
{
    settings.endGroup();
}

QStringList PluginManager::unique(const QStringList &list)
{
    QMap<QString, bool> m;
    foreach (QString s, list)
        m.insert(s, true);
        
    return m.keys();
}

void PluginManager::registerPath(const QString &path)
{
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

    QDir dir(path);
    if (!dir.exists(QLatin1String("."))) {
        qWarning("invalid plugin path: %s", path.latin1());
        return;
    }
        
    QStringList candidates = dir.entryList(filters, QDir::Files);
    foreach (QString plugin, candidates) {
        QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugin);
        registerPlugin(fileName);
    }

}

QStringList PluginManager::registeredPlugins() const
{
    QStringList plugins = settings.value("RegisteredPlugins").toStringList();
    return unique(plugins);
}

void PluginManager::registerPlugin(const QString &plugin)
{
    QStringList plugins = registeredPlugins();
    
    QPluginLoader loader(plugin);
    if (loader.load() && !plugins.contains(plugin)) {
        plugins.append(plugin);
        settings.setValue("RegisteredPlugins", plugins);
    }
}

void PluginManager::unregisterPlugin(const QString &plugin)
{
    QPluginLoader loader(plugin);
    if (loader.unload()) {
        QStringList plugins = registeredPlugins();
        plugins.removeAll(plugin);
        settings.setValue("RegisteredPlugins", plugins);
    }
}

QStringList PluginManager::pluginPaths() const
{
    return settings.value("PluginPaths").toStringList();
}

void PluginManager::setPluginPaths(const QStringList &paths)
{
    settings.setValue("PluginPaths", paths);
}

void PluginManager::clearPluginPaths()
{
    setPluginPaths(QStringList());
}

void PluginManager::addPluginPath(const QString &path)
{
    QStringList paths = pluginPaths();
    paths.append(path);
    setPluginPaths(paths);
}

void PluginManager::removePluginPath(const QString &path)
{
    QStringList paths = pluginPaths();
    paths.removeAll(path);
    setPluginPaths(paths);
}

QObject *PluginManager::instance(const QString &plugin) const
{
    QPluginLoader loader(plugin);
    if (loader.load())
        return loader.instance();
        
    return 0;
}
