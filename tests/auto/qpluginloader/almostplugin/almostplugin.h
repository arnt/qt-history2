#ifndef ALMOSTPLUGIN_H
#define ALMOSTPLUGIN_H

#include <QObject>
#include "../theplugin/plugininterface.h"

class AlmostPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    QString pluginName() const;
    void unresolvedSymbol() const;
};

#endif // ALMOSTPLUGIN_H
