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

#ifndef QFILEINFO_P_H
#define QFILEINFO_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QIODevice. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qfileinfo.h"

class QFileInfoPrivate
{
public:
    QFileInfoPrivate(const QFileInfo *copy=0);
    ~QFileInfoPrivate();

    void initFileEngine(const QString &);

    enum Access {
        ReadAccess,
        WriteAccess,
        ExecuteAccess
    };
    bool hasAccess(Access access) const;

    uint getFileFlags(QAbstractFileEngine::FileFlags) const;
    QDateTime &getFileTime(QAbstractFileEngine::FileTime) const;
    QString getFileName(QAbstractFileEngine::FileName) const;

    enum {
        CachedFileFlags = 0x01,
        CachedLinkTypeFlag = 0x02,
        CachedMTime = 0x10,
        CachedCTime = 0x20,
        CachedATime = 0x40,
        CachedSize = 0x08
    };

    struct Data
    {
        inline Data()
            : ref(1), fileEngine(0), cache_enabled(1)
        {
            clear();
        }

        inline Data(const Data &copy)
            : ref(1), fileEngine(QAbstractFileEngine::create(copy.fileName)),
              fileName(copy.fileName), cache_enabled(copy.cache_enabled)
        {
            clear();
        }

        inline ~Data()
        {
            delete fileEngine;
        }

        inline void clear()
        {
            fileNames.clear();
            fileFlags = 0;
            cachedFlags = 0;
        }

        mutable QAtomic ref;

        QAbstractFileEngine *fileEngine;
        mutable QString fileName;
        mutable QHash<int, QString> fileNames;
        mutable uint cachedFlags : 31;
        mutable uint cache_enabled : 1;
        mutable uint fileFlags;
        mutable qint64 fileSize;
        mutable QDateTime fileTimes[3];

        inline bool getCachedFlag(uint c) const
        { return cache_enabled ? (cachedFlags & c) : 0; }

        inline void setCachedFlag(uint c)
        { if (cache_enabled) cachedFlags |= c; }
    } *data;

    inline void reset() {
        detach();
        data->clear();
    }

    void detach();
};

#endif

