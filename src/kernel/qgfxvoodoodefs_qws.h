/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Voodoo defines
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

#define STATUS 0x000
#define INTCTRL 0x004
#define CLIP0MIN 0x008
#define CLIP0MAX 0x00c
#define DSTBASEADDR 0x010
#define DSTFORMAT 0x014
#define SRCCOLORKEYMIN 0x018
#define SRCCOLORKEYMAX 0x01c
#define DSTCOLORKEYMIN 0x020
#define DSTCOLORKEYMAX 0x024
#define BRESERROR0 0x028
#define BRESERROR1 0x02c
#define ROP 0x030
#define SRCBASEADDR 0x034
#define COMMANDEXTRA 0x038
#define LINESTIPPLE 0x03c
#define LINESTYLE 0x040
#define PATTERN0ALIAS 0x044
#define PATTERN1ALIAS 0x048
#define CLIP1MIN 0x04c
#define CLIP1MAX 0x050
#define SRCFORMAT 0x054
#define SRCSIZE 0x058
#define SRCXY 0x05c
#define COLORBACK 0x060
#define COLORFORE 0x064
#define DSTSIZE 0x068
#define DSTXY 0x06c
#define COMMAND 0x070
#define LAUNCHAREA 0x080
#define COLORPATTERN 0x100
