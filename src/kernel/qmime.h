/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmime.h#3 $
**
** Definition of mime classes
**
** Created : 981204
**
** Copyright (C) 1998-1999 Troll Tech AS.  All rights reserved.
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

#ifndef QMIME_H
#define QMIME_H

#ifndef QT_H
#include "qwindowdefs.h"
#endif // QT_H

class Q_EXPORT QMimeSource {
public:
    virtual ~QMimeSource();
    virtual const char* format( int n = 0 ) const = 0;
    virtual bool provides( const char* ) const;
    virtual QByteArray encodedData( const char* ) const = 0;
};

#endif // QMIME_H
