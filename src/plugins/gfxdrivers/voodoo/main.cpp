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
#include <qgfxvoodoo_qws.h>

class GfxVoodooDriver : public QGfxDriverPlugin
{
public:
    GfxVoodooDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxVoodooDriver::GfxVoodooDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxVoodooDriver::keys() const
{
    QStringList list;
    list << "Voodoo3";
    return list;
}

QScreen* GfxVoodooDriver::create(const QString& driver, int displayId)
{
    if (driver.toLower() == "voodoo3")
        return new QVoodooScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxVoodooDriver)
