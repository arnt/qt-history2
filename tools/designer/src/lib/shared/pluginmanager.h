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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "shared_global.h"

#include <QtCore/QStringList>
#include <QtCore/QSettings>

class QT_SHARED_EXPORT PluginManager: public QObject
{
    Q_OBJECT
public:
    PluginManager(QObject *parent = 0);
    virtual ~PluginManager();

    QObject *instance(const QString &plugin) const;

    QStringList registeredPlugins() const;

    QStringList findPlugins(const QString &path);
    
    void setPluginPaths(const QStringList &plugin_paths);
    QStringList pluginPaths() const;
    void setDisabledPlugins(const QStringList &disabled_plugins);
    QStringList disabledPlugins() const;
        
    bool syncSettings();

private:
    void updateRegisteredPlugins();
    void registerPath(const QString &path);
    void registerPlugin(const QString &plugin);
    
    QStringList m_pluginPaths;
    QStringList m_registeredPlugins;
    QStringList m_disabledPlugins;
};

#endif // PLUGINMANAGER_H
