/****************************************************************************
** $Id$
**
** Implementation of PocketPC-like style class
**
** Created : 2001/08/07 
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the styles module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QPOCKETPCSTYLE_H
#define QPOCKETPCSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_POCKETPC

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_POCKETPC
#else
#define Q_EXPORT_STYLE_POCKETPC Q_EXPORT
#endif


#ifndef Q_QDOC
class Q_EXPORT_STYLE_POCKETPC QPocketPCStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QPocketPCStyle();
    virtual ~QPocketPCStyle();

    virtual void polishPopupMenu( QPopupMenu* );
    virtual void polish( QApplication* );
    virtual void unPolish( QApplication* );

    // new stuff
    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			SFlags flags = Style_Default,
			const QStyleOption& = QStyleOption::Default ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags how = Style_Default,
		      const QStyleOption& = QStyleOption::Default ) const;

    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* widget,
			     const QRect& r,
			     const QColorGroup& cg,
			     SFlags how = Style_Default,
			     SCFlags sub = SC_All,
			     SCFlags subActive = SC_None,
			     const QStyleOption& = QStyleOption::Default ) const;

    int pixelMetric( PixelMetric metric,
		     const QWidget *widget = 0 ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  const QStyleOption& = QStyleOption::Default ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *widget,
			    const QSize &contentsSize,
			    const QStyleOption& = QStyleOption::Default ) const;

    QPixmap stylePixmap( StylePixmap stylepixmap,
			 const QWidget *widget = 0,
			 const QStyleOption& = QStyleOption::Default ) const;


private:
    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPocketPCStyle( const QPocketPCStyle & );
    QPocketPCStyle& operator=( const QPocketPCStyle & );
#endif
};
#endif // Q_QDOC

#endif // QT_NO_STYLE_POCKETPC

#endif // QPOCKETPCSTYLE_H
