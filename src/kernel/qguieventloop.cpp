#include <qguieventloop.h>
#include "private/qguieventloop_p.h"


QGuiEventLoop::QGuiEventLoop( QObject *parent = 0, const char *name = 0 )
    : QEventLoop(new QGuiEventLoopPrivate, parent, name)
{
}

QGuiEventLoop::~QGuiEventLoop()
{
}
