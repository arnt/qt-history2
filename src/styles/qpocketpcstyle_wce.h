/****************************************************************************
**
** Implementation of PocketPC-like style class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the styles module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPOCKETPCSTYLE_WCE_H
#define QPOCKETPCSTYLE_WCE_H

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

    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* widget,
			     const QRect& r,
			     const QPalette& pal,
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

#endif
