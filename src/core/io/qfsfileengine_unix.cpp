/****************************************************************************
**
** Implementation of QFFileEngine class for Unix
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
#include <qfileengine.h>
#include "qfileengine_p.h"
#include <qplatformdefs.h>
#include <qfile.h>

#define d d_func()
#define q q_func()

QString qt_fixToQtSlashes(const QString &path)
{
    return path;
}

void 
QFSFileEnginePrivate::init()
{
}

int
QFSFileEnginePrivate::sysOpen(const QString &fileName, int flags)
{
    return ::open(QFile::encodeName(fileName), flags, 0666);
}

bool
QFSFileEngine::remove()
{
    return unlink(QFile::encodeName(d->file)) == 0;
}

bool 
QFSFileEngine::rename(const QString &newName)
{
    return ::rename(QFile::encodeName(d->file), QFile::encodeName(newName)) == 0;
}

QFile::Offset
QFSFileEngine::size() const
{
    QT_STATBUF st;
    int ret = 0;
    if(d->fd != -1) 
        ret = QT_FSTAT(d->fd, &st);
    else 
        ret = QT_STAT(QFile::encodeName(d->file), &st);
    if (ret == -1)
        return 0;
    return st.st_size;
}

uchar
*QFSFileEngine::map(Q_LONG /*len*/)
{
    return 0;
}




