/****************************************************************************
** $Id: $
**
** Definition of Windows-like style class
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

#ifndef QWINDOWSSTYLE_H
#define QWINDOWSSTYLE_H

#ifndef QT_H
#include "qcommonstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_WINDOWS

#if defined(QT_PLUGIN_STYLE_WINDOWS)
#define Q_EXPORT_STYLE_WINDOWS
#else
#define Q_EXPORT_STYLE_WINDOWS Q_EXPORT
#endif

class QWindowsStylePrivate;

class Q_EXPORT_STYLE_WINDOWS QWindowsStyle : public QCommonStyle
{
    Q_OBJECT
public:
    QWindowsStyle();
    virtual ~QWindowsStyle();

    virtual void polish( QWidget * );
    virtual void unPolish( QWidget * );

    virtual void polishPopupMenu( QPopupMenu* );

    // new stuff
    void drawPrimitive( PrimitiveOperation op,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			PFlags flags = PStyle_Default,
			void **data = 0 ) const;

    void drawControl( ControlElement element,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      CFlags how = CStyle_Default,
		      void **data = 0 ) const;

    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* w,
			     const QRect& r,
			     const QColorGroup& cg,
			     CFlags flags = CStyle_Default,
			     SCFlags sub = SC_None,
			     SCFlags subActive = SC_None,
			     void **data = 0 ) const;

    void drawSubControl( SCFlags subCtrl,
			 QPainter* p,
			 const QWidget* w,
			 const QRect& r,
			 const QColorGroup& cg,
			 CFlags flags = CStyle_Default,
			 SCFlags subActive = SC_None,
			 void **data = 0 ) const;

    int pixelMetric( PixelMetric metic,
		     const QWidget *widget = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *w,
			    const QSize &contentsSize,
			    void **data ) const;

    QPixmap stylePixmap( StylePixmap stylepixmap,
			 const QWidget * = 0,
			 void **data = 0 ) const;


protected:
    bool eventFilter( QObject *o, QEvent *e );


private:
    QWindowsStylePrivate *d;

    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWindowsStyle( const QWindowsStyle & );
    QWindowsStyle& operator=( const QWindowsStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QWINDOWSSTYLE_H
