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
   spinwidget isn't quite right
   titlebar isn't complete
   slider doesn't repaint properly
*/

#include "qmacstyle_mac.h"

#if defined( Q_WS_MAC ) && !defined( QT_NO_STYLE_AQUA )
#include "qaquastyle_p.h"
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qt_mac.h>
#include <qtabbar.h>
#include "private/qtitlebar_p.h"
#include <Appearance.h>
#include <qbitmap.h>
#include <qprogressbar.h>

#include <qpainter.h>
class QMacPainter : public QPainter
{
public:
    ~QMacPainter();
    void noop() { QPainter::initPaintDevice(TRUE); }
private:
    QMacPainter();
};
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=NULL)
{
    static Rect r;
    QPoint tl(qr.topLeft());
    if(pd && pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    //Qt says be inclusive!
    SetRect(&r, tl.x(), tl.y(), tl.x() + qr.width() - 1, tl.y() + qr.height() - 1);
    return &r;
}

#define private public //ugh, what I'll do..
#include <qslider.h>
#undef private

static int mac_count = 0;
QMacStyle::QMacStyle(  )  : QAquaStyle()
{
    if(!mac_count++)
	RegisterAppearanceClient();
}

QMacStyle::~QMacStyle()
{
    if(!(--mac_count))
	UnregisterAppearanceClient();
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
	QAquaStyle::drawPrimitive( pe, p, r, cg, flags, opt);
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
    if(how & Style_Enabled)
	tds |= kThemeStateActive;
    else
	tds |= kThemeStateInactive;
    if(how & Style_Down)
	tds = kThemeStatePressed;
    
    switch(element) {
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
	if(qAquaActive(cg))
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
	DrawThemeTab(qt_glb_mac_rect(r, p->device()), tts, ttd, NULL, 0);
	break; }
    case CE_PushButton: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	((QMacPainter *)p)->noop();
	DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemePushButton,
			&info, NULL, NULL, NULL, 0);
	break; }
    default:
	QAquaStyle::drawControl(element, p, widget, r, cg, how, opt);
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
    case CC_SpinWidget: {
	QSpinWidget * sw = (QSpinWidget *) widget;
	if ( sub & SC_SpinWidgetFrame )
	    QAquaStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, 
					   SC_SpinWidgetFrame, subActive, opt);
	if((sub & SC_SpinWidgetDown) || (sub & SC_SpinWidgetUp)) {
	    if(subActive == SC_SpinWidgetDown)
		tds |= kThemeStatePressedDown;
	    else if(subActive == SC_SpinWidgetUp)
		tds |= kThemeStatePressedUp;
	    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(sw->upRect() | sw->downRect(), p->device()), 
			    kThemeIncDecButton, &info, NULL, NULL, NULL, 0);
	}
	break; }
#if 0
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
	    twa = kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
	else if(tbar->testWFlags( WStyle_SysMenu)) 
	    twa = kThemeWindowHasCloseBox;
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
	DrawThemeWindowFrame(kThemeUtilityWindow, rect, tds, &twm, twa, NULL, 0);
#endif
	break; }
#endif
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
	if(qAquaActive(cg))
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
	if(qAquaActive(cg))
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
	QAquaStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, opt);
    }
}

int QMacStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret = 0;
    switch(metric) {
    case PM_TabBarTabHSpace:
	ret = kThemeLargeTabHeight;
	break;
    case PM_TabBarBaseOverlap:
	ret = kThemeTabPaneOverlap;
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
	ret = QAquaStyle::pixelMetric(metric, widget);
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
	if(qAquaActive(w->colorGroup()))
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
	if(qAquaActive(w->colorGroup()))
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
	ret = QAquaStyle::querySubControlMetrics(control, w, sc, opt);
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
	ret = QAquaStyle::subRect(r, w);
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
	if(qAquaActive(widget->colorGroup()))
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
	ret = QAquaStyle::querySubControl(control, widget, pos, opt);
	break;
    }
    return ret;
}

#endif
