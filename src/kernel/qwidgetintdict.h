/****************************************************************************
**
** Definition of QWidgetIntDict.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWIDGETINTDICT_H
#define QWIDGETINTDICT_H

#ifndef QT_H
#include "qwidget.h"
#include "qintdict.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
//Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDict<QWidget>;
//Q_TEMPLATE_EXTERN template class Q_EXPORT QIntDictIterator<QWidget>;
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


#endif
