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

#include <qguieventloop.h>
#include "private/qguieventloop_p.h"


QGuiEventLoop::QGuiEventLoop(QObject *parent)
    : QEventLoop(*new QGuiEventLoopPrivate, parent)
{
    init();
}

QGuiEventLoop::~QGuiEventLoop()
{
    cleanup();
}

// ##### Huge Hack

#include <private/qthread_p.h>
extern QThreadData *qt_getMainData();

QGuiEventLoopPrivate::QGuiEventLoopPrivate()
{
    QThreadData::setCurrent(qt_getMainData());
}
