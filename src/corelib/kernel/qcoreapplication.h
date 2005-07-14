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

#include <QtCore/qobject.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qeventloop.h>

#ifdef QT_INCLUDE_COMPAT
#include <QtCore/qstringlist.h>
#endif

QT_MODULE(Core)

#if defined(Q_WS_WIN) && !defined(tagMSG)
typedef struct tagMSG MSG;
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
    Q_PROPERTY(QString organizationName READ organizationName WRITE setOrganizationName)
    Q_PROPERTY(QString organizationDomain READ organizationDomain WRITE setOrganizationDomain)

    Q_DECLARE_PRIVATE(QCoreApplication)
public:
    QCoreApplication(int &argc, char **argv);
    ~QCoreApplication();

    static int argc();
    static char **argv();

    static void setOrganizationDomain(const QString &orgDomain);
    static QString organizationDomain();
    static void setOrganizationName(const QString &orgName);
    static QString organizationName();
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
    static bool hasPendingEvents();

    virtual bool notify(QObject *, QEvent *);

    static bool startingUp();
    static bool closingDown();

    static QString applicationDirPath();
    static QString applicationFilePath();

#ifndef QT_NO_LIBRARY
    static void setLibraryPaths(const QStringList &);
    static QStringList libraryPaths();
    static void addLibraryPath(const QString &);
    static void removeLibraryPath(const QString &);
#endif // QT_NO_LIBRARY

#ifndef QT_NO_TRANSLATION
    static void installTranslator(QTranslator *);
    static void removeTranslator(QTranslator *);
#endif
    enum Encoding { DefaultCodec, UnicodeUTF8 };
    static QString translate(const char * context,
                             const char * key,
                             const char * comment = 0,
                             Encoding encoding = DefaultCodec);

    static void flush();

#if defined(QT3_SUPPORT)
    inline QT3_SUPPORT void lock() {}
    inline QT3_SUPPORT void unlock(bool = true) {}
    inline QT3_SUPPORT bool locked() { return false; }
    inline QT3_SUPPORT bool tryLock() { return false; }

    static inline QT3_SUPPORT void processOneEvent()
    { processEvents(QEventLoop::WaitForMoreEvents); }
    static QT3_SUPPORT int enter_loop();
    static QT3_SUPPORT void exit_loop();
    static QT3_SUPPORT int loopLevel();
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
    static void quit();

signals:
    void aboutToQuit();
    void unixSignal(int);

protected:
    bool event(QEvent *);

    virtual bool compressEvent(QEvent *, QObject *receiver, QPostEventList *);

protected:
    QCoreApplication(QCoreApplicationPrivate &p);

private:
    static bool sendSpontaneousEvent(QObject *receiver, QEvent *event);

    void init();

    static QCoreApplication *self;

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
Q_CORE_EXPORT QString qAppName();                // get application name

#if defined(Q_WS_WIN) && !defined(QT_NO_DEBUG_STREAM)
Q_CORE_EXPORT QString decodeMSG(const MSG &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const MSG &);
#endif

#endif // QCOREAPPLICATION_H
