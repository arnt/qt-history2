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

#include <qgfxdriverplugin_qws.h>
#include <qgfxshadowfb_qws.h>

class GfxShadowFbDriver : public QGfxDriverPlugin
{
public:
    GfxShadowFbDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxShadowFbDriver::GfxShadowFbDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxShadowFbDriver::keys() const
{
    QStringList list;
    list << "ShadowFb";
    return list;
}

QScreen* GfxShadowFbDriver::create(const QString& driver, int displayId)
{
    if (driver.lower() == "shadowfb")
        return new QShadowFbScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxShadowFbDriver)
