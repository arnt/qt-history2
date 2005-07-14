/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGFXVOODOODEFS_QWS_H
#define QGFXVOODOODEFS_QWS_H

#include "QtCore/qglobal.h"

QT_MODULE(Gui)

#define VOODOOSTATUS (0x000)
#define INTCTRL (0x004+0x0100000)
#define CLIP0MIN (0x008+0x0100000)
#define CLIP0MAX (0x00c+0x0100000)
#define DSTBASEADDR (0x010+0x0100000)
#define DSTFORMAT (0x014+0x0100000)
#define SRCCOLORKEYMIN (0x018+0x0100000)
#define SRCCOLORKEYMAX (0x01c+0x0100000)
#define DSTCOLORKEYMIN (0x020+0x0100000)
#define DSTCOLORKEYMAX (0x024+0x0100000)
#define BRESERROR0 (0x028+0x0100000)
#define BRESERROR1 (0x02c+0x0100000)
#define ROP (0x030+0x0100000)
#define SRCBASEADDR (0x034+0x0100000)
#define COMMANDEXTRA (0x038+0x0100000)
#define LINESTIPPLE (0x03c+0x0100000)
#define LINESTYLE (0x040+0x0100000)
#define PATTERN0ALIAS (0x044+0x0100000)
#define PATTERN1ALIAS (0x048+0x0100000)
#define CLIP1MIN (0x04c+0x0100000)
#define CLIP1MAX (0x050+0x0100000)
#define SRCFORMAT (0x054+0x0100000)
#define SRCSIZE (0x058+0x0100000)
#define SRCXY (0x05c+0x0100000)
#define COLORBACK (0x060+0x0100000)
#define COLORFORE (0x064+0x0100000)
#define DSTSIZE (0x068+0x0100000)
#define DSTXY (0x06c+0x0100000)
#define COMMAND (0x070+0x0100000)
#define LAUNCHAREA (0x080+0x0100000)
#define COLORPATTERN (0x100+0x0100000)

#define VIDPROCCFG 0x5c
#define HWCURC0 0x68
#define HWCURC1 0x6c
#define HWCURPATADDR 0x60
#define HWCURLOC 0x64

#endif // QGFXVOODOODEFS_QWS_H
