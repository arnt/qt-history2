/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstringlist.h#8 $
**
** Definition of QStringList class
**
** Created : 990406
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

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#ifndef QT_H
#include "qvaluelist.h"
#include "qstring.h"
#endif // QT_H

class QStringList : public QValueList<QString>
{
public:
    QStringList() { }
    QStringList( const QStringList& l ) : QValueList<QString>(l) { }
    QStringList( const QString& i ) { append(i); }
#ifndef QT_NO_CAST_ASCII
    QStringList( const char* i ) { append(i); }
#endif

    void sort();
    // ... stringlist-specific convenience functions go here.
};

class QDataStream;
  
extern Q_EXPORT QDataStream &operator>>( QDataStream &, QStringList& );
extern Q_EXPORT QDataStream &operator<<( QDataStream &, const QStringList& );

#endif
