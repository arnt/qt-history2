/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.h#138 $
**
** Definition of QApplication class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#ifndef QT_H
#include "qwidget.h"
#include "qasciidict.h"
#include "qpalette.h"
#include "qtranslator.h"
#endif // QT_H

class QSessionManager;
class QStyle;
class QTranslator;
#if defined(_WS_QWS_)
class QWSDecorator;
#endif
template <class type> class QList;

class QApplication;
extern Q_EXPORT QApplication *qApp;		// global application object

#if defined(QT_DLL) || defined(QT_MAKEDLL)
#define QT_BASEAPP
typedef QApplication QNonBaseApplication;
#define QApplication QBaseApplication
#else
#define QNonBaseApplication QApplication
#endif


class Q_EXPORT QApplication : public QObject
{
    Q_OBJECT
public:
    QApplication( int &argc, char **argv);
    QApplication( int &argc, char **argv, bool GUIenabled );
    enum Type { Tty, GuiClient, GuiServer };
    QApplication( int &argc, char **argv, Type );
#if defined(_WS_X11_)
    QApplication( Display* dpy );
#endif
    virtual ~QApplication();

    int		    argc()	const;
    char	  **argv()	const;

    static QStyle  &style();
    static void	    setStyle( QStyle* );

#if 1	/* OBSOLETE */
    enum ColorMode { NormalColors, CustomColors };
    static ColorMode colorMode();
    static void      setColorMode( QApplication::ColorMode );
#endif

    enum ColorSpec { NormalColor=0, CustomColor=1, ManyColor=2 };
    static int	     colorSpec();
    static void      setColorSpec( int );
#ifndef QT_NO_CURSOR
    static QCursor  *overrideCursor();
    static void	     setOverrideCursor( const QCursor &, bool replace=FALSE );
    static void	     restoreOverrideCursor();
#endif
    static bool	     hasGlobalMouseTracking();
    static void	     setGlobalMouseTracking( bool enable );
#ifndef QT_NO_PALETTE
    static QPalette  palette( const QWidget* = 0 );
    static void	     setPalette( const QPalette &, bool informWidgets=FALSE,
				 const char* className = 0 );
#endif
    static QFont     font( const QWidget* = 0 );
    static void	     setFont( const QFont &, bool informWidgets=FALSE,
			      const char* className = 0 );
    static QFontMetrics fontMetrics();

    QWidget	    *mainWidget()  const;
    virtual void     setMainWidget( QWidget * );
    virtual void     polish( QWidget * );

    static QWidgetList *allWidgets();
    static QWidgetList *topLevelWidgets();
    static QWidget     *desktop();
    static QWidget     *activePopupWidget();
    static QWidget     *activeModalWidget();
    static QClipboard  *clipboard();
    QWidget	       *focusWidget() const;
    QWidget	       *activeWindow() const;

    static QWidget  *widgetAt( int x, int y, bool child=FALSE );
    static QWidget  *widgetAt( const QPoint &, bool child=FALSE );

    int		     exec();
    void	     processEvents();
    void	     processEvents( int maxtime );
    void	     processOneEvent();
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

    static void	     flushX();
    static void	     syncX();

    static void	     beep();

#ifndef QT_NO_TRANSLATION
    void	     setDefaultCodec( QTextCodec* );
    QTextCodec*	     defaultCodec() const;
    void	     installTranslator( QTranslator * );
    void	     removeTranslator( QTranslator * );
#endif
    QString	     translate( const char *, const char * ) const;
    QString	     translate( const char *, const char *,
				const char * ) const;
#ifndef QT_NO_PALETTE
    static void      setWinStyleHighlightColor( const QColor & );
    static const QColor &winStyleHighlightColor();
#endif
    static void      setDesktopSettingsAware( bool );
    static bool      desktopSettingsAware();

    static void      setCursorFlashTime( int );
    static int       cursorFlashTime();

    static void      setDoubleClickInterval( int );
    static int       doubleClickInterval();

    static void      setWheelScrollLines( int );
    static int       wheelScrollLines();

    static void	     setGlobalStrut( const QSize & );
    static QSize     globalStrut();

    static void setStartDragTime( int ms );
    static int startDragTime();
    static void setStartDragDistance( int l );
    static int startDragDistance();

#if defined(_WS_MAC_)
    void	     do_mouse_down(void *);
    virtual bool     macEventFilter( MSG * );
    int              macProcessEvent( MSG * );
#elif defined(_WS_WIN_)
    virtual bool     winEventFilter( MSG * );
#elif defined(_WS_X11_)
    virtual bool     x11EventFilter( XEvent * );
    virtual int	     x11ClientMessage( QWidget*, XEvent*, bool passive_only);
    int              x11ProcessEvent( XEvent* );
#elif defined(_WS_QWS_)
    virtual bool     qwsEventFilter( QWSEvent * );
    int              qwsProcessEvent( QWSEvent* );
#ifndef QT_NO_QWS_MANAGER
    static QWSDecorator &qwsDecorator();
    static void      qwsSetDecorator( QWSDecorator *);
#endif
#endif

#if defined(_WS_WIN_)
    static WindowsVersion winVersion();
    void	     winFocus( QWidget *, bool );
    static void	     winMouseButtonUp();
    static bool winEffectSupport( Qt::WindowsEffect );
#endif

#ifndef QT_NO_SESSIONMANAGER
    // session management
    bool	     isSessionRestored() const;
    QString	     sessionId() const;
    virtual void     commitData( QSessionManager& sm );
    virtual void     saveState( QSessionManager& sm );
#endif
#if defined(_WS_X11_)
    static void create_xim();
    static void close_xim();
#endif

    void	     wakeUpGuiThread();

signals:
    void	     lastWindowClosed();
    void	     aboutToQuit();
    void	     guiThreadAwake();

public slots:
    void	     quit();
    void	     closeAllWindows();

private:
    void	     construct( int &argc, char **argv, Type );
    bool	     processNextEvent( bool );
    void	     initialize( int, char ** );
    void	     init_precmdline();
    void	     process_cmdline( int* argcptr, char ** argv );

    int		     app_argc;
    char	   **app_argv;
    bool	     quit_now;
    int		     quit_code;
    static QStyle   *app_style;
    static int	     app_cspec;
#ifndef QT_NO_PALETTE
    static QPalette *app_pal;
#endif
    static QFont    *app_font;
#ifndef QT_NO_CURSOR
    static QCursor  *app_cursor;
#endif
    static int	     app_tracking;
    static bool	     is_app_running;
    static bool	     is_app_closing;
    static bool	     app_exit_loop;
    static int	     loop_level;
    static QWidget  *main_widget;
    static QWidget  *focus_widget;
    static QWidget  *active_window;
    static bool	     obey_desktop_settings;
    static int	     cursor_flash_time;
    static int	     mouse_double_click_time;
    static int	     wheel_scroll_lines;
    QList<QTranslator> *translators;
#ifndef QT_NO_SESSIONMANAGER
    QSessionManager *session_manager;
    QString	     session_id;
    bool	     is_session_restored;
#endif
    static QSize     app_strut;

    static QAsciiDict<QPalette> *app_palettes;
    static QAsciiDict<QFont>    *app_fonts;

    static QWidgetList *popupWidgets;
    bool	     inPopupMode() const;
    void	     closePopup( QWidget *popup );
    void	     openPopup( QWidget *popup );
    void 	     setActiveWindow( QWidget* act );

    static void      removePostedEvent( QEvent * );

    friend class QWidget;
    friend class QETWidget;
    friend class QEvent;
#if defined QT_BASEAPP
    friend QNonBaseApplication;
#endif

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QApplication( const QApplication & );
    QApplication &operator=( const QApplication & );
#endif
};


inline int QApplication::argc() const
{
    return app_argc;
}

inline char **QApplication::argv() const
{
    return app_argv;
}

inline QStyle& QApplication::style()
{
    return *app_style;
}

#ifndef QT_NO_CURSOR
inline QCursor *QApplication::overrideCursor()
{
    return app_cursor;
}
#endif
inline bool QApplication::hasGlobalMouseTracking()
{
    return app_tracking > 0;
}

inline QWidget *QApplication::mainWidget() const
{
    return main_widget;
}

inline QWidget *QApplication::focusWidget() const
{
    return focus_widget;
}

inline QWidget *QApplication::activeWindow() const
{
    return active_window;
}

inline QWidget *QApplication::widgetAt( const QPoint &p, bool child )
{
    return widgetAt( p.x(), p.y(), child );
}

inline bool QApplication::inPopupMode() const
{
    return popupWidgets != 0;
}
#ifndef QT_NO_SESSIONMANAGER
inline bool QApplication::isSessionRestored() const
{
    return is_session_restored;
}

inline QString QApplication::sessionId() const
{
    return session_id;
}
#endif
inline QSize QApplication::globalStrut()
{
    return app_strut;
}

#if defined(QT_BASEAPP)


#undef QApplication

class QApplication : public QBaseApplication
{
public:
    QApplication( int &, char ** );
    QApplication( int &, char **, bool );
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QApplication( const QApplication & );
    QApplication &operator=( const QApplication & );
#endif
    friend QBaseApplication;
};

inline bool QBaseApplication::sendEvent( QObject *receiver, QEvent *event )
{ return qApp->notify( receiver, event ); }

#if defined(Q_MOC_OUTPUT_REVISION) && defined(Q_MOC_QApplication)
#if defined(QT_MAKEDLL)
#define QApplication QBaseApplication
#endif
#endif

#else
inline bool QApplication::sendEvent( QObject *receiver, QEvent *event )
{ return qApp->notify( receiver, event ); }
#endif

#ifdef QT_NO_TRANSLATION
// Simple versions
inline QString QApplication::translate( const char *, const char *key ) const
{ return key; }
inline QString	QApplication::translate( const char *, const char *key,
			    const char * ) const
{ return key; }
#endif

#endif // QAPPLICATION_H
