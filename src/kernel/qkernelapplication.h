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

#ifndef QT_H
#include <qobject.h>
#include <qstringlist.h>
#endif // QT_H

class QKernelApplicationPrivate;
class QTextCodec;
class QTranslator;
class QEventLoop;

class Q_KERNEL_EXPORT QKernelApplication : public QObject
{
    Q_OBJECT
    Q_DECL_PRIVATE(QKernelApplication);
public:
    QKernelApplication(int &argc, char **argv);
    QKernelApplication(QKernelApplicationPrivate *, QEventLoop *);
    ~QKernelApplication();

    static QKernelApplication *instance() { return self; }
    static QEventLoop *eventLoop();

    virtual int      exec();
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

#ifndef QT_NO_COMPONENT
    static void      setLibraryPaths( const QStringList & );
    static QStringList libraryPaths();
    static void      addLibraryPath( const QString & );
    static void      removeLibraryPath( const QString & );
#endif // QT_NO_COMPONENT

#ifndef QT_NO_TRANSLATION
# ifndef QT_NO_TEXTCODEC
    void	     setDefaultCodec( QTextCodec * );
    QTextCodec*	     defaultCodec() const;
# endif
    void	     installTranslator( QTranslator * );
    void	     removeTranslator( QTranslator * );
#endif
    enum Encoding { DefaultCodec, UnicodeUTF8 };
    QString	     translate( const char * context,
				const char * key,
				const char * comment = 0,
				Encoding encoding = DefaultCodec ) const;

    static void flush();

#if defined(QT_THREAD_SUPPORT) && !defined(QT_NO_COMPAT)
    void	     lock();
    void	     unlock(bool wakeUpGui = TRUE);
    bool	     locked();
    bool             tryLock();
#endif

public slots:
    void	     quit();

signals:
    void	     aboutToQuit();

protected:
    bool event(QEvent *);

private:
    void init();
    static bool      sendSpontaneousEvent( QObject *receiver, QEvent *event );
    static void      removePostedEvent( QEvent * );
    bool notify_helper( QObject *, QEvent * );

    static bool is_app_running;
    static bool is_app_closing;

    static QKernelApplication *self;

    friend class QEvent;
    friend class QEventLoop;
    friend class QGuiEventLoop;
    friend class QApplication;
    friend class QETWidget;
    friend class QAccelManager;
    friend class QWidget;
    friend class QWidgetPrivate;
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

#ifdef QT_NO_TRANSLATION
// Simple versions
inline QString QKernelApplication::translate( const char *, const char *sourceText,
					const char *, Encoding encoding ) const
{
#ifndef QT_NO_TEXTCODEC
    if ( encoding == UnicodeUTF8 )
	return QString::fromUtf8( sourceText );
    else
#endif
	return QString::fromLatin1( sourceText );
}
#endif

Q_KERNEL_EXPORT void qAddPostRoutine( QtCleanUpFunction );
Q_KERNEL_EXPORT void qRemovePostRoutine( QtCleanUpFunction );
Q_KERNEL_EXPORT const char *qAppName();		// get application name


#endif
