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

bool QLibraryPrivate::load_sys()
{
    QT_WA({
        pHnd = LoadLibraryW((TCHAR*)fileName.utf16());
    } , {
        pHnd = LoadLibraryA(QFile::encodeName(fileName).data());
    });

    if (!pHnd) {
        QString name = fileName + ".dll";
        QT_WA({
            pHnd = LoadLibraryW((TCHAR*)name.utf16());
        } , {
            pHnd = LoadLibraryA(QFile::encodeName(name).data());
        });
    }

    if (!pHnd)
        qErrnoWarning("QLibrary::load_sys: Cannot load %s", QFile::encodeName(fileName).constData());
    return pHnd != 0;
}

bool QLibraryPrivate::unload_sys()
{
    if (!FreeLibrary(pHnd)) {
        qErrnoWarning("QLibrary::unload_sys: Cannot unload %s", QFile::encodeName(fileName).constData());
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
        qErrnoWarning("QLibrary::resolve_sys: Symbol \"%s\" undefined in %s", symbol,
                  QFile::encodeName(fileName).constData());
#endif
    return address;
}

