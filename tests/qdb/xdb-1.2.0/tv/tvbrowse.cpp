/*  $Id: tvbrowse.cpp,v 1.2 1999/03/19 10:56:33 willy Exp $

    Xbase project source code

    This file contains a set of TurboVision interface classes for
    Xbase DBMS library.

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

#include <xdb/xbase.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iomanip.h>
#include <strstream.h>

#define Uses_TEvent
#include <tvision/tv.h>

#include "tvbrowse.h"

const char xbTvBrowseField::fillChar = '\xB1';
const char xbTvBrowseCaption::fieldSep = '\xB3';

xbTvBrowseField::xbTvBrowseField(short aWidth, const char *aName)
{
	Width = 0;
	Name = NULL;
	setWidth(aWidth);
	setName(aName);
}

xbTvBrowseField::xbTvBrowseField(xbDbf *dbf, xbShort field)
{
	Width = 0;
	Name = NULL;
	setWidth(dbf->GetFieldLen(field));
	setName(dbf->GetFieldName(field));
}

xbTvBrowseField::~xbTvBrowseField()
{
	if (NULL != Name) {
		free(Name);
	}
}

void
xbTvBrowseField::setWidth(short aWidth) 
{
	Width = aWidth;
}

short
xbTvBrowseField::getWidth()
{
	return Width;
}

void
xbTvBrowseField::setName(const char *aName)
{
	if (NULL != Name) {
		free(Name);
	}
	if (aName) {
		Name = strdup(aName);
	}
}

void
xbTvBrowseField::getName(char *Text, short maxlen)
{
	short l = strlen(Name);
	short i = max(0, (maxlen - l - 2)/2);

	memset(Text, fillChar, maxlen);
	strncpy(Text+i+1, Name, min(l, maxlen));
	Text[i] = Text[i+l+1] = ' ';
	Text[maxlen+1] = '\0';
}

xbTvBrowseCaption::xbTvBrowseCaption(const TRect &r, xbTvBrowse *browse):
	TView(r)
{
	Browse = browse;
}

void
xbTvBrowseCaption::draw()
{
	int curCol;
	int j;
	int indent;
	short colWidth;
	ushort color;
	TDrawBuffer b;

	indent = Browse->hScrollBar ? Browse->hScrollBar->value : 0;
	curCol = 0;
	b.moveChar(0, ' ', getColor(1), size.x);
	color = getColor(1);
	for (j = indent; j < Browse->getFieldCount(); j++) {
		char text[256];
		char buf[256];
		colWidth = Browse->getWidth(j);
		Browse->getFieldName(text, j, colWidth);
		memmove(buf, text, colWidth);
		buf[colWidth] = EOS;
		b.moveStr(curCol+1, buf, color);
		curCol += colWidth + 1;
		b.moveChar(curCol, fieldSep, color, 1);
		if (curCol > size.x) {
			break;
		}
	}
	writeLine(0, 0, size.x, 1, b);
}


const char xbTvBrowse::emptyText[] = "<<< No data >>>";
const char xbTvBrowse::fieldSep = '\xB3'; // = '|';

xbTvBrowse::xbTvBrowse(xbDbf *data)
	:TListViewer(TRect(0, 0, 1, 1), 1, NULL, NULL)
{
	Fields = new TNSCollection(10, 10);
	Data = data;
	populate();
	setRange();
}

void 
xbTvBrowse::handleEvent(TEvent& event)
{
	if (event.what == evMouseDown) {
		message(owner, evBroadcast, cmListItemSelected, this);
		clearEvent(event);
	}
	TListViewer::handleEvent(event);
}

short
xbTvBrowse::getWidth(short field)
{
	return ((xbTvBrowseField*)(Fields->at(field)))->getWidth();
}

short
xbTvBrowse::getFieldCount()
{
	return Fields->getCount();
}

void
xbTvBrowse::draw()
{
	int curCol;
	int i, j;
	int indent;
	ccIndex item;
	short colWidth;
	ushort color;
	ushort focusedColor = getColor(3);
	ushort normalColor;
	ushort selectedColor;
	TDrawBuffer b;
	uchar scOff;

	if ((state&(sfSelected | sfActive)) == (sfSelected | sfActive)) {
		normalColor = getColor(1);
		focusedColor = getColor(3);
		selectedColor = getColor(4);
	} else {
		normalColor = getColor(2);
		selectedColor = getColor(4);
	}

	indent = hScrollBar ? hScrollBar->value : 0;
	curCol = 0;
	writeLine(0, 0, size.x, 1, b);
	for (i = 0; i < size.y; i++) {
		curCol = 0;
		b.moveChar(0, ' ', normalColor, size.x);
		item =  i + topItem;
		if ((state & (sfSelected | sfActive)) ==
		    (sfSelected | sfActive) &&
		    focused == item &&
		    range > 0) {
			color = focusedColor;
			setCursor(1, i);
			scOff = 0;
		} else if (item < range && isSelected(item)) {
			color = selectedColor;
			scOff = 2;
		} else {
			color = normalColor;
			scOff = 4;
		}

		if (item < range) {
			for (j = indent; j < getFieldCount(); j++) {
				colWidth = getWidth(j);
				char text[256];
				getText(text, item, j, colWidth);
				char buf[256];
				memmove(buf, text, colWidth);
				buf[colWidth] = EOS;
				b.moveStr(curCol+1, buf, color);
				curCol += colWidth + 1;
				b.moveChar(curCol, fieldSep, getColor(5), 1);
				if (curCol > size.x) {
					break;
				}
			}
			if(showMarkers) {
				b.putChar(0, specialChars[scOff]);
				b.putChar(size.x, specialChars[scOff+1]);
			} else {
				b.putChar(size.x, ' ');
			}
		} else if(i == 1) {
			b.moveStr(max(0, (size.x - strlen(emptyText)) / 2), 
				  emptyText, getColor(1));
		}
		writeLine(0, i, size.x, 1, b);
	}
}

void
xbTvBrowse::getFieldName(char *Text, short field, short maxlen)
{
	((xbTvBrowseField*)(Fields->at(field)))->getName(Text, maxlen);
}

void
xbTvBrowse::getText(char *Text, long item, short field, short maxlen)
{
	Data->GetRecord(xbULong(item));
	Data->GetField(xbShort(field), Text);
	return;
}

TPalette& 
xbTvBrowse::getPalette() const 
{
	static TPalette palette(cpBrowse, sizeof(cpBrowse)-1);
	return palette;
}

void 
xbTvBrowse::focusItem(short item)
{
	TListViewer::focusItem(item);
	if (item) {
		message(owner, evBroadcast, cmDataFocused, Data);
	}
}

void
xbTvBrowse::addField(xbTvBrowseField *field)
{
	Fields->insert(field);
}

void
xbTvBrowse::setRange()
{
	TListViewer::setRange(short(Data->NoOfRecords()));
}

void
xbTvBrowse::populate()
{
	short i;

	for (i = 0; i < Data->FieldCount(); i++) {
		addField(new xbTvBrowseField(Data, i));
	}
}


xbTvBrowseWindow::xbTvBrowseWindow(const TRect &rect, char *title, 
			     xbTvBrowse *aBrowse, int BottomGap)
	:TWindow(rect, title, wnNoNumber), 
	 TWindowInit(TWindow::initFrame)
{
	listScroller = new TScrollBar(TRect(size.x - 1, 1, size.x,
					    size.y - 1 - BottomGap));
	listScroller->growMode = gfGrowLoX|gfGrowHiX|gfGrowHiY;

	listBox = aBrowse;
	listBox->setBounds(TRect(1, 2, size.x - 1, size.y - 1 - BottomGap));
	listBox->growMode = gfGrowHiX|gfGrowHiY;
	listBox->vScrollBar = listScroller;
	listBox->setRange();

	insert(aBrowse);
	insert(listScroller);
	insert(new xbTvBrowseCaption(TRect(1, 1, size.x - 1, 2), aBrowse));
	listBox->focusItem(0);
}

void 
xbTvBrowseWindow::handleEvent(TEvent &event) 
{
	switch(event.what) {
	case evBroadcast:
		switch (event.message.command) {
//		case cmListItemSelected:
//			clearEvent(event);
//			break;
		}
	}
	TWindow::handleEvent(event);
}
