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

#define QT_DEBUG_COMPONENT
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
    if (!pHnd)
        pHnd = (void*)shl_load(QFile::encodeName(fileName + ".sl"), BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
    if (!pHnd) {
        QFileInfo fi(fileName);
        pHnd = (void*)shl_load(QFile::encodeName(fi.path() + "/lib" + fi.fileName() + ".sl"),
                               BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH, 0);
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

bool QLibraryPrivate::load_sys()
{
    QFileInfo fi(fileName);
    QString path = fi.path();
    QString name = fi.fileName();
    if (path == QLatin1String(".") && !fileName.startsWith(path))
        path.clear();
    else
        path += QLatin1Char('/');

    QStringList suffixes(""), prefixes("");
    prefixes << "lib";
#if defined(Q_OS_HPUX)
    suffixes << ".sl";
#elif defined(Q_OS_AIX)
    suffixes << ".a";
#else
    suffixes << ".so";
#endif
# ifdef Q_OS_MAC
    suffixes << ".bundle" << ".dylib";
#endif
    for(int prefix = 0; !pHnd && prefix < prefixes.size(); prefix++) {
        for(int suffix = 0; !pHnd && suffix < suffixes.size(); suffix++) {
            QString attempt(path + prefixes[prefix] + name + suffixes[suffix]);
            QFileInfo attempt_fi(attempt);
            if(attempt_fi.exists() && !attempt_fi.isDir())
                pHnd = dlopen(QFile::encodeName(attempt), RTLD_LAZY);
        }
    }
#ifdef Q_OS_MAC
    if (!pHnd) {
        if(QCFType<CFBundleRef> bundle = CFBundleGetBundleWithIdentifier(QCFString(fileName))) {
            QCFType<CFURLRef> url = CFBundleCopyExecutableURL(bundle);
            QCFString str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
            qDebug("attemping %s", QFile::encodeName(str).data());
            pHnd = dlopen(QFile::encodeName(str), RTLD_LAZY);
        }
    }
# endif
#if defined(QT_DEBUG_COMPONENT)
    if (!pHnd)
        qWarning("QLibrary: Cannot load '%s' :%s", QFile::encodeName(fileName).constData(), dlerror());
#endif
    return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
    if (dlclose(pHnd)) {
        qWarning("QLibrary: Cannot unload '%s': %s", QFile::encodeName(fileName).constData(), dlerror());
        return false;
    }
    return true;
}

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

