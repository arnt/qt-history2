/****************************************************************************
** $Id: //depot/qt/main/examples/themes/wood.h#1 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef WOOD_H
#define WOOD_H

#include <qmotifstyle.h>

#ifdef _CC_MSVC_
//workaround for Visual C++ overloaded function bug
#include "qapplication.h"
#endif

class NorwegianWoodStyle : public QMotifStyle
{
public:
    NorwegianWoodStyle();
    void polish( QApplication*);
    void polish( QWidget* );
    void unPolish( QWidget* );
    void unPolish( QApplication*);

    void drawButton( QPainter *p, int x, int y, int w, int h,
			     const QColorGroup &g, bool sunken = FALSE,
			     const QBrush *fill = 0 );
    void drawBevelButton( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken = FALSE,
			  const QBrush *fill = 0 );
    QRect buttonRect( int x, int y, int w, int h);
    void drawButtonMask( QPainter *p, int x, int y, int w, int h);


    void drawPushButton( QPushButton* btn, QPainter *p);
    void drawPushButtonLabel( QPushButton* btn, QPainter *p);
    
private:
    QPalette oldPalette;

};

#endif
