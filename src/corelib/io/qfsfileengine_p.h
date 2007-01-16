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

#ifndef QFSFILEENGINE_P_H
#define QFSFILEENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qplatformdefs.h"
#include "QtCore/qfsfileengine.h"
#include "private/qabstractfileengine_p.h"

class QFSFileEnginePrivate : public QAbstractFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileEngine)

public:
#ifdef Q_WS_WIN
    static QByteArray win95Name(const QString &path);
    static QString longFileName(const QString &path);
#endif

    QString filePath;
    QByteArray nativeFilePath;
    QIODevice::OpenMode openMode;

    void nativeInitFileName();
    bool nativeOpen(QIODevice::OpenMode openMode);
    bool openFh(QIODevice::OpenMode flags, FILE *fh);
    bool openFd(QIODevice::OpenMode flags, int fd);
    bool nativeClose();
    bool closeFdFh();
    bool nativeFlush();
    bool flushFh();
    qint64 nativeSize() const;
    qint64 sizeFdFh() const;
    qint64 nativePos() const;
    qint64 posFdFh() const;
    bool nativeSeek(qint64);
    bool seekFdFh(qint64);
    qint64 nativeRead(char *data, qint64 maxlen);
    qint64 readFdFh(char *data, qint64 maxlen);
    qint64 nativeReadLine(char *data, qint64 maxlen);
    qint64 readLineFdFh(char *data, qint64 maxlen);
    qint64 nativeWrite(const char *data, qint64 len);
    qint64 writeFdFh(const char *data, qint64 len);
    int nativeHandle() const;
    bool nativeIsSequential() const;
    bool isSequentialFdFh() const;

    int fd;
    FILE *fh;
#ifdef Q_WS_WIN
    HANDLE fileHandle;
#endif

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
#ifdef Q_OS_UNIX
    mutable uint need_lstat : 1;
    mutable uint is_link : 1;
#endif
#ifdef Q_WS_WIN
    mutable DWORD fileAttrib;
#else
    mutable QT_STATBUF st;
#endif
    bool doStat() const;
    bool isSymlink() const;

#if defined(Q_OS_WIN32)
    int sysOpen(const QString &, int flags);
#endif

    bool closeFileHandle;
    enum LastIOCommand
    {
        IOFlushCommand,
        IOReadCommand,
        IOWriteCommand
    };
    LastIOCommand  lastIOCommand;
    bool lastFlushFailed;
#if defined(Q_OS_WIN32)
    static void resolveLibs();
    static bool resolveUNCLibs_NT();
    static bool resolveUNCLibs_9x();
    static bool uncListSharesOnServer(const QString &server, QStringList *list);
#endif

protected:
    QFSFileEnginePrivate();

    void init();

#if defined(Q_OS_WIN32)
    QAbstractFileEngine::FileFlags getPermissions() const;
    QString getLink() const;
#endif
};

#endif // QFSFILEENGINE_P_H
