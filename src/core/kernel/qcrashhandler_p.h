/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCRASHHANDLER_P_H
#define QCRASHHANDLER_P_H

typedef void (*QtCrashHandler)();

class QSegfaultHandler
{
    friend void qt_signal_handler(int);
    static QtCrashHandler callback;
public:
    static void initialize(char **, int);

    inline static void installCrashHandler(QtCrashHandler h) { callback = h; }
    inline static QtCrashHandler crashHandler() { return callback; }

private:
};

#endif // QCRASHHANDLER_P_H
