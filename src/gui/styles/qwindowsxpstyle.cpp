/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwindowsxpstyle.h"

#if !defined(QT_NO_STYLE_WINDOWSXP) || defined(QT_PLUGIN)

#include <qmenubar.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtabbar.h>
#include <qheader.h>
#include <qspinbox.h>
#include <qgroupbox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <private/qtitlebar_p.h>
#include <qlistview.h>
#include <qcleanuphandler.h>
#include <qbitmap.h>
#include <qlibrary.h>
#include <qdesktopwidget.h>
#include <qdockwindow.h>
#include <qdockarea.h>
#include <qwidgetstack.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qdrawutil.h>
#include <qmap.h>
#include <qevent.h>
#include <qt_windows.h>

#include <uxtheme.h>
#include <tmschema.h>

#include <limits.h>

/* XPM */
static char * dockCloseXPM[] = {
"8 8 2 1",
" 	c none",
".	c #FFFFFF",
"..    ..",
"...  ...",
" ...... ",
"  ....  ",
"  ....  ",
" ...... ",
"...  ...",
"..    .."};

static ulong ref = 0;
static bool use_xp  = FALSE;
static QMap<QString,HTHEME> *handleMap = 0;

typedef bool ( WINAPI *PtrIsAppThemed )();
typedef bool ( WINAPI *PtrIsThemeActive )();
typedef HRESULT ( WINAPI *PtrGetThemePartSize )( HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
    OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz );
typedef HTHEME ( WINAPI *PtrOpenThemeData )( HWND hwnd, LPCWSTR pszClassList );
typedef HRESULT ( WINAPI *PtrCloseThemeData )( HTHEME hTheme );
typedef HRESULT ( WINAPI *PtrDrawThemeBackground )(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef HRESULT ( WINAPI *PtrGetThemeColor )(HTHEME hTheme, int iPartId,
    int iStateId, int iPropId, OUT COLORREF *pColor);
typedef HRESULT ( WINAPI *PtrGetThemeBackgroundRegion )( HTHEME hTheme, OPTIONAL HDC hdc,
    int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion );
typedef BOOL ( WINAPI *PtrIsThemeBackgroundPartiallyTransparent )(HTHEME hTheme,
    int iPartId, int iStateId);


static PtrIsAppThemed		    pIsAppThemed = 0;
static PtrIsThemeActive		    pIsThemeActive = 0;
static PtrGetThemePartSize	    pGetThemePartSize = 0;
static PtrOpenThemeData		    pOpenThemeData = 0;
static PtrCloseThemeData	    pCloseThemeData = 0;
static PtrDrawThemeBackground	    pDrawThemeBackground = 0;
static PtrGetThemeColor		    pGetThemeColor = 0;
static PtrGetThemeBackgroundRegion  pGetThemeBackgroundRegion = 0;
static PtrIsThemeBackgroundPartiallyTransparent	pIsThemeBackgroundPartiallyTransparent = 0;

bool QWindowsXPStyle::resolveSymbols()
{
    static bool tried = FALSE;
    if ( !tried ) {
	tried = TRUE;
	QLibrary lib( "uxtheme" );
	lib.setAutoUnload( FALSE );

	pIsAppThemed = (PtrIsAppThemed)lib.resolve( "IsAppThemed" );
	if ( pIsAppThemed ) {
	    pIsThemeActive = (PtrIsThemeActive)lib.resolve( "IsThemeActive" );
	    pGetThemePartSize = (PtrGetThemePartSize)lib.resolve( "GetThemePartSize" );
	    pOpenThemeData = (PtrOpenThemeData)lib.resolve( "OpenThemeData" );
	    pCloseThemeData = (PtrCloseThemeData)lib.resolve( "CloseThemeData" );
	    pDrawThemeBackground = (PtrDrawThemeBackground)lib.resolve( "DrawThemeBackground" );
	    pGetThemeColor = (PtrGetThemeColor)lib.resolve( "GetThemeColor" );
	    pGetThemeBackgroundRegion = (PtrGetThemeBackgroundRegion)lib.resolve( "GetThemeBackgroundRegion" );
	    pIsThemeBackgroundPartiallyTransparent = (PtrIsThemeBackgroundPartiallyTransparent)lib.resolve( "IsThemeBackgroundPartiallyTransparent" );
	}
    }

    return pIsAppThemed != 0;
}

class QWindowsXPStylePrivate
{
public:
    QWindowsXPStylePrivate()
	: hotWidget( 0 ), hotTab( 0 ), hotSpot( -1, -1 ), dockCloseActive( 0 ), dockCloseInactive( 0 )
    {
	init();
    }
    ~QWindowsXPStylePrivate()
    {
	cleanup();
    }

    void init( bool force = FALSE )
    {
	if ( ref++ && !force )
	    return;

	use_xp = QWindowsXPStyle::resolveSymbols() && pIsThemeActive() && pIsAppThemed();

	COLORREF cref;
	// Active Title Bar ( Color 1 in the gradient )
	cref = GetSysColor(COLOR_ACTIVECAPTION);
	dockColorActive = qRgb( GetRValue(cref), GetGValue(cref), GetBValue(cref) );
	// 3D Objects
	cref = GetSysColor(COLOR_3DFACE);
	dockColorInactive = qRgb( GetRValue(cref), GetGValue(cref), GetBValue(cref) );

	dockCloseActive = new QPixmap( 10, 10 );
	dockCloseInactive = new QPixmap( 10, 10 );
	dockCloseActive->fill( dockColorActive );
	dockCloseInactive->fill( dockColorInactive );

	QPixmap tmp_ex( (const char **) dockCloseXPM );

	QPainter p1( dockCloseActive );
	QPainter p2( dockCloseInactive );
	tmp_ex.fill( Qt::white );
	p1.drawPixmap( 1, 1, tmp_ex );
	tmp_ex.fill( Qt::black );
	p2.drawPixmap( 1, 1, tmp_ex );
    }

    void cleanup( bool force = FALSE )
    {
	if ( --ref && !force )
	    return;

	use_xp  = FALSE;
	if ( handleMap ) {
	    QMap<QString, HTHEME>::Iterator it;
	    for ( it = handleMap->begin(); it != handleMap->end(); ++it )
		pCloseThemeData( it.value() );
	    delete handleMap;
	    handleMap = 0;
	}
	delete limboWidget;
	delete tabbody;
	limboWidget = 0;
	tabbody = 0;
	delete dockCloseActive;
	delete dockCloseInactive;
	dockCloseActive = dockCloseInactive = 0;
    }

    static bool getThemeResult( HRESULT res )
    {
	if ( res == S_OK )
	    return TRUE;
	return FALSE;
    }

    static HWND winId( const QWidget *widget )
    {
	if ( widget )
	    return widget->winId();

	if ( currentWidget )
	    return currentWidget->winId();

	if ( !limboWidget )
	    limboWidget = new QWidget( 0, "xp_limbo_widget" );

	return limboWidget->winId();
    }

    const QPixmap *tabBody( QWidget *widget );

    // hot-widget stuff

    const QWidget *hotWidget;
    static const QWidget *currentWidget;

    QTab *hotTab;
    QRect hotHeader;

    QPoint hotSpot;
    QRgb groupBoxTextColor;
    QRgb groupBoxTextColorDisabled;
    QRgb tabPaneBorderColor;
    QColor dockColorActive;
    QColor dockColorInactive;
    QPixmap *dockCloseActive;
    QPixmap *dockCloseInactive;

private:
    static QWidget *limboWidget;
    static QPixmap *tabbody;
};

const QWidget *QWindowsXPStylePrivate::currentWidget = 0;
QWidget *QWindowsXPStylePrivate::limboWidget = 0;
QPixmap *QWindowsXPStylePrivate::tabbody = 0;


struct XPThemeData
{
    XPThemeData( const QWidget *w = 0, QPainter *p = 0, const QString &theme = QString::null, int part = 0, int state = 0, const QRect &r = QRect(), QRgb tabBorderColor = 0 )
        : widget( w ), painter( p ), name( theme ),partId( part ), stateId( state ), rec( r ), tbBorderColor( tabBorderColor ), htheme( 0 ), flipped( FALSE ), mirrored(FALSE)
    {
    }
    ~XPThemeData()
    {
    }

    HTHEME handle()
    {
	if ( !use_xp )
	    return NULL;

	if ( !htheme && handleMap )
	    htheme = handleMap->operator[]( name );

        if ( !htheme ) {
            htheme = pOpenThemeData( QWindowsXPStylePrivate::winId( widget ), (TCHAR*)name.ucs2() );
	    if ( htheme ) {
		if ( !handleMap )
		    handleMap = new QMap<QString, HTHEME>;
		handleMap->operator[]( name ) = htheme;
	    }
	}

        return htheme;
    }

    bool isValid()
    {
	return use_xp && !!name && handle();
    }

    RECT rect()
    {
        RECT r;
        r.left = rec.x();
        r.right = rec.x() + rec.width();
        r.top = rec.y();
        r.bottom = rec.y() + rec.height();

        return r;
    }

    HRGN mask()
    {
	if ( pIsThemeBackgroundPartiallyTransparent( handle(), partId, stateId ) ) {
	    HRGN hrgn;
	    pGetThemeBackgroundRegion( handle(), painter ? painter->handle() : 0, partId, stateId, &rect(), &hrgn );
	    return hrgn;
	}
	return 0;
    }

    void setTransparency()
    {
	HRGN hrgn = mask();
	if ( hrgn )
	    SetWindowRgn( QWindowsXPStylePrivate::winId( widget ), hrgn, TRUE );
    }

    void setFlipped( bool b = TRUE )
    {
	flipped = b;
    }

    void setMirrored( bool b = TRUE )
    {
	mirrored = b;
    }

    void drawBackground( int pId = 0, int sId = 0 )
    {
	if ( pId )
	    partId = pId;
	if ( sId )
	    stateId = sId;

	if ( name == "TAB" && (
	    partId == TABP_TABITEMLEFTEDGE ||
	    partId == TABP_TABITEMRIGHTEDGE ||
	    partId == TABP_TABITEM ) ) {
	    QRect oldrec = rec;
	    rec = QRect( 0, 0, rec.width(), rec.height() );
	    QPixmap pm( rec.size() );
	    QPainter p( &pm );
	    p.eraseRect( 0, 0, rec.width(), rec.height() );
	    pDrawThemeBackground( handle(), p.handle(), partId, stateId, &rect(), 0 );
	    rec = oldrec;
	    p.end();
	    if ( flipped ) {
		QWMatrix m;
		m.scale( 1, -1 );
		pm = pm.xForm( m );
	    }
	    painter->drawPixmap( rec.x(), rec.y(), pm );
	    painter->setPen( tbBorderColor );
	    if ( !flipped )
		painter->drawLine( rec.left(), rec.bottom(), rec.right()+1, rec.bottom() );
	} else if ( name == "TREEVIEW" ) {
	    pDrawThemeBackground( handle(), painter->handle(), partId, stateId, &rect(), 0 );
	} else if ( (name == "EDIT" || name == "LISTVIEW") && ::qt_cast<QComboBox*>(widget) == 0 ) {
	    // We assume upto 2px border on the lineedits and Styled Panels,
	    // and clip the contents.
	    QRect rt = rec;
	    rec = painter->xForm( rec );
	    HRGN hr1 = CreateRectRgn( rec.left(), rec.top(), rec.right() + 1, rec.bottom() + 1 );
	    HRGN hr2 = CreateRectRgn( rec.left() + 2, rec.top() + 2, rec.right() - 1, rec.bottom() - 1 );
	    CombineRgn( hr1, hr1, hr2, RGN_DIFF );
	    SelectClipRgn( painter->handle(), hr1 );
	    pDrawThemeBackground( handle(), painter->handle(), partId, stateId, &rect(), 0 );
	    SelectClipRgn( painter->handle(), 0 );
	    DeleteObject( hr1 );
	    DeleteObject( hr2 );
	    rec = rt;
	} else {
	    QRect rt = rec;
	    rec = painter->xForm( rec );
	    if (!mirrored) {
		pDrawThemeBackground( handle(), painter->handle(), partId, stateId, &rect(), 0 );
	    } else {
		QRect oldrec = rec;
		rec = QRect(0, 0, rec.width(), rec.height());
		QPixmap pm(rec.size());
		QPainter p(&pm);
		if(widget)
		    p.setBackground(widget->palette().color(widget->backgroundRole()));
		else
		    //p.setBackgroundColor(qApp->palette().active().background());
		    p.setBackground(qApp->palette().background());
		p.eraseRect( 0, 0, rec.width(), rec.height() );
		pDrawThemeBackground( handle(), p.handle(), partId, stateId, &rect(), 0 );
		p.end();
		rec = oldrec;

		QWMatrix m;
		m.scale( -1, 1 );
		pm = pm.xForm( m );

		painter->drawPixmap(rec.x(), rec.y(), pm);
	    }
	    rec = rt;
	}
    }

    void setName ( const QString &name ) {
	this->name = name;
    }

    int partId;
    int stateId;
    QRect rec;
    QRgb tbBorderColor;

private:
    const QWidget *widget;
    QPainter *painter;
    QString name;
    HTHEME htheme;
    uint workAround :1;
    uint flipped :1;
    uint mirrored :1;
};

const QPixmap *QWindowsXPStylePrivate::tabBody( QWidget *widget )
{
    if ( !tabbody ) {
	tabbody = new QPixmap( 1, 1 );
	QPainter painter( tabbody );
	XPThemeData theme( widget, &painter, "TAB", TABP_BODY, 0 );
	SIZE sz;
	pGetThemePartSize( theme.handle(), painter.handle(), TABP_BODY, 0, 0, TS_TRUE, &sz );

	// Get color for border of tab pane
	COLORREF cref;
	pGetThemeColor( theme.handle(), TABP_PANE, 0, TMT_BORDERCOLORHINT, &cref );
	tabPaneBorderColor = qRgb( GetRValue(cref), GetGValue(cref), GetBValue(cref) );

	painter.end();
	tabbody->resize( sz.cx, QApplication::desktop()->screenGeometry().height() );
	painter.begin( tabbody );
	theme.rec = QRect( 0, 0, sz.cx, sz.cy );
	theme.drawBackground();
	// We fill with the last line of the themedata, that
	// way we don't get a tiled pixmap inside big tabs
	QPixmap temp( sz.cx, 1 );
	bitBlt( &temp, 0,0, tabbody, 0, sz.cy-1 );
	painter.drawTiledPixmap( 0, sz.cy, sz.cx, tabbody->height()-sz.cy, temp );
	painter.end();
    }
    return tabbody;
}


static const int windowsItemFrame		=  2; // menu item frame width
static const int windowsSepHeight		=  7; // separator item height
static const int windowsItemHMargin		=  3; // menu item hor text margin
static const int windowsItemVMargin		=  0; // menu item ver text margin
static const int windowsArrowHMargin		=  6; // arrow horizontal margin
static const int windowsCheckMarkHMargin	=  0; // horiz. margins of check mark
static const int windowsRightBorder		= 12; // right border on windows

/*!
    \class QWindowsXPStyle
    \brief The QWindowsXPStyle class provides a Microsoft WindowsXP-like look and feel.

    \ingroup appearance

    \warning This style is only available on the Windows XP platform
    because it makes use of Windows XP's style engine.
*/

/*!
    Constructs a QWindowsStyle
*/
QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new QWindowsXPStylePrivate;
}

/*! \reimp */
QWindowsXPStyle::~QWindowsXPStyle()
{
    delete d;
}

/*! \reimp */
void QWindowsXPStyle::unPolish( QApplication *app )
{
    QWindowsStyle::unPolish( app );
}

/*! \reimp */
void QWindowsXPStyle::polish( QApplication *app )
{
    QWindowsStyle::polish( app );

    if (!use_xp)
	return;

    // Get text color for groupbox labels
    COLORREF cref;
    XPThemeData theme( 0, 0, "BUTTON", 0, 0 );
    pGetThemeColor( theme.handle(), BP_GROUPBOX, GBS_NORMAL, TMT_TEXTCOLOR, &cref );
    d->groupBoxTextColor = qRgb( GetRValue(cref), GetGValue(cref), GetBValue(cref) );
    pGetThemeColor( theme.handle(), BP_GROUPBOX, GBS_DISABLED, TMT_TEXTCOLOR, &cref );
    d->groupBoxTextColor = qRgb( GetRValue(cref), GetGValue(cref), GetBValue(cref) );
}

/*! \reimp */
void QWindowsXPStyle::polish( QWidget *widget )
{
    QWindowsStyle::polish( widget );
    if ( !use_xp )
	return;
    if ( qt_cast<QButton*>(widget) ) {
	widget->installEventFilter( this );
	//widget-setBackgroundOrigin( QWidget::ParentOrigin );
	if ( qt_cast<QToolButton*>(widget) && !QString::compare( "qt_close_button1", widget->objectName() ) ) {
	    QToolButton *tb = (QToolButton*)widget;
	    tb->setPixmap( *(d->dockCloseActive) );
	    tb->setAutoRaise( TRUE );
	    // ugly hack, please look away
	    tb->setFixedSize( 16, 16 );
	    QDockWindow *dw = static_cast<QDockWindow *>(tb->parent()->parent());
	    if ( dw->area() && dw->area()->orientation() == Horizontal )
		tb->move( 0, 2 );
	    // ok, you can look again
	}
    } else if ( widget->inherits( "QDockWindowHandle" ) ) {
	QWidget *p = (QWidget*)widget->parent();
	if ( !((QDockWindow*)p)->isToolbar ) {
	    QPalette pal = widget->palette();
	    pal.setColor( QPalette::Active, QPalette::Background, d->dockColorActive );
	    pal.setColor( QPalette::Inactive, QPalette::Background, d->dockColorActive );
	    widget->setPalette( pal );
	}
    } else if ( qt_cast<QTabBar*>(widget) ) {
	widget->installEventFilter( this );
	widget->setAutoMask( TRUE );
	widget->setMouseTracking( TRUE );
	connect( widget, SIGNAL(selected(int)), this, SLOT(activeTabChanged()) );
    } else if ( qt_cast<QHeader*>(widget) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( qt_cast<QComboBox*>(widget) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( qt_cast<QSpinWidget*>(widget) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( qt_cast<QScrollBar*>(widget) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( qt_cast<QTitleBar*>(widget) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	widget->installEventFilter( this );
    } else if ( qt_cast<QSlider*>(widget) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( qt_cast<QWidgetStack*>(widget) &&
		qt_cast<QTabWidget*>(widget->parent()) ) {
	QPalette p = widget->palette();
	p.setBrush(widget->backgroundRole(), QBrush(*d->tabBody(widget)));
	widget->setPalette(p);
    }
    QWidget *pW = static_cast<QWidget *>(widget->parent());
    if ( !widget->testAttribute(QWidget::WA_SetPalette) && pW && !pW->palette().brush(pW->backgroundRole()).pixmap() ) {
	//widget->setBackgroundOrigin( QWidget::AncestorOrigin );
	if ( ::qt_cast<QWidgetStack*>(pW) ) {
	    // Repolish all children of a tab page to get
	    // gradient right. ### FIX properly in 4.0!
	    QObjectList objList = widget->queryList( "QWidget" );
	    for ( QObjectList::ConstIterator it = objList.begin(); it != objList.end(); ++it )
		polish((QWidget*)(*it));
	}
    }

    updateRegion( widget );
}

/*! \reimp */
void QWindowsXPStyle::unPolish( QWidget *widget )
{
    // Unpolish of widgets is the first thing that
    // happens when a theme changes, or the theme
    // engine is turned off. So we detect it here.
    bool newState = QWindowsXPStyle::resolveSymbols() && pIsThemeActive() && pIsAppThemed();
    if ( use_xp != newState ) {
	if ( use_xp = newState ) {
	    d->cleanup( TRUE );
	    d->init( TRUE );
	}
    } else if (handleMap) {
    // this is called a couple of times for
    // complex containers, but that doesn't really matter
    // as we get the handles back when we need them.
	QMap<QString, HTHEME>::Iterator it;
	for ( it = handleMap->begin(); it != handleMap->end(); ++it )
	    pCloseThemeData( it.value() );
	delete handleMap;
	handleMap = 0;
    }

    widget->removeEventFilter( this );

    if ( qt_cast<QTitleBar*>(widget) && !widget->inherits( "QDockWindowTitleBar" ) ) {
	SetWindowRgn( widget->winId(), 0, TRUE );
	if ( !QString::compare( widget->objectName(), "_workspacechild_icon_" ) )
	    SetWindowRgn( widget->parentWidget()->winId(), 0, TRUE );
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	SetWindowRgn( widget->winId(), 0, TRUE );
    } else if ( qt_cast<QWidgetStack*>(widget) &&
		qt_cast<QTabWidget*>(widget->parentWidget()) ) {
	widget->setPalette(QPalette());
    } else if ( qt_cast<QTabBar*>(widget) ) {
	disconnect( widget, SIGNAL(selected(int)), this, SLOT(activeTabChanged()) );
    } else if ( widget->inherits( "QDockWindowHandle" ) ||
		qt_cast<QMenuBar*>(widget) ||
		( qt_cast<QToolButton*>(widget) &&
		  !QString::compare( "qt_close_button1", widget->objectName() ) ) ) {
	widget->setPalette(QPalette());
    }

//    if ( !widget->testAttribute(WA_SetPalette) )
//	 widget->setBackgroundOrigin( QWidget::WidgetOrigin );

    QWindowsStyle::unPolish( widget );
}

/*! \reimp */
void QWindowsXPStyle::updateRegion( QWidget *widget )
{
    if ( !use_xp )
	return;

    if ( widget->inherits( "QDockWindowTitleBar" ) ) {
	XPThemeData theme( widget, 0, "WINDOW", WP_SMALLCAPTION, CS_ACTIVE, widget->rect() );
	theme.setTransparency();
    } else if ( qt_cast<QTitleBar*>(widget) && !QString::compare( widget->objectName(), "_workspacechild_icon_" ) ) {
	XPThemeData theme( widget, 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect() );
	theme.setTransparency();
	XPThemeData theme2( widget->parentWidget(), 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect() );
	theme2.setTransparency();
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	if (widget->isMinimized()) {
	    XPThemeData theme( widget, 0, "WINDOW", WP_SMALLCAPTION, CS_ACTIVE, widget->rect() );
	    theme.setTransparency();
	} else {
	    XPThemeData theme( widget, 0, "WINDOW", WP_CAPTION, CS_ACTIVE, widget->rect() );
	    theme.setTransparency();
	}
    }
}

/*! \reimp */
void QWindowsXPStyle::drawPrimitive( PrimitiveElement op,
				    QPainter *p,
				    const QRect &r,
				    const QPalette &pal,
				    SFlags flags,
				    const QStyleOption &opt ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawPrimitive( op, p, r, pal, flags, opt );
	return;
    }

    QString name;
    int partId = 0;
    int stateId = 0;
    QRect rect = r;
    bool mirror = FALSE;

    switch ( op ) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
	name = "BUTTON";
	partId = BP_PUSHBUTTON;
	if ( !(flags & Style_Enabled) )
	    stateId = PBS_DISABLED;
	else if ( flags & Style_Down || flags & Style_Sunken )
	    stateId = PBS_PRESSED;
	else if ( flags & Style_MouseOver )
	    stateId = PBS_HOT;
	else if ( flags & Style_ButtonDefault )
	    stateId = PBS_DEFAULTED;
	else
	    stateId = PBS_NORMAL;

	break;

    case PE_ButtonTool:
	name = "TOOLBAR";
	partId = TP_BUTTON;
	if ( !flags & Style_Enabled )
	    stateId = TS_DISABLED;
	else if ( flags & Style_Down || flags & Style_Sunken )
	    stateId = TS_PRESSED;
	else if ( flags & Style_MouseOver )
	    stateId = flags & Style_On ? TS_HOTCHECKED : TS_HOT;
	else if ( flags & Style_On )
	    stateId = TS_CHECKED;
	else
	    stateId = TS_NORMAL;
	break;

    case PE_ButtonDropDown:
	name = "TOOLBAR";
	partId = TP_SPLITBUTTONDROPDOWN;
	if ( !flags & Style_Enabled )
	    stateId = TS_DISABLED;
	else if ( flags & Style_Down || flags & Style_Sunken )
	    stateId = TS_PRESSED;
	else if ( flags & Style_MouseOver )
	    stateId = flags & Style_On ? TS_HOTCHECKED : TS_HOT;
	else if ( flags & Style_On )
	    stateId = TS_CHECKED;
	else
	    stateId = TS_NORMAL;
	break;

    case PE_Indicator:
	name = "BUTTON";
	partId = BP_CHECKBOX;
	if ( !(flags & Style_Enabled) )
	    stateId = CBS_UNCHECKEDDISABLED;
	else if ( flags & Style_Down )
	    stateId = CBS_UNCHECKEDPRESSED;
	else if ( flags & Style_MouseOver )
	    stateId = CBS_UNCHECKEDHOT;
	else
	    stateId = CBS_UNCHECKEDNORMAL;

	if ( flags & Style_On )
	    stateId += CBS_CHECKEDNORMAL-1;
	else if ( flags & Style_NoChange )
	    stateId += CBS_MIXEDNORMAL-1;

	break;
    case PE_IndicatorMask:
	p->fillRect( r, color1 );
	return;

    case PE_ExclusiveIndicator:
	name = "BUTTON";
	partId = BP_RADIOBUTTON;
	if ( !(flags & Style_Enabled) )
	    stateId = RBS_UNCHECKEDDISABLED;
	else if ( flags & Style_Down )
	    stateId = RBS_UNCHECKEDPRESSED;
	else if ( flags & Style_MouseOver )
	    stateId = RBS_UNCHECKEDHOT;
	else
	    stateId = RBS_UNCHECKEDNORMAL;

	if ( flags & Style_On )
	    stateId += RBS_CHECKEDNORMAL-1;
	break;

    case PE_ExclusiveIndicatorMask:
	p->fillRect( r, color1 );
	return;

    case PE_Splitter:
    case PE_DockWindowResizeHandle:
	return;

    case PE_Panel:
	if ( flags & Style_Raised )
	    return;
	name = "LISTVIEW";
	partId = LVP_LISTGROUP;
	break;

    case PE_PanelLineEdit:
	name = "EDIT";
	partId = EP_EDITTEXT;
	if ( !(flags & Style_Enabled) )
	    stateId = ETS_DISABLED;
	else
	    stateId = ETS_NORMAL;
	break;

    case PE_PanelTabWidget:
	name = "TAB";
	partId = TABP_PANE;
	break;

    case PE_PanelPopup:
	p->save();
	p->setPen( pal.dark() );
	p->drawRect( r );
	p->restore();
	return;

    case PE_PanelMenuBar:
	break;

    case PE_PanelDockWindow:
	name = "REBAR";
	partId = RP_BAND;
	stateId = 1;
	break;

    case PE_HeaderSection:
	name = "HEADER";
	partId = HP_HEADERITEM;
	if ( flags & Style_Down )
	    stateId = HIS_PRESSED;
	else if ( r == d->hotHeader )
	    stateId = HIS_HOT;
	else
	    stateId = HIS_NORMAL;
	break;

    case PE_HeaderArrow:
	{
#if 0 // XP theme engine doesn't know about this :(
	    name = "HEADER";
	    partId = HP_HEADERSORTARROW;
	    if ( flags & Style_Down )
		stateId = HSAS_SORTEDDOWN;
	    else
		stateId = HSAS_SORTEDUP;
#else
	    p->save();
	    p->setPen(pal.dark());
	    p->translate(0, r.height()/2 - 4);
	    if ( flags & Style_Up ) { // invert logic to follow Windows style guide
		p->drawLine(r.x(), r.y(), r.x()+8, r.y());
		p->drawLine(r.x()+1, r.y()+1, r.x()+7, r.y()+1);
		p->drawLine(r.x()+2, r.y()+2, r.x()+6, r.y()+2);
		p->drawLine(r.x()+3, r.y()+3, r.x()+5, r.y()+3);
		p->drawPoint(r.x()+4, r.y()+4);
	    } else {
		p->drawLine(r.x(), r.y()+4, r.x()+8, r.y()+4);
		p->drawLine(r.x()+1, r.y()+3, r.x()+7, r.y()+3);
		p->drawLine(r.x()+2, r.y()+2, r.x()+6, r.y()+2);
		p->drawLine(r.x()+3, r.y()+1, r.x()+5, r.y()+1);
		p->drawPoint(r.x()+4, r.y());
	    }
	    p->restore();
	    return;
#endif
	}
	break;

    case PE_StatusBarSection:
	name = "STATUS";
	partId = SP_PANE;
	break;

    case PE_PanelGroupBox:
	name = "BUTTON";
	partId = BP_GROUPBOX;
	if ( !(flags & Style_Enabled) )
	    stateId = GBS_DISABLED;
	else
	    stateId = GBS_NORMAL;
	break;

    case PE_SizeGrip:
	name = "STATUS";
	partId = SP_GRIPPER;
	// empiric correction values...
	rect.addCoords( -4, -8, 0, 0 );
	mirror = qApp->reverseLayout();
	break;

    case PE_ScrollBarAddLine:
	name = "SCROLLBAR";
	break;

    case PE_ScrollBarSubLine:
	name = "SCROLLBAR";
	break;

    case PE_ScrollBarAddPage:
	name = "SCROLLBAR";
	break;

    case PE_ScrollBarSubPage:
	name = "SCROLLBAR";
	break;

    case PE_ScrollBarSlider:
	name = "SCROLLBAR";
	break;

    case PE_ScrollBarFirst:
	name = "SCROLLBAR";
	break;

    case PE_ScrollBarLast:
	name = "SCROLLBAR";
	break;

    case PE_ProgressBarChunk:
	name = "PROGRESS";
	partId = PP_CHUNK;
	stateId = 1;
	rect = QRect( r.x(), r.y() + 3, r.width(), r.height() - 5 );
	break;

    case PE_DockWindowHandle:
	{
	    QString title;
	    bool drawDockTitle = FALSE;
	    bool isDockWindow = FALSE;
	    QWidget *w = 0;
	    if ( p && p->device()->devType() == QInternal::Widget ) {
		w = (QWidget *) p->device();
		QWidget *p = w->parentWidget();
		if ( qt_cast<QDockWindow*>(p) && !qt_cast<QToolBar*>(p) ) {
		    int drawArea = qMin( r.right() - r.left(), r.bottom() - r.top() );
		    drawDockTitle = ( drawArea >= w->fontMetrics().height() );
		    isDockWindow = TRUE;
		    title = p->windowTitle();
		}
	    }

	    if ( !isDockWindow ) {
		name = "REBAR";
		if ( flags & Style_Horizontal )
		    partId = RP_GRIPPER;
		else
		    partId = RP_GRIPPERVERT;
		break;
	    }

	    // Dock window...
	    name = "WINDOW";
	    partId = WP_MAXCAPTION;
	    if ( !(flags & Style_Enabled) )
		stateId = GBS_DISABLED;
	    else
		stateId = GBS_NORMAL;

	    if ( drawDockTitle ) {
		QRect rt = r;
		if ( w )
		    p->setPen( pal.color(w->isActiveWindow() ? QPalette::Active :
					 QPalette::Inactive, QPalette::HighlightedText ));

		if ( flags & Style_Horizontal ) {
		    // Vertical Title  ( Horizontal DockWindow )
		    rt.addCoords( 2, 4, -1, -4 );
		    p->rotate( 270.0 );
		    p->translate( -(rt.height()+rt.y()), (rt.width()-rt.x()) );
		    p->drawText( 0,0, title );

		} else {
		    // Horizontal Title
		    rt.addCoords( 4, 1, -4, 1 );
		    p->drawText( rt, AlignLeft, title );
		}

		// Change close button to match new title
		if ( w ) {
		    QToolButton *tb = (QToolButton*)w->child( "qt_close_button1", "QToolButton", FALSE );
		    if ( !tb )
			return;
		    if ( w->isActiveWindow() ) {
			QPalette pl = tb->palette();
			pl.setColor( QPalette::Button, d->dockColorActive );
			tb->setPalette( pl );
			tb->setPixmap( *(d->dockCloseActive) );
		    } else {
			QPalette pl = tb->palette();
			pl.setColor( QPalette::Button, d->dockColorInactive );
			tb->setPalette( pl );
			tb->setPixmap( *(d->dockCloseInactive) );
		    }
		}
	    }
	    return;
	}

    case PE_DockWindowSeparator:
	name = "TOOLBAR";
	if ( flags & Style_Horizontal )
	    partId = TP_SEPARATOR;
	else
	    partId = TP_SEPARATORVERT;
	break;

    case PE_WindowFrame:
	{
	    name = "WINDOW";
	    if ( flags & Style_Active )
		stateId = FS_ACTIVE;
	    else
		stateId = FS_INACTIVE;

	    int fwidth = pixelMetric( PM_MDIFrameWidth );

	    if ( !opt.isDefault() )
		fwidth = opt.lineWidth() + opt.midLineWidth();

	    XPThemeData theme( 0, p, name, 0, stateId );
	    if ( !theme.isValid() )
		break;

	    theme.rec = QRect( r.x(), r.y()+fwidth, r.x()+fwidth, r.height()-fwidth );
	    theme.partId = WP_FRAMELEFT;
	    theme.drawBackground();
	    theme.rec = QRect( r.width()-fwidth, r.y()+fwidth, fwidth, r.height()-fwidth );
	    theme.partId = WP_FRAMERIGHT;
	    theme.drawBackground();
	    theme.rec = QRect( r.x(), r.height()-fwidth, r.width(), fwidth );
	    theme.partId = WP_FRAMEBOTTOM;
	    theme.drawBackground();
	    theme.rec = QRect( r.x()-5, r.y()-5, r.width()+10, r.y()+fwidth+5 );
	    theme.partId = WP_CAPTION;
	    theme.drawBackground();

	    return;
	}

    case PE_TreeBranch: {
	static const int decoration_size = 9;
	int mid_h = r.width() / 2;
	int mid_v = r.height() / 2;
  	int bef_h = mid_h;
  	int bef_v = mid_v;
 	int aft_h = mid_h;
 	int aft_v = mid_v;
	if (flags & QStyle::Style_Children) {
	    int delta = decoration_size / 2;
	    bef_h -= delta;
	    bef_v -= delta;
	    aft_h += delta;
	    aft_v += delta;
	    XPThemeData theme(0, p, "TREEVIEW");
	    theme.rec = QRect(bef_h, bef_v, decoration_size, decoration_size);
	    theme.drawBackground(TVP_GLYPH, flags & QStyle::Style_Open ? GLPS_OPENED : GLPS_CLOSED);
	}
	// ### BUG: the dotted lines don't follow a the y coordinates (causes drawing errors)
	// ### also see qwindowsstyle
	QBrush brush(pal.dark(), Qt::Dense4Pattern);
// 	QPoint org(p->xForm(QPoint(0, 0)));
// 	p->setBrushOrigin(org);
	if (flags & QStyle::Style_Item)
	    p->fillRect(aft_h, mid_v, r.right() - aft_h + 1, 1, brush);
	if (flags & QStyle::Style_Sibling)
	    p->fillRect(mid_h, aft_v, 1, r.bottom() - aft_v + 1, brush);
	if (flags & (QStyle::Style_Open|QStyle::Style_Children|QStyle::Style_Item|QStyle::Style_Sibling))
	    p->fillRect(mid_h, r.y(), 1, bef_v - r.y(), brush);
	break; }


    default:
	break;
    }

    XPThemeData theme( 0, p, name, partId, stateId, rect );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawPrimitive( op, p, r, pal, flags, opt );
	return;
    }
    theme.setMirrored(mirror);
    theme.drawBackground();
}

/*! \reimp */
void QWindowsXPStyle::drawControl( ControlElement element,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  const QPalette &pal,
				  SFlags flags,
				  const QStyleOption &opt ) const
{
    bool doFlipp = FALSE;
    d->currentWidget = widget;

    if ( !use_xp ) {
	QWindowsStyle::drawControl( element, p, widget, r, pal, flags, opt );
	return;
    }

    QRect rect(r);
    QString name;
    int partId = 0;
    int stateId = 0;
    if ( widget->testAttribute(QWidget::WA_UnderMouse) && widget->isActiveWindow() )
	flags |= Style_MouseOver;

    switch ( element ) {
    case CE_PushButton:
	//    case CE_PushButtonLabel:
	{
	    name = "BUTTON";
	    partId = BP_PUSHBUTTON;
	    QPushButton *pb = (QPushButton*)widget;
	    if ( !(flags & Style_Enabled) && !pb->isFlat() )
		stateId = PBS_DISABLED;
	    else if ( pb->isFlat() && !(flags & Style_Down) )
		return;
	    else if ( flags & Style_Down || flags & Style_Sunken || pb->isOn() )
		stateId = PBS_PRESSED;
	    else if ( flags & Style_MouseOver )
		stateId = PBS_HOT;
	    else if ( flags & Style_ButtonDefault )
		stateId = PBS_DEFAULTED;
	    else
		stateId = PBS_NORMAL;
	}
	break;

    case CE_TabBarTab:
	//    case CE_TabBarLabel:
	name = "TAB";
	{
	    QTabBar *bar = (QTabBar*)widget;
	    QTab *t = opt.tab();
	    if ( (bar->shape() == QTabBar::RoundedBelow) ||
		 (bar->shape() == QTabBar::TriangularBelow) )
		 doFlipp = TRUE;

	    int idx = bar->indexOf( t->identifier() );
	    int aidx = bar->indexOf( bar->currentTab() );
	    int lastTab = bar->count()-1;
	    if ( idx == 0 )
		partId = TABP_TABITEMLEFTEDGE;
	    else if ( idx == lastTab )
		partId = TABP_TABITEM;
	    else
		partId = TABP_TABITEM;

	    if ( !(flags & Style_Enabled) )
		stateId = TIS_DISABLED;
	    else if ( flags & Style_HasFocus )
		stateId = TIS_FOCUSED;
	    else if ( flags & Style_Selected )
		stateId = TIS_SELECTED;
	    else if ( t && d->hotTab == t )
		stateId = TIS_HOT;
	    else
		stateId = TIS_NORMAL;
	    if ( doFlipp )
		if ( (flags & Style_Selected) || (flags & Style_HasFocus) ) {
		    rect.addCoords( 0, 0, 0, 0 );
		} else {
		    rect.addCoords( 0, 0, 0, -2 );
		    if ( idx != aidx+1 )
			rect.addCoords( 1, 0, 0, 0 );
		    if ( idx != aidx-1 )
			rect.addCoords( 0, 0, -1, 0 );
		}
	    else {
		if ( (flags & Style_Selected) || (flags & Style_HasFocus) ) {
		    rect.addCoords( 0, 0, 0, 1 );
		} else {
		    rect.addCoords( 0, 2, 0, 0 );
		    if ( idx != aidx+1 )
			rect.addCoords( 1, 0, 0, 0 );
		    if ( idx != aidx-1 )
			rect.addCoords( 0, 0, -1, 0 );
		}
	    }
	}
	break;

    case CE_ProgressBarGroove:
	name = "PROGRESS";
	partId = PP_BAR;
	stateId = 1;
	break;

#ifdef QT_COMPAT
#ifndef QT_NO_POPUPMENU
    case CE_Q3PopupMenuItem:
	{
	    if (! widget || opt.isDefault())
		break;

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    QMenuItem *mi = opt.menuItem();
	    if ( !mi )
		break;

	    int tab = opt.tabWidth();
	    int maxpmw = opt.maxIconWidth();
	    bool dis = !(flags&Style_Enabled);
	    bool checkable = popupmenu->isCheckable();
	    bool act = flags & Style_Active;
	    int x, y, w, h;

	    r.rect(&x, &y, &w, &h);

	    if ( checkable ) {
		// space for the checkmarks
		maxpmw = qMax( maxpmw, 20 );
	    }

	    int checkcol = maxpmw;

	    if ( mi && mi->isSeparator() ) {                    // draw separator
		p->setPen( pal.dark() );
		p->drawLine( x, y + h/2, x+w, y + h/2 );
		p->setPen( pal.light() );
		p->drawLine( x, y+1 + h/2, x+w, y+1 + h/2 );
		return;
	    }

	    QBrush fill = (act ?
			   pal.highlight() :
			   pal.button());
	    p->fillRect( x, y, w, h, fill);

	    if ( !mi )
		return;

	    int xpos = x;
	    QRect vrect = visualRect( QRect( xpos, y, checkcol, h ), r );
	    int xvis = vrect.x();
	    if ( mi->isChecked() ) {
		if ( act && !dis )
		    qDrawShadePanel( p, xvis, y, checkcol, h,
				     pal, TRUE, 1, &pal.button() );
		else {
		    QBrush fill( pal.light(), Dense4Pattern );
		    // set the brush origin for the hash pattern to the x/y coordinate
		    // of the menu item's checkmark... this way, the check marks have
		    // a consistent look
		    QPoint origin = p->brushOrigin();
		    p->setBrushOrigin( xvis, y );
		    qDrawShadePanel( p, xvis, y, checkcol, h, pal, TRUE, 1,
				     &fill );
		    // restore the previous brush origin
		    p->setBrushOrigin( origin );
		}
	    } else if (! act)
		p->fillRect(xvis, y, checkcol , h, pal.brush( QColorGroup::Button ));

	    if ( mi->iconSet() ) {              // draw iconset
		QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
		if (act && !dis )
		    mode = QIconSet::Active;
		QPixmap pixmap;
		if ( checkable && mi->isChecked() )
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
		else
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		if ( act && !dis && !mi->isChecked() )
		    qDrawShadePanel( p, xvis, y, checkcol, h, pal, FALSE, 1,
				     &pal.button() );
		QRect pmr( 0, 0, pixw, pixh );
		pmr.moveCenter( vrect.center() );
		p->setPen( pal.text() );
		p->drawPixmap( pmr.topLeft(), pixmap );

		fill = (act ?
			pal.highlight() :
			pal.button());
		int xp = xpos + checkcol + 1;
		p->fillRect( visualRect( QRect( xp, y, w - checkcol - 1, h ), r ), fill);
	    } else  if ( checkable ) {  // just "checking"...
		if ( mi->isChecked() ) {
		    int xp = xpos + windowsItemFrame;

		    SFlags cflags = Style_Default;
		    if (! dis)
			cflags |= Style_Enabled;
		    if (act)
			cflags |= Style_On;

		    drawPrimitive(PE_CheckMark, p,
				  visualRect( QRect(xp, y + windowsItemFrame,
					checkcol - 2*windowsItemFrame,
					h - 2*windowsItemFrame), r ), pal, cflags);
		}
	    }

	    p->setPen( act ? pal.highlightedText() : pal.buttonText() );

	    QColor discol;
	    if ( dis ) {
		discol = pal.text();
		p->setPen( discol );
	    }

	    int xm = windowsItemFrame + checkcol + windowsItemHMargin;
	    xpos += xm;

	    vrect = visualRect( QRect( xpos, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin ), r );
	    xvis = vrect.x();
	    if ( mi->custom() ) {
		p->save();
		if ( dis && !act ) {
		    p->setPen( pal.light() );
		    mi->custom()->paint( p, pal, act, !dis,
					 xvis+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin );
		    p->setPen( discol );
		}
		mi->custom()->paint( p, pal, act, !dis,
				     xvis, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin );
		p->restore();
	    }
	    QString s = mi->text();
	    if ( !s.isNull() ) {                        // draw text
		int t = s.indexOf( '\t' );
		int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
		if (!styleHint(SH_UnderlineAccelerator, widget))
		    text_flags |= NoAccel;
		text_flags |= (QApplication::reverseLayout() ? AlignRight : AlignLeft );
		if ( t >= 0 ) {                         // draw tab text
		    int xp = x + w - tab - windowsItemHMargin - windowsItemFrame + 1;
		    xp -= 20;
		    int xoff = visualRect( QRect( xp, y+windowsItemVMargin, tab, h-2*windowsItemVMargin ), r ).x();
		    if ( dis && !act ) {
			p->setPen( pal.light() );
			p->drawText( xoff+1, y+windowsItemVMargin+1, tab, h-2*windowsItemVMargin, text_flags, s.mid( t+1 ));
			p->setPen( discol );
		    }
		    p->drawText( xoff, y+windowsItemVMargin, tab, h-2*windowsItemVMargin, text_flags, s.mid( t+1 ) );
		    s = s.left( t );
		}
		if ( dis && !act ) {
		    p->setPen( pal.light() );
		    p->drawText( xvis+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin, text_flags, s, t );
		    p->setPen( discol );
		}
		p->drawText( xvis, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin, text_flags, s, t );
	    } else if ( mi->pixmap() ) {                        // draw pixmap
		QPixmap *pixmap = mi->pixmap();
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( OpaqueMode );
		p->drawPixmap( xvis, y+windowsItemFrame, *pixmap );
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( TransparentMode );
	    }
	    if ( mi->popup() ) {                        // draw sub menu arrow
		int dim = (h-2*windowsItemFrame) / 2;
		PrimitiveElement arrow;
		arrow = ( QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight );
		xpos = x+w - windowsArrowHMargin - windowsItemFrame - dim;
		vrect = visualRect( QRect(xpos, y + h / 2 - dim / 2, dim, dim), r );
		if ( act ) {
		    QPalette pal2 = pal;
		    pal2.setColor( QPalette::ButtonText, pal2.highlightedText() );
		    drawPrimitive(arrow, p, vrect,
				  pal2, dis ? Style_Default : Style_Enabled);
		} else {
		    drawPrimitive(arrow, p, vrect,
				  pal, dis ? Style_Default : Style_Enabled );
		}
	    }

	    break;
	}
#endif
#endif

    case CE_MenuBarItem:
	{
	    if (opt.isDefault())
		break;

	    if (flags & Style_Active)
		p->fillRect(r, pal.brush( QPalette::Highlight) );
	    else
		p->fillRect(r, pal.brush( QPalette::Button) );

	    QAction *mi = opt.action();
	    QPixmap pix = mi->icon().pixmap(QIconSet::Small, QIconSet::Normal);
	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, pal,
		     flags & Style_Enabled, pix.isNull() ? 0 : &pix, mi->text(), -1,
		     flags & Style_Active ? &pal.highlightedText().color() : &pal.buttonText().color());
	}
	return;

#ifdef QT_COMPAT
    case CE_Q3MenuBarItem:
	{
	    if (opt.isDefault())
		break;

	    QMenuItem *mi = opt.menuItem();
	    if (flags & Style_Active)
		p->fillRect(r, pal.brush( QPalette::Highlight) );
	    else
		p->fillRect(r, pal.brush( QPalette::Button) );

	    drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, pal,
		     flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		     flags & Style_Active ? &pal.highlightedText().color() : &pal.buttonText().color());
	}
	return;
#endif

    case CE_ToolButtonLabel:
	if ( !opt.isDefault() )
	    return;
	break;

    default:
	break;
    }

    XPThemeData theme( widget, p, name, partId, stateId, rect, d->tabPaneBorderColor );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawControl( element, p, widget, rect, pal, flags, opt );
	return;
    }

    if ( doFlipp )
	theme.setFlipped();
    theme.drawBackground();

    d->currentWidget = 0;
}

/*! \reimp */
void QWindowsXPStyle::drawControlMask( ControlElement element,
			  QPainter *p,
			  const QWidget *widget,
			  const QRect &r,
			  const QStyleOption &option ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawControlMask( element, p, widget, r, option );
	return;
    }

    QString name;
    int partId = 0;
    int stateId = 0;
    switch ( element ) {
    case CE_PushButton:
	//    case CE_PushButtonLabel:
	name = "BUTTON";
	partId = BP_PUSHBUTTON;
	break;

    case CE_RadioButton:
	name = "BUTTON";
	partId = BP_RADIOBUTTON;
	break;

    case CE_CheckBox:
	name = "BUTTON";
	partId = BP_CHECKBOX;
	break;

    default:
	break;
    }

    QRect rect = r;
    rect.addCoords( 0, 0, 1, 1 );
    XPThemeData theme( widget, p, name, partId, stateId, rect );
    HRGN rgn = theme.mask();

    if ( !rgn ) {
	QWindowsStyle::drawControlMask( element, p, widget, r, option );
	return;
    }

    p->save();
    p->setBrush( color1 );
    PaintRgn( p->handle(), rgn );
    p->restore();
}

static int qPositionFromValue( const QRangeControl * rc, int logical_val,
			       int span )
{
    if ( span <= 0 || logical_val < rc->minValue() ||
	 rc->maxValue() <= rc->minValue() )
	return 0;
    if ( logical_val > rc->maxValue() )
	return span;

    uint range = rc->maxValue() - rc->minValue();
    uint p = logical_val - rc->minValue();

    if ( range > (uint)INT_MAX/4096 ) {
	const int scale = 4096*2;
	return ( (p/scale) * span ) / (range/scale);
	// ### the above line is probably not 100% correct
	// ### but fixing it isn't worth the extreme pain...
    } else if ( range > (uint)span ) {
	return (2*p*span + range) / (2*range);
    } else {
	uint div = span / range;
	uint mod = span % range;
	return p*div + (2*p*mod + range) / (2*range);
    }
    //equiv. to (p*span)/range + 0.5
    // no overflow because of this implicit assumption:
    // span <= 4096
}

/*! \reimp */
void QWindowsXPStyle::drawComplexControl( ComplexControl control,
					 QPainter* p,
					 const QWidget* w,
					 const QRect& r,
					 const QPalette& pal,
					 SFlags flags,
					 SCFlags sub,
					 SCFlags subActive,
					 const QStyleOption &opt ) const
{
    d->currentWidget = w;

    if ( !use_xp ) {
	QWindowsStyle::drawComplexControl( control, p, w, r, pal, flags, sub, subActive, opt );
	return;
    }

    int partId = 0;
    int stateId = 0;
    if ( w->testAttribute(QWidget::WA_UnderMouse) && w->isActiveWindow() )
	flags |= Style_MouseOver;

    switch ( control ) {
    case CC_SpinWidget:
        {
	    QSpinWidget *spin = (QSpinWidget*)w;
	    XPThemeData theme( w, p, "SPIN" );

	    if ( sub & SC_SpinWidgetFrame ) {
		partId = EP_EDITTEXT;
		if ( (!flags & Style_Enabled) )
		    stateId = ETS_DISABLED;
		else if ( flags & Style_HasFocus )
		    stateId = ETS_FOCUSED;
		else
		    stateId = ETS_NORMAL;

		XPThemeData ftheme( w, p, "EDIT", partId, stateId, r );
		ftheme.drawBackground();
	    }
	    if ( sub & SC_SpinWidgetUp ) {
		theme.rec = visualRect(querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetUp, opt ),w);
		partId = SPNP_UP;
		if ( !spin->isUpEnabled() )
		    stateId = UPS_DISABLED;
		else if ( subActive == SC_SpinWidgetUp )
		    stateId = UPS_PRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = UPS_HOT;
		else
		    stateId = UPS_NORMAL;
		theme.drawBackground( partId, stateId );
	    }
	    if ( sub & SC_SpinWidgetDown ) {
		theme.rec = visualRect(querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetDown, opt ),w);
		partId = SPNP_DOWN;
		if ( !spin->isDownEnabled() )
		    stateId = DNS_DISABLED;
		else if ( subActive == SC_SpinWidgetDown )
		    stateId = DNS_PRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = DNS_HOT;
		else
		    stateId = DNS_NORMAL;

		theme.drawBackground( partId, stateId );
	    }
        }
        break;

    case CC_ComboBox:
        {
	    if ( sub & SC_ComboBoxEditField ) {
		partId = EP_EDITTEXT;
		if ( !(flags & Style_Enabled ) )
		    stateId = ETS_DISABLED;
		else if ( flags & Style_HasFocus )
		    stateId = ETS_FOCUSED;
		else
		    stateId = ETS_NORMAL;
		XPThemeData theme( w, p, "EDIT", partId, stateId, r );

		theme.drawBackground();
		if ( !((QComboBox*)w)->editable() ) {
	    	    QRect re = visualRect(querySubControlMetrics(CC_ComboBox, w, SC_ComboBoxEditField, opt), w);
		    if ( w->hasFocus() ) {
			p->fillRect(re, pal.brush( QPalette::Highlight) );
			p->setPen( pal.highlightedText() );
			p->setBackground( pal.highlight() );
		    } else {
			p->fillRect(re, pal.brush( QPalette::Base ) );
			p->setPen( pal.text() );
			p->setBackground( pal.base() );
		    }
		}
	    }

	    if ( sub & SC_ComboBoxArrow ) {
		XPThemeData theme( w, p, "COMBOBOX" );
		theme.rec = visualRect(querySubControlMetrics(CC_ComboBox, w, SC_ComboBoxArrow, opt), w);
		partId = CP_DROPDOWNBUTTON;

		if ( !(flags & Style_Enabled) )
		    stateId = CBXS_DISABLED;
		else if ( subActive == SC_ComboBoxArrow )
                    stateId = CBXS_PRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = CBXS_HOT;
		else
		    stateId = CBXS_NORMAL;

		theme.drawBackground( partId, stateId );
	    }
        }
        break;

    case CC_ScrollBar:
        {
	    XPThemeData theme( w, p, "SCROLLBAR" );
	    QScrollBar *bar = (QScrollBar*)w;
	    bool maxedOut = ( bar->maximum() == bar->minimum() );
	    if ( maxedOut )
		flags &= ~Style_Enabled;

	    if ( sub & SC_ScrollBarAddLine ) {
		theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarAddLine, opt );
		partId = SBP_ARROWBTN;
		if ( !( flags & Style_Enabled ) )
		    stateId = ABS_DOWNDISABLED;
		else if ( subActive == SC_ScrollBarAddLine )
		    stateId = ABS_DOWNPRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = ABS_DOWNHOT;
		else
		    stateId = ABS_DOWNNORMAL;
		if ( flags & Style_Horizontal )
		    stateId += 8;

		theme.drawBackground( partId, stateId );
	    }
	    if ( sub & SC_ScrollBarSubLine ) {
		theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSubLine, opt );
		partId = SBP_ARROWBTN;
		if ( !( flags & Style_Enabled ) )
		    stateId = ABS_UPDISABLED;
		else if ( subActive == SC_ScrollBarSubLine )
		    stateId = ABS_UPPRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = ABS_UPHOT;
		else
		    stateId = ABS_UPNORMAL;
		if ( flags & Style_Horizontal )
		    stateId += 8;

		theme.drawBackground( partId, stateId );
	    }
	    if ( maxedOut ) {
		theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSlider, opt );
		theme.rec = theme.rec.unite( querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSubPage, opt ) );
		theme.rec = theme.rec.unite( querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarAddPage, opt ) );
		partId = bar->orientation() == Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
		stateId = SCRBS_DISABLED;

		theme.drawBackground( partId, stateId );
	    } else {
		if ( sub & SC_ScrollBarAddPage ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarAddPage, opt );
		    partId = flags & Style_Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
		    if ( !(flags & Style_Enabled) )
			stateId = SCRBS_DISABLED;
		    else if ( subActive == SC_ScrollBarAddPage )
			stateId = SCRBS_PRESSED;
		    else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
			stateId = SCRBS_HOT;
		    else
			stateId = SCRBS_NORMAL;

		    theme.drawBackground( partId, stateId );
		}
		if ( sub & SC_ScrollBarSubPage ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSubPage, opt );
		    partId = flags & Style_Horizontal ? SBP_UPPERTRACKHORZ : SBP_UPPERTRACKVERT;
		    if ( !(flags & Style_Enabled ) )
			stateId = SCRBS_DISABLED;
		    else if ( subActive == SC_ScrollBarSubPage )
			stateId = SCRBS_PRESSED;
		    else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
			stateId = SCRBS_HOT;
		    else
			stateId = SCRBS_NORMAL;

		    theme.drawBackground( partId, stateId );
		}
		if ( sub & SC_ScrollBarFirst ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarFirst, opt );
		}
		if ( sub & SC_ScrollBarLast ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarLast, opt );
		}
		if ( sub & SC_ScrollBarSlider ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSlider, opt );
		    if ( !(flags & Style_Enabled ) )
			stateId = SCRBS_DISABLED;
		    else if ( subActive == SC_ScrollBarSlider )
			stateId = SCRBS_PRESSED;
		    else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
			stateId = SCRBS_HOT;
		    else
			stateId = SCRBS_NORMAL;

		    const int swidth = theme.rec.width();
		    const int sheight = theme.rec.height();

		    theme.drawBackground( flags & Style_Horizontal ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT, stateId );

		    // paint gripper if there is enough space
		    SIZE size;
		    pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		    if ( sheight > size.cy ) {
			QRect gr;
			if ( flags & Style_Horizontal ) {
			    gr.setLeft( theme.rec.left() + swidth/2 - 5 );
			    gr.setRight( gr.left() + 10 );
			    gr.setTop( theme.rec.top() + sheight/2 - 3 );
			    gr.setBottom( gr.top() + 6 );
			} else {
			    gr.setLeft( theme.rec.left() + swidth/2 - 3 );
			    gr.setRight( gr.left() + 6 );
			    gr.setTop( theme.rec.top() + sheight/2 - 5 );
			    gr.setBottom( gr.top() + 10 );
			}

			theme.rec = gr;
			theme.drawBackground( flags & Style_Horizontal ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT, 1 );
		    }
		}
		if ( sub & SC_ScrollBarGroove ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarGroove );
		}
	    }
        }
        break;
#ifndef QT_NO_SLIDER
    case CC_Slider:
	{
	    XPThemeData theme( w, p, "TRACKBAR" );
	    QSlider *sl = (QSlider*)w;
	    QRegion tickreg = sl->rect();
	    if ( sub & SC_SliderGroove ) {
		theme.rec = querySubControlMetrics( CC_Slider, w, SC_SliderGroove, opt );
		if ( sl->orientation() == Horizontal ) {
		    partId = TKP_TRACK;
		    stateId = TRS_NORMAL;
		    theme.rec = QRect( 0, theme.rec.center().y() - 2, sl->width(), 4 );
		} else {
		    partId = TKP_TRACKVERT;
		    stateId = TRVS_NORMAL;
		    theme.rec = QRect( theme.rec.center().x() - 2, 0, 4, sl->height() );
		}
		theme.drawBackground( partId, stateId );
		tickreg -= theme.rec;
	    }
	    p->setClipRegion( tickreg );
	    if ( sub & SC_SliderTickmarks ) {
		QWindowsStyle::drawComplexControl( control, p, w, r, pal, flags, SC_SliderTickmarks, subActive, opt );
		// Reenable XP style tickmarks when the
		// styles actually have usable pixmaps!
		/*
		int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
		int ticks = sl->tickmarks();
		int thickness = pixelMetric( PM_SliderControlThickness, sl );
		int len = pixelMetric( PM_SliderLength, sl );
		int available = pixelMetric( PM_SliderSpaceAvailable, sl );
		int interval = sl->tickInterval();

		if ( interval <= 0 ) {
		    interval = sl->lineStep();
		    if ( qPositionFromValue( sl, interval, available ) -
			 qPositionFromValue( sl, 0, available ) < 3 )
			interval = sl->pageStep();
		}

		int fudge = len / 2;
		int pos;

		if ( !interval )
		    interval = 1;
		int v = sl->minValue();

		const int aboveend = tickOffset-2;
		const int belowstart = tickOffset+thickness+1;
		const int belowend = belowstart+available-2;

		if ( sl->orientation() == Horizontal ) {
		    if ( ticks & QSlider::Above )
			p->fillRect( 0, 0, sl->width(), aboveend, pal.brush( QPalette::Background ) );
		    if ( ticks & QSlider::Below )
			p->fillRect( 0, belowstart, sl->width(), belowend, pal.brush( QPalette::Background ) );

		    partId = TKP_TICS;
		    stateId = TSS_NORMAL;
		    while ( v <= sl->maxValue() + 1 ) {
			pos = qPositionFromValue( sl, v, available ) + fudge;
			if ( ticks & QSlider::Above ) {
			    theme.rec.setCoords( pos, 0, pos, aboveend );
			    theme.drawBackground( partId, stateId );
			}
			if ( ticks & QSlider::Below ) {
			    theme.rec.setCoords( pos, belowstart, pos, belowend );
			    theme.drawBackground( partId, stateId );
			}

			v += interval;
		    }
		} else {
		    if ( ticks & QSlider::Left )
			p->fillRect( 0, 0, aboveend, sl->height(), pal.brush( QPalette::Background ) );
		    if ( ticks & QSlider::Right )
			p->fillRect( belowstart, 0, belowend, sl->height(), pal.brush( QPalette::Background ) );

		    partId = TKP_TICSVERT;
		    stateId = TSVS_NORMAL;
		    while ( v <= sl->maxValue() + 1 ) {
			pos = qPositionFromValue( sl, v, available ) + fudge;
			if ( ticks & QSlider::Left ) {
			    theme.rec.setCoords( 0, pos, aboveend, pos );
			    theme.drawBackground( partId, stateId );
			}
			if ( ticks & QSlider::Right ) {
			    theme.rec.setCoords( belowstart, pos, belowend, pos );
			    theme.drawBackground( partId, stateId );
			}
			v += interval;
		    }
		}
		*/
	    }
	    p->setClipping( FALSE );
	    if ( sub & SC_SliderHandle ) {
		theme.rec = querySubControlMetrics( CC_Slider, w, SC_SliderHandle, opt );
		p->fillRect( theme.rec, pal.brush( QPalette::Background ) );
		if ( sl->orientation() == Horizontal ) {
		    if ( sl->tickmarks() == QSlider::Above )
			partId = TKP_THUMBTOP;
		    else if ( sl->tickmarks() == QSlider::Below )
			partId = TKP_THUMBBOTTOM;
		    else
			partId = TKP_THUMB;

		    if ( !w->isEnabled() )
			stateId = TUS_DISABLED;
		    else if ( subActive == SC_SliderHandle )
			stateId = TUS_PRESSED;
		    else if ( flags & Style_HasFocus )
			stateId = TUS_FOCUSED;
		    else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
			stateId = TUS_HOT;
		    else
			stateId = TUS_NORMAL;
		} else {
		    if ( sl->tickmarks() == QSlider::Left )
			partId = TKP_THUMBLEFT;
		    else if ( sl->tickmarks() == QSlider::Right )
			partId = TKP_THUMBRIGHT;
		    else
			partId = TKP_THUMBVERT;

		    if ( !w->isEnabled() )
			stateId = TUVS_DISABLED;
		    else if ( subActive == SC_SliderHandle )
			stateId = TUVS_PRESSED;
		    else if ( flags & Style_HasFocus )
			stateId = TUVS_FOCUSED;
		    else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
			stateId = TUS_HOT;
		    else
			stateId = TUVS_NORMAL;
		}
		theme.drawBackground( partId, stateId );
	    }
	    if ( flags & Style_HasFocus ) {
		QRect re = subRect( SR_SliderFocusRect, sl );
		drawPrimitive( PE_FocusRect, p, re, pal );
	    }
	}
	break;
#endif

    case CC_ToolButton:
	{
	    XPThemeData theme( w, p, "TOOLBAR" );
	    QToolButton *tb = (QToolButton*)w;

	    SFlags bflags = flags,
		   mflags = flags;

	    if (subActive == SC_ToolButton)
		bflags |= Style_Down;
	    else if (subActive == SC_ToolButtonMenu)
		mflags |= Style_Down;

	    if ( sub & SC_ToolButton ) {
		theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButton, opt );
		QWidget *pW = static_cast<QWidget *>(tb->parent());
		if ( !opt.isDefault() ) {
		    Qt::ArrowType type = opt.arrowType();

#define TBL_STATE(prefix) \
		    if ( !tb->isEnabled() ) \
			stateId = prefix##_DISABLED; \
		    else if ( bflags & (Style_Down | Style_On) ) \
			stateId = prefix##_PRESSED; \
		    else if ( bflags & Style_MouseOver && (d->hotWidget == w) ) \
			stateId = prefix##_HOT; \
		    else \
			stateId = prefix##_NORMAL;

		    switch( type )
		    {
		    case Qt::RightArrow:
			partId = SPNP_UPHORZ;
			TBL_STATE(UPHZS);
			break;
		    case Qt::LeftArrow:
			partId = SPNP_DOWNHORZ;
			TBL_STATE(DNHZS);
			break;
		    case Qt::UpArrow:
			partId = SPNP_UP;
			TBL_STATE(UPS);
			break;
		    case Qt::DownArrow:
		    default:
			partId = SPNP_DOWN;
			TBL_STATE(DNS);
			break;
		    }
		    theme.setName( "SPIN" );
		    theme.drawBackground( partId, stateId );
		} else if (bflags & (Style_Down | Style_On | Style_Raised)) {
		    if ( sub & SC_ToolButtonMenu ) {
			partId = TP_SPLITBUTTON;
			if ( !flags & Style_Enabled )
			    stateId = TS_DISABLED;
			else if ( flags & Style_Down || flags & Style_Sunken )
			    stateId = TS_PRESSED;
			else if ( flags & Style_MouseOver )
			    stateId = flags & Style_On ? TS_HOTCHECKED : TS_HOT;
			else if ( flags & Style_On )
			    stateId = TS_CHECKED;
			else
			    stateId = TS_NORMAL;

			theme.drawBackground( partId, stateId );
		    } else {
			if ( !qt_cast<QToolBar*>(w->parentWidget()) )
			    drawPrimitive( PE_ButtonBevel, p, theme.rec, pal, bflags, opt );
			else
			    drawPrimitive( PE_ButtonTool, p, theme.rec, pal, bflags, opt );
		    }
		} else if ( pW &&
			    pW->palette().brush(pW->backgroundRole()).pixmap() &&
			   !pW->palette().brush(pW->backgroundRole()).pixmap()->isNull() ) {
		    QPixmap pixmap = *(pW->palette().brush(pW->backgroundRole()).pixmap());
		    p->drawTiledPixmap( r, pixmap, tb->pos() );
		}
	    }
	    if ( sub & SC_ToolButtonMenu ) {
		theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButtonMenu, opt );
		drawPrimitive(PE_ButtonDropDown, p, theme.rec, pal, mflags, opt);
	    }

	    if ( tb->hasFocus() && !tb->focusProxy() ) {
		QRect fr = tb->rect();
		fr.addCoords(3, 3, -3, -3);
		drawPrimitive( PE_FocusRect, p, fr, pal );
	    }
	}
	break;

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
	{
	    const QTitleBar *titlebar = (const QTitleBar *) w;

	    XPThemeData theme( w, p, "WINDOW" );
	    if ( sub & SC_TitleBarLabel ) {
		theme.rec = titlebar->rect();
		partId = titlebar->testWFlags( WStyle_Tool ) ? WP_SMALLCAPTION :
			( titlebar->window() && titlebar->window()->isMinimized() ? WP_MINCAPTION : WP_CAPTION );
		if ( titlebar->inherits( "QDockWindowTitleBar" ) )
		    partId = WP_SMALLCAPTION;
		if ( !titlebar->isEnabled() )
		    stateId = CS_DISABLED;
		else if ( !titlebar->usesActiveColor() )
		    stateId = CS_INACTIVE;
		else
		    stateId = CS_ACTIVE;

		theme.drawBackground( partId, stateId );

		QRect ir = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, SC_TitleBarLabel ), w );
		p->setPen( pal.color(w->isActiveWindow() || !titlebar->window() ?
				     QPalette::Active : QPalette::Inactive,
				     QPalette::HighlightedText ));
		p->drawText(ir.x()+2, ir.y(), ir.width(), ir.height(),
			    AlignAuto | AlignVCenter | SingleLine, titlebar->visibleText() );
	    }
	    if ( sub & SC_TitleBarSysMenu ) {
		theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarSysMenu ), w );
		partId = titlebar->testWFlags( WStyle_Tool ) ? WP_SYSBUTTON : WP_SYSBUTTON;
		if ( !w->isEnabled() )
		    stateId = SBS_DISABLED;
		else if ( subActive == SC_TitleBarSysMenu )
		    stateId = SBS_PUSHED;
		else if ( theme.rec.contains( d->hotSpot ) )
		    stateId = SBS_HOT;
		else
		    stateId = SBS_NORMAL;
		theme.drawBackground( partId, stateId );
		if ( !titlebar->windowIcon().isNull() )
		    drawItem( p, theme.rec, AlignCenter, titlebar->palette(), true, titlebar->windowIcon() );
	    }
	    if ( titlebar->window() ) {
		if ( sub & SC_TitleBarMinButton ) {
		    theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarMinButton ), w );
		    partId = titlebar->testWFlags( WStyle_Tool ) ? WP_MINBUTTON : WP_MINBUTTON;
		    if ( !w->isEnabled() )
			stateId = MINBS_DISABLED;
		    else if ( subActive == SC_TitleBarMinButton )
			stateId = MINBS_PUSHED;
		    else if ( theme.rec.contains( d->hotSpot ) )
			stateId = MINBS_HOT;
		    else
			stateId = MINBS_NORMAL;
		    theme.drawBackground( partId, stateId );
		}
		if ( sub & SC_TitleBarMaxButton ) {
		    theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarMaxButton ), w );
		    partId = titlebar->testWFlags( WStyle_Tool ) ? WP_MAXBUTTON : WP_MAXBUTTON;
		    if ( !w->isEnabled() )
			stateId = MAXBS_DISABLED;
		    else if ( subActive == SC_TitleBarMaxButton )
			stateId = MAXBS_PUSHED;
		    else if ( theme.rec.contains( d->hotSpot ) )
			stateId = MAXBS_HOT;
		    else
			stateId = MAXBS_NORMAL;
		    theme.drawBackground( partId, stateId );
		}
		if ( sub & SC_TitleBarNormalButton ) {
		    theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarNormalButton ), w );
		    partId = titlebar->testWFlags( WStyle_Tool ) ? WP_RESTOREBUTTON : WP_RESTOREBUTTON;
		    if ( !w->isEnabled() )
			stateId = RBS_DISABLED;
		    else if ( subActive == SC_TitleBarNormalButton )
			stateId = RBS_PUSHED;
		    else if ( theme.rec.contains( d->hotSpot ) )
			stateId = RBS_HOT;
		    else
			stateId = RBS_NORMAL;
		    theme.drawBackground( partId, stateId );
		}
		if ( sub & SC_TitleBarShadeButton ) {
		    theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarShadeButton ), w );
		    partId = titlebar->testWFlags( WStyle_Tool ) ? WP_MINBUTTON : WP_MINBUTTON;
		    if ( !w->isEnabled() )
			stateId = MINBS_DISABLED;
		    else if ( subActive == SC_TitleBarShadeButton )
			stateId = MINBS_PUSHED;
		    else if ( theme.rec.contains( d->hotSpot ) )
			stateId = MINBS_HOT;
		    else
			stateId = MINBS_NORMAL;
		    theme.drawBackground( partId, stateId );
		}
		if ( sub & SC_TitleBarUnshadeButton ) {
		    theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarUnshadeButton ), w );
		    partId = titlebar->testWFlags( WStyle_Tool ) ? WP_RESTOREBUTTON : WP_RESTOREBUTTON;
		    if ( !w->isEnabled() )
			stateId = RBS_DISABLED;
		    else if ( subActive == SC_TitleBarUnshadeButton )
			stateId = RBS_PUSHED;
		    else if ( theme.rec.contains( d->hotSpot ) )
			stateId = RBS_HOT;
		    else
			stateId = RBS_NORMAL;
		    theme.drawBackground( partId, stateId );
		}
	    }
	    if ( sub & SC_TitleBarCloseButton ) {
		theme.rec = visualRect( querySubControlMetrics( CC_TitleBar, w, SC_TitleBarCloseButton ), w );
		partId = titlebar->testWFlags( WStyle_Tool ) ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON;
		if ( !w->isEnabled() )
		    stateId = CBS_DISABLED;
		else if ( subActive == SC_TitleBarCloseButton )
		    stateId = CBS_PUSHED;
		else if ( theme.rec.contains( d->hotSpot ) )
		    stateId = CBS_HOT;
		else
		    stateId = CBS_NORMAL;
		theme.drawBackground( partId, stateId );
	    }
	}
	break;
#endif
    case CC_ListView:
#ifndef QT_NO_LISTVIEW
	{
	    if ( sub & SC_ListView ) {
		const QListView *lv = (const QListView*)w;
		QWindowsStyle::drawComplexControl( control, p, w, r, pal, flags, sub, subActive, opt );
		if ( !lv->showSortIndicator() )
		    break;

		int sort = opt.isDefault() ? 0 : opt.lineWidth(); //### hackydiho; use sortColumn() in 3.1
		if ( sort < 0 )
		    break;
	    }
	    if ( sub & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
		if (opt.isDefault())
		    break;

		QListViewItem *item = opt.listViewItem(),
			     *child = item->firstChild();

		int y = r.y();
		int c;
		int dotoffset;
		QPointArray dotlines;
		if ( subActive == SC_All && sub == SC_ListViewExpand ) {
		    c = 2;
		    dotlines.resize(2);
		    dotlines[0] = QPoint( r.right(), r.top() );
		    dotlines[1] = QPoint( r.right(), r.bottom() );
		} else {
		    int linetop = 0, linebot = 0;
		    // each branch needs at most two lines, ie. four end points
		    dotoffset = (item->itemPos() + item->height() - y) %2;
		    dotlines.resize( item->childCount() * 4 );
		    c = 0;

		    // skip the stuff above the exposed rectangle
		    while ( child && y + child->height() <= 0 ) {
			y += child->totalHeight();
			child = child->nextSibling();
		    }

		    int bx = r.width() / 2;

		    // paint stuff in the magical area
		    QListView* v = item->listView();
		    int lh = qMax( p->fontMetrics().height() + 2 * v->itemMargin(),
				   QApplication::globalStrut().height() );
		    if ( lh % 2 > 0 )
			lh++;

		    XPThemeData theme( w, p, "TREEVIEW" );

		    // paint stuff in the magical area
		    while ( child && y < r.height() ) {
			linebot = y + lh/2;
			if ( (child->isExpandable() || child->childCount()) &&
			     (child->height() > 0) ) {
			    theme.rec = QRect( bx-4 + (int)p->translationX(), linebot-4+(int)p->translationY(), 9, 9 );
			    theme.drawBackground( TVP_GLYPH, child->isOpen() ? GLPS_OPENED : GLPS_CLOSED );
			    // dotlinery
			    p->setPen( pal.mid() );
			    dotlines[c++] = QPoint( bx, linetop );
			    dotlines[c++] = QPoint( bx, linebot - 5 );
			    dotlines[c++] = QPoint( bx + 5, linebot );
			    dotlines[c++] = QPoint( r.width(), linebot );
			    linetop = linebot + 5;
			} else {
			    // just dotlinery
			    dotlines[c++] = QPoint( bx+1, linebot );
			    dotlines[c++] = QPoint( r.width(), linebot );
			}

			y += child->totalHeight();
			child = child->nextSibling();
		    }

		    // Expand line height to edge of rectangle if there's any
		    // visible child below
		    while ( child && child->height() <= 0)
			child = child->nextSibling();
		    if ( child )
			linebot = r.height();

		    if ( linetop < linebot ) {
			dotlines[c++] = QPoint( bx, linetop );
			dotlines[c++] = QPoint( bx, linebot );
		    }
		}
		p->setPen( pal.dark() );

		static QBitmap *verticalLine = 0, *horizontalLine = 0;
		static QCleanupHandler<QBitmap> qlv_cleanup_bitmap;
		if ( !verticalLine ) {
		    // make 128*1 and 1*128 bitmaps that can be used for
		    // drawing the right sort of lines.
		    verticalLine = new QBitmap( 1, 129, TRUE );
		    horizontalLine = new QBitmap( 128, 1, TRUE );
		    QPointArray a( 64 );
		    QPainter p;
		    p.begin( verticalLine );
		    int i;
		    for( i=0; i<64; i++ )
			a.setPoint( i, 0, i*2+1 );
		    p.setPen( color1 );
		    p.drawPoints( a );
		    p.end();
		    QApplication::flush();
		    verticalLine->setMask( *verticalLine );
		    p.begin( horizontalLine );
		    for( i=0; i<64; i++ )
			a.setPoint( i, i*2+1, 0 );
		    p.setPen( color1 );
		    p.drawPoints( a );
		    p.end();
		    QApplication::flush();
		    horizontalLine->setMask( *horizontalLine );
		    qlv_cleanup_bitmap.add( &verticalLine );
		    qlv_cleanup_bitmap.add( &horizontalLine );
		}

		int line; // index into dotlines
		if ( sub & SC_ListViewBranch ) for( line = 0; line < c; line += 2 ) {
		    // assumptions here: lines are horizontal or vertical.
		    // lines always start with the numerically lowest
		    // coordinate.

		    // point ... relevant coordinate of current point
		    // end ..... same coordinate of the end of the current line
		    // other ... the other coordinate of the current point/line
		    if ( dotlines[line].y() == dotlines[line+1].y() ) {
			int end = dotlines[line+1].x();
			int point = dotlines[line].x();
			int other = dotlines[line].y();
			while( point < end ) {
			    int i = 128;
			    if ( i+point > end )
				i = end-point;
			    p->drawPixmap( point, other, *horizontalLine,
					   0, 0, i, 1 );
			    point += i;
			}
		    } else {
			int end = dotlines[line+1].y();
			int point = dotlines[line].y();
			int other = dotlines[line].x();
			int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;
			while( point < end ) {
			    int i = 128;
			    if ( i+point > end )
				i = end-point;
			    p->drawPixmap( other, point, *verticalLine,
					   0, pixmapoffset, 1, i );
			    point += i;
			}
		    }
		}
	    }
	}
	break;
#endif //QT_NO_LISTVIEW

    default:
	QWindowsStyle::drawComplexControl( control, p, w, r, pal, flags, sub, subActive, opt );
	break;
    }

    d->currentWidget = 0;
}

/*! \reimp */
int QWindowsXPStyle::pixelMetric( PixelMetric metric,
				 const QWidget *widget ) const
{
    if ( !use_xp )
	return QWindowsStyle::pixelMetric( metric, widget );

    switch ( metric ) {
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
	{
	    XPThemeData theme( widget, 0, "BUTTON", BP_CHECKBOX, CBS_UNCHECKEDNORMAL );

	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		if ( metric == PM_IndicatorWidth )
		    return size.cx+2;
		return size.cy+2;
	    }
	}
	break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	{
	    XPThemeData theme( widget, 0, "BUTTON", BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL );

	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		if ( metric == PM_ExclusiveIndicatorWidth )
		    return size.cx+2;
		return size.cy+2;
	    }
	}
	break;

    case PM_ProgressBarChunkWidth:
	{
	    XPThemeData theme( widget, 0, "PROGRESS", PP_CHUNK );

	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cx;
	    }
	}
	break;

    case PM_ScrollBarExtent:
	{
	    XPThemeData theme( widget, 0, "SCROLLBAR", SBP_LOWERTRACKHORZ );

	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cy;
	    }
	}
	break;

    case PM_SliderThickness:
	{
	    XPThemeData theme( widget, 0, "TRACKBAR", TKP_THUMB );

	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cy;
	    }
	}
	break;

    case PM_MenuButtonIndicator:
	{
	    XPThemeData theme( widget, 0, "TOOLBAR", TP_SPLITBUTTONDROPDOWN );

	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cx;
	    }
	}
	break;

    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
	return 1;

    case PM_TabBarTabOverlap:
    	return 2;

    case PM_TabBarBaseOverlap:
    case PM_TabBarBaseHeight:
	return 0;

    case PM_TitleBarHeight:
	return QWindowsStyle::pixelMetric( metric, widget ) + 4;

    case PM_MDIFrameWidth:
	{
	    XPThemeData theme( widget, 0, "WINDOW", WP_FRAMELEFT, FS_ACTIVE );
	    if ( theme.isValid() ) {
		SIZE size;
		pGetThemePartSize( theme.handle(), NULL, WP_FRAMELEFT, FS_ACTIVE, 0, TS_TRUE, &size );
		return size.cx-1;
	    }
	}
	break;

    case PM_MDIMinimizedWidth:
	return 160;

    case PM_PopupMenuFrameHorizontalExtra:
	return 2;
    case PM_PopupMenuFrameVerticalExtra:
	return 2;
    case PM_SplitterWidth:
	return qMax( 5, QApplication::globalStrut().width() );

    default:
	break;
    }

    return QWindowsStyle::pixelMetric( metric, widget );
}

/*! \reimp */
QRect QWindowsXPStyle::querySubControlMetrics( ComplexControl control,
					      const QWidget *widget,
					      SubControl sc,
					      const QStyleOption &option ) const
{
    if ( !use_xp )
	return QWindowsStyle::querySubControlMetrics( control, widget, sc, option );

    switch ( control ) {
    case CC_TitleBar: {
#ifndef QT_NO_TITLEBAR
	const QTitleBar *titlebar = (const QTitleBar *) widget;
	const int controlTop = widget->testWFlags( WStyle_Tool ) ? 4 : 6;
	const int controlHeight = widget->height() - controlTop - 3;

	switch (sc) {
	case SC_TitleBarLabel: {
	    const QTitleBar *titlebar = (QTitleBar*)widget;
	    QRect ir( 0, 0, titlebar->width(), titlebar->height() );
	    if ( titlebar->testWFlags( WStyle_Tool ) ) {
		if ( titlebar->testWFlags( WStyle_SysMenu ) )
		    ir.addCoords( 0, 0, -controlHeight-3, 0 );
		if ( titlebar->testWFlags( WStyle_MinMax ) )
		    ir.addCoords( 0, 0, -controlHeight-2, 0 );
	    } else {
		if ( titlebar->testWFlags( WStyle_SysMenu ) )
		    ir.addCoords( controlHeight+3, 0, -controlHeight-3, 0 );
		if ( titlebar->testWFlags( WStyle_Minimize ) )
		    ir.addCoords( 0, 0, -controlHeight-2, 0 );
		if ( titlebar->testWFlags( WStyle_Maximize ) )
		    ir.addCoords( 0, 0, -controlHeight-2, 0 );
	    }
	    return ir; }

	case SC_TitleBarCloseButton:
	    return QRect(titlebar->width()-( controlHeight + 1 )-controlTop,
			 controlTop, controlHeight, controlHeight);

	case SC_TitleBarMaxButton:
	case SC_TitleBarShadeButton:
	case SC_TitleBarUnshadeButton:
	    return QRect(titlebar->width()-((controlHeight + 1 ) * 2)-controlTop,
			 controlTop, controlHeight, controlHeight);
	    break;

	case SC_TitleBarMinButton:
	case SC_TitleBarNormalButton: {
	    int offset = controlHeight + 1;
	    if ( !titlebar->testWFlags( WStyle_Maximize ) )
		offset *= 2;
	    else
		offset *= 3;
	    return QRect(titlebar->width() - offset-controlTop,
			 controlTop, controlHeight, controlHeight); }

	case SC_TitleBarSysMenu:
	    return QRect( 3, controlTop, controlHeight, controlHeight);

	default:
	    break;
	}
#endif
	return QRect(); } //are you sure you want to do this? ###
    case CC_ComboBox: {
	int x = 0, y = 0, wi = widget->width(), he = widget->height();
	int xpos = x;
	xpos += wi - 1 - 16;

	switch ( sc ) {
	case SC_ComboBoxFrame:
	    return widget->rect();
	case SC_ComboBoxArrow:
	    return QRect(xpos, y+1, 16, he-2);
	case SC_ComboBoxEditField:
	    return QRect(x+2, y+2, wi-3-16, he-4);
	case SC_ComboBoxListBoxPopup:
	    return option.rect();
	default:
	    break;
	}
	break; }
    default:
	break;
    }
    return QWindowsStyle::querySubControlMetrics( control, widget, sc, option );
}

/*!
  \reimp
*/
QSize QWindowsXPStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       const QStyleOption& opt ) const
{
    if ( !use_xp )
	return QWindowsStyle::sizeFromContents( contents, widget, contentsSize, opt );

    QSize sz(contentsSize);

    switch (contents) {
    case CT_Q3PopupMenuItem:
	{
#ifndef QT_NO_POPUPMENU
	    if (! widget || opt.isDefault())
		break;

	    const QPopupMenu *popup = (const QPopupMenu *) widget;
	    bool checkable = popup->isCheckable();
	    QMenuItem *mi = opt.menuItem();
	    int maxpmw = opt.maxIconWidth();
	    int w = sz.width(), h = sz.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan())
		    h += 2*windowsItemVMargin + 2*windowsItemFrame;
	    } else if ( mi->widget() ) {
	    } else if (mi->isSeparator()) {
		w = 10; // arbitrary
		h = windowsSepHeight;
	    } else {
		if (mi->pixmap())
		    h = qMax(h, mi->pixmap()->height() + 2*windowsItemFrame);
		else if (! mi->text().isNull())
		    h = qMax(h, popup->fontMetrics().height() + 2*windowsItemVMargin +
			     2*windowsItemFrame);

		if (mi->iconSet() != 0)
		    h = qMax(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height() +
			     2*windowsItemFrame);
	    }

	    if (! mi->text().isNull() && mi->text().indexOf('\t') >= 0) {
		w += 20;
	    } else if (mi->popup()) {
		w += 2*windowsArrowHMargin;
	    }

	    if (checkable && maxpmw < 20)
		w += 20 - maxpmw;
	    if (checkable || maxpmw > 0)
		w += windowsCheckMarkHMargin;
	    w += 20;

	    sz = QSize(w, h);
#endif
	    break;
	}

    default:
	sz = QWindowsStyle::sizeFromContents(contents, widget, sz, opt);
	break;
    }

    return sz;
}

/*! \reimp */
int QWindowsXPStyle::styleHint( StyleHint stylehint,
			   const QWidget *widget,
			   const QStyleOption& opt,
			   QStyleHintReturn* returnData ) const
{
    if ( !use_xp )
	return QWindowsStyle::styleHint( stylehint, widget, opt, returnData );

    switch ( stylehint ) {
    case SH_TitleBar_NoBorder:
	return 1;

    case SH_GroupBox_TextLabelColor:
	if ( widget->isEnabled() )
	    return d->groupBoxTextColor;
	else
	    return d->groupBoxTextColorDisabled;

    case SH_Table_GridLineColor:
	return 0xC0C0C0;

    case SH_LineEdit_PasswordCharacter:
	{
	    const QFontMetrics &fm = widget->fontMetrics();
	    if ( fm.inFont( QChar(0x25CF) ) )
		return 0x25CF;
	    else if ( fm.inFont( QChar(0x2022) ) )
		return 0x2022;
	    else
		return '*';
	}

    default:
	return QWindowsStyle::styleHint( stylehint, widget, opt, returnData );
    }
}

// HotSpot magic
/*! \reimp */
bool QWindowsXPStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !o->isWidgetType() || e->type() == QEvent::Paint || !use_xp )
	return QWindowsStyle::eventFilter( o, e );

    QWidget *widget = (QWidget*)o;

    switch ( e->type() ) {
    case QEvent::MouseMove:
	{
	    if (!widget->isActiveWindow() || !widget->isEnabled())
		break;

	    QMouseEvent *me = (QMouseEvent*)e;

	    d->hotWidget = widget;
	    d->hotSpot = me->pos();

	    if ( qt_cast<QTabBar*>(o) ) {
		QTabBar* bar = (QTabBar*)o;
		QTab * t = bar->selectTab( me->pos() );
		if ( d->hotTab != t ) {
		    d->hotTab = t;
		    widget->repaint();
		}
	    } else if ( qt_cast<QHeader*>(o) ) {
		QHeader *header = (QHeader*)o;
		QRect oldHeader = d->hotHeader;

		if ( header->orientation() == Horizontal )
		    d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.x() + header->offset() ) );
		else
		    d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.y() + header->offset() ) );

		if ( oldHeader != d->hotHeader ) {
		    if ( oldHeader.isValid() )
			header->update( oldHeader );
		    if ( d->hotHeader.isValid() )
			header->update( d->hotHeader );
		}
#ifndef QT_NO_TITLEBAR
	    } else if ( qt_cast<QTitleBar*>(o) ) {
		static SubControl clearHot = SC_TitleBarLabel;
		QTitleBar *titlebar = (QTitleBar*)o;
		SubControl sc = querySubControl( CC_TitleBar, titlebar, d->hotSpot );
		if ( sc != clearHot || clearHot != SC_TitleBarLabel ) {
		    QRect rect = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, clearHot ), titlebar );
		    titlebar->repaint( rect );

		    clearHot = sc;
		    rect = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, sc ), titlebar );
		    titlebar->repaint( rect );
		}
#endif
#ifndef QT_NO_SLIDER
	    } else if ( ::qt_cast<QSlider*>(o) ) {
		static bool clearSlider = FALSE;
		QSlider *slider = (QSlider*)o;
		const QRect rect = querySubControlMetrics(CC_Slider, (QWidget*)o, SC_SliderHandle, 0);
		const bool inSlider = rect.contains( d->hotSpot );
		if ( ( inSlider && !clearSlider ) || ( !inSlider && clearSlider ) ) {
		    clearSlider = inSlider;
		    slider->repaint( rect );
		}
#endif
	    } else if ( ::qt_cast<QComboBox*>(o) ) {
		static bool clearCombo = FALSE;
	    	const QRect rect = visualRect(querySubControlMetrics( CC_ComboBox, (QWidget*)o, SC_ComboBoxArrow ), (QWidget*)o);
		const bool inArrow = rect.contains( d->hotSpot );
		if ( ( inArrow && !clearCombo ) || ( !inArrow && clearCombo ) ) {
		    clearCombo = inArrow;
		    widget->repaint();
		}
	    } else {
		widget->repaint();
	    }
	}
        break;

    case QEvent::WindowActivate:
	if ( !widget->testAttribute(QWidget::WA_UnderMouse) )
	    break;
	// FALL THROUGH
    case QEvent::Enter:
	if (!widget->isActiveWindow() || !widget->isEnabled())
	    break;
        d->hotWidget = widget;
	widget->repaint();
        break;

    case QEvent::Leave:
	if ( !widget->isActiveWindow() )
	    break;
	// FALL THROUGH
    case QEvent::WindowDeactivate:
        if ( widget == d->hotWidget) {
            d->hotWidget = 0;
	    d->hotHeader = QRect();
	    d->hotTab = 0;
	    widget->repaint();
        }
        break;

    case QEvent::FocusOut:
    case QEvent::FocusIn:
	widget->repaint();
	break;

    case QEvent::Resize:
	updateRegion( widget );
	break;

    case QEvent::Move:
	//if ( widget->paletteBackgroundPixmap() &&
	//     widget->backgroundOrigin() != QWidget::WidgetOrigin )
	//    widget->update();
	break;

    default:
        break;
    }

    return QWindowsStyle::eventFilter( o, e );
}

/*! \reimp */
void QWindowsXPStyle::activeTabChanged()
{
    const QObject *s = sender();
    if ( !qt_cast<QTabBar*>(s) )
	return;

    ((QWidget *)s)->repaint();
}

#endif //QT_NO_STYLE_WINDOWSXP
