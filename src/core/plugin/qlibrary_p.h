/****************************************************************************
**
** Definition of an internal QLibrary class.
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

#ifndef QLIBRARY_P_H
#define QLIBRARY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//
//


#ifdef Q_WS_WIN
# include "qt_windows.h"
#endif


#ifndef QT_H
#include "qlibrary.h"
#include "qpointer.h"
#include "qstringlist.h"
#endif // QT_H

class QLibraryPrivate
{
public:

#ifdef Q_WS_WIN
    HINSTANCE
#else
    void *
#endif
    pHnd;

    QString fileName;

    bool load();
    bool loadPlugin(); // loads and resolves instance
    bool unload();
    void release();
    void *resolve(const char *);

    static QString findLib(const QString &fileName);
    static QLibraryPrivate *findOrCreate(const QString &canonicalFileName);

    typedef QObject *(*InstanceFn)();
    InstanceFn instance;
    uint qt_version;
    QString lastModified;

    bool isPlugin();


private:
    QLibraryPrivate(const QString &canonicalFileName);
    ~QLibraryPrivate();

    bool load_sys();
    bool unload_sys();
    void *resolve_sys(const char *);

    QAtomic libraryRefCount;
    QAtomic libraryUnloadCount;

    enum {IsAPlugin, IsNotAPlugin, MightBeAPlugin } pluginState;
    friend class QLibraryPrivateHasFriends;
};

#endif // QLIBRARY_P_H
