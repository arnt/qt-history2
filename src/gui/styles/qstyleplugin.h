/****************************************************************************
**
** Definition of QStylePlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_STYLE
#ifndef QT_NO_COMPONENT

class QStyle;
class QStylePluginPrivate;

class Q_GUI_EXPORT QStylePlugin : public QGPlugin
{
    Q_OBJECT
public:
    QStylePlugin();
    ~QStylePlugin();

    virtual QStringList keys() = 0;
    virtual QStyle *create(const QString &key) = 0;

private:
    QStylePluginPrivate *d;
};

#endif // QT_NO_COMPONENT
#endif // QT_NO_STYLE

#endif // QSTYLEPLUGIN_H
