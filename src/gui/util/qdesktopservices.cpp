/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesktopservices.h"

#include <qdebug.h>

#if defined(Q_WS_QWS)
#include "qdesktopservices_qws.cpp"
#elif defined(Q_WS_X11)
#include "qdesktopservices_x11.cpp"
#elif defined(Q_WS_WIN)
#include "qdesktopservices_win.cpp"
#elif defined(Q_WS_MAC)
#include "qdesktopservices_mac.cpp"
#endif

#include <qhash.h>
#include <qobject.h>
#include <qcoreapplication.h>

class QOpenUrlHandlerRegistry : public QObject
{
    Q_OBJECT
public:
    struct Handler
    {
        QObject *receiver;
        const char *name;
    };
    typedef QHash<QString, Handler> HandlerHash;
    HandlerHash handlers;

public Q_SLOTS:
    void handlerDestroyed(QObject *handler);

};

Q_GLOBAL_STATIC(QOpenUrlHandlerRegistry, handlerRegistry)

void QOpenUrlHandlerRegistry::handlerDestroyed(QObject *handler)
{
    HandlerHash::Iterator it = handlers.begin();
    while (it != handlers.end()) {
        if (it->receiver == handler) {
            it = handlers.erase(it);
        } else {
            ++it;
        }
    }
}

/*!
    \class QDesktopServices
    \brief The QDesktopServices class provides access to system wide services such as opening document and launching a web browser
    \since 4.2
    \ingroup application
*/

/*!
    Opens the \a url in either the default web browser or an application that can handle it
    and returns true on success otherwise false.

    Passing a mailto url will result in a e-mail composer window opening in the default
    e-mail client similar to when a user clicks on a mailto link in a web browser.

    Example mailto url:
    \code
    "mailto:user@foo.com?subject=Test&body=Just a test"
    \endcode

    Note: Only some e-mail clients support @attachment and can handle unicode.

    \sa setUrlHandler()
*/
bool QDesktopServices::openUrl(const QUrl &url)
{
    static bool insideHandlerCall = false;

    if (!insideHandlerCall) {
        QOpenUrlHandlerRegistry::HandlerHash::ConstIterator handler = handlerRegistry()->handlers.find(url.scheme());
        if (handler != handlerRegistry()->handlers.end()) {
            insideHandlerCall = true;
            bool result = QMetaObject::invokeMethod(handler->receiver, handler->name, Qt::DirectConnection, Q_ARG(QUrl, url));
            insideHandlerCall = false;
            return result; // ### support bool slot return type
        }
    }

    bool result;
    if (url.scheme() == QLatin1String("file"))
        result = openDocument(url);
    else
        result = launchWebBrowser(url);

    return result;
}

/*!
    This function provides a way to customize the behavior of QDesktopServices::openUrl(). If openUrl is called
    with a url that has the specified \a scheme then the given \a method on the \a receiver object is called instead
    of QDesktopServices launching an external application.

    The provided method must be marked as a slot and take a QUrl as single argument.

    This makes it easy for example to implement your own help system. Help could be provided in labels and text browsers
    using help://myapplication/mytopic URLs and by registering a handler it becomes possible to display the help text
    inside the application:
    \code
    class MyHelpHandler : public QObject
    {
        Q_OBJECT
    public:
        ...
    public slots:
        void showHelp(const QUrl &url);
    };

    QDesktopServices::registerUrlHandler("help", helpInstance, "showHelp");
    \endcode

    \sa openUrl()
*/
void QDesktopServices::setUrlHandler(const QString &scheme, QObject *receiver, const char *method)
{
    if (!receiver) {
        handlerRegistry()->handlers.remove(scheme);
        return;
    }
    if (receiver->thread() != QCoreApplication::instance()->thread()) {
        qWarning("QDesktopServices::setUrlHandler: handler for scheme '%s' does not live in the GUI thread, not registering.", qPrintable(scheme));
        return;
    }
    QOpenUrlHandlerRegistry *registry = handlerRegistry();
    QOpenUrlHandlerRegistry::Handler h;
    h.receiver = receiver;
    h.name = method;
    registry->handlers.insert(scheme, h);
    QObject::connect(receiver, SIGNAL(destroyed(QObject *)),
                     registry, SLOT(handlerDestroyed(QObject *)));
}

#include "qdesktopservices.moc"
