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

    void polish(QApplication *app);
    void unPolish(QApplication *app);

    void drawPrimitive(PrimitiveElement, QPainter *, const QRect &, const QColorGroup &,
		       SFlags = Style_Default,
		       const QStyleOption & = QStyleOption::Default ) const;

    void drawControl(ControlElement, QPainter *, const QWidget *, const QRect &,
		     const QColorGroup &, SFlags = Style_Default,
		     const QStyleOption & = QStyleOption::Default ) const;
    void drawControlMask(ControlElement, QPainter *, const QWidget *, const QRect &,
			 const QStyleOption & = QStyleOption::Default) const;

    QRect subRect(SubRect, const QWidget *) const;

    void drawComplexControl(ComplexControl, QPainter *, const QWidget *, const QRect &,
			    const QColorGroup &, SFlags = Style_Default,
			    SCFlags = SC_All, SCFlags = SC_None,
			    const QStyleOption & = QStyleOption::Default ) const;

    QRect querySubControlMetrics(ComplexControl, const QWidget *, SubControl,
				 const QStyleOption & = QStyleOption::Default ) const;

    SubControl querySubControl(ComplexControl, const QWidget *, const QPoint &,
			       const QStyleOption &data = QStyleOption::Default ) const;

    int pixelMetric(PixelMetric, const QWidget * = 0 ) const;

    QSize sizeFromContents(ContentsType, const QWidget *, const QSize &,
			   const QStyleOption & = QStyleOption::Default ) const;

    int styleHint(StyleHint, const QWidget * = 0,
		  const QStyleOption & = QStyleOption::Default,
		  QStyleHintReturn * = 0 ) const;
};


#endif // LIGHTSTYLE_H
