/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qiconset.h#15 $
**
** Definition of QIconSet class
**
** Created : 980318
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

#ifndef QICONSET_H
#define QICONSET_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H

#ifndef QT_NO_ICONSET
struct QIconSetPrivate;


class Q_EXPORT QIconSet
{
public:
    enum Size { Automatic, Small, Large };

    enum Mode { Normal, Disabled, Active };

    QIconSet();
    QIconSet( const QPixmap &, Size = Automatic );
    QIconSet( const QPixmap &smallPix, const QPixmap &largePix );
    QIconSet( const QIconSet & );
    virtual ~QIconSet();

    void reset( const QPixmap &, Size );

    virtual void setPixmap( const QPixmap &, Size, Mode = Normal );
    virtual void setPixmap( const QString &, Size, Mode = Normal );
    QPixmap pixmap( Size, Mode ) const;
    QPixmap pixmap( Size s, bool enabled ) const;
    QPixmap pixmap() const;
    bool isGenerated( Size, Mode ) const;

    bool isNull() const;

    void detach();

    QIconSet &operator=( const QIconSet & );

private:
    QIconSetPrivate * d;
};

#endif // QT_NO_ICONSET
#endif
