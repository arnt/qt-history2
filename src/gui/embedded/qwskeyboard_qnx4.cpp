/****************************************************************************
**
** Implementation of Qt/Embedded Qnx keyboard drivers.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwskeyboard_qnx4.h"
 
#if defined(Q_OS_QNX4)

#include <qkeyboard_qws.h>
#include <qsocketnotifier.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/keyboard.h>

QWSQnx4KeyboardHandler::QWSQnx4KeyboardHandler() {
    gState = GuidantNone;
    shift = 0;
    alt   = 0;
    ctrl  = 0;
    extended = FALSE;
    prevuni = 0;
    prevkey = 0;

    kbdFD = open("/dev/kbd", O_RDONLY);
    if (kbdFD == -1) 
	qFatal("Cannot access keyboard device\n");
    QSocketNotifier *kbdNotifier = new QSocketNotifier(kbdFD, 
					       QSocketNotifier::Read, this );
    connect(kbdNotifier, SIGNAL(activated(int)),this, SLOT(readKbdData(int)));
    notifiers.append( kbdNotifier ); 
}

void QWSQnx4KeyboardHandler::readKbdData(int fd) {
	char inChar;
	int ret = read(kbdFD, &inChar, 1);
	switch (gState) {
		case GuidantNone:
			if ( inChar == 85 || inChar == 86 )
				gState = inChar == 85 ? GuidantPressed : GuidantReleased;
			else
				doKey(inChar);
			break;
		case GuidantDropped:
			gState = GuidantNone;
			break;
		case GuidantReleased:
			inChar |= 0x80;
		case GuidantPressed:
			gState = GuidantDropped;
			doKey(inChar);
			break;
	};
}

QWSQnx4KeyboardHandler::~QWSQnx4KeyboardHandler() {
    close(kbdFD);
}

#endif
