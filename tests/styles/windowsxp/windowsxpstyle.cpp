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
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <private/qtitlebar_p.h>
#include <qlistview.h>
#include <qcleanuphandler.h>
#include <qbitmap.h>

#include <qt_windows.h>
#include <uxtheme.h>
#include <tmschema.h>

static ulong ref = 0;
static bool use_xp  = FALSE;
static bool init_xp = FALSE;

class QWindowsXPStylePrivate
{
public:
    QWindowsXPStylePrivate()
	: hotWidget( 0 ), hotTab( 0 ), hotSpot( -1, -1 )
    {
	init();
    }
    ~QWindowsXPStylePrivate()
    {
	cleanup();
    }

    void init()
    {
	if ( qWinVersion() != Qt::WV_XP ) {
	    use_xp = FALSE;
	    init_xp = TRUE;
	}

        if ( !init_xp ) {
	    init_xp = TRUE;
	    use_xp = IsThemeActive() && IsAppThemed();
	}
	if ( use_xp )
	    ref++;
    }

    void cleanup()
    {
	init_xp = FALSE;
	if ( use_xp ) {
	    if ( !--ref ) {		
		use_xp  = FALSE;
		delete limboWidget;
		limboWidget = 0;
		delete tabbody;
		tabbody = 0;
	    }
	}
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

private:
    static QWidget *limboWidget;
    static QPixmap *tabbody;
};

const QWidget *QWindowsXPStylePrivate::currentWidget = 0;
QWidget *QWindowsXPStylePrivate::limboWidget = 0;
QPixmap *QWindowsXPStylePrivate::tabbody = 0;

struct XPThemeData
{
    XPThemeData( const QWidget *w = 0, const QPainter *p = 0, LPCWSTR theme = 0, int part = 0, int state = 0, const QRect &r = QRect() )
        : widget( w ), painter( p ), name( theme ),partId( part ), stateId( state ), rec( r ), htheme( 0 )
    {
    }
    ~XPThemeData()
    {
        if ( htheme )
            CloseThemeData( htheme );
    }
    
    HTHEME handle()
    {
	if ( !use_xp )
	    return NULL;
	
        if ( !htheme )
            htheme = OpenThemeData( QWindowsXPStylePrivate::winId( widget ), name ); 
	
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
    
    void operator=( const XPThemeData &orig )
    {
        if ( htheme )
            CloseThemeData( htheme );
	
        name = orig.name;
        partId = orig.partId;
        stateId = orig.stateId;
        rec = orig.rec;
        htheme = orig.htheme;
    }

    HRGN mask()
    {
	if ( IsThemeBackgroundPartiallyTransparent( handle(), partId, stateId ) ) {
	    HRGN hrgn;
	    GetThemeBackgroundRegion( handle(), painter ? painter->handle() : 0, partId, stateId, &rect(), &hrgn );
	    return hrgn;
	}
	return 0;
    }

    void setTransparency()
    {
	HRGN hrgn = mask();
	if ( hrgn )
	    SetWindowRgn( QWindowsXPStylePrivate::winId( widget ), hrgn, FALSE );
    }

    void drawBackground( int pId = 0, int sId = 0 )
    {
	if ( pId )
	    partId = pId;
	if ( sId )
	    stateId = sId;

	ulong res = DrawThemeBackground( handle(), painter->handle(), partId, stateId, &rect(), 0 );
    }

    int partId;
    int stateId;
    QRect rec;    

private:
    const QWidget *widget;
    const QPainter *painter;
    LPCWSTR name;
    HTHEME htheme;
};

const QPixmap *QWindowsXPStylePrivate::tabBody( QWidget *widget )
{
    if ( !tabbody ) {
	tabbody = new QPixmap( 1, 1 );
	QPainter painter( tabbody );
	XPThemeData theme( widget, &painter, L"TAB", TABP_BODY, 0 );
	SIZE sz;
	GetThemePartSize( theme.handle(), painter.handle(), TABP_BODY, 0, 0, TS_TRUE, &sz );
	painter.end();
	tabbody->resize( sz.cx, sz.cy );
	painter.begin( tabbody );
	theme.rec = QRect( 0, 0, sz.cx, sz.cy );
	theme.drawBackground();
	painter.end();
    }
    return tabbody;
}


QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new QWindowsXPStylePrivate;
}

QWindowsXPStyle::~QWindowsXPStyle()
{
    delete d;
}

void QWindowsXPStyle::unPolish( QApplication *app )
{
    d->cleanup();
    QWindowsStyle::unPolish( app );
}

void QWindowsXPStyle::polish( QApplication *app )
{
    d->init();
    QWindowsStyle::polish( app );
}

void QWindowsXPStyle::polish( QWidget *widget )
{
    if ( !use_xp ) {
	QWindowsStyle::polish( widget );
	return;
    }

    if ( widget->inherits( "QButton" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QTabBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
	connect( widget, SIGNAL(selected(int)), this, SLOT(tabBarSelectionChanged()) );
    } else if ( widget->inherits( "QHeader" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QComboBox" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QSpinWidget" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QScrollBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QTitleBar" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	widget->installEventFilter( this );
    } else if ( widget->inherits( "QSlider" ) ) {
	widget->installEventFilter( this );
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QWidgetStack" ) &&
		widget->parentWidget() &&
		widget->parentWidget()->inherits( "QTabWidget" ) ) {
	widget->setPaletteBackgroundPixmap( *d->tabBody( widget ) );
    }

    updateRegion( widget );
    QWindowsStyle::polish( widget );
}

void QWindowsXPStyle::unPolish( QWidget *widget )
{
    widget->removeEventFilter( this );
    if ( widget->inherits( "QTitleBar" ) && !widget->inherits( "QDockWindowTitleBar" ) ) {
	SetWindowRgn( widget->winId(), 0, TRUE );
	if ( widget->isMinimized() ) {
	    SetWindowRgn( widget->parentWidget()->winId(), 0, TRUE );
	}
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	SetWindowRgn( widget->winId(), 0, TRUE );
    }
    QWindowsStyle::unPolish( widget );
}

void QWindowsXPStyle::updateRegion( QWidget *widget )
{
    if ( !use_xp )
	return;

    if ( widget->inherits( "QTitleBar" ) && !widget->inherits( "QDockWindowTitleBar" ) ) {
	if ( widget->isMinimized() ) {
	    XPThemeData theme( widget, 0, L"WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect() );
	    theme.setTransparency();
	    XPThemeData theme2( widget->parentWidget(), 0, L"WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect() );
	    theme2.setTransparency();
	} else {
	    XPThemeData theme( widget, 0, L"WINDOW", WP_CAPTION, CS_ACTIVE, widget->rect() );
	    theme.setTransparency();
	}
    } else if ( widget->inherits( "QWorkspaceChild" ) ) {
	XPThemeData theme( widget, 0, L"WINDOW", WP_CAPTION, CS_ACTIVE, widget->rect() );
	theme.setTransparency();
	theme.rec = widget->parentWidget()->rect();
	RECT r = theme.rect();
	InvalidateRect( widget->parentWidget()->winId(), &r, TRUE );
    }
}

void QWindowsXPStyle::drawPrimitive( PrimitiveElement op,
				    QPainter *p,
				    const QRect &r,
				    const QColorGroup &cg,
				    SFlags flags,
				    const QStyleOption &opt ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawPrimitive( op, p, r, cg, flags, opt );
	return;
    }
    
    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;
    QRect rect = r;
    
    switch ( op ) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
	name = L"BUTTON";
	partId = BP_PUSHBUTTON;
	if ( !flags & Style_Enabled )
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
	name = L"TOOLBAR";
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
	name = L"TOOLBAR";
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
	name = L"BUTTON";
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
	name = L"BUTTON";
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

    case PE_Panel:
	break;

    case PE_PanelLineEdit:
	name = L"EDIT";
	partId = EP_EDITTEXT;
	if ( !(flags & Style_Enabled) )
	    stateId = ETS_DISABLED;
	else
	    stateId = ETS_NORMAL;
	break;

    case PE_PanelTabWidget:
	name = L"TAB";
	partId = TABP_PANE;
	break;

    case PE_PanelPopup:
	p->save();
	p->setPen( cg.dark() );
	p->drawRect( r );
	p->restore();
	return;

    case PE_PanelMenuBar:
	break;

    case PE_PanelDockWindow:
	name = L"REBAR";
	partId = RP_BAND;
	stateId = 1;
	break;
	
    case PE_HeaderSection:
	name = L"HEADER";
	partId = HP_HEADERITEM;
	if ( flags & Style_Down )
	    stateId = HIS_PRESSED;
	else if ( r == d->hotHeader )
	    stateId = HIS_HOT;
	else
	    stateId = HIS_NORMAL;
	break;

    case PE_HeaderArrow:
	name = L"HEADER";
	partId = HP_HEADERSORTARROW;
	if ( flags & Style_Down )
	    stateId = HSAS_SORTEDDOWN;
	else
	    stateId = HSAS_SORTEDUP;
	break;
	
    case PE_StatusBarSection:
	name = L"STATUS";
	partId = SP_PANE;
	break;
	
    case PE_GroupBoxFrame:
	name = L"BUTTON";
	partId = BP_GROUPBOX;
	if ( !(flags & Style_Enabled) )
	    stateId = GBS_DISABLED;
	else
	    stateId = GBS_NORMAL;
	break;

    case PE_SizeGrip:
	name = L"STATUS";
	partId = SP_GRIPPER;
	break;

    case PE_ScrollBarAddLine:
	name = L"SCROLLBAR";
	break;

    case PE_ScrollBarSubLine:
	name = L"SCROLLBAR";
	break;

    case PE_ScrollBarAddPage:
	name = L"SCROLLBAR";
	break;

    case PE_ScrollBarSubPage:
	name = L"SCROLLBAR";
	break;

    case PE_ScrollBarSlider:
	name = L"SCROLLBAR";
	break;

    case PE_ScrollBarFirst:
	name = L"SCROLLBAR";
	break;
	
    case PE_ScrollBarLast:
	name = L"SCROLLBAR";
	break;
	
    case PE_ProgressBarChunk:
	name = L"PROGRESS";
	partId = PP_CHUNK;
	stateId = 1;
	rect = QRect( r.x(), r.y() + 3, r.width(), r.height() - 5 );
	break;

    case PE_DockWindowSeparator:
	name = L"TOOLBAR";
	if ( flags & Style_Horizontal )
	    partId = TP_SEPARATOR;
	else
	    partId = TP_SEPARATORVERT;
	break;

    case PE_WindowFrame:
	{
	    name = L"WINDOW";
	    if ( flags & Style_Active )
		stateId = FS_ACTIVE;
	    else
		stateId = FS_INACTIVE;

	    int fwidth = 2;
	    if ( !opt.isDefault() )
		fwidth = opt.lineWidth() + opt.midLineWidth();

	    XPThemeData theme( 0, p, name, 0, stateId );
	    if ( !theme.isValid() )
		break;

	    theme.rec = QRect( r.x(), r.y()+fwidth, r.x()+fwidth, r.height()-fwidth );
	    theme.partId = WP_FRAMELEFT;
	    theme.drawBackground();
	    theme.rec = QRect( r.x(), r.height()-fwidth, r.width(), r.height() );
	    theme.partId = WP_FRAMEBOTTOM;
	    theme.drawBackground();
	    theme.rec = QRect( r.width()-fwidth, r.y()+fwidth, r.width(), r.height()-fwidth );
	    theme.partId = WP_FRAMERIGHT;
	    theme.drawBackground();
	    theme.rec = QRect( r.x()-5, r.y()-5, r.width()+10, r.y()+fwidth+5 );
	    theme.partId = WP_CAPTION;
	    theme.drawBackground();
    
	    return;
	}

    default:
	break;
    }
    
    XPThemeData theme( 0, p, name, partId, stateId, rect );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawPrimitive( op, p, r, cg, flags, opt );
	return;
    }
    theme.drawBackground();
}

void QWindowsXPStyle::drawControl( ControlElement element,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  const QColorGroup &cg,
				  SFlags flags,
				  const QStyleOption &opt ) const
{
    d->currentWidget = widget;

    if ( !use_xp ) {
	QWindowsStyle::drawControl( element, p, widget, r, cg, flags, opt );
	return;
    }
    
    QRect rect(r);
    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;
    if ( widget->hasMouse() )
	flags |= Style_MouseOver;
   
    switch ( element ) {
    case CE_PushButton:
	//    case CE_PushButtonLabel:
	{
	    name = L"BUTTON";
	    partId = BP_PUSHBUTTON;
	    QPushButton *pb = (QPushButton*)widget;
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
	}
	break;

    case CE_CheckBox:
	//    case CE_CheckBoxLabel:
	drawPrimitive( PE_Indicator, p, subRect( SR_CheckBoxIndicator, widget ), cg, flags, opt );
	return;

    case CE_RadioButton:
	//    case CE_RadioButtonLabel:
	drawPrimitive( PE_ExclusiveIndicator, p, subRect( SR_RadioButtonIndicator, widget ), cg, flags, opt );
	return;

    case CE_TabBarTab:
	//    case CE_TabBarLabel:
	name = L"TAB";
	{
	    QTabBar *bar = (QTabBar*)widget;
	    QTab *t = opt.tab();
	    if ( bar->indexOf(t->identifier() == 0 ) )
		partId = TABP_TABITEMLEFTEDGE;
	    else if ( bar->indexOf(t->identifier() == bar->count() ) )
		partId = TABP_TABITEMRIGHTEDGE;
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
	    if ( !(flags & Style_Selected) || (flags & Style_HasFocus) )
		rect.addCoords( 0, 2, 0, 0 );
	    else
		rect.addCoords( 0, 1, 0, 0 );
	}
	break;

    case CE_ProgressBarGroove:
	name = L"PROGRESS";
	partId = PP_BAR;
	stateId = 1;
	break;

    case CE_MenuBarItem:
	{
	    if (opt.isDefault())
		break;

	    QMenuItem *mi = opt.menuItem();
	    if (flags & Style_Active) {
		p->fillRect(r, cg.brush( QColorGroup::Highlight) );
		QMenuItem *mi = opt.menuItem();
		drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
			 flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
			 &cg.highlightedText());
	    } else {
		p->fillRect(r, cg.brush( QColorGroup::Button) );
		drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine, cg,
			 flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
			 &cg.buttonText());
	    }
	}
	return;

    default:
	break;
    }
    
    XPThemeData theme( widget, p, name, partId, stateId, rect );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawControl( element, p, widget, rect, cg, flags, opt );
	return;
    }
    
    theme.drawBackground();

    d->currentWidget = 0;
}

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

    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;
    switch ( element ) {
    case CE_PushButton:
	//    case CE_PushButtonLabel:
	name = L"BUTTON";
	partId = BP_PUSHBUTTON;
	break;

    case CE_RadioButton:
	name = L"BUTTON";
	partId = BP_RADIOBUTTON;
	break;

    case CE_CheckBox:
	name = L"BUTTON";
	partId = BP_CHECKBOX;
	break;

    default:
	break;
    }

    XPThemeData theme( widget, p, name, partId, stateId, r );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawControlMask( element, p, widget, r, option );
	return;
    }

    HRGN rgn = theme.mask();
    if ( rgn ) {
	p->save();
	p->setBrush( color1 );
	PaintRgn( p->handle(), rgn );
	p->restore();
    }
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

void QWindowsXPStyle::drawComplexControl( ComplexControl control,
					 QPainter* p,
					 const QWidget* w,
					 const QRect& r,
					 const QColorGroup& cg,
					 SFlags flags,
					 SCFlags sub,
					 SCFlags subActive,
					 const QStyleOption &opt ) const
{
    d->currentWidget = w;

    if ( !use_xp ) {
	QWindowsStyle::drawComplexControl( control, p, w, r, cg, flags, sub, subActive, opt );
	return;
    }
    
    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;
    if ( w->hasMouse() )
	flags |= Style_MouseOver;
    
    switch ( control ) {
    case CC_SpinWidget:
        {
	    QSpinWidget *spin = (QSpinWidget*)w;
	    XPThemeData theme( w, p, L"SPIN" );

	    if ( sub & SC_SpinWidgetFrame ) {
		partId = EP_EDITTEXT;
		if ( (!flags & Style_Enabled) )
		    stateId = ETS_DISABLED;
		else if ( flags & Style_HasFocus )
		    stateId = ETS_FOCUSED;
		else 
		    stateId = ETS_NORMAL;

		XPThemeData ftheme( w, p, L"EDIT", partId, stateId, r );
		ftheme.drawBackground();
	    }
	    if ( sub & SC_SpinWidgetUp ) {
		theme.rec = querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetUp, opt );
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
		theme.rec = querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetDown, opt );
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
		XPThemeData theme( w, p, L"EDIT", partId, stateId, r );

		theme.drawBackground();
	    }

	    if ( sub & SC_ComboBoxArrow ) {
		XPThemeData theme( w, p, L"COMBOBOX" );
		theme.rec = querySubControlMetrics( CC_ComboBox, w, SC_ComboBoxArrow, opt );
		partId = CP_DROPDOWNBUTTON;
		QComboBox *cb = (QComboBox*)w;
		if ( cb->listBox() && cb->listBox()->isVisible() )
		    subActive = SC_ComboBoxArrow;
    
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
	    XPThemeData theme( w, p, L"SCROLLBAR" );
	    QScrollBar *bar = (QScrollBar*)w;
	    bool maxedOut = ( bar->maxValue() == bar->minValue() );
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
		if ( sub & SC_ScrollBarGroove ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarGroove );
		}
	    }
        }
        break;
    case CC_Slider:
	{
	    XPThemeData theme( w, p, L"TRACKBAR" );
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
			p->fillRect( 0, 0, sl->width(), aboveend, cg.brush( QColorGroup::Background ) );
		    if ( ticks & QSlider::Below )
			p->fillRect( 0, belowstart, sl->width(), belowend, cg.brush( QColorGroup::Background ) );

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
			p->fillRect( 0, 0, aboveend, sl->height(), cg.brush( QColorGroup::Background ) );
		    if ( ticks & QSlider::Right )
			p->fillRect( belowstart, 0, belowend, sl->height(), cg.brush( QColorGroup::Background ) );

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
	    }
	    if ( sub & SC_SliderHandle ) {
		theme.rec = querySubControlMetrics( CC_Slider, w, SC_SliderHandle, opt );
		p->fillRect( theme.rec, cg.brush( QColorGroup::Background ) );
		p->setClipping( FALSE );
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
		    else if ( w->hasMouse() && theme.rec.contains( d->hotSpot ) )
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
		    else if ( w->hasMouse() && theme.rec.contains( d->hotSpot ) )
			stateId = TUS_HOT;
		    else
			stateId = TUVS_NORMAL;
		}
		theme.drawBackground( partId, stateId );
	    }
	}
	break;
	
    case CC_ToolButton:
	{
	    XPThemeData theme( w, p, L"TOOLBAR" );
	    QToolButton *tb = (QToolButton*)w;
	
	    SFlags bflags = flags,
		   mflags = flags;

	    if (subActive == SC_ToolButton)
		bflags |= Style_Down;
	    else if (subActive == SC_ToolButtonMenu)
		mflags |= Style_Down;
	  
	    if ( sub & SC_ToolButton ) {
		theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButton, opt );
		if (bflags & (Style_Down | Style_On | Style_Raised)) {
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
			if ( !w->parentWidget() || !w->parentWidget()->inherits( "QToolBar" ) )
			    drawPrimitive( PE_ButtonBevel, p, theme.rec, cg, bflags, opt );
			else
			    drawPrimitive( PE_ButtonTool, p, theme.rec, cg, bflags, opt );
		    }
		} else if ( tb->parentWidget() &&
			  tb->parentWidget()->backgroundPixmap() &&
			  !tb->parentWidget()->backgroundPixmap()->isNull() ) {
		    QPixmap pixmap = *(tb->parentWidget()->backgroundPixmap());

		    p->drawTiledPixmap( r, pixmap, tb->pos() );
		}
	    }
	    if ( sub & SC_ToolButtonMenu ) {
		theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButtonMenu, opt );
		if (mflags & (Style_Down | Style_On | Style_Raised))
		    drawPrimitive(PE_ButtonDropDown, p, theme.rec, cg, mflags, opt);
		else
		    drawPrimitive( PE_ArrowDown, p, theme.rec, cg, mflags, opt );
	    }

	    if ( tb->hasFocus() && !tb->focusProxy() ) {
		QRect fr = tb->rect();
		fr.addCoords(3, 3, -3, -3);
		drawPrimitive( PE_FocusRect, p, fr, cg );
	    }
	}
	break;

    case CC_TitleBar:
	{
	    const QTitleBar *titlebar = (const QTitleBar *) w;

	    XPThemeData theme( w, p, L"WINDOW" );
	    if ( sub & SC_TitleBarLabel ) {
		theme.rec = titlebar->rect();
		if ( titlebar->inherits( "QDockWindowTitleBar" ) )
		    theme.rec.addCoords( -10, -10, 10, 0 );
		partId = titlebar->testWFlags( WStyle_Tool ) ? WP_SMALLCAPTION : 
			( titlebar->window() && titlebar->window()->isMinimized() ? WP_MINCAPTION : WP_CAPTION );
		if ( !titlebar->isEnabled() )
		    stateId = CS_DISABLED;
		else if ( !titlebar->isActive() )
		    stateId = CS_INACTIVE;
		else 
		    stateId = CS_ACTIVE;

		theme.drawBackground( partId, stateId );

		QRect ir = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, SC_TitleBarLabel ), w );
		QColorGroup cgroup = titlebar->isActive() || !titlebar->window() ?
		    titlebar->palette().active() : titlebar->palette().inactive();
		p->setPen( cgroup.highlightedText() );
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
		if ( titlebar->icon() )
		    drawItem( p, theme.rec, AlignCenter, titlebar->colorGroup(), TRUE, titlebar->icon(), QString::null );
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
		    qDebug( "TODO: XP Shade button" );
		}
		if ( sub & SC_TitleBarUnshadeButton ) {
		    qDebug( "TODO: XP Unshade button" );
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

    case CC_ListView:
#ifndef QT_NO_LISTVIEW
	{
	    if ( sub & SC_ListView ) {
		const QListView *lv = (const QListView*)w;
		QWindowsStyle::drawComplexControl( control, p, w, r, cg, flags, sub, subActive, opt );
		if ( !lv->showSortIndicator() )
		    break;

		int sort = opt.isDefault() ? 0 : opt.lineWidth(); //### hackydiho; use sortColumn() in 3.1
		if ( sort < 0 )
		    break;
	    }
	    if ( sub & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
		QColor listViewGray( 128, 128, 128 );
		if (opt.isDefault())
		    break;

		QListViewItem *item = opt.listViewItem(),
			     *child = item->firstChild();

		int linetop = 0, linebot = 0, y = r.y();
		// each branch needs at most two lines, ie. four end points
		int dotoffset = (item->itemPos() + item->height() - y) %2;
		QPointArray dotlines( item->childCount() * 4 );
		int c = 0;

		// skip the stuff above the exposed rectangle
		while ( child && y + child->height() <= 0 ) {
		    y += child->totalHeight();
		    child = child->nextSibling();
		}

		int bx = 6;

		XPThemeData theme( w, p, L"TREEVIEW" );

		// paint stuff in the magical area
		while ( child && y < r.height() ) {
		    linebot = y + child->height()/2;
		    if ( (child->isExpandable() || child->childCount()) &&
			 (child->height() > 0) ) {
			theme.rec = QRect( bx-4 + (int)p->translationX(), linebot-4+(int)p->translationY(), 9, 9 );
			theme.drawBackground( TVP_GLYPH, child->isOpen() ? GLPS_OPENED : GLPS_CLOSED );
			// dotlinery
			p->setPen( listViewGray );
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

		if ( child ) // there's a child, so move linebot to edge of rectangle
		    linebot = r.height();

		if ( linetop < linebot ) {
		    dotlines[c++] = QPoint( bx, linetop );
		    dotlines[c++] = QPoint( bx, linebot );
		}

		p->setPen( cg.dark() );

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
		    QApplication::flushX();
		    verticalLine->setMask( *verticalLine );
		    p.begin( horizontalLine );
		    for( i=0; i<64; i++ )
			a.setPoint( i, i*2+1, 0 );
		    p.setPen( color1 );
		    p.drawPoints( a );
		    p.end();
		    QApplication::flushX();
		    horizontalLine->setMask( *horizontalLine );
		    qlv_cleanup_bitmap.add( &verticalLine );
		    qlv_cleanup_bitmap.add( &horizontalLine );
		}

		int line; // index into dotlines
		for( line = 0; line < c; line += 2 ) {
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
	QWindowsStyle::drawComplexControl( control, p, w, r, cg, flags, sub, subActive, opt );
	break;
    }

    d->currentWidget = 0;
}

int QWindowsXPStyle::pixelMetric( PixelMetric metric,
				 const QWidget *widget ) const
{
    if ( !use_xp )
	return QWindowsStyle::pixelMetric( metric, widget );
    
    switch ( metric ) {
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
	{
	    XPThemeData theme( widget, 0, L"BUTTON", BP_CHECKBOX, CBS_UNCHECKEDNORMAL );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		if ( metric == PM_IndicatorWidth )
		    return size.cx+2;
		return size.cy+2;
	    }
	}
	break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	{
	    XPThemeData theme( widget, 0, L"BUTTON", BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		if ( metric == PM_ExclusiveIndicatorWidth )
		    return size.cx+2;
		return size.cy+2;
	    }
	}
	break;

    case PM_ProgressBarChunkWidth:
	{
	    XPThemeData theme( widget, 0, L"PROGRESS", PP_CHUNK );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cx;
	    }
	}
	break;

    case PM_ScrollBarExtent:
	{
	    XPThemeData theme( widget, 0, L"SCROLLBAR", SBP_LOWERTRACKHORZ );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cy;
	    }
	}
	break;

    case PM_ScrollBarSliderMin:
	{
	    XPThemeData theme( widget, 0, L"SCROLLBAR", SBP_THUMBBTNVERT );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cy;
	    }
	}
	break;

    case PM_SliderThickness:
	{
	    XPThemeData theme( widget, 0, L"TRACKBAR", TKP_THUMB );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cy;
	    }
	}
	break;

    case PM_MenuButtonIndicator:
	{
	    XPThemeData theme( widget, 0, L"TOOLBAR", TP_SPLITBUTTONDROPDOWN );

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return size.cy;
	    }
	}
	break;

    case PM_TabBarTabOverlap:
    	return -1;

    case PM_TabBarBaseOverlap:
	return -2;

    case PM_TabBarBaseHeight:
	return -1;

    default:
	break;	
    }

    return QWindowsStyle::pixelMetric( metric, widget );
}

QRect QWindowsXPStyle::querySubControlMetrics( ComplexControl control,
					      const QWidget *widget,
					      SubControl sc,
					      const QStyleOption &option ) const
{
    if ( !use_xp )
	return QWindowsStyle::querySubControlMetrics( control, widget, sc, option );

    switch ( control ) {
    case CC_TitleBar:
	{
	    QRect r = QWindowsStyle::querySubControlMetrics( control, widget, sc, option );
	    if ( sc == SC_TitleBarLabel )
		r.addCoords( 2, 0, -4, 0 );
	    else if ( sc == SC_TitleBarSysMenu )
		r.addCoords( 2, 0, 2, 0 );
	    else if ( sc > SC_TitleBarSysMenu )
		r.addCoords( -4, 0, -4, 0 );

	    return r;
	}
	break;

    default:
	break;
    }
    return QWindowsStyle::querySubControlMetrics( control, widget, sc, option );
}

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

    default:
	return QWindowsStyle::styleHint( stylehint, widget, opt, returnData );
    }
}

// HotSpot magic
/*! \reimp */
bool QWindowsXPStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !o->isWidgetType() || e->type() == QEvent::Paint || !use_xp)
	return QWindowsStyle::eventFilter( o, e );

    QWidget *widget = (QWidget*)o;

    switch ( e->type() ) {
    case QEvent::MouseMove:
	{
	    if ( !widget->isActiveWindow() )
		break;

	    QMouseEvent *me = (QMouseEvent*)e;

	    d->hotWidget = widget;
	    d->hotSpot = me->pos();

	    if ( o->inherits( "QTabBar" ) ) {
		QTabBar* bar = (QTabBar*)o;
		QTab * t = bar->selectTab( me->pos() );
		if ( d->hotTab != t ) {
		    d->hotTab = t;
		    widget->repaint( FALSE );
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
	    } else if ( o->inherits( "QTitleBar" ) ) {
		static SubControl clearHot = SC_TitleBarLabel;
		QTitleBar *titlebar = (QTitleBar*)o;
		SubControl sc = querySubControl( CC_TitleBar, titlebar, d->hotSpot );
		if ( sc != clearHot || clearHot != SC_TitleBarLabel ) {
		    QRect rect = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, clearHot ), titlebar );
		    titlebar->repaint( rect, FALSE );

		    clearHot = sc;
		    rect = visualRect( querySubControlMetrics( CC_TitleBar, titlebar, sc ), titlebar );
		    titlebar->repaint( rect, FALSE );
		}
	    } else if ( o->inherits( "QSlider" ) ) {
		static clearSlider = FALSE;
		QSlider *slider = (QSlider*)o;
		const QRect rect = slider->sliderRect();
		const bool inSlider = rect.contains( d->hotSpot );
		if ( ( inSlider && !clearSlider ) || ( !inSlider && clearSlider ) ) {
		    clearSlider = inSlider;
		    slider->repaint( rect, FALSE );
		}
	    } else if ( o->inherits( "QComboBox" ) ) {
		static clearCombo = FALSE;
		const QRect rect = querySubControlMetrics( CC_ComboBox, (QWidget*)o, SC_ComboBoxArrow );
		const bool inArrow = rect.contains( d->hotSpot );
		if ( ( inArrow && !clearCombo ) || ( !inArrow && clearCombo ) ) {
		    clearCombo = inArrow;
		    widget->repaint( rect, FALSE );
		}
	    } else {
		widget->repaint( FALSE );
	    }
	}
        break;

    case QEvent::MouseButtonRelease:
	if ( widget->inherits( "QTabBar" ) )
	    widget->repaint( FALSE );
	break;

    case QEvent::Enter:
	if ( !widget->isActiveWindow() )
	    break;
        d->hotWidget = widget;
        widget->repaint( FALSE );
        break;

    case QEvent::Leave:
	if ( !widget->isActiveWindow() )
	    break;
        if ( widget == d->hotWidget) {
            d->hotWidget = 0;
	    d->hotHeader = QRect();
	    d->hotTab = 0;
            widget->repaint( FALSE );
        }
        break;

    case QEvent::FocusOut:
    case QEvent::FocusIn:
	widget->repaint( FALSE );
	break;

    case QEvent::Resize:
	updateRegion( widget );
	break;

    default:
        break;
    }

    return QWindowsStyle::eventFilter( o, e );
}

void QWindowsXPStyle::tabBarSelectionChanged()
{
    const QObject *o = sender();
    if ( !o->inherits( "QTabBar" ) )
	return;
    ((QWidget*)o)->repaint();
}

#endif //QT_NO_STYLE_WINDOWSXP
