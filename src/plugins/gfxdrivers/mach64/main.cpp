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
#include <qgfxmach64_qws.h>

class GfxMach64Driver : public QGfxDriverPlugin
{
public:
    GfxMach64Driver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxMach64Driver::GfxMach64Driver()
: QGfxDriverPlugin()
{
}

QStringList GfxMach64Driver::keys() const
{
    QStringList list;
    list << "Mach64";
    return list;
}

QScreen* GfxMach64Driver::create(const QString& driver, int displayId)
{
    if (driver.lower() == "mach64")
        return new QMachScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxMach64Driver)
