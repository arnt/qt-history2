/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.h#10 $
**
** Definition of vertical box layout widget class
**
** Created : 990124
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QVBOX_H
#define QVBOX_H

#ifndef QT_H
#include "qhbox.h"
#endif // QT_H

class Q_EXPORT QVBox : public QHBox
{
    Q_OBJECT
public:
    QVBox( QWidget *parent=0, const char *name=0, WFlags f=0,  bool allowLines=TRUE );
};

#endif //QVBOX_H
