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
