
#include "qwindowsystem_qws.h"
#include "qwsutils_qws.h"
#include "qgfx_qws.h"
 
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qnamespace.h>
#include <qtimer.h>
 
#include <stdlib.h>
#include <stdio.h>
 
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
 
#if defined(_OS_QNX6_) && !defined(QT_NO_QWS_KEYBOARD)

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
//printf("code = %hhd shift = %d ctrl = %d\n",code,shift,ctrl);
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


void QWSKeyboardHandler::processKeyEvent(int unicode, int keycode, int modifiers,
            bool isPress, bool autoRepeat)
{
    qwsServer->processKeyEvent( unicode, keycode, modifiers, isPress, autoRepeat );
}

#include "qwskeyboard_qnx.moc"

#endif
