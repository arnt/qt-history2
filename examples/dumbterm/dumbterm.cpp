#include <qapplication.h>
#include "dumbterm.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

DumbTerminal::DumbTerminal(QWidget* parent, const char* name) :
    QMultiLineEdit(parent,name), socket(this), cpid(0)
{
    char ptynam[16];
    char ttynam[16];
    int ptyfd = -1;
    for (const char* c0 = "pqrstuvwxyzabcde"; ptyfd < 0 && *c0 != 0; c0++) {
	for (const char* c1 = "0123456789abcdef"; ptyfd < 0 && *c1 != 0; c1++) {
	    sprintf(ptynam,"/dev/pty%c%c",*c0,*c1);
	    sprintf(ttynam,"/dev/tty%c%c",*c0,*c1);
	    if ((ptyfd = ::open(ptynam,O_RDWR)) >= 0) {
		if (geteuid() != 0 && !access(ttynam,R_OK|W_OK) == 0) {
		    ::close(ptyfd);
		    ptyfd = -1;
		}
	    }
	}
    }
    if ( ptyfd < 0 ) {
	qApp->exit(1);
	return;
    }

    cpid = fork();
    if ( cpid ) {
	// parent - continue as a widget
	socket.setSocket(ptyfd);
	connect(&socket,SIGNAL(readyRead()), this, SLOT(readPty()));
	connect(&socket,SIGNAL(error(int)), this, SLOT(error()));
	setFont(QFont("courier",10));
	setMaxLineLength(80);
	setMaxLines(24);
    } else {
	// child - exec shell on tty
	for (int sig = 1; sig < NSIG; sig++) signal(sig,SIG_DFL);
	int tty = open(ttynam, O_RDWR);
	dup2(tty,fileno(stdin));
	dup2(tty,fileno(stdout));
	dup2(tty,fileno(stderr));
	static struct termios ttmode;
	setsid();
	ioctl(0,TCGETS,(char *)&ttmode);
	ttmode.c_cc[VINTR] = 3;
	ttmode.c_cc[VERASE] = 8;
	ioctl(0,TCSETS,(char *)&ttmode);
	setenv("TERM","dumb",1);

	if ( getuid() == 0 )
	    execl("/bin/login", "/bin/login", 0);
	else
	    execl("/bin/bash", "/bin/bash", 0);
	exit(1);
    }
}

DumbTerminal::~DumbTerminal()
{
    int status;

    socket.close();

    if (cpid) {
	kill(cpid, SIGKILL);
	waitpid(cpid, &status, 0);
    }
}

QSize DumbTerminal::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    int h = fm.lineSpacing()*(24-1)+fm.height() + frameWidth()*2;
    int w = fm.width('x')*80+24 + frameWidth()*2; // 24 for scrollbar
    return QSize(w,h);
}

void DumbTerminal::keyPressEvent(QKeyEvent* e)
{
    int ch = e->ascii();
    if ( ch ) {
	//qDebug("put %d (%c)",ch,ch);
	socket.putch(ch);
    }
}

void DumbTerminal::error()
{
    close();
    emit closed();
}

void DumbTerminal::readPty()
{
    int ch;
    QString s;
    end();
    while ( (ch = socket.getch()) >= 0 ) {
	//qDebug("get %d (%c)",ch,ch);
	if ( ch < 32 ) {
	    if ( ch == 8 ) {
		if ( s.length() == 0 ) {
		    backspace();
		} else {
		    s.truncate(s.length()-1);
		}
	    } else if ( ch == '\n' || ch == '\t' ) {
		s += ch;
	    }
	} else {
	    s += QChar(ch);
	}
    }
    insert(s);
}

