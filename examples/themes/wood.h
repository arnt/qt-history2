/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WOOD_H
#define WOOD_H


#include <qpalette.h>

#ifndef QT_NO_STYLE_WINDOWS

#include <qwindowsstyle.h>


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
			const QPalette &pal,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QPalette &pal,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    void drawControlMask( ControlElement element,
			  QPainter *p,
			  const QWidget *widget,
			  const QRect &r,
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

    void drawComplexControlMask( ComplexControl control,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QStyleOption& = QStyleOption::Default ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  const QStyleOption& = QStyleOption::Default ) const;

    QRect subRect( SubRect r, const QWidget *widget ) const;


private:
    void drawSemicircleButton(QPainter *p, const QRect &r, int dir,
			      bool sunken, const QPalette &pal ) const;
    QPalette oldPalette;
    QPixmap *sunkenDark;
    QPixmap *sunkenLight;

};

#endif

#endif
