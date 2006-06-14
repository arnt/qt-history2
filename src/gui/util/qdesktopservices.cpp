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
#include <qurl.h>
#include <qmutex.h>

class QOpenUrlHandlerRegistry : public QObject
{
    Q_OBJECT
public:
    inline QOpenUrlHandlerRegistry() : mutex(QMutex::Recursive) {}

    QMutex mutex;

    struct Handler
    {
        QObject *receiver;
        QByteArray name;
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
    \brief The QDesktopServices namespace provides methods for accessing common desktop services.
    \since 4.2
    \ingroup desktop

    Many desktop environments provide services that can be used by applications to
    perform common tasks, such as opening a web page, in a way that is both consistent
    and takes into account the user's application preferences.

    This namespace contains functions that provide simple interfaces to these services
    that indicate whether they succeeded or failed.

    The launchWebBrowser() function is used to open arbitrary URLs in the user's
    web browser, and is typically most useful when you need to display the contents
    of external documents.

    The openDocument() function is used to execute files located at arbitrary URLs.
    The user's desktop settings control whether certain executable file types are
    opened for browsing, or if they are executed instead. Some desktop environments
    are configured to prevent users from executing files obtained from non-local URLs,
    or to ask the user's permission before doing so.

    \sa QSystemTrayIcon, QProcess
*/

/*!
    Opens the given \a url in the appropriate web browser for the user's desktop
    environment, and returns true if successful; otherwise returns false.

    If the \a url is a reference to a local file (i.e. the url scheme is "file") then
    it will be opened with a suitable application instead of a web browser.

    If a \c mailto URL is specified, the user's e-mail client will be used to open a
    composer window containing the options specified in the URL, similar to the way
    \c mailto links are handled by a web browser.

    For example, the following URL contains a recipient (\c{user@foo.com}), a
    subject (\c{Test}), and a message body (\c{Just a test}):

    \code
    "mailto:user@foo.com?subject=Test&body=Just a test"
    \endcode

    \bold{Note:} Although many e-mail clients can send attachments and are
    unicode-aware, the user may have configured their client without these features.

    \sa setUrlHandler()

*/
bool QDesktopServices::openUrl(const QUrl &url)
{
    QOpenUrlHandlerRegistry *registry = handlerRegistry();
    QMutexLocker locker(&registry->mutex);
    static bool insideOpenUrlHandler = false;

    if (!insideOpenUrlHandler) {
        QOpenUrlHandlerRegistry::HandlerHash::ConstIterator handler = registry->handlers.find(url.scheme());
        if (handler != registry->handlers.constEnd()) {
            insideOpenUrlHandler = true;
            bool result = QMetaObject::invokeMethod(handler->receiver, handler->name, Qt::DirectConnection, Q_ARG(QUrl, url));
            insideOpenUrlHandler = false;
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

    If inside the handler you decide that you can't open the requested url you can just call QDesktopServices::openUrl
    and it will try to open the url using the operating system.

    Note that the handler will always be called from within the same thread that calls QDesktopServices::openUrl.

    \sa openUrl()
*/
void QDesktopServices::setUrlHandler(const QString &scheme, QObject *receiver, const char *method)
{
    QOpenUrlHandlerRegistry *registry = handlerRegistry();
    QMutexLocker locker(&registry->mutex);
    if (!receiver) {
        registry->handlers.remove(scheme);
        return;
    }
    QOpenUrlHandlerRegistry::Handler h;
    h.receiver = receiver;
    h.name = method;
    registry->handlers.insert(scheme, h);
    QObject::connect(receiver, SIGNAL(destroyed(QObject *)),
                     registry, SLOT(handlerDestroyed(QObject *)));
}

#include "qdesktopservices.moc"
