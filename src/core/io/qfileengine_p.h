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

#ifndef QFILEENGINE_P_H
#define QFILEENGINE_P_H

#include <qplatformdefs.h>
#include <qiodevice.h>
#include <qioengine.h>
#include <private/qioengine_p.h>

class QFileEngine;
class QFileEnginePrivate : public QIOEnginePrivate
{
protected:
    Q_DECLARE_PUBLIC(QFileEngine)
private:
    //just in case I need this later --Sam
};

class QFSFileEngine;
class QFSFileEnginePrivate : public QFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileEngine)

public:
#ifdef Q_WS_WIN
    static QString fixToQtSlashes(const QString &path);
    static QByteArray win95Name(const QString &path);
#else
    static inline QString fixToQtSlashes(const QString &path) { return path; }
#endif

    QString file;

    inline void resetErrors() {
        errorStatus = QIODevice::UnspecifiedError;
        errorString.clear();
    }
    inline void setError(QIODevice::Status status, int errorCode) {
        errorStatus = status;
        errorString = qt_errorstr(errorCode);
    }
    inline void setError(QIODevice::Status status, QString error = QString()) {
        errorStatus = status;
        errorString = error;
    }
    QIODevice::Status errorStatus;
    QString errorString;

    int fd;
    mutable uint sequential : 1;
    mutable uint external_file : 1;
    QByteArray ungetchBuffer;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
    mutable QT_STATBUF st;
    bool doStat() const;
    int sysOpen(const QString &, int flags);

protected:
    QFSFileEnginePrivate();

    void init();

#if defined(Q_OS_WIN32)
    uint getPermissions() const;
    QString getLink() const;
#endif
};

#endif // QFILEENGINE_P_H
