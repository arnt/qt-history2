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
#include <qgfxsnap_qws.h>
#include <qstringlist.h>

class GfxSnapDriver : public QGfxDriverPlugin
{
public:
    GfxSnapDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxSnapDriver::GfxSnapDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxSnapDriver::keys() const
{
    QStringList list;
    list << "snap";
    return list;
}

QScreen* GfxSnapDriver::create(const QString& driver, int displayId)
{
    if (driver.toLower() == "snap")
        return new QSNAPScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxSnapDriver)
