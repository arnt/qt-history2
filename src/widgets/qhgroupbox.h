/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qhgroupbox.h#1 $
**
** Definition of QHGroupBox widget class
**
** Created : 990602
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the widgets
** module and therefore may only be used if the widgets module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QHGROUPBOX_H
#define QHGROUPBOX_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS

class Q_EXPORT QHGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    QHGroupBox( QWidget *parent=0, const char *name=0 );
    QHGroupBox( const QString &title, QWidget *parent=0, const char* name=0 );
   ~QHGroupBox();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHGroupBox( const QHGroupBox & );
    QHGroupBox &operator=( const QHGroupBox & );
#endif
};

#endif // QT_NO_COMPLEXWIDGETS

#endif // QHGROUPBOX_H
