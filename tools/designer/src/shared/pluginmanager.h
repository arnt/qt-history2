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

    QStringList pluginPaths() const;
    void disablePlugin(const QString &path, bool disabled);
    void addPluginPath(const QString &path);
    void removePluginPath(const QString &path);
    
    bool syncSettings();

private:
    void registerPath(const QString &path);
    void registerPlugin(const QString &plugin);
    void unregisterPlugin(const QString &plugin);
    
    QStringList m_pluginPaths;
    QStringList m_registeredPlugins;
    QStringList m_disabledPlugins;
};

#endif // PLUGINMANAGER_H
