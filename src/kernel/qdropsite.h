/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdropsite.h#7 $
**
** Definitation of Drag and Drop support
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

#ifndef QDROPSITE_H
#define QDROPSITE_H

#ifndef QT_H
#include "qglobal.h"
#endif


class QWidget;


class Q_EXPORT QDropSite {
public:
    QDropSite( QWidget* parent );
    virtual ~QDropSite();
};


#endif  // QDROPSITE_H
