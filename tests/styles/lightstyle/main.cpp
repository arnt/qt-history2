#include <qstyleplugin.h>
#include "lightstyle.h"

class LightStylePlugin : public QStylePlugin
{
public:
    LightStylePlugin();

    QStringList keys() const;
    QStyle *create(const QString &);
};

LightStylePlugin::LightStylePlugin()
    : QStylePlugin()
{
}

QStringList LightStylePlugin::keys() const
{
    QStringList list;
    list << "Light";
    return list;
}

QStyle *LightStylePlugin::create(const QString &s)
{
    if (s.lower() == "light")
	return new LightStyle;
    return 0;
}

Q_EXPORT_PLUGIN( LightStylePlugin )
