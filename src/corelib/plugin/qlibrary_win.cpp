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

#include "qplatformdefs.h"
#include "qlibrary_p.h"
#include "qfile.h"
#include "qfileinfo.h"

#include "qt_windows.h"

extern QString qt_error_string(int code);

bool QLibraryPrivate::load_sys()
{
    QString attempt = fileName;

    //avoid 'Bad Image' messagebox
    UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    QT_WA({
        pHnd = LoadLibraryW((TCHAR*)attempt.utf16());
    } , {
        pHnd = LoadLibraryA(QFile::encodeName(attempt).data());
    });

    
    if (pluginState != IsAPlugin) {
        if (!pHnd && ::GetLastError() == ERROR_MOD_NOT_FOUND) {
            attempt += QLatin1String(".dll");
            QT_WA({
                pHnd = LoadLibraryW((TCHAR*)attempt.utf16());
            } , {
                pHnd = LoadLibraryA(QFile::encodeName(attempt).data());
            });
        }
    }

    SetErrorMode(oldmode);
    if (!pHnd) {
        errorString = QLibrary::tr("QLibrary::load_sys: Cannot load %1 (%2)").arg(fileName).arg(::qt_error_string());
    }
    if (pHnd) {
        qualifiedFileName = attempt;
        errorString.clear();
    }
    return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
    if (!FreeLibrary(pHnd)) {
        errorString = QLibrary::tr("QLibrary::unload_sys: Cannot unload %1 (%2)").arg(fileName).arg(::qt_error_string());
        return false;
    }
    errorString.clear();
    return true;
}

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
#ifdef Q_OS_TEMP
    void* address = (void*)GetProcAddress(pHnd, (const wchar_t*)QString(symbol).ucs2());
#else
    void* address = (void*)GetProcAddress(pHnd, symbol);
#endif
    if (!address) {
        errorString = QLibrary::tr("QLibrary::resolve_sys: Symbol \"%1\" undefined in %2 (%3)").arg(
            QString::fromAscii(symbol)).arg(fileName).arg(::qt_error_string());
    } else {
        errorString.clear();
    }
    return address;
}

