#include "qmacstylecg_mac.h"

#if QT_MACOSX_VERSION >= 0x1030

#include "qaquastyle_p.h"
#include "qt_mac.h"
#include "qpainter.h"
#include "qpaintdevice.h"

const int qt_mac_hitheme_version = 0; //the HITheme version we speak

// Utility to generate correct rectangles for AppManager internals
static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=NULL,
					  bool off=TRUE, const QRect &rect=QRect())
{
    static HIRect r;
    QPoint tl(qr.topLeft());
    if(pd && pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    int offset = 0;
    if(off)
	offset = 1;
    r = CGRectMake(tl.x()+rect.x(), tl.y()+rect.y(), 
		   qr.width() - offset - rect.width(), qr.height() - offset - rect.height());
    return &r;
}
static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
					    bool off=TRUE, const QRect &rect=QRect())
{
    int x, y;
    p->map(qr.x(), qr.y(), &x, &y);
    QRect r(QPoint(x, y), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if(p && p->device()->devType() == QInternal::Widget)
	return qt_aqua_size_constrain((QWidget*)p->device());
    return qt_aqua_size_constrain(NULL);
}

//externals
QRegion qt_mac_convert_mac_region(HIShapeRef); //qregion_mac.cpp

//HITheme QMacStyle
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
    ThemeDrawState tds = kThemeStateActive;
    if(flags & Style_Down) {
	tds = kThemeStatePressed;
    } else if(qAquaActive(pal)) {
	if(!(flags & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if(flags & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch(pe) {
    case PE_CheckListController:
	break;
    case PE_CheckListExclusiveIndicator:
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = (qt_mac_get_size_for_painter(p) == QAquaSizeSmall) ? kThemeSmallRadioButton : kThemeRadioButton;
	info.value = (flags & Style_On) ? kThemeButtonOn : kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	const HIRect *mac_r = qt_glb_mac_rect(r, p);
	if(pe == PE_ExclusiveIndicatorMask) {
	    p->save();
	    HIShapeRef shape;
	    HIThemeGetButtonShape(mac_r, &info, &shape);
	    p->setClipRegion(qt_mac_convert_mac_region(shape));
	    p->fillRect(r, color1);
	    p->restore();
	} else {
	    HIThemeDrawButton(mac_r, &info, (CGContextRef)p->handle(), kHIThemeOrientationNormal, 0);
	}
	break; }
    case PE_CheckListIndicator:
    case PE_IndicatorMask:
    case PE_Indicator: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = (qt_mac_get_size_for_painter(p) == QAquaSizeSmall) ? kThemeSmallCheckBox : kThemeCheckBox;
	info.value = (flags & Style_On) ? kThemeButtonOn : kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	const HIRect *mac_r = qt_glb_mac_rect(r, p);
	if(pe == PE_ExclusiveIndicatorMask) {
	    p->save();
	    HIShapeRef shape;
	    HIThemeGetButtonShape(mac_r, &info, &shape);
	    p->setClipRegion(qt_mac_convert_mac_region(shape));
	    p->fillRect(r, color1);
	    p->restore();
	} else {
	    HIThemeDrawButton(mac_r, &info, (CGContextRef)p->handle(), kHIThemeOrientationNormal, 0);
	}
	break; }
    case PE_FocusRect:
	break;     //This is not used because of the QAquaFocusWidget thingie..
    default:
	QWindowsStyle::drawPrimitive(pe, p, r, pal, flags, opt);
	break;
    }
}

void 
QMacStyleCG::drawControl(ControlElement element, QPainter *p, const QWidget *widget, const QRect &r,
			 const QPalette &pal, SFlags how, const QStyleOption &opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if(how & Style_Down) {
	tds = kThemeStatePressed;
    } else if(qAquaActive(pal)) {
	if(!(how & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if(how & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch(element) {
    case CE_PushButton: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = kThemePushButton;
	info.value = kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	HIThemeDrawButton(qt_glb_mac_rect(r, p), &info, (CGContextRef)p->handle(), kHIThemeOrientationNormal, 0);
	break; }
    default:
	QWindowsStyle::drawControl(element, p, widget, r, pal, how, opt);
    }
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
    int ret;
    switch(metric) {
    case PM_IndicatorWidth:
    case PM_IndicatorHeight: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = kThemeStateActive;
	info.kind = (qt_aqua_size_constrain(widget) == QAquaSizeSmall) ? kThemeSmallRadioButton : kThemeRadioButton;
	info.value = kThemeButtonOn;
	info.adornment = kThemeAdornmentNone;
	HIRect outr;
	HIThemeGetButtonContentBounds(qt_glb_mac_rect(widget->rect()), &info, &outr);
	qDebug("%s: %f %f %f %f", widget->objectName(), outr.origin.x, outr.origin.y, outr.size.width, outr.size.height);
	ret = (int)((metric == PM_IndicatorHeight) ? outr.size.width : outr.size.height);
	break; }
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
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

#endif
