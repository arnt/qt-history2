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

#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#ifndef QT_H
#include "qasciidict.h"
#include "qpalette.h"
#include "qstrlist.h"
#include "qstringlist.h"
#include "qkernelapplication.h"
#include "qpoint.h"
#include "qsize.h"
#endif // QT_H

#ifdef QT_INCLUDE_COMPAT
#include "qdesktopwidget.h"
#endif

class QSessionManager;
class QDesktopWidget;
class QStyle;
class QEventLoop;
template <typename T> class QList;
#if defined(Q_WS_QWS)
class QWSDecoration;
#endif


class QApplication;
class QApplicationPrivate;
#define qApp (static_cast<QApplication *>(QKernelApplication::instance()))

class Q_EXPORT QApplication : public QKernelApplication
{
    Q_OBJECT
public:
    QApplication( int &argc, char **argv );
    QApplication( int &argc, char **argv, bool GUIenabled );
    enum Type { Tty, GuiClient, GuiServer };
    QApplication( int &argc, char **argv, Type );
#if defined(Q_WS_X11)
    QApplication( Display* dpy, HANDLE visual = 0, HANDLE cmap = 0 );
    QApplication( Display *dpy, int argc, char **argv,
		  HANDLE visual = 0, HANDLE cmap= 0 );
#endif
    virtual ~QApplication();

    int		    argc()	const;
    char	  **argv()	const;

    Type type() const;

#ifndef QT_NO_STYLE
    static QStyle  &style();
    static void	    setStyle( QStyle* );
    static QStyle*  setStyle( const QString& );
#endif
#ifndef Q_QDOC
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

    static QWidgetList allWidgets();
    static QWidgetList topLevelWidgets();

    static QDesktopWidget   *desktop();

    static QWidget     *activePopupWidget();
    static QWidget     *activeModalWidget();
#ifndef QT_NO_CLIPBOARD
    static QClipboard  *clipboard();
#endif
    QWidget	       *focusWidget() const;
    QWidget	       *activeWindow() const;

    static QWidget  *widgetAt( int x, int y, bool child=FALSE );
    static QWidget  *widgetAt( const QPoint &, bool child=FALSE );

#ifndef QT_NO_COMPAT
    inline static void	     flushX() { flush(); }
#endif
    static void	     syncX();

    static void	     beep();

#ifndef QT_NO_DIR
    QString   applicationDirPath();
    QString   applicationFilePath();
#endif
#ifndef QT_NO_COMPAT
#ifndef QT_NO_PALETTE
    // obsolete functions
    static inline void setWinStyleHighlightColor( const QColor &c ) {
	QPalette p( palette() );
	p.setColor( QPalette::Highlight, c );
	setPalette( p, TRUE);
    }
    static inline const QColor &winStyleHighlightColor() {
	return palette().color(QPalette::Active, QPalette::Highlight);
    }
#endif
#endif
    static void      setDesktopSettingsAware( bool );
    static bool      desktopSettingsAware();

    static void      setCursorFlashTime( int );
    static int       cursorFlashTime();

    static void      setDoubleClickInterval( int );
    static int       doubleClickInterval();
#ifndef QT_NO_WHEELEVENT
    static void      setWheelScrollLines( int );
    static int       wheelScrollLines();
#endif
    static void	     setGlobalStrut( const QSize & );
    static QSize     globalStrut();

    static void setStartDragTime( int ms );
    static int startDragTime();
    static void setStartDragDistance( int l );
    static int startDragDistance();

    static void setReverseLayout( bool b );
    static bool reverseLayout();

    static Alignment horizontalAlignment( Alignment align );

    static bool	    isEffectEnabled( Qt::UIEffect );
    static void	    setEffectEnabled( Qt::UIEffect, bool enable = TRUE );

#if defined(Q_WS_MAC)
    virtual bool     macEventFilter( EventHandlerCallRef, EventRef );
#endif
#if defined(Q_WS_WIN)
    virtual bool     winEventFilter( MSG * );
#endif
#if defined(Q_WS_X11)
    virtual bool     x11EventFilter( XEvent * );
    virtual int	     x11ClientMessage( QWidget*, XEvent*, bool passive_only);
    int              x11ProcessEvent( XEvent* );
#endif
#if defined(Q_WS_QWS)
    virtual bool     qwsEventFilter( QWSEvent * );
    int              qwsProcessEvent( QWSEvent* );
    void             qwsSetCustomColors( QRgb *colortable, int start, int numColors );
#ifndef QT_NO_QWS_MANAGER
    static QWSDecoration &qwsDecoration();
    static void      qwsSetDecoration( QWSDecoration *);
#endif
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN)
    static WindowsVersion winVersion();
#elif defined(Q_OS_MAC)
    static MacintoshVersion macVersion();
#endif
#if defined(Q_WS_WIN)
    void	     winFocus( QWidget *, bool );
    static void	     winMouseButtonUp();
#endif

#ifndef QT_NO_SESSIONMANAGER
    // session management
    bool	     isSessionRestored() const;
    QString 	sessionId() const;
    QString 	sessionKey() const;
    virtual void     commitData( QSessionManager& sm );
    virtual void     saveState( QSessionManager& sm );
#endif
#if defined(Q_WS_X11)
    static void create_xim();
    static void close_xim();
    static bool x11_apply_settings();
#endif

    int exec();
    bool notify(QObject *, QEvent *);


signals:
    void	     lastWindowClosed();

public slots:
    void	     closeAllWindows();
    void	     aboutQt();

#if defined(Q_WS_QWS)
protected:
    void setArgs(int, char **);
#endif

protected:
    bool event(QEvent *);

#ifndef QT_NO_COMPAT
public:
    inline static bool hasGlobalMouseTracking() {return true;}
    inline static void setGlobalMouseTracking(bool) {};

#endif // QT_NO_COMPAT
private:
    bool notify_helper( QObject *receiver, QEvent * e);

    void construct(Type);
    void initialize();
    void process_cmdline();
#if defined(Q_WS_QWS)
    static QWidget *findChildWidget( const QWidget *p, const QPoint &pos );
    static QWidget *findWidget( const QObjectList&, const QPoint &, bool rec );
#endif

#if defined(Q_WS_MAC)
    bool do_mouse_down(Point *, bool *);
    static QMAC_PASCAL OSStatus globalEventProcessor(EventHandlerCallRef,  EventRef, void *);
    static QMAC_PASCAL void qt_context_timer_callbk(EventLoopTimerRef, void *);
    static QMAC_PASCAL void qt_select_timer_callbk(EventLoopTimerRef, void *);
    static bool qt_mac_apply_settings();
    friend class QMacInputMethod;
    friend QMAC_PASCAL OSStatus qt_window_event(EventHandlerCallRef, EventRef, void *);
    friend void qt_mac_update_os_settings();
    friend bool qt_set_socket_handler( int, int, QObject *, bool);
    friend void qt_mac_destroy_widget(QWidget *);
    friend void qt_init(QApplicationPrivate *priv, QApplication::Type);
#endif

    static QStyle   *app_style;
    static int	     app_cspec;
#ifndef QT_NO_PALETTE
    static QPalette *app_pal;
#endif
    static QFont    *app_font;
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
    static bool	     animate_toolbox;
    static bool	     widgetCount; // Coupled with -widgetcount switch

#if defined(Q_WS_X11) && !defined (QT_NO_STYLE )
    static void x11_initialize_style();
#endif

    static QSize     app_strut;
    static QAsciiDict<QPalette> *app_palettes;
    static QAsciiDict<QFont>    *app_fonts;

    static QWidgetList *popupWidgets;
    bool	     inPopupMode() const;
    void	     closePopup( QWidget *popup );
    void	     openPopup( QWidget *popup );
    void	     setActiveWindow( QWidget* act );

    // ### the next 2 friends should go away
    friend class QEventLoop;
    friend class QEvent;
    friend class QWidget;
    friend class QETWidget;
    friend class QDialog;
    friend class QAccelManager;
    friend class QTranslator;
#if defined(Q_WS_QWS)
    friend class QInputContext;
#endif
private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QApplication( const QApplication & );
    QApplication &operator=( const QApplication & );
#endif

    Q_DECL_PRIVATE(QApplication);
};

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

inline QSize QApplication::globalStrut()
{
    return app_strut;
}

inline Qt::Alignment QApplication::horizontalAlignment( Alignment align )
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

