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

#include <qt_windows.h>
#include <uxtheme.h>

static bool repaintByMouseMove          = FALSE;
static int activeScrollBarElement       = 0;

#define BP_PUSHBUTTON		1
#define BP_RADIOBUTTON		2
#define BP_CHECKBOX		3
#define BP_GROUPBOX		4
#define BP_USERBUTTON		5

#define CLP_TIME		1

#define CP_DROPDOWNBUTTON	1

#define EP_CARET		1
#define EP_EDITTEXT		2

#define GP_BORDER		1
#define GP_LINEHORZ		2
#define GP_LINEVERT		3

#define HP_HEADERITEM		1
#define HP_HEADERITEMLEFT	2
#define HP_HEADERITEMRIGHT	3
#define HP_HEADERSORTITEM	4

#define LVP_EMPTYTEXT		1
#define LVP_LISTDETAIL		2
#define LVP_LISTGROUP		3
#define LVP_LISTITEM		4
#define LVP_LISSTORTEDDETAIL	5

#define MP_MENUBARDROPDOWN	1
#define MP_MENUBARITEM		2
#define MP_CHEVRON		3
#define MP_MENUDROPDOWN		4
#define MP_MENUITEM		5
#define MP_SEPARATOR		6

#define PGRP_DOWN		1
#define PGRP_DOWNHORZ		2
#define PGRP_UP                 3
#define PGRP_UPHORZ		4

#define PP_BAR			1
#define PP_BARVERT		2
#define PP_CHUNK		3
#define PP_CHUNKVERT		4

#define RP_BAND			1
#define RP_CHEVRON		2
#define RP_GRIPPER		3
#define RP_GRIPPERVERT		4

#define SBP_ARROWBTN		1
#define SBP_LOWERTRACKHORZ	4
#define SBP_UPPERTRACKHORZ	5
#define SBP_THUMBBTNHORZ	2
#define SBP_GRIPPERHORZ		8
#define SBP_LOWERTRACKVERT	6
#define SBP_UPPERTRACKVERT	7
#define SBP_THUMBBTNVERT	3
#define SBP_GRIPPERVERT		9
#define SBP_SIZEBOX		10

#define SPNP_UP			1
#define SPNP_DOWN		2
#define SPNP_UPHORZ		3
#define SPNP_DOWNHORZ		4

#define SPP_LOGOFF		1
#define SPP_LOGOFFBUTTONS	2
#define SPP_MOREPROGRAMS	3
#define SPP_MOREPROGRAMSARROW	4
#define SPP_PLACELIST		5
#define SPP_PLACELISTSEPARATOR	6
#define SPP_PREVIEW		7
#define SPP_PROGLIST		8
#define SPP_PROGLISTSEPARATOR	9
#define SPP_USERPANE		10
#define SPP_USERPICTURE		11

#define SP_PANE			1
#define SP_GRIPPER		2

#define TABP_BODY		1
#define TABP_PANE		8
#define TABP_TABITEM		1
#define TABP_TABITEMBOTHEDGE	2
#define TABP_TABITEMLEFTEDGE	3
#define TABP_TABITEMRIGHTEDGE	4
#define TABP_TOPTABITEM		6
#define TABP_TOPTABITEMBOTHEDGE	6
#define TABP_TOPTABITEMLEFTEDGE	6
#define TABP_TOPTABITEMRIGHTEDGE 6

#define TDP_GROUPCOUNT		1

#define TBP_BACKGROUNDBOTTOM	1
#define TBP_BACKGROUNDLEFT	2
#define TBP_BACKGROUNDRIGHT	3
#define TBP_BACKGROUNDTOP	4
#define TBP_SIZINGBARBOTTOM	5
#define TBP_SIZINGBARBOTTOMLEFT	6
#define TBP_SIZINGBARRIGHT	7
#define TBP_SIZINGBARTOP	8


#define TP_BUTTON		1
#define TP_DROPDOWNBUTTON	2
#define TP_SPLITBUTTON		3
#define TP_SPLITBUTTONDROPDOWN	4
#define TP_SEPARATOR		5
#define TP_SEPARATORVERT	6

#define TTP_BALLOON		1
#define TTP_BALLOONTITLE	2
#define TTP_STANDARD		3
#define TTP_STANDARDTITLE	4

#define TKP_THUMB		3
#define TKP_THUMBBOTTOM		4
#define TKP_THUMBLEFT		5
#define TKP_THUMBRIGHT		6
#define TKP_THUMBTOP		7
#define TKP_THUMBVERT		8
#define TKP_TICS		9
#define TKP_TICSVERT		10
#define TKP_TRACK		1
#define TKP_TRACKVERT		2

#define TNP_ANIMBACKGROUND	1
#define TNP_BACKGROUND		2

#define TVP_BRANCH		1
#define TVP_GLYPH		2
#define TVP_TREEITEM		3


static ulong ref = 0;
static bool use_xp  = FALSE;
static bool init_xp = FALSE;

class QWindowsXPStylePrivate
{
public:
    QWindowsXPStylePrivate()
	: hotWidget( 0 ), hotSpot( -500, -500 ), hotTab( 0 )
    {
        if ( !init_xp && qWinVersion() == Qt::WV_XP ) {
	    init_xp = TRUE;
	    if ( SearchPathA( NULL, "uxtheme.dll", NULL, 0, NULL, NULL ) ) {
		limboWidget = new QWidget( 0, "xp_limbo_widget" );
		hwnd = limboWidget->winId();

		use_xp = IsThemeActive() && IsAppThemed();
	    } else {
		use_xp = FALSE;
	    }
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
	    }
	}
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

    // hot-widget stuff
    QPoint hotSpot;

    QWidget *hotWidget;
    QTab *hotTab;
    QRect hotHeader;
    QPalette oldPalette;

private:
    static QWidget *limboWidget;
};

QWidget *QWindowsXPStylePrivate::limboWidget = 0;
HWND QWindowsXPStylePrivate::hwnd = NULL;


struct XPThemeData
{
    XPThemeData()
        : name( 0 ), partId( 0 ), stateId( 0 ), htheme( 0 )
    {
    }

    XPThemeData( LPCWSTR theme, int part, int state, const QRect &r )
        : name( theme ),partId( part ), stateId( state ), rec( r ), htheme( 0 )
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
            htheme = OpenThemeData( QWindowsXPStylePrivate::hwnd, name ); 

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

    LPCWSTR name;
    int partId;
    int stateId;
    QRect rec;
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

void QWindowsXPStyle::drawPrimitive( PrimitiveOperation op,
		    QPainter *p,
		    const QRect &r,
		    const QColorGroup &cg,
		    PFlags flags,
		    void **data ) const
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
    case PO_ButtonCommand:
    case PO_ButtonBevel:
        name = L"BUTTON";
        partId = BP_PUSHBUTTON;
        if ( !flags & PStyle_Enabled )
            stateId = 4;
        else if ( flags & PStyle_Down )
	    stateId = 3;
	else if ( flags & PStyle_MouseOver )
	    stateId  =2;
        else
            stateId = 1;

        stateId = flags & PStyle_Sunken ? 2 : 1;
        break;

    case PO_ButtonTool:
        name = L"TOOLBAR";
        partId = TP_BUTTON;
        if ( !flags & PStyle_Enabled )
	    stateId = 4;
        else if ( flags & PStyle_Down )
	    stateId = 3;
        else if ( flags & PStyle_MouseOver )
	    stateId = 2;
        else if ( flags & PStyle_On )
	    stateId = 5;
        else
	    stateId = 1;
        break;

    case PO_ButtonDropDown:
        name = L"TOOLBAR";
        partId = TP_SPLITBUTTON;
        if ( !flags & PStyle_Enabled )
	    stateId = 4;
        else if ( flags & PStyle_Sunken )
	    stateId = 3;
        else if ( !cg.brightText().isValid() )
	    stateId = 2;
        else if ( flags & PStyle_On )
	    stateId = 5;
        else
	    stateId = 1;
        break;

    case PO_Indicator:
        name = L"BUTTON";
        partId = BP_CHECKBOX;
        if ( !flags & PStyle_Enabled ) {
	    if ( flags & PStyle_On )
	        stateId = 8;
	    else
	        stateId = 4;
        } else if ( flags & PStyle_Down ) {
	    if ( flags & PStyle_On )
	        stateId = 7;
	    else
	        stateId = 3;
        } else if ( flags & PStyle_MouseOver ) {
	    if ( flags & PStyle_On )
	        stateId = 6;
	    else
	        stateId = 2;
        } else {
	    if ( flags & PStyle_On )
	        stateId = 5;
	    else
	        stateId = 1;
        }
        break;
    case PO_IndicatorMask:
        break;

    case PO_ExclusiveIndicator:
        name = L"BUTTON";
        partId = BP_RADIOBUTTON;
        if ( !(flags & PStyle_Enabled) ) {
	    if ( flags & PStyle_On )
	        stateId = 8;
	    else
	        stateId = 4;
        } else if ( flags & PStyle_Down ) {
	    if ( flags & PStyle_On )
	        stateId = 7;
	    else
	        stateId = 3;
        } else if ( flags & PStyle_MouseOver ) {
	    if ( flags & PStyle_On )
	        stateId = 6;
	    else
	        stateId = 2;
        } else {
	    if ( flags & PStyle_On )
	        stateId = 5;
	    else
	        stateId = 1;
        }
        break;

    case PO_ExclusiveIndicatorMask:
        break;

    case PO_HeaderSection:
        name = L"HEADER";
        partId = HP_HEADERITEM;
        if ( flags & PStyle_Down )
	    stateId = 3;
        else if ( r == d->hotHeader )
	    stateId = 2;
        else
	    stateId = 1;
        break;

    case PO_StatusBarSection:
        name = L"STATUS";
        partId = SP_PANE;
        stateId = 1;
        break;
	
    case PO_GroupBoxFrame:
	name = L"BUTTON";
	partId = BP_GROUPBOX;
	stateId = 1;
	break;

    case PO_SizeGrip:
	name = L"STATUS";
	partId = SP_GRIPPER;
	stateId = 1;
	break;

    case PO_ScrollBarAddLine:
	name = L"SCROLLBAR";
	break;

    case PO_ScrollBarSubLine:
	name = L"SCROLLBAR";
	break;

    case PO_ScrollBarAddPage:
	name = L"SCROLLBAR";
	break;

    case PO_ScrollBarSubPage:
	name = L"SCROLLBAR";
	break;

    case PO_ScrollBarSlider:
	name = L"SCROLLBAR";
	break;

    case PO_ScrollBarFirst:
	name = L"SCROLLBAR";
	break;

    case PO_ScrollBarLast:
	name = L"SCROLLBAR";
	break;

    case PO_ProgressBarChunk:
	name = L"PROGRESS";
	partId = PP_CHUNK;
	stateId = 1;
	rect = QRect( r.x(), r.y() + 3, r.width(), r.height() - 5 );
	break;

    default:
        break;
    }

    XPThemeData theme( name, partId, stateId, rect );
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
		  CFlags how,
		  void **data ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, data );
	return;
    }
	
    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;

    PFlags f = PStyle_Default;
    if ( widget->isEnabled() )
	f |= PStyle_Enabled;
    if ( widget->hasFocus() )
	f |= PStyle_HasFocus;
    if ( widget == d->hotWidget )
	f |= PStyle_MouseOver;

    switch ( element ) {
    case CE_PushButton:
//    case CE_PushButtonLabel:
        {
	    name = L"BUTTON";
	    partId = BP_PUSHBUTTON;
            QPushButton *pb = (QPushButton*)widget;
            if ( !pb->isEnabled() )
                stateId = 4;
            else if ( pb->isDown() )
	        stateId = 3;
	    else if ( d->hotWidget == pb )
	        stateId  =2;
	    else if ( pb->isDefault() )
	        stateId = 5;
            else
                stateId = 1;
        }
        break;

    case CE_CheckBox:
//    case CE_CheckBoxLabel:
	{
	    QCheckBox *cb = (QCheckBox*)widget;
	    if ( cb->isOn() )
		f |= PStyle_On;
	    else
		f |= PStyle_Off;
	    if ( cb->isDown() )
		f |= PStyle_Down;

	    drawPrimitive( PO_Indicator, p, subRect( SR_CheckBoxIndicator, widget ), cg, f, data );
	    return;
	}

    case CE_RadioButton:
//    case CE_RadioButtonLabel:
	{
	    QRadioButton *rb = (QRadioButton*)widget;
	    if ( rb->isOn() )
		f |= PStyle_On;
	    else
		f |= PStyle_Off;
	    if ( rb->isDown() )
		f |= PStyle_Down;

	    drawPrimitive( PO_ExclusiveIndicator, p, subRect( SR_RadioButtonIndicator, widget ), cg, f, data );
	    return;
	}

    case CE_TabBarTab:
//    case CE_TabBarLabel:
        name = L"TAB";
        {
            QTabBar *bar = (QTabBar*)widget;
            QTab *t = (QTab*)data[0];
            Q_ASSERT(t);
	    partId = TABP_TABITEM;

	    if ( !t->isEnabled() )
		stateId = 4;
	    else if ( how & CStyle_Selected )
		stateId = widget->hasFocus() ? 2 : 3;
	    else if ( d->hotTab == t )
		stateId = 2;
	    else 
		stateId = 1;
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

    XPThemeData theme( name, partId, stateId, r );
    if ( !theme.isValid() ) {
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, data );
	return;
    }

    DrawThemeBackground( theme.handle(), p->handle(), theme.partId, theme.stateId, &theme.rect(), 0 );
}

void QWindowsXPStyle::drawComplexControl( ComplexControl control,
			 QPainter* p,
			 const QWidget* w,
			 const QRect& r,
			 const QColorGroup& cg,
			 CFlags flags,
			 SCFlags sub,
			 SCFlags subActive,
			 void **data ) const
{
    if ( !use_xp ) {
	QWindowsStyle::drawComplexControl( control, p, w, r, cg, flags, sub, subActive, data );
	return;
    }

    LPCWSTR name = 0;
    int partId = 0;
    int stateId = 0;

    switch ( control ) {
    case CC_SpinWidget:
        {
	    QSpinWidget *spin = (QSpinWidget*)w;
            XPThemeData theme;
            theme.name = L"SPIN";

            if ( sub & SC_SpinWidgetUp ) {
                theme.rec = querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetUp, data );
                partId = SPNP_UP;
                if ( !spin->isUpEnabled() )
                    stateId = 4;
                else if ( subActive == SC_SpinWidgetUp )
                    stateId = 3;
                else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
                    stateId = 2;
                else
                    stateId = 1;
                DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
            }
            if ( sub & SC_SpinWidgetDown ) {
                theme.rec = querySubControlMetrics( CC_SpinWidget, w, SC_SpinWidgetDown, data );
                partId = SPNP_DOWN;
                if ( !spin->isDownEnabled() )
                    stateId = 4;
                else if ( subActive == SC_SpinWidgetDown )
                    stateId = 3;
                else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
                    stateId = 2;
                else
                    stateId = 1;
                DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
            }
        }
        break;

    case CC_ComboBox:
        {
	    if ( sub & SC_ComboBoxEditField ) {
		XPThemeData theme;
		theme.rec = querySubControlMetrics( CC_ComboBox, w, SC_ComboBoxEditField, data );
		theme.rec = w->rect();
		theme.name = L"GLOBALS";
		partId = 1;
		stateId = 1;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
            if ( sub & SC_ComboBoxArrow ) {
		XPThemeData theme;
                theme.rec = querySubControlMetrics( CC_ComboBox, w, SC_ComboBoxArrow, data );
		theme.name = L"COMBOBOX";
                partId = CP_DROPDOWNBUTTON;
		QComboBox *cb = (QComboBox*)w;
		if ( cb->listBox() && cb->listBox()->isVisible() )
		    subActive = SC_ComboBoxArrow;

                if ( !w->isEnabled() )
                    stateId = 4;
                else if ( subActive == SC_ComboBoxArrow )
                    stateId = 3;
		else if ( d->hotWidget == w && theme.rec.contains( d->hotSpot ) )
		    stateId = 2;
                else
                    stateId = 1;
                DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
            }
        }
        break;

    case CC_ScrollBar:
        {
            XPThemeData theme;
            theme.name = L"SCROLLBAR";
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
	    XPThemeData theme;
	    theme.name = L"TRACKBAR";
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

    case CC_ToolButton:
	{
	    XPThemeData theme;
	    theme.name = L"TOOLBAR";
	    QToolButton *tb = (QToolButton*)w;

	    PFlags flags = PStyle_Default;
	    if ( tb->isEnabled() )
		flags |= PStyle_Enabled;
	    if ( tb->isDown() )
		flags |= PStyle_Down;
	    if ( tb->isOn() )
		flags |= PStyle_On;
	    if ( d->hotWidget == tb )
		flags |= PStyle_MouseOver;

	    if ( sub & SC_ToolButton ) {
		theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButton, data );
		drawPrimitive( PO_ButtonTool, p, theme.rec, cg, flags, data );
	    }
	    if ( sub & SC_ToolButtonMenu ) {
		theme.rec = querySubControlMetrics( CC_ToolButton, w, SC_ToolButtonMenu, data );
		drawPrimitive( PO_ButtonDropDown, p, theme.rec, cg, flags, data );
	    }
	}
	break;

    case CC_TitleBar:
	{
	    XPThemeData theme;
	    theme.name = L"WINDOW";
	    if ( sub & SC_TitleBarSysMenu ) {
	    }
	    if ( sub & SC_TitleBarMinButton ) {
		partId = 13;
		stateId = 1;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_TitleBarMaxButton ) {
		partId = 14;
		stateId = 1;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_TitleBarCloseButton ) {
		partId = 16;
		stateId = 1;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_TitleBarLabel ) {
		partId = 6;
		stateId = 1;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_TitleBarNormalButton ) {
		partId = 19;
		stateId = 1;
		DrawThemeBackground( theme.handle(), p->handle(), partId, stateId, &theme.rect(), 0 );
	    }
	    if ( sub & SC_TitleBarShadeButton ) {
	    }
	    if ( sub & SC_TitleBarUnshadeButton ) {
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
    case SR_CheckBoxIndicator:
	{
	    XPThemeData theme;
	    theme.name = L"BUTTON";
	    theme.partId = BP_CHECKBOX;
	    theme.stateId = 1;

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, TS_TRUE, &size );
		return QRect( 0, 0, size.cx, size.cy );
	    }
	}
	break;

    case SR_RadioButtonIndicator:
	{
	    XPThemeData theme;
	    theme.name = L"BUTTON";
	    theme.partId = BP_RADIOBUTTON;
	    theme.stateId = 1;

	    if ( theme.isValid() ) {
		SIZE size;
		GetThemePartSize( theme.handle(), NULL, theme.partId, theme.stateId, TS_TRUE, &size );
		return QRect( 0, 0, size.cx, size.cy );
	    }
	}
	break;

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
			void **data ) const
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
		     void **data ) const
{
    if ( !use_xp )
	return QWindowsStyle::stylePixmap( stylepixmap, w, data );

    switch ( stylepixmap ) {
    default:
	return QWindowsStyle::stylePixmap( stylepixmap, w, data );
    }
}


/*
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

// Push button
void QWindowsXPStyle::getButtonShift( int &x, int &y) const
{
    x = y = 0;
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
	DrawThemeBackground( htheme, p->handle(), 1, 3, &r2, 0 );
    else
	DrawThemeBackground( htheme, p->handle(), 1,
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
    if ( disabled )
	statusId = 5;
    else if ( down )
	statusId = 3;
    else if ( d->hotWidget == p->device() )
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

*/
// HotSpot magic
bool QWindowsXPStyle::eventFilter( QObject *o, QEvent *e )
{
    if ( use_xp && o && o->isWidgetType() ) {
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
	    {
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
		} else if ( oldHot->inherits( "QSpinWidget" ) ) {
		    QSpinWidget *spin = (QSpinWidget*)oldHot;
		    spin->repaint(FALSE);
		} else {
		    oldHot->update();
		}
	    }
	    break;
	case QEvent::FocusIn:
	case QEvent::FocusOut:
	    ((QWidget*)o)->repaint(FALSE);
	    break;
	default:
	    break;
	}
    }
    return QWindowsStyle::eventFilter( o, e );
}

#endif //QT_NO_STYLE_WINDOWSXP
