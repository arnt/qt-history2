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

/*!
    \class QGuiEventLoop
    \brief The QGuiEventLoop class manages Qt's event queue, including GUI-related events.

    It inherits most of its interface from QEventLoop and implements
    some platform-specific, GUI-related functionality.

    \sa QApplication
*/

/*
    Constructs a QGuiEventLoop object. This object becomes the global
    event loop object. There can only be one event loop object. The
    QGuiEventLoop is usually constructed by calling
    QApplication::eventLoop(). If you want to create your own event
    loop object you \e must create it before you instantiate the
    QApplication object.

    The \a parent argument is passed on to the QObject constructor.
*/
QGuiEventLoop::QGuiEventLoop(QObject *parent)
    : QEventLoop(*new QGuiEventLoopPrivate, parent)
{
    init();
}

/*
    Destructs the QGuiEventLoop object.
*/
QGuiEventLoop::~QGuiEventLoop()
{
    cleanup();
}
