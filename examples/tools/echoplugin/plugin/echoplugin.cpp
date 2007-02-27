#include <QtGui>

#include "echoplugin.h"

QString EchoPlugin::echo(const QString &message)
{
    return message;
}

Q_EXPORT_PLUGIN2(pnp_echoplugin, EchoPlugin);
