/****************************************************************************
**
** Definition of the CDE-like style class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef QCDESTYLE_H
#define QCDESTYLE_H

#ifndef QT_H
#include "qmotifstyle.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_CDE) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_CDE
#else
#define Q_EXPORT_STYLE_CDE Q_EXPORT
#endif

class Q_EXPORT_STYLE_CDE QCDEStyle : public QMotifStyle
{
    Q_OBJECT
public:

    QCDEStyle( bool useHighlightCols = FALSE );
    virtual ~QCDEStyle();

    int pixelMetric( PixelMetric metric, const QWidget *widget = 0 ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default ) const;

};

#endif // QT_NO_STYLE_CDE

#endif // QCDESTYLE_H
