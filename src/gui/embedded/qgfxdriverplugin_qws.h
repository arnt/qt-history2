/****************************************************************************
**
** Definition of QGfxDriverPlugin.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXDRIVERPLUGIN_QWS_H
#define QGFXDRIVERPLUGIN_QWS_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

class QScreen;
class QGfxDriverPluginPrivate;

class Q_GUI_EXPORT QGfxDriverPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QGfxDriverPlugin();
    ~QGfxDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QScreen* create(const QString& driver, int displayId) = 0;

private:
    QGfxDriverPluginPrivate *d;
};

#endif // QT_NO_COMPONENT

#endif // QGFXDRIVERPLUGIN_QWS_H
