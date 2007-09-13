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

#include "qdebug.h"
#include "qfsfileengine_iterator_p.h"
#include "qfsfileengine_p.h"
#include "qplatformdefs.h"

#include <QtCore/qvariant.h>
#include <QtCore/qmutex.h>
#include <private/qmutexpool_p.h>

QT_BEGIN_NAMESPACE

class QFSFileEngineIteratorPlatformSpecificData
{
public:
    inline QFSFileEngineIteratorPlatformSpecificData()
        : uncShareIndex(-1), findFileHandle(INVALID_HANDLE_VALUE),
          done(false), uncFallback(false)
    { }

    QFSFileEngineIterator *it;

    QStringList uncShares;
    int uncShareIndex;

    HANDLE findFileHandle;
    WIN32_FIND_DATA findData;
    bool done;
    bool uncFallback;

    void advance();
    void saveCurrentFileName();
};

void QFSFileEngineIteratorPlatformSpecificData::saveCurrentFileName()
{
    if (uncFallback) {
        // Windows share / UNC path
        it->currentEntry = uncShares.at(uncShareIndex - 1);
    } else {
        // Local directory
        QT_WA({
            it->currentEntry = QString::fromUtf16((unsigned short *)findData.cFileName);
        } , {
            it->currentEntry = QString::fromLocal8Bit((const char *)findData.cFileName);
        });
    }
}

void QFSFileEngineIterator::advance()
{
    platform->saveCurrentFileName();

    if (platform->done)
        return;

    if (platform->uncFallback) {
        ++platform->uncShareIndex;
    } else if (platform->findFileHandle != INVALID_HANDLE_VALUE) {
        QT_WA({
            if (!FindNextFile(platform->findFileHandle, &platform->findData)) {
                platform->done = true;        
                FindClose(platform->findFileHandle);
            }
        } , {
            if (!FindNextFileA(platform->findFileHandle, (WIN32_FIND_DATAA *)&platform->findData)) {
                platform->done = true;
                FindClose(platform->findFileHandle);
            }
        });
    }
}

void QFSFileEngineIterator::newPlatformSpecifics()
{
    platform = new QFSFileEngineIteratorPlatformSpecificData;
    platform->it = this;
}

void QFSFileEngineIterator::deletePlatformSpecifics()
{
    delete platform;
    platform = 0;
}

bool QFSFileEngineIterator::hasNext() const
{
    if (platform->done)
        return false;
    
    if (platform->uncFallback)
        return platform->uncShareIndex > 0 && platform->uncShareIndex <= platform->uncShares.size();
    
    if (platform->findFileHandle == INVALID_HANDLE_VALUE) {
        QString path = this->path();
        // Local directory
        if (path.endsWith(QLatin1String(".lnk")))
            path = QFileInfo(path).readLink();

        if (!path.endsWith(QLatin1Char('/')))
            path.append(QLatin1Char('/'));
        path.append(QLatin1String("*.*"));

        QT_WA({
            QString fileName = QFSFileEnginePrivate::longFileName(path);
            platform->findFileHandle = FindFirstFileW((TCHAR *)fileName.utf16(),
                                                      &platform->findData);
        }, {
            // Cast is safe, since char is at end of WIN32_FIND_DATA
            platform->findFileHandle = FindFirstFileA(QFSFileEnginePrivate::win95Name(path),
                                                      (WIN32_FIND_DATAA*)&platform->findData);
        });

        if (platform->findFileHandle == INVALID_HANDLE_VALUE) {
            if (path.startsWith(QLatin1String("//"))) {
                path = this->path();
                // UNC
                QStringList parts = QDir::toNativeSeparators(path).split(QLatin1Char('\\'), QString::SkipEmptyParts);

                if (parts.count() == 1 && QFSFileEnginePrivate::uncListSharesOnServer(QLatin1String("\\\\") + parts.at(0),
                                                                                      &platform->uncShares)) {
                    if (platform->uncShares.isEmpty()) {
                        platform->done = true;
                    } else {
                        platform->uncShareIndex = 1;
                    }
                    platform->uncFallback = true;
                } else {
                    platform->done = true;
                }
            } else {
                platform->done = true;        
            }
        }

        if (!platform->done && (!platform->uncFallback || !platform->uncShares.isEmpty()))
            platform->saveCurrentFileName();
    }

    return !platform->done;
}

QT_END_NAMESPACE
