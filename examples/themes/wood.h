/****************************************************************************
** $Id$
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef WOOD_H
#define WOOD_H

#include <qwindowsstyle.h>
#include <qpalette.h>

class NorwegianWoodStyle : public QWindowsStyle
{
public:
    NorwegianWoodStyle();
    void polish( QApplication*);
    void polish( QWidget* );
    void unPolish( QWidget* );
    void unPolish( QApplication*);

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			SFlags flags = Style_Default,
			void **data = 0 ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags how = Style_Default,
		      void **data = 0 ) const;

    void drawControlMask( ControlElement element,
			  QPainter *p,
			  const QWidget *widget,
			  const QRect &r,
			  void **data = 0 ) const;

    void drawComplexControl( ComplexControl cc,
			     QPainter *p,
			     const QWidget *widget,
			     const QRect &r,
			     const QColorGroup &cg,
			     SFlags how = Style_Default,
			     SCFlags sub = SC_All,
			     SCFlags subActive = SC_None,
			     void **data = 0 ) const;

    void drawComplexControlMask( ComplexControl control,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 void **data = 0 ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  void **data = 0 ) const;

    QRect subRect( SubRect r, const QWidget *widget ) const;


private:
    void drawSemicircleButton(QPainter *p, const QRect &r, int dir,
			      bool sunken, const QColorGroup &g ) const;
    QPalette oldPalette;
    QPixmap *sunkenDark;
    QPixmap *sunkenLight;

};

#endif
