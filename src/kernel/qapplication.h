/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.h#138 $
**
** Definition of QApplication class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#ifndef QT_H
#include "qdesktopwidget.h"
#include "qasciidict.h"
#include "qpalette.h"
#include "qtranslator.h"
#include "qstrlist.h"
#include "qstringlist.h"
#endif // QT_H

class QSessionManager;
class QStyle;
class QTranslator;
class QSettings;
#if defined(Q_WS_QWS)
class QWSDecoration;
#endif
template <class type> class QList;

class QApplication;
extern Q_EXPORT QApplication *qApp;		// global application object

#if defined(QT_THREAD_SUPPORT)
class QMutex;
#endif

// REMOVE IN 3.0 (just here for moc source compatibility)
#define QNonBaseApplication QApplication

class Q_EXPORT QApplication : public QObject
{
    Q_OBJECT
public:
    QApplication( int &argc, char **argv);
    QApplication( int &argc, char **argv, bool GUIenabled );
    enum Type { Tty, GuiClient, GuiServer };
    QApplication( int &argc, char **argv, Type );
#if defined(Q_WS_X11)
    QApplication( Display* dpy );
    QApplication( Display *dpy, int argc, char **argv);
#endif
    virtual ~QApplication();

    int		    argc()	const;
    char	  **argv()	const;

#ifndef QT_NO_STYLE
    static QStyle  &style();
    static void	    setStyle( QStyle* );
    static QStyle*  setStyle( const QString& );
#endif
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

    static QDesktopWidget   *desktop();

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

    static void	     flushX();
    static void	     syncX();

    static void	     beep();

#ifndef QT_NO_TRANSLATION
    void	     setDefaultCodec( QTextCodec * );
    QTextCodec*	     defaultCodec() const;
    void	     installTranslator( QTranslator * );
    void	     removeTranslator( QTranslator * );
#endif
    QString	     translate( const char * context,
				const char * key,
				const char * comment = 0 ) const;
#ifndef QT_NO_PALETTE
    // obsolete functions
    static void      setWinStyleHighlightColor( const QColor &c ) {
	QPalette p( palette() );
	p.setColor( QColorGroup::Highlight, c );
	setPalette( p, TRUE);
    }
    static const QColor &winStyleHighlightColor() {
	return palette().active().highlight();
    }
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

    static void      setLibraryPaths(const QStringList &);
    static QStringList libraryPaths();
    static void      addLibraryPath(const QString &);
    static void      removeLibraryPath(const QString &);

    static QSettings *settings();

    static void setStartDragTime( int ms );
    static int startDragTime();
    static void setStartDragDistance( int l );
    static int startDragDistance();

    static void setReverseLayout( bool b );
    static bool reverseLayout();

    static int horizontalAlignment( int align );

    static bool	    isEffectEnabled( Qt::UIEffect );
    static void	    setEffectEnabled( Qt::UIEffect, bool enable = TRUE );

#if defined(Q_WS_MAC)
    virtual bool     macEventFilter( EventRef );
#elif defined(Q_WS_WIN)
    virtual bool     winEventFilter( MSG * );
#elif defined(Q_WS_X11)
    virtual bool     x11EventFilter( XEvent * );
    virtual int	     x11ClientMessage( QWidget*, XEvent*, bool passive_only);
    int              x11ProcessEvent( XEvent* );
#elif defined(Q_WS_QWS)
    virtual bool     qwsEventFilter( QWSEvent * );
    int              qwsProcessEvent( QWSEvent* );
    void             qwsSetCustomColors( QRgb *colortable, int start, int numColors );
#ifndef QT_NO_QWS_MANAGER
    static QWSDecoration &qwsDecoration();
    static void      qwsSetDecoration( QWSDecoration *);
#endif
#endif

#if defined(Q_WS_WIN)
    static WindowsVersion winVersion();
    void	     winFocus( QWidget *, bool );
    static void	     winMouseButtonUp();
#endif

#ifndef QT_NO_SESSIONMANAGER
    // session management
    bool	     isSessionRestored() const;
    QString	     sessionId() const;
    virtual void     commitData( QSessionManager& sm );
    virtual void     saveState( QSessionManager& sm );
#endif
#if defined(Q_WS_X11)
    static void create_xim();
    static void close_xim();
#endif
    void	     wakeUpGuiThread();
#if defined(QT_THREAD_SUPPORT)
    void	     lock();
    void	     unlock(bool wakeUpGui = TRUE);
    bool 	     locked();
    bool             tryLock();
#endif

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
    bool	     internalNotify( QObject *, QEvent * );
#if defined(Q_WS_QWS)
    static QWidget *findChildWidget( const QWidget *p, const QPoint &pos );
    static QWidget *findWidget( const QObjectList&, const QPoint &, bool rec );
#endif

#if defined(Q_WS_MAC)
    bool	     do_mouse_down(Point *);
    static QMAC_PASCAL OSStatus globalEventProcessor(EventHandlerCallRef,  EventRef, void *);
    static QMAC_PASCAL void qt_trap_context_mouse(EventLoopTimerRef, void *);
    static QMAC_PASCAL void qt_idle_timer_callbk(EventLoopTimerRef, void *);
    friend void qt_mac_destroy_widget(QWidget *);
    friend void qt_init(int *, char **, QApplication::Type);
#endif

#if defined(QT_THREAD_SUPPORT)
    static QMutex * qt_mutex;
#endif

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

    static bool	     animate_ui;
    static bool	     animate_menu;
    static bool	     animate_tooltip;
    static bool	     animate_combo;
    static bool	     fade_menu;
    static bool	     fade_tooltip;

    QList<QTranslator> *translators;
#ifndef QT_NO_SESSIONMANAGER
    QSessionManager *session_manager;
    QString	     session_id;
    bool	     is_session_restored;
#endif
#if defined(Q_WS_X11) && !defined (QT_NO_STYLE )
    static void x11_initialize_style();
#endif

    static QSize     app_strut;

    static QStringList *app_libpaths;

    static QSettings *app_settings;

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
#ifndef QT_NO_STYLE
inline QStyle& QApplication::style()
{
    return *app_style;
}
#endif
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

inline bool QApplication::sendEvent( QObject *receiver, QEvent *event )
{ return qApp ? qApp->notify( receiver, event ) : FALSE; }

#ifdef QT_NO_TRANSLATION
// Simple versions
inline QString QApplication::translate( const char *, const char *key ) const
{
    return key;
}

inline QString	QApplication::translate( const char *, const char *key,
			    const char * ) const
{
    return key;
}
#endif

inline int QApplication::horizontalAlignment( int align )
{
    align &= AlignHorizontal_Mask;
    if ( align == AlignAuto ) {
	if ( reverseLayout() )
	    align = AlignRight;
	else
	    align = AlignLeft;
    }
    return align;
}

#endif // QAPPLICATION_H
