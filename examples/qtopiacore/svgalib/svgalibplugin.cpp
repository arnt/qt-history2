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

#include "svgalibscreen.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class SvgalibPlugin : public QScreenDriverPlugin
{
public:
    SvgalibPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

SvgalibPlugin::SvgalibPlugin()
    : QScreenDriverPlugin()
{
}

QStringList SvgalibPlugin::keys() const
{
    return (QStringList() << "svgalib");
}

QScreen* SvgalibPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() != "svgalib")
        return 0;

    return new SvgalibScreen(displayId);
}

Q_EXPORT_STATIC_PLUGIN(Svgalib)
Q_EXPORT_PLUGIN2(svgalibscreendriver, SvgalibPlugin)
