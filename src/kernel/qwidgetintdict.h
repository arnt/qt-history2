/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidgetintdict.h#10 $
**
** Definition of QWidgetIntDict
**
** Created : 950116
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWIDINTDICT_H
#define QWIDINTDICT_H

#ifndef QT_H
#include "qwidget.h"
#include "qintdict.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QIntDict<QWidget>;
template class Q_EXPORT QIntDictIterator<QWidget>;
#endif


class Q_EXPORT QWidgetIntDict : public QIntDict<QWidget>
{
public:
    QWidgetIntDict(int size=17) : QIntDict<QWidget>(size) {}
    QWidgetIntDict( const QWidgetIntDict &dict ) : QIntDict<QWidget>(dict) {}
   ~QWidgetIntDict() { clear(); }
    QWidgetIntDict &operator=(const QWidgetIntDict &dict)
	{ return (QWidgetIntDict&)QIntDict<QWidget>::operator=(dict); }
};

class Q_EXPORT QWidgetIntDictIt : public QIntDictIterator<QWidget>
{
public:
    QWidgetIntDictIt( const QWidgetIntDict &d ) : QIntDictIterator<QWidget>(d) {}
    QWidgetIntDictIt &operator=(const QWidgetIntDictIt &i)
	{ return (QWidgetIntDictIt&)QIntDictIterator<QWidget>::operator=(i); }
};


#endif // QWIDINTDICT_H
