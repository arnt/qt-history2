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

#include "qscreenahigl_qws.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class QAhiGLScreenPlugin : public QScreenDriverPlugin
{
public:
    QAhiGLScreenPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

QAhiGLScreenPlugin::QAhiGLScreenPlugin()
    : QScreenDriverPlugin()
{
}

QStringList QAhiGLScreenPlugin::keys() const
{
    return (QStringList() << "ahigl");
}

QScreen* QAhiGLScreenPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() != "ahigl")
        return 0;

    return new QAhiGLScreen(displayId);
}

Q_EXPORT_STATIC_PLUGIN(QAhiGLScreen)
Q_EXPORT_PLUGIN2(qahiglscreen, QAhiGLScreenPlugin)
