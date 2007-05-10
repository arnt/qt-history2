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

#include "qscreenahi_qws.h"

#include <QtGui/qscreendriverplugin_qws.h>
#include <QtCore/qstringlist.h>

class AhiScreenDriverPlugin : public QScreenDriverPlugin
{
public:
    AhiScreenDriverPlugin();

    QStringList keys() const;
    QScreen* create(const QString&, int displayId);
};

AhiScreenDriverPlugin::AhiScreenDriverPlugin()
    : QScreenDriverPlugin()
{
}

QStringList AhiScreenDriverPlugin::keys() const
{
    return (QStringList() << "ahi");
}

QScreen* AhiScreenDriverPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() != "ahi")
        return 0;

    return new QAhiScreen(displayId);
}

Q_EXPORT_STATIC_PLUGIN(AhiScreenDriver)
Q_EXPORT_PLUGIN2(qahiscreendriver, AhiScreenDriverPlugin)
