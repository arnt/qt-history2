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
#include <QLibraryInfo>
#include <qdebug.h>

static QStringList unique(const QStringList &list)
{
    QMap<QString, bool> m;
    foreach (QString s, list) {
        if (s.isEmpty())
            continue;

        m.insert(s, true);
    }

    return m.keys();
}

static QString fixPath(const QString path)
{
    QFileInfo fi(path);
    return fi.absoluteFilePath();
}

static QStringList fixPathList(const QStringList &pathList)
{
    QStringList result;
    foreach (QString path, pathList)
        result.append(fixPath(path));
    return result;
}

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
    QSettings settings;
    
    settings.beginGroup(QLatin1String("PluginManager"));
    
    if (!settings.contains(QLatin1String("PluginPaths"))) {
        // first time designer is run - set some defaults
        QString path = QLibraryInfo::location(QLibraryInfo::PluginsPath) 
                            + QDir::separator() + QLatin1String("designer");
        settings.setValue(QLatin1String("PluginPaths"), QStringList() << path);
    }
    
    m_disabledPlugins
        = unique(settings.value(QLatin1String("DisabledPlugins")).toStringList());
    m_pluginPaths = fixPathList(unique(
            settings.value(QLatin1String("PluginPaths")).toStringList()));
                            
    settings.endGroup();

    foreach (QString path, m_pluginPaths)
        registerPath(path);
}

PluginManager::~PluginManager()
{
    syncSettings();
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
    if (!dir.exists()) {
        qWarning("invalid plugin path: %s", path.toLatin1().constData());
        return;
    }

    QStringList candidates = dir.entryList(filters);
    foreach (QString plugin, candidates) {
        QString fileName = dir.absoluteFilePath(plugin);
        registerPlugin(fileName);
    }
}

void PluginManager::disablePlugin(const QString &path, bool disabled)
{
    if (disabled) {
        m_registeredPlugins.removeAll(path);
        if (!m_disabledPlugins.contains(path))
            m_disabledPlugins.append(path);
    } else {
        m_disabledPlugins.removeAll(path);
    }
}

QStringList PluginManager::registeredPlugins() const
{
    return m_registeredPlugins;
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

void PluginManager::unregisterPlugin(const QString &plugin)
{
    if (!m_registeredPlugins.contains(plugin))
        return;
    m_registeredPlugins.removeAll(plugin);
}

QStringList PluginManager::pluginPaths() const
{
    return m_pluginPaths;
}

void PluginManager::addPluginPath(const QString &path)
{
    QString fixedPath = fixPath(path);
    if (m_pluginPaths.contains(fixedPath))
        return;
    m_pluginPaths.append(fixedPath);
    registerPath(fixedPath);
}

void PluginManager::removePluginPath(const QString &path)
{
    QString fixedPath = fixPath(path);
    if (!m_pluginPaths.contains(fixedPath))
        return;
    m_pluginPaths.removeAll(fixedPath);
    foreach (QString plugin, m_registeredPlugins) {
        QFileInfo fi(plugin);
        if (fi.absolutePath() == fixedPath)
            unregisterPlugin(plugin);
    }
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
    settings.setValue("DisabledPlugins", m_disabledPlugins);
    settings.endGroup();
    return settings.status() == QSettings::NoError;
}
