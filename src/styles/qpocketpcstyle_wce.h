/****************************************************************************
** $Id: $
**
** Definition of PocketPC-like style class
**
** Created : 981231
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
			void **data = 0 ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags how = Style_Default,
		      void **data = 0 ) const;

    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* widget,
			     const QRect& r,
			     const QColorGroup& cg,
			     SFlags how = Style_Default,
			     SCFlags sub = SC_All,
			     SCFlags subActive = SC_None,
			     void **data = 0 ) const;

    int pixelMetric( PixelMetric metric,
		     const QWidget *widget = 0 ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  void **data = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *widget,
			    const QSize &contentsSize,
			    void **data ) const;

    QPixmap stylePixmap( StylePixmap stylepixmap,
			 const QWidget *widget = 0,
			 void **data = 0 ) const;


private:
    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPocketPCStyle( const QPocketPCStyle & );
    QPocketPCStyle& operator=( const QPocketPCStyle & );
#endif
};

#endif // QT_NO_STYLE_POCKETPC

#endif // QPOCKETPCSTYLE_H
