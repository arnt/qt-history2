#include <QtGui>

#include "simplestyleplugin.h"
#include "simplestyle.h"

QStringList SimpleStylePlugin::keys() const
{
    return QStringList() << "SimpleStyle";
}

QStyle *SimpleStylePlugin::create(const QString &key)
{
    if (key.toLower() == "simplestyle")
	return new SimpleStyle;
    return 0;
}

Q_EXPORT_PLUGIN2(pnp_simplestyleplugin, SimpleStylePlugin)
