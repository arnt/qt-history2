/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsizepolicy.h#17 $
**
** Definition of the QSizePolicy class
**
** Created : 980929
**
** Copyright (C) 1998-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class Q_EXPORT QSizePolicy
{
private:
    enum { HSize = 6, HMask = 0x3f, VMask = HMask << HSize,
	   MayGrow = 1, ExpMask = 2, MayShrink = 4 };
public:
    enum SizeType { Fixed = 0,
		    Minimum = MayGrow,
		    Maximum = MayShrink,
		    Preferred = MayGrow|MayShrink ,
		    MinimumExpanding = Minimum|ExpMask,
		    Expanding = MinimumExpanding|MayShrink };

    enum ExpandData { NoDirection = 0,
		      Horizontal = 1,
		      Vertical = 2,
		      BothDirections = Horizontal | Vertical };

    QSizePolicy() { data = 0; }

    QSizePolicy( SizeType hor, SizeType ver, bool hfw = FALSE );

    SizeType horData() const { return (SizeType)( data & HMask ); }
    SizeType verData() const { return (SizeType)(( data & VMask ) >> HSize); }

    bool mayShrinkHorizontally() const { return horData() & MayShrink; }
    bool mayShrinkVertically() const { return verData() & MayShrink; }
    bool mayGrowHorizontally() const { return horData() & MayGrow; }
    bool mayGrowVertically() const { return verData() & MayGrow; }

    ExpandData expanding() const
    {
	return (ExpandData)( (int)(verData()&ExpMask ? Vertical : 0)+
			     (int)(horData()&ExpMask ? Horizontal : 0) );
    }

    void setHorData( SizeType d ) { data = (data & ~HMask) | d; }
    void setVerData( SizeType d ) { data = (data & ~(HMask<<HSize)) |
					   (d<<HSize); }
		
    void setHeightForWidth( bool b ) { data = b ? data & ~( 1 << 2*HSize )
					      :  data | ( 1 << 2*HSize ); }
    bool hasHeightForWidth() const { return data & ( 1 << 2*HSize ); }

    bool operator==( const QSizePolicy& s) const { return data == s.data; }
    bool operator!=( const QSizePolicy& s) const { return data != s.data; }
    
private:
    QSizePolicy( int i ): data( i ) {}

    Q_UINT16 data;
};

inline QSizePolicy::QSizePolicy( SizeType hor, SizeType ver, bool hfw )
	: data( hor | (ver<<HSize) | (hfw ? (1<<2*HSize) : 0) ) {}


#endif
