/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture_win.cpp#18 $
**
** Implementation of QPicture class for Win32
**
** Created : 940802
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qpicture.h"
#include "qt_windows.h"


QPicture::QPicture()
    : QPaintDevice( QInternal::Picture | QInternal::ExternalDevice )	  // set device type
{
}

QPicture::~QPicture()
{
}
