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
#include <qgfxtransformed_qws.h>

class GfxTransformedDriver : public QGfxDriverPlugin
{
public:
    GfxTransformedDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxTransformedDriver::GfxTransformedDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxTransformedDriver::keys() const
{
    QStringList list;
    list << "Transformed";
    return list;
}

QScreen* GfxTransformedDriver::create(const QString& driver, int displayId)
{
    if (driver.lower() == "transformed")
        return new QTransformedScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxTransformedDriver)
