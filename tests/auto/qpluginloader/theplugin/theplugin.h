#ifndef THEPLUGIN_H
#define THEPLUGIN_H

#include <QObject>
#include <plugininterface.h>

class ThePlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    virtual QString pluginName() const;
};

#endif // THEPLUGIN_H

