/****************************************************************************
**
** Definition of private QFileInfoEngine classes.
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
#ifndef __QFILEINFOENGINE_P_H__
#define __QFILEINFOENGINE_P_H__

#include <qplatformdefs.h>

class QFileInfoEngine;
class QFileInfoEnginePrivate
{
protected:
    QFileInfoEngine *q_ptr;
private:
    Q_DECLARE_PUBLIC(QFileInfoEngine)
protected:
    inline QFileInfoEnginePrivate(QFileInfoEngine *qq) : q_ptr(qq) { }
    ~QFileInfoEnginePrivate() { q_ptr = 0; }
};


class QFSFileInfoEngine;
class QFSFileInfoEnginePrivate : public QFileInfoEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileInfoEngine)
protected:
    QFSFileInfoEnginePrivate(QFSFileInfoEngine *qq);

    void init();
private:
    QString file;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
    mutable QT_STATBUF st;
    bool doStat() const;

#if defined(Q_OS_WIN32)
    uint getPermissions() const;
    QString getLink() const;
#endif
};

#endif /* __QFILEINFOENGINE_P_H__ */
