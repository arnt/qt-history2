/****************************************************************************
**
** Definition of QApplication class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QKERNELAPPLICATION_H
#define QKERNELAPPLICATION_H

#include <qobject.h>


class QKernelApplicationPrivate;
class QEventLoop;
class QMutex;

class Q_EXPORT QKernelApplication : public QObject
{
    Q_OBJECT
    Q_DECL_PRIVATE(QKernelApplication);
public:
    QKernelApplication(int &argc, char **argv);
    QKernelApplication(QKernelApplicationPrivate *, QEventLoop *);
    ~QKernelApplication();

    static QKernelApplication *instance() { return self; }
    static QEventLoop *eventLoop();

    int		     exec();
    void	     processEvents();
    void	     processEvents( int maxtime );
    void	     processOneEvent();
    bool	     hasPendingEvents();
    int		     enter_loop();
    void	     exit_loop();
    int		     loopLevel() const;
    static void	     exit( int retcode=0 );

    static bool	     sendEvent( QObject *receiver, QEvent *event );
    static void	     postEvent( QObject *receiver, QEvent *event );
    static void	     sendPostedEvents( QObject *receiver, int event_type );
    static void	     sendPostedEvents();

    static void      removePostedEvents( QObject *receiver );

    virtual bool     notify( QObject *, QEvent * );

    static bool	     startingUp();
    static bool	     closingDown();


#if defined(QT_THREAD_SUPPORT)
    void	     lock();
    void	     unlock(bool wakeUpGui = TRUE);
    bool	     locked();
    bool             tryLock();
#endif

public slots:
    void	     quit();

protected:
    bool event(QEvent *);

private:
    void init();
    static bool      sendSpontaneousEvent( QObject *receiver, QEvent *event );
    static void      removePostedEvent( QEvent * );
    bool notify_helper( QObject *, QEvent * );

    static QEventLoop* eventloop;
    static bool is_app_running;
    static bool is_app_closing;
#ifdef QT_THREAD_SUPPORT
    static QMutex   *qt_mutex;
#endif // QT_THREAD_SUPPORT


    static QKernelApplication *self;

    friend class QEvent;
    friend class QEventLoop;
    friend class QGuiEventLoop;
    friend class QApplication;
    friend class QETWidget;
    friend class QAccelManager;
    friend class QWidget;
#if defined(Q_WS_WIN) || defined (Q_WS_MAC)
    friend bool qt_sendSpontaneousEvent( QObject*, QEvent* );
#endif
};

inline bool QKernelApplication::sendEvent( QObject *receiver, QEvent *event )
{  if ( event ) event->spont = FALSE; return self ? self->notify( receiver, event ) : FALSE; }

inline bool QKernelApplication::sendSpontaneousEvent( QObject *receiver, QEvent *event )
{ if ( event ) event->spont = TRUE; return self ? self->notify( receiver, event ) : FALSE; }

inline void QKernelApplication::sendPostedEvents() { sendPostedEvents( 0, 0 ); }

inline void QKernelApplication::processEvents() { processEvents( 3000 ); }


#endif
