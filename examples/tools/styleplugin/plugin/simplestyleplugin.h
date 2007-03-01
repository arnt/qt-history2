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

#ifndef SIMPLESTYLEPLUGIN_H
#define SIMPLESTYLEPLUGIN_H

#include <QStylePlugin>

class QStringList;
class QStyle;

class SimpleStylePlugin : public QStylePlugin
{
    Q_OBJECT

public:
    SimpleStylePlugin() {};

    QStringList keys() const;
    QStyle *create(const QString &key);
};

#endif
