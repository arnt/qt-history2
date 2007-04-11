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

#ifndef ECHOINTERFACE_H
#define ECHOINTERFACE_H

class QString;

class EchoInterface
{
public:
    virtual ~EchoInterface() {}
    virtual QString echo(const QString &message) = 0;
};

Q_DECLARE_INTERFACE(EchoInterface,
                    "com.trolltech.Plugin.EchoInterface/1.0");

#endif
