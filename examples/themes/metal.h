/****************************************************************************
**
** Definition of the Metal Style for the themes example.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef METAL_H
#define METAL_H


#include <qpalette.h>

#ifndef QT_NO_STYLE_WINDOWS

#include <qwindowsstyle.h>


class MetalStyle : public QWindowsStyle
{
public:
    MetalStyle();
    void polish( QApplication*);
    void unPolish( QApplication*);
    void polish( QWidget* );
    void unPolish( QWidget* );

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QPalette &pal,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QPalette &pal,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    void drawComplexControl( ComplexControl cc,
			     QPainter *p,
			     const QWidget *widget,
			     const QRect &r,
			     const QPalette &pal,
			     SFlags how = Style_Default,
			     SCFlags sub = SC_All,
			     SCFlags subActive = SC_None,
			     const QStyleOption& = QStyleOption::Default ) const;
    int pixelMetric( PixelMetric, const QWidget * ) const;


private:
    void drawMetalFrame(  QPainter *p, int x, int y, int w, int h ) const;
    void drawMetalGradient( QPainter *p, int x, int y, int w, int h,
			  bool sunken, bool horz, bool flat=FALSE ) const;
    void drawMetalButton( QPainter *p, int x, int y, int w, int h,
			  bool sunken, bool horz, bool flat=FALSE ) const;
    QPalette oldPalette;
};

#endif

#endif
