#include "qmacstylecg_mac.h"

QMacStyleCG::QMacStyleCG()
{

}

QMacStyleCG::~QMacStyleCG()
{

}

void 
QMacStyleCG::polish(QWidget *w)
{
    QWindowsStyle::polish(w);
}

void 
QMacStyleCG::unPolish(QWidget *w)
{
    QWindowsStyle::unPolish(w);
}

void 
QMacStyleCG::polish(QApplication *app)
{
    QWindowsStyle::polish(app);
}

void 
QMacStyleCG::drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r, const QPalette &pal,
			   SFlags flags, const QStyleOption &opt) const
{
    QWindowsStyle::drawPrimitive(pe, p, r, pal, flags, opt);
}

void 
QMacStyleCG::drawControl(ControlElement element, QPainter *p, const QWidget *widget, const QRect &r,
			 const QPalette &pal, SFlags how, const QStyleOption &opt) const
{
    QWindowsStyle::drawControl(element, p, widget, r, pal, how, opt);
}

void 
QMacStyleCG::drawComplexControl(ComplexControl control, QPainter* p, const QWidget* w, const QRect& r,
				const QPalette& pal, SFlags flags, SCFlags sub, SCFlags subActive,
				const QStyleOption &opt) const
{
    QWindowsStyle::drawComplexControl(control, p, w, r, pal, flags, sub, subActive, opt);
}

int 
QMacStyleCG::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    return QWindowsStyle::pixelMetric(metric, widget);
}


QRect 
QMacStyleCG::querySubControlMetrics(ComplexControl control, const QWidget *widget, SubControl sc,
				    const QStyleOption &opt) const
{
    return QWindowsStyle::querySubControlMetrics(control, widget, sc, opt);
}

QRect 
QMacStyleCG::subRect(SubRect sr, const QWidget *widget) const
{
    return QWindowsStyle::subRect(sr, widget);
}

QStyle::SubControl 
QMacStyleCG::querySubControl(ComplexControl control, const QWidget *widget, const QPoint &pos,
			     const QStyleOption &opt) const
{
    return QWindowsStyle::querySubControl(control, widget, pos, opt);
}

int
QMacStyleCG::styleHint(StyleHint sh, const QWidget *widget, const QStyleOption &opt,
		       QStyleHintReturn *ret) const
{
    return QWindowsStyle::styleHint(sh, widget, opt, ret);
}

QSize 
QMacStyleCG::sizeFromContents(ContentsType contents, const QWidget *w, const QSize &contentsSize,
			      const QStyleOption &opt) const
{
    return QWindowsStyle::sizeFromContents(contents, w, contentsSize, opt);
}

QPixmap 
QMacStyleCG::stylePixmap(StylePixmap sp, const QWidget *widget, const QStyleOption &opt) const
{
    return QWindowsStyle::stylePixmap(sp, widget, opt);
}

QPixmap 
QMacStyleCG::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
			 const QPalette &pal, const QStyleOption &opt) const
{
    return QCommonStyle::stylePixmap(pixmaptype, pixmap, pal, opt);
}

