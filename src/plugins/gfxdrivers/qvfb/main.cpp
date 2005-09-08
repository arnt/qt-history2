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

#include <qscreendriverplugin_qws.h>
#include <qscreenvfb_qws.h>
#include <qstringlist.h>

class ScreenVfbDriver : public QScreenDriverPlugin
{
public:
    ScreenVfbDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

ScreenVfbDriver::ScreenVfbDriver()
: QScreenDriverPlugin()
{
}

QStringList ScreenVfbDriver::keys() const
{
    QStringList list;
    list << "QVFb";
    return list;
}

QScreen* ScreenVfbDriver::create(const QString& driver, int displayId)
{
    printf("ScreenVfbDriver create %s\n", driver.toLatin1().constData());
    if (driver.toLower() == "qvfb")
        return new QVFbScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(ScreenVfbDriver)
