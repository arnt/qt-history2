#include "qws.h"
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

#include "qwsaccel.h"

#define VTSWITCHSIG SIGUSR2

static const char terminalName[] = "/dev/tty0";

static int kbdFD = -1;
static struct termios origTermData;
static QSocketNotifier *kbNotifier = 0;
static bool vtActive = TRUE;
static int  vtQws = 0;

static QWSServer *server = 0;

typedef struct KeyMap {
    int  key_code;
    char ascii;
    char shift_ascii;
};

// ###### This is a gross hack to get some keyboard stuff working before Monday
static KeyMap keyMap[] = {
    {	Qt::Key_unknown,	0  , 0    },
    {	Qt::Key_Escape,		27 , 27   },
    {	Qt::Key_1,		'1', '!'  },
    {	Qt::Key_2,		'2', '@'  },
    {	Qt::Key_3,		'3', '#'  },
    {	Qt::Key_4,		'4', '$'  },
    {	Qt::Key_5,		'5', '%'  },
    {	Qt::Key_6,		'6', '^'  },
    {	Qt::Key_7,		'7', '&'  },
    {	Qt::Key_8,		'8', '*'  },
    {	Qt::Key_9,		'9', '('  },	// 10
    {	Qt::Key_0,		'0', ')'  },
    {	Qt::Key_Minus,		'-', '_'  },
    {	Qt::Key_Equal,		'=', '+'  },
    {	Qt::Key_Backspace,	0  , 0    },
    {	Qt::Key_Tab,		9  , 9    },
    {	Qt::Key_Q,		'q', 'Q'  },
    {	Qt::Key_W,		'w', 'W'  },
    {	Qt::Key_E,		'e', 'E'  },
    {	Qt::Key_R,		'r', 'R'  },
    {	Qt::Key_T,		't', 'T'  },  // 20
    {	Qt::Key_Y,		'y', 'Y'  },
    {	Qt::Key_U,		'u', 'U'  },
    {	Qt::Key_I,		'i', 'I'  },
    {	Qt::Key_O,		'o', 'O'  },
    {	Qt::Key_P,		'p', 'P'  },
    {	Qt::Key_BraceLeft,	'[', '{'  },
    {	Qt::Key_Escape,		']', '}'  },
    {	Qt::Key_Enter,		13 , 13   },
    {	Qt::Key_Control,	0  , 0    },
    {	Qt::Key_A,		'a', 'A'  },  // 30
    {	Qt::Key_S,		's', 'S'  },
    {	Qt::Key_D,		'd', 'D'  },
    {	Qt::Key_F,		'f', 'F'  },
    {	Qt::Key_G,		'g', 'G'  },
    {	Qt::Key_H,		'h', 'H'  },
    {	Qt::Key_J,		'j', 'J'  },
    {	Qt::Key_K,		'k', 'K'  },
    {	Qt::Key_L,		'l', 'L'  },
    {	Qt::Key_Semicolon,	';', ':'  },
    {	Qt::Key_Apostrophe,	'\'','\"' },  // 40
    {	Qt::Key_QuoteLeft,	'`', '~'  },
    {	Qt::Key_Shift,		0  , 0    },
    {	Qt::Key_Backslash,	'\\','|'  },
    {	Qt::Key_Z,		'z', 'Z'  },
    {	Qt::Key_X,		'x', 'X'  },
    {	Qt::Key_C,		'c', 'C'  },
    {	Qt::Key_V,		'v', 'V'  },
    {	Qt::Key_B,		'b', 'B'  },
    {	Qt::Key_N,		'n', 'N'  },
    {	Qt::Key_M,		'm', 'M'  },  // 50
    {	Qt::Key_Comma,		',', '<'  },
    {	Qt::Key_Period,		'.', '>'  },
    {	Qt::Key_Slash,		'/', '?'  },
    {	Qt::Key_Shift,		0  , 0    },
    {	Qt::Key_Asterisk,	'*', '*'  },
    {	Qt::Key_Alt,		0  , 0    },
    {	Qt::Key_Space,		' ', ' '  },
    {	Qt::Key_CapsLock,	0  , 0    },
    {	Qt::Key_F1,		0  , 0    },
    {	Qt::Key_F2,		0  , 0    },  // 60
    {	Qt::Key_F3,		0  , 0    },
    {	Qt::Key_F4,		0  , 0    },
    {	Qt::Key_F5,		0  , 0    },
    {	Qt::Key_F6,		0  , 0    },
    {	Qt::Key_F7,		0  , 0    },
    {	Qt::Key_F8,		0  , 0    },
    {	Qt::Key_F9,		0  , 0    },
    {	Qt::Key_F10,		0  , 0    },
    {	Qt::Key_NumLock,	0  , 0    },
    {	Qt::Key_ScrollLock,	0  , 0    },  // 70
    {	Qt::Key_7,		'7', '7'  },
    {	Qt::Key_8,		'8', '8'  },
    {	Qt::Key_9,		'9', '9'  },
    {	Qt::Key_Minus,		'-', '-'  },
    {	Qt::Key_4,		'4', '4'  },
    {	Qt::Key_5,		'5', '5'  },
    {	Qt::Key_6,		'6', '6'  },
    {	Qt::Key_Plus,		'+', '+'  },
    {	Qt::Key_1,		'1', '1'  },
    {	Qt::Key_2,		'2', '2'  },  // 80
    {	Qt::Key_3,		'3', '3'  },
    {	Qt::Key_0,		'0', '0'  },
    {	Qt::Key_Period,		'.', '.'  },
    {	Qt::Key_unknown,	0  , 0    },
    {	Qt::Key_unknown,	0  , 0    },
    {	Qt::Key_unknown,	0  , 0    },
    {	Qt::Key_F11,		0  , 0    },
    {	Qt::Key_F12,		0  , 0    },
    {	Qt::Key_unknown,	0  , 0    },
    {	Qt::Key_unknown,	0  , 0    }	// 90
};


void vtSwitchHandler(int sig)
{
    if (vtActive) {
	server->enablePainting(FALSE);
	if (ioctl(kbdFD, VT_RELDISP, 1) == 0) {
	    printf("VT switched out\n");
	    vtActive = FALSE;
	    server->closeMouse();
	}
	else {
	    server->enablePainting(TRUE);
	}
	usleep(200000);
    }
    else {
	if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
	    printf("VT switched in\n");
	    server->enablePainting(TRUE);
	    vtActive = TRUE;
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
	printf("Cannot open %s (%s)\n", terminalName, strerror(errno));
    }

    // save for restore.
    tcgetattr( kbdFD, &origTermData );

    struct termios termdata;
    tcgetattr( kbdFD, &termdata );

    printf("termdata.c_iflag %08x\n", termdata.c_iflag);

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

    kbNotifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
    connect( kbNotifier, SIGNAL(activated(int)),this, SLOT(readKeyboardData()) );

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

    server = this;
}

void QWSServer::closeKeyboard()
{
    if (kbdFD >= 0)
    {
	delete kbNotifier;
	ioctl(kbdFD, KDSKBMODE, K_XLATE);
	tcsetattr(kbdFD, TCSANOW, &origTermData);
	close(kbdFD);
	kbdFD = -1;
    }
}

void QWSServer::readKeyboardData()
{
    static int shift = 0;
    static int alt   = 0;
    static int ctrl  = 0;
    static bool extended = FALSE;

    unsigned char buf[81];
    int n;

    n = read(kbdFD, buf, 80 );
    for ( int loop = 0; loop < n; loop++ ) {
	int ch = buf[loop];
	int keyCode = Qt::Key_unknown;
	bool release = FALSE;


	if (ch == 224) {
	    // extended
	    extended = TRUE;
	    continue;
	}

	if (ch & 0x80) {
	    release = TRUE;
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
	if (alt && keyCode >= Qt::Key_F1 && keyCode <= Qt::Key_F10)
	    term = keyCode - Qt::Key_F1 + 1;
	else if (alt && keyCode == Qt::Key_Left)
	    term = qMax(vtQws - 1, 1);
	else if (alt && keyCode == Qt::Key_Right)
	    term = qMin(vtQws + 1, 10);
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
	    int ascii = 0;
	    if (!extended)
	    {
		if (shift)
		    ascii =  keyMap[ch].shift_ascii ?  keyMap[ch].shift_ascii : 0xffff;
		else
		    ascii =  keyMap[ch].ascii ?  keyMap[ch].ascii : 0xffff;
		//printf("ascii: %c\r\n", ascii);
	    }
	    int unicode = keyCode << 16;
	    unicode |= (ascii & 0xFF);
	    int modifiers = alt | ctrl | shift;
	    sendKeyEvent( unicode, modifiers, !release, FALSE );
	}
	extended = FALSE;
    }
}
