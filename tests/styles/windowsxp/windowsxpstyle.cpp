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
	: hotWidget( 0 ), hotSpot( -1, -1 ), hotTab( 0 ), lastScrollbarRect( 0, -1, 0, -1 ), lastSliderRect( 0, -1, 0, -1 )
    {
	if ( qWinVersion() != Qt::WV_XP )
	    init_xp = TRUE;

        if ( !init_xp ) {
	    init_xp = TRUE;
	    use_xp = IsThemeActive() && IsAppThemed();
	}
	if ( use_xp )
	    ref++;
    }
    ~QWindowsXPStylePrivate()
    {
	if ( use_xp ) {
	    if ( !--ref ) {
		init_xp = FALSE;
		use_xp  = FALSE;
		delete limboWidget;
		limboWidget = 0;
		hwnd = 0;
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
	if ( !hwnd ) {
	    if ( widget ) {
		hwnd = widget->winId();
	    } else {
		limboWidget = new QWidget( 0, "xp_limbo_widget" );
		hwnd = limboWidget->winId();
	    }
	}
	if ( widget )
	    return widget->winId();
	return hwnd;
    }
    
    // hot-widget stuff
   
    const QWidget *hotWidget;
    QTab *hotTab;
    QRect hotHeader;

    QPoint hotSpot;
    QRect lastScrollbarRect;
    QRect lastSliderRect;
    
private:
    static HWND hwnd;
    static QWidget *limboWidget;
};

QWidget *QWindowsXPStylePrivate::limboWidget = 0;
HWND QWindowsXPStylePrivate::hwnd = NULL;


struct XPThemeData
{
    XPThemeData( const QWidget *w = 0, LPCWSTR theme = 0, int part = 0, int state = 0, const QRect &r = QRect() )
        : widget( w ), name( theme ),partId( part ), stateId( state ), rec( r ), htheme( 0 )
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
        return use_xp && partId && stateId && handle();
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

    int partId;
    int stateId;
    QRect rec;    

private:
    const QWidget *widget;
    LPCWSTR name;
    HTHEME htheme;
};



QWindowsXPStyle::QWindowsXPStyle()
: QWindowsStyle()
{
    d = new QWindowsXPStylePrivate;
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
	widget->setMouseTracking( TRUE );
    } else if ( widget->inherits( "QSpinWidget" ) ) {
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

void QWindowsXPStyle::drawPrimitive( PrimitiveElement op,
				    QPainter *p,
				    const QRect &r,
				    const QColorGroup &cg,
				    SFlags flags,
				    const QStyleOption &data ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawPrimitive( op, p, r, cg, flags, data );
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
	    stateId  =PBS_HOT;
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
	else if ( flags & Style_Sunken )
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
        break;
	
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
	break;
	
    case PE_Panel:
    case PE_PanelPopup:
    case PE_PanelMenuBar:
    case PE_PanelDockWindow:
	{
	    XPThemeData theme( 0, L"GLOBALS", 1, 1, r );
	    DrawThemeEdge( theme.handle(), p->handle(), theme.partId, theme.stateId, &theme.rect(),
		BDR_RAISEDINNER, BF_FLAT | BF_RECT, 0 );
	    return;
	}
	
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
	
    case PE_StatusBarSection:
	name = L"STATUS";
	partId = SP_PANE;
	stateId = 1;
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
	stateId = 1;
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
	
    default:
	break;
    }
    
    XPThemeData theme( 0, name, partId, stateId, rect );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawPrimitive( op, p, r, cg, flags, data );
	return;
    }
    
    DrawThemeBackground( theme.handle(), p->handle(), theme.partId, theme.stateId, &theme.rect(), 0 );
}

void QWindowsXPStyle::drawControl( ControlElement element,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  const QColorGroup &cg,
				  SFlags flags,
				  const QStyleOption &data ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawControl( element, p, widget, r, cg, flags, data );
	return;
    }
    
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
	    else if ( flags & Style_Default )
		stateId = PBS_DEFAULTED;
	    else
		stateId = PBS_NORMAL;
	}
	break;

    case CE_CheckBox:
	//    case CE_CheckBoxLabel:
	{
	    QCheckBox *cb = (QCheckBox*)widget;
	    drawPrimitive( PE_Indicator, p, subRect( SR_CheckBoxIndicator, widget ), cg, flags, data );
	    return;
	}

    case CE_RadioButton:
	//    case CE_RadioButtonLabel:
	{
	    QRadioButton *rb = (QRadioButton*)widget;
	    drawPrimitive( PE_ExclusiveIndicator, p, subRect( SR_RadioButtonIndicator, widget ), cg, flags, data );
	    return;
	}
	
    case CE_TabBarTab:
	//    case CE_TabBarLabel:
	name = L"TAB";
	{
	    QTabBar *bar = (QTabBar*)widget;
	    QTab *t = data.tab();
	    Q_ASSERT(t);
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
	}
	break;

    case CE_ProgressBarGroove:
	name = L"PROGRESS";
	partId = PP_BAR;
	stateId = 1;
	break;
	
    case CE_PopupMenuItem:
    case CE_MenuBarItem:
    default:
	break;
    }
    
    XPThemeData theme( widget, name, partId, stateId, r );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawControl( element, p, widget, r, cg, flags, data );
	return;
    }
    
    DrawThemeBackground( theme.handle(), p->handle(), theme.partId, theme.stateId, &theme.rect(), 0 );
}

void QWindowsXPStyle::drawComplexControl( ComplexControl control,
					 QPainter* p,
					 const QWidget* w,
					 const QRect& r,
					 const QColorGroup& cg,
					 SFlags flags,
					 SCFlags sub,
					 SCFlags subActive,
					 const QStyleOption &data ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawComplexControl( control, p, w, r, cg, flags, sub, subActive, data );
	return;
    }
    
    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;
    if ( w->hasFocus() )
	flags |= Style_MouseOver;
    
    switch ( control ) {
    case CC_SpinWidget:
        {
	    QSpinWidget *spin = (QSpinWidget*)w;
	    XPThemeData theme( w, L"SPIN" );

	    if ( sub & SC_SpinWidgetFrame )
		drawPrimitive( PE_Panel, p, w->rect(), cg, Style_Default, data );

	    if ( sub & SC_SpinWidgetUp ) {
		theme.rec = querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetUp, data );
		partId = SPNP_UP;
		if ( !spin->isUpEnabled() )
		    stateId = UPS_DISABLED;
		else if ( subActive == SC_SpinWidgetUp )
		    stateId = UPS_PRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = UPS_HOT;
		else
		    stateId = UPS_NORMAL;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_SpinWidgetDown ) {
		theme.rec = querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetDown, data );
		partId = SPNP_DOWN;
		if ( !spin->isDownEnabled() )
		    stateId = DNS_DISABLED;
		else if ( subActive == SC_SpinWidgetDown )
		    stateId = DNS_PRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = DNS_HOT;
		else
		    stateId = DNS_NORMAL;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
        }
        break;
	
    case CC_ComboBox:
        {
	    if ( sub & SC_ComboBoxEditField )
		drawPrimitive( PE_Panel, p, w->rect(), cg, Style_Default, data );

	    if ( sub & SC_ComboBoxArrow ) {
		XPThemeData theme( w, L"COMBOBOX" );
		theme.rec = querySubControlMetrics( CC_ComboBox, w, SC_ComboBoxArrow, data );
		partId = CP_DROPDOWNBUTTON;
		QComboBox *cb = (QComboBox*)w;
		if ( cb->listBox() && cb->listBox()->isVisible() )
		    subActive = SC_ComboBoxArrow;
    
		if ( !w->isEnabled() )
		    stateId = CBXS_DISABLED;
		else if ( subActive == SC_ComboBoxArrow )
		    stateId = CBXS_PRESSED;
		else if ( flags & Style_MouseOver && theme.rec.contains( d->hotSpot ) )
		    stateId = CBXS_HOT;
		else
		    stateId = CBXS_NORMAL;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
        }
        break;
	
    case CC_ScrollBar:
        {
	    XPThemeData theme( w, L"SCROLLBAR" );
	    QScrollBar *bar = (QScrollBar*)w;
	    bool maxedOut = ( bar->maxValue() == bar->minValue() );

	    if ( sub & SC_ScrollBarAddLine ) {
		theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarAddLine, data );
		partId = SBP_ARROWBTN;
		if ( maxedOut || !w->isEnabled() )
		    stateId = 8;
		else if ( subActive == SC_ScrollBarAddLine )
		    stateId = 7;
		else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
		    stateId = 6;
		else
		    stateId = 5;
		if ( bar->orientation() == Horizontal )
		    stateId += 8;
    
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_ScrollBarSubLine ) {
		theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSubLine, data );
		partId = SBP_ARROWBTN;
		if ( maxedOut || !w->isEnabled() )
		    stateId = 4;
		else if ( subActive == SC_ScrollBarSubLine )
		    stateId = 3;
		else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
		    stateId = 2;
		else
		    stateId = 1;
		if ( bar->orientation() == Horizontal )
		    stateId += 8;
    
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( maxedOut ) {
		theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarAddPage, data );
		partId = bar->orientation() == Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
		stateId = 4;
    
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    } else {
		if ( sub & SC_ScrollBarAddPage ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarAddPage, data );
		    partId = bar->orientation() == Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
		    if ( subActive == SC_ScrollBarAddPage )
			stateId = 3;
		    else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
			stateId = 2;
		    else
			stateId = 1;
		    
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_ScrollBarSubPage ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSubPage, data );
		    partId = bar->orientation() == Horizontal ? SBP_UPPERTRACKHORZ : SBP_UPPERTRACKVERT;
		    if ( subActive == SC_ScrollBarSubPage )
			stateId = 3;
		    else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
			stateId = 2;
		    else
			stateId = 1;
        
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_ScrollBarFirst ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarFirst, data );
		}
		if ( sub & SC_ScrollBarLast ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarLast, data );
		}
		if ( sub & SC_ScrollBarSlider ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarSlider, data );
		    if ( subActive == SC_ScrollBarSlider )
			stateId = 3;
		    else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
			stateId = 2;
		    else
			stateId = 1;
        
		    RECT sr = theme.rect();
		    const int swidth = sr.right - sr.left;
		    const int sheight = sr.bottom - sr.top;
		    
		    RECT gr;
		    if ( bar->orientation() == Horizontal ) {
			gr.left = sr.left + swidth/2 - 5;
			gr.right = gr.left + 10;
			gr.top = sr.top + sheight/2 - 3;
			gr.bottom = gr.top + 6;
		    } else {
			gr.left = sr.left + swidth/2 - 3;
			gr.right = gr.left + 6;
			gr.top = sr.top + sheight/2 - 5;
			gr.bottom = gr.top + 10;
		    }
		    DrawThemeBackground( theme.handle(), p->handle(), bar->orientation() == Horizontal ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT, stateId, &sr, 0 );
		    DrawThemeBackground( theme.handle(), p->handle(), bar->orientation() == Horizontal ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT, 1, &gr, 0 );
		}
		if ( sub & SC_ScrollBarGroove ) {
		    theme.rec = querySubControlMetrics( CC_ScrollBar, w, SC_ScrollBarGroove );
		}
	    }
        }
        break;
    case CC_Slider:
	{
	    XPThemeData theme( w, L"TRACKBAR" );
	    QSlider *sl = (QSlider*)w;

	    if ( sub & SC_SliderGroove ) {
		theme.rec = querySubControlMetrics( CC_Slider, w, SC_SliderGroove, data );
		partId = sl->orientation() == Horizontal ? TKP_TRACK : TKP_TRACKVERT;
		if ( !w->isEnabled() )
		    stateId = 4;
		else
		    stateId = 1;
    
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_SliderHandle ) {
		theme.rec = querySubControlMetrics( CC_Slider, w, SC_SliderHandle, data );
		partId = sl->orientation() == Horizontal ? TKP_THUMB : TKP_THUMBVERT;
		if ( !w->isEnabled() )
		    stateId = 4;
		else
		    stateId = 1;
    
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_SliderTickmarks ) {
		theme.rec = querySubControlMetrics( CC_Slider, w, SC_SliderTickmarks, data );
		partId = sl->orientation() == Horizontal ? TKP_TICS : TKP_TICSVERT;
		if ( !w->isEnabled() )
		    stateId = 4;
		else
		    stateId = 1;
    
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	}
	break;
	
	/*    case CC_ToolButton:
	{
	XPThemeData theme;
	theme.name = L"TOOLBAR";
	QToolButton *tb = (QToolButton*)w;
	
	  PFlags flags = Style_Default;
	  if ( tb->isEnabled() )
	  flags |= Style_Enabled;
	  if ( tb->isDown() )
	  flags |= Style_Down;
	  if ( tb->isOn() )
	  flags |= Style_On;
	  if ( d->hotWidget == tb )
	  flags |= Style_MouseOver;
	  
	    if ( sub & SC_ToolButton ) {
	    theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButton, data );
	    drawPrimitive( PE_ButtonTool, p, theme.rec, cg, flags, data );
	    }
	    if ( sub & SC_ToolButtonMenu ) {
	    theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButtonMenu, data );
	    drawPrimitive( PE_ButtonDropDown, p, theme.rec, cg, flags, data );
	    }
	    }
	    break;
	*/
    case CC_TitleBar:
	{
	    const QTitleBar *titlebar = (const QTitleBar *) w;

	    XPThemeData theme( w, L"WINDOW" );
	    if ( sub & SC_TitleBarLabel ) {
		theme.rec = titlebar->rect();
		partId = WP_CAPTION;
		if ( titlebar->isActive() )
		    stateId = CS_ACTIVE;
		else
		    stateId = CS_INACTIVE;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );

		QRect ir = querySubControlMetrics( CC_TitleBar, titlebar, SC_TitleBarLabel );
		QColorGroup cgroup = titlebar->isActive() || !titlebar->window() ?
		    titlebar->palette().active() : titlebar->palette().inactive();
		p->setPen( cgroup.highlightedText() );
		p->drawText(ir.x()+2, ir.y(), ir.width(), ir.height(),
			    AlignAuto | AlignVCenter | SingleLine, titlebar->visibleText() );
	    }
	    if ( titlebar->window() ) {
		if ( sub & SC_TitleBarSysMenu ) {
		    theme.rec = querySubControlMetrics( CC_TitleBar, w, SC_TitleBarSysMenu );
		    partId = WP_SYSBUTTON;
		    stateId = 1;
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_TitleBarMinButton ) {
		    theme.rec = querySubControlMetrics( CC_TitleBar, w, SC_TitleBarMinButton );
		    partId = WP_MINBUTTON;
		    stateId = 1;
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_TitleBarMaxButton ) {
		    theme.rec = querySubControlMetrics( CC_TitleBar, w, SC_TitleBarMaxButton );
		    partId = WP_MAXBUTTON;
		    stateId = 1;
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_TitleBarCloseButton ) {
		    theme.rec = querySubControlMetrics( CC_TitleBar, w, SC_TitleBarCloseButton );
		    partId = WP_CLOSEBUTTON;
		    stateId = 1;
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_TitleBarNormalButton ) {
		    theme.rec = querySubControlMetrics( CC_TitleBar, w, SC_TitleBarNormalButton );
		    partId = WP_RESTOREBUTTON;
		    stateId = 1;
		    DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
		}
		if ( sub & SC_TitleBarShadeButton ) {
		}
		if ( sub & SC_TitleBarUnshadeButton ) {
		}
	    }
	}
	break;
	
    case CC_ListView:
    default:
	QWindowsStyle::drawComplexControl( control, p, w, r, cg, flags, sub, subActive, data );
	break;
    }
}

QRect QWindowsXPStyle::subRect( SubRect r, const QWidget *widget ) const
{
    if ( !use_xp )
	return QWindowsStyle::subRect( r, widget );
    
    switch ( r ) {
/*
    case SR_CheckBoxIndicator:
	{
	    XPThemeData theme( widget, L"BUTTON" );
	    theme.partId = BP_CHECKBOX;
	    theme.stateId = CBS_UNCHECKEDNORMAL;

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return QRect( 0, 0, size.cx, size.cy );
	    }
	}
	break;
	
    case SR_RadioButtonIndicator:
	{
	    XPThemeData theme( widget, L"BUTTON" );
	    theme.partId = BP_RADIOBUTTON;
	    theme.stateId = RBS_UNCHECKEDNORMAL;

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size );
		return QRect( 0, 0, size.cx, size.cy );
	    }
	}
	break;
*/
    default:
	break;
    }
    return QWindowsStyle::subRect( r, widget );
}

int QWindowsXPStyle::pixelMetric( PixelMetric metric,
				 const QWidget *widget ) const
{
    if ( !use_xp )
	return QWindowsStyle::pixelMetric( metric, widget );
    
    switch ( metric ) {
    default:
	return QWindowsStyle::pixelMetric( metric, widget );
    }
}

QSize QWindowsXPStyle::sizeFromContents( ContentsType contents,
					const QWidget *w,
					const QSize &contentsSize,
					const QStyleOption &data ) const
{
    if ( !use_xp )
	return QWindowsStyle::sizeFromContents( contents, w, contentsSize, data );
    
    switch ( contents ) {
    default:
	return QWindowsStyle::sizeFromContents( contents, w, contentsSize, data );
    }
}

QPixmap QWindowsXPStyle::stylePixmap( StylePixmap stylepixmap,
				     const QWidget * w,
				     const QStyleOption &data ) const
{
    if ( !use_xp )
	return QWindowsStyle::stylePixmap( stylepixmap, w, data );
    
    switch ( stylepixmap ) {
    default:
	return QWindowsStyle::stylePixmap( stylepixmap, w, data );
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
    case QEvent::MouseButtonPress:
        {
#ifndef QT_NO_SCROLLBAR
	    if ( widget->inherits( "QScrollBar" ) ) {
		d->lastScrollbarRect = ((QScrollBar*)widget)->sliderRect();
		widget->repaint( FALSE );
	    } else 
#endif
	    {
#ifndef QT_NO_SLIDER
		if ( widget->inherits("QSlider") ) {
		    d->lastSliderRect = ((QSlider*)widget)->sliderRect();
		    widget->repaint( FALSE );
		}
#endif
	    }
        }
        break;

    case QEvent::MouseButtonRelease:
        {
	    if ( widget->inherits( "QScrollBar" ) ) {
		QRect oldRect = d->lastScrollbarRect;
		d->lastScrollbarRect = QRect( 0, -1, 0, -1 );
		widget->repaint( oldRect, FALSE );
	    } else if ( widget->inherits("QSlider") ) {
		QRect oldRect = d->lastSliderRect;
		d->lastSliderRect = QRect( 0, -1, 0, -1 );
		widget->repaint( oldRect, FALSE );

            }
        }
        break;

    case QEvent::MouseMove:
	{
	    if ( !widget->isActiveWindow() )
		break;
	    if ( ((QMouseEvent*)e)->button() )
		break;

	    QMouseEvent *me = (QMouseEvent*)e;

	    d->hotWidget = widget;
	    d->hotSpot = me->pos();

	    if ( o->inherits( "QTabBar" ) ) {
		QTabBar* bar = (QTabBar*)o;
		QTab * t = bar->selectTab( me->pos() );
		if ( d->hotTab != t ) {
		    if ( d->hotTab )
			widget->update( d->hotTab->rect() );
		    d->hotTab = t;
		    if ( d->hotTab )
			widget->update( d->hotTab->rect() );
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
	    } else {
		widget->repaint( FALSE );
	    }
	}
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

    default:
        break;
    }


/*






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
		} else if ( o->inherits( "QSpinWidget" ) ) {
		    QSpinWidget *spin = (QSpinWidget*)o;
		    spin->repaint(FALSE);
		} else if ( o->inherits( "QScrollBar" ) ) {
		    repaintByMouseMove = TRUE;
		    ((QScrollBar*)o)->repaint( FALSE );
		    repaintByMouseMove = FALSE;
		} else if ( o->inherits( "QComboBox" ) ) {
		    repaintByMouseMove = TRUE;
		    ((QWidget*)o)->repaint( FALSE );
		    repaintByMouseMove = FALSE;
		}
	    }
	    break;
    }
*/
    return QWindowsStyle::eventFilter( o, e );
}

#endif //QT_NO_STYLE_WINDOWSXP
