/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded keyboard drivers
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
#include "qwindowsystem_qws.h"
#include "qwsutils_qws.h"
#include "qgfx_qws.h"

#include "qapplication.h"
#include "qsocketnotifier.h"
#include "qnamespace.h"
#include "qtimer.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#ifdef _OS_LINUX_
#include <linux/kd.h>
#endif
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#ifndef QT_NO_QWS_KEYBOARD

#ifdef Q_OS_QNX6
#include "qwskeyboard_qnx.cpp"
#endif

#ifdef QT_QWS_YOPY
#include <qwidgetlist.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include <linux/yopy_button.h>
extern "C" {
    int getpgid(int);
}
#endif

#include <termios.h>
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX6)
#include <sys/kd.h>
#include <sys/vt.h>
#endif

#ifdef QT_QWS_CUSTOM
#include <asm/sharp_char.h>
#endif


#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
static int xform_dirkey(int key)
{
    int xf = qt_screen->transformOrientation();
    return (key-Qt::Key_Left+xf)%4+Qt::Key_Left;
}
#endif

#define VTSWITCHSIG SIGUSR2

static bool vtActive = true;
static int  vtQws = 0;
static int  kbdFD = -1;

class QWSyopyButtonsHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSyopyButtonsHandler();
    virtual ~QWSyopyButtonsHandler();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    struct termios newT, oldT;
    QSocketNotifier *notifier;
};

#ifdef QT_QWS_CUSTOM
static const QWSServer::KeyMap keyM[] = {
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 00
    {	Qt::Key_A,		'a'     , 'A'     , 'A'-64  }, // 01
    {	Qt::Key_B,		'b'     , 'B'     , 'B'-64  }, // 02
    {	Qt::Key_C,		'c'     , 'C'     , 'C'-64  }, // 03
    {	Qt::Key_D,		'd'     , 'D'     , 'D'-64  }, // 04
    {	Qt::Key_E,		'e'     , 'E'     , 'E'-64  }, // 05
    {	Qt::Key_F,		'f'     , 'F'     , 'F'-64  }, // 06
    {	Qt::Key_G,		'g'     , 'G'     , 'G'-64  }, // 07
    {	Qt::Key_H,		'h'     , 'H'     , 'H'-64  }, // 08
    {	Qt::Key_I,		'i'     , 'I'     , 'I'-64  }, // 09
    {	Qt::Key_J,		'j'     , 'J'     , 'J'-64  }, // 0a 10
    {	Qt::Key_K,		'k'     , 'K'     , 'K'-64  }, // 0b
    {	Qt::Key_L,		'l'     , 'L'     , 'L'-64  }, // 0c
    {	Qt::Key_M,		'm'     , 'M'     , 'M'-64  }, // 0d
    {	Qt::Key_N,		'n'     , 'N'     , 'N'-64  }, // 0e
    {	Qt::Key_O,		'o'     , 'O'     , 'O'-64  }, // 0f
    {	Qt::Key_P,		'p'     , 'P'     , 'P'-64  }, // 10
    {	Qt::Key_Q,		'q'     , 'Q'     , 'Q'-64  }, // 11
    {	Qt::Key_R,		'r'     , 'R'     , 'R'-64  }, // 12
    {	Qt::Key_S,		's'     , 'S'     , 'S'-64  }, // 13
    {	Qt::Key_T,		't'     , 'T'     , 'T'-64  }, // 14 20
    {	Qt::Key_U,		'u'     , 'U'     , 'U'-64  }, // 15
    {	Qt::Key_V,		'v'     , 'V'     , 'V'-64  }, // 16
    {	Qt::Key_W,		'w'     , 'W'     , 'W'-64  }, // 17
    {	Qt::Key_X,		'x'     , 'X'     , 'X'-64  }, // 18
    {	Qt::Key_Y,		'y'     , 'Y'     , 'Y'-64  }, // 19
    {	Qt::Key_Z,		'z'     , 'Z'     , 'Z'-64  }, // 1a
    {	Qt::Key_Shift,		0xffff  , 0xffff  , 0xffff  }, // 1b
    {	Qt::Key_Return,		13      , 13      , 0xffff  }, // 1c
    {	Qt::Key_F11,	        0xffff  , 0xffff  , 0xffff  }, // 1d todo
    {	Qt::Key_F22,		0xffff  , 0xffff  , 0xffff  }, // 1e 30
    {	Qt::Key_Backspace,	8       , 8       , 0xffff  }, // 1f
    {	Qt::Key_F31,		0xffff  , 0xffff  , 0xffff  }, // 20
    {	Qt::Key_F35,		0xffff  , 0xffff  , 0xffff  }, // 21 light
    {	Qt::Key_Escape,		0xffff  , 0xffff  , 0xffff  }, // 22

    // Direction key code are for *UNROTATED* display.
    {	Qt::Key_Up,		0xffff  , 0xffff  , 0xffff  }, // 23
    {	Qt::Key_Right,		0xffff  , 0xffff  , 0xffff  }, // 24
    {	Qt::Key_Left,		0xffff  , 0xffff  , 0xffff  }, // 25
    {	Qt::Key_Down,		0xffff  , 0xffff  , 0xffff  }, // 26

    {	Qt::Key_F33,		0xffff  , 0xffff  , 0xffff  }, // 27 OK
    {	Qt::Key_F12,		0xffff  , 0xffff  , 0xffff  }, // 28 40 home
    {	Qt::Key_1,		'1'     , 'q'     , 'Q'-64  }, // 29
    {	Qt::Key_2,		'2'     , 'w'     , 'W'-64  }, // 2a
    {	Qt::Key_3,		'3'     , 'e'     , 'E'-64  }, // 2b
    {	Qt::Key_4,		'4'     , 'r'     , 'R'-64  }, // 2c
    {	Qt::Key_5,		'5'     , 't'     , 'T'-64  }, // 2d
    {	Qt::Key_6,		'6'     , 'y'     , 'Y'-64  }, // 2e
    {	Qt::Key_7,		'7'     , 'u'     , 'U'-64  }, // 2f
    {	Qt::Key_8,		'8'     , 'i'     , 'I'-64  }, // 30
    {	Qt::Key_9,		'9'     , 'o'     , 'O'-64  }, // 31
    {	Qt::Key_0,		'0'     , 'p'     , 'P'-64  }, // 32 50
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 33
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 34
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 35
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 36
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 37
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 38
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 39
    {	Qt::Key_Minus,		'-'     , 'b'     , 'B'-64  }, // 3a
    {	Qt::Key_Plus,		'+'     , 'n'     , 'N'-64  }, // 3b
    {	Qt::Key_CapsLock,	0xffff  , 0xffff  , 0xffff  }, // 3c 60
    {	Qt::Key_At,		'@'     , 's'     , 'S'-64  }, // 3d
    {	Qt::Key_Question,	'?'     , '?'     , 0xffff  }, // 3e
    {	Qt::Key_Comma,		','     , ','     , 0xffff  }, // 3f
    {	Qt::Key_Period,		'.'     , '.'     , 0xffff  }, // 40
    {	Qt::Key_Tab,		9       , '\\'    , 0xffff  }, // 41
    {	Qt::Key_X,		0xffff 	, 'x'  	  , 'X'-64  }, // 42
    {	Qt::Key_C,		0xffff 	, 'c'     , 'C'-64  }, // 43
    {	Qt::Key_V,		0xffff 	, 'v'     , 'V'-64  }, // 44
    {	Qt::Key_Slash,		'/'     , '/'     , 0xffff  }, // 45
    {	Qt::Key_Apostrophe,	'\''    , '\''    , 0xffff  }, // 46 70
    {	Qt::Key_Semicolon,	';'     , ';'     , 0xffff  }, // 47
    {	Qt::Key_QuoteDbl,	'\"'    , '\"'    , 0xffff  }, // 48
    {	Qt::Key_Colon,		':'     , ':'     , 0xffff  }, // 49
    {	Qt::Key_NumberSign,	'#'     , 'd'     , 'D'-64  }, // 4a
    {	Qt::Key_Dollar,		'$'     , 'f'     , 'F'-64  }, // 4b
    {	Qt::Key_Percent,	'%'     , 'g'     , 'G'-64  }, // 4c
    {	Qt::Key_Underscore,	'_'     , 'h'     , 'H'-64  }, // 4d
    {	Qt::Key_Ampersand,	'&'     , 'j'     , 'J'-64  }, // 4e
    {	Qt::Key_Asterisk,	'*'     , 'k'     , 'K'-64  }, // 4f
    {	Qt::Key_ParenLeft,	'('     , 'l'     , 'L'-64  }, // 50 80
    {	Qt::Key_Delete,		'['     , '['     , '['     }, // 51
    {	Qt::Key_Z,		0xffff 	, 'z'     , 'Z'-64  }, // 52
    {	Qt::Key_Equal,		'='     , 'm'     , 'M'-64  }, // 53
    {	Qt::Key_ParenRight,	')'     , ']'     , ']'     }, // 54
    {	Qt::Key_AsciiTilde,	'~'     , '^'     , '^'     }, // 55
    {	Qt::Key_Less,		'<'     , '{'     , '{'     }, // 56
    {	Qt::Key_Greater,	'>'     , '}'     , '}'     }, // 57
    {	Qt::Key_F9,		0xffff  , 0xffff  , 0xffff  }, // 58 datebook
    {	Qt::Key_F10,		0xffff  , 0xffff  , 0xffff  }, // 59 address
    {	Qt::Key_F13,	        0xffff  , 0xffff  , 0xffff  }, // 5a 90 email
    {	Qt::Key_F30,		' '      , ' '    , 0xffff  }, // 5b select
    {	Qt::Key_Space,		' '     , '|'     , '`'     }, // 5c
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 5d
    {	Qt::Key_Exclam,		'!'     , 'a'     , 'A'-64  }, // 5e
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 5f
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 60
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 61
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 62
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 63
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 64
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 65
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 66
    {	Qt::Key_Meta,		0xffff  , 0xffff  , 0xffff  }, // 67
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 68
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 69
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6a
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6b
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6c
    {	Qt::Key_F34,		0xffff  , 0xffff  , 0xffff  }, // 6d power
    {	Qt::Key_F13,		0xffff  , 0xffff  , 0xffff  }, // 6e mail long
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6f
    {	Qt::Key_NumLock,	0xffff  , 0xffff  , 0xffff  }, // 70
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 71
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 72
    { 	0x20ac,	0xffff  , 0x20ac , 0x20ac }, // 73 Euro sign
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 74
    {	Qt::Key_F32,		0xffff  , 0xffff  , 0xffff  }, // 75 Sync
    {	0,			0xffff  , 0xffff  , 0xffff  }
};
#else
// Standard PC101
static const QWSServer::KeyMap keyM[] = {
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
#endif
static const int keyMSize = sizeof(keyM)/sizeof(QWSServer::KeyMap)-1;

/*!
  \class QWSKeyboardHandler qkeyboard_qws.h
  \ingroup qws
  \brief The QWSKeyboardHandler class implements the keyboard
  driver/handler for Qt/Embedded.

  The keyboard driver/handler handles events from system devices and
  generates key events.

  A QWSKeyboardHandler will usually open some system device in its
  constructor, create a QSocketNotifier on that opened device and when
  it receives data, it will call processKeyEvent() to send the event
  to Qt/Embedded for relaying to clients.
*/


/*!
  Constructs a keyboard handler. The handler \e may be passed to
  the system for later destruction with QWSServer::setKeyboardHandler(),
  although even without doing this, the handler can function,
  calling processKeyEvent() to emit events.
*/
QWSKeyboardHandler::QWSKeyboardHandler()
{
}

/*!
  Destroys a keyboard handler. Note that if you have called
  QWSServer::setKeyboardHandler(), you may not delete the handler.
*/
QWSKeyboardHandler::~QWSKeyboardHandler()
{
}


/*!
  Subclasses call this function to send a key event. The server may
  additionally filter the event before sending it on to applications.

  \list
  \i \a unicode is the Unicode value for the key, or 0xFFFF is none is appropriate.
  \i \a keycode is the Qt keycode for the key (see \l{Qt::Key}).
       for the list of codes).
  \i \a modifiers is the set of modifier keys (see \l{Qt::Modifier}).
  \i \a isPress says whether this is a press or a release.
  \i \a autoRepeat says whether this event was generated by an auto-repeat
	    mechanism, or an actual key press.
  \endlist
*/
void QWSKeyboardHandler::processKeyEvent(int unicode, int keycode, int modifiers,
			bool isPress, bool autoRepeat)
{
    qwsServer->processKeyEvent( unicode, keycode, modifiers, isPress, autoRepeat );
}

class QWSPC101KeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSPC101KeyboardHandler();
    virtual ~QWSPC101KeyboardHandler();

    void doKey(uchar scancode);

private slots:
    void autoRepeat();
private:
    bool shift;
    bool alt;
    bool ctrl;
#if defined(QT_QWS_CUSTOM)
    bool meta;
    bool fn;
    bool numLock;
#endif
    bool caps;
    bool extended;
    int modifiers;
    int prevuni;
    int prevkey;

#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    // Could be used by other subclasses
    QTimer* repeater;
    int repeatdelay, repeatperiod;
#endif
};

void QWSPC101KeyboardHandler::autoRepeat()
{
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    processKeyEvent( prevuni, prevkey, modifiers, FALSE, TRUE );
    processKeyEvent( prevuni, prevkey, modifiers, TRUE, TRUE );
    repeater->start(repeatperiod);
#endif
}


class QWSTtyKeyboardHandler : public QWSPC101KeyboardHandler
{
    Q_OBJECT
public:
    QWSTtyKeyboardHandler();
    virtual ~QWSTtyKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    struct termios origTermData;
};

class QWSUsbKeyboardHandler : public QWSPC101KeyboardHandler
{
    Q_OBJECT
public:
    QWSUsbKeyboardHandler();
    virtual ~QWSUsbKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    int fd;
};

class QWSVr41xxButtonsHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSVr41xxButtonsHandler();
    virtual ~QWSVr41xxButtonsHandler();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};


static void vtSwitchHandler(int /*sig*/)
{
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX6)
    if (vtActive) {
	qwsServer->enablePainting(false);
	qt_screen->save();
	if (ioctl(kbdFD, VT_RELDISP, 1) == 0) {
	    vtActive = false;
	    qwsServer->closeMouse();
	}
	else {
	    qwsServer->enablePainting(true);
	}
	usleep(200000);
    }
    else {
	if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
	    qwsServer->enablePainting(true);
	    vtActive = true;
	    qt_screen->restore();
	    qwsServer->openMouse();
	    qwsServer->refresh();
	}
    }
    signal(VTSWITCHSIG, vtSwitchHandler);
#endif
}

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
#if defined(QT_QWS_CUSTOM)
    meta = FALSE;
    fn = FALSE;

    numLock = FALSE;
    sharp_kbdctl_modifstat  st;
    int dev = ::open("/dev/sharp_kbdctl", O_RDWR);
    if( dev >= 0 ) {
	memset(&st, 0, sizeof(st));
	st.which = 3;
	int ret = ioctl(dev, SHARP_KBDCTL_GETMODIFSTAT, (char*)&st);
	if( !ret )
	    numLock = (bool)st.stat;
	::close(dev);
    }
#endif
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    repeatdelay = 400;
    repeatperiod = 80;
    repeater = new QTimer(this);
    connect(repeater, SIGNAL(timeout()), this, SLOT(autoRepeat()));
#endif
}

QWSPC101KeyboardHandler::~QWSPC101KeyboardHandler()
{
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
#if !defined(QT_QWS_CUSTOM)
    if (code == 224) {
	// extended
	extended = true;
	return;
    }
#endif

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
#if defined(QT_QWS_CUSTOM)
	if ( fn && !meta && (code >= 0x42 && code <= 0x52) ) {
	    ushort unicode=0;
	    int scan=0;
	    if ( code == 0x42 ) { unicode='X'-'@'; scan=Key_X; } // Cut
	    else if ( code == 0x43 ) { unicode='C'-'@'; scan=Key_C; } // Copy
	    else if ( code == 0x44 ) { unicode='V'-'@'; scan=Key_V; } // Paste
	    else if ( code == 0x52 ) { unicode='Z'-'@'; scan=Key_Z; } // Undo
	    if ( scan ) {
		processKeyEvent( unicode, scan, ControlButton, !release, FALSE );
		return;
	    }
	}
#endif
	if (code < keyMSize) {
	    keyCode = QWSServer::keyMap()[code].key_code;
	}

#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX) // need autorepeat implemented here?
	bool repeatable = TRUE;

#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX) && !defined(QT_QWS_CUSTOM)
	switch (code) {
	    case 0x7a: case 0x7b: case 0x7c: case 0x7d:
		keyCode = code - 0x7a + Key_F9;
		repeatable = FALSE;
		break;
	    case 0x79:
		keyCode = Key_SysReq;
		repeatable = FALSE;
		break;
	    case 0x78:
		keyCode = Key_Escape;
		repeatable = FALSE;
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
#endif

#if defined(QT_QWS_CUSTOM)
	if ( release && ( keyCode == Key_F34 || keyCode == Key_F35 ) )
	    return; // no release for power and light keys
	if ( keyCode >= Key_F1 && keyCode <= Key_F35
		|| keyCode == Key_Escape || keyCode == Key_Home
		|| keyCode == Key_Shift || keyCode == Key_Meta )
	    repeatable = FALSE;
#endif

	if ( qt_screen->isTransformed()
		&& keyCode >= Qt::Key_Left && keyCode <= Qt::Key_Down )
	{
	    keyCode = xform_dirkey(keyCode);
	}

	if ( repeatable && !release )
	    repeater->start(repeatdelay,TRUE);
	else
	    repeater->stop();
#endif
    }

    /*
      Keypad consists of extended keys 53 and 28,
      and non-extended keys 55 and 71 through 83.
    */
    if ( extended ? (code == 53 || code == 28) :
	 (code == 55 || ( code >= 71 && code <= 83 )) )
	keypad = Qt::Keypad;

    // Virtual console switching
    int term = 0;
    if (ctrl && alt && keyCode >= Qt::Key_F1 && keyCode <= Qt::Key_F10)
	term = keyCode - Qt::Key_F1 + 1;
    else if (ctrl && alt && keyCode == Qt::Key_Left)
	term = QMAX(vtQws - 1, 1);
    else if (ctrl && alt && keyCode == Qt::Key_Right)
	term = QMIN(vtQws + 1, 10);
    if (term && !release) {
	ctrl = false;
	alt = false;
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX6)
	ioctl(kbdFD, VT_ACTIVATE, term);
#endif
	return;
    }

#if defined(QT_QWS_CUSTOM)
    // Ctrl-Alt-Delete exits qws
    if (ctrl && alt && keyCode == Qt::Key_Delete) {
	qApp->quit();
    }
#else
    // Ctrl-Alt-Backspace exits qws
    if (ctrl && alt && keyCode == Qt::Key_Backspace) {
	qApp->quit();
    }
#endif

#if defined(QT_QWS_CUSTOM)
    if (keyCode == Qt::Key_F22) { /* Fn key */
	fn = !release;
    } else if ( keyCode == Key_NumLock ) {
	if ( release )
	    numLock = !numLock;
    } else
#endif

    if (keyCode == Qt::Key_Alt) {
	alt = !release;
    } else if (keyCode == Qt::Key_Control) {
	ctrl = !release;
    } else if (keyCode == Qt::Key_Shift) {
	shift = !release;
#if defined(QT_QWS_CUSTOM)
    } else if (keyCode == Qt::Key_Meta) {
	meta = !release;
#endif
    } else if ( keyCode == Qt::Key_CapsLock && release ) {
	caps = !caps;
#if defined(_OS_LINUX_) && !defined(QT_QWS_CUSTOM)
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
#if !defined(QT_QWS_CUSTOM)
                bool bCaps = shift ||
		    (caps ? QChar(QWSServer::keyMap()[code].unicode).isLetter() : FALSE);
#else
                bool bCaps = caps ^ shift;
		if (fn) {
		    if ( shift ) {
			bCaps = bShift = FALSE;
			bCtrl = TRUE;
		    }
		    if ( meta ) {
			bCaps = bShift = TRUE;
			bAlt = TRUE;
		    }
		} else if ( meta ) {
		    bCaps = bShift = TRUE;
		}
		if ( code > 40 && caps ) {
		    // fn-keys should only react to shift, not caps
		    bCaps = bShift = shift;
		}
		if ( numLock ) {
		    if ( keyCode != Key_Space && keyCode != Key_Tab )
			bCaps = bShift = FALSE;
		}
		if ( keyCode == Key_Delete && (bAlt || bCtrl) ) {
		    keyCode = Key_BraceLeft;
		    unicode = '[';
		    bCaps = bShift = bAlt = bCtrl = FALSE;
		} else if (keyCode == Qt::Key_F31 && bCtrl) {
		    keyCode = Key_QuoteLeft;
		    unicode = '`';
		} else
#endif
		
		if (bCtrl)
		    unicode =  QWSServer::keyMap()[code].ctrl_unicode ?  QWSServer::keyMap()[code].ctrl_unicode : 0xffff;
		else if (bCaps)
		    unicode =  QWSServer::keyMap()[code].shift_unicode ?  QWSServer::keyMap()[code].shift_unicode : 0xffff;
		else
		    unicode =  QWSServer::keyMap()[code].unicode ?  QWSServer::keyMap()[code].unicode : 0xffff;
	    } else {
		if ( code == 53 )
		    unicode = '/';
	    }
	}

	modifiers = 0;
	if ( bAlt ) modifiers |= AltButton;
	if ( bCtrl ) modifiers |= ControlButton;
	if ( bShift ) modifiers |= ShiftButton;
	if ( keypad ) modifiers |= Keypad;

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
    extended = false;
}


//
// Tty keyboard
//

QWSTtyKeyboardHandler::QWSTtyKeyboardHandler()
{
    kbdFD=open("/dev/tty0", O_RDWR | O_NDELAY, 0);

    if ( kbdFD >= 0 ) {
	QSocketNotifier *notifier;
	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }

    // save for restore.
    tcgetattr( kbdFD, &origTermData );

    struct termios termdata;
    tcgetattr( kbdFD, &termdata );

#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX6)
    ioctl(kbdFD, KDSKBMODE, K_RAW);
#endif

    termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termdata.c_oflag = 0;
    termdata.c_cflag = CREAD | CS8;
    termdata.c_lflag = 0;
    termdata.c_cc[VTIME]=0;
    termdata.c_cc[VMIN]=1;
    cfsetispeed(&termdata, 9600);
    cfsetospeed(&termdata, 9600);
    tcsetattr(kbdFD, TCSANOW, &termdata);

    signal(VTSWITCHSIG, vtSwitchHandler);

#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX6)
    struct vt_mode vtMode;
    ioctl(kbdFD, VT_GETMODE, &vtMode);

    // let us control VT switching
    vtMode.mode = VT_PROCESS;
    vtMode.relsig = VTSWITCHSIG;
    vtMode.acqsig = VTSWITCHSIG;
    ioctl(kbdFD, VT_SETMODE, &vtMode);

    struct vt_stat vtStat;
    ioctl(kbdFD, VT_GETSTATE, &vtStat);
    vtQws = vtStat.v_active;
#endif
}

QWSTtyKeyboardHandler::~QWSTtyKeyboardHandler()
{
    if (kbdFD >= 0)
    {
#if !defined(Q_OS_FREEBSD) && !defined(Q_OS_SOLARIS) && !defined(Q_OS_QNX6)
	ioctl(kbdFD, KDSKBMODE, K_XLATE);
#endif
	tcsetattr(kbdFD, TCSANOW, &origTermData);
	::close(kbdFD);
	kbdFD = -1;
    }
}

void QWSTtyKeyboardHandler::readKeyboardData()
{
    unsigned char buf[81];
    int n = read(kbdFD, buf, 80 );
    for ( int loop = 0; loop < n; loop++ )
	doKey(buf[loop]);
}


/* USB driver */

QWSUsbKeyboardHandler::QWSUsbKeyboardHandler()
{
    fd = open(getenv("QWS_USB_KEYBOARD"),O_RDONLY, 0);
    if ( fd >= 0 ) {
	QSocketNotifier *notifier;
	notifier = new QSocketNotifier( fd, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSUsbKeyboardHandler::~QWSUsbKeyboardHandler()
{
    close(fd);
}

struct Myinputevent {

    unsigned int dummy1;
    unsigned int dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int value;

};

void QWSUsbKeyboardHandler::readKeyboardData()
{
    Myinputevent event;
    int n = read(fd, &event, sizeof(Myinputevent) );
    if ( n != 16 )
	return;
    int key=event.code;
    if(key==103) {
	processKeyEvent( 0, Qt::Key_Up, 0, event.value!=0, false );
    } else if(key==106) {
	processKeyEvent( 0, Qt::Key_Right, 0, event.value!=0, false  );
    } else if(key==108) {
	processKeyEvent( 0, Qt::Key_Down, 0, event.value!=0, false );
    } else if(key==105) {
	processKeyEvent( 0, Qt::Key_Left, 0, event.value!=0, false );
    } else {
	if(event.value==0) {
	    key=key | 0x80;
	}
	doKey(key);
    }
}

/*
 * YOPY buttons driver
 * Contributed by Ron Victorelli (victorrj at icubed.com)
 */

QWSyopyButtonsHandler::QWSyopyButtonsHandler() : QWSKeyboardHandler()
{
#ifdef QT_QWS_YOPY
    terminalName = "/dev/tty1";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0) {
	qFatal("Cannot open %s\n", terminalName.latin1());
    } else {

       tcsetpgrp(buttonFD, getpgid(0));

       /* put tty into "straight through" mode.
       */
       if (tcgetattr(buttonFD, &oldT) < 0) {
           qFatal("Linux-kbd: tcgetattr failed");
       }

       newT = oldT;
       newT.c_lflag &= ~(ICANON | ECHO  | ISIG);
       newT.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
       newT.c_iflag |= IGNBRK;
       newT.c_cc[VMIN]  = 0;
       newT.c_cc[VTIME] = 0;


       if (tcsetattr(buttonFD, TCSANOW, &newT) < 0) {
           qFatal("Linux-kbd: TCSANOW tcsetattr failed");
       }

       if (ioctl(buttonFD, KDSKBMODE, K_MEDIUMRAW) < 0) {
           qFatal("Linux-kbd: KDSKBMODE tcsetattr failed");
       }

	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
#endif
}

QWSyopyButtonsHandler::~QWSyopyButtonsHandler()
{
#ifdef QT_QWS_YOPY
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
#endif
}

void QWSyopyButtonsHandler::readKeyboardData()
{
#ifdef QT_QWS_YOPY
    uchar buf[1];
    char c='1';
    int fd;

    int n=read(buttonFD,buf,1);
    if (n<0) {
	qDebug("Keyboard read error %s",strerror(errno));
    } else {
	uint code = buf[0]&YPBUTTON_CODE_MASK;
        bool press = !(buf[0]&0x80);
        // printf("Key=%d/%d/%d\n",buf[1],code,press);
        int k=(-1);
        switch(code) {
          case 39:       k=Qt::Key_Up;     break;
          case 44:       k=Qt::Key_Down;   break;
          case 41:       k=Qt::Key_Left;   break;
          case 42:       k=Qt::Key_Right;  break;
          case 56:       k=Qt::Key_F1;     break; //windows
          case 29:       k=Qt::Key_F2;     break; //cycle
          case 24:       k=Qt::Key_F3;     break; //record
          case 23:       k=Qt::Key_F4;     break; //mp3
          case 4:        k=Qt::Key_F5;     break; // PIMS
          case 1:        k=Qt::Key_Escape; break; // Escape
          case 40:       k=Qt::Key_Up;     break; // prev
          case 45:       k=Qt::Key_Down;   break; // next
          case 35:       if( !press ) {
                           fd = open("/proc/sys/pm/sleep",O_RDWR,0);
                           if( fd >= 0 ) {
                               write(fd,&c,sizeof(c));
                               close(fd);
                               //
                               // Updates all widgets.
                               //
                               QWidgetList  *list = QApplication::allWidgets();
                               QWidgetListIt it( *list );          // iterate over the widgets
                               QWidget * w;
                               while ( (w=it.current()) != 0 ) {   // for each widget...
                                 ++it;
                                 w->update();
                               }
                               delete list;
                               // qApp->desktop()->repaint();
                           }
                         }
                         break;

          default: k=(-1); break;
        }

	if ( k >= 0 ) {
		qwsServer->processKeyEvent( 0, k, 0, press, false );
	}
    }
#endif
}


/*
 * vr41xx buttons driver
 */

QWSVr41xxButtonsHandler::QWSVr41xxButtonsHandler() : QWSKeyboardHandler()
{
#ifdef QT_QWS_CASSIOPEIA
    terminalName = "/dev/buttons";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	qWarning("Cannot open %s\n", terminalName.latin1());
    }

    if ( buttonFD >= 0 ) {
	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }

    kbdBufferLen = 80;
    kbdBuffer = new unsigned char [kbdBufferLen];
    kbdIdx = 0;
#endif
}

QWSVr41xxButtonsHandler::~QWSVr41xxButtonsHandler()
{
#ifdef QT_QWS_CASSIOPEIA
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
    delete notifier;
    notifier = 0;
    delete [] kbdBuffer;
#endif
}

void QWSVr41xxButtonsHandler::readKeyboardData()
{
#ifdef QT_QWS_CASSIOPEIA
    int n = 0;
    do {
	n  = read(buttonFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx );
	if ( n > 0 )
	    kbdIdx += n;
    } while ( n > 0 );

    int idx = 0;
    while ( kbdIdx - idx >= 2 ) {
	unsigned char *next = kbdBuffer + idx;
	unsigned short *code = (unsigned short *)next;
	int keycode = Qt::Key_unknown;
	switch ( (*code) & 0x0fff ) {
	    case 0x7:
		keycode = Qt::Key_Up;
		break;
	    case 0x9:
		keycode = Qt::Key_Right;
		break;
	    case 0x8:
		keycode = Qt::Key_Down;
		break;
	    case 0xa:
		keycode = Qt::Key_Left;
		break;
	    case 0x3:
		keycode = Qt::Key_Up;
		break;
	    case 0x4:
		keycode = Qt::Key_Down;
		break;
	    case 0x1:
		keycode = Qt::Key_Return;
		break;
	    case 0x2:
		keycode = Qt::Key_F4;
		break;
	    default:
		qDebug("Unrecognised key sequence %d", (int)code );
	}
	if ( (*code) & 0x8000 )
	    processKeyEvent( 0, keycode, 0, FALSE, FALSE );
	else
	    processKeyEvent( 0, keycode, 0, TRUE, FALSE );
/*
	unsigned short t = *code;
	for ( int i = 0; i < 16; i++ ) {
	    keycode = (t & 0x8000) ? Qt::Key_1 : Qt::Key_0;
	    int unicode = (t & 0x8000) ? '1' : '0';
	    processKeyEvent( unicode, keycode, 0, TRUE, FALSE );
	    processKeyEvent( unicode, keycode, 0, FALSE, FALSE );
	    t <<= 1;
	}
	keycode = Qt::Key_Space;
//	processKeyEvent( ' ', keycode, 0, TRUE, FALSE );
//	processKeyEvent( ' ', keycode, 0, FALSE, FALSE );
*/
	idx += 2;
    }

    int surplus = kbdIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
#endif
}


/*
 * keyboard driver instantiation
 */

QWSKeyboardHandler *QWSServer::newKeyboardHandler( const QString &spec )
{
    QWSKeyboardHandler *handler = 0;

#ifdef Q_OS_QNX6
    handler = new QWSQnxKeyboardHandler();
#else
    if ( spec == "Buttons" ) {
#if defined(QT_QWS_YOPY)
	handler = new QWSyopyButtonsHandler();
#elif defined(QT_QWS_CASSIOPEIA)
	handler = new QWSVr41xxButtonsHandler();
#endif
    } else if ( spec == "TTY" ) {
	if(getenv("QWS_USB_KEYBOARD")) {
	    handler = new QWSUsbKeyboardHandler();
	} else {
	    handler = new QWSTtyKeyboardHandler();
	}
    } else {
	qWarning( "Keyboard type %s unsupported", spec.latin1() );
    }
#endif // Q_OS_QNX6
    return handler;
}

#include "qkeyboard_qws.moc"

const QWSServer::KeyMap *QWSServer::keyMap()
{
    return keyM;
}

#endif // QT_NO_QWS_KEYBOARD

