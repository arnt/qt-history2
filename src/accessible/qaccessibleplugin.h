/****************************************************************************
**
** Definition of the QAccessiblePlugin class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEPLUGIN_H
#define QACCESSIBLEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H
#ifndef QT_NO_ACCESSIBLEPLUGIN

class QAccessiblePluginPrivate;
struct QAccessibleInterface;

class Q_GUI_EXPORT QAccessiblePlugin : public QGPlugin
{
public:
    QAccessiblePlugin();
    ~QAccessiblePlugin();

    virtual QStringList keys() const = 0;
    virtual QAccessibleInterface *create( const QString &key, QObject *object ) = 0;

private:
    QAccessiblePluginPrivate *d;
};

#endif // QT_NO_ACCESSIBLEPLUGIN

#endif // QACCESSIBLEPLUGIN_H
