/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded keyboard drivers
**
** Created : 20020218
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qkbdpc101_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qgfx_qws.h"
#include "qwindowsystem_qws.h"
#include "qnamespace.h"
#include "qapplication.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef Q_OS_LINUX
#include <sys/kd.h>
#include <sys/vt.h>
#endif

static const QWSKeyMap pc101KeyM[] = {
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Escape,     27      , 27      , 0xffff  },
    {   Qt::Key_1,      '1'     , '!'     , 0xffff  },
    {   Qt::Key_2,      '2'     , '@'     , 0xffff  },
    {   Qt::Key_3,      '3'     , '#'     , 0xffff  },
    {   Qt::Key_4,      '4'     , '$'     , 0xffff  },
    {   Qt::Key_5,      '5'     , '%'     , 0xffff  },
    {   Qt::Key_6,      '6'     , '^'     , 0xffff  },
    {   Qt::Key_7,      '7'     , '&'     , 0xffff  },
    {   Qt::Key_8,      '8'     , '*'     , 0xffff  },
    {   Qt::Key_9,      '9'     , '('     , 0xffff  },  // 10
    {   Qt::Key_0,      '0'     , ')'     , 0xffff  },
    {   Qt::Key_Minus,      '-'     , '_'     , 0xffff  },
    {   Qt::Key_Equal,      '='     , '+'     , 0xffff  },
    {   Qt::Key_Backspace,  8       , 8       , 0xffff  },
    {   Qt::Key_Tab,        9       , 9       , 0xffff  },
    {   Qt::Key_Q,      'q'     , 'Q'     , 'Q'-64  },
    {   Qt::Key_W,      'w'     , 'W'     , 'W'-64  },
    {   Qt::Key_E,      'e'     , 'E'     , 'E'-64  },
    {   Qt::Key_R,      'r'     , 'R'     , 'R'-64  },
    {   Qt::Key_T,      't'     , 'T'     , 'T'-64  },  // 20
    {   Qt::Key_Y,      'y'     , 'Y'     , 'Y'-64  },
    {   Qt::Key_U,      'u'     , 'U'     , 'U'-64  },
    {   Qt::Key_I,      'i'     , 'I'     , 'I'-64  },
    {   Qt::Key_O,      'o'     , 'O'     , 'O'-64  },
    {   Qt::Key_P,      'p'     , 'P'     , 'P'-64  },
    {   Qt::Key_BraceLeft,  '['     , '{'     , 0xffff  },
    {   Qt::Key_Escape,     ']'     , '}'     , 0xffff  },
    {   Qt::Key_Return,     13      , 13      , 0xffff  },
    {   Qt::Key_Control,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_A,      'a'     , 'A'     , 'A'-64  },  // 30
    {   Qt::Key_S,      's'     , 'S'     , 'S'-64  },
    {   Qt::Key_D,      'd'     , 'D'     , 'D'-64  },
    {   Qt::Key_F,      'f'     , 'F'     , 'F'-64  },
    {   Qt::Key_G,      'g'     , 'G'     , 'G'-64  },
    {   Qt::Key_H,      'h'     , 'H'     , 'H'-64  },
    {   Qt::Key_J,      'j'     , 'J'     , 'J'-64  },
    {   Qt::Key_K,      'k'     , 'K'     , 'K'-64  },
    {   Qt::Key_L,      'l'     , 'L'     , 'L'-64  },
    {   Qt::Key_Semicolon,  ';'     , ':'     , 0xffff  },
    {   Qt::Key_Apostrophe, '\''    , '"'     , 0xffff  },  // 40
    {   Qt::Key_QuoteLeft,  '`'     , '~'     , 0xffff  },
    {   Qt::Key_Shift,      0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Backslash,  '\\'    , '|'     , 0xffff  },
    {   Qt::Key_Z,      'z'     , 'Z'     , 'Z'-64  },
    {   Qt::Key_X,      'x'     , 'X'     , 'X'-64  },
    {   Qt::Key_C,      'c'     , 'C'     , 'C'-64  },
    {   Qt::Key_V,      'v'     , 'V'     , 'V'-64  },
    {   Qt::Key_B,      'b'     , 'B'     , 'B'-64  },
    {   Qt::Key_N,      'n'     , 'N'     , 'N'-64  },
    {   Qt::Key_M,      'm'     , 'M'     , 'M'-64  },  // 50
    {   Qt::Key_Comma,      ','     , '<'     , 0xffff  },
    {   Qt::Key_Period,     '.'     , '>'     , 0xffff  },
    {   Qt::Key_Slash,      '/'     , '?'     , 0xffff  },
    {   Qt::Key_Shift,      0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Asterisk,   '*'     , '*'     , 0xffff  },
    {   Qt::Key_Alt,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Space,      ' '     , ' '     , 0xffff  },
    {   Qt::Key_CapsLock,   0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F1,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F2,     0xffff  , 0xffff  , 0xffff  },  // 60
    {   Qt::Key_F3,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F4,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F5,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F6,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F7,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F8,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F9,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F10,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_NumLock,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_ScrollLock, 0xffff  , 0xffff  , 0xffff  },  // 70
    {   Qt::Key_7,      '7'     , '7'     , 0xffff  },
    {   Qt::Key_8,      '8'     , '8'     , 0xffff  },
    {   Qt::Key_9,      '9'     , '9'     , 0xffff  },
    {   Qt::Key_Minus,      '-'     , '-'     , 0xffff  },
    {   Qt::Key_4,      '4'     , '4'     , 0xffff  },
    {   Qt::Key_5,      '5'     , '5'     , 0xffff  },
    {   Qt::Key_6,      '6'     , '6'     , 0xffff  },
    {   Qt::Key_Plus,       '+'     , '+'     , 0xffff  },
    {   Qt::Key_1,      '1'     , '1'     , 0xffff  },
    {   Qt::Key_2,      '2'     , '2'     , 0xffff  },  // 80
    {   Qt::Key_3,      '3'     , '3'     , 0xffff  },
    {   Qt::Key_0,      '0'     , '0'     , 0xffff  },
    {   Qt::Key_Period,     '.'     , '.'     , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F11,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F12,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },  // 90
    {   0,          0xffff  , 0xffff  , 0xffff  }
};

static const int keyMSize = sizeof(pc101KeyM)/sizeof(QWSKeyMap)-1;

//===========================================================================

//
// PC-101 type keyboards
//

QWSPC101KeyboardHandler::QWSPC101KeyboardHandler()
{
    shift = false;
    alt   = false;
    ctrl  = false;
    extended = false;
    prevuni = 0;
    prevkey = 0;
    caps = FALSE;
}

QWSPC101KeyboardHandler::~QWSPC101KeyboardHandler()
{
}

const QWSKeyMap *QWSPC101KeyboardHandler::keyMap() const
{
    return pc101KeyM;
}

#ifdef Q_OS_QNX6
void QWSPC101KeyboardHandler::doKey(uchar code){};
void QWSQnxKeyboardHandler::doKey(uchar code) 
#else
void QWSPC101KeyboardHandler::doKey(uchar code)
#endif
{
    int keyCode = Qt::Key_unknown;
    bool release = false;
    int keypad = 0;
    bool softwareRepeat = FALSE;

    if (code == 224) {
	// extended
	extended = true;
	return;
    }

    if (code & 0x80) {
	release = true;
	code &= 0x7f;
    }

    if (extended) {
	switch (code) {
	case 72:
	    keyCode = Qt::Key_Up;
	    break;
	case 75:
	    keyCode = Qt::Key_Left;
	    break;
	case 77:
	    keyCode = Qt::Key_Right;
	    break;
	case 80:
	    keyCode = Qt::Key_Down;
	    break;
	case 82:
	    keyCode = Qt::Key_Insert;
	    break;
	case 71:
	    keyCode = Qt::Key_Home;
	    break;
	case 73:
	    keyCode = Qt::Key_Prior;
	    break;
	case 83:
	    keyCode = Qt::Key_Delete;
	    break;
	case 79:
	    keyCode = Qt::Key_End;
	    break;
	case 81:
	    keyCode = Qt::Key_Next;
	    break;
	case 28:
	    keyCode = Qt::Key_Enter;
	    break;
	case 53:
	    keyCode = Qt::Key_Slash;
	    break;
	}
    } else {
	if (code < keyMSize) {
	    keyCode = pc101KeyM[code].key_code;
	}

#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
	softwareRepeat = TRUE;

	switch (code) {
	    case 0x7a: case 0x7b: case 0x7c: case 0x7d:
		keyCode = code - 0x7a + Key_F9;
		softwareRepeat = FALSE;
		break;
	    case 0x79:
		keyCode = Key_SysReq;
		softwareRepeat = FALSE;
		break;
	    case 0x78:
		keyCode = Key_Escape;
		softwareRepeat = FALSE;
		break;
	    case 0x60:
		keyCode = Key_Return;
		break;
	    case 0x67:
		keyCode = Key_Right;
		break;
	    case 0x69:
		keyCode = Key_Up;
		break;
	    case 0x6a:
		keyCode = Key_Down;
		break;
	    case 0x6c:
		keyCode = Key_Left;
		break;
	}

	if ( qt_screen->isTransformed()
		&& keyCode >= Qt::Key_Left && keyCode <= Qt::Key_Down )
	{
	    keyCode = transformDirKey(keyCode);
	}
#endif
    }

    /*
      Keypad consists of extended keys 53 and 28,
      and non-extended keys 55 and 71 through 83.
    */
    if ( extended ? (code == 53 || code == 28) :
	 (code == 55 || ( code >= 71 && code <= 83 )) )
	keypad = Qt::Keypad;

    // Ctrl-Alt-Backspace exits qws
    if (ctrl && alt && keyCode == Qt::Key_Backspace) {
	qApp->quit();
    }

    if (keyCode == Qt::Key_Alt) {
	alt = !release;
    } else if (keyCode == Qt::Key_Control) {
	ctrl = !release;
    } else if (keyCode == Qt::Key_Shift) {
	shift = !release;
    } else if ( keyCode == Qt::Key_CapsLock && release ) {
	caps = !caps;
#if defined(Q_OS_LINUX)
	char leds;
	ioctl(0, KDGETLED, &leds);
	leds = leds & ~LED_CAP;
	if ( caps ) leds |= LED_CAP;
	ioctl(0, KDSETLED, leds);
#endif
    }
    if (keyCode != Qt::Key_unknown) {
	bool bAlt = alt;
	bool bCtrl = ctrl;
	bool bShift = shift;
	int unicode = 0;
	if (code < keyMSize) {
	    if (!extended) {
		bool bCaps = shift ||
		    (caps ? QChar(keyMap()[code].unicode).isLetter() : FALSE);
		if (bCtrl)
		    unicode =  keyMap()[code].ctrl_unicode ?  keyMap()[code].ctrl_unicode : 0xffff;
		else if (bCaps)
		    unicode =  keyMap()[code].shift_unicode ?  keyMap()[code].shift_unicode : 0xffff;
		else
		    unicode =  keyMap()[code].unicode ?  keyMap()[code].unicode : 0xffff;
	    } else {
		if ( code == 53 )
		    unicode = '/';
	    }
	}

	modifiers = 0;
	if ( bAlt ) modifiers |= Qt::AltButton;
	if ( bCtrl ) modifiers |= Qt::ControlButton;
	if ( bShift ) modifiers |= Qt::ShiftButton;
	if ( keypad ) modifiers |= Qt::Keypad;

	// looks wrong -- WWA
	bool repeat = FALSE;
	if (prevuni == unicode && prevkey == keyCode && !release)
	    repeat = TRUE;

	processKeyEvent( unicode, keyCode, modifiers, !release, repeat );

	if (!release) {
	    prevuni = unicode;
	    prevkey = keyCode;
	} else {
	    prevkey = prevuni = 0;
	}
    }

    if ( softwareRepeat && !release )
	beginAutoRepeat( prevuni, prevkey, modifiers );
    else
	endAutoRepeat();

    extended = false;
}

#endif // QT_NO_QWS_KEYBOARD

