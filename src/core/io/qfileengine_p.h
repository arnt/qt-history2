/****************************************************************************
**
** Definition of private QFileEngine classes.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef __QFILEENGINE_P_H__
#define __QFILEENGINE_P_H__

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
protected:
    QFSFileEnginePrivate();

    void init();
    int sysOpen(const QString &, int flags);
#ifdef Q_WS_WIN
    QString fixToQtSlashes(const QString &path);
    QByteArray win95Name(const QString &path)
#else
    inline QString fixToQtSlashes(const QString &path) { return path; }
#endif
private:
    QString file;

    inline void resetErrors() {
        errorStatus = QIODevice::UnspecifiedError;
        errorMessage = QString::null;
    }
    inline void setError(QIODevice::Status status, int errorCode) {
        errorStatus = status;
        errorMessage = qt_errorstr(errorCode);
    }
    inline void setError(QIODevice::Status status, QString error=QString::null) {
        errorStatus = status;
        errorMessage = error;
    }
    QIODevice::Status errorStatus;
    QString errorMessage;

    int fd;
    mutable uint sequential : 1;
    mutable uint external_file : 1;
    QByteArray ungetchBuffer;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
    mutable QT_STATBUF st;
    bool doStat() const;

#if defined(Q_OS_WIN32)
    uint getPermissions() const;
    QString getLink() const;
#endif

};

#endif /* __QFILEENGINE_P_H__ */
