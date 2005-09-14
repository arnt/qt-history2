/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIGNALDUMPER_H
#define QSIGNALDUMPER_H

class QByteArray;

class QSignalDumper
{
public:
    static void startDump();
    static void endDump();

    static void ignoreClass(const QByteArray &klass);
    static void clearIgnoredClasses();
};

#endif

