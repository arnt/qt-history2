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

#include <qplatformdefs.h>
#include <qiodevice.h>

class QFileEngine;
class QFileEnginePrivate
{
protected:
    QFileEngine *q_ptr;
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
        error = QFile::UnspecifiedError;
        errorString.clear();
    }
    inline void setError(QFile::Error err, int errorCode) {
        error = err;
        errorString = qt_error_string(errorCode);
    }
    inline void setError(QFile::Error err, QString errStr = QString()) {
        error = err;
        errorString = errStr;
    }
    QFile::Error error;
    QString errorString;

    int fd;
    mutable uint sequential : 1;
    mutable uint external_file : 1;
    QByteArray ungetchBuffer;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
#ifdef Q_OS_UNIX
    mutable uint isSymLink : 1;
#endif
    mutable QT_STATBUF st;
    bool doStat() const;
    int sysOpen(const QString &, int flags);

protected:
    QFSFileEnginePrivate();

    void init();

#if defined(Q_OS_WIN32)
    QFileEngine::FileFlags getPermissions() const;
    QString getLink() const;
#endif
};

#endif // QFILEENGINE_P_H
