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
#include "private/qlibrary_p.h"

#ifndef QT_NO_LIBRARY

#if defined(QT_AOUT_UNDERSCORE)
#include <string.h>
#endif

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/

#if defined(QT_HPUX_LD) // for HP-UX < 11.x and 32 bit

bool QLibraryPrivate::loadLibrary()
{
    if (pHnd)
        return true;

    QString filename = library->library();

    pHnd = (void*)shl_load(filename.latin1(), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if (!pHnd)
        qWarning("%s: failed to load library!", filename.latin1());
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if (!pHnd)
        return true;

    if (shl_unload((shl_t)pHnd)) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
        QString filename = library->library();
        qWarning("%s: Failed to unload library!", filename.latin1());
#endif
        return false;
    }
    pHnd = 0;
    return true;
}

void* QLibraryPrivate::resolveSymbol(const char* symbol)
{
    if (!pHnd)
        return 0;

    void* address = 0;
    if (shl_findsym((shl_t*)&pHnd, symbol, TYPE_UNDEFINED, &address) < 0) {
#if defined(QT_DEBUG_COMPONENT)
        QString filename = library->library();
        qWarning("%s: couldn't resolve symbol \"%s\"", filename.latin1(), symbol);
#endif
    }
    return address;
}

#else // POSIX
#include <dlfcn.h>

bool QLibraryPrivate::loadLibrary()
{
    if (pHnd)
        return true;

    QString filename = library->library();

    pHnd = dlopen(filename.latin1(), RTLD_LAZY);
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    if (!pHnd)
        qWarning("%s", dlerror());
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if (!pHnd)
        return true;

    if (dlclose(pHnd)) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
        qWarning("%s", dlerror());
#endif
        return false;
    }

    pHnd = 0;
    return true;
}

void* QLibraryPrivate::resolveSymbol(const char* symbol)
{
    if (!pHnd)
        return 0;

#if defined(QT_AOUT_UNDERSCORE)
    // older a.out systems add an underscore in front of symbols
    char* undrscr_symbol = new char[strlen(symbol)+2];
    undrscr_symbol[0] = '_';
    strcpy(undrscr_symbol+1, symbol);
    void* address = dlsym(pHnd, undrscr_symbol);
    delete [] undrscr_symbol;
#else
    void* address = dlsym(pHnd, symbol);
#endif
#if defined(QT_DEBUG_COMPONENT)
    const char* error = dlerror();
    if (error)
        qWarning("%s", error);
#endif
    return address;
}

#endif // POSIX

#endif
