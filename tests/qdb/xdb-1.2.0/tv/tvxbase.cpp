/*  $Id: tvxbase.cpp,v 1.2 1999/03/19 10:56:33 willy Exp $

    Xbase project source code

    This file contains example program for TurboVision interface classes
    for Xbase DBMS library.

    Copyright (C) 1998,1999 Vitaly Fedrushkov <fedrushkov@acm.org>
    www   - http://www.startech.keller.tx.us/xbase.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., or see http://www.gnu.org/.
*/

#include <iostream.h>

#define Uses_TApplication
#define Uses_TButton
#define Uses_TDeskTop
#define Uses_TDialog
#define Uses_TEvent
#define Uses_TFileDialog
#define Uses_TKeys
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TRect
#define Uses_TStaticText
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#include <tvision/tv.h>
#include <xdb/xbase.h>

#include "tvbrowse.h"

// Interface classes

const int cmHelpAbout	 = 350;
const int cmOpenDbf	 = 351;

class xbTvApp : public TApplication
{
public:
	xbTvApp();
private:
	static		TStatusLine *initStatusLine(TRect r);
	static		TMenuBar *initMenuBar(TRect r);
	virtual void	handleEvent(TEvent& event);
	void		DialogHelpAbout();
	void		open(const char *);
	void		openDbf(char*);
	xbTvBrowseWindow	*Browse;
	xbXBase			*XBase;
};

xbTvApp::xbTvApp() :
	TProgInit(xbTvApp::initStatusLine,
		   xbTvApp::initMenuBar,
		   xbTvApp::initDeskTop)
{
	TEvent ev;
	
	XBase = new xbXBase();
	ev.what = evCommand;
	ev.message.command = cmHelpAbout;
	TProgram::putEvent(ev);
}

TStatusLine *
xbTvApp::initStatusLine(TRect r)
{
	r.a.y = r.b.y - 1;
	return new TStatusLine(r,
	*new TStatusDef(0, 0xFFFF) +
	    *new TStatusItem(0, kbF10, cmMenu) +
	    *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
	    *new TStatusItem("~F3~ Open", kbF3, cmOpen) +
	    *new TStatusItem("~Alt-F3~ Close", kbAltF3, cmClose)
	);
}

TMenuBar *
xbTvApp::initMenuBar(TRect r)
{
	r.b.y = r.a.y + 1;
	return new TMenuBar(r,
	*new TSubMenu("~F~ile", kbAltF)+
            *new TMenuItem("~O~pen", cmOpen, kbF3, hcNoContext, "F3")+
			newLine()+
            *new TMenuItem("E~x~it", cmQuit, kbAltX, hcNoContext, "Alt-X")+
	*new TSubMenu("~H~elp", kbAltH)+
            *new TMenuItem("~A~bout", cmHelpAbout, kbNoKey, hcNoContext)
       );
}

void 
xbTvApp::handleEvent(TEvent& event)
{
	TApplication::handleEvent(event);
	if(event.what == evCommand) {
		switch(event.message.command) {
		case cmHelpAbout:
			DialogHelpAbout();
			break;
		case cmOpen:
			open("*.dbf");
			break;
		case cmOpenDbf:
			openDbf((char*)event.message.infoPtr);
			free(event.message.infoPtr);
			break;
		default:
			return;
		}
		clearEvent(event);
	}
}

#ifndef __MSDOS__
#define ABOUT_HEIGHT	21
#else
#define ABOUT_HEIGHT	18
#endif

void 
xbTvApp::DialogHelpAbout()
{
	TDialog *pd = new TDialog(TRect(10, 2, 70, ABOUT_HEIGHT), 
				  "About Xbase+TV Browse demo");
	if (pd) {
		TRect r = pd->getExtent();
		r.grow(-3, -2);
		r.b.y = r.a.y + 4;
		pd->insert(new TStaticText(r, "\03Xbase DBMS library\n \n"
			"Copyright (C) 1997  StarTech, Gary A. Kunkel\n"
			"Please visit http://www.startech.keller.tx.us/xbase/"));
		r.move(0,5);
		pd->insert(new TStaticText(r, 
			"Turbo Vision interface classes for Xbase\n"
			"Copyright (C) 1998,1999  Vitaly V Fedrushkov"));
		r.move(0,3);
		pd->insert(new TStaticText(r, "Turbo Vision library\n"
			"Copyright (c) 1991, 1994 by Borland International"));
#ifndef __MSDOS__
		r.move(0,3);
		pd->insert(new TStaticText(r, "TurboVision for UNIX\n"
			"All changes copyright (c) 1997 Sergio Sigala"));
#endif
		r.move(0,2);
		r.grow(-22,-1);
		pd->insert(new TButton(r, "~O~K", cmOK, bfDefault));
		pd->options |= ofCentered;
		deskTop->execView(pd);
	}
	destroy(pd);
}

void
xbTvApp::open(const char *fileSpec)
{
	TFileDialog *d= (TFileDialog *)validView(
		new TFileDialog(fileSpec, "Open a File", "~N~ame", 
				fdOpenButton, 100 ));

	if (d != 0 && deskTop->execView(d) != cmCancel) {
		char fileName[PATH_MAX];
		TEvent ev;

		d->getFileName( fileName );
		ev.what = evCommand;
		ev.message.command = cmOpenDbf;
		ev.message.infoPtr = strdup(fileName);
		TProgram::putEvent(ev);
	}
	destroy(d);
}
void
xbTvApp::openDbf(char *name)
{
	TRect r = deskTop->getExtent();
	xbDbf *db = new xbDbf(XBase);
	xbTvBrowse *br;

	db->OpenDatabase(name);
	br = new xbTvBrowse(db);
	Browse = new xbTvBrowseWindow(r, name, br);
	if (validView(Browse)) {
		Browse->eventMask |= evBroadcast;
		deskTop->insert(Browse);
	}
}

xbTvApp	*MainApp;

int
main()
{
	MainApp = new xbTvApp();
	MainApp->run();
	delete MainApp;
	return 0;
}
