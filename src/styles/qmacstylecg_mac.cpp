#include "qmacstylecg_mac.h"

#if QT_MACOSX_VERSION >= 0x1030

#include "qaquastyle_p.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qpushbutton.h"
#include "qpointer.h"
#include "qt_mac.h"

const int qt_mac_hitheme_version = 0; //the HITheme version we speak
// Utility to generate correct rectangles for AppManager internals
static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=0,
					    bool off=true, const QRect &rect=QRect())
{
    static HIRect r;
    QPoint tl(qr.topLeft());
    if (pd && pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    int offset = 0;
    if (off)
	offset = 1;
    r = CGRectMake(tl.x()+rect.x(), tl.y()+rect.y(),
		   qr.width() - offset - rect.width(), qr.height() - offset - rect.height());
    return &r;
}

static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
					    bool off=true, const QRect &rect=QRect())
{
    int x, y;
    p->map(qr.x(), qr.y(), &x, &y);
    QRect r(QPoint(x, y), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if (p && p->device()->devType() == QInternal::Widget)
	return qt_aqua_size_constrain((QWidget*)p->device());
    return qt_aqua_size_constrain(0);
}

class QMacStyleCGPrivate : public QObject 
{
public:
    static const int RefreshRate = 33;  // Gives us about 30 frames a second.
    QPointer<QPushButton> defaultButton, noPulse;
    CFAbsoluteTime defaultButtonStart;
    int timerID;
    
    QMacStyleCGPrivate() { defaultButtonStart = 0; timerID = -1; }
    inline void animateButton(QPushButton *btn) {
        defaultButton = btn;
        defaultButtonStart = CFAbsoluteTimeGetCurrent();
        if (timerID <= 0)
            timerID = startTimer(RefreshRate);
    }
    inline void stopButtonAnimation() {
        if (!defaultButton)
            return;
        QPushButton *tmp = defaultButton;
        defaultButton = 0; // Lazily let the other stuff be reset in timerEvent.
        if (tmp->isVisible())
            tmp->update();
    }
protected:
    inline void timerEvent(QTimerEvent *)
    {
        if (!defaultButton.isNull()) {
            defaultButton->update();
        } else {
            // We somehow got out of sync.
            killTimer(timerID);
            timerID = -1;
            defaultButtonStart = 0;
        }
    }
};

//externals
QRegion qt_mac_convert_mac_region(HIShapeRef); //qregion_mac.cpp

//HITheme QMacStyle
QMacStyleCG::QMacStyleCG() : QWindowsStyle()
{
    d = new QMacStyleCGPrivate;
}

QMacStyleCG::~QMacStyleCG()
{

}

void QMacStyleCG::polish(QWidget *w)
{
    QPushButton *btn = ::qt_cast<QPushButton *>(w);
    if (btn) {
        btn->installEventFilter(this);
        if (btn->isDefault() || (btn->autoDefault() && btn->hasFocus())) {
            d->animateButton(btn);
        }
    }
    QWindowsStyle::polish(w);
}

void QMacStyleCG::unPolish(QWidget *w)
{
    QWindowsStyle::unPolish(w);
}

void QMacStyleCG::polish(QApplication *app)
{
    QWindowsStyle::polish(app);
}

void QMacStyleCG::drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r,
				const QPalette &pal, SFlags flags, const QStyleOption &opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if (flags & Style_Down) {
	tds = kThemeStatePressed;
    } else if (qAquaActive(pal)) {
	if (!(flags & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if (flags & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch (pe) {
    case PE_CheckListController:
	break;
    case PE_CheckListExclusiveIndicator:
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = (qt_mac_get_size_for_painter(p) == QAquaSizeSmall) ? kThemeSmallRadioButton
	            : kThemeRadioButton;
	info.value = (flags & Style_On) ? kThemeButtonOn : kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	const HIRect *mac_r = qt_glb_mac_rect(r, p);
	if (pe == PE_ExclusiveIndicatorMask) {
	    QRegion saveRegion = p->clipRegion();
	    HIShapeRef shape;
	    HIThemeGetButtonShape(mac_r, &info, &shape);
	    p->setClipRegion(qt_mac_convert_mac_region(shape));
	    p->fillRect(r, color1);
	    p->setClipRegion(saveRegion);
	} else {
	    HIThemeDrawButton(mac_r, &info, static_cast<CGContextRef>(p->handle()),
			      kHIThemeOrientationNormal, 0);
	}
	break; }
    case PE_CheckListIndicator:
    case PE_IndicatorMask:
    case PE_Indicator: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = (qt_mac_get_size_for_painter(p) == QAquaSizeSmall) ? kThemeSmallCheckBox
		    : kThemeCheckBox;
	if (flags & Style_NoChange)
	    info.value = kThemeButtonMixed;
	else if (flags & Style_On)
	    info.value = kThemeButtonOn;
	else
	    info.value = kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	const HIRect *mac_r = qt_glb_mac_rect(r, p);
	if (pe == PE_IndicatorMask) {
	    QRegion saveRegion = p->clipRegion();
	    HIShapeRef shape;
	    HIThemeGetButtonShape(mac_r, &info, &shape);
	    p->setClipRegion(qt_mac_convert_mac_region(shape));
	    p->fillRect(r, color1);
	    p->setClipRegion(saveRegion);
	} else {
	    HIThemeDrawButton(mac_r, &info, static_cast<CGContextRef>(p->handle()),
			      kHIThemeOrientationNormal, 0);
	}
	break; }
    case PE_FocusRect:
	break;     //This is not used because of the QAquaFocusWidget thingie...
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        HIThemePopupArrowDrawInfo info;
        info.version = qt_mac_hitheme_version;
        info.state = tds;
        switch (pe) {
        case PE_ArrowUp:
            info.orientation = kThemeArrowUp;
            break;
        case PE_ArrowDown:
            info.orientation = kThemeArrowDown;
            break;
        case PE_ArrowRight:
            info.orientation = kThemeArrowRight;
            break;
        case PE_ArrowLeft:
            info.orientation = kThemeArrowLeft;
            break;
        default:     // Stupid compiler should know better.
            break;
        }
        if (r.width() < 8)
            info.size = kThemeArrow5pt;
        else
            info.size = kThemeArrow9pt;
        HIThemeDrawPopupArrow(qt_glb_mac_rect(r, p), &info, static_cast<CGContextRef>(p->handle()),
                              kHIThemeOrientationNormal);
        break; }
    default:
	QWindowsStyle::drawPrimitive(pe, p, r, pal, flags, opt);
	break;
    }
}

void QMacStyleCG::drawControl(ControlElement element, QPainter *p, const QWidget *widget,
			      const QRect &r, const QPalette &pal, SFlags how,
			      const QStyleOption &opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if (qAquaActive(pal)) {
	if (!(how & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if (how & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch(element) {
    case CE_PushButton: {
	const QPushButton *btn = static_cast<const QPushButton *>(widget);
        if (btn->isFlat() && !(how & Style_Down))
	    return;
        HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
        if (how & (Style_On | Style_Down))
            info.state = kThemeStatePressed;
        else
            info.state = tds;
        info.value = kThemeButtonOff;
        if (btn->isMenuButton() || btn->isFlat()) {
            info.kind = kThemeMediumBevelButton;
        } else {
            info.kind = kThemePushButton;
            info.adornment = kThemeAdornmentNone;
        }

        if (how & Style_ButtonDefault && btn == d->defaultButton) {
            info.adornment = kThemeAdornmentDefault;
            info.animation.time.start = d->defaultButtonStart;
            info.animation.time.current = CFAbsoluteTimeGetCurrent();
        }
	HIThemeDrawButton(qt_glb_mac_rect(r, p), &info, static_cast<CGContextRef>(p->handle()),
		          kHIThemeOrientationNormal, 0);
	break; }
    default:
	QWindowsStyle::drawControl(element, p, widget, r, pal, how, opt);
    }
}

void QMacStyleCG::drawComplexControl(ComplexControl control, QPainter* p, const QWidget* w,
				     const QRect& r, const QPalette& pal, SFlags flags, SCFlags sub,
				     SCFlags subActive, const QStyleOption &opt) const
{
    QWindowsStyle::drawComplexControl(control, p, w, r, pal, flags, sub, subActive, opt);
}

int QMacStyleCG::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret;
    switch(metric) {
    case PM_IndicatorWidth: {
	ThemeMetric tm = kThemeMetricCheckBoxWidth;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallCheckBoxWidth;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_IndicatorHeight: {
	ThemeMetric tm = kThemeMetricCheckBoxHeight;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallCheckBoxHeight;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_ExclusiveIndicatorHeight: {
	ThemeMetric tm = kThemeMetricRadioButtonHeight;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallRadioButtonHeight;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_ExclusiveIndicatorWidth: {
	ThemeMetric tm = kThemeMetricRadioButtonWidth;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallRadioButtonWidth;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_MenuButtonIndicator:
        GetThemeMetric(kThemeMetricLittleArrowsWidth, &ret);
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
}


QRect QMacStyleCG::querySubControlMetrics(ComplexControl control, const QWidget *widget,
					  SubControl sc, const QStyleOption &opt) const
{
    return QWindowsStyle::querySubControlMetrics(control, widget, sc, opt);
}

QRect QMacStyleCG::subRect(SubRect sr, const QWidget *widget) const
{
    QRect subrect;
    switch (sr) {
        case SR_PushButtonContents: {
            CGRect inRect = CGRectMake(0, 0, widget->width(), widget->height());
            CGRect outRect;
            HIThemeButtonDrawInfo info;
            info.version = qt_mac_hitheme_version;
            info.state = kThemeStateActive;
            info.kind = kThemePushButton;
            info.adornment = kThemeAdornmentNone;
            HIThemeGetButtonContentBounds(&inRect, &info, &outRect);
            subrect = QRect(QPoint((int)outRect.origin.x, (int)outRect.origin.y),
                            QSize((int)outRect.size.width, (int)outRect.size.height));
            break;
        }
        default:
            subrect = QWindowsStyle::subRect(sr, widget);
    }
    return subrect;
}

QStyle::SubControl QMacStyleCG::querySubControl(ComplexControl control, const QWidget *widget,
						const QPoint &pos, const QStyleOption &opt) const
{
    return QWindowsStyle::querySubControl(control, widget, pos, opt);
}

int QMacStyleCG::styleHint(StyleHint sh, const QWidget *widget, const QStyleOption &opt,
		       QStyleHintReturn *ret) const
{
    return QWindowsStyle::styleHint(sh, widget, opt, ret);
}

QSize QMacStyleCG::sizeFromContents(ContentsType contents, const QWidget *w,
				    const QSize &contentsSize, const QStyleOption &opt) const
{
    return QWindowsStyle::sizeFromContents(contents, w, contentsSize, opt);
}

QPixmap QMacStyleCG::stylePixmap(StylePixmap sp, const QWidget *widget,
				 const QStyleOption &opt) const
{
    return QWindowsStyle::stylePixmap(sp, widget, opt);
}

QPixmap QMacStyleCG::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
				 const QPalette &pal, const QStyleOption &opt) const
{
    return QCommonStyle::stylePixmap(pixmaptype, pixmap, pal, opt);
}

bool QMacStyleCG::eventFilter(QObject *o, QEvent *e)
{
    QPushButton *btn = ::qt_cast<QPushButton *>(o);
    if (btn) {
        switch (e->type()) {
        default:
            break;
        case QEvent::FocusIn:
            if (btn->autoDefault())
                d->animateButton(btn);
            break;
        case QEvent::Hide:
            if (btn == d->defaultButton)
                d->stopButtonAnimation();
            break;
        case QEvent::MouseButtonPress:
            // From looking at Panther it seems that pulsing stops whenever the mouse
            // is pressed, most people will probably be happy enough if we do it
            // just for buttons.           
            d->stopButtonAnimation();
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::FocusOut:
        case QEvent::Show: {
            // Find the correct button and animate it.
            QObjectList list = btn->topLevelWidget()->queryList("QPushButton");
            for (int i = 0; i < list.size(); ++i) {
                QPushButton *pBtn = static_cast<QPushButton *>(list.at(i));
                if ((e->type() == QEvent::FocusOut 
                     && (pBtn->isDefault() || (pBtn->autoDefault() && pBtn->hasFocus()))
                     && pBtn != btn)
                    || ((e->type() == QEvent::Show || e->type() == QEvent::MouseButtonRelease)
                        && pBtn->isDefault())) {
                    if (pBtn->topLevelWidget()->isActiveWindow())
                        d->animateButton(pBtn);
                    break;
                }
            }
            break; }
        }
    }
    return QWindowsStyle::eventFilter(o, e);
}

#endif
