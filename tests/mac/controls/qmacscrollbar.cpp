/****************************************************************************
**
** Implementation of a custom QMacControl.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qmacscrollbar.h"
ControlActionUPP QMacScrollBar::actionUPP = NULL;

QMacScrollBar::QMacScrollBar(QWidget *p, const char *n) : QMacControl(p, n)
{
    Rect r;
    SetRect(&r, 0, 0, 20, 200);
    ControlRef scrollbar;
    CreateScrollBarControl((WindowPtr)handle(), &r, 0, 0, 100, 0, false, nil, &scrollbar);
    setControl(scrollbar);
}

void
QMacScrollBar::trackControlEvent(QMacTrackEvent *te)
{
    ControlRef control;
    Point where = { te->y(), te->x() };
    SInt16 part = FindControl( where, (WindowPtr)handle(), &control );
    ControlActionUPP upp = NULL;
    if(part == kControlUpButtonPart || part == kControlDownButtonPart ||
       part == kControlPageUpPart || part == kControlPageDownPart) {
	if(!actionUPP)
	    actionUPP = NewControlActionUPP(QMacScrollBar::actionCallbk);
	upp = actionUPP;
    } else if(part ==  kControlIndicatorPart) {
	upp = (ControlActionUPP)-1L;
    }
    if(upp) {
	te->accept();
	TrackControl( control, where, upp );
    }
}

QMAC_PASCAL void
QMacScrollBar::actionCallbk(ControlRef control, ControlPartCode part)
{
    switch (part) {
    case kControlUpButtonPart:
	if(GetControlValue(control) > GetControlMinimum(control))
	    SetControlValue(control, GetControlValue(control) - 1);
	break;
    case kControlDownButtonPart:
	if(GetControlValue(control) < GetControlMaximum(control))
	    SetControlValue(control, GetControlValue(control) + 1);
	break;
    case kControlPageUpPart:
	if(GetControlValue(control) > GetControlMinimum(control))
	    SetControlValue(control, GetControlValue(control) - 10);
	break;
    case kControlPageDownPart:
	if(GetControlValue(control) < GetControlMaximum(control))
	    SetControlValue(control, GetControlValue(control) + 10);
	break;
    }			
}
