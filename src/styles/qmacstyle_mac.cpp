/****************************************************************************
** $Id$
**
** Implementation of Mac native theme
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/* broken things:
   titlebar isn't complete
   headers are too short
   we don't animate yet
*/

#include "qmacstyle_mac.h"

#if defined( Q_WS_MAC ) && !defined( QT_NO_STYLE_MAC )
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qt_mac.h>
#include <qtabbar.h>
#include "private/qtitlebar_p.h"
#include <Appearance.h>
#include <qbitmap.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qcombobox.h>
#include <qdrawutil.h>
#include "qwidgetlist.h"
#include "private/qaquastyle_p.h"

#define QMAC_NO_MACSTYLE_ANIMATE //just disable animations for now

//externals
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp

//static utility variables
static QColor qt_mac_highlight_color = QColor( 0xC2, 0xC2, 0xC2 ); //color of highlighted text
static const int macItemFrame         = 2;    // menu item frame width
static const int macItemHMargin       = 3;    // menu item hor text margin
static const int macItemVMargin       = 2;    // menu item ver text margin
static const int macRightBorder       = 12;   // right border on mac

//hack, but usefull
#include <qpainter.h>
class QMacPainter : public QPainter
{
public:
    ~QMacPainter();
    void noop() { QPainter::initPaintDevice(TRUE); }
private:
    QMacPainter();
};
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=NULL, 
					  bool off=TRUE, const QRect &rect=QRect())
{
    static Rect r;
    QPoint tl(qr.topLeft());
    if(pd && pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    if(rect.isValid())
	tl += rect.topLeft();
    int offset = 0;
    if(off)
	offset = 1;
    SetRect(&r, tl.x(), tl.y(), tl.x() + qr.width() - offset, tl.y() + qr.height() - offset);
    if(rect.isValid()) {
	r.right -= rect.width();
	r.bottom -= rect.height();
    }
    return &r;
}

//private
class QMacStylePrivate : public QAquaAnimate
{
    ControlRef button, progressbar;
public:
    QMacStylePrivate() : QAquaAnimate(), button(0), progressbar(0) { }
    ~QMacStylePrivate();
    ControlRef control(QAquaAnimate::Animates);
protected:
    void doAnimate(QAquaAnimate::Animates);
};
ControlRef 
QMacStylePrivate::control(QAquaAnimate::Animates as)
{	
    if(as == QAquaAnimate::AquaPushButton) {
	if(!button) {
	    if( CreatePushButtonControl((WindowPtr)qt_mac_safe_pdev->handle(), 
					qt_glb_mac_rect(QRect(0, 0, 40, 40), 0, FALSE),
					0, &button)) {
		qDebug("Unexpected error: %s:%d", __FILE__, __LINE__);
	    } else {
		Boolean t = true;
		SetControlData(button, 0, kControlPushButtonDefaultTag, sizeof(t), &t);
		ShowControl(button);
	    }
	}
	return button;
    } else if(as == QAquaAnimate::AquaProgressBar) {
	if(progressbar)
	    return progressbar;
    }
    return 0;
}
QMacStylePrivate::~QMacStylePrivate()
{
    if(button)
	DisposeControl(button);
    if(progressbar)
	DisposeControl(progressbar);
}
void QMacStylePrivate::doAnimate(QAquaAnimate::Animates)
{
#ifndef QMAC_NO_MACSTYLE_ANIMATE
    if(QWidgetList *list = qApp->topLevelWidgets()) {
	for ( QWidget *widget = list->first(); widget; widget = list->next() ) {
	    if(widget->isActiveWindow()) 
		IdleControls((WindowPtr)widget->handle());
	}
    }
#endif
}

#define private public //ugh, what I'll do..
#include <qslider.h>
#undef private

static int mac_count = 0;
QMacStyle::QMacStyle(  )  : QWindowsStyle()
{
    d = new QMacStylePrivate;
    if(!mac_count++)
	RegisterAppearanceClient();
}

QMacStyle::~QMacStyle()
{
    if(!(--mac_count))
	UnregisterAppearanceClient();
}

void QMacStyle::polish( QApplication* app )
{
    QPalette pal = app->palette();

    QPixmap px(200, 200, 32);
    {
	QPainter p(&px);
	((QMacPainter *)&p)->noop();
	SetThemeBackground(kThemeBrushDialogBackgroundActive, px.depth(), true);
	EraseRect(qt_glb_mac_rect(QRect(0, 0, px.width(), px.height()), 0, FALSE));
    }
    QBrush background( Qt::black, px );
    pal.setBrush( QColorGroup::Background, background );
    pal.setBrush( QColorGroup::Button, background );

    pal.setColor( QPalette::Inactive, QColorGroup::ButtonText, QColor( 148,148,148 ));
    pal.setColor( QPalette::Disabled, QColorGroup::ButtonText, QColor( 148,148,148 ));

    pal.setColor( QPalette::Active, QColorGroup::Highlight, qt_mac_highlight_color );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight, qt_mac_highlight_color.light() );
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight, QColor( 0xC2, 0xC2, 0xC2 ) );

    pal.setColor( QColorGroup::HighlightedText, Qt::black);

    app->setPalette( pal, TRUE );
}

void QMacStyle::polish( QWidget* w )
{
#ifndef QMAC_NO_MACSTYLE_ANIMATE
    d->addWidget(w);
#endif
    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( FALSE );
	if( btn->group() ){
	    btn->group()->setMargin( 0 );
	    btn->group()->setInsideSpacing( 0 );
	}
    } else if( w->inherits("QToolBar") ){
	QToolBar * bar = (QToolBar *) w;
	QBoxLayout * layout = bar->boxLayout();
	layout->setSpacing( 0 );
	layout->setMargin( 0 );
    }

    qAquaPolishFont(w);
}

void QMacStyle::unPolish( QWidget* w )
{
#ifndef QMAC_NO_MACSTYLE_ANIMATE
    d->removeWidget(w);
#endif
    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( TRUE );
    } 
}

/*! \reimp */
void QMacStyle::drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString& text,
			   int len, const QColor* penColor ) const
{
    //No accelerators drawn here!
    QWindowsStyle::drawItem( p, r, flags | NoAccel, g, enabled, pixmap, text,
			     len, penColor );
}

/*! \reimp */
void QMacStyle::drawPrimitive( PrimitiveElement pe,
			       QPainter *p,
			       const QRect &r,
			       const QColorGroup &cg,
			       SFlags flags,
			       const QStyleOption& opt ) const
{
    ThemeDrawState tds = 0;
    if(flags & Style_Enabled)
	tds |= kThemeStateActive;
    else
	tds |= kThemeStateInactive;
    if(flags & Style_Down)
	tds = kThemeStatePressed;

    switch(pe) {
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
	p->save();
	p->setPen( cg.text() );
	QPointArray a;
	if ( pe == PE_ArrowDown )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y(), r.x() + (r.width() / 2) , 
			 r.bottom());
	else if( pe == PE_ArrowRight )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y() + (r.height() / 2), r.x(), 
			 r.bottom());
	else if( pe == PE_ArrowUp )
	    a.setPoints( 3, r.x() + (r.width() / 2), r.y(), r.right(), r.bottom(), r.x(), 
			 r.bottom());
	else
	    a.setPoints( 3, r.x(), r.y() + (r.height() / 2), r.right(), r.y(), r.right(), 
			 r.bottom());
	p->setBrush( cg.text() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
	p->restore();
	break; }
    case PE_GroupBoxFrame: {
	if ( opt.isDefault() )
	    break;
	if(opt.frameShape() == QFrame::Box && opt.frameShadow() == QFrame::Sunken) {
	    DrawThemePrimaryGroup(qt_glb_mac_rect(r, p->device()), kThemeStateActive);
	} else {
	    QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	}
	break; }
    case PE_FocusRect:
	break;     //###
    case PE_TabBarBase: 
	DrawThemeTabPane(qt_glb_mac_rect(r, p->device()), tds);
	break; 
    case PE_Splitter:
	DrawThemeSeparator(qt_glb_mac_rect(r, p->device()), tds & ~(kThemeStatePressed));
	break;
    case PE_HeaderArrow: 
    case PE_HeaderSection: {
	ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_Down)
	    info.state |= kThemeStatePressed;
	if(flags & Style_Sunken)
	    info.value = kThemeButtonOn;
	if(pe == PE_HeaderArrow && (flags & Style_Up))
	    info.adornment |= kThemeAdornmentArrowUpArrow;
	((QMacPainter *)p)->noop();
	DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemeListHeaderButton, 
			&info, NULL, NULL, NULL, 0);
	break; }
    case PE_ExclusiveIndicatorMask: 
    case PE_ExclusiveIndicator: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_On)
	    info.value = kThemeButtonOn;
	if(pe == PE_ExclusiveIndicator) {
	    p->fillRect(r, white);
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemeRadioButton, 
			    &info, NULL, NULL, NULL, 0);
	} else {
	    p->save();
	    QRegion rgn;
	    GetThemeButtonRegion(qt_glb_mac_rect(r, p->device()), kThemeRadioButton,
				 &info, rgn.handle(TRUE));
	    p->setClipRegion(rgn);
	    p->fillRect(r, color1);
	    p->restore();
	}
	break; }
    case PE_IndicatorMask: 
    case PE_Indicator: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_NoChange)
	    info.value = kThemeButtonMixed;
	else if(flags & Style_On)
	    info.value = kThemeButtonOn;
	if(pe == PE_Indicator) {
	    p->fillRect(r, white);
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemeCheckBox,
			    &info, NULL, NULL, NULL, 0);
	} else {
	    p->save();
	    QRegion rgn;
	    GetThemeButtonRegion(qt_glb_mac_rect(r, p->device()), kThemeCheckBox,
				 &info, rgn.handle(TRUE));
	    p->setClipRegion(rgn);
	    p->fillRect(r, color1);
	    p->restore();
	}
	break; }
    default:
	QWindowsStyle::drawPrimitive( pe, p, r, cg, flags, opt);
	break;
    }
}


void QMacStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags how,
				 const QStyleOption& opt ) const
{
    ThemeDrawState tds = 0;
    if(how & Style_Enabled || widget->isEnabled())
	tds |= kThemeStateActive;
    else
	tds |= kThemeStateInactive;
    if(how & Style_Down)
	tds = kThemeStatePressed;
    
    switch(element) {
    case CE_PopupMenuItem: {
	if(!widget || opt.isDefault())
	    break;
	QPopupMenu *popupmenu = (QPopupMenu *)widget;
	QMenuItem *mi = opt.menuItem();
	if ( !mi )
	    break;

	QComboBox *combo = NULL;
	if(popupmenu->parentWidget() && popupmenu->parentWidget()->inherits("QComboBox"))
	    combo = (QComboBox*)popupmenu->parentWidget();
	const QColorGroup & g = cg;
	QColorGroup itemg = g;
	bool dis = !mi->isEnabled();
	int tab = opt.tabWidth();
	int maxpmw = opt.maxIconWidth();
	bool checked = combo ? mi->id() == popupmenu->idAt(combo->currentItem()) : mi->isChecked();
	bool checkable = popupmenu->isCheckable();
	bool act = how & Style_Active;
	int x, y, w, h;
	r.rect(&x, &y, &w, &h);
	Rect mrect = *qt_glb_mac_rect(popupmenu->rect(), p->device()),
	     irect = *qt_glb_mac_rect(r, p->device(), FALSE);

	if ( checkable )
	    maxpmw = QMAX( maxpmw, 12 ); // space for the checkmarks

	int checkcol = maxpmw;
	if ( mi && mi->isSeparator() ) {
	    ((QMacPainter *)p)->noop();
	    DrawThemeMenuSeparator(&irect);
	    return;
	}

	ThemeMenuState tms = kThemeMenuActive;
	if(!mi->isEnabled())
	    tms |= kThemeMenuDisabled;
	if(how & Style_Active)
	    tms |= kThemeMenuSelected;
	ThemeMenuItemType tmit = kThemeMenuItemPlain;
	if(mi->popup())
	    tmit |= kThemeMenuItemHierarchical;
	if(mi->iconSet())
	    tmit |= kThemeMenuItemHasIcon;
	((QMacPainter *)p)->noop();
	DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, NULL, 0);
	bool reverse = QApplication::reverseLayout();

	int xpos = x;
	if ( reverse )
	    xpos += w - checkcol;
	if ( checked ) {
	    int mw = checkcol + macItemFrame;
	    int mh = h - 2*macItemFrame;
	    ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOn, kThemeAdornmentDrawIndicatorOnly };
	    if(!(how & Style_Enabled) || !widget->isEnabled())
		info.state = kThemeStateInactive;
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(QRect(x + macItemFrame + 2, y + macItemFrame, mw-4, mh), p->device()), 
			    kThemeCheckBox, &info, NULL, NULL, NULL, 0);
	} 

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
	    if ( act && !dis ) {
		if ( !mi->isChecked() )
		    qDrawShadePanel( p, xpos, y, checkcol, h, g, FALSE, 1,
				     &g.brush( QColorGroup::Button ) );
	    }
	    QRect cr( xpos, y, checkcol, h );
	    QRect pmr( 0, 0, pixw, pixh );
	    pmr.moveCenter( cr.center() );
	    p->setPen( itemg.text() );
	    p->drawPixmap( pmr.topLeft(), pixmap );
	} else  if ( checkable ) {  // just "checking"...
	    int mw = checkcol + macItemFrame;
	    int mh = h - 2*macItemFrame;
	    if ( mi->isChecked() ) {
		int xp = xpos;
		if( reverse )
		    xp -= macItemFrame;
		else
		    xp += macItemFrame;

		SFlags cflags = Style_Default;
		if (! dis)
		    cflags |= Style_Enabled;
		if (act)
		    cflags |= Style_On;
		drawPrimitive(PE_CheckMark, p, QRect(xp, y+macItemFrame, mw, mh), cg, cflags);
	    }
	}

	p->setPen( act ? Qt::white/*g.highlightedText()*/ : g.buttonText() );

	QColor discol;
	if ( dis ) {
	    discol = itemg.text();
	    p->setPen( discol );
	}

	int xm = macItemFrame + checkcol + macItemHMargin;
	if ( reverse )
	    xpos = macItemFrame + tab;
	else
	    xpos += xm;

	if ( mi->custom() ) {
	    int m = macItemVMargin;
	    p->save();
	    if ( dis && !act ) {
		p->setPen( g.light() );
		mi->custom()->paint( p, itemg, act, !dis,
				     xpos+1, y+m+1, w-xm-tab+1, h-2*m );
		p->setPen( discol );
	    }
	    mi->custom()->paint( p, itemg, act, !dis,
				 x+xm, y+m, w-xm-tab+1, h-2*m );
	    p->restore();
	}
	QString s = mi->text();
	if ( !s.isNull() ) {                        // draw text
	    int t = s.find( '\t' );
	    int m = macItemVMargin;
	    const int text_flags = AlignVCenter|NoAccel | DontClip | SingleLine;
	    if ( t >= 0 ) {                         // draw tab text
		int xp;
		if( reverse )
		    xp = x + macRightBorder+macItemHMargin+macItemFrame - 1;
		else
		    xp = x + w - tab - macRightBorder-macItemHMargin-macItemFrame+1;
		if ( dis && !act ) {
		    p->setPen( g.light() );
		    p->drawText( xp, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
		    p->setPen( discol );
		}
		p->drawText( xp, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
		s = s.left( t );
	    }
	    if ( dis && !act ) {
		p->setPen( g.light() );
		p->drawText( xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t );
		p->setPen( discol );
	    }
	    p->drawText( xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
	} else if ( mi->pixmap() ) {                        // draw pixmap
	    QPixmap *pixmap = mi->pixmap();
	    if ( pixmap->depth() == 1 )
		p->setBackgroundMode( OpaqueMode );
	    p->drawPixmap( xpos, y+macItemFrame, *pixmap );
	    if ( pixmap->depth() == 1 )
		p->setBackgroundMode( TransparentMode );
	}
	break; }
    case CE_MenuBarItem: {
	if(!widget)
	    break;
	const QMenuBar *mbar = (const QMenuBar *)widget;
	Rect mrect = *qt_glb_mac_rect(mbar->rect(), p->device()),
	     irect = *qt_glb_mac_rect(r, p->device());
	ThemeMenuState tms = kThemeMenuActive;
	if(!(how & Style_Active))
	    tms |= kThemeMenuDisabled;
	if(how & Style_Down)
	    tms |= kThemeMenuSelected;
	((QMacPainter *)p)->noop();
	DrawThemeMenuTitle(&mrect, &irect, tms, 0, NULL, 0);
	QCommonStyle::drawControl(element, p, widget, r, cg, how, opt);
	break; }
    case CE_ProgressBarContents: {
	if(!widget)
	    break;
	QProgressBar *pbar = (QProgressBar *) widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeLargeProgressBar;
	ttdi.bounds = *qt_glb_mac_rect(r, p->device());
	ttdi.max = pbar->totalSteps();
	ttdi.value = pbar->progress();
	ttdi.attributes |= kThemeTrackHorizontal;
	if(widget->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!pbar->isEnabled())
	    ttdi.enableState |= kThemeTrackDisabled;
	((QMacPainter *)p)->noop();
	DrawThemeTrack(&ttdi, NULL, NULL, 0);
	break; }
    case CE_TabBarTab: {
	if(!widget)
	    break;
	if(how & Style_Sunken)
	    tds |= kThemeStatePressed;
	QTabBar * tb = (QTabBar *) widget;
	ThemeTabStyle tts = kThemeTabNonFront;
	if(how & Style_Selected) {
	    if(!(how & Style_Enabled))
		tts = kThemeTabFrontInactive;
	    else
		tts = kThemeTabFront;
	} else if(!(how & Style_Enabled)) {
	    tts = kThemeTabNonFrontPressed;
	} else if((how & Style_Sunken) && (how & Style_MouseOver)) {
	    tts = kThemeTabNonFrontPressed;
	}
	ThemeTabDirection ttd = kThemeTabNorth;
	if( tb->shape() == QTabBar::RoundedBelow )
	    ttd = kThemeTabSouth;
	((QMacPainter *)p)->noop();
	DrawThemeTab(qt_glb_mac_rect(r, p->device(), FALSE), tts, ttd, NULL, 0);
	break; }
    case CE_PushButton: {
#ifndef QMAC_NO_MACSTYLE_ANIMATE
	if(d->animatable(QAquaAnimate::AquaPushButton, (QWidget *)widget)) {
	    ControlRef btn = d->control(QAquaAnimate::AquaPushButton);
	    SetControlBounds(btn, qt_glb_mac_rect(r, p->device(), TRUE, QRect(3, 3, 6, 6)));
	    ((QMacPainter *)p)->noop();
	    DrawControlInCurrentPort(btn);
	} else 
#endif
	{
	    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(r, p->device(), TRUE, QRect(3, 3, 6, 6)), 
			    kThemePushButton, &info, NULL, NULL, NULL, 0);
	}
	break; }
    default:
	QWindowsStyle::drawControl(element, p, widget, r, cg, how, opt);
    }
}

void QMacStyle::drawComplexControl( ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt ) const
{
    ThemeDrawState tds = 0;
    if(flags & Style_Enabled)
	tds |= kThemeStateActive;
    else
	tds |= kThemeStateInactive;

    switch(ctrl) {
    case CC_ToolButton: {
	if(!widget)
	    break;
	QToolButton *toolbutton = (QToolButton *) widget;

	QRect button, menuarea;
	button   = querySubControlMetrics(ctrl, widget, SC_ToolButton, opt);
	menuarea = querySubControlMetrics(ctrl, widget, SC_ToolButtonMenu, opt);
	SFlags bflags = flags,
	       mflags = flags;
	if (subActive & SC_ToolButton)
	    bflags |= Style_Down;
	if (subActive & SC_ToolButtonMenu)
	    mflags |= Style_Down;

	if (sub & SC_ToolButton) {
	    if (bflags & (Style_Down | Style_On | Style_Raised)) {
		ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
		if(toolbutton->isOn() || toolbutton->isDown())
		    info.value |= kThemeStatePressed;
#if 0
		QWidget *btn_prnt = toolbutton->parentWidget();
		if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		    QToolBar * bar  = (QToolBar *) btn_prnt;
		    if( bar->orientation() == Qt::Vertical )
			mod += "v";
		    QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		    QObjectListIt it( *l );
		    if ( it.toFirst() == toolbutton )
			mod += "left";
		    else if( it.toLast() == toolbutton && !toolbutton->popup() )
			mod += "right";
		    else
			mod += "mid";
		    delete l;
		} else {
		    mod += "mid";
		}
#endif
		((QMacPainter *)p)->noop();
		DrawThemeButton(qt_glb_mac_rect(button, p->device()), 
				kThemeBevelButton, &info, NULL, NULL, NULL, 0);
	    } else if ( toolbutton->parentWidget() &&
			toolbutton->parentWidget()->backgroundPixmap() &&
			! toolbutton->parentWidget()->backgroundPixmap()->isNull() ) {
		p->drawTiledPixmap( r, *(toolbutton->parentWidget()->backgroundPixmap()),
				    toolbutton->pos() );
	    }
	}

	if (sub & SC_ToolButtonMenu) {
	    ThemeButtonDrawInfo info = { tds, kThemeDisclosureDown, kThemeAdornmentNone };
	    if(toolbutton->isOn() || toolbutton->isDown() || (subActive & SC_ToolButtonMenu)) 
		info.value |= kThemeStatePressed;
#if 0
	    QWidget *btn_prnt = toolbutton->parentWidget();
	    if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		QToolBar * bar  = (QToolBar *) btn_prnt;
		if( bar->orientation() == Qt::Vertical )
		    mod += "v";
		QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		QObjectListIt it( *l );
		if ( it.toFirst() == toolbutton ) {
		    if( bar->orientation() == Qt::Horizontal )
			mod += "left";
		} else if( it.toLast() == toolbutton && !toolbutton->popup() ){
		    if( bar->orientation() == Qt::Horizontal )
			mod += "right";
		} else {
		    mod += "mid";
		}
		delete l;
	    } else {
		mod += "mid";
	    }
#endif
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(menuarea, p->device()), 
			    kThemeDisclosureButton, &info, NULL, NULL, NULL, 0);
	}
	break; }
    case CC_ListView: {
	if ( sub & SC_ListView ) {
	    QWindowsStyle::drawComplexControl( ctrl, p, widget, r, cg, flags, sub, subActive, opt );
	}
	if ( sub & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
	    if (opt.isDefault())
		break;
	    QListViewItem *item = opt.listViewItem();
	    int y=r.y(), h=r.height();
	    for(QListViewItem *child = item->firstChild(); child && y < h;
		y += child->totalHeight(), child = child->nextSibling()) {
		if(y + child->height() > 0) {
		    if ( child->isExpandable() || child->childCount() )
			drawPrimitive( child->isOpen() ? PE_ArrowDown : PE_ArrowRight, p,
				       QRect(r.right() - 10, (y + child->height()/2) - 4, 9, 9), cg );
		}
	    }
	}
	break; }
    case CC_SpinWidget: {
	QSpinWidget * sw = (QSpinWidget *) widget;
	if((sub & SC_SpinWidgetDown) || (sub & SC_SpinWidgetUp)) {
	    if(subActive == SC_SpinWidgetDown)
		tds |= kThemeStatePressedDown;
	    else if(subActive == SC_SpinWidgetUp)
		tds |= kThemeStatePressedUp;
	    if(sw->isUpEnabled() || sw->isDownEnabled())
		tds &= ~kThemeStateInactive;
	    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	    QRect updown = sw->upRect() | sw->downRect();
	    if(sw->backgroundPixmap())
		p->drawPixmap(updown, *sw->backgroundPixmap());
	    else
		p->fillRect(updown, sw->backgroundColor());
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(updown, p->device()), kThemeIncDecButton, 
			    &info, NULL, NULL, NULL, 0);
	}
	if ( sub & SC_SpinWidgetFrame )
	    QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, 
					      SC_SpinWidgetFrame, subActive, opt);
	break; }
    case CC_TitleBar: {
	if(!widget)
	    break;
	QTitleBar *tbar = (QTitleBar *) widget;
	ThemeWindowMetrics twm;
	memset(&twm, '\0', sizeof(twm));
	twm.metricSize = sizeof(twm);
	twm.titleWidth = tbar->width();
	twm.titleHeight = tbar->height();
	ThemeWindowAttributes twa = kThemeWindowHasTitleText;
	if(tbar->window()) 
	    twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
	else if(tbar->testWFlags( WStyle_SysMenu)) 
	    twa |= kThemeWindowHasCloseBox;
	const Rect *rect = qt_glb_mac_rect(r, p->device());
	((QMacPainter *)p)->noop();
#if 0
	if(sub & SC_TitleBarCloseButton) 
	    DrawThemeTitleBarWidget(kThemeUtilityWindow, rect, 
				    tds | ((subActive & SC_TitleBarCloseButton) ? kThemeStatePressed : 0), 
				    &twm, twa, kThemeWidgetCloseBox);
	if(sub & SC_TitleBarMinButton) 
	    DrawThemeTitleBarWidget(kThemeUtilityWindow, rect, 
				    tds | ((subActive & SC_TitleBarMinButton) ? kThemeStatePressed : 0), 
				    &twm, twa, kThemeWidgetCollapseBox);
	if(sub & SC_TitleBarNormalButton) 
	    DrawThemeTitleBarWidget(kThemeUtilityWindow, rect, 
				    tds | ((subActive & SC_TitleBarNormalButton) ? kThemeStatePressed : 0), 
				    &twm, twa | kThemeWindowIsCollapsed, kThemeWidgetCollapseBox);
	if(sub & SC_TitleBarMaxButton) 
	    DrawThemeTitleBarWidget(kThemeUtilityWindow, rect, 
				    tds | ((subActive & SC_TitleBarMaxButton) ? kThemeStatePressed : 0), 
				    &twm, twa, kThemeWidgetZoomBox);
#else
	DrawThemeWindowFrame(kThemeDocumentWindow, rect, tds, &twm, twa, NULL, 0);
#endif
	break; }
    case CC_ScrollBar: {
	if(!widget)
	    break;
	QScrollBar *scrollbar = (QScrollBar *) widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumScrollBar;
	ttdi.bounds = *qt_glb_mac_rect(r, p->device());
	ttdi.min = scrollbar->minValue();
	ttdi.max = scrollbar->maxValue();
	ttdi.value = scrollbar->value();
	ttdi.attributes |= kThemeTrackShowThumb;
	if(scrollbar->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(widget->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!scrollbar->isEnabled())
	    ttdi.enableState |= kThemeTrackDisabled;
	if(subActive == SC_ScrollBarSubLine)
	    ttdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed | 
						  kThemeLeftOutsideArrowPressed;
	else if(subActive == SC_ScrollBarAddLine)
	    ttdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed |
						  kThemeRightOutsideArrowPressed;
	else if(subActive == SC_ScrollBarAddPage)
	    ttdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
	else if(subActive == SC_ScrollBarSubPage)
	    ttdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
	else if(subActive == SC_ScrollBarSlider)
	    ttdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
	ttdi.trackInfo.scrollbar.viewsize = scrollbar->pageStep();
	((QMacPainter *)p)->noop();
	DrawThemeTrack(&ttdi, NULL, NULL, 0);
	break; }
    case CC_Slider: {
	if(!widget)
	    break;
	QSlider *sldr = (QSlider *)widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumSlider;
	ttdi.bounds = *qt_glb_mac_rect(widget->rect(), p->device());
	ttdi.min = sldr->minValue();
	ttdi.max = sldr->maxValue();
	ttdi.value = sldr->valueFromPosition(sldr->sliderStart());
	ttdi.attributes |= kThemeTrackShowThumb;
	if(sldr->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(widget->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!sldr->isEnabled())
	    ttdi.enableState |= kThemeTrackDisabled;
	if(sldr->tickmarks() == QSlider::Above)
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
	else
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
	if(subActive == SC_SliderGroove)
	    ttdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
	else if(subActive == SC_SliderHandle)
	    ttdi.trackInfo.slider.pressState = kThemeThumbPressed;
	if(sldr->backgroundPixmap())
	    p->drawPixmap(r, *sldr->backgroundPixmap());
	else
	    p->fillRect(r, sldr->backgroundColor());
	((QMacPainter *)p)->noop();
	DrawThemeTrack(&ttdi, NULL, NULL, 0);
	if ( sub & SC_SliderTickmarks )
	    DrawThemeTrackTickMarks(&ttdi, sldr->maxValue() / sldr->pageStep(), NULL, 0);
	break; }
    case CC_ComboBox: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	((QMacPainter *)p)->noop();
	DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemePopupButton, 
			&info, NULL, NULL, NULL, 0);
	break; }
    default:
	QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, opt);
    }
}

int QMacStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret = 0;
    switch(metric) {
    case PM_TabBarTabOverlap:
	ret = 0;
	break;
    case PM_ScrollBarSliderMin:
	ret = 24;
	break;
    case PM_TabBarBaseHeight:
	ret = 8;
	break;
    case PM_SpinBoxFrameWidth:
	ret = 1;
	break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;
    case PM_MaximumDragDistance:
	ret = -1;
	break;
    case PM_SliderLength:
	ret = 17;
	break;
    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;
    case PM_TabBarBaseOverlap:
	ret = kThemeTabPaneOverlap;
//	GetThemeMetric(kThemeMetricTabOverlap, &ret);
	break;
    case PM_IndicatorHeight:
	GetThemeMetric(kThemeMetricCheckBoxHeight, &ret);
	break;
    case PM_IndicatorWidth:
	GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
	break;
    case PM_ExclusiveIndicatorHeight:
	GetThemeMetric(kThemeMetricRadioButtonHeight, &ret);
	break;
    case PM_ExclusiveIndicatorWidth:
	GetThemeMetric(kThemeMetricRadioButtonWidth, &ret);
	break;
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
}

QRect QMacStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    const QStyleOption& opt ) const
{
    QRect ret;
    switch(control) {
    case CC_ComboBox: {
	ret = QWindowsStyle::querySubControlMetrics(control, w, sc, opt);
	if(sc == SC_ComboBoxEditField)
	    ret.setWidth(ret.width() - 5);
	break; }
    case CC_ScrollBar: {
	if(!w)
	    break;
	QScrollBar *scrollbar = (QScrollBar *) w;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumScrollBar;
	ttdi.bounds = *qt_glb_mac_rect(w->rect());
	ttdi.min = scrollbar->minValue();
	ttdi.max = scrollbar->maxValue();
	ttdi.value = scrollbar->value();
	ttdi.attributes |= kThemeTrackShowThumb;
	if(scrollbar->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(w->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!scrollbar->isEnabled())
	    ttdi.enableState |= kThemeTrackDisabled;
	ttdi.trackInfo.scrollbar.viewsize = scrollbar->pageStep();
	switch(sc) {
	case SC_ScrollBarGroove: {
	    Rect mrect;
	    GetThemeTrackBounds(&ttdi, &mrect);
	    ret = QRect(mrect.left, mrect.top, 
			mrect.right - mrect.left, mrect.bottom - mrect.top);
	    break; }
	case SC_ScrollBarSlider: {
	    QRegion rgn;
	    GetThemeTrackThumbRgn(&ttdi, rgn.handle(TRUE));
	    ret = rgn.boundingRect();
	    break; }
	default:
	    break;
	}
	break; }
    case CC_Slider: {
	if(!w)
	    break;
	QSlider *sldr = (QSlider *)w;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumSlider;
	ttdi.bounds = *qt_glb_mac_rect(w->rect());
	ttdi.min = sldr->minValue();
	ttdi.max = sldr->maxValue();
	ttdi.value = sldr->valueFromPosition(sldr->sliderStart());
	ttdi.attributes |= kThemeTrackShowThumb;
	if(sldr->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(w->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!sldr->isEnabled())
	    ttdi.enableState |= kThemeTrackDisabled;
	if(sldr->tickmarks() == QSlider::Above)
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
	else
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
	switch(sc) {
	case SC_SliderGroove: {
	    Rect mrect;
	    GetThemeTrackBounds(&ttdi, &mrect);
	    ret = QRect(mrect.left, mrect.top, 
			mrect.right - mrect.left, mrect.bottom - mrect.top);
	    break; }
	case SC_SliderHandle: {
	    QRegion rgn;
	    GetThemeTrackThumbRgn(&ttdi, rgn.handle(TRUE));
	    ret = rgn.boundingRect();
	    break; }
	default:
	    break;
	}
	break; }
    default:
	ret = QWindowsStyle::querySubControlMetrics(control, w, sc, opt);
	break;
    }
    return ret;
}

QRect QMacStyle::subRect( SubRect r, const QWidget *w ) const
{
    QRect ret;
    switch(r) {
    case SR_ProgressBarLabel:
    case SR_ProgressBarGroove:
	break;
    case SR_ProgressBarContents:
	ret = w->rect();
	break;
    default:
	ret = QWindowsStyle::subRect(r, w);
	break;
    }
    return ret;
}

QStyle::SubControl QMacStyle::querySubControl(ComplexControl control,
						 const QWidget *widget,
						 const QPoint &pos,
						 const QStyleOption& opt ) const
{
    SubControl ret = SC_None;
    switch(control) {
    case CC_ScrollBar: {
	if(!widget)
	    break;
	QScrollBar *scrollbar = (QScrollBar *) widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumScrollBar;
	ttdi.bounds = *qt_glb_mac_rect(widget->rect());
	ttdi.min = scrollbar->minValue();
	ttdi.max = scrollbar->maxValue();
	ttdi.value = scrollbar->value();
	ttdi.attributes |= kThemeTrackShowThumb;
	if(scrollbar->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(widget->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!scrollbar->isEnabled())
	    ttdi.enableState |= kThemeTrackDisabled;
	ttdi.trackInfo.scrollbar.viewsize = scrollbar->pageStep();
	Point pt = { pos.y(), pos.x() };
	Rect mrect;
	GetThemeTrackBounds(&ttdi, &mrect);
	ControlPartCode cpc;
	if(HitTestThemeScrollBarArrows(&ttdi.bounds, ttdi.enableState, 
				       0, scrollbar->orientation() == Qt::Horizontal,
				       pt, &mrect, &cpc)) {
	    if(cpc == kControlUpButtonPart)
		ret = SC_ScrollBarSubLine;
	    else if(cpc == kControlDownButtonPart)
		ret = SC_ScrollBarAddLine;
	} else if(HitTestThemeTrack(&ttdi, pt, &cpc)) {
	    if(cpc == kControlPageUpPart)
		ret = SC_ScrollBarSubPage;
	    else if(cpc == kControlPageDownPart)
		ret = SC_ScrollBarAddPage;
	    else
		ret = SC_ScrollBarSlider;
	} 
	break; }
    default:
	ret = QWindowsStyle::querySubControl(control, widget, pos, opt);
	break;
    }
    return ret;
}

int QMacStyle::styleHint(StyleHint sh, const QWidget *w, 
			  const QStyleOption &opt,QStyleHintReturn *d) const
{
    int ret = 0;
    switch(sh) {
    case SH_ScrollBar_StopMouseOverSlider:
        ret = TRUE;
        break;
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
        break;
    case SH_ComboBox_Popup:
        ret = TRUE;
        break;
    case SH_Workspace_FillSpaceOnMaximize:
        ret = TRUE;
        break;
    case SH_Widget_ShareActivation:
        ret = TRUE;
        break;
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignRight;
        break;
    case SH_TabBar_Alignment:
        ret = Qt::AlignHCenter;
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, w, opt, d);
        break;
    }
    return ret;
}

QSize QMacStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       const QStyleOption& opt ) const
{
    QSize sz(contentsSize);
    switch(contents) {
    case CT_TabBarTab: {
	SInt32 lth = kThemeLargeTabHeight;
	if(sz.height() > lth) 
	    sz.setHeight(lth);
	break; }
    case CT_PopupMenuItem: {
	if(!widget || opt.isDefault())
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
		h += 8;
	} else if ( mi->widget() ) {
	} else if (mi->isSeparator()) {
	    w = 10;
	    SInt16 ash;
	    GetThemeMenuSeparatorHeight(&ash);
	    h = ash;
	} else {
	    if (mi->pixmap())
		h = QMAX(h, mi->pixmap()->height() + 4);
	    else
		h = QMAX(h, popup->fontMetrics().height() + 8);

	    if (mi->iconSet() != 0)
		h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						  QIconSet::Normal).height() + 4);
	}

	if (! mi->text().isNull()) {
	    if (mi->text().find('\t') >= 0)
		w += 12;
	}

	if (maxpmw)
	    w += maxpmw + 6;
	if (checkable && maxpmw < 20)
	    w += 20 - maxpmw;
	if (checkable || maxpmw > 0)
	    w += 2;
	w += 12;
	if(widget->parentWidget() && widget->parentWidget()->inherits("QComboBox")) 
	    w = QMAX(w, widget->parentWidget()->width() - 20);
	sz = QSize(w, h);
	break; }
    case CT_PushButton:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	sz = QSize(sz.width() + 16, sz.height()); //###
	break;
    default:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	break;
    }
    return sz;
}

#endif
