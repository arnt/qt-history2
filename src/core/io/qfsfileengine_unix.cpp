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
QFSFileEngine::remove(const QString &fileName)
{
    return unlink(QFile::encodeName(fileName)) == 0;
}

uchar
*QFSFileEngine::map(Q_ULONG len)
{
    return 0;
}




