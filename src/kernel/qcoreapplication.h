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

#ifndef QCOREAPPLICATION_H
#define QCOREAPPLICATION_H

#ifndef QT_H
#include <qobject.h>
#include <qstringlist.h>
#endif // QT_H

class QCoreApplicationPrivate;
class QTextCodec;
class QTranslator;
class QEventLoop;

class Q_CORE_EXPORT QCoreApplication : public QObject
{
    Q_OBJECT
    Q_DECL_PRIVATE(QCoreApplication);
public:
    QCoreApplication(int &argc, char **argv);
    QCoreApplication(QCoreApplicationPrivate &, QEventLoop *);
    ~QCoreApplication();

    int		    argc()	const;
    char	  **argv()	const;

    static QCoreApplication *instance() { return self; }
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

#ifndef QT_NO_DIR
    QString   applicationDirPath();
    QString   applicationFilePath();
#endif

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

#if defined(QT_THREAD_SUPPORT) && defined(QT_COMPAT)
    QT_COMPAT void	     lock();
    QT_COMPAT void	     unlock(bool wakeUpGui = TRUE);
    QT_COMPAT bool	     locked();
    QT_COMPAT bool             tryLock();
#endif

#if defined(Q_WS_WIN)
    virtual bool     winEventFilter( MSG * );
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

    static QCoreApplication *self;

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

inline bool QCoreApplication::sendEvent( QObject *receiver, QEvent *event )
{  if ( event ) event->spont = FALSE; return self ? self->notify( receiver, event ) : FALSE; }

inline bool QCoreApplication::sendSpontaneousEvent( QObject *receiver, QEvent *event )
{ if ( event ) event->spont = TRUE; return self ? self->notify( receiver, event ) : FALSE; }

inline void QCoreApplication::sendPostedEvents() { sendPostedEvents( 0, 0 ); }

inline void QCoreApplication::processEvents() { processEvents( 3000 ); }

#ifdef QT_NO_TRANSLATION
// Simple versions
inline QString QCoreApplication::translate( const char *, const char *sourceText,
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

Q_CORE_EXPORT void qAddPostRoutine( QtCleanUpFunction );
Q_CORE_EXPORT void qRemovePostRoutine( QtCleanUpFunction );
Q_CORE_EXPORT const char *qAppName();		// get application name


#endif
