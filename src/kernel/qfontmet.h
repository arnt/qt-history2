/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmet.h#2 $
**
** Definition of QFontMetrics class
**
** Author  : Haavard Nord
** Created : 940514
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTMET_H
#define QFONTMET_H

#include "qfont.h"


class QFontMetrics
{
public:
    QFontMetrics( const QFont & );

    int	 ascent()	const;
    int	 descent()	const;
    int	 height()	const;

    int	 width( const char *, int len = -1 ) const;

private:
#if defined(_WS_WIN_)
#elif defined(_WS_PM_)
#elif defined(_WS_X11_)
    QXFontStruct *f;
#endif
};


#endif // QFONTMET_H
