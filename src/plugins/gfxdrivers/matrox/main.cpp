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
#include <qgfxmatrox_qws.h>
#include <qstringlist.h>

class GfxMatroxDriver : public QGfxDriverPlugin
{
public:
    GfxMatroxDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxMatroxDriver::GfxMatroxDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxMatroxDriver::keys() const
{
    QStringList list;
    list << "Matrox";
    return list;
}

QScreen* GfxMatroxDriver::create(const QString& driver, int displayId)
{
    if (driver.toLower() == "matrox")
        return new QMatroxScreen(displayId);

    return 0;
}

Q_EXPORT_STATIC_PLUGIN(GfxMatroxDriver)
Q_EXPORT_PLUGIN2(qgfxmatrox, GfxMatroxDriver)
