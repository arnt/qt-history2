/****************************************************************************
**
** Implementation of QFileInfoDirEngine classes
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
#include "qdirengine.h"
#include "qdirengine_p.h"

#define d d_func()
#define q q_func()

QDirEngine::QDirEngine(QDirEnginePrivate &dd)  : d_ptr(&dd)
{ 
    d->q_ptr = this;
}

QDirEngine::~QDirEngine() 
{ 
    delete d_ptr; 
    d_ptr = 0; 
}

//**************** QFSDirEnginePrivate
QFSDirEnginePrivate::QFSDirEnginePrivate() : QDirEnginePrivate() 
{ 

}

//**************** QFSDirEngine
QFSDirEngine::QFSDirEngine(const QString &path)  : QDirEngine(*new QFSDirEnginePrivate)
{
    d->path = path;
}

bool
QFSDirEngine::setPath(const QString &path, bool force)
{
    if(!force && !d->sysExists(path))
        return false;
    d->path = path;
    return true;
}

