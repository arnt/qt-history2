/****************************************************************************
** $Id: //depot/qt/main/extensions/imageio/src/qimageio.cpp#2 $
**
** Implementation of QImage IO Library API
**
** Created : 970521
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qjpegio.h"
#include "qpngio.h"

void qInitImageIO()
{
    qInitJpegIO();
    qInitPngIO();
}
