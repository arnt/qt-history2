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

#ifndef QSCRIPTNAMEID_P_H
#define QSCRIPTNAMEID_P_H

class QScriptNameIdImpl
{
public:
    QString s;
    uint h;
    QScriptNameIdImpl *next;
    uint used: 1;
    uint persistent: 1;
    uint unique: 1;
    uint pad: 29;

    inline QScriptNameIdImpl(const QString &_s):
        s(_s), h(0), next(0), used(0), persistent(0), unique(0), pad(0) { }
};

#endif // QSCRIPTNAMEID_P_H
