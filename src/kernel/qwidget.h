/****************************************************************************
**
** Definition of QWidget class.
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

#ifndef QWIDGET_H
#define QWIDGET_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#include "qpaintdevice.h"
#include "qpalette.h"
#include "qfont.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qsizepolicy.h"
#include "qregion.h"
#endif // QT_H

#ifdef QT_INCLUDE_COMPAT
#include "qevent.h"
#endif

class QLayout;
class QCursor;
class QWSRegionManager;
class QStyle;

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QFocusEvent;
class QPaintEvent;
class QMoveEvent;
class QResizeEvent;
class QCloseEvent;
class QContextMenuEvent;
class QIMEvent;
class QTabletEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QShowEvent;
class QHideEvent;


struct QWidgetPrivate;

class Q_EXPORT QWidget : public QObject, public QPaintDevice
{
    Q_OBJECT
    Q_ENUMS( BackgroundMode FocusPolicy BackgroundOrigin )
    Q_PROPERTY( bool isTopLevel READ isTopLevel )
    Q_PROPERTY( bool isDialog READ isDialog )
    Q_PROPERTY( bool isModal READ isModal )
    Q_PROPERTY( bool isPopup READ isPopup )
    Q_PROPERTY( bool isDesktop READ isDesktop )
    Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled )
    Q_PROPERTY( QRect geometry READ geometry WRITE setGeometry )
    Q_PROPERTY( QRect frameGeometry READ frameGeometry )
    Q_PROPERTY( int x READ x )
    Q_PROPERTY( int y READ y )
    Q_PROPERTY( QPoint pos READ pos WRITE move DESIGNABLE false STORED false )
    Q_PROPERTY( QSize frameSize READ frameSize )
    Q_PROPERTY( QSize size READ size WRITE resize DESIGNABLE false STORED false )
    Q_PROPERTY( int width READ width )
    Q_PROPERTY( int height READ height )
    Q_PROPERTY( QRect rect READ rect )
    Q_PROPERTY( QRect childrenRect READ childrenRect )
    Q_PROPERTY( QRegion childrenRegion READ childrenRegion )
    Q_PROPERTY( QSizePolicy sizePolicy READ sizePolicy WRITE setSizePolicy )
    Q_PROPERTY( QSize minimumSize READ minimumSize WRITE setMinimumSize )
    Q_PROPERTY( QSize maximumSize READ maximumSize WRITE setMaximumSize )
    Q_PROPERTY( int minimumWidth READ minimumWidth WRITE setMinimumWidth STORED false DESIGNABLE false )
    Q_PROPERTY( int minimumHeight READ minimumHeight WRITE setMinimumHeight STORED false DESIGNABLE false )
    Q_PROPERTY( int maximumWidth READ maximumWidth WRITE setMaximumWidth STORED false DESIGNABLE false )
    Q_PROPERTY( int maximumHeight READ maximumHeight WRITE setMaximumHeight STORED false DESIGNABLE false )
    Q_PROPERTY( QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement )
    Q_PROPERTY( QSize baseSize READ baseSize WRITE setBaseSize )
    Q_PROPERTY( BackgroundMode backgroundMode READ backgroundMode WRITE setBackgroundMode DESIGNABLE false )
    Q_PROPERTY( QColor paletteForegroundColor READ paletteForegroundColor WRITE setPaletteForegroundColor RESET unsetPalette )
    Q_PROPERTY( QColor paletteBackgroundColor READ paletteBackgroundColor WRITE setPaletteBackgroundColor RESET unsetPalette )
    Q_PROPERTY( QPixmap paletteBackgroundPixmap READ paletteBackgroundPixmap WRITE setPaletteBackgroundPixmap RESET unsetPalette )
    Q_PROPERTY( QBrush backgroundBrush READ backgroundBrush )
    Q_PROPERTY( QColorGroup colorGroup READ colorGroup )
    Q_PROPERTY( QPalette palette READ palette WRITE setPalette RESET unsetPalette  STORED ownPalette )
    Q_PROPERTY( BackgroundOrigin backgroundOrigin READ backgroundOrigin WRITE setBackgroundOrigin )
    Q_PROPERTY( bool ownPalette READ ownPalette )
    Q_PROPERTY( QFont font READ font WRITE setFont RESET unsetFont STORED ownFont )
    Q_PROPERTY( bool ownFont READ ownFont )
#ifndef QT_NO_CURSOR
    Q_PROPERTY( QCursor cursor READ cursor WRITE setCursor RESET unsetCursor STORED ownCursor )
    Q_PROPERTY( bool ownCursor READ ownCursor )
#endif
#ifndef QT_NO_WIDGET_TOPEXTRA
    Q_PROPERTY( QString caption READ caption WRITE setCaption )
    Q_PROPERTY( QPixmap icon READ icon WRITE setIcon )
    Q_PROPERTY( QString iconText READ iconText WRITE setIconText )
#endif
    Q_PROPERTY( bool mouseTracking READ hasMouseTracking WRITE setMouseTracking )
    Q_PROPERTY( bool isActiveWindow READ isActiveWindow )
    Q_PROPERTY( bool focusEnabled READ isFocusEnabled )
    Q_PROPERTY( FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy )
    Q_PROPERTY( bool focus READ hasFocus )
    Q_PROPERTY( bool updatesEnabled READ isUpdatesEnabled WRITE setUpdatesEnabled DESIGNABLE false )
    Q_PROPERTY( bool visible READ isVisible )
    Q_PROPERTY( QRect visibleRect READ visibleRect ) // obsolete
    Q_PROPERTY( bool hidden READ isHidden WRITE setHidden DESIGNABLE false SCRIPTABLE false )
    Q_PROPERTY( bool shown READ isShown WRITE setShown DESIGNABLE false SCRIPTABLE false )
    Q_PROPERTY( bool minimized READ isMinimized )
    Q_PROPERTY( bool maximized READ isMaximized )
    Q_PROPERTY( bool fullScreen READ isFullScreen )
    Q_PROPERTY( QSize sizeHint READ sizeHint )
    Q_PROPERTY( QSize minimumSizeHint READ minimumSizeHint )
    Q_PROPERTY( QRect microFocusHint READ microFocusHint )
    Q_PROPERTY( bool acceptDrops READ acceptDrops WRITE setAcceptDrops )
    Q_PROPERTY( bool autoMask READ autoMask WRITE setAutoMask DESIGNABLE false SCRIPTABLE false )
    Q_PROPERTY( bool customWhatsThis READ customWhatsThis )
    Q_PROPERTY( bool inputMethodEnabled READ isInputMethodEnabled WRITE setInputMethodEnabled DESIGNABLE false SCRIPTABLE false )

public:
    explicit QWidget( QWidget* parent=0, const char* name=0, WFlags f=0 );
    ~QWidget();

    WId		 winId() const;
    void	 setName( const char *name );
#ifndef QT_NO_STYLE
    // GUI style setting

    QStyle     &style() const;
    void        setStyle( QStyle * );
    QStyle*	setStyle( const QString& );
#endif
    // Widget types and states

    bool	 isTopLevel()	const;
    bool	 isDialog()	const;
    bool	 isPopup()	const;
    bool	 isDesktop()	const;
    bool	 isModal()	const;

    bool	 isEnabled()	const;
    bool	 isEnabledTo(QWidget*) const;
    bool	 isEnabledToTLW() const;

public slots:
    void setEnabled( bool );
    void setDisabled( bool );

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
    QRegion	 childrenRegion() const;

    QSize	 minimumSize()	 const;
    QSize	 maximumSize()	 const;
    int		 minimumWidth()	 const;
    int		 minimumHeight() const;
    int		 maximumWidth()	 const;
    int		 maximumHeight() const;
    void	 setMinimumSize( const QSize & );
    void setMinimumSize( int minw, int minh );
    void	 setMaximumSize( const QSize & );
    void setMaximumSize( int maxw, int maxh );
    void	 setMinimumWidth( int minw );
    void	 setMinimumHeight( int minh );
    void	 setMaximumWidth( int maxw );
    void	 setMaximumHeight( int maxh );

    QSize	 sizeIncrement() const;
    void	 setSizeIncrement( const QSize & );
    void setSizeIncrement( int w, int h );
    QSize	 baseSize() const;
    void	 setBaseSize( const QSize & );
    void	 setBaseSize( int basew, int baseh );

    void	setFixedSize( const QSize & );
    void	setFixedSize( int w, int h );
    void	setFixedWidth( int w );
    void	setFixedHeight( int h );

    // Widget coordinate mapping

    QPoint	 mapToGlobal( const QPoint & )	 const;
    QPoint	 mapFromGlobal( const QPoint & ) const;
    QPoint	 mapToParent( const QPoint & )	 const;
    QPoint	 mapFromParent( const QPoint & ) const;
    QPoint	 mapTo( QWidget *, const QPoint & ) const;
    QPoint	 mapFrom( QWidget *, const QPoint & ) const;

    QWidget	*topLevelWidget()   const;

    // Widget appearance functions

    BackgroundMode	backgroundMode() const;
    void setBackgroundMode( BackgroundMode );
    void 		setBackgroundMode( BackgroundMode, BackgroundMode );

    const QColor &	foregroundColor() const;

    const QColor &	eraseColor() const;
    void setEraseColor( const QColor & );

    const QPixmap *	erasePixmap() const;
    void setErasePixmap( const QPixmap & );

#ifndef QT_NO_PALETTE
    const QColorGroup & colorGroup() const;
    const QPalette &	palette()    const;
    bool		ownPalette() const;
    void	setPalette( const QPalette & );
    void		unsetPalette();
#endif

    const QColor &	paletteForegroundColor() const;
    void		setPaletteForegroundColor( const QColor & );

    const QColor &	paletteBackgroundColor() const;
    void setPaletteBackgroundColor( const QColor & );

    const QPixmap *	paletteBackgroundPixmap() const;
    void setPaletteBackgroundPixmap( const QPixmap & );

    const QBrush&	backgroundBrush() const;

    QFont		font() const;
    bool		ownFont() const;
    void setFont( const QFont & );
    void		unsetFont();
    QFontMetrics	fontMetrics() const;
    QFontInfo	 	fontInfo() const;

#ifndef QT_NO_CURSOR
    const QCursor      &cursor() const;
    bool		ownCursor() const;
    void setCursor( const QCursor & );
    void unsetCursor();
#endif
#ifndef QT_NO_WIDGET_TOPEXTRA
    QString		caption() const;
    const QPixmap      *icon() const;
    QString		iconText() const;
#endif

    void setMouseTracking(bool enable);
    bool hasMouseTracking() const;
    bool underMouse() const;

    void setMask( const QBitmap & );
    void setMask( const QRegion & );
    void clearMask();

#ifndef QT_NO_COMPAT
    const QColor &backgroundColor() const;
    void setBackgroundColor( const QColor & );
    const QPixmap *backgroundPixmap() const;
    void setBackgroundPixmap( const QPixmap & );
#endif

public slots:
#ifndef QT_NO_WIDGET_TOPEXTRA
    void setCaption( const QString &); // ### becomes setWidgetTitle
    void setIcon( const QPixmap & ); // ### becomes setWidgetIcon(const QIconSet &)
    void setIconText( const QString &); // ### becomes extern qt_x11_setIconText(...)
#endif
    // Keyboard input focus functions

    void setFocus();
    void clearFocus();

public:
    enum FocusPolicy {
	NoFocus = 0,
	TabFocus = 0x1,
	ClickFocus = 0x2,
	StrongFocus = TabFocus | ClickFocus | 0x8,
	WheelFocus = StrongFocus | 0x4
    };

    bool		isActiveWindow() const;
    void setActiveWindow();
    bool		isFocusEnabled() const;

    FocusPolicy		focusPolicy() const;
    void setFocusPolicy( FocusPolicy );
    bool		hasFocus() const;
    static void		setTabOrder( QWidget *, QWidget * );
    void setFocusProxy( QWidget * );
    QWidget *		focusProxy() const;

    void setInputMethodEnabled( bool b );
    bool isInputMethodEnabled() const;
    // Grab functions

    void		grabMouse();
#ifndef QT_NO_CURSOR
    void		grabMouse( const QCursor & );
#endif
    void		releaseMouse();
    void		grabKeyboard();
    void		releaseKeyboard();
    static QWidget *	mouseGrabber();
    static QWidget *	keyboardGrabber();

    // Update/refresh functions

    bool	 	isUpdatesEnabled() const;

#if 0 //def Q_WS_QWS
    void		repaintUnclipped( const QRegion &, bool erase = TRUE );
#endif
public slots:
    void setUpdatesEnabled( bool enable );
    void		update();
    void		update( int x, int y, int w, int h );
    void		update( const QRect& );
    void		repaint();
    void		repaint( bool erase );
    void		repaint( int x, int y, int w, int h, bool erase=TRUE );
    void		repaint( const QRect &, bool erase = TRUE );
    void		repaint( const QRegion &, bool erase = TRUE );

    // Widget management functions

    virtual void	show();
    virtual void	hide();
    void		setShown( bool show );
    void		setHidden( bool hide );
    void showMinimized();
    void showMaximized();
    void showFullScreen();
    void showNormal();
    bool		close();

    void		raise();
    void		lower();
    void		stackUnder( QWidget* );
    void move( int x, int y );
    void		move( const QPoint & );
    void resize( int w, int h );
    void		resize( const QSize & );
    void setGeometry( int x, int y, int w, int h );
    void setGeometry( const QRect & ); // ### make non virtual in Qt 4?
    virtual void  	adjustSize();

public:
    bool close( bool alsoDelete );
    bool		isVisible()	const;
    bool		isVisibleTo(QWidget*) const;
#ifndef QT_NO_COMPAT
    bool		isVisibleToTLW() const; // obsolete
    QRect		visibleRect() const; // obsolete
#endif
    bool 		isHidden() const;
    bool 		isShown() const;
    bool		isMinimized() const;
    bool		isMaximized() const;
    bool		isFullScreen() const;

    virtual QSize	sizeHint() const;
    virtual QSize	minimumSizeHint() const;
    virtual QSizePolicy	sizePolicy() const;
    void	setSizePolicy( QSizePolicy );
    void 		setSizePolicy( QSizePolicy::SizeType hor, QSizePolicy::SizeType ver, bool hfw = FALSE );
    virtual int heightForWidth(int) const;

    QRegion	clipRegion() const;

public:
#ifndef QT_NO_LAYOUT
    QLayout *layout() const;
#endif
    void		updateGeometry();

    void reparent(QWidget *parent, WFlags f);
    void reparent(QWidget *parent);

#ifndef QT_NO_COMPAT
    void iconify() { showMinimized(); }
    void constPolish() const { ensurePolished(); }
    void reparent( QWidget *parent, WFlags f, const QPoint &p, bool showIt=false )
    { reparent(parent, f); move(p); if (showIt) show(); }
    void reparent( QWidget *parent, const QPoint &p, bool showIt=false )
    { reparent(parent, getWFlags() & ~WType_Mask); move(p); if (showIt) show(); }
    void recreate( QWidget *parent, WFlags f, const QPoint & p, bool showIt=false )
    { reparent(parent, f, p, showIt); }
    bool hasMouse() const { return underMouse(); }
#endif

    void		erase();
    void		erase( int x, int y, int w, int h );
    void		erase( const QRect & );
    void		erase( const QRegion & );
    void		scroll( int dx, int dy );
    void		scroll( int dx, int dy, const QRect& );

    void drawText( const QPoint &, const QString &);
    inline void drawText(int x, int y, const QString &s)
	{ drawText(QPoint(x, y), s); }

    // Misc. functions

    QWidget *		focusWidget() const;
    // #### Find a reasonable name
    QWidget *nextInFocusChain() const;
    QRect               microFocusHint() const;

    // drag and drop

    bool		acceptDrops() const;
    void setAcceptDrops( bool on );

    // transparency and pseudo transparency

    void setAutoMask(bool);
    bool		autoMask() const;

    enum BackgroundOrigin { WidgetOrigin, ParentOrigin, WindowOrigin, AncestorOrigin };

    void setBackgroundOrigin( BackgroundOrigin );
    BackgroundOrigin backgroundOrigin() const;
    QPoint backgroundOffset() const;

    // whats this help
    virtual bool customWhatsThis() const;

    QWidget *parentWidget() const;

    WState		testWState( WState s ) const;
    WFlags		testWFlags( WFlags f ) const;
    static QWidget *	find( WId );
    static QWidgetMapper *wmapper();

    QWidget  *childAt( int x, int y, bool includeThis = FALSE ) const;
    QWidget  *childAt( const QPoint &, bool includeThis = FALSE ) const;

#if defined(Q_WS_QWS)
    virtual QGfx * graphicsContext(bool clip_children=TRUE) const;
#endif
#if defined(Q_WS_MAC)
    QRegion clippedRegion(bool do_children=TRUE);
    uint clippedSerial(bool do_children=TRUE);
#ifndef QMAC_NO_QUARTZ
    CGContextRef macCGContext(bool clipped=TRUE) const;
#endif
#endif

    enum WidgetAttribute {
	WA_Disabled,
	WA_UnderMouse,
	WA_MouseTracking,

	WA_ForceDisabled = 32,
	WA_KeyCompression,
	WA_PendingMoveEvent,
	WA_PendingResizeEvent
    };
    void setAttribute(WidgetAttribute, bool);
    bool testAttribute(WidgetAttribute) const;

protected:
    // Event handlers
    bool	 event( QEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent( QWheelEvent * );
#endif
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
    virtual void contextMenuEvent( QContextMenuEvent * );
    virtual void imStartEvent( QIMEvent * );
    virtual void imComposeEvent( QIMEvent * );
    virtual void imEndEvent( QIMEvent * );
    virtual void tabletEvent( QTabletEvent * );

#ifndef QT_NO_DRAGANDDROP
    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent * );
#endif

    virtual void showEvent( QShowEvent * );
    virtual void hideEvent( QHideEvent * );

#if defined(Q_WS_MAC)
    virtual bool macEvent( MSG * );
#endif
#if defined(Q_WS_WIN)
    virtual bool winEvent( MSG * );
#endif
#if defined(Q_WS_X11)
    virtual bool x11Event( XEvent * );
#endif
#if defined(Q_WS_QWS)
    virtual bool qwsEvent( QWSEvent * );
    virtual unsigned char *scanLine( int ) const;
    virtual int bytesPerLine() const;
#endif

    virtual void updateMask();

    // Misc. protected functions

#ifndef QT_NO_STYLE
    virtual void styleChange( QStyle& );
#endif
    virtual void enabledChange( bool oldEnabled );
#ifndef QT_NO_PALETTE
    virtual void paletteChange( const QPalette & );
#endif
    virtual void fontChange( const QFont & );
    virtual void windowActivationChange( bool oldActive );
    virtual void languageChange();

    int		 metric( int )	const;

    void	 resetInputContext();

    void create( WId = 0, bool initializeWindow = TRUE,
			 bool destroyOldWindow = TRUE );
    void destroy( bool destroyWindow = TRUE,
			  bool destroySubWindows = TRUE );
    uint	 getWState() const;
    void setWState( uint );
    void	 clearWState( uint n );
    WFlags	 getWFlags() const;
    void setWFlags( WFlags );
    void	 clearWFlags( WFlags n );

    virtual bool focusNextPrevChild( bool next );

    void setMicroFocusHint(int x, int y, int w, int h, bool text=TRUE, QFont *f = 0);

#if defined(Q_WS_MAC)
    void dirtyClippedRegion(bool);
    bool isClippedRegionDirty();
    virtual void setRegionDirty(bool);
    virtual void macWidgetChangedWindow();
#endif

#ifndef QT_NO_COMPAT
public:
    QWidget *parentWidget( bool sameWindow ) const;
    inline void setKeyCompression(bool b) { setAttribute(WA_KeyCompression, b); }
    inline void setFont( const QFont &f, bool ) { setFont( f ); }
#ifndef QT_NO_PALETTE
    inline void setPalette( const QPalette &p, bool ) { setPalette( p ); }
#endif
#endif

protected:
    explicit QWidget( QWidgetPrivate *d, QWidget* parent, const char* name, WFlags f);
private:
    void	 setFontSys( QFont *f = 0 );
#if defined(Q_WS_MAC)
    uint    own_id : 1, macDropEnabled : 1;
    EventHandlerRef window_event;
    //mac event functions
    void    propagateUpdates(bool update_rgn=TRUE);
    void    update( const QRegion& );
    //friends, way too many - fix this immediately!
    friend void qt_clean_root_win();
    friend bool qt_recreate_root_win();
    friend QPoint posInWindow(QWidget *);
    friend bool qt_mac_update_sizer(QWidget *, int);
    friend QWidget *qt_recursive_match(QWidget *widg, int x, int y);
    friend bool qt_paint_children(QWidget *,QRegion &, uchar ops);
    friend QMAC_PASCAL OSStatus qt_window_event(EventHandlerCallRef er, EventRef event, void *);
    friend void qt_event_request_updates(QWidget *, const QRegion &, bool subtract=FALSE);
    friend bool qt_window_rgn(WId, short, RgnHandle, bool);
    friend class QDragManager;
#endif

    void	 setWinId( WId );
    void	 showWindow();
    void	 hideWindow();
    void	 showChildren( bool spontaneous );
    void	 hideChildren( bool spontaneous );
    void	 reparentSys( QWidget *parent, WFlags, const QPoint &,  bool showIt);
    void	 deactivateWidgetCleanup();
    void setGeometry_helper(int, int, int, int, bool);
    void show_helper();
    void hide_helper();
    void setEnabled_helper(bool);
    void	 reparentFocusWidgets( QWidget * );
    void         setBackgroundFromMode();
    void         setBackgroundColorDirect( const QColor & );
    void   	 setBackgroundPixmapDirect( const QPixmap & );
    void         setBackgroundModeDirect( BackgroundMode );
    void         setBackgroundEmpty();
    void	 updateFrameStrut() const;

    WId		 winid;
    uint	 widget_state; // will go away, eventually
    uint widget_attributes;
    bool testAttribute_helper(WidgetAttribute) const;
    uint	 widget_flags;
    uint	 focus_policy : 4;
    uint 	 own_font :1;
    uint 	 own_palette :1;
    uint 	 sizehint_forced :1;
    uint 	 is_closing :1;
    uint 	 in_show : 1;
    uint 	 in_show_maximized : 1;
    uint	 fstrut_dirty : 1;
    uint	 im_enabled : 1;
    QRect	 crect;
    QColor	 bg_col;
#ifndef QT_NO_PALETTE
    QPalette	 pal;
#endif
    QFont	 fnt;
#if defined(Q_WS_QWS)
    QRegion	 req_region;			// Requested region
    mutable QRegion	 paintable_region;	// Paintable region
    mutable bool         paintable_region_dirty;// needs to be recalculated
    mutable QRegion      alloc_region;          // Allocated region
    mutable bool         alloc_region_dirty;    // needs to be recalculated
    mutable int          overlapping_children;  // Handle overlapping children

    int		 alloc_region_index;
    int		 alloc_region_revision;

    void updateOverlappingChildren() const;
    void setChildrenAllocatedDirty();
    void setChildrenAllocatedDirty( const QRegion &r, const QWidget *dirty=0 );
    bool isAllocatedRegionDirty() const;
    void updateRequestedRegion( const QPoint &gpos );
    QRegion requestedRegion() const;
    QRegion allocatedRegion() const;
    QRegion paintableRegion() const;

    void updateGraphicsContext( QGfx *qgfx_qws, bool clip_children ) const;
#ifndef QT_NO_CURSOR
    void updateCursor( const QRegion &r ) const;
#endif

    // used to accumulate dirty region when children moved/resized.
    QRegion dirtyChildren;
    bool isSettingGeometry;
    friend class QWSManager;
#endif
    static int instanceCounter;  // Current number of widget instances
    static int maxInstances;     // Maximum number of widget instances

    static QWidgetMapper *mapper;

    friend class QApplication;
    friend class QBaseApplication;
    friend class QPainter;
    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QETWidget;
    friend class QLayout;
    friend class QWidgetItem;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWidget( const QWidget & );
    QWidget &operator=( const QWidget & );
#endif

    Q_DECL_PRIVATE( QWidget );
};

typedef QPointer<QWidget> QWidgetPointer;


inline Qt::WState QWidget::testWState( WState s ) const
{ return (widget_state & s); }

inline Qt::WFlags QWidget::testWFlags( WFlags f ) const
{ return (widget_flags & f); }


inline WId QWidget::winId() const
{ return winid; }

inline bool QWidget::isTopLevel() const
{ return testWFlags(WType_TopLevel); }

inline bool QWidget::isDialog() const
{ return testWFlags(WType_Dialog); }

inline bool QWidget::isPopup() const
{ return testWFlags(WType_Popup); }

inline bool QWidget::isDesktop() const
{ return testWFlags(WType_Desktop); }

inline bool QWidget::isEnabled() const
{ return !testAttribute(WA_Disabled); }

inline bool QWidget::isModal() const
{ return testWFlags(WShowModal); }

inline bool QWidget::isEnabledToTLW() const
{ return isEnabled(); }

inline const QRect &QWidget::geometry() const
{ return crect; }

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
{ return maximumSize().width(); }

inline int QWidget::maximumHeight() const
{ return maximumSize().height(); }

inline void QWidget::setMinimumSize( const QSize &s )
{ setMinimumSize(s.width(),s.height()); }

inline void QWidget::setMaximumSize( const QSize &s )
{ setMaximumSize(s.width(),s.height()); }

inline void QWidget::setSizeIncrement( const QSize &s )
{ setSizeIncrement(s.width(),s.height()); }

inline void QWidget::setBaseSize( const QSize &s )
{ setBaseSize(s.width(),s.height()); }

inline const QColor &QWidget::eraseColor() const
{ return bg_col; }

#ifndef QT_NO_PALETTE
inline const QPalette &QWidget::palette() const
{ return pal; }
#endif

inline QFont QWidget::font() const
{ return fnt; }

inline QFontMetrics QWidget::fontMetrics() const
{ return QFontMetrics(font()); }

inline QFontInfo QWidget::fontInfo() const
{ return QFontInfo(font()); }

inline void QWidget::setMouseTracking(bool enable)
{ setAttribute(WA_MouseTracking, enable); }

inline bool QWidget::hasMouseTracking() const
{ return testAttribute(WA_MouseTracking); }

inline bool QWidget::underMouse() const
{ return testAttribute(WA_UnderMouse); }

inline bool  QWidget::isFocusEnabled() const
{ return focusPolicy() != NoFocus; }

inline bool QWidget::isUpdatesEnabled() const
{ return !testWState(WState_BlockUpdates); }

inline void QWidget::update( const QRect &r )
{ update( r.x(), r.y(), r.width(), r.height() ); }

inline void QWidget::repaint()
{ repaint( TRUE ); }

inline void QWidget::repaint( const QRect &r, bool erase )
{ repaint( r.x(), r.y(), r.width(), r.height(), erase ); }

inline void QWidget::erase()
{ erase( 0, 0, crect.width(), crect.height() ); }

inline void QWidget::erase( const QRect &r )
{ erase( r.x(), r.y(), r.width(), r.height() ); }

inline bool QWidget::close()
{ return close( FALSE ); }

inline bool QWidget::isVisible() const
{ return testWState(WState_Visible); }

inline bool QWidget::isVisibleToTLW() const // obsolete
{ return isVisible(); }

inline bool QWidget::isHidden() const
{ return testWState(WState_Hidden); }

inline bool QWidget::isShown() const
{ return !testWState(WState_Hidden); }

inline void QWidget::move( const QPoint &p )
{ move( p.x(), p.y() ); }

inline void QWidget::resize( const QSize &s )
{ resize( s.width(), s.height()); }

inline void QWidget::setGeometry( const QRect &r )
{ setGeometry( r.left(), r.top(), r.width(), r.height() ); }

#ifndef QT_NO_COMPAT
inline QWidget *QWidget::parentWidget( bool sameWindow ) const
{
    if ( sameWindow )
	return isTopLevel() ? 0 : (QWidget *)QObject::parent();
    return (QWidget *)QObject::parent();
}
#endif

inline QWidget *QWidget::parentWidget() const
{ return static_cast<QWidget *>(QObject::parent()); }

inline QWidgetMapper *QWidget::wmapper()
{ return mapper; }

inline uint QWidget::getWState() const
{ return widget_state; }

inline void QWidget::setWState( uint f )
{ widget_state |= f; }

inline void QWidget::clearWState( uint f )
{ widget_state &= ~f; }

inline Qt::WFlags QWidget::getWFlags() const
{ return widget_flags; }

inline void QWidget::setWFlags( WFlags f )
{ widget_flags |= f; }

inline void QWidget::clearWFlags( WFlags f )
{ widget_flags &= ~f; }

#ifndef QT_NO_CURSOR
inline bool QWidget::ownCursor() const
{
    return testWState( WState_OwnCursor );
}
#endif
inline bool QWidget::ownFont() const
{
    return own_font;
}
#ifndef QT_NO_PALETTE
inline bool QWidget::ownPalette() const
{
    return own_palette;
}
#endif

inline void QWidget::setSizePolicy( QSizePolicy::SizeType hor, QSizePolicy::SizeType ver, bool hfw )
{
    setSizePolicy( QSizePolicy( hor, ver, hfw) );
}

inline bool QWidget::isInputMethodEnabled() const
{
    return (bool)im_enabled;
}

inline bool QWidget::testAttribute(WidgetAttribute attribute) const
{
    if (attribute < (1<<sizeof(uint)))
	return widget_attributes & (1<<attribute);
    return testAttribute_helper(attribute);
}

#ifndef QT_NO_COMPAT
inline const QColor & QWidget::backgroundColor() const { return eraseColor(); }
inline void QWidget::setBackgroundColor( const QColor &c ) { setEraseColor( c ); }
inline const QPixmap *QWidget::backgroundPixmap() const { return erasePixmap(); }
inline void QWidget::setBackgroundPixmap( const QPixmap &pm ) { setErasePixmap( pm ); }
#endif

#define QWIDGETSIZE_MAX 32767

#define Q_DEFINED_QWIDGET
#include "qwinexport.h"

#endif // QWIDGET_H
