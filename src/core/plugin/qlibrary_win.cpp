/****************************************************************************
**
** Implementation of QLibraryPrivate class.
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

#include "qplatformdefs.h"
#include "qlibrary_p.h"
#include <qfile.h>

#include "qt_windows.h"

bool QLibraryPrivate::load_sys()
{
    QT_WA({
        pHnd = LoadLibraryW((TCHAR*)fileName.utf16());
    } , {
        pHnd = LoadLibraryA(QFile::encodeName(fileName).data());
    });
    if (!pHnd)
        qSystemWarning("Failed to load library %s", QFile::encodeName(fileName).data());
    return pHnd != 0;
}

bool QLibraryPrivate::unload_sys()
{
    if (!FreeLibrary(pHnd)) {
        qSystemWarning("Failed to unload library");
        return false;
    }
    return return true;
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
        qSystemWarning("Couldn't resolve symbol \"%s\"", symbol);
#endif
    return address;
}

