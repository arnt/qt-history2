/****************************************************************************
** $Id$
**
** Implementation of Qt/Embedded Qnx keyboard drivers
**
** Copyright (C) 1999-2002 Trolltech AS.  All rights reserved.
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
