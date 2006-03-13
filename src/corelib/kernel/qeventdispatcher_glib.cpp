#include "qeventdispatcher_glib_p.h"

#include <private/qthread_p.h>

#include "qcoreapplication.h"
#include "qsocketnotifier.h"

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qpair.h>

#include <glib.h>

struct GSocketNotifierSource
{
    GSource source;
    GPollFD pollfd;
    QSocketNotifier *socketNotifier;
};

static gboolean socketNotifierSourcePrepare(GSource *, gint *timeout)
{
    if (timeout)
        *timeout = -1;
    return false;
}

static gboolean socketNotifierSourceCheck(GSource *source)
{
    return reinterpret_cast<GSocketNotifierSource *>(source)->pollfd.revents != 0;
}

static gboolean socketNotifierSourceDispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
    QEvent event(QEvent::SockAct);
    QCoreApplication::sendEvent(reinterpret_cast<GSocketNotifierSource *>(source)->socketNotifier, &event);
    if (callback)
        return callback(user_data);
    return true; // ??? don't remove, right?
}

static GSourceFuncs socketNotifierSourceFuncs = {
    socketNotifierSourcePrepare,
    socketNotifierSourceCheck,
    socketNotifierSourceDispatch,
    NULL,
    NULL,
    NULL
};

struct GTimerSource
{
    GSource *source;
    int timerId;
    int interval;
    QObject *object;
};

static gboolean timerSourceCallback(gpointer data)
{
    GTimerSource *source = reinterpret_cast<GTimerSource *>(data);
    QTimerEvent event(source->timerId);
    QCoreApplication::sendEvent(source->object, &event);
    return true;
}

struct GPostEventSource
{
    GSource source;
    GPollFD pollfd;
    int wakeUpPipe[2];
    QEventLoop::ProcessEventsFlags flags;
};

static gboolean postEventSourcePrepare(GSource *, gint *timeout)
{
    QThreadData *data = QThreadData::get(QThread::currentThread());
    if (!data)
        return false;
    gint dummy;
    if (!timeout)
        timeout = &dummy;
    return (*timeout = data->canWait ? -1 : 0) == 0;
}

static gboolean postEventSourceCheck(GSource *source)
{
    return postEventSourcePrepare(0, 0) || reinterpret_cast<GPostEventSource *>(source)->pollfd.revents != 0;
}

static gboolean postEventSourceDispatch(GSource *s, GSourceFunc, gpointer)
{
    GPostEventSource *source = reinterpret_cast<GPostEventSource *>(s);

    char c[16];
    while (::read(source->wakeUpPipe[0], c, sizeof(c)) > 0)
        ;

    QCoreApplication::sendPostedEvents(0, (source->flags & QEventLoop::DeferredDeletion) ? -1 : 0);
    return true; // ??? don't remove, right again?
}

static GSourceFuncs postEventSourceFuncs = {
    postEventSourcePrepare,
    postEventSourceCheck,
    postEventSourceDispatch,
    NULL,
    NULL,
    NULL
};


QEventDispatcherGlibPrivate::QEventDispatcherGlibPrivate()
{
    QCoreApplication *app = QCoreApplication::instance();
    mainContext = (app && QThread::currentThread() == app->thread()
                   ? g_main_context_default()
                   : g_main_context_new());
    g_main_context_ref(mainContext);

    postEventSource = reinterpret_cast<GPostEventSource *>(g_source_new(&postEventSourceFuncs,
                                                                        sizeof(GPostEventSource)));
    g_source_set_can_recurse(&postEventSource->source, true);

    pipe(postEventSource->wakeUpPipe);
    fcntl(postEventSource->wakeUpPipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(postEventSource->wakeUpPipe[1], F_SETFD, FD_CLOEXEC);
    fcntl(postEventSource->wakeUpPipe[0], F_SETFL,
          fcntl(postEventSource->wakeUpPipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(postEventSource->wakeUpPipe[1], F_SETFL,
          fcntl(postEventSource->wakeUpPipe[1], F_GETFL) | O_NONBLOCK);

    postEventSource->pollfd.fd = postEventSource->wakeUpPipe[0];
    postEventSource->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
    postEventSource->flags = QEventLoop::AllEvents;

    g_source_add_poll(&postEventSource->source, &postEventSource->pollfd);
    g_source_attach(&postEventSource->source, mainContext);
}

QEventDispatcherGlib::QEventDispatcherGlib(QObject *parent)
    : QAbstractEventDispatcher(*(new QEventDispatcherGlibPrivate), parent)
{
}

QEventDispatcherGlib::~QEventDispatcherGlib()
{
    Q_D(QEventDispatcherGlib);

    // destroy all timer and socket notifier sources
    for (int i = 0; i < d->timerSources.count(); ++i) {
        GTimerSource *s = d->timerSources.at(i);
        g_source_destroy(s->source);
        delete s;
    }
//     for (int i = 0; i < d->socketNotifierSources.count(); ++i) {
//         GSocketNotifierSource *s = d->socketNotifierSources.at(i);
    foreach (GSocketNotifierSource *s, d->socketNotifierSources) {
        g_source_remove_poll(&s->source, &s->pollfd);
        g_source_destroy(&s->source);
    }

    // destroy post event source
    g_source_remove_poll(&d->postEventSource->source, &d->postEventSource->pollfd);
    close(d->postEventSource->wakeUpPipe[0]);
    close(d->postEventSource->wakeUpPipe[1]);
    d->postEventSource->wakeUpPipe[0] = 0;
    d->postEventSource->wakeUpPipe[1] = 0;

    g_source_destroy(&d->postEventSource->source);
    d->postEventSource = 0;

    g_main_context_unref(d->mainContext);
    d->mainContext = 0;
}

bool QEventDispatcherGlib::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherGlib);

    const bool canWait = (flags & QEventLoop::WaitForMoreEvents);
    if (canWait)
        emit aboutToBlock();
    else
        emit awake();

    QEventLoop::ProcessEventsFlags saved_flags = d->postEventSource->flags;
    d->postEventSource->flags = flags;
    bool result = g_main_context_iteration(d->mainContext, canWait);
    while (!result && canWait)
        result = g_main_context_iteration(d->mainContext, canWait);
    d->postEventSource->flags = saved_flags;

    if (canWait)
        emit awake();

    return result;
}

bool QEventDispatcherGlib::hasPendingEvents()
{
    Q_D(QEventDispatcherGlib);
    return g_main_context_pending(d->mainContext);
}

void QEventDispatcherGlib::registerSocketNotifier(QSocketNotifier *socketNotifier)
{
    int sockfd;
    int type;
    if (!socketNotifier
        || (sockfd = socketNotifier->socket()) < 0
        || sockfd > FD_SETSIZE
        || (type = socketNotifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (socketNotifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
        return;
    }

    Q_D(QEventDispatcherGlib);

    GSocketNotifierSource *source =
        reinterpret_cast<GSocketNotifierSource *>(g_source_new(&socketNotifierSourceFuncs,
                                                               sizeof(GSocketNotifierSource)));
    g_source_set_can_recurse(&source->source, true);

    source->pollfd.fd = sockfd;
    switch (type) {
    case QSocketNotifier::Read:
        source->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
        break;
    case QSocketNotifier::Write:
        source->pollfd.events = G_IO_OUT | G_IO_ERR;
        break;
    case QSocketNotifier::Exception:
        source->pollfd.events = G_IO_PRI | G_IO_ERR;
        break;
    }
    source->socketNotifier = socketNotifier;
    d->socketNotifierSources.insert(socketNotifier, source);

    g_source_add_poll(&source->source, &source->pollfd);
    g_source_attach(&source->source, d->mainContext);
}

void QEventDispatcherGlib::unregisterSocketNotifier(QSocketNotifier *socketNotifier)
{
    int sockfd;
    int type;
    if (!socketNotifier
        || (sockfd = socketNotifier->socket()) < 0
        || sockfd > FD_SETSIZE
        || (type = socketNotifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (socketNotifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
        return;
    }

    Q_D(QEventDispatcherGlib);

    GSocketNotifierSource *source = d->socketNotifierSources.take(socketNotifier);
    if (!source)
        return; // oops

    g_source_remove_poll(&source->source, &source->pollfd);
    g_source_destroy(&source->source);
}

void QEventDispatcherGlib::registerTimer(int timerId, int interval, QObject *object)
{
    if (timerId < 1 || interval < 0 || !object) {
        qWarning("QEventDispatcherUNIX::registerTimer: invalid arguments");
        return;
    } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::startTimer: timers cannot be started from another thread");
        return;
    }

    Q_D(QEventDispatcherGlib);

    GTimerSource *source = new GTimerSource;
    source->source = g_timeout_source_new(interval);
    g_source_set_can_recurse(source->source, true);

    source->timerId = timerId;
    source->interval = interval;
    source->object = object;
    d->timerSources.append(source);

    g_source_set_callback(source->source, timerSourceCallback, source, 0);
    g_source_attach(source->source, d->mainContext);
}

bool QEventDispatcherGlib::unregisterTimer(int timerId)
{
    if (timerId < 1) {
        qWarning("QEventDispatcherUNIX::unregisterTimer: invalid argument");
        return false;
    } else if (thread() != QThread::currentThread()) {
        qWarning("QObject::killTimer: timers cannot be stopped from another thread");
        return false;
    }

    Q_D(QEventDispatcherGlib);

    GTimerSource *source = 0;
    for (int i = 0; i < d->timerSources.count(); ++i) {
        GTimerSource *s = d->timerSources.at(i);
        if (s->timerId == timerId) {
            d->timerSources.removeAt(i);
            source = s;
            break;
        }
    }
    if (!source)
        return false;

    g_source_destroy(source->source);
    delete source;

    return true;
}

bool QEventDispatcherGlib::unregisterTimers(QObject *object)
{
    if (!object) {
        qWarning("QEventDispatcherUNIX::unregisterTimers: invalid argument");
        return false;
    } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::killTimers: timers cannot be stopped from another thread");
        return false;
    }

    Q_D(QEventDispatcherGlib);

    bool returnValue = false;
    for (int i = 0; i < d->timerSources.count(); ++i) {
        GTimerSource *source = d->timerSources.at(i);
        if (source->object != object)
            continue;

        d->timerSources.removeAt(i--);
        g_source_destroy(source->source);
        delete source;

        returnValue = true;
    }

    return returnValue;
}

QList<QEventDispatcherGlib::TimerInfo> QEventDispatcherGlib::registeredTimers(QObject *object) const
{
    if (!object) {
        qWarning("QEventDispatcherUNIX:registeredTimers: invalid argument");
        return QList<TimerInfo>();
    }

    Q_D(const QEventDispatcherGlib);

    QList<TimerInfo> returnValue;
    for (int i = 0; i < d->timerSources.count(); ++i) {
        GTimerSource *source = d->timerSources.at(i);
        if (source->object != object)
            continue;

        returnValue.append(TimerInfo(source->timerId, source->interval));
    }

    return returnValue;
}

void QEventDispatcherGlib::interrupt()
{
    wakeUp();
}

void QEventDispatcherGlib::wakeUp()
{
    Q_D(QEventDispatcherGlib);
    char c = 0;
    ::write(d->postEventSource->wakeUpPipe[1], &c, 1);
}

void QEventDispatcherGlib::flush()
{
}

QEventDispatcherGlib::QEventDispatcherGlib(QEventDispatcherGlibPrivate &dd, QObject *parent)
    : QAbstractEventDispatcher(dd, parent)
{
}
