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
