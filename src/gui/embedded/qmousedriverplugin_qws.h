/****************************************************************************
**
** Definition of QMouseDriverPlugin.
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

#ifndef QMOUSEDRIVERPLUGIN_QWS_H
#define QMOUSEDRIVERPLUGIN_QWS_H

#ifndef QT_H
//////#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_COMPONENT

class QWSMouseHandler;
class QMouseDriverPluginPrivate;

class Q_GUI_EXPORT QMouseDriverPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QMouseDriverPlugin();
    ~QMouseDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QWSMouseHandler* create(const QString& driver, const QString &device) = 0;

private:
    QMouseDriverPluginPrivate *d;
};

#endif // QT_NO_COMPONENT

#endif // QMOUSEDRIVERPLUGIN_QWS_H
