/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.cpp#4 $
**
** Implementation of Qt/Embedded keyboard drivers
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qwindowsystem_qws.h"
#include "qwsutils_qws.h"
#include "qgfx_qws.h"

#include <qapplication.h>
#include <qnamespace.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include <termios.h>
#include <sys/kd.h>
#include <sys/vt.h>

#define VTSWITCHSIG SIGUSR2

static QWSServer *server;

static bool vtActive = true;
static int  vtQws = 0;
static int  kbdFD = -1;

class QWSTtyKeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSTtyKeyboardHandler();
    virtual ~QWSTtyKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    QSocketNotifier *notifier;
    struct termios origTermData;
};

class QWSUsbKeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSUsbKeyboardHandler();
    virtual ~QWSUsbKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    QSocketNotifier *notifier;
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
    QSocketNotifier *notifier;
};

class QWSVFbKeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSVFbKeyboardHandler();
    virtual ~QWSVFbKeyboardHandler();

    bool isOpen() { return kbdFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int kbdFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};


void vtSwitchHandler(int /*sig*/)
{
    if (vtActive) {
	server->enablePainting(false);
	qt_screen->save();
	if (ioctl(kbdFD, VT_RELDISP, 1) == 0) {
	    vtActive = false;
	    server->closeMouse();
	}
	else {
	    server->enablePainting(true);
	}
	usleep(200000);
    }
    else {
	if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
	    server->enablePainting(true);
	    vtActive = true;
	    qt_screen->restore();
	    server->openMouse();
	    server->refresh();
	}
    }

    signal(VTSWITCHSIG, vtSwitchHandler);
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
    {	Qt::Key_Enter,		13      , 13      , 0xffff  },
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
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }	// 90
};


const QWSServer::KeyMap *QWSServer::keyMap()
{
    return keyM;
}


/*
 * Standard keyboard
 */

QWSTtyKeyboardHandler::QWSTtyKeyboardHandler() : QWSKeyboardHandler()
{
    terminalName = "/dev/tty";
    kbdFD = -1;
    notifier = 0;

    if ((kbdFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	printf("Cannot open %s\n", terminalName.latin1() );
    }

    // save for restore.
    tcgetattr( kbdFD, &origTermData );

    struct termios termdata;
    tcgetattr( kbdFD, &termdata );

    ioctl(kbdFD, KDSKBMODE, K_RAW);

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

    if ( kbdFD >= 0 ) {
	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }

    printf("\033[?25l"); fflush(stdout); // VT100 cursor off
}

QWSTtyKeyboardHandler::~QWSTtyKeyboardHandler()
{
    if (kbdFD >= 0)
    {
	ioctl(kbdFD, KDSKBMODE, K_XLATE);
	tcsetattr(kbdFD, TCSANOW, &origTermData);
	::close(kbdFD);
	kbdFD = -1;
    }
    delete notifier;
    notifier = 0;

    printf("\033[?25h"); fflush(stdout); // VT100 cursor on
}


void QWSTtyKeyboardHandler::readKeyboardData()
{
    static int shift = 0;
    static int alt   = 0;
    static int ctrl  = 0;
    static bool extended = false;
    static int prevuni = 0;
    static int prevkey = 0;

    unsigned char buf[81];
    int n;

    n = read(kbdFD, buf, 80 );
    for ( int loop = 0; loop < n; loop++ ) {
	int ch = buf[loop];
	int keyCode = Qt::Key_unknown;
	bool release = false;


	if (ch == 224) {
	    // extended
	    extended = true;
	    continue;
	}

	if (ch & 0x80) {
	    release = true;
	    ch &= 0x7f;
	}

/*
	printf( "%d ", ch );
	if (extended)
	    printf(" (Extended) ");
	if (release)
	    printf(" (Release) ");
	printf("\r\n");
*/

	if (extended)
	{
	    switch (ch)
	    {
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
	    }
	}
	else
	{
	    if (ch < 90)
	    {
		keyCode = QWSServer::keyMap()[ch].key_code;
	    }
	}

	// Virtual console switching
	int term = 0;
	if (ctrl && alt && keyCode >= Qt::Key_F1 && keyCode <= Qt::Key_F10)
	    term = keyCode - Qt::Key_F1 + 1;
	else if (ctrl && alt && keyCode == Qt::Key_Left)
	    term = QMAX(vtQws - 1, 1);
	else if (ctrl && alt && keyCode == Qt::Key_Right)
	    term = QMIN(vtQws + 1, 10);
	if (term && !release) {
	    ctrl = 0;
	    alt = 0;
	    ioctl(kbdFD, VT_ACTIVATE, term);
	    return;
	}

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
	else if (keyCode != Qt::Key_unknown) {
	    int unicode = 0;
	    if (!extended)
	    {
		if (shift)
		    unicode =  QWSServer::keyMap()[ch].shift_unicode ?  QWSServer::keyMap()[ch].shift_unicode : 0xffff;
		else if (ctrl)
		    unicode =  QWSServer::keyMap()[ch].ctrl_unicode ?  QWSServer::keyMap()[ch].ctrl_unicode : 0xffff;
		else
		    unicode =  QWSServer::keyMap()[ch].unicode ?  QWSServer::keyMap()[ch].unicode : 0xffff;
		//printf("unicode: %c\r\n", unicode);
	    }
	    int modifiers = alt | ctrl | shift;
	    bool repeat = FALSE;
	    if (prevuni == unicode && prevkey == keyCode && !release)
		repeat = TRUE;
	    server->processKeyEvent( unicode, keyCode, modifiers, !release, repeat );
	    if (!release) {
		prevuni = unicode;
		prevkey = keyCode;
	    } else {
		prevkey = prevuni = 0;
	    }
	}
	extended = false;
    }
}

/* USB driver */

QWSUsbKeyboardHandler::QWSUsbKeyboardHandler() : QWSKeyboardHandler()
{
    terminalName = "/dev/input/event0";
    kbdFD = -1;
    notifier = 0;

    if ((kbdFD = open(terminalName, O_RDONLY, 0)) < 0)
    {
	printf("Cannot open %s\n", terminalName.latin1() );
    } else {
	printf("Opened USB %s\n",terminalName.latin1());
    }
    
    if ( kbdFD >= 0 ) {
	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    
    }
    
}

QWSUsbKeyboardHandler::~QWSUsbKeyboardHandler()
{
    if (kbdFD >= 0)
    {
	::close(kbdFD);
	kbdFD = -1;
    }
    delete notifier;
    notifier = 0;
}


void QWSUsbKeyboardHandler::readKeyboardData()
{
    static int shift = 0;
    static int alt   = 0;
    static int ctrl  = 0;
    static bool extended = false;
    static int prevuni = 0;
    static int prevkey = 0;

    unsigned char buf[81];

    qDebug("USB read data");
    
    int n = read(kbdFD, buf, 16 );
    if ( n != 16 )
	return;

    int ch = buf[10];
    int keyCode = Qt::Key_unknown;
    bool release = buf[12]==0 ? true : false;


    if (ch == 224) {
	// extended
	extended = true;
    }

    if (ch & 0x80) {
	release = true;
	ch &= 0x7f;
    }

    /*
	printf( "%d ", ch );
	if (extended)
	    printf(" (Extended) ");
	if (release)
	    printf(" (Release) ");
	printf("\r\n");
*/

    if (extended)
 {
     switch (ch)
 {
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
 }
 }
    else
 {
     if (ch < 90)
 {
     keyCode = QWSServer::keyMap()[ch].key_code;
 }
 }

    // Virtual console switching
    int term = 0;
    if (ctrl && alt && keyCode >= Qt::Key_F1 && keyCode <= Qt::Key_F10)
	term = keyCode - Qt::Key_F1 + 1;
    else if (ctrl && alt && keyCode == Qt::Key_Left)
	term = QMAX(vtQws - 1, 1);
    else if (ctrl && alt && keyCode == Qt::Key_Right)
	term = QMIN(vtQws + 1, 10);
    if (term && !release) {
	ctrl = 0;
	alt = 0;
	ioctl(kbdFD, VT_ACTIVATE, term);
	return;
    }

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
    else if (keyCode != Qt::Key_unknown) {
	int unicode = 0;
	if (!extended)
    {
	if (shift)
	    unicode =  QWSServer::keyMap()[ch].shift_unicode ?  QWSServer::keyMap()[ch].shift_unicode : 0xffff;
	else if (ctrl)
	    unicode =  QWSServer::keyMap()[ch].ctrl_unicode ?  QWSServer::keyMap()[ch].ctrl_unicode : 0xffff;
	else
	    unicode =  QWSServer::keyMap()[ch].unicode ?  QWSServer::keyMap()[ch].unicode : 0xffff;
	//printf("unicode: %c\r\n", unicode);
    }
	int modifiers = alt | ctrl | shift;
	bool repeat = FALSE;
	if (prevuni == unicode && prevkey == keyCode && !release)
	    repeat = TRUE;
	server->processKeyEvent( unicode, keyCode, modifiers, !release, repeat );
	if (!release) {
	    prevuni = unicode;
	    prevkey = keyCode;
	} else {
	    prevkey = prevuni = 0;
	}
    }
    extended = false;

}

/*
 * vr41xx buttons driver
 */

QWSVr41xxButtonsHandler::QWSVr41xxButtonsHandler() : QWSKeyboardHandler()
{
    terminalName = "/dev/buttons";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	printf("Cannot open %s\n", terminalName.latin1());
    }

    if ( buttonFD >= 0 ) {
	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSVr41xxButtonsHandler::~QWSVr41xxButtonsHandler()
{
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
    delete notifier;
    notifier = 0;
}

void QWSVr41xxButtonsHandler::readKeyboardData()
{
    char buf[2];
    int n=read(buttonFD,buf,2);
    if(n<0) {
	qDebug("Keyboard read error %s",strerror(errno));
    } else {
	int keycode;
	unsigned int x=buf[1];
	if(buf[0]==7) {
	    keycode=Qt::Key_Up;
	} else if(buf[0]==0x9) {
	    keycode=Qt::Key_Right;
	} else if(buf[0]==0x8) {
	    keycode=Qt::Key_Down;
	} else if(buf[0]==0xa) {
	    keycode=Qt::Key_Left;
	} else if(buf[0]==0x3) {
	    keycode=Qt::Key_Up;
	} else if(buf[0]==0x4) {
	    keycode=Qt::Key_Down;
	} else if(buf[0]==0x1) {
	    keycode=Qt::Key_Backspace;
	} else if(buf[0]==0x2) {
	    keycode=Qt::Key_Escape;
	} else if(x==0xffffff80) {
	    keycode=0;
	    qApp->quit();
	} else {
	    qDebug("Unrecognised key sequence %d %d",buf[0],buf[1]);
	    keycode=Qt::Key_unknown;
	}
	server->processKeyEvent( 0, keycode, 0, true, false );
	server->processKeyEvent( 0, keycode, 0, false, false );
    }
}


/*
 * Virtual framebuffer keyboard driver
 */

#ifndef QT_NO_QWS_VFB
#include "qvfbhdr_qws.h"
#endif

QWSVFbKeyboardHandler::QWSVFbKeyboardHandler()
{
    kbdFD = -1;
#ifndef QT_NO_QWS_VFB
    kbdIdx = 0;
    kbdBufferLen = sizeof( QVFbKeyData ) * 5;
    kbdBuffer = new unsigned char [kbdBufferLen];

    terminalName = QT_VFB_KEYBOARD_PIPE;

    if ((kbdFD = open( terminalName.local8Bit(), O_RDWR | O_NDELAY)) < 0) {
	qDebug( "Cannot open %s (%s)", terminalName.latin1(),
	strerror(errno));
    } else {
	// Clear pending input
	char buf[2];
	while (read(kbdFD, buf, 1) > 0) { }

	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect(notifier, SIGNAL(activated(int)),this, SLOT(readKeyboardData()));
    }
#endif
}

QWSVFbKeyboardHandler::~QWSVFbKeyboardHandler()
{
#ifndef QT_NO_QWS_VFB
    if ( kbdFD >= 0 )
	close( kbdFD );
    delete [] kbdBuffer;
#endif
}


void QWSVFbKeyboardHandler::readKeyboardData()
{
#ifndef QT_NO_QWS_VFB
    int n;
    do {
	n  = read(kbdFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx );
	if ( n > 0 )
	    kbdIdx += n;
    } while ( n > 0 );

    int idx = 0;
    while ( kbdIdx - idx >= sizeof( QVFbKeyData ) ) {
	QVFbKeyData *kd = (QVFbKeyData *)(kbdBuffer + idx);
	if ( kd->unicode == 0 ) {
	    // magic exit key
	    qWarning( "Instructed to quit by Virtual Keyboard" );
	    qApp->quit();
	}
	server->processKeyEvent( kd->unicode&0xffff, kd->unicode>>16,
				 kd->modifiers, kd->press, kd->repeat );
	idx += sizeof( QVFbKeyData );
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
    server = this;

    QWSKeyboardHandler *handler = 0;
    
    if ( spec == "Buttons" ) {
	handler = new QWSVr41xxButtonsHandler();
    } else if ( spec == "QVFbKeyboard" ) {
	handler = new QWSVFbKeyboardHandler();
    } else if ( spec == "TTY" ) {
	if(getenv("QWS_USB_KEYBOARD")) {
	    handler = new QWSUsbKeyboardHandler();
	} else {
	    handler = new QWSTtyKeyboardHandler();
	}
    } else {
	qWarning( "Keyboard type %s unsupported", spec.latin1() );
    }

    return handler;
}

#include "qkeyboard_qws.moc"

