/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Matrox defines
**
** Created : 940721
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

#define CXLEFT 0x1ca0
#define CXRIGHT 0x1ca4
#define YTOP 0x1c98
#define YBOT 0x1c9c
#define PITCH 0x1c8c
#define YDSTORG 0x1c94
#define MACCESS 0x1c04
#define CXLEFT 0x1ca0
#define CXRIGHT 0x1ca4
#define PLNWT 0x1c1c
#define FXLEFT 0x1ca8
#define FXRIGHT 0x1cac
#define YDST 0x1c90
#define LEN 0x1c5c
#define DWGCTL 0x1c00
#define FCOL 0x1c24
#define STATUS 0x1e14
#define BCOL 0x1c20
#define FXBNDRY 0x1c84
#define SGN 0x1c58

#define AR0 0x1c60
#define AR1 0x1c64
#define AR2 0x1c68
#define AR3 0x1c6c
#define AR4 0x1c70
#define AR5 0x1c74

#define EXEC 0x0100
#define DWG_REPLACE 0x000c0000

#define DWG_TRAP 0x04
#define DWG_SOLID 0x0800
#define DWG_ARZERO 0x1000
#define DWG_SGNZERO 0x2000
#define DWG_SHIFTZERO 0x4000
#define DWG_TRANSC 0x40000000
#define DWG_BITBLT 0x08
#define DWG_BFCOL 0x04000000

#define DWG_MODE (DWG_TRAP | DWG_SOLID | DWG_ARZERO | DWG_SGNZERO | DWG_SHIFTZERO | DWG_TRANSC | DWG_REPLACE)

