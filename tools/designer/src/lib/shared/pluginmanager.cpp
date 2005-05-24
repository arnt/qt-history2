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

#include "pluginmanager_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QPluginLoader>
#include <QtCore/QLibrary>
#include <QtCore/QLibraryInfo>
#include <QtCore/qdebug.h>
#include <QtCore/QCoreApplication>

namespace qdesigner_internal {

static QStringList unique(const QStringList &lst)
{
    QSet<QString> s = QSet<QString>::fromList(lst);
    return s.toList();
}

}

QStringList PluginManager::defaultPluginPaths() const
{
    QStringList result;

    QStringList path_list = QCoreApplication::libraryPaths();
    foreach (const QString &path, path_list)
        result.append(path + QDir::separator() + QLatin1String("designer"));

    result.append(QDir::homePath()
                    + QDir::separator()
                    + QLatin1String(".designer")
                    + QDir::separator()
                    + QLatin1String("plugins"));

    return result;
}

PluginManager::PluginManager(QDesignerFormEditorInterface *core)
    : QObject(core), m_core(core)
{
    QSettings settings;

    settings.beginGroup(QLatin1String("PluginManager"));

    m_pluginPaths = defaultPluginPaths();
    m_disabledPlugins
        = qdesigner_internal::unique(
            settings.value(QLatin1String("DisabledPlugins")).toStringList());
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
    QStringList candidates = dir.entryList(QDir::Files | QDir::NoSymLinks);
    foreach (QString plugin, candidates) {
        if (!QLibrary::isLibrary(plugin))
            continue;
        result.append(dir.absoluteFilePath(plugin));
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
    return loader.instance();
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
        if (!c->isInitialized()) {
            c->initialize(core());
        }
    }
}

QList<QDesignerCustomWidgetInterface*> PluginManager::registeredCustomWidgets() const
{
    const_cast<PluginManager*>(this)->ensureInitialized();
    return m_customWidgets;
}

QList<QObject*> PluginManager::instances() const
{
    QStringList plugins = registeredPlugins();

    QList<QObject*> lst;
    foreach (QString plugin, plugins) {
        if (QObject *o = instance(plugin))
            lst.append(o);
    }

    return lst;
}

