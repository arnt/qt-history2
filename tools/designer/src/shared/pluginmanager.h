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

#include <QStringList>
#include <qsettings.h>

class QT_SHARED_EXPORT PluginManager: public QObject
{
    Q_OBJECT
public:
    PluginManager(QObject *parent = 0);
    virtual ~PluginManager();

    QObject *instance(const QString &plugin) const;

    QStringList registeredPlugins() const;
    void registerPlugin(const QString &plugin);
    void unregisterPlugin(const QString &plugin);
    
    void registerPath(const QString &path);

    QStringList pluginPaths() const;
    void setPluginPaths(const QStringList &paths);

    void clearPluginPaths();

    void addPluginPath(const QString &path);
    void removePluginPath(const QString &path);
    
private:
    static QStringList unique(const QStringList &list);
    
private:
    QSettings settings;
};

#endif // PLUGINMANAGER_H
