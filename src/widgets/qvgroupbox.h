/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qvgroupbox.h#1 $
**
** Definition of QVGroupBox widget class
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

#ifndef QVGROUPBOX_H
#define QVGROUPBOX_H

#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H

#ifndef QT_NO_COMPLEXWIDGETS

class Q_EXPORT QVGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    QVGroupBox( QWidget *parent=0, const char *name=0 );
    QVGroupBox( const QString &title, QWidget *parent=0, const char* name=0 );
   ~QVGroupBox();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QVGroupBox( const QVGroupBox & );
    QVGroupBox &operator=( const QVGroupBox & );
#endif
};

#endif // QT_NO_COMPLEXWIDGETS

#endif // QVGROUPBOX_H
