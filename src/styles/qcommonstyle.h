/****************************************************************************
** $Id: $
**
** Definition of QCommonStyle class
**
** Created : 980616
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#ifndef QT_H
#include "qstyle.h"
#include "qpushbutton.h" // compile!
#endif // QT_H

#ifndef QT_NO_STYLE

class Q_EXPORT QCommonStyle: public QStyle
{
    Q_OBJECT
private:
    QCommonStyle(GUIStyle);
    ~QCommonStyle();

    friend class QMotifStyle;
    friend class QWindowsStyle;


public:
    virtual void drawPrimitive( PrimitiveElement pe,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags = Style_Default,
				void **data = 0 ) const;

    virtual void drawControl( ControlElement element,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      SFlags how = Style_Default,
			      void **data = 0 ) const;
    virtual void drawControlMask( ControlElement element,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  void **data = 0 ) const;

    virtual QRect subRect( SubRect r, const QWidget *widget ) const;

    virtual void drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *widget,
				     const QRect &r,
				     const QColorGroup &cg,
				     SFlags how = Style_Default,
				     SCFlags sub = SC_All,
				     SCFlags subActive = SC_None,
				     void **data = 0 ) const;
    virtual void drawComplexControlMask( ComplexControl control,
					 QPainter *p,
					 const QWidget *widget,
					 const QRect &r,
					 void **data = 0 ) const;

    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *widget,
					  SubControl sc,
					  void **data = 0 ) const;
    virtual SubControl querySubControl( ComplexControl control,
					const QWidget *widget,
					const QPoint &pos,
					void **data = 0 ) const;

    virtual int pixelMetric( PixelMetric m, const QWidget *widget = 0 ) const;

    virtual QSize sizeFromContents( ContentsType s,
				    const QWidget *widget,
				    const QSize &contentsSize,
				    void **data = 0 ) const;

    virtual int styleHint( StyleHint sh,
			   const QWidget *widget = 0,
			   void ***returnData = 0 ) const;

    virtual QPixmap stylePixmap( StylePixmap sp,
				 const QWidget *widget = 0,
				 void **data = 0 ) const;


private:
    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCommonStyle( const QCommonStyle & );
    QCommonStyle &operator=( const QCommonStyle & );
#endif
};



#endif // QT_NO_STYLE

#endif // QCOMMONSTYLE_H
