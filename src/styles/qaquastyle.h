/****************************************************************************
** $Id: $
**
** Definition of Aqua-like style class
**
** Created : 001129
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software. This file and its contents may
** not be distributed onto any other platform or included in any other licensed
** package unless explicit permission is granted.
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
    void polish( QApplication* );

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
			     const QWidget* w,
			     const QRect& r,
			     const QColorGroup& cg,
			     SFlags flags = Style_Default,
			     SCFlags sub = SC_None,
			     SCFlags subActive = SC_None,
			     const QStyleOption& = QStyleOption::Default ) const;

    int pixelMetric( PixelMetric metric,
		     const QWidget *widget = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *w,
			    const QSize &contentsSize,
			    const QStyleOption& = QStyleOption::Default ) const;

    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *w,
					  SubControl sc,
					  const QStyleOption& = QStyleOption::Default ) const;

    virtual QRect subRect( SubRect, const QWidget *w ) const;

    virtual int styleHint(StyleHint sh, const QWidget *, const QStyleOption &, QStyleHintReturn *) const;

    virtual void drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString &text,
			   int len = -1, const QColor *penColor = 0 ) const;

#ifdef Q_WS_MAC
    static void appearanceChanged();
#endif

protected:
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QAquaStyle( const QAquaStyle & );
    QAquaStyle& operator=( const QAquaStyle & );
#endif
    QAquaStylePrivate *d;
};

#endif // QT_NO_STYLE_AQUA

#endif // QAQUASTYLE_H
