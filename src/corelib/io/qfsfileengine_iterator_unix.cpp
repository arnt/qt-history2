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

#include "qfsfileengine_iterator_p.h"
#include "qplatformdefs.h"

#include <QtCore/qvariant.h>

class QFSFileEngineIteratorPlatformSpecificData
{
public:
    inline QFSFileEngineIteratorPlatformSpecificData()
        : dir(0), dirEntry(0), done(false)
    { }

    DIR *dir;
    dirent *dirEntry;
    bool done;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    // for readdir_r
    union {
        struct dirent mt_file;
        char b[sizeof(struct dirent) + MAXNAMLEN + 1];
    } u;
#endif
};

void QFSFileEngineIterator::advance()
{
    currentEntry = platform->dirEntry ? QFile::decodeName(QByteArray(platform->dirEntry->d_name)) : QString();

    if (!platform->dir)
        return;
    
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    if (::readdir_r(platform->dir, &platform->u.mt_file, &platform->dirEntry) != 0)
        platform->done = true;
#else
    // ### add local lock to prevent breaking reentrancy
    platform->dirEntry = ::readdir(platform->dir);
#endif // _POSIX_THREAD_SAFE_FUNCTIONS
    if (!platform->dirEntry) {
        ::closedir(platform->dir);
        platform->dir = 0;
        platform->done = true;
    }
}

void QFSFileEngineIterator::newPlatformSpecifics()
{
    platform = new QFSFileEngineIteratorPlatformSpecificData;
}

void QFSFileEngineIterator::deletePlatformSpecifics()
{
    if (platform->dir)
        ::closedir(platform->dir);
    delete platform;
    platform = 0;
}

bool QFSFileEngineIterator::hasNext() const
{
    if (!platform->done && !platform->dir) {
        QFSFileEngineIterator *that = const_cast<QFSFileEngineIterator *>(this);
        if ((that->platform->dir = ::opendir(QFile::encodeName(path()).data())) == 0)
            that->platform->done = true;
        else
            that->advance();
    }
    return !platform->done;
}
