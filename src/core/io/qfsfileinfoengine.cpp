/****************************************************************************
**
** Implementation of QFSFileInfoEngine class
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

#include "qfileinfoengine.h"
#include "qfileinfoengine_p.h"

#define d d_func()
#define q q_func()

extern QString qt_fixToQtSlashes(const QString &path);

QFileInfoEngine::QFileInfoEngine(QFileInfoEnginePrivate &dd)  : d_ptr(&dd)
{ 
    d->q_ptr = this;
}

QFileInfoEngine::~QFileInfoEngine() 
{ 
    delete d_ptr; 
    d_ptr = 0; 
}

//**************** QFSFileInfoEnginePrivate
QFSFileInfoEnginePrivate::QFSFileInfoEnginePrivate() : QFileInfoEnginePrivate() 
{ 
    tried_stat = false;
    init();
}

//**************** QFSFileInfoEngine
QFSFileInfoEngine::QFSFileInfoEngine(const QString &file) : QFileInfoEngine(*new QFSFileInfoEnginePrivate)
{
    d->file = qt_fixToQtSlashes(file);
}

void
QFSFileInfoEngine::setFileName(const QString &file)
{
    d->file = qt_fixToQtSlashes(file);
    d->tried_stat = false;
}

QIODevice::Offset
QFSFileInfoEngine::size() const
{
    if(d->doStat())
        return (QIODevice::Offset)d->st.st_size;
    return 0;
}

QDateTime
QFSFileInfoEngine::fileTime(FileTime time) const
{
    QDateTime ret;
    if(d->doStat()) {
        if(time == CreationTime)
            ret.setTime_t(d->st.st_ctime ? d->st.st_ctime : d->st.st_mtime);
        else if(time == ModificationTime)
            ret.setTime_t(d->st.st_mtime);
        else if(time == AccessTime)
            ret.setTime_t(d->st.st_atime);
    }
    return ret;
}

