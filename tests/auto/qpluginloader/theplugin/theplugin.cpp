#include <QtCore/QString>
#include "theplugin.h"
#include <QtCore/qplugin.h>

QString ThePlugin::pluginName() const
{
    return QLatin1String("Plugin ok");
}

Q_EXPORT_PLUGIN2(theplugin, ThePlugin)

