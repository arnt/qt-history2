/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#include "glinfo.h"

#include "qstring.h"


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>


GLInfo::GLInfo()
{
    infotext = new QString("GLTest:\n");
    viewlist = new QStringList();
 
};

QString GLInfo::getText()
{
  return *infotext;
}

QStringList GLInfo::getViewList()
{
  return *viewlist;
}
