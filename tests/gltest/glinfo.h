/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef GLINFO_H
#define GLINFO_H

#include "qstring.h"
#include <X11/Xlib.h>

class GLInfo
{
public:
    GLInfo();
    QString getText();

protected:
    QString *infotext;
    void print_screen_info(Display *dpy, int scrnum);
};

#endif
