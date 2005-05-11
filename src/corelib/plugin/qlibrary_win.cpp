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

#include "qplatformdefs.h"
#include "qlibrary_p.h"
#include "qfile.h"
#include "qfileinfo.h"

#include "qt_windows.h"

extern QString qt_error_string(int code);

bool QLibraryPrivate::load_sys()
{
    QString attempt = fileName;
    if (QLibrary::isLibrary(fileName)) {
        QT_WA({
                pHnd = LoadLibraryW((TCHAR*)attempt.utf16());
            } , {
                  pHnd = LoadLibraryA(QFile::encodeName(attempt).data());
              });
    }

    if (pluginState != IsAPlugin) {
        if (!pHnd) {
            attempt += ".dll";
            QT_WA({
                    pHnd = LoadLibraryW((TCHAR*)attempt.utf16());
                } , {
                      pHnd = LoadLibraryA(QFile::encodeName(attempt).data());
                  });
        }
    }

#if defined(QT_DEBUG_COMPONENT)
    if (!pHnd) {
        qWarning("QLibrary::load_sys: Cannot load %s (%s)",
                 QFile::encodeName(fileName).constData(),
                 qt_error_string(GetLastError()).latin1());
    }
#endif
    if (pHnd)
        qualifiedFileName = attempt;
    return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
    if (!FreeLibrary(pHnd)) {
#if defined(QT_DEBUG_COMPONENT)
        qWarning("QLibrary::unload_sys: Cannot unload %s (%s)",
                 QFile::encodeName(fileName).constData(),
                 qt_error_string(GetLastError()).toLatin1().data());
#endif
        return false;
    }
    return true;
}

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
#ifdef Q_OS_TEMP
    void* address = (void*)GetProcAddress(pHnd, (const wchar_t*)QString(symbol).ucs2());
#else
    void* address = (void*)GetProcAddress(pHnd, symbol);
#endif
#if defined(QT_DEBUG_COMPONENT)
    if (!address)
        qWarning("QLibrary::resolve_sys: Symbol \"%s\" undefined in %s (%s)",
                 symbol,
                 QFile::encodeName(fileName).constData(),
                 qt_error_string(GetLastError()).latin1());
#endif
    return address;
}

