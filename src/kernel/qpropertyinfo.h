/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpropertyinfo.h $
**
** Definition of QPropertyInfo class
**
** Created : 991212
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

#ifndef QPROPERTYINFO_H
#define QPROPERTYINFO_H

#ifndef QT_H
#include "qstrlist.h"
#include "qvaluelist.h"
#endif // QT_H

class QMetaProperty;

class Q_EXPORT QPropertyInfo
{
public:
    QPropertyInfo( QMetaProperty* = 0);
    QPropertyInfo( const QPropertyInfo& );

    const char* name() const;
    const char* type() const;
    bool isEnumType() const;
    QStrList enumNames() const;
    
    bool readable() const;
    bool writeable() const;

private:
    QMetaProperty* meta;
};

typedef QValueList<QPropertyInfo > QPropertyInfoList;


#endif
