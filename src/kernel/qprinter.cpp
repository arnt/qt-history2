/**********************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter.cpp#1 $
**
** Implementation of printer classes
**
** Author  : Eirik Eng
** Created : 941003
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qprinter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qprinter.cpp#1 $";
#endif


/*!
\class QPrinter qprinter.h
\brief The QPrinter class is a paint device that prints graphics on a printer.

\warning This version of Qt has no printer support.
*/

/*!
Constructs a printer paint device.
*/

QPrinter::QPrinter()
    : QPaintDevice( PDT_PRINTER | PDF_EXTDEV )    // set device type
{
}
