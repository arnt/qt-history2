/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qkeyboard_qws.cpp#4 $
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

#include <qapplication.h>
#include <qsocketnotifier.h>
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




#ifndef QT_NO_QWS_KEYBOARD

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
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};

class QWSiPaqButtonsHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSiPaqButtonsHandler();
    virtual ~QWSiPaqButtonsHandler();

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




/*
 * Standard keyboard
 */

QWSTtyKeyboardHandler::QWSTtyKeyboardHandler() : QWSKeyboardHandler()
{

#if defined(QT_DEMO_SINGLE_FLOPPY)
    terminalName = "/dev/tty0";
#else
    terminalName = "/dev/tty";
#endif

    kbdFD = -1;
    notifier = 0;

    if ((kbdFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	qWarning("Cannot open %s\n", terminalName.latin1() );
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
	int keypad = 0;

	if (ch == 224) {
	    // extended
	    extended = true;
	    continue;
	}

	if (ch & 0x80) {
	    release = true;
	    ch &= 0x7f;
	}


	if (extended) {
	    switch (ch) {
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
	    if (ch < 90) {
		keyCode = QWSServer::keyMap()[ch].key_code;
	    }
	}

	/*
	  Keypad consists of extended keys 53 and 28,
	  and non-extended keys 55 and 71 through 83.
	*/
	if ( extended ? (ch == 53 || ch == 28) :
	     (ch == 55 || ( ch >= 71 && ch <= 83 )) )
	    keypad = Qt::Keypad;



#if 0 //debug
	printf( "%d ", ch );
	if (extended)
	    printf(" (Extended) ");
	if (release)
	    printf(" (Release) ");
	if (keypad)
	    printf(" (Keypad) ");
	printf("\r\n");
#endif



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
	if (keyCode != Qt::Key_unknown) {
	    int unicode = 0;
	    if (!extended) {
		if (shift)
		    unicode =  QWSServer::keyMap()[ch].shift_unicode ?  QWSServer::keyMap()[ch].shift_unicode : 0xffff;
		else if (ctrl)
		    unicode =  QWSServer::keyMap()[ch].ctrl_unicode ?  QWSServer::keyMap()[ch].ctrl_unicode : 0xffff;
		else
		    unicode =  QWSServer::keyMap()[ch].unicode ?  QWSServer::keyMap()[ch].unicode : 0xffff;
		//printf("unicode: %c\r\n", unicode);
	    } else {
		if ( ch == 53 )
		    unicode = '/';
	    }

	    int modifiers = alt | ctrl | shift | keypad;
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
    // Environment variable is guaranteed to exist if we got here
    terminalName = getenv("QWS_USB_KEYBOARD");
    kbdFD = -1;
    notifier = 0;

    if ((kbdFD = open(terminalName, O_RDONLY, 0)) < 0)
    {
	qDebug("Cannot open %s\n", terminalName.latin1() );
    } else {
	qDebug("Opened USB %s\n",terminalName.latin1());
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
 * iPAQ buttons driver
 */

QWSiPaqButtonsHandler::QWSiPaqButtonsHandler() : QWSKeyboardHandler()
{
    terminalName = "/dev/h3600_key";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0) {
	qFatal("Cannot open %s\n", terminalName.latin1());
    } else {
	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSiPaqButtonsHandler::~QWSiPaqButtonsHandler()
{
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
}

void QWSiPaqButtonsHandler::readKeyboardData()
{
    uchar buf[1];
    int n=read(buttonFD,buf,1);
    if (n<0) {
	qDebug("Keyboard read error %s",strerror(errno));
    } else {
	uint code = buf[0]&0x7f;
	bool press = !(buf[0]&0x80);
	const uint nbut = 10;
	int keycode[nbut] = {
	    0, // power
	    Qt::Key_Escape, // audio
	    Qt::Key_F1,
	    Qt::Key_F2,
	    Qt::Key_F3,
	    Qt::Key_F4,
	    Qt::Key_Up,
	    Qt::Key_Right,
	    Qt::Key_Left,
	    Qt::Key_Down
	};
	if ( code < nbut ) {
	    int k = keycode[code];
	    if ( k ) {
		server->processKeyEvent( 0, k, 0, press, false );
	    } else {
		qApp->quit();
	    }
	}
    }
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
}

QWSVr41xxButtonsHandler::~QWSVr41xxButtonsHandler()
{
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
    delete notifier;
    notifier = 0;
    delete [] kbdBuffer;
}

void QWSVr41xxButtonsHandler::readKeyboardData()
{
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
		keycode = Qt::Key_Backspace;
		break;
	    case 0x2:
		keycode = Qt::Key_Escape;
		break;
	    default:
		qDebug("Unrecognised key sequence %d", (int)code );
	}
//	if ( (*code) & 0x8000 )
	    server->processKeyEvent( 0, keycode, 0, TRUE, FALSE );
//	else
	    server->processKeyEvent( 0, keycode, 0, FALSE, FALSE );
/*
	unsigned short t = *code;
	for ( int i = 0; i < 16; i++ ) {
	    keycode = (t & 0x8000) ? Qt::Key_1 : Qt::Key_0;
	    int unicode = (t & 0x8000) ? '1' : '0';
	    server->processKeyEvent( unicode, keycode, 0, TRUE, FALSE );
	    server->processKeyEvent( unicode, keycode, 0, FALSE, FALSE );
	    t <<= 1;
	}
	keycode = Qt::Key_Space;
	server->processKeyEvent( ' ', keycode, 0, TRUE, FALSE );
	server->processKeyEvent( ' ', keycode, 0, FALSE, FALSE );
*/
	idx += 2;
    }

    int surplus = kbdIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
}


/*
 * Virtual framebuffer keyboard driver
 */

#ifndef QT_NO_QWS_VFB
#include "qvfbhdr.h"
extern int qws_display_id;
#endif

QWSVFbKeyboardHandler::QWSVFbKeyboardHandler()
{
    kbdFD = -1;
#ifndef QT_NO_QWS_VFB
    kbdIdx = 0;
    kbdBufferLen = sizeof( QVFbKeyData ) * 5;
    kbdBuffer = new unsigned char [kbdBufferLen];

    terminalName = QString(QT_VFB_KEYBOARD_PIPE).arg(qws_display_id);

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
    while ( kbdIdx - idx >= (int)sizeof( QVFbKeyData ) ) {
	QVFbKeyData *kd = (QVFbKeyData *)(kbdBuffer + idx);
	if ( kd->unicode == 0 && kd->modifiers == 0 && kd->press ) {
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
#ifdef QT_QWS_IPAQ
	handler = new QWSiPaqButtonsHandler();
#else
	handler = new QWSVr41xxButtonsHandler();
#endif
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

#endif //QT_NO_QWS_KEYBOARD


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
