/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmime.cpp#2 $
**
** Implementation of MIME support
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qmime.h"

/*!
  \class QMimeSource qmime.h
  \brief An abstract piece of formatted data.

  \link QDragObject Drag-and-drop\endlink and \link QClipboard
  clipboard\endlink use this abstraction.
*/


/*!
  Provided to ensure subclasses destruct correctly.
*/
QMimeSource::~QMimeSource()
{
}

/*!
  \fn QByteArray QMimeSource::encodedData(const char*) const

  Returns the encoded payload of this object, in the specified
  MIME format.

  Subclasses must override this function.
*/



/*!
  Returns TRUE if the object can provide the data
  in format \a mimeType.  The default implementation
  iterates over format().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
	if ( !qstricmp(mimeType,fmt) )
	    return TRUE;
    }
    return FALSE;
}


/*!
  \fn const char * QMimeSource::format(int i) const

  Returns the \e ith format, or NULL.
*/

