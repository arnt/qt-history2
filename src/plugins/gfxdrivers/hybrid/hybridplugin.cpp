/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "hybridscreen.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class HybridPlugin : public QScreenDriverPlugin
{
public:
    HybridPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

HybridPlugin::HybridPlugin()
    : QScreenDriverPlugin()
{
}

QStringList HybridPlugin::keys() const
{
    return (QStringList() << "hybrid");
}

QScreen* HybridPlugin::create(const QString &driver, int displayId)
{
    if (driver.toLower() != "hybrid")
        return 0;

    return new HybridScreen(displayId);
}

Q_EXPORT_STATIC_PLUGIN(Hybrid)
Q_EXPORT_PLUGIN2(hybridscreendriver, HybridPlugin)
