/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qiconset.h#3 $
**
** Definition of QIconSet class
**
** Created : 980318
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QICONSET_H
#define QICONSET_H

#include "qpixmap.h"


struct QIconSetPrivate;


class QIconSet
{
public:
    enum Size { Automatic, Small, Large };

    enum Mode { Normal, Disabled, Active };

    QIconSet( const QPixmap &, Size = Automatic );
    QIconSet( const QIconSet & );
    virtual ~QIconSet();

    void reset( const QPixmap &, Size );

    void setPixmap( const QPixmap &, Size, Mode = Normal );
    void setPixmap( const char *, Size, Mode = Normal );
    QPixmap pixmap( Size, Mode ) const;
    QPixmap pixmap() const;
    bool isGenerated( Size, Mode ) const;

    QIconSet &operator=( const QIconSet & );

private:
    QIconSetPrivate * d;
};


#endif
