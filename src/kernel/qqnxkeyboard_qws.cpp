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

#if defined(Q_OS_QNX6) && !defined(QT_NO_QWS_KEYBOARD)

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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "qqnxkeyboard_qws.moc"

#include <sys/dcmd_input.h> 

#include <qkeyboard_qws.h>

// QNX keyboard handler
class QWSQnxKeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSQnxKeyboardHandler();
    ~QWSQnxKeyboardHandler();
 
    void doKey(uchar);
 
public slots:
        void readKbdData(int);

private:
    int shift;
    int alt;
    int ctrl;
    bool extended;
    int modifiers;
    int prevuni;
    int prevkey;

        int kbdFD;
    QList<QSocketNotifier> notifiers; 
};

QWSQnxKeyboardHandler::QWSQnxKeyboardHandler() {
    shift = 0;
    alt   = 0;
    ctrl  = 0;
    extended = false;
    prevuni = 0;
    prevkey = 0;

        kbdFD = open("/dev/devi/keyboard0", O_RDONLY);
 
    if (kbdFD == -1) {
        qFatal("Cannot access keyboard device\n");
    }
 
    QSocketNotifier *kbdNotifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this );
    connect(kbdNotifier, SIGNAL(activated(int)),this, SLOT(readKbdData(int)));
    notifiers.append( kbdNotifier ); 
}

void QWSQnxKeyboardHandler::readKbdData(int fd) {
        _keyboard_packet *packet = (_keyboard_packet *)malloc(sizeof(_keyboard_packet));
        read(fd, (void *)packet, sizeof(_keyboard_packet));
        doKey(packet->data.key_scan);
        free((void *)packet);
}

QWSQnxKeyboardHandler::~QWSQnxKeyboardHandler() {
        close(kbdFD);
}

QWSKeyboardHandler *QWSServer::newKeyboardHandler( const QString &spec )
{
    QWSKeyboardHandler *handler = 0;
        handler = new QWSQnxKeyboardHandler();
    return handler;
}

void QWSQnxKeyboardHandler::doKey(uchar code)
{
printf("code = %hhd shift = %d ctrl = %d\n",code,shift,ctrl);
    int keyCode = Qt::Key_unknown;
    bool release = false;
    int keypad = 0;
 
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
    if (code < 90) {
        keyCode = QWSServer::keyMap()[code].key_code;
    }
    }
 
//      Keypad consists of extended keys 53 and 28,
    // Ctrl-Alt-Backspace exits qws
    if (ctrl && alt && keyCode == Qt::Key_Backspace) {
    qApp->quit();
    }
 
    if (keyCode == Qt::Key_Alt) {
    alt = release ? 0 : AltButton;
    }
    else if (keyCode == Qt::Key_Control) {
    ctrl = release ? 0 : ControlButton;
    }
    else if (keyCode == Qt::Key_Shift) {
    shift = release ? 0 : ShiftButton;
    }
    if (keyCode != Qt::Key_unknown) {
    int unicode = 0;
    if (code < 90) {
        if (!extended) {
        if (shift)
            unicode =  QWSServer::keyMap()[code].shift_unicode ?  QWSServer::keyMap()[code].shift_unicode : 0xffff;
        else if (ctrl)
            unicode =  QWSServer::keyMap()[code].ctrl_unicode ?  QWSServer::keyMap()[code].ctrl_unicode : 0xffff;
        else
            unicode =  QWSServer::keyMap()[code].unicode ?  QWSServer::keyMap()[code].unicode : 0xffff;
        //printf("unicode: %c\r\n", unicode);
        } else {
        if ( code == 53 )
            unicode = '/';
        }
    }
 
    modifiers = alt | ctrl | shift | keypad;
 
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

QWSKeyboardHandler::~QWSKeyboardHandler() { }

void QWSKeyboardHandler::processKeyEvent(int unicode, int keycode, int modifiers,
            bool isPress, bool autoRepeat)
{
    qwsServer->processKeyEvent( unicode, keycode, modifiers, isPress, autoRepeat );
}

static const QWSServer::KeyMap keyM[] = {
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Escape,		27      , 27      , 0xffff  },
    {	Qt::Key_1,		'1'     , '!'     , 0xffff  },
    {	Qt::Key_2,		'2'     , '@'     , 0xffff  },
    {	Qt::Key_3,		'3'     , '#'     , 0xffff  },
    {	Qt::Key_4,		'4'     , '$'     , 0xffff  },
    {	Qt::Key_5,		'5'     , '%'     , 0xffff  },
    {	Qt::Key_6,		'6'     , '^'     , 0xffff  },
    {	Qt::Key_7,		'7'     , '&'     , 0xffff  },
    {	Qt::Key_8,		'8'     , '*'     , 0xffff  },
    {	Qt::Key_9,		'9'     , '('     , 0xffff  },	// 10
    {	Qt::Key_0,		'0'     , ')'     , 0xffff  },
    {	Qt::Key_Minus,		'-'     , '_'     , 0xffff  },
    {	Qt::Key_Equal,		'='     , '+'     , 0xffff  },
    {	Qt::Key_Backspace,	8       , 8       , 0xffff  },
    {	Qt::Key_Tab,		9       , 9       , 0xffff  },
    {	Qt::Key_Q,		'q'     , 'Q'     , 'Q'-64  },
    {	Qt::Key_W,		'w'     , 'W'     , 'W'-64  },
    {	Qt::Key_E,		'e'     , 'E'     , 'E'-64  },
    {	Qt::Key_R,		'r'     , 'R'     , 'R'-64  },
    {	Qt::Key_T,		't'     , 'T'     , 'T'-64  },  // 20
    {	Qt::Key_Y,		'y'     , 'Y'     , 'Y'-64  },
    {	Qt::Key_U,		'u'     , 'U'     , 'U'-64  },
    {	Qt::Key_I,		'i'     , 'I'     , 'I'-64  },
    {	Qt::Key_O,		'o'     , 'O'     , 'O'-64  },
    {	Qt::Key_P,		'p'     , 'P'     , 'P'-64  },
    {	Qt::Key_BraceLeft,	'['     , '{'     , 0xffff  },
    {	Qt::Key_Escape,		']'     , '}'     , 0xffff  },
    {	Qt::Key_Return,		13      , 13      , 0xffff  },
    {	Qt::Key_Control,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_A,		'a'     , 'A'     , 'A'-64  },  // 30
    {	Qt::Key_S,		's'     , 'S'     , 'S'-64  },
    {	Qt::Key_D,		'd'     , 'D'     , 'D'-64  },
    {	Qt::Key_F,		'f'     , 'F'     , 'F'-64  },
    {	Qt::Key_G,		'g'     , 'G'     , 'G'-64  },
    {	Qt::Key_H,		'h'     , 'H'     , 'H'-64  },
    {	Qt::Key_J,		'j'     , 'J'     , 'J'-64  },
    {	Qt::Key_K,		'k'     , 'K'     , 'K'-64  },
    {	Qt::Key_L,		'l'     , 'L'     , 'L'-64  },
    {	Qt::Key_Semicolon,	';'     , ':'     , 0xffff  },
    {	Qt::Key_Apostrophe,	'\''    , '"'     , 0xffff  },  // 40
    {	Qt::Key_QuoteLeft,	'`'     , '~'     , 0xffff  },
    {	Qt::Key_Shift,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Backslash,	'\\'    , '|'     , 0xffff  },
    {	Qt::Key_Z,		'z'     , 'Z'     , 'Z'-64  },
    {	Qt::Key_X,		'x'     , 'X'     , 'X'-64  },
    {	Qt::Key_C,		'c'     , 'C'     , 'C'-64  },
    {	Qt::Key_V,		'v'     , 'V'     , 'V'-64  },
    {	Qt::Key_B,		'b'     , 'B'     , 'B'-64  },
    {	Qt::Key_N,		'n'     , 'N'     , 'N'-64  },
    {	Qt::Key_M,		'm'     , 'M'     , 'M'-64  },  // 50
    {	Qt::Key_Comma,		','     , '<'     , 0xffff  },
    {	Qt::Key_Period,		'.'     , '>'     , 0xffff  },
    {	Qt::Key_Slash,		'/'     , '?'     , 0xffff  },
    {	Qt::Key_Shift,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Asterisk,	'*'     , '*'     , 0xffff  },
    {	Qt::Key_Alt,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Space,		' '     , ' '     , 0xffff  },
    {	Qt::Key_CapsLock,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F1,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F2,		0xffff  , 0xffff  , 0xffff  },  // 60
    {	Qt::Key_F3,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F4,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F5,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F6,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F7,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F8,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F9,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F10,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_NumLock,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_ScrollLock,	0xffff  , 0xffff  , 0xffff  },  // 70
    {	Qt::Key_7,		'7'     , '7'     , 0xffff  },
    {	Qt::Key_8,		'8'     , '8'     , 0xffff  },
    {	Qt::Key_9,		'9'     , '9'     , 0xffff  },
    {	Qt::Key_Minus,		'-'     , '-'     , 0xffff  },
    {	Qt::Key_4,		'4'     , '4'     , 0xffff  },
    {	Qt::Key_5,		'5'     , '5'     , 0xffff  },
    {	Qt::Key_6,		'6'     , '6'     , 0xffff  },
    {	Qt::Key_Plus,		'+'     , '+'     , 0xffff  },
    {	Qt::Key_1,		'1'     , '1'     , 0xffff  },
    {	Qt::Key_2,		'2'     , '2'     , 0xffff  },  // 80
    {	Qt::Key_3,		'3'     , '3'     , 0xffff  },
    {	Qt::Key_0,		'0'     , '0'     , 0xffff  },
    {	Qt::Key_Period,		'.'     , '.'     , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F11,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_F12,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },	// 90
    {	0,			0xffff  , 0xffff  , 0xffff  }
};


const QWSServer::KeyMap *QWSServer::keyMap()
{
    return keyM;
}


#endif 
