#ifndef QWINDOWSXPSTYLE_H
#define QWINDOWSXPSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_WINDOWSXP

#if defined(QT_PLUGIN_STYLE_WINDOWSXP)
#define Q_EXPORT_STYLE_WINDOWSXP
#else
#define Q_EXPORT_STYLE_WINDOWSXP Q_EXPORT
#endif

class QWindowsXPStylePrivate;

class Q_EXPORT_STYLE_WINDOWSXP QWindowsXPStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QWindowsXPStyle();
    ~QWindowsXPStyle();

    void polish( QWidget* );
    void unPolish( QWidget* );

    void drawPrimitive( PrimitiveElement op,
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

    QRect subRect( SubRect r, const QWidget *widget ) const;

    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* w,
			     const QRect& r,
			     const QColorGroup& cg,
			     SFlags flags = Style_Default,
			     SCFlags sub = SC_All,
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
    QWindowsXPStylePrivate *d;

#if defined(Q_DISABLE_COPY)
    QWindowsXPStyle( const QWindowsXPStyle & );
    QWindowsXPStyle& operator=( const QWindowsXPStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWSXP

#endif // QWINDOWSXPSTYLE_H
