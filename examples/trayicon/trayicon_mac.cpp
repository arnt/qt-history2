/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "trayicon.h"
#include "Carbon/Carbon.h"
#include <unistd.h>

static EventTypeSpec trayicon_events[] = {
    { kEventClassWindow, kEventWindowExpand },
    { kEventClassWindow, kEventWindowGetDockTileMenu }
};
static QMAC_PASCAL OSStatus qt_trayicon_event(EventHandlerCallRef, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    if(eclass == kEventClassWindow) {
	if(ekind == kEventWindowExpand) {
	    qDebug("double click?");
	    return noErr;
	} else if(ekind == kEventWindowGetDockTileMenu) {
	    qDebug("popup click?");
	    return noErr;
	}
    }
    return eventNotHandledErr;
}

class TrayIcon::TrayIconPrivate
{
public:
    //funcs..
    TrayIconPrivate();
    ~TrayIconPrivate();
	
    //all
    static int counter;
    static EventHandlerUPP eventUPP;
    //individual
    WindowRef hd;
    EventHandlerRef hd_ev;
};
int TrayIcon::TrayIconPrivate::counter = 0;
EventHandlerUPP TrayIcon::TrayIconPrivate::eventUPP = NULL;

TrayIcon::TrayIconPrivate::TrayIconPrivate() : hd(NULL), hd_ev(NULL) { 
    Rect r;
    SetRect(&r, 0, 0, 0, 0);
    if(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes, &r, &hd) == noErr) {
	ChangeWindowAttributes(hd, 0, kWindowInWindowMenuAttribute);
	ShowHide(hd, 1);
	CollapseWindow(hd, TRUE);
	if(!counter++) 
	    eventUPP = NewEventHandlerUPP(qt_trayicon_event);
	InstallWindowEventHandler((WindowRef)hd, eventUPP, GetEventTypeCount(trayicon_events),
				  trayicon_events, (void *)this, &hd_ev);
    }
}

TrayIcon::TrayIconPrivate::~TrayIconPrivate() {
    if(!--counter) {
	DisposeEventHandlerUPP(eventUPP);
	eventUPP = NULL;
    }
    if(hd_ev) {
	RemoveEventHandler(hd_ev);
	hd_ev = NULL;
    }
    if(hd) {
	DisposeWindow(hd);
	hd = NULL;
    }
}

void TrayIcon::sysInstall()
{
    d = new TrayIconPrivate;
}

void TrayIcon::sysRemove()
{
    delete d;
    d = NULL;
}

void TrayIcon::sysUpdateIcon()
{
#if 0
    if(d && d->hd) {
	QImage i = pm.convertToImage().convertDepth(32).smoothScale(40, 40);
	for(int y = 0; y < i.height(); y++) {
	    uchar *l = i.scanLine(y);
	    for(int x = 0; x < i.width(); x+=4)
		*(l+x) = 255;
	}
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef dp = CGDataProviderCreateWithData(NULL, i.bits(), i.numBytes(), NULL);
	CGImageRef ir = CGImageCreate(i.width(), i.height(), 8, 32, i.bytesPerLine(),
				      cs, kCGImageAlphaNoneSkipFirst, dp,
				      0, 0, kCGRenderingIntentDefault);
	//cleanup
	SetApplicationDockTileImage(ir);
	CGImageRelease(ir);
	CGColorSpaceRelease(cs);
	CGDataProviderRelease(dp);
    }
#endif
}

void TrayIcon::sysUpdateToolTip()
{
    if(d && d->hd) {
    	CFStringRef str = CFStringCreateWithCharacters(NULL, (UniChar *)tip.unicode(), tip.length());
	SetWindowTitleWithCFString((WindowPtr)d->hd, str);
    }
}

