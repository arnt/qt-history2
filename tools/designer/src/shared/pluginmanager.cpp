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
    : QObject(parent)
{
    QSettings settings;
    settings.beginGroup("PluginManager");
    m_registeredPlugins = unique(settings.value("RegisteredPlugins").toStringList());
    m_pluginPaths = unique(settings.value("PluginPaths").toStringList());
    settings.endGroup();
}

PluginManager::~PluginManager()
{
    syncSettings();
}

QStringList PluginManager::unique(const QStringList &list)
{
    QMap<QString, bool> m;
    foreach (QString s, list) {
        if (s.isEmpty())
            continue;

        m.insert(s, true);
    }

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
        qWarning("invalid plugin path: %s", path.toLatin1().constData());
        return;
    }

    QStringList candidates = dir.entryList(filters);
    foreach (QString plugin, candidates) {
        QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugin);
        registerPlugin(fileName);
    }

}

QStringList PluginManager::registeredPlugins() const
{
    return m_registeredPlugins;
}

void PluginManager::registerPlugin(const QString &plugin)
{
    QPluginLoader loader(plugin);
    if (loader.load() && !m_registeredPlugins.contains(plugin))
        m_registeredPlugins += plugin;
}

void PluginManager::unregisterPlugin(const QString &plugin)
{
    m_registeredPlugins.removeAll(plugin);
}

QStringList PluginManager::pluginPaths() const
{
    return m_pluginPaths;
}

void PluginManager::setPluginPaths(const QStringList &paths)
{
    m_pluginPaths = paths;
}

void PluginManager::clearPluginPaths()
{
    m_pluginPaths.clear();
}

void PluginManager::addPluginPath(const QString &path)
{
    m_pluginPaths += path;
}

void PluginManager::removePluginPath(const QString &path)
{
    m_pluginPaths.removeAll(path);
}

QObject *PluginManager::instance(const QString &plugin) const
{
    QPluginLoader loader(plugin);
    if (loader.load())
        return loader.instance();

    return 0;
}

bool PluginManager::syncSettings()
{
    QSettings settings;
    settings.beginGroup("PluginManager");
    settings.setValue("PluginPaths", m_pluginPaths);
    settings.setValue("RegisteredPlugins", m_registeredPlugins);
    settings.endGroup();
    return settings.status() == QSettings::NoError;
}
