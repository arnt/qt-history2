/*  $Id: tvbrowse.h,v 1.2 1999/03/19 10:56:33 willy Exp $

    Xbase project source code

    This file contains declarations for TurboVision interface classes
    for Xbase DBMS library.

    Copyright (C) 1998,1999 Vitaly Fedrushkov <fedrushkov@acm.org>
    www   - http://www.startech.keller.tx.us/xbase.html

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., or see http://www.gnu.org/.
*/

#ifndef __XB_TVBROWSE_H__
#define __XB_TVBROWSE_H__

#define Uses_TObject
#define Uses_TListViewer
#define Uses_TRect
#define Uses_TScrollBar
#define Uses_TWindow
#include <tvision/tv.h>
#include <xdb/xbase.h>

const int cmDataFocused		= 301;

class xbTvBrowseField : private TObject
{
private:
	short	Width;
	char	*Name;
	static const char	fillChar;
public:
	xbTvBrowseField(short, const char *);
	xbTvBrowseField(xbDbf *, short);
	~xbTvBrowseField();
	virtual short getWidth();
	virtual char *getName()  { return Name;  };
	virtual void  getName(char *, short);
	virtual void  setWidth(short);
	virtual void  setName(const char *);
};

class xbTvBrowseCaption;

class xbTvBrowse : public TListViewer {
public:
	xbTvBrowse(xbDbf *);
	virtual void handleEvent(TEvent&);
	virtual void focusItem( short item );
	virtual void draw();
	virtual void addField(xbTvBrowseField *);
	virtual void setRange();
	TPalette& getPalette() const;
	friend 	xbTvBrowseCaption;
protected:
	static const char emptyText[];
	virtual short getFieldCount();
	virtual short getWidth(short);
	virtual void getText(char *, long, short, short);
	virtual void getFieldName(char *, short, short);
	virtual void populate();
private:
	xbDbf	*Data;
	static const char fieldSep;
	TNSCollection *Fields;
};

#define cpBrowse "\x03\x03\x04\x05\x06"

class xbTvBrowseCaption: public TView
{
private:
	xbTvBrowse *Browse;
	static const char fieldSep;
public:
	xbTvBrowseCaption(const TRect &, xbTvBrowse*);
	virtual void draw();
};

class xbTvBrowseWindow : public TWindow 
{
public:
	xbTvBrowseWindow(const TRect &, char *, xbTvBrowse *, int=0);
	~xbTvBrowseWindow() {
		delete listBox;
		delete listScroller;
	}
	virtual void handleEvent(TEvent &);
	virtual void setRange() { listBox->setRange(); }
	xbTvBrowse *listBox;
protected:
private:
	TScrollBar *listScroller;
};

#endif // __XB_TVBROWSE_H__

/*
 * Local Variables:
 * mode: c++
 */
