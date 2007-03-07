/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "simplestyleplugin.h"
#include "simplestyle.h"

QStringList SimpleStylePlugin::keys() const
{
    return QStringList() << "SimpleStyle";
}

QStyle *SimpleStylePlugin::create(const QString &key)
{
    if (key.toLower() == "simplestyle")
        return new SimpleStyle;
    return 0;
}

Q_EXPORT_PLUGIN2(simplestyleplugin, SimpleStylePlugin)
