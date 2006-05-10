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

#include <qfile.h>
#include "qlibrary_p.h"
#include <qfileinfo.h>

#ifndef QT_NO_LIBRARY

#ifdef Q_OS_MAC
#  include <private/qcore_mac_p.h>
#endif

#if defined(QT_AOUT_UNDERSCORE)
#include <string.h>
#endif

#if defined(QT_HPUX_LD) // for HP-UX < 11.x and 32 bit

bool QLibraryPrivate::load_sys()
{
    pHnd = (void*)shl_load(QFile::encodeName(fileName), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
    if (pluginState != IsAPlugin) {
        if (!pHnd)
            pHnd = (void*)shl_load(QFile::encodeName(fileName + ".sl"), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
        if (!pHnd) {
            QFileInfo fi(fileName);
            pHnd = (void*)shl_load(QFile::encodeName(fi.path() + "/lib" + fi.fileName() + ".sl"),
                                   BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
        }
    }
#if defined(QT_DEBUG_COMPONENT)
    if (!pHnd)
        qWarning("QLibrary: Cannot load %s", QFile::encodeName(fileName).constData());
#endif
    return pHnd != 0;
}

bool QLibraryPrivate::unload_sys()
{
    if (shl_unload((shl_t)pHnd)) {
        qWarning("QLibrary: Cannot unload %s", QFile::encodeName(fileName).constData());
        return false;
    }
    return true;
}

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
    void* address = 0;
    if (shl_findsym((shl_t*)&pHnd, symbol, TYPE_UNDEFINED, &address) < 0) {
#if defined(QT_DEBUG_COMPONENT)
        qWarning("QLibrary: Undefined symbol \"%s\" in %s", symbol, QFile::encodeName(fileName).constData());
#endif
        address = 0;
    }
    return address;
}

#else // POSIX
#include <dlfcn.h>

#if defined(QT_DEBUG_COMPONENT)
static const char *qdlerror()
{
    const char *err = dlerror();
    return err ? err : "";
}
#endif

bool QLibraryPrivate::load_sys()
{
    QFileInfo fi(fileName);
    QString path = fi.path();
    QString name = fi.fileName();
    if (path == QLatin1String(".") && !fileName.startsWith(path))
        path.clear();
    else
        path += QLatin1Char('/');

    // The first filename we want to attempt to load is the filename as the callee specified.
    // Thus, the first attempt we do must be with an empty prefix and empty suffix.
    QStringList suffixes(QLatin1String("")), prefixes(QLatin1String(""));
    if (pluginState != IsAPlugin) {
        prefixes << QLatin1String("lib");
#if defined(Q_OS_HPUX)
        if (majorVerNum > -1) {
            suffixes << QString::fromLatin1(".sl.%1").arg(majorVerNum);
        } else {
            suffixes << QLatin1String(".sl");
        }
# if defined(__ia64)
        if (majorVerNum > -1) {
            suffixes << QString::fromLatin1(".so.%1").arg(majorVerNum);
        } else {
            suffixes << QLatin1String(".so");
        }
# endif
#elif defined(Q_OS_AIX)
        suffixes << ".a";
#else
        if (majorVerNum > -1) {
            suffixes << QString::fromLatin1(".so.%1").arg(majorVerNum);
        } else {
            suffixes << QLatin1String(".so");
        }
#endif
# ifdef Q_OS_MAC
        if (majorVerNum > -1) {
            suffixes << QString::fromLatin1(".%1.bundle").arg(majorVerNum);
            suffixes << QString::fromLatin1(".%1.dylib").arg(majorVerNum);
        } else {
            suffixes << QLatin1String(".bundle") << QLatin1String(".dylib");
        }
#endif
    }

    QString attempt;
    for(int prefix = 0; !pHnd && prefix < prefixes.size(); prefix++) {
        for(int suffix = 0; !pHnd && suffix < suffixes.size(); suffix++) {
            if (!prefixes.at(prefix).isEmpty() && name.startsWith(prefixes.at(prefix)))
                continue;
            if (!suffixes.at(suffix).isEmpty() && name.endsWith(suffixes.at(suffix)))
                continue;
            attempt = path + prefixes.at(prefix) + name + suffixes.at(suffix);
            pHnd = dlopen(QFile::encodeName(attempt), RTLD_LAZY);
        }
    }
#ifdef Q_OS_MAC
    if (!pHnd) {
        if(QCFType<CFBundleRef> bundle = CFBundleGetBundleWithIdentifier(QCFString(fileName))) {
            QCFType<CFURLRef> url = CFBundleCopyExecutableURL(bundle);
            QCFString str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            pHnd = dlopen(QFile::encodeName(str), RTLD_LAZY);
            attempt = str;
        }
    }
# endif
#if defined(QT_DEBUG_COMPONENT)
    if (!pHnd) {
        qWarning("QLibrary: Cannot load '%s' :%s", QFile::encodeName(fileName).constData(),
                 qdlerror());
    }
#endif
    if (pHnd)
        qualifiedFileName = attempt;
    return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
    if (dlclose(pHnd)) {
#if defined(QT_DEBUG_COMPONENT)
        qWarning("QLibrary: Cannot unload '%s': %s", QFile::encodeName(fileName).constData(),
                 qdlerror());
#endif
        return false;
    }
    return true;
}

#ifdef Q_OS_MAC
Q_CORE_EXPORT void *qt_mac_resolve_sys(void *handle, const char *symbol)
{
    return dlsym(handle, symbol);
}
#endif

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
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
    if (!address)
        qWarning("QLibrary: Undefined symbol \"%s\" in %s", symbol, QFile::encodeName(fileName).constData());
#endif
    return address;
}

#endif // POSIX

#endif // QT_NO_LIBRARY
