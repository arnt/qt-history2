#include "qwindowsystem_qws.h"
#include "qwsevent.h"
#include "qwscommand.h"
#include "qwsutils.h"

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

#ifdef __MIPSEL__
static const char *terminalName = "/dev/buttons";
#else
static const char *terminalName = "/dev/tty";
#endif

static QWSServer *server;


static int kbdFD = -1;
static struct termios origTermData;
static QSocketNotifier *kbNotifier = 0;
static bool vtActive = true;
static int  vtQws = 0;

typedef struct KeyMap {
    int  key_code;
    ushort unicode;
    ushort shift_unicode;
    ushort ctrl_unicode;
};

// ###### This is a gross hack to get some keyboard stuff working before Monday
static const KeyMap keyMap[] = {
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


void vtSwitchHandler(int /*sig*/)
{
    if (vtActive) {
	server->enablePainting(false);
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
	    server->openMouse();
	    server->refresh();
	}
    }

    signal(VTSWITCHSIG, vtSwitchHandler);
}


void QWSServer::openKeyboard()
{
    if (kbdFD > 0)
	closeKeyboard();

    if ((kbdFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	printf("Cannot open %s\n", terminalName);
    }

#ifndef __MIPSEL__
    // save for restore.
    tcgetattr( kbdFD, &origTermData );

    struct termios termdata;
    tcgetattr( kbdFD, &termdata );

    // the X way -- mess up the keyboard

 #if USE_MEDIUMRAW_KBD
    ioctl(kbdFD, KDSKBMODE, K_MEDIUMRAW);
 #else
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

    if ( kbdFD >= 0 ) {
	kbNotifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect( kbNotifier, SIGNAL(activated(int)),this, 
		 SLOT(readKeyboardData()) );
    }

    server = this;
}

void QWSServer::closeKeyboard()
{
    emergency_cleanup();
    delete kbNotifier;
}


void QWSServer::emergency_cleanup()
{
#ifndef __MIPSEL__
    if (kbdFD >= 0)
    {
	ioctl(kbdFD, KDSKBMODE, K_XLATE);
	tcsetattr(kbdFD, TCSANOW, &origTermData);
	::close(kbdFD);
	kbdFD = -1;
    }
#endif
}


void QWSServer::readKeyboardData()
{
#ifdef __MIPSEL__
    char buf[2];
    int n=read(kbdFD,buf,2);
    if(n<0) {
	qDebug("Keyboard read error %s",strerror(errno));
    } else {
	int keycode;
	unsigned int x=buf[2];
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
	int unicode=keycode << 16;
	server->processKeyEvent( unicode, 0, true, false );
	server->processKeyEvent( unicode, 0, false, false );
    }
#else
    static int shift = 0;
    static int alt   = 0;
    static int ctrl  = 0;
    static bool extended = false;
    static int previous = 0;

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

#if 0 //debug
	printf( "%d ", ch );
	if (extended)
	    printf(" (Extended) ");
	if (release)
	    printf(" (Release) ");
	printf("\r\n");
#endif

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
		keyCode = keyMap[ch].key_code;
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
		    unicode =  keyMap[ch].shift_unicode ?  keyMap[ch].shift_unicode : 0xffff;
		else if (ctrl)
		    unicode =  keyMap[ch].ctrl_unicode ?  keyMap[ch].ctrl_unicode : 0xffff;
		else
		    unicode =  keyMap[ch].unicode ?  keyMap[ch].unicode : 0xffff;
		//printf("unicode: %c\r\n", unicode);
	    }
	    unicode |= keyCode << 16;
	    int modifiers = alt | ctrl | shift;
	    bool repeat = FALSE;
	    if (previous == unicode && !release)
		repeat = TRUE;
	    server->processKeyEvent( unicode, modifiers, !release, repeat );
	    if (!release)
		previous = unicode;
	    else
		previous = 0;
	}
	extended = false;
    }
#endif
}

