/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstringlist.cpp#3 $
**
** Implementation of QStringList
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qstringlist.h"
#include "qdatastream.h"
#include "qtl.h"

QDataStream &operator>>( QDataStream& s, QStringList& l )
{
  l.clear();
  
  Q_UINT32 c;
  s >> c;
  for( uint i = 0; i < c; ++i )
  {
    QString tmp;
    s >> tmp;
    l.append( tmp );
  }

  return s;
}

QDataStream &operator<<( QDataStream& s, const QStringList& l )
{
  s << (Q_UINT32)l.count();

  QStringList::ConstIterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    s << *it;
  }

  return s;
}

void QStringList::sort()
{
    qHeapSort(*this);
}
