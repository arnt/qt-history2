/****************************************************************************
** $Id: $
**
** Implementation of Motif-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmacstyle_mac.h"

#if defined( Q_WS_MAC ) && !defined( QT_NO_STYLE_AQUA )
#include "qaquastyle_p.h"
#include <qpushbutton.h>
#include <qt_mac.h>
#include <Appearance.h>

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

static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd)
{
    static Rect r;
    QPoint tl(qr.topLeft());
    if(pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    //Qt says be inclusive!
    SetRect(&r, tl.x(), tl.y(), tl.x() + qr.width() - 1, tl.y() + qr.height() - 1);
    return &r;
}

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
    if(!qAquaActive(cg))
	tds |= kThemeStateInactive;
    else
	tds |= kThemeStateActive;
    if(flags & Style_Down)
	tds = kThemeStatePressed;

    switch(pe) {
    case PE_HeaderArrow: 
    case PE_HeaderSection: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_Sunken)
	    info.value = kThemeButtonOn;
	if(pe == PE_HeaderArrow && (flags & Style_Up))
	    info.adornment |= kThemeAdornmentArrowUpArrow;
	((QMacPainter *)p)->noop();
	DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemeListHeaderButton, 
			&info, NULL, NULL, NULL, 0);
	break; }
#if 0
    case PE_ExclusiveIndicatorMask: 
    case PE_ExclusiveIndicator: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_On)
	    info.value = kThemeButtonOn;
	if(pe == PE_ExclusiveIndicator) {
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemeRadioButton, 
			    &info, NULL, NULL, NULL, 0);
	} else {
	    p->save();
	    QRegion rgn;
	    GetThemeButtonRegion(qt_glb_mac_rect(r, p->device()), kThemeRadioButton,
				 &info, rgn.handle(TRUE));
	    p->setClipRegion(rgn);
	    p->fillRect(r, black);
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
	    ((QMacPainter *)p)->noop();
	    DrawThemeButton(qt_glb_mac_rect(r, p->device()), kThemeCheckBox,
			    &info, NULL, NULL, NULL, 0);
	} else {
	    p->save();
	    QRegion rgn;
	    GetThemeButtonRegion(qt_glb_mac_rect(r, p->device()), kThemeCheckBox,
				 &info, rgn.handle(TRUE));
	    p->setClipRegion(rgn);
	    p->fillRect(r, black);
	    p->restore();
	}
	break; }
#endif
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
    if(!qAquaActive(cg))
	tds |= kThemeStateInactive;
    else
	tds |= kThemeStateActive;
    if(how & Style_Down)
	tds = kThemeStatePressed;
    
    switch(element) {
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

#endif
