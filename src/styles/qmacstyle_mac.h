/****************************************************************************
**
** Definition of ...
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

#ifndef QMACSTYLE_MAC_H
#define QMACSTYLE_MAC_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#if defined( Q_WS_MAC ) && !defined( QT_NO_STYLE_MAC )

class QPalette;

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_MAC
#else
#define Q_EXPORT_STYLE_MAC Q_EXPORT
#endif

class QMacStylePrivate;

class Q_EXPORT_STYLE_MAC QMacStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QMacStyle( );
    virtual ~QMacStyle();

    void polish( QWidget * w );
    void unPolish( QWidget * w );
    void polish( QApplication* );

    void drawItem( QPainter *p, const QRect &r,
		   int flags, const QColorGroup &g, bool enabled,
		   const QString &text, int len = -1,
		   const QColor *penColor = 0 ) const;

    void drawItem( QPainter *p, const QRect &r,
		   int flags, const QColorGroup &g, bool enabled,
		   const QPixmap &pixmap,
		   const QColor *penColor = 0 ) const;

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


    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *w,
					  SubControl sc,
					  const QStyleOption& = QStyleOption::Default ) const;

    virtual QRect subRect( SubRect, const QWidget *w ) const;

    SubControl querySubControl( ComplexControl control,
				const QWidget *widget,
				const QPoint &pos,
				const QStyleOption& = QStyleOption::Default ) const;

    virtual int styleHint(StyleHint sh, const QWidget *, const QStyleOption &,
			  QStyleHintReturn *) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *w,
			    const QSize &contentsSize,
			    const QStyleOption& = QStyleOption::Default ) const;

    enum FocusRectPolicy { FocusEnabled, FocusDisabled, FocusDefault };
    static void setFocusRectPolicy( QWidget *w, FocusRectPolicy policy);
    static FocusRectPolicy focusRectPolicy( QWidget *w );

    enum WidgetSizePolicy { SizeSmall, SizeLarge, SizeDefault };
    static void setWidgetSizePolicy( QWidget *w, WidgetSizePolicy policy);
    static WidgetSizePolicy widgetSizePolicy( QWidget *w );

protected:
    bool event(QEvent *);

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMacStyle( const QMacStyle & );
    QMacStyle& operator=( const QMacStyle & );
#endif

protected:
    QMacStylePrivate *d;
};

#endif // Q_WS_MAC

#endif // QMACSTYLE_H
