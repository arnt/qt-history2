/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qiconset.h#10 $
**
** Definition of QIconSet class
**
** Created : 980318
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

#ifndef QICONSET_H
#define QICONSET_H

#include "qpixmap.h"


struct QIconSetPrivate;


class Q_EXPORT QIconSet
{
public:
    enum Size { Automatic, Small, Large };

    enum Mode { Normal, Disabled, Active };

    QIconSet( const QPixmap &, Size = Automatic );
    QIconSet( const QIconSet & );
    virtual ~QIconSet();

    void reset( const QPixmap &, Size );

    virtual void setPixmap( const QPixmap &, Size, Mode = Normal );
    virtual void setPixmap( const QString &, Size, Mode = Normal );
    QPixmap pixmap( Size, Mode ) const;
    QPixmap pixmap() const;
    bool isGenerated( Size, Mode ) const;

    void detach();

    QIconSet &operator=( const QIconSet & );

private:
    QIconSetPrivate * d;
};


#endif
