/****************************************************************************
**
** Definition of Aqua-like style class
**
** Created : 001129
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#ifndef QAQUASTYLE_H
#define QAQUASTYLE_H

#ifndef QT_H
#include "qvariant.h"	// for template-challenged compilers
//#include "qmotifstyle.h"
#include "qwindowsstyle.h"
#include "qpalette.h"
#include "qvaluelist.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_STYLE_AQUA

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_AQUA
#else
#define Q_EXPORT_STYLE_AQUA Q_EXPORT
#endif

class QAquaStylePrivate;

class Q_EXPORT_STYLE_AQUA QAquaStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QAquaStyle();
    virtual ~QAquaStyle();

    void polish( QWidget * w );
    void unPolish( QWidget * w );
    void polish( QPalette & pal );
    void unPolish( QPalette & pal );

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

    int pixelMetric( PixelMetric metric,
		     const QWidget *widget = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *w,
			    const QSize &contentsSize,
			    void **data ) const;

    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *w,
					  SubControl sc,
					  void **data = 0 ) const;

    virtual QRect subRect( SubRect, const QWidget *w ) const;

    virtual int styleHint(StyleHint sh, const QWidget *, void ***) const;

    virtual void drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString &text,
			   int len = -1, const QColor *penColor = 0 ) const;

protected:
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QAquaStyle( const QAquaStyle & );
    QAquaStyle& operator=( const QAquaStyle & );
#endif
    QPalette oldPalette;

    QAquaStylePrivate *d;
};

#endif // QT_NO_STYLE_AQUA

#endif // QAQUASTYLE_H
