#include <qguieventloop.h>
#include "private/qguieventloop_p.h"


QGuiEventLoop::QGuiEventLoop( QObject *parent, const char *name )
    : QEventLoop(new QGuiEventLoopPrivate, parent, name)
{
    init();
}

QGuiEventLoop::~QGuiEventLoop()
{
    cleanup();
}
