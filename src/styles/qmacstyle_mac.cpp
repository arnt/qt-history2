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

static inline const Rect *mac_rect(const QRect &qr)
{
    static Rect r;
    SetRect(&r, qr.left(), qr.top(), qr.right()+1, qr.bottom()+1); //qt says be inclusive!
    return &r;
}
static inline const Rect *mac_rect(const QPoint &qp, const QSize &qs) 
{ return mac_rect(QRect(qp, qs)); }


QMacStyle::QMacStyle(  )  : QAquaStyle()
{
}

QMacStyle::~QMacStyle()
{
}

/*! \reimp */
void QMacStyle::drawPrimitive( PrimitiveElement pe,
			       QPainter *p,
			       const QRect &r,
			       const QColorGroup &cg,
			       SFlags flags,
			       const QStyleOption& opt ) const
{
    QAquaStyle::drawPrimitive( pe, p, r, cg, flags, opt);
}


void QMacStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags how,
				 const QStyleOption& opt ) const
{
    SFlags flags = Style_Default;
    if (widget->isEnabled())
	flags |= Style_Enabled;

    switch(element) {
    case CE_PushButton: {
	ThemeButtonDrawInfo info;
	if(!qAquaActive(cg))
	    info.state |= kThemeStateInactive;
	else
	    info.state |= kThemeStateActive;
	if(how & Style_Down)
	    info.state = kThemeStatePressed;
	info.value = 0;
	info.adornment = kThemeAdornmentNone;
	DrawThemeButton(mac_rect(r), kThemePushButton, &info, NULL, NULL, NULL, 0);
	break; }
    default:
	QAquaStyle::drawControl(element, p, widget, r, cg, how, opt);
    }
}

#endif
