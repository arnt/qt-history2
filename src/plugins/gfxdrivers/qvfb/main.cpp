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
#include <qgfxvfb_qws.h>

class GfxVfbDriver : public QGfxDriverPlugin
{
public:
    GfxVfbDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxVfbDriver::GfxVfbDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxVfbDriver::keys() const
{
    QStringList list;
    list << "QVFb";
    return list;
}

QScreen* GfxVfbDriver::create(const QString& driver, int displayId)
{
    if (driver.toLower() == "qvfb")
        return new QVFbScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxVfbDriver)
