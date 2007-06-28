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

/*!
  \class QAhiGLScreenPlugin
  \brief The QAhiGLScreenPlugin class is the plugin for the ATI handheld device graphics driver.

  QAhiGLScreenPlugin inherits QScreenDriverPlugin. See
  \l{How to Create Qt Plugins} for details.
 */

/*!
  This is the default constructor.
 */
QAhiGLScreenPlugin::QAhiGLScreenPlugin()
    : QScreenDriverPlugin()
{
}

/*!
  Returns a string list containing the string "ahigl" which
  is the only screen driver supported by this plugin.
 */
QStringList QAhiGLScreenPlugin::keys() const
{
    return (QStringList() << "ahigl");
}

/*!
  Creates a screen driver of the kind specified by \a driver.
  The \a displayId identifies the Qtopia Core server to connect to.
 */
QScreen* QAhiGLScreenPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() != "ahigl")
        return 0;

    return new QAhiGLScreen(displayId);
}

Q_EXPORT_STATIC_PLUGIN(QAhiGLScreen)
Q_EXPORT_PLUGIN2(qahiglscreen, QAhiGLScreenPlugin)
