#ifndef LIGHTSTYLE_H
#define LIGHTSTYLE_H


#ifndef QT_H
#include <qstyle.h>
#include <qwindowsstyle.h>
#endif // QT_H


#ifdef QT_PLUGIN
#  define Q_EXPORT_STYLE_LIGHT
#else
#  define Q_EXPORT_STYLE_LIGHT Q_EXPORT
#endif // QT_PLUGIN


class Q_EXPORT_STYLE_LIGHT LightStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    LightStyle();
    virtual ~LightStyle();

    void unPolish(QWidget *widget);

    void polish(QApplication *app);
    void unPolish(QApplication *app);

    void polishPopupMenu(QPopupMenu *menu);

    void drawPrimitive( PrimitiveElement pe,
			QPainter *p,
			const QRect &r,
			const QColorGroup &cg,
			SFlags flags = Style_Default,
			const QStyleOption &data = QStyleOption::Default ) const;

    void drawControl( ControlElement control,
		      QPainter *p,
		      const QWidget *widget,
		      const QRect &r,
		      const QColorGroup &cg,
		      SFlags flags = Style_Default,
		      const QStyleOption &data = QStyleOption::Default ) const;
    void drawControlMask( ControlElement control,
			  QPainter *p,
			  const QWidget *widget,
			  const QRect &r,
			  const QStyleOption &data = QStyleOption::Default ) const;

    void drawComplexControl( ComplexControl control,
			     QPainter* p,
			     const QWidget* widget,
			     const QRect& r,
			     const QColorGroup& cg,
			     SFlags flags = Style_Default,
			     SCFlags controls = SC_All,
			     SCFlags active = SC_None,
			     const QStyleOption &data = QStyleOption::Default ) const;

    QRect querySubControlMetrics( ComplexControl control,
				  const QWidget *widget,
				  SubControl sc,
				  const QStyleOption &data = QStyleOption::Default ) const;

    SubControl querySubControl( ComplexControl control,
				const QWidget *widget,
				const QPoint &pos,
				const QStyleOption &data = QStyleOption::Default ) const;

    int pixelMetric( PixelMetric metric,
		     const QWidget *widget = 0 ) const;

    QSize sizeFromContents( ContentsType contents,
			    const QWidget *widget,
			    const QSize &contentsSize,
			    const QStyleOption &data = QStyleOption::Default ) const;
};


#endif // LIGHTSTYLE_H
