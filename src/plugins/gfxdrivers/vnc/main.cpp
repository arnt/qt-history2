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
#include <qgfxvnc_qws.h>

class GfxVncDriver : public QGfxDriverPlugin
{
public:
    GfxVncDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxVncDriver::GfxVncDriver()
: QGfxDriverPlugin()
{
}

QStringList GfxVncDriver::keys() const
{
    QStringList list;
    list << "VNC";
    return list;
}

QScreen* GfxVncDriver::create(const QString& driver, int displayId)
{
    if (driver.lower() == "vnc")
        return new QVNCScreen(displayId);

    return 0;
}

Q_EXPORT_PLUGIN(GfxVncDriver)
