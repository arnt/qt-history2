/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.h#190 $
**
** Definition of QWidget class
**
** Created : 931029
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QWIDGET_H
#define QWIDGET_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#include "qpaintdevice.h"
#include "qpalette.h"
#include "qcursor.h"
#include "qfont.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qstyle.h"
//#include "qabstractlayout.h"
#include "qsizepolicy.h"
#endif // QT_H

class QLayout;
struct QWExtra;
class QFocusData;


class Q_EXPORT QWidget : public QObject, public QPaintDevice
{
    Q_OBJECT
public:
    QWidget( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QWidget();

    WId		 winId() const;

  // GUI style setting

    QStyle& style() const;

  // Widget types and states

    bool	 isTopLevel()	const;
    bool	 isModal()	const;
    bool	 isPopup()	const;
    bool	 isDesktop()	const;

    bool	 isEnabled()	const;
    bool	 isEnabledTo(QWidget*) const;
    bool	 isEnabledToTLW() const;
public slots:
    virtual void setEnabled( bool );

  // Widget coordinates

public:
    QRect	 frameGeometry() const;
    const QRect &geometry()	const;
    int		 x()		const;
    int		 y()		const;
    QPoint	 pos()		const;
    QSize	 frameSize()    const;
    QSize	 size()		const;
    int		 width()	const;
    int		 height()	const;
    QRect	 rect()		const;
    QRect	 childrenRect() const;

    QSize	 minimumSize()	 const;
    QSize	 maximumSize()	 const;
    int		 minimumWidth()	 const;
    int		 minimumHeight() const;
    int		 maximumWidth()	 const;
    int		 maximumHeight() const;
    void	 setMinimumSize( const QSize & );
    virtual void setMinimumSize( int minw, int minh );
    void	 setMaximumSize( const QSize & );
    virtual void setMaximumSize( int maxw, int maxh );
    void	 setMinimumWidth( int minw );
    void	 setMinimumHeight( int minh );
    void	 setMaximumWidth( int maxw );
    void	 setMaximumHeight( int maxh );
    virtual void setMask(QBitmap);
    virtual void setMask(const QRegion&);
    void	 clearMask();

    QSize	 sizeIncrement() const;
    void	 setSizeIncrement( const QSize & );
    virtual void setSizeIncrement( int w, int h );

    void	setFixedSize( const QSize & );
    void	setFixedSize( int w, int h );
    void	setFixedWidth( int w );
    void	setFixedHeight( int h );

  // Widget coordinate mapping

    QPoint	 mapToGlobal( const QPoint & )	 const;
    QPoint	 mapFromGlobal( const QPoint & ) const;
    QPoint	 mapToParent( const QPoint & )	 const;
    QPoint	 mapFromParent( const QPoint & ) const;

    QWidget	*topLevelWidget()   const;

  // Widget attribute functions

    enum BackgroundMode { FixedColor, FixedPixmap, NoBackground,
			  PaletteForeground, PaletteButton, PaletteLight,
			  PaletteMidlight, PaletteDark, PaletteMid,
			  PaletteText, PaletteBrightText, PaletteBase,
			  PaletteBackground, PaletteShadow, PaletteHighlight,
			  PaletteHighlightedText };

    BackgroundMode backgroundMode() const;
    virtual void	   setBackgroundMode( BackgroundMode );

    const QColor  &backgroundColor() const;
    const QColor  &foregroundColor() const;
    virtual void  setBackgroundColor( const QColor & );

    const QPixmap *backgroundPixmap() const;
    virtual void   setBackgroundPixmap( const QPixmap & );

    const QColorGroup &colorGroup() const;
    const QPalette    &palette()    const;
    virtual void       setPalette( const QPalette & );
    void setPalette( const QPalette &, bool fixed );

    const QFont &font()		const;
    virtual void setFont( const QFont & );
    void setFont( const QFont &, bool fixed );
    QFontMetrics fontMetrics()	const;
    QFontInfo	 fontInfo()	const;

    enum PropagationMode { NoChildren, AllChildren,
			   SameFont, SamePalette = SameFont };

    PropagationMode fontPropagation() const;
    virtual void setFontPropagation( PropagationMode );

    PropagationMode palettePropagation() const;
    virtual void setPalettePropagation( PropagationMode );

    const QCursor      &cursor() const;
    virtual void	setCursor( const QCursor & );

    QString		caption() const;
    const QPixmap      *icon() const;
    QString		iconText() const;
    bool		hasMouseTracking() const;

public slots:
    virtual void	setCaption( const QString &);
    virtual void	setIcon( const QPixmap & );
    virtual void	setIconText( const QString &);
    virtual void	setMouseTracking( bool enable );

  // Keyboard input focus functions

public:
    enum FocusPolicy
    { NoFocus = 0, TabFocus = 0x1, ClickFocus = 0x2, StrongFocus = 0x3 };

    bool	 isActiveWindow() const;
    virtual void setActiveWindow();
    bool	 isFocusEnabled() const;
    FocusPolicy	 focusPolicy() const;
    virtual void setFocusPolicy( FocusPolicy );
    bool	 hasFocus() const;
    virtual void setFocus();
    void	 clearFocus();
    static void	 setTabOrder( QWidget *, QWidget * );
    virtual void setFocusProxy( QWidget * );
    QWidget *	 focusProxy() const;

  // Grab functions

    void	 grabMouse();
    void	 grabMouse( const QCursor & );
    void	 releaseMouse();
    void	 grabKeyboard();
    void	 releaseKeyboard();
    static QWidget *mouseGrabber();
    static QWidget *keyboardGrabber();

  // Update/refresh functions

    bool	 isUpdatesEnabled() const;

public slots:
    virtual void	 setUpdatesEnabled( bool enable );
    void	 update();
    void	 update( int x, int y, int w, int h );
    void	 update( const QRect& );
    void	 repaint( bool erase=TRUE );
    void	 repaint( int x, int y, int w, int h, bool erase=TRUE );
    void	 repaint( const QRect &, bool erase=TRUE );
    void	 repaint( const QRegion &, bool erase=TRUE );

  // Widget management functions

    virtual void show();
    virtual void hide();
#ifndef QT_NO_COMPAT
    void	 iconify()
	{ showMinimized(); }
#endif
    virtual void showMinimized();
    virtual void showMaximized();
    virtual void showNormal();
    virtual void polish();
    bool	 close();

public:
    virtual bool close( bool forceKill );
    bool	 isVisible()	const;
    bool	 isVisibleTo(QWidget*) const;
    bool	 isVisibleToTLW() const;

public slots:
    void	 raise();
    void	 lower();
    virtual void move( int x, int y );
    void	 move( const QPoint & );
    virtual void resize( int w, int h );
    void	 resize( const QSize & );
    virtual void setGeometry( int x, int y, int w, int h );
    virtual void setGeometry( const QRect & );

public:
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    virtual void  adjustSize();
    QLayout *layout() const { return lay_out; }
    virtual void reparent( QWidget *parent, WFlags, const QPoint &,
			   bool showIt=FALSE );
#ifndef QT_NO_COMPAT
    void	 recreate( QWidget *parent, WFlags f, const QPoint & p,
			   bool showIt=FALSE )
	{ reparent(parent,f,p,showIt); }
#endif

    void	 erase();
    void	 erase( int x, int y, int w, int h );
    void	 erase( const QRect & );
    void	 erase( const QRegion & );
    void	 scroll( int dx, int dy );

    void	 drawText( int x, int y, const QString &);
    void	 drawText( const QPoint &, const QString &);

  // Misc. functions

    QWidget	*focusWidget() const;

  // drag and drop

    virtual void setAcceptDrops( bool on );
    bool	 acceptDrops() const;

    virtual void setAutoMask(bool);
    bool autoMask() const;

				
public:
    QWidget	*parentWidget() const;
    bool	 testWFlags( WFlags n ) const;
    static QWidget	 *find( WId );
    static QWidgetMapper *wmapper();

  // Event handlers

protected:
    bool	 event( QEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void wheelEvent( QWheelEvent * );
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );
    virtual void enterEvent( QEvent * );
    virtual void leaveEvent( QEvent * );
    virtual void paintEvent( QPaintEvent * );
    virtual void moveEvent( QMoveEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual void closeEvent( QCloseEvent * );

    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent * );

    virtual void showEvent( QShowEvent * );
    virtual void hideEvent( QHideEvent * );
    virtual void customEvent( QCustomEvent * );

#if defined(_WS_MAC_)
    virtual bool macEvent( MSG * );		// Macintosh event
#elif defined(_WS_WIN_)
    virtual bool winEvent( MSG * );		// Windows event
#elif defined(_WS_X11_)
    virtual bool x11Event( XEvent * );		// X11 event
#endif

    virtual void updateMask();

  // Misc. protected functions

protected:
    virtual void styleChange( GUIStyle );
    virtual void enabledChange( bool );
    virtual void backgroundColorChange( const QColor & );
    virtual void backgroundPixmapChange( const QPixmap & );
    virtual void paletteChange( const QPalette & );
    virtual void fontChange( const QFont & );

    int		 metric( int )	const;

    virtual void create( WId = 0, bool initializeWindow = TRUE,
			 bool destroyOldWindow = TRUE );
    virtual void destroy( bool destroyWindow = TRUE,
			  bool destroySubWindows = TRUE );
    WFlags	 getWFlags()	const;
    virtual void setWFlags( WFlags );
    void	 clearWFlags( WFlags n );

    virtual void setFRect( const QRect & );
    virtual void setCRect( const QRect & );

    virtual bool focusNextPrevChild( bool next );

    QWExtra	*extraData();

    QFocusData	*focusData();

    void	repaintResizedBorder( QResizeEvent*, int bw );

    void	setSizeGrip(bool);
    void	setKeyCompression(bool);

private slots:
    void	 focusProxyDestroyed();

private:
    void 	 setLayout( QLayout *l );
    virtual void setWinId( WId );
    void	 showWindow();
    void	 hideWindow();
    void	 createTLExtra();
    void	 createExtra();
    void	 createSysExtra();
    void	 deleteExtra();
    void	 deleteSysExtra();
    void	 internalMove( int, int );
    void	 internalResize( int, int );
    void	 internalSetGeometry( int, int, int, int );
    void	 deferMove( const QPoint & );
    void	 deferResize( const QSize & );
    void	 cancelMove();
    void	 cancelResize();
    void	 sendDeferredEvents();
    void	 reparentFocusWidgets( QWidget *parent );
    QFocusData	*focusData( bool create );
    void         setBackgroundFromMode();
    void         setBackgroundColorDirect( const QColor & );
    void   	    setBackgroundPixmapDirect( const QPixmap & );
    void         setBackgroundModeDirect( BackgroundMode );
    void         setBackgroundEmpty();

    WId		 winid;
    WFlags	 flags;
    QPoint	 fpos;
    QRect	 crect;
    QColor	 bg_col;
    QPalette	 pal;
    QFont	 fnt;
    QLayout 	*lay_out;
    QWExtra	*extra;
    uint automask : 1;
    uint polished : 1;
    uint paletteState : 2; // 0 uninitialized, 1 initialized, 2 fix
    uint fontState : 2; // 0 uninitialized, 1 initialized, 2 fix
    uint propagateFont: 2;
    uint propagatePalette: 2;
    uint dnd : 1; // drop enable
    uint keyCompression : 1;
#if defined(_WS_X11_)
    uint usposition : 1;
#endif    

    static void	 createMapper();
    static void	 destroyMapper();
    static QWidgetList	 *wList();
    static QWidgetList	 *tlwList();
    static QWidgetMapper *mapper;
    friend class QApplication;
    friend class QPainter;
    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QETWidget;
#if 1 //def TOTAL_LOSER_COMPILER
    friend class QLayout;
#else
    friend void QLayout::setWidgetLayout( QWidget *, QLayout * );
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWidget( const QWidget & );
    QWidget &operator=( const QWidget & );
#endif
};


inline bool QWidget::testWFlags( WFlags f ) const
{ return (flags & f) != 0; }

inline WId QWidget::winId() const
{ return winid; }

inline bool QWidget::isTopLevel() const
{ return testWFlags(WType_TopLevel); }

inline bool QWidget::isModal() const
{ return testWFlags(WType_Modal); }

inline bool QWidget::isPopup() const
{ return testWFlags(WType_Popup); }

inline bool QWidget::isDesktop() const
{ return testWFlags(WType_Desktop); }

inline bool QWidget::isEnabled() const
{ return !testWFlags(WState_Disabled); }

inline QRect QWidget::frameGeometry() const
{ return QRect(fpos,frameSize()); }

inline const QRect &QWidget::geometry() const
{ return crect; }

inline int QWidget::x() const
{ return fpos.x(); }

inline int QWidget::y() const
{ return fpos.y(); }

inline QPoint QWidget::pos() const
{ return fpos; }

inline QSize QWidget::size() const
{ return crect.size(); }

inline int QWidget::width() const
{ return crect.width(); }

inline int QWidget::height() const
{ return crect.height(); }

inline QRect QWidget::rect() const
{ return QRect(0,0,crect.width(),crect.height()); }

inline int QWidget::minimumWidth() const
{ return minimumSize().width(); }

inline int QWidget::minimumHeight() const
{ return minimumSize().height(); }

inline int QWidget::maximumWidth() const
{ return minimumSize().width(); }

inline int QWidget::maximumHeight() const
{ return minimumSize().height(); }

inline void QWidget::setMinimumSize( const QSize &s )
{ setMinimumSize(s.width(),s.height()); }

inline void QWidget::setMaximumSize( const QSize &s )
{ setMaximumSize(s.width(),s.height()); }

inline void QWidget::setSizeIncrement( const QSize &s )
{ setSizeIncrement(s.width(),s.height()); }

inline const QColor &QWidget::backgroundColor() const
{ return bg_col; }

inline QFontMetrics QWidget::fontMetrics() const
{ return QFontMetrics(font()); }

inline QFontInfo QWidget::fontInfo() const
{ return QFontInfo(font()); }

inline bool QWidget::hasMouseTracking() const
{ return testWFlags(WState_MouseTracking); }

inline bool  QWidget::isFocusEnabled() const
{ return testWFlags(WState_TabToFocus|WState_ClickToFocus); }

inline QWidget::FocusPolicy QWidget::focusPolicy() const
{ return (FocusPolicy)((testWFlags(WState_TabToFocus) ? (int)TabFocus : 0) +
		       (testWFlags(WState_ClickToFocus)?(int)ClickFocus:0)); }

inline bool QWidget::isUpdatesEnabled() const
{ return !testWFlags(WState_BlockUpdates); }

inline void QWidget::update( const QRect &r )
{ update( r.x(), r.y(), r.width(), r.height() ); }

inline void QWidget::repaint( bool erase )
{ repaint( 0, 0, crect.width(), crect.height(), erase ); }

inline void QWidget::repaint( const QRect &r, bool erase )
{ repaint( r.x(), r.y(), r.width(), r.height(), erase ); }

inline void QWidget::erase()
{ erase( 0, 0, crect.width(), crect.height() ); }

inline void QWidget::erase( const QRect &r )
{ erase( r.x(), r.y(), r.width(), r.height() ); }

inline bool QWidget::close()
{ return close( FALSE ); }

inline bool QWidget::isVisible() const
{ return testWFlags(WState_Visible); }

inline void QWidget::move( const QPoint &p )
{ move( p.x(), p.y() ); }

inline void QWidget::resize( const QSize &s )
{ resize( s.width(), s.height()); }

inline void QWidget::setGeometry( const QRect &r )
{ setGeometry( r.left(), r.top(), r.width(), r.height() ); }

inline void QWidget::drawText( const QPoint &p, const QString &s )
{ drawText( p.x(), p.y(), s ); }

inline QWidget *QWidget::parentWidget() const
{ return (QWidget *)QObject::parent(); }

inline QWidgetMapper *QWidget::wmapper()
{ return mapper; }

inline WFlags QWidget::getWFlags() const
{ return flags; }

inline void QWidget::setWFlags( WFlags f )
{ flags |= f; }

inline void QWidget::clearWFlags( WFlags f )
{ flags &= ~f; }


// Extra QWidget data
//  - to minimize memory usage for members that are seldom used.
//  - top-level widgets have extra extra data to reduce cost further

class QFocusData;
#if defined(_WS_WIN_)
class QOleDropTarget;
#endif

struct QTLWExtra {
    QString  caption;				// widget caption
    QString  iconText;				// widget icon text
    QPixmap *icon;				// widget icon
    QFocusData *focusData;			// focus data (for TLW)
    QSize    fsize;				// rect of frame
    short    incw, inch;			// size increments
    uint     iconic: 1;				// iconified [cur. win32 only]
#if defined(_WS_X11_)
    QRect    normalGeometry;			// used by showMin/maximized
#endif
};

struct QWExtra {
    short    minw, minh;			// minimum size
    short    maxw, maxh;			// maximum size
    QPixmap *bg_pix;				// background pixmap
    QWidget *focus_proxy;
    QCursor *curs;
    QTLWExtra *topextra;			// only useful for TLWs
#if defined(_WS_WIN_)
    HANDLE   winIcon;				// internal Windows icon
    QOleDropTarget *dropTarget;			// drop target
#endif
#if defined(_WS_X11_)
    void    *xic;				// XIM Input Context
#endif
    char     bg_mode;				// background mode
    uint sizegrip : 1;				// size grip
};



#endif // QWIDGET_H
