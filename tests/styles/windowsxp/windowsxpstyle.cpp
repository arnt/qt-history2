#include "windowsxpstyle.h"

#if !defined (Q_WS_WIN)
#error "This style can only be compiled on Windows XP"
#endif

#ifndef QT_NO_STYLE_WINDOWSXP

#include "qmenubar.h"
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

# include <qt_windows.h>
# include <uxtheme.h>

static bool repaintByMouseMove          = FALSE;
static int activeScrollBarElement       = 0;

# define Q_RECT					\
	RECT r;					\
	r.left = x;				\
	r.right = x+w;				\
	r.top = y;				\
	r.bottom = y+h;

#ifndef BP_CHECKBOX
#define BP_CHECKBOX	    3
#define BP_GROUPBOX	    4
#define BP_PUSHBUTTON	    1
#define BP_RADIOBUTTON	    2
#define BP_USERBUTTON	    5

#define CLP_TIME	    1

#define CP_DROPDOWNBUTTON   1
#endif

class QWindowsXPStyle::Private
{
public:
    Private()
	: hotWidget( 0 ), hotSpot( -500, -500 ), hotTab( 0 )
    {
	if ( qWinVersion() == WV_XP && !init_xp ) {
	    init_xp = TRUE;
/*
	    uxtheme = new QLibrary( "uxtheme" );
	    Q_LOAD_THEME_FNC( IsThemeActive )
	    Q_LOAD_THEME_FNC( SetWindowTheme )
	    Q_LOAD_THEME_FNC( IsThemeBackgroundPartiallyTransparent )
	    Q_LOAD_THEME_FNC( GetThemeSysSize )
	    Q_LOAD_THEME_FNC( CloseThemeData )
	    Q_LOAD_THEME_FNC( DrawThemeBackground )
	    Q_LOAD_THEME_FNC( DrawThemeBorder )
	    Q_LOAD_THEME_FNC( DrawThemeIcon )
	    Q_LOAD_THEME_FNC( DrawThemeLine )
	    Q_LOAD_THEME_FNC( DrawThemeText )
	    Q_LOAD_THEME_FNC( FormatThemeMessage )
	    Q_LOAD_THEME_FNC( GetThemeAppProperties )
	    Q_LOAD_THEME_FNC( GetThemeBackgroundContentRect )
	    Q_LOAD_THEME_FNC( GetThemeBackgroundExtent )
	    Q_LOAD_THEME_FNC( GetThemeBackgroundRegion )
	    Q_LOAD_THEME_FNC( GetThemeBool )
	    Q_LOAD_THEME_FNC( GetThemeColor )
	    Q_LOAD_THEME_FNC( GetThemeFilename )
	    Q_LOAD_THEME_FNC( GetThemeEnumValue )
	    Q_LOAD_THEME_FNC( GetThemeSysFont )
	    Q_LOAD_THEME_FNC( GetThemeFont )
	    Q_LOAD_THEME_FNC( GetThemeInt )
//		Q_LOAD_THEME_FNC( GetThemeIntList )		    Not found in beta1
	    Q_LOAD_THEME_FNC( GetThemeLastErrorContext )
	    Q_LOAD_THEME_FNC( GetThemeMargins )
	    Q_LOAD_THEME_FNC( GetThemeMetric )
	    Q_LOAD_THEME_FNC( GetThemePartSize )
	    Q_LOAD_THEME_FNC( GetThemePosition )
	    Q_LOAD_THEME_FNC( GetThemePropertyOrigin )
	    Q_LOAD_THEME_FNC( GetThemeRect )
	    Q_LOAD_THEME_FNC( GetThemeString )
	    Q_LOAD_THEME_FNC( GetThemeSysBool )
	    Q_LOAD_THEME_FNC( GetThemeSysColor )
	    Q_LOAD_THEME_FNC( GetThemeDocumentationProperty )
//		Q_LOAD_THEME_FNC( GetThemeSysColorBrush )	    Not found in beta1
	    Q_LOAD_THEME_FNC( GetThemeSysString )
	    Q_LOAD_THEME_FNC( GetThemeTextExtent )
	    Q_LOAD_THEME_FNC( GetThemeTextMetrics )
	    Q_LOAD_THEME_FNC( GetWindowTheme )
	    Q_LOAD_THEME_FNC( HitTestThemeBackground )
	    Q_LOAD_THEME_FNC( SetThemeAppProperties )
	    Q_LOAD_THEME_FNC( IsAppThemed )
//		Q_LOAD_THEME_FNC( SetWindowThemeStyle )		    Not found in beta1
	    Q_LOAD_THEME_FNC( IsThemePartDefined )
	    Q_LOAD_THEME_FNC( OpenThemeData )
*/
	    limboWidget = new QWidget( 0, "xp_limbo_widget" );
	    hwnd = limboWidget->winId();

	    use_xp = IsThemeActive() && IsAppThemed();
	}
	if ( use_xp )
	    ref++;
    }
    ~Private()
    {
	if ( use_xp ) {
	    if ( !--ref ) {
/*
		delete uxtheme;
		uxtheme = 0;
*/
		init_xp = FALSE;
		use_xp  = FALSE;
		delete limboWidget;
		limboWidget = 0;
	    }
	}
    }

    static HTHEME getThemeData( TCHAR*c )
    {
	if ( !use_xp )
	    return NULL;

	return OpenThemeData( hwnd, c );
    }

    static bool getThemeResult( HRESULT res )
    {
	if ( res == S_OK )
	    return TRUE;
	THEME_ERROR_CONTEXT myThemeErrorContext;
	HRESULT hRslt = GetThemeLastErrorContext(&myThemeErrorContext);
	if ( hRslt != S_OK ) {
	    qSystemWarning( "GetThemeLastErrorContext failed!", hRslt );
	} else {
//	    FormatThemeMessage( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &myThemeErrorContext );
	}
	return FALSE;
    }

    static HWND hwnd;
    static bool use_xp;

    // hot-widget stuff
    QPoint hotSpot;

    QWidget *hotWidget;
    QTab *hotTab;
    QRect hotHeader;
    QPalette oldPalette;

private:
    static ulong ref;
    static bool init_xp;
    static QWidget *limboWidget;
};

ulong QWindowsXPStyle::Private::ref = 0;
bool QWindowsXPStyle::Private::use_xp  = FALSE;
bool QWindowsXPStyle::Private::init_xp = FALSE;
QWidget *QWindowsXPStyle::Private::limboWidget = 0;
HWND QWindowsXPStyle::Private::hwnd = NULL;

QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new Private;
}

QWindowsXPStyle::~QWindowsXPStyle()
{
    delete d;
}

void QWindowsXPStyle::polish( QWidget *widget )
{
    if ( widget->inherits( "QButton" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QTabBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QHeader" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QComboBox" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QSpinBox" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QScrollBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    }
    QWindowsStyle::polish( widget );
}

void QWindowsXPStyle::unPolish( QWidget *widget )
{
    widget->removeEventFilter( this );
    QWindowsStyle::unPolish( widget );
}

// shapes
void QWindowsXPStyle::drawPanel( QPainter *p, int x, int y, int w, int h,
		const QColorGroup &g, bool sunken, int lineWidth, const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"TAB" );
    if ( !htheme ) {
	QWindowsStyle::drawPanel( p, x, y, w, h, g, sunken, lineWidth, fill );
	return;
    }

    Q_RECT

    DrawThemeBackground( htheme, p->handle(), 9, 1, &r, 0 );

    CloseThemeData( htheme );

#if 0 // strange place for a brute force algorithm...
    HWND hwnd = ((QWidget*)p->device())->winId();
    unsigned short* const bla = new unsigned short[40];
    const int start = 3;
    for ( int a = 0; a < 40; ++a )
	bla[a] = '\0';
    bla[0] = 'P';bla[1] = 'O';bla[2] = 'P';

    while( TRUE ) {

	for ( int i = 'A'; i <= 'Z'; i++ ) {
	    HTHEME htheme = OpenThemeData( hwnd, bla );
	    if ( htheme ) {
		QString str = qt_winQString( bla );
		qDebug( "Theme for %s", str.latin1() );
		HRESULT res = CloseThemeData( htheme );
		if ( res != S_OK )
		    qDebug( "Handle couldn't be closed" );
		return;
	    }
	    if ( bla[start] == 'Z' ) {
		int b = start;
		while ( b < 39 && bla[b] == 'Z' ) {
		    bla[b] = 'A';
		    b+=1;
		}
		if ( b < 39 ) {
		    if ( bla[b] == 0 )
			bla[b] = 'A';
		    else
			bla[b] += 1;
		} else {
		    qDebug( "Done!" );
		    return;
		}
	    } else {
		bla[start] += 1;
	    }
	}
    }
#endif
}


void QWindowsXPStyle::drawButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool sunken, const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawButton( p, x, y, w, h, g, sunken, fill );
	return;
    }

    Q_RECT

    if ( sunken )
	DrawThemeBackground( htheme, p->handle(), 1, 2, &r, 0 );
    else
	DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool sunken, const QBrush *fill )
{
    drawButton( p, x, y, w, h, g, sunken, fill );
}

void QWindowsXPStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool on, bool down, bool enabled,
		 bool autoRaised, const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"TOOLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolButton( p, x, y, w, h, g, on, down, enabled, autoRaised, fill );
	return;
    }

    Q_RECT

    int statusId;
    if ( !enabled )
	statusId = 4;
    else if ( down )
	statusId = 3;
    else if ( !g.brightText().isValid() )
	statusId = 2;
    else if ( on )
	statusId = 5;
    else
	statusId = 1;

    DrawThemeBackground( htheme, p->handle(), 1, statusId, &r, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawDropDownButton( QPainter *p, int x, int y, int w, int h,
		 const QColorGroup &g, bool down, bool enabled, bool autoRaised,
		 const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"TOOLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawDropDownButton( p, x, y, w, h, g, down, enabled, autoRaised, fill );
	return;
    }

    Q_RECT

    int statusId;
    if ( !enabled )
	statusId = 4;
    else if ( down )
	statusId = 3;
    else if ( !g.brightText().isValid() )
	statusId = 2;
    else
	statusId = 1;

    DrawThemeBackground( htheme, p->handle(), 3, statusId, &r, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawPopupPanel( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g,  int lineWidth, const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawPopupPanel( p, x, y, w, h, g, lineWidth, fill );
	return;
    }

    Q_RECT

    // ### too dark
    DrawThemeBorder( htheme, p->handle(), 1, &r );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawArrow( QPainter *p, Qt::ArrowType type, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool enabled, const QBrush *fill )
{
    QWindowsStyle::drawArrow( p, type, down, x, y, w, h, g, enabled, fill );
}


// Push button
void QWindowsXPStyle::getButtonShift( int &x, int &y) const
{
    x = y = 0;
}

void QWindowsXPStyle::drawPushButton( QPushButton* btn, QPainter *p)
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawPushButton( btn, p );
	return;
    }

    RECT r;
    r.left = 0;
    r.right = btn->width();
    r.top = 0;
    r.bottom = btn->height();

    if ( btn->isEnabled() ) {
	int stateId = 1;
	if ( btn->isDown() ) {
	    stateId = 3;
	} else if ( d->hotWidget == btn ) {
	    stateId  =2;
	} else if ( btn->isDefault() ) {
	    stateId = 5;
	}
	DrawThemeBackground( htheme, p->handle(), BP_PUSHBUTTON, stateId, &r, 0);
    } else {
	DrawThemeBackground( htheme, p->handle(), BP_PUSHBUTTON, 4, &r, 0);
    }

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p )
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawPushButton( btn, p );
	return;
    }

    RECT r;
    r.left = 0;
    r.right = btn->width();
    r.top = 0;
    r.bottom = btn->height();

    int stateId;
    if ( btn->isEnabled() ) {
	if ( btn->isDown() ) {
	    stateId = 3;
	} else if ( btn->isOn() ) {
	    stateId  =2;
	} else if ( btn->hasFocus() ) {
	    stateId = 5;
	} else
	    stateId = 1;
    } else {
	stateId = 4;
    }

    RECT extent;
    GetThemeTextExtent( htheme, p->handle(), BP_PUSHBUTTON, stateId, 
	(TCHAR*)qt_winTchar( btn->text(), TRUE ), -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE, NULL, &extent );

    DrawThemeText( htheme, p->handle(), BP_PUSHBUTTON, stateId,
	(TCHAR*)qt_winTchar( btn->text(), TRUE ), -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE, 0, &r );

    CloseThemeData( htheme );
}

// Radio button
QSize QWindowsXPStyle::exclusiveIndicatorSize() const
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	return QWindowsStyle::exclusiveIndicatorSize();
    }

    SIZE size;
    GetThemePartSize( htheme, NULL, BP_RADIOBUTTON, 1, TS_TRUE, &size );
    return QSize( size.cx, size.cy );
}

void QWindowsXPStyle::drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
		    const QColorGroup &g, bool on, bool down, bool enabled )
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawExclusiveIndicator( p, x, y, w, h, g, on, down, enabled );
	return;
    }

    Q_RECT

    int stateId;
    if ( !enabled ) {
	if ( on )
	    stateId = 8;
	else
	    stateId = 4;
    } else if ( down ) {
	if ( on )
	    stateId = 7;
	else
	    stateId = 3;
    } else if ( !g.brightText().isValid() ) {
	if ( on )
	    stateId = 6;
	else
	    stateId = 2;
    } else {
	if ( on )
	    stateId = 5;
	else
	    stateId = 1;
    }

    QRegion reg = p->clipRegion();

    BOOL pt = IsThemeBackgroundPartiallyTransparent( htheme, BP_RADIOBUTTON, stateId );
    if ( pt ) 
	p->fillRect( x, y, w, h, g.background() );
    DrawThemeBackground( htheme, p->handle(), BP_RADIOBUTTON, stateId, &r, 0 );
    p->setClipRegion( reg );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawExclusiveIndicatorMask( QPainter *p, int x, int y, int w, int h, bool on)
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawExclusiveIndicatorMask( p, x, y, w, h, on );
	return;
    }

    int stateId;
    if ( on )
	stateId = 5;
    else
	stateId = 1;

    BOOL pt = IsThemeBackgroundPartiallyTransparent( htheme, BP_RADIOBUTTON, stateId );
    if ( pt ) {
	Q_RECT
	HRGN region;
	GetThemeBackgroundRegion( htheme, BP_RADIOBUTTON, stateId, &r, &region );
	p->fillRect( x, y, w, h, color1 );
    }
    p->fillRect( x, y, w, h, color1 );
    CloseThemeData( htheme );
}

// CheckBox
QSize QWindowsXPStyle::indicatorSize() const
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	return QWindowsStyle::indicatorSize();
    }

    SIZE size;
    GetThemePartSize( htheme, NULL, BP_CHECKBOX, 1, TS_TRUE, &size );
    return QSize( size.cx, size.cy );    
}

void QWindowsXPStyle::drawIndicator( QPainter* p, int x, int y, int w, int h, const QColorGroup &g,
		    int state, bool down, bool enabled )
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawIndicator( p, x, y, w, h, g, state, down, enabled );
	return;
    }

    Q_RECT

    int stateId;
    switch ( state ) {
    case QButton::On:
	stateId = 5;
	break;
    case QButton::NoChange:
	stateId = 9;
	break;
    default:
	stateId = 1;
	break;
    }
    if ( down )
	stateId += 2;
    else if ( !enabled )
	stateId += 3;
    else if ( !g.brightText().isValid() )
	stateId += 1;

    DrawThemeBackground( htheme, p->handle(), BP_CHECKBOX, stateId, &r, 0 );

    CloseThemeData( htheme );
}

// ComboBox
void QWindowsXPStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
		  const QColorGroup &g, bool sunken, bool editable, bool enabled, const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"COMBOBOX" );
    if ( !htheme ) {
	QWindowsStyle::drawComboButton( p, x, y, w, h, g, sunken, editable, enabled, fill );
	return;
    }

    Q_RECT

    DrawThemeBorder( htheme, p->handle(), 1, &r );

    int xpos = x;
    if( !QApplication::reverseLayout() )
	xpos += w - 2 - 16;

    RECT r2;
    r2.left = xpos;
    r2.right = xpos+16;
    r2.top = y+2;
    r2.bottom = y+h-2;

    if ( sunken )
	DrawThemeBackground( htheme, p->handle(), CP_DROPDOWNBUTTON, 3, &r2, 0 );
    else
	DrawThemeBackground( htheme, p->handle(), CP_DROPDOWNBUTTON,
	    enabled ? ( d->hotWidget == p->device() ? 2 : 1 ) : 4, &r2, 0 );

    CloseThemeData( htheme );
}

// Toolbar
int QWindowsXPStyle::toolBarHandleExtent() const
{
    return 15;
}

void QWindowsXPStyle::drawToolBarHandle( QPainter *p, const QRect &r,
				Qt::Orientation orientation,
				bool highlight, const QColorGroup &cg,
				bool drawBorder )
{
    HTHEME htheme = Private::getThemeData( L"REBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolBarHandle( p, r, orientation, highlight, cg, drawBorder );
	return;
    }

    RECT rect;
    rect.left = r.left();
    rect.top = r.top();
    rect.right = r.right();
    rect.bottom = r.bottom();

    if ( orientation == Horizontal )
	DrawThemeBackground( htheme, p->handle(), 1, 1, &rect, 0 );
    else
	DrawThemeBackground( htheme, p->handle(), 2, 1, &rect, 0 );

    CloseThemeData( htheme );
}

int QWindowsXPStyle::toolBarFrameWidth() const
{
    return QWindowsStyle::toolBarFrameWidth();
}

void QWindowsXPStyle::drawToolBarPanel( QPainter *p, int x, int y, int w, int h,
			       const QColorGroup &g, const QBrush *fill )
{
    HTHEME htheme = Private::getThemeData( L"REBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolBarPanel( p, x, y, w, h, g, fill );
	return;
    }

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x+w;
    rect.bottom = y+h;

    DrawThemeBackground( htheme, p->handle(), 3, 1, &rect, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawToolBarSeparator( QPainter *p, int x, int y, int w, int h,
				   const QColorGroup & g, Orientation orientation )
{
    HTHEME htheme = Private::getThemeData( L"TOOLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawToolBarSeparator( p, x, y, w, h, g, orientation );
	return;
    }

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x+w;
    rect.bottom = y+h;

    if ( orientation == Horizontal )
	DrawThemeBackground( htheme, p->handle(), 5, 1, &rect, 0 );
    else
	DrawThemeBackground( htheme, p->handle(), 6, 1, &rect, 0 );

    CloseThemeData( htheme );
}

QSize QWindowsXPStyle::toolBarSeparatorSize( Qt::Orientation orientation ) const
{
    return QWindowsStyle::toolBarSeparatorSize( orientation );
}

// TabBar
void QWindowsXPStyle::drawTab( QPainter* p, const QTabBar *bar, QTab *tab, bool selected )
{
    HTHEME htheme = Private::getThemeData( L"TAB" );
    if ( !htheme ) {
	QWindowsStyle::drawTab( p, bar, tab, selected );
	return;
    }

    QRect rect( tab->rect() );
    RECT r;
    r.left = rect.left();
    r.right = rect.right()+ selected;
    r.top = rect.top();
    r.bottom = rect.bottom()+ selected;

    if ( selected )
	DrawThemeBackground( htheme, p->handle(), 6, d->hotTab == tab ? 5 : 3, &r, 0 );
    else {
	if ( !tab->identitifer() )
	    DrawThemeBackground( htheme, p->handle(), 2, d->hotTab == tab ? 2 : 1, &r, 0 );
	else
	    DrawThemeBackground( htheme, p->handle(), 1, d->hotTab == tab ? 2 : 1, &r, 0 );
    }

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawTabBarExtension( QPainter * p, int x, int y, int w, int h,
				  const QColorGroup & cg, const QTabWidget * tw )
{
    HTHEME htheme = Private::getThemeData( L"TAB" );
    if ( !htheme ) {
	QWindowsStyle::drawTabBarExtension( p, x, y, w, h, cg, tw );
	return;
    }
    CloseThemeData( htheme );
}

// ScrollBar
QSize QWindowsXPStyle::scrollBarExtent() const
{
    HTHEME htheme = Private::getThemeData( L"SCROLLBAR" );
    if ( !htheme ) {
	return QWindowsStyle::scrollBarExtent();
    }

    RECT cr;
    RECT r;
    RECT bound;
    bound.left = 0;
    bound.right = 16;
    bound.top = 0;
    bound.bottom = 16;
    GetThemeBackgroundContentRect( htheme, 0, 1, 1, &bound, &cr );
    GetThemeBackgroundExtent( htheme, 0, 1, 1, &cr, &r );

    int hsize = r.right - r.left;
    int vsize = r.bottom - r.top;

    return QSize( vsize, hsize );
}

void QWindowsXPStyle::drawScrollBarControls( QPainter *p,  const QScrollBar *sb,
			int sliderStart, uint controls, uint activeControl )
{
    HTHEME htheme = Private::getThemeData( L"SCROLLBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawScrollBarControls( p, sb, sliderStart, controls, activeControl );
	return;
    }

#define HORIZONTAL      (sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL        !HORIZONTAL
#define WINDOWS_BORDER  2
#define SLIDER_MIN      9

    QColorGroup g = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) // sanity check
	sliderStart = sliderMax;

    int b = 0;
    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
	subY = addY = ( extent - dimB ) / 2;
	subX = b;
	addX = length - dimB - b;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = b;
	addY = length - dimB - b;
    }

    subB.setRect( subX,subY,dimB,dimB );
    addB.setRect( addX,addY,dimB,dimB );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent - b*2;
    if ( HORIZONTAL ) {
	subPageR.setRect( subB.right() + 1, b,
			  sliderStart - subB.right() - 1 , sliderW );
	addPageR.setRect( sliderEnd, b, addX - sliderEnd, sliderW );
	sliderR .setRect( sliderStart, b, sliderLength, sliderW );
    } else {
	subPageR.setRect( b, subB.bottom() + 1, sliderW,
			  sliderStart - subB.bottom() - 1 );
	addPageR.setRect( b, sliderEnd, sliderW, addY - sliderEnd );
	sliderR .setRect( b, sliderStart, sliderW, sliderLength );
    }

    bool isScrollBarUpToDate = FALSE;
    if ( repaintByMouseMove ) {
        if ( addB.contains( d->hotSpot ) ) {
            isScrollBarUpToDate = ( activeScrollBarElement == AddLine );
            activeScrollBarElement = AddLine;
        } else if ( subB.contains( d->hotSpot )) {
            isScrollBarUpToDate = ( activeScrollBarElement == SubLine );
            activeScrollBarElement = SubLine;
        } else if ( sliderR.contains( d->hotSpot )) {
            isScrollBarUpToDate = ( activeScrollBarElement == Slider );
            activeScrollBarElement = Slider;
        } else {
            activeScrollBarElement = 0;
        }
    } else {
        activeScrollBarElement = 0;
    }

    bool maxedOut = (sb->maxValue() == sb->minValue());

    if ( isScrollBarUpToDate ) {
	CloseThemeData( htheme );
	return;
    }

    if ( controls & AddLine ) {
	RECT r;
	r.left = addB.left();
	r.right = addB.right();
	r.top = addB.top();
	r.bottom = addB.bottom();

	int stateId;
	if ( maxedOut )
	    stateId = 8;
	else if ( activeControl == AddLine )
	    stateId = 7;
	else if ( d->hotWidget == (QWidget*)sb && addB.contains( d->hotSpot ) )
	    stateId = 6;
	else
	    stateId = 5;
	if ( HORIZONTAL )
	    stateId += 8;

	DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0 );
    }
    if ( controls & SubLine ) {
	RECT r;
	r.left = subB.left();
	r.right = subB.right();
	r.top = subB.top();
	r.bottom = subB.bottom();

	int stateId;
	if ( maxedOut )
	    stateId = 4;
	else if ( activeControl == SubLine )
	    stateId = 3;
	else if ( d->hotWidget == (QWidget*)sb && subB.contains( d->hotSpot ) )
	    stateId = 2;
	else
	    stateId = 1;
	if ( HORIZONTAL )
	    stateId += 8;

	DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0 );
    }
    if ( maxedOut ) {
	RECT r;
	r.left = sliderR.left();
	r.right = sliderR.right() + HORIZONTAL;
	r.top = sliderR.top();
	r.bottom = sliderR.bottom() + !HORIZONTAL;

	const int swidth = r.right - r.left;
	const int sheight = r.bottom - r.top;

	RECT gr;
	if ( HORIZONTAL ) {
	    gr.left = r.left + swidth/2 - 5;
	    gr.right = gr.left + 10;
	    gr.top = r.top + sheight/2 - 3;
	    gr.bottom = gr.top + 6;
	} else {
	    gr.left = r.left + swidth/2 - 3;
	    gr.right = gr.left + 6;
	    gr.top = r.top + sheight/2 - 5;
	    gr.bottom = gr.top + 10;
	}

	DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 2 : 3, 4, &r, 0 );
	DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 8 : 9, 4, &gr, 0 );
    } else {
	if (controls & SubPage ) {
	    RECT r;
	    r.left = subPageR.left();
	    r.right = subPageR.right() + HORIZONTAL;
	    r.top = subPageR.top();
	    r.bottom = subPageR.bottom() + !HORIZONTAL;

	    int stateId;
	    if ( SubPage == activeControl )
		stateId = 3;
	    else
		stateId = 1;

	    DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 4 : 5, stateId, &r, 0 );
	}
	if ( controls  & AddPage ) {
	    RECT r;
	    r.left = addPageR.left() - HORIZONTAL;
	    r.right = addPageR.right();
	    r.top = addPageR.top() - !HORIZONTAL;
	    r.bottom = addPageR.bottom();

	    int stateId;
	    if ( AddPage == activeControl )
		stateId = 3;
	    else
		stateId = 1;

	    DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 4 : 5, stateId, &r, 0 );
	}
	if ( controls & Slider ) {
	    if ( !maxedOut ) {
		RECT r;
		r.left = sliderR.left();
		r.right = sliderR.right();
		r.top = sliderR.top();
		r.bottom = sliderR.bottom();

		int stateId;
		if ( activeControl == Slider )
		    stateId = 3;
		else if ( d->hotWidget == (QWidget*)sb && sliderR.contains( d->hotSpot ) )
		    stateId = 2;
		else
		    stateId = 1;

		const int swidth = r.right - r.left;
		const int sheight = r.bottom - r.top;

		RECT gr;
		if ( HORIZONTAL ) {
		    gr.left = r.left + swidth/2 - 5;
		    gr.right = gr.left + 10;
		    gr.top = r.top + sheight/2 - 3;
		    gr.bottom = gr.top + 6;
		} else {
		    gr.left = r.left + swidth/2 - 3;
		    gr.right = gr.left + 6;
		    gr.top = r.top + sheight/2 - 5;
		    gr.bottom = gr.top + 10;
		}

		DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 2 : 3, stateId, &r, 0 );
		DrawThemeBackground( htheme, p->handle(), HORIZONTAL ? 8 : 9, stateId, &gr, 0 );
	    }
	}
    }
    // ### perhaps this should not be able to accept focus if maxedOut?
    if ( sb->hasFocus() && (controls & Slider) )
	drawFocusRect(p, QRect(sliderR.x()+2, sliderR.y()+2,
			       sliderR.width()-5, sliderR.height()-5), g,
		      &sb->backgroundColor());

    CloseThemeData( htheme );
}

// Slider
int QWindowsXPStyle::sliderLength() const
{
    return 19;
}

int QWindowsXPStyle::sliderThickness() const
{
    return 19;
}

void QWindowsXPStyle::drawSlider( QPainter *p,
			int x, int y, int w, int h,
			const QColorGroup &g,
			Orientation orientation, bool tickAbove, bool tickBelow)
{
    HTHEME htheme = Private::getThemeData( L"TRACKBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawSlider( p, x, y, w, h, g, orientation, tickAbove, tickBelow );
	return;
    }

    Q_RECT

    int statusId;
    int partId = 3;
/*    if ( disabled )
	statusId = 5;
    else if ( down )
	statusId = 3;
    else */if ( d->hotWidget == p->device() )
	statusId = 2;
    else
	statusId = 1;

    if ( orientation == Horizontal ) {
	if ( tickBelow )
	    r.bottom += 6;
	if ( tickAbove )
	    r.top -= 3;
    } else {
	if ( tickBelow )
	    r.right += 6;
	if ( tickAbove )
	    r.left -= 3;
    }

    if ( orientation == Vertical || !tickBelow)
	partId = 4;

    DrawThemeBackground( htheme, p->handle(), partId, statusId, &r, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawSliderGroove( QPainter *p,
			int x, int y, int w, int h,
			const QColorGroup& g, QCOORD c,
			Orientation orientation )
{
    HTHEME htheme = Private::getThemeData( L"TRACKBAR" );
    if ( !htheme ) {
	QWindowsStyle::drawSliderGroove( p, x, y, w, h, g, c, orientation );
	return;
    }

    RECT r;
    if ( orientation == Horizontal ) {
	r.left = x;
	r.right = x+w;
	r.top = y + 6;
	r.bottom = y+h - 6;
    } else {
	r.left = x + 6;
	r.right = x+w - 6;
	r.top = y;
	r.bottom = y+h;
    }

    DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    CloseThemeData( htheme );
}

// Splitter
void QWindowsXPStyle::drawSplitter( QPainter *p,
		    int x, int y, int w, int h,
		    const QColorGroup &g,
		    Orientation orientation)
{
    QWindowsStyle::drawSplitter( p, x, y, w, h, g, orientation );
}

// Popup Menu
void QWindowsXPStyle::drawCheckMark( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, bool act, bool dis )
{
    QPointArray a( 7*2 );
    int i, xx, yy;
    xx = x+3;
    yy = y+5;
    for ( i=0; i<3; i++ ) {
	a.setPoint( 2*i,   xx, yy );
	a.setPoint( 2*i+1, xx, yy+2 );
	xx++; yy++;
    }
    yy -= 2;
    for ( i=3; i<7; i++ ) {
	a.setPoint( 2*i,   xx, yy );
	a.setPoint( 2*i+1, xx, yy+2 );
	xx++; yy--;
    }
    p->setPen( g.text() );
    p->drawLineSegments( a );
}

void QWindowsXPStyle::drawPopupMenuItem( QPainter* p, bool checkable,
		    int maxpmw, int tab, QMenuItem* mi,
		    const QPalette& pal,
		    bool act, bool enabled,
		    int x, int y, int w, int h)
{
    HTHEME htheme = Private::getThemeData( L"" );
    if ( !htheme ) {
	QWindowsStyle::drawPopupMenuItem( p, checkable, maxpmw, tab, mi, pal, act, enabled, x, y, w, h );
	return;
    }

    CloseThemeData( htheme );
}

// MenuBar
void QWindowsXPStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
		    QMenuItem* mi, QColorGroup& g, bool active,
		    bool down, bool hasFocus )
{
    // most whister apps use some theme magic here.
    QRect r( x, y, w, h );
    p->fillRect( r, g.brush( QColorGroup::Button ) );

    if ( active && hasFocus )
	p->fillRect( r, g.highlight() );

    if ( active )
	drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
		g, mi->isEnabled(), mi->pixmap(), mi->text(), -1, &g.highlightedText() );
    else
	drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
		g, mi->isEnabled(), mi->pixmap(), mi->text(), -1, &g.buttonText() );
}

void QWindowsXPStyle::drawMenuBarPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &g, const QBrush *fill )
{
}

// TitleBar
/*
void QWindowsXPStyle::drawTitleBar( QPainter *p, int x, int y, int w, int h,
				   const QColor &left, const QColor &right,
				   bool active )
{
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawTitleBar( p, x, y, w, h, left, right, active );
	return;
    }

    Q_RECT

    if ( h < 15 )
	DrawThemeBackground( htheme, p->handle(), 2, active ? 1 : 2, &r, 0 );
    else
	DrawThemeBackground( htheme, p->handle(), 6, active ? 1 : 2, &r, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawTitleBarLabel( QPainter *p, int x, int y, int w, int h,
					const QString &text,
					const QColor &tc, bool active )
{
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawTitleBarLabel( p, x, y, w, h, text, tc, active );
	return;
    }

    Q_RECT

    DrawThemeText( htheme, p->handle(), 6, active ? 1 : 2,
	(TCHAR*)qt_winTchar(text, FALSE), text.length(), DT_LEFT | DT_BOTTOM | DT_SINGLELINE, 0, &r );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawTitleBarButton( QPainter *p, int x, int y, int w, int h,
					 const QColorGroup &g, bool down )
{
}

void QWindowsXPStyle::drawTitleBarButtonLabel( QPainter *p, int x, int y, int w, int h,
					      const QPixmap *pm, int button, bool down )
{
    HTHEME htheme = Private::getThemeData( L"WINDOW" );
    if ( !htheme ) {
	QWindowsStyle::drawTitleBarButtonLabel( p, x, y, w, h, pm, button, down );
	return;
    }

    Q_RECT

    int stateId = down ? 3 : ( p->device() == d->hotWidget ? 2 : 1 );

    int partId = 0;
    switch ( button ) {
    case 1: // min
	partId = 13;
	break;
    case 2: // max
	partId = 14;
	break;
    case 3: // close
	partId = 16;
	break;
    case 4: // restore
    case 5:
	partId = 19;
	break;
    case 6: // context help
	partId = 22;
	break;
    case 7: // shade
    case 8:
    default:
	QWindowsStyle::drawTitleBarButtonLabel( p, x, y, w, h, pm, button, down );
	break;
    }
    if ( partId )
	DrawThemeBackground( htheme, p->handle(), partId, stateId, &r, 0 );

    CloseThemeData( htheme );
}
*/
// Header
void QWindowsXPStyle::drawHeaderSection( QPainter *p, int x, int y, int w, int h,
					const QColorGroup &g, bool down )
{
    HTHEME htheme = Private::getThemeData( L"HEADER" );
    if ( !htheme ) {
	QWindowsStyle::drawHeaderSection( p, x, y, w, h, g, down );
	return;
    }

    Q_RECT

    int stateId;
    if ( down )
	stateId = 3;
    else if ( QRect( x, y, w, h ) == d->hotHeader )
	stateId = 2;
    else
	stateId = 1;

    DrawThemeBackground( htheme, p->handle(), 1, stateId, &r, 0 );

    CloseThemeData( htheme );
}

// SpinBox
int QWindowsXPStyle::spinBoxFrameWidth() const
{
    return 0;
}

// range control widget
void QWindowsXPStyle::drawRangeControlWidgetButton( QPainter *p, int x, int y, int w, int h,
						    const QColorGroup &g, QRangeControlWidget* rc,
						    bool downbtn, bool enabled, bool down )
{
    if ( !d->use_xp )
	QWindowsStyle::drawRangeControlWidgetButton( p, x, y, w, h, g, rc, downbtn, enabled, down );
}

void QWindowsXPStyle::drawRangeControlWidgetSymbol( QPainter *p, int x, int y, int w, int h,
						    const QColorGroup &g, QRangeControlWidget* rc,
						    bool downbtn, bool enabled, bool down )
{
    HTHEME htheme = Private::getThemeData( L"SPIN" );
    if ( !htheme ) {
	QWindowsStyle::drawRangeControlWidgetSymbol( p, x, y, w, h, g, rc, downbtn, enabled, down );
	return;
    }

    Q_RECT

    int stateId;
    if ( !enabled )
	stateId = 4;
    else if ( down )
	stateId = 3;
    else if ( d->hotWidget == (QWidget*)rc && QRect( x, y, w, h ).contains( d->hotSpot ) )
	stateId = 2;
    else
	stateId = 1;

    DrawThemeBackground( htheme, p->handle(), downbtn ? 2 : 1, stateId, &r, 0 );

    CloseThemeData( htheme );
}

// GroupBox
void QWindowsXPStyle::drawGroupBoxTitle( QPainter *p, int x, int y, int w, int h,
					const QColorGroup &g, const QString &text, bool enabled )
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawGroupBoxTitle( p, x, y, w, h, g, text, enabled );
	return;
    }

    Q_RECT

    DrawThemeText( htheme, p->handle(), 4, 1, (TCHAR*)qt_winTchar( text, FALSE ), text.length(), DT_CENTER | DT_SINGLELINE | DT_VCENTER, 0, &r );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawGroupBoxFrame( QPainter *p, int x, int y, int w, int h,
					const QColorGroup &g, const QGroupBox *gb )
{
    HTHEME htheme = Private::getThemeData( L"BUTTON" );
    if ( !htheme ) {
	QWindowsStyle::drawGroupBoxFrame( p, x, y, w, h, g, gb );
	return;
    }

    Q_RECT

    DrawThemeBackground( htheme, p->handle(), 4, 1, &r, 0 );

    CloseThemeData( htheme );
}

// statusbar
void QWindowsXPStyle::drawStatusBarSection( QPainter *p, int x, int y, int w, int h,
					   const QColorGroup &g, bool permanent )
{
    HTHEME htheme = Private::getThemeData( L"STATUS" );
    if ( !htheme ) {
	QWindowsStyle::drawStatusBarSection( p, x, y, w, h, g, permanent );
	return;
    }

    Q_RECT

    DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    CloseThemeData( htheme );
}

void QWindowsXPStyle::drawSizeGrip( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    HTHEME htheme = Private::getThemeData( L"STATUS" );
    if ( !htheme ) {
	QWindowsStyle::drawSizeGrip( p, x, y, w, h, g );
	return;
    }

    Q_RECT

    DrawThemeBackground( htheme, p->handle(), 2, 1, &r, 0 );

    CloseThemeData( htheme );
}

// progressbar
/*!
 \reimp
 */
int QWindowsXPStyle::progressChunkWidth() const
{
    return 9;
}

/*!
  \reimp
*/
void QWindowsXPStyle::drawProgressBar( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    HTHEME htheme = Private::getThemeData( L"PROGRESS" );
    if ( !htheme ) {
	QWindowsStyle::drawProgressBar( p, x, y, w, h, g );
	return;
    }

    Q_RECT

    DrawThemeBackground( htheme, p->handle(), 1, 1, &r, 0 );

    CloseThemeData( htheme );
}

/*!
 \reimp
 */
void QWindowsXPStyle::drawProgressChunk( QPainter *p, int x, int y, int w, int h, const QColorGroup &g )
{
    HTHEME htheme = Private::getThemeData( L"PROGRESS" );
    if ( !htheme ) {
	QWindowsStyle::drawProgressChunk( p, x, y, w, h, g );
	return;
    }

    RECT r;
    r.left = x + 1;
    r.right = x+w-1;
    r.top = y + 1;
    r.bottom = y+h-1;

    DrawThemeBackground( htheme, p->handle(), 3, 1, &r, 0 );

    CloseThemeData( htheme );
}

// HotSpot magic
bool QWindowsXPStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( o && o->isWidgetType() ) {
	switch ( e->type() ) {
	case QEvent::MouseMove:
	    {
		if ( d->hotWidget != o )
		    break;
		QMouseEvent *me = (QMouseEvent*)e;
		d->hotSpot = me->pos();
		if ( o->inherits( "QTabBar" ) ) {
		    QTabBar* bar = (QTabBar*)o;
		    QTab * t = bar->selectTab( me->pos() );
		    if ( d->hotTab != t ) {
			if ( d->hotTab )
			    d->hotWidget->update( d->hotTab->rect() );
			d->hotTab = t;
			if ( d->hotTab )
			    d->hotWidget->update( d->hotTab->rect() );
		    }
		} else if ( o->inherits( "QHeader" ) ) {
		    QHeader *header = (QHeader*)o;
		    QRect oldHeader = d->hotHeader;

		    if ( header->orientation() == Horizontal )
			d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.x() ) );
		    else
			d->hotHeader = header->sectionRect( header->sectionAt( d->hotSpot.y() ) );

		    if ( oldHeader != d->hotHeader ) {
			if ( oldHeader.isValid() )
			    header->update( oldHeader );
			if ( d->hotHeader.isValid() )
			    header->update( d->hotHeader );
		    }
		} else if ( o->inherits( "QSpinBox" ) ) {
		    QSpinBox *spinbox = (QSpinBox*)o;
		    QRect rect = spinbox->downRect().unite( spinbox->upRect() );
		    spinbox->update( rect );
		}
		else if ( o->inherits( "QScrollBar" ) ) {
		    repaintByMouseMove = TRUE;
		    ((QScrollBar*)o)->repaint( FALSE );
		    repaintByMouseMove = FALSE;
		}
	    }
	    break;
	case QEvent::Enter:
	    {
		// since the double buffer pixmap of some widgets is palette-key based
		// we have to change the palette to force a redraw
		if ( d->hotWidget && d->hotWidget->inherits( "QButton" ) ) {
		    QWidget *oldHot = d->hotWidget;
		    d->hotWidget = 0;
		    oldHot->setPalette( d->oldPalette );
		    oldHot->update();
		}
		d->hotWidget = (QWidget*)o;
		d->hotSpot = d->hotWidget->mapFromGlobal( QCursor::pos() );

		if ( d->hotWidget->inherits( "QButton" ) ) {
		    d->oldPalette = d->hotWidget->palette();
		    QPalette newPal = d->oldPalette;
		    newPal.setColor( QColorGroup::BrightText, QColor() );
		    d->hotWidget->setPalette( newPal );
		} else {
		    d->hotWidget->update();
		}
	    }
	    break;
	case QEvent::Leave:
	    if ( o != d->hotWidget )
		break;

	    QPoint curPos = QCursor::pos();
	    d->hotSpot = d->hotWidget->mapFromGlobal( curPos );
	    if ( QApplication::widgetAt( curPos, TRUE ) == d->hotWidget )
		break;

	    QWidget *oldHot = d->hotWidget;
	    QTab *oldTab = d->hotTab;
	    QRect oldHeader = d->hotHeader;
	    d->hotWidget = 0;
	    d->hotTab = 0;
	    d->hotHeader = QRect();

	    if ( !oldHot )
		break;
	    if ( oldHot->inherits( "QButton" ) ) {
		oldHot->setPalette( d->oldPalette );
	    } else if ( oldHot->inherits( "QTabBar" ) ) {
		if ( oldTab )
		    oldHot->update( oldTab->rect() );
	    } else if ( oldHot->inherits( "QHeader" ) ) {
		if ( oldHeader.isValid() )
		    oldHot->update( oldHeader );
	    } else if ( oldHot->inherits( "QSpinBox" ) ) {
		QSpinBox *spinbox = (QSpinBox*)oldHot;
		spinbox->update( spinbox->downRect().unite( spinbox->upRect() ) );
	    } else {
		oldHot->update();
	    }
	    break;
	}
    }

    return QWindowsStyle::eventFilter( o, e );
}

#endif //QT_NO_STYLE_WINDOWSXP
