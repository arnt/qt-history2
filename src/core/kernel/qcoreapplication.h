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

#ifndef QCOREAPPLICATION_H
#define QCOREAPPLICATION_H

#include <qobject.h>
#include <qcoreevent.h>
#include <qeventloop.h>

#ifdef QT_INCLUDE_COMPAT
#include <qstringlist.h>
#endif

#ifdef Q_WS_WIN32
# include <qt_windows.h>
#endif

class QCoreApplicationPrivate;
class QTextCodec;
class QTranslator;
class QPostEventList;
class QStringList;

class Q_CORE_EXPORT QCoreApplication : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString applicationName READ applicationName WRITE setApplicationName)
    Q_PROPERTY(QString organizationDomain READ organizationDomain WRITE setOrganizationDomain)

    Q_DECLARE_PRIVATE(QCoreApplication)
public:
    QCoreApplication(int &argc, char **argv);
    QCoreApplication(QCoreApplicationPrivate &p);
    ~QCoreApplication();

    int argc() const;
    char **argv() const;

    static void setOrganizationDomain(const QString &organization);
    static QString organizationDomain();
    static void setApplicationName(const QString &application);
    static QString applicationName();

    static QCoreApplication *instance() { return self; }

    static int exec();
    static void processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    static void processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime);
    static void exit(int retcode=0);

    static bool sendEvent(QObject *receiver, QEvent *event);
    static void postEvent(QObject *receiver, QEvent *event);
    static void sendPostedEvents(QObject *receiver, int event_type);
    static void sendPostedEvents();

    static void removePostedEvents(QObject *receiver);

    virtual bool notify(QObject *, QEvent *);

    static bool startingUp();
    static bool closingDown();

#ifndef QT_NO_DIR
    QString   applicationDirPath();
    QString   applicationFilePath();
#endif

#ifndef QT_NO_COMPONENT
    static void setLibraryPaths(const QStringList &);
    static QStringList libraryPaths();
    static void addLibraryPath(const QString &);
    static void removeLibraryPath(const QString &);
#endif // QT_NO_COMPONENT

#ifndef QT_NO_TRANSLATION
# ifndef QT_NO_TEXTCODEC
    void setDefaultCodec(QTextCodec *);
    QTextCodec *defaultCodec() const;
# endif
    void installTranslator(QTranslator *);
    void removeTranslator(QTranslator *);
#endif
    enum Encoding { DefaultCodec, UnicodeUTF8 };
    static QString translate(const char * context,
                             const char * key,
                             const char * comment = 0,
                             Encoding encoding = DefaultCodec);

    static void flush();

#if defined(QT_COMPAT)
    inline QT_COMPAT void lock() {}
    inline QT_COMPAT void unlock(bool = true) {}
    inline QT_COMPAT bool locked() { return false; }
    inline QT_COMPAT bool tryLock() { return false; }

    static inline QT_COMPAT void processOneEvent()
    { processEvents(QEventLoop::WaitForMoreEvents); }
    static QT_COMPAT bool hasPendingEvents();
    static QT_COMPAT int enter_loop();
    static QT_COMPAT void exit_loop();
    static QT_COMPAT int loopLevel();
#endif

#if defined(Q_WS_WIN)
    virtual bool winEventFilter(MSG *message, long *result);
#endif

#ifdef Q_OS_UNIX
    static void watchUnixSignal(int signal, bool watch);
#endif

    typedef bool (*EventFilter)(void *message, long *result);
    EventFilter setEventFilter(EventFilter filter);
    bool filterEvent(void *message, long *result);

public slots:
    void quit();

signals:
    void aboutToQuit();
    void unixSignal(int);

protected:
    bool event(QEvent *);

    virtual bool compressEvent(QEvent *, QObject *receiver, QPostEventList *);

private:
    void init();
    static bool sendSpontaneousEvent(QObject *receiver, QEvent *event);
    static void removePostedEvent(QEvent *);
    bool notify_helper(QObject *, QEvent *);

    static bool is_app_running;
    static bool is_app_closing;

    static QCoreApplication *self;

    friend class QEvent;
    friend class QEventDispatcherUNIXPrivate;
    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QETWidget;
    friend class Q3AccelManager;
    friend class QShortcutMap;
    friend class QWidget;
    friend class QWidgetPrivate;
#if defined(Q_WS_WIN) || defined (Q_WS_MAC) || defined (Q_WS_QWS)
    friend bool qt_sendSpontaneousEvent(QObject*, QEvent*);
#endif
};

inline bool QCoreApplication::sendEvent(QObject *receiver, QEvent *event)
{  if (event) event->spont = false; return self ? self->notify(receiver, event) : false; }

inline bool QCoreApplication::sendSpontaneousEvent(QObject *receiver, QEvent *event)
{ if (event) event->spont = true; return self ? self->notify(receiver, event) : false; }

inline void QCoreApplication::sendPostedEvents() { sendPostedEvents(0, 0); }

#ifdef QT_NO_TRANSLATION
// Simple versions
inline QString QCoreApplication::translate(const char *, const char *sourceText,
                                           const char *, Encoding encoding)
{
#ifndef QT_NO_TEXTCODEC
    if (encoding == UnicodeUTF8)
        return QString::fromUtf8(sourceText);
#else
    Q_UNUSED(encoding)
#endif
    return QString::fromLatin1(sourceText);
}
#endif

typedef void (*QtCleanUpFunction)();

Q_CORE_EXPORT void qAddPostRoutine(QtCleanUpFunction);
Q_CORE_EXPORT void qRemovePostRoutine(QtCleanUpFunction);
Q_CORE_EXPORT const char *qAppName();                // get application name

#if defined(Q_WS_WIN) && !defined(QT_NO_DEBUG)
Q_CORE_EXPORT QString decodeMSG(const MSG &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const MSG &);
#endif

#endif
