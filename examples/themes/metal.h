/****************************************************************************
** $Id: //depot/qt/main/examples/themes/metal.h#1 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef METAL_H
#define METAL_H

#include <qwindowsstyle.h>
#include <qpalette.h>

#ifdef _CC_MSVC_
//workaround for Visual C++ overloaded function bug
#include "qapplication.h"
#endif

class MetalStyle : public QWindowsStyle
{
public:
    MetalStyle();
    void polish( QApplication*);
    void unPolish( QApplication*);
    void polish( QWidget* );
    void unPolish( QWidget* );

    void drawButton( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, bool sunken = FALSE,
			     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );

    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);
    void drawPanel( QPainter *p, int x, int y, int w, int h,
		    const QColorGroup &, bool sunken,
		    int lineWidth, const QBrush *fill );

private:
    QPalette oldPalette;
};

#endif
