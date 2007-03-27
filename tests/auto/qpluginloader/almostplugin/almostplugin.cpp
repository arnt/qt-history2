#include <QtCore/QString>
#include "almostplugin.h"
#include <QtCore/qplugin.h>

QString AlmostPlugin::pluginName() const
{
    unresolvedSymbol();
    return QLatin1String("Plugin ok");
}

Q_EXPORT_PLUGIN2(almostplugin, AlmostPlugin)
