/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qprinter_mac.cpp
**
** Implementation of QPrinter class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qprinter.h"

#ifndef QT_NO_PRINTER

#include "qpaintdevicemetrics.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qpsprinter_p.h"
#include "qprintdialog.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include <stdlib.h>


// NOT REVISED

/*****************************************************************************
  QPrinter member functions
 *****************************************************************************/

// QPrinter states

#define PST_IDLE	0
#define PST_ACTIVE	1
#define PST_ERROR	2
#define PST_ABORTED	3


/*!
  Constructs a printer paint device.
*/

QPrinter::QPrinter()
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    orient = Portrait;
    page_size = A4;
    ncopies = 1;
    from_pg = to_pg = min_pg = max_pg = 0;
    state = PST_IDLE;
    output_file = FALSE;
    to_edge	= FALSE;
}

/*!
  Destructs the printer paint device and cleans up.
*/

QPrinter::~QPrinter()
{
}


/*!
  Advances to a new page on the printer.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QPrinter::newPage()
{
    return FALSE;
}


/*!
  Aborts the print job.
  Returns TRUE if successful, otherwise FALSE.
  \sa aborted()
*/

bool QPrinter::abort()
{
    return FALSE;
}

/*!
  Returns TRUE is the printer job was aborted, otherwise FALSE.
  \sa abort()
*/

bool QPrinter::aborted() const
{
    return FALSE;
}


/*!
  Opens a printer setup dialog and asks the user to specify what printer
  to use and miscellaneous printer settings.

  Returns TRUE if the user pressed "OK" to print, or FALSE if the
  user cancelled the operation.
*/

bool QPrinter::setup( QWidget * parent )
{
    QPrintDialog prndlg( this, parent );
    return prndlg.exec() == QDialog::Accepted;
}


/*!
  \internal
  Handles painter commands to the printer.
*/

bool QPrinter::cmd( int, QPainter *, QPDevCmdParam * )
{
    return TRUE;
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  \internal
  Hard coded return values for PostScript under X.
*/

int QPrinter::metric( int ) const
{
    return 1;
}


/*!  Returns the width of the left/right and top/bottom margins of the
printer.  This is a best-effort guess, not based on perfect knowledge.

If you have called setFullPage( TRUE ) (this is recommended for
high-quality printing), margins().width() may be treated as the
smallest sane left/right margin you can use, and margins().height() as
the smallest sane top/bottom margins you can use.

If you have called setFullPage( FALSE ) (this is the default),
margins() is automatically subtracted from the pageSize() by QPrinter.

\sa setFullPage() QPaintDeviceMetrics PageSize
*/

QSize QPrinter::margins() const
{
    return (orient == Portrait) ? QSize( 36, 22 ) : QSize( 22, 36 );
}

#endif
