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
#include <qgfxvga16_qws.h>

class GfxVga16Driver : public QGfxDriverPlugin
{
public:
    GfxVga16Driver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxVga16Driver::GfxVga16Driver()
: QGfxDriverPlugin()
{
}

QStringList GfxVga16Driver::keys() const
{
    QStringList list;
    list << "VGA16";
    return list;
}

QScreen* GfxVga16Driver::create(const QString& driver, int displayId)
{
    if (driver.lower() == "vga16")
        return new QVga16Screen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxVga16Driver)
