/****************************************************************************
**
** Implementation of Mac native theme.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmacstyle_mac.h"
#include "qmacstyleqd_mac.h"

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)

#include <private/qdialogbuttons_p.h>
#include <private/qpainter_p.h>
#include <private/qt_mac_p.h>
#include <private/qtitlebar_p.h>
#include <qapplication.h>
#include <qaquastyle_mac.h>
#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qdrawutil.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qmenu.h>
#include <qpaintengine_mac.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qpointer.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qrubberband.h>
#include <qscrollbar.h>
#include <qscrollview.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstyleoption.h>
#include <qtabbar.h>
#include <qtoolbutton.h>

extern QRegion qt_mac_convert_mac_region(RgnHandle rgn);
extern QRegion qt_mac_convert_mac_region(HIShapeRef shape);
class QListViewItem;

class QMacStyleQDPainter : public QPainter
{
public:
    QMacStyleQDPainter(QPaintDevice *p) : QPainter(p) { }
    QPoint domap(int x, int y) { map(x, y, &x, &y); return QPoint(x, y); }
    inline void setport();
};
void QMacStyleQDPainter::setport()
{
    QQuickDrawPaintEngine *mpe = NULL;
    if (d_ptr->engine && (d_ptr->engine->type() == QPaintEngine::QuickDraw
                         || d_ptr->engine->type() == QPaintEngine::CoreGraphics))
        mpe = static_cast<QQuickDrawPaintEngine *>(d_ptr->engine);
    if (mpe) {
        mpe->updateState(d_ptr->state);
        if(mpe->type() == QPaintEngine::QuickDraw) {
            mpe->setupQDPort(true);
        } else {
            QRegion rgn;
            mpe->setupQDPort(true, 0, &rgn);
            QMacSavedPortInfo::setClipRegion(rgn);
        }
    }
    NormalizeThemeDrawingState();
}

//externals
QPixmap qt_mac_convert_iconref(IconRef, int, int); //qpixmap_mac.cpp
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp

//static utility variables
static const QDialogButtons::Button macBtnOrder[] = { QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
                                                      QDialogButtons::All, QDialogButtons::Apply, QDialogButtons::Abort,
                                                      QDialogButtons::Retry, QDialogButtons::Ignore };
static ThemeWindowType macWinType = kThemeUtilityWindow;
static const int macSpinBoxSep        = 5;    // distance between spinwidget and the lineedit
static const int macItemFrame         = 2;    // menu item frame width
static const int macItemHMargin       = 3;    // menu item hor text margin
static const int macItemVMargin       = 2;    // menu item ver text margin
static const int macRightBorder       = 12;   // right border on mac

#define QMAC_DO_SECONDARY_GROUPBOXES

// Utility to generate correct rectangles for AppManager internals
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=NULL,
                                          bool off=true, const QRect &rect=QRect())
{
    static Rect r;
    bool use_rect = (rect.x() || rect.y() || rect.width() || rect.height());
    QPoint tl(qr.topLeft());
    if(pd && pd->devType() == QInternal::Widget) {
        QWidget *w = (QWidget*)pd;
        tl = w->mapTo(w->topLevelWidget(), tl);
    }
    if(use_rect)
        tl += rect.topLeft();
    int offset = 0;
    if(off)
        offset = 1;
    SetRect(&r, tl.x(), tl.y(), (tl.x() + qr.width()) - offset, (tl.y() + qr.height()) - offset);
    if(use_rect) {
        r.right -= rect.width();
        r.bottom -= rect.height();
    }
    return &r;
}
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
                                          bool off=true, const QRect &rect=QRect())
{
    QPoint pt = qr.topLeft();
    QRect r(((QMacStyleQDPainter*)p)->domap(pt.x(), pt.y()), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if(p && p->device()->devType() == QInternal::Widget)
        return qt_aqua_size_constrain((QWidget*)p->device());
    return qt_aqua_size_constrain(NULL);
}

static void getSliderInfo(QStyle::ComplexControl cc, const QStyleOptionSlider *slider,
                          const QPainter *p, ThemeTrackDrawInfo *tdi, const QWidget *needToRemove)
{
    memset(tdi, 0, sizeof(ThemeTrackDrawInfo));
    tdi->filler1 = 0;
    bool isScrollbar = (cc == QStyle::CC_ScrollBar);
    switch (qt_aqua_size_constrain(needToRemove)) {
    case QAquaSizeUnknown:
    case QAquaSizeLarge:
        if (isScrollbar)
            tdi->kind = kThemeMediumScrollBar;
        else
            tdi->kind = kThemeMediumSlider;
        break;
    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
            if (isScrollbar)
                tdi->kind = kThemeMiniScrollBar;
            else
                tdi->kind = kThemeMiniSlider;
            break;
        }
#endif
    case QAquaSizeSmall:
        if (isScrollbar)
            tdi->kind = kThemeSmallScrollBar;
        else
            tdi->kind = kThemeSmallSlider;
        break;
    }
    // Grr... this is dumb
    if (!p) {
        tdi->bounds = *qt_glb_mac_rect(slider->rect);
    } else {
        tdi->bounds = *qt_glb_mac_rect(slider->rect, p);
    }
    tdi->min = slider->minimum;
    tdi->max = slider->maximum;
    tdi->value = slider->sliderPosition;
    tdi->attributes = kThemeTrackShowThumb;
    if (slider->state & QStyle::Style_HasFocus)
        tdi->attributes |= kThemeTrackHasFocus;
    if (slider->orientation == Qt::Horizontal)
        tdi->attributes |= kThemeTrackHorizontal;
    if (slider->useRightToLeft)
        tdi->attributes |= kThemeTrackRightToLeft;
    tdi->enableState = slider->state & QStyle::Style_Enabled ? kThemeTrackActive
                                                             : kThemeTrackDisabled;
    if (!qAquaActive(slider->palette))
        tdi->enableState = kThemeTrackDisabled;
    if (!isScrollbar) {
        if (slider->tickmarks == QSlider::NoMarks || slider->tickmarks == QSlider::Both)
            tdi->trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickmarks == QSlider::Above)
            tdi->trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi->trackInfo.slider.thumbDir = kThemeThumbDownward;
    }
}

//private
class QMacStyleQDFocusWidget : public QAquaFocusWidget
{
public:
    QMacStyleQDFocusWidget(QWidget *w) : QAquaFocusWidget(false, w) { }

protected:
    void drawFocusRect(QMacStyleQDPainter *p) const;

    virtual QRegion focusRegion() const;
    virtual void paintEvent(QPaintEvent *);
    virtual int focusOutset() const;
};
QRegion QMacStyleQDFocusWidget::focusRegion() const
{
    const QRgb fillColor = qRgb(192, 191, 190);
    QImage img;
    {
        QPixmap pix(size(), 32);
        pix.fill(fillColor);
        QMacStyleQDPainter p(&pix);
        drawFocusRect(&p);
        img = pix;
    }
    QImage mask(img.width(), img.height(), 1, 2, QImage::LittleEndian);
    for(int y = 0; y < img.height(); y++) {
        for(int x = 0; x < img.width(); x++) {
            QRgb clr = img.pixel(x, y);
            int diff = (((qRed(clr)-qRed(fillColor))*((qRed(clr)-qRed(fillColor)))) +
                        ((qGreen(clr)-qGreen(fillColor))*((qGreen(clr)-qGreen(fillColor)))) +
                        ((qBlue(clr)-qBlue(fillColor))*((qBlue(clr)-qBlue(fillColor)))));
            mask.setPixel(x, y, diff < 100);
        }
    }
    QBitmap qmask;
    qmask = mask;
    return QRegion(qmask);
}
void QMacStyleQDFocusWidget::paintEvent(QPaintEvent *)
{
    QMacStyleQDPainter p(this);
    drawFocusRect(&p);
}
void QMacStyleQDFocusWidget::drawFocusRect(QMacStyleQDPainter *p) const
{
    p->setport();
    QRect r(focusOutset(), focusOutset(),  width() - (focusOutset()*2),
            height() - (focusOutset()*2));
    DrawThemeFocusRect(qt_glb_mac_rect(r, p, true, QRect(1, 1, 1, 1)), true);
}
int QMacStyleQDFocusWidget::focusOutset() const
{
    SInt32 ret = 0;
    GetThemeMetric(kThemeMetricFocusRectOutset, &ret);
    return ret;
}

class QMacStyleQDPrivate : public QAquaAnimate
{
    QPointer<QMacStyleQDFocusWidget> focusWidget;
public:
    struct ButtonState {
        int frame;
        enum { ButtonDark, ButtonLight } dir;
    } buttonState;
    struct ProgressBarState {
        int frame;
    } progressbarState;
    struct ListViewItemState {
        QMap<QListViewItem*, int> lvis;
    } lviState;
    QMacStyleQDPrivate();
    ~QMacStyleQDPrivate();
protected:
    bool doAnimate(QAquaAnimate::Animates);
    void doFocus(QWidget *);
};

QMacStyleQDPrivate::QMacStyleQDPrivate() : QAquaAnimate()
{
    progressbarState.frame = 0;
    buttonState.frame = 0;
    buttonState.dir = ButtonState::ButtonDark;
}
QMacStyleQDPrivate::~QMacStyleQDPrivate()
{
    buttonState.frame = 0;
    buttonState.dir = ButtonState::ButtonDark;
    progressbarState.frame = 0;
}
bool QMacStyleQDPrivate::doAnimate(QAquaAnimate::Animates as)
{
    if(as == AquaPushButton) {
        if(buttonState.frame == 25 && buttonState.dir == ButtonState::ButtonDark)
            buttonState.dir = ButtonState::ButtonLight;
        else if(!buttonState.frame && buttonState.dir == ButtonState::ButtonLight)
            buttonState.dir = ButtonState::ButtonDark;
        buttonState.frame += ((buttonState.dir == ButtonState::ButtonDark) ? 1 : -1);
    } else if(as == AquaProgressBar) {
        progressbarState.frame++;
    } else if(as == AquaListViewItemOpen) {
        // To be revived later...
    }
    return true;
}
void QMacStyleQDPrivate::doFocus(QWidget *w)
{
    if(!focusWidget)
        focusWidget = new QMacStyleQDFocusWidget(w);
    focusWidget->setFocusWidget(w);
}

static int mac_count = 0;

QMacStyleQD::QMacStyleQD() : QWindowsStyle()
{
    d = new QMacStyleQDPrivate;
    if(!mac_count++)
        RegisterAppearanceClient();
}

QMacStyleQD::~QMacStyleQD()
{
    if(!(--mac_count))
        UnregisterAppearanceClient();
    delete d;
}

void QMacStyleQD::polish(QApplication* app)
{
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QColor pc(Qt::black);
    {
        QPainter p(&px);
        ((QMacStyleQDPainter *)&p)->setport();
        SetThemeBackground(kThemeBrushDialogBackgroundActive, px.depth(), true);
        EraseRect(qt_glb_mac_rect(QRect(0, 0, px.width(), px.height()), (QPaintDevice*)0, false));
        RGBColor c;
        GetThemeBrushAsColor(kThemeBrushDialogBackgroundActive, 32, true, &c);
        pc = QColor(c.red / 256, c.green / 256, c.blue / 256);
    }
    QBrush background(pc, px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
    app->setPalette(pal);
}

void QMacStyleQD::polish(QWidget* w)
{
    QPixmap *bgPixmap = w->palette().brush(w->backgroundRole()).pixmap();
    if(!w->isTopLevel() && ::qt_cast<QSplitter*>(w) == 0
       && bgPixmap && qApp->palette().brush(QPalette::Active, QColorGroup::Background).pixmap()
       && bgPixmap->serialNumber() == qApp->palette().brush(QPalette::Active, QColorGroup::Background).pixmap()->serialNumber()) {
        // w->setBackgroundOrigin(QWidget::AncestorOrigin); // I currently do nothing.
    }
    d->addWidget(w);

#ifdef QMAC_DO_SECONDARY_GROUPBOXES
    if(w->parentWidget() && ::qt_cast<QGroupBox*>(w->parentWidget())
            && !w->testAttribute(Qt::WA_SetPalette)
            && w->parentWidget()->parentWidget()
            && ::qt_cast<QGroupBox*>(w->parentWidget()->parentWidget())) {
        QPalette pal = w->palette();
        QPixmap px(200, 200, 32);
        QColor pc(Qt::black);
        {
            QPainter p(&px);
            ((QMacStyleQDPainter *)&p)->setport();
            Rect r; SetRect(&r, 0, 0, px.width(), px.height());
            ApplyThemeBackground(kThemeBackgroundSecondaryGroupBox, &r,
                                 kThemeStateActive, px.depth(), true);
            EraseRect(&r);
        }
        QBrush background(pc, px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
    }
#endif

    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_JAGUAR) {
        if(::qt_cast<QGroupBox*>(w))
            w->setAttribute(Qt::WA_ContentsPropagated, true);
    }
    if(QLineEdit *lined = ::qt_cast<QLineEdit*>(w)) {
#if 0
        if(::qt_cast<QComboBox*>(w->parentWidget()))
            lined->setFrameStyle(QFrame::LineEditPanel | QFrame::Sunken);
        SInt32 frame_size;
        GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
        lined->setLineWidth(frame_size);
#else
        Q_UNUSED(lined);
//# warning "Do we need to replace this with something else for the new QLineEdit? --Sam"
#endif
    } else if(QDialogButtons *btns = ::qt_cast<QDialogButtons*>(w)) {
        if(btns->buttonText(QDialogButtons::Help).isNull())
            btns->setButtonText(QDialogButtons::Help, "?");
    } else if(QToolButton *btn = ::qt_cast<QToolButton*>(w)) {
        btn->setAutoRaise(false);
    }
#ifndef QT_NO_MAINWINDOW
    else if(QToolBar *bar = ::qt_cast<QToolBar*>(w)) {
        QBoxLayout * layout = bar->boxLayout();
        layout->setSpacing(0);
        layout->setMargin(0);
    }
#endif
    else if(w->inherits("QTipLabel")) {   // QTipLabel is declared in qtooltip.cpp :-(
        QLabel *label = static_cast<QLabel *>(w);
        label->setFrameStyle(QFrame::NoFrame);
        label->setLineWidth(1);
        label->setWindowOpacity(0.95);
        /*
#ifdef QT_COMPAT
    } else if(Q3PopupMenu *popup = ::qt_cast<Q3PopupMenu*>(w)) {
        popup->setMargin(0);
        popup->setLineWidth(0);
        w->setWindowOpacity(0.95);
#endif
*/
    } else if(QRubberBand *rubber = ::qt_cast<QRubberBand*>(w)) {
        rubber->setWindowOpacity(0.75);
    } else if(QMenu *menu = ::qt_cast<QMenu*>(w)) {
        menu->setWindowOpacity(0.95);
    } else if(QTitleBar *tb = ::qt_cast<QTitleBar *>(w)) {
//        w->font().setPixelSize(10);
        tb->setAutoRaise(true);
    }
    QWindowsStyle::polish(w);
}

void QMacStyleQD::unPolish(QWidget* w)
{
    d->removeWidget(w);
    QToolButton *btn = ::qt_cast<QToolButton*>(w);
    if(btn) {
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise(true);
        /*
#ifdef QT_COMPAT
    } else if(::qt_cast<Q3PopupMenu*>(w)) {
        w->setWindowOpacity(1.0);
#endif
*/
    } else if(QRubberBand *rubber = ::qt_cast<QRubberBand*>(w)) {
        rubber->setWindowOpacity(1.0);
    } else if(::qt_cast<QMenu*>(w)) {
        w->setWindowOpacity(1.0);
    }
}

int QMacStyleQD::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret = 0;
    switch(metric) {
    case PM_TabBarTabVSpace:
        ret = 4;
        break;
    case PM_CheckListControllerSize:
        break;
    case PM_CheckListButtonSize: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        break; }
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        ret = 0;
        break;
    case PM_DialogButtonsSeparator:
        ret = -5;
        break;
    case PM_DialogButtonsButtonHeight: {
        QSize sz;
        ret = qt_aqua_size_constrain(NULL, QStyle::CT_PushButton, QSize(-1, -1), &sz);
        if(sz == QSize(-1, -1))
            ret = 32;
        else
            ret = sz.height();
        break; }
    case PM_DialogButtonsButtonWidth: {
        QSize sz;
        ret = qt_aqua_size_constrain(NULL, QStyle::CT_PushButton, QSize(-1, -1), &sz);
        if(sz == QSize(-1, -1))
            ret = 70;
        else
            ret = sz.width();
        break; }
    case PM_MenuDesktopFrameWidth:
        ret = 15;
        break;
    case PM_MenuScrollerHeight:
#if 0
        SInt16 ash, asw;
        GetThemeMenuItemExtra(kThemeMenuItemScrollUpArrow, &ash, &asw);
        ret = ash;
#else
        ret = 15; //I hate having magic numbers in here...
#endif
        break;
    case PM_DefaultFrameWidth:
#ifndef QT_NO_MAINWINDOW
        if(widget && (widget->isTopLevel() || !widget->parentWidget()
                || (::qt_cast<QMainWindow*>(widget->parentWidget())
                   && ((QMainWindow*)widget->parentWidget())->centralWidget() == widget))
                && (::qt_cast<QScrollView*>(widget) || widget->inherits("QWorkspaceChild")))
            ret = 0;
        else
#endif
            ret = QWindowsStyle::pixelMetric(metric, widget);
        break;
    case PM_MaximumDragDistance:
        ret = -1;
        break;
    case PM_ScrollBarSliderMin:
        ret = 24;
        break;
    case PM_TabBarBaseHeight:
        ret = 8;
        break;
    case PM_SpinBoxFrameWidth:
        GetThemeMetric(kThemeMetricEditTextFrameOutset, &ret);
        ret += 2;
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;
    case PM_SliderLength:
        ret = 17;
        break;
    case PM_ButtonDefaultIndicator:
        ret = 0;
        break;
    case PM_TitleBarHeight: {
        if(!widget)
            break;
        QTitleBar *tbar = (QTitleBar*)widget;
        ThemeWindowMetrics twm;
        memset(&twm, '\0', sizeof(twm));
        twm.metricSize = sizeof(twm);
        twm.titleWidth = tbar->width();
        twm.titleHeight = tbar->height();
        ThemeWindowAttributes twa = kThemeWindowHasTitleText;
        if(tbar->window())
            twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
        else if(tbar->testWFlags(Qt::WStyle_SysMenu))
            twa |= kThemeWindowHasCloseBox;

        Rect r;
        RgnHandle rgn = qt_mac_get_rgn();
        GetThemeWindowRegion(macWinType, qt_glb_mac_rect(tbar->rect()), kThemeStateActive, &twm, twa, kWindowTitleBarRgn, rgn);
        GetRegionBounds(rgn, &r);
        ret = (r.bottom - r.top);
        qt_mac_dispose_rgn(rgn);
        break; }
    case PM_TabBarTabOverlap:
        GetThemeMetric(kThemeMetricTabOverlap, &ret);
        break;
    case PM_ScrollBarExtent: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricScrollBarWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniScrollBarWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallScrollBarWidth, &ret);
            break;
        }
        break; }
    case PM_TabBarBaseOverlap:
        GetThemeMetric(kThemeMetricTabFrameOverlap, &ret);
        break;
    case PM_IndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxHeight, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxHeight, &ret);
            break;
        }
        break; }
    case PM_IndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        break; }
    case PM_ExclusiveIndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonHeight, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniRadioButtonHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonHeight, &ret);
            break;
        }
        break; }
    case PM_ExclusiveIndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniRadioButtonWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonWidth, &ret);
            break;
        }
        break; }
    case PM_MenuVMargin:
        ret = 4;
        break;
    default:
        ret = QWindowsStyle::pixelMetric(metric, widget);
        break;
    }
    return ret;
}

int QMacStyleQD::styleHint(StyleHint sh, const QWidget *w,
                          const Q3StyleOption &opt,QStyleHintReturn *d) const
{
    SInt32 ret = 0;
    switch(sh) {
    case SH_Menu_AllowActiveAndDisabled:
        ret = false;
        break;
    case SH_Menu_SubMenuPopupDelay:
        ret = 100;
        break;
    case SH_ScrollBar_LeftClickAbsolutePosition:
        extern bool qt_scrollbar_jump_to_pos; //qapplication_mac.cpp
        ret = qt_scrollbar_jump_to_pos;
        break;
    case SH_TabBar_PreferNoArrows:
        ret = true;
        break;
    case SH_LineEdit_PasswordCharacter:
        ret = 0x25AA;
        break;
    case SH_DialogButtons_DefaultButton:
        ret = QDialogButtons::Reject;
        break;
    case SH_GroupBox_TextLabelColor:
        ret = (int) (w ? w->palette().foreground().color().rgb() : 0);
        break;
    case SH_Menu_SloppySubMenus:
        ret = true;
        break;
    case SH_GroupBox_TextLabelVerticalAlignment:
        ret = Qt::AlignTop;
        break;
    case SH_ScrollView_FrameOnlyAroundContents:
        if(w && (w->isTopLevel() || !w->parentWidget() || w->parentWidget()->isTopLevel())
            && (::qt_cast<QScrollView*>(w) || w->inherits("QWorkspaceChild")))
            ret = true;
        else
            ret = QWindowsStyle::styleHint(sh, w, opt, d);
        break;
    case SH_Menu_FillScreenWithScroll:
        ret = (QSysInfo::MacintoshVersion < QSysInfo::MV_PANTHER);
        break;
    case SH_Menu_Scrollable:
        ret = true;
        break;
    case SH_RichText_FullWidthSelection:
        ret = true;
        break;
    case SH_BlinkCursorWhenTextSelected:
        ret = false;
        break;
    case SH_ScrollBar_StopMouseOverSlider:
        ret = true;
        break;
    case SH_ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
        break;
    case SH_ComboBox_Popup:
        ret = (!w || !((QComboBox*)w)->editable());
        break;
    case SH_Workspace_FillSpaceOnMaximize:
        ret = true;
        break;
    case SH_Widget_ShareActivation:
        ret = true;
        break;
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignRight;
        break;
    case SH_TabBar_Alignment:
        ret = Qt::AlignHCenter;
        break;
    case SH_UnderlineShortcut:
        ret = false;
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, w, opt, d);
        break;
    }
    return ret;
}

QPixmap QMacStyleQD::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                                const QPalette &pal, const Q3StyleOption &opt) const
{
    switch(pixmaptype) {
    case PT_Pressed:
    case PT_Disabled: {
        QImage img;
        img = pixmap;
        QRgb pixel;
        for(int y = 0; y < img.height(); y++) {
            for(int x = 0; x < img.width(); x++) {
                pixel = img.pixel(x, y);
                if(pixmaptype == PT_Pressed) {
                    QColor hsvColor(pixel);
                    int h, s, v;
                    hsvColor.getHsv(&h, &s, &v);
                    s = (s / 2);
                    v = (v / 2) + 117; //a little less than half 128 (ie 0xFF/2)
                    hsvColor.setHsv(h, s, v);
                    pixel = hsvColor.rgb();
                } else if(pixmaptype == PT_Disabled) {
                    pixel = qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), qAlpha(pixel)/2);
                }
                img.setPixel(x, y, pixel);
            }
        }
        QPixmap ret(img);
        if(pixmap.mask())
            ret.setMask(*pixmap.mask());
        return ret;
    }
    default:
        break;
    }
    return QCommonStyle::stylePixmap(pixmaptype, pixmap, pal, opt);
}

QPixmap QMacStyleQD::stylePixmap(StylePixmap stylepixmap, const QWidget *widget,
                                 const Q3StyleOption& opt) const
{
    IconRef icon = 0;
    switch(stylepixmap) {
    case SP_MessageBoxInformation:
        GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertNoteIcon, &icon);
        break;
    case SP_MessageBoxWarning:
        GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertCautionIcon, &icon);
        break;
    case SP_MessageBoxCritical:
        GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertStopIcon, &icon);
        break;
    case SP_MessageBoxQuestion:
        //no idea how to do this ###
    default:
        break;
    }
    if(icon) {
        QPixmap ret = qt_mac_convert_iconref(icon, 64, 64);
        ReleaseIconRef(icon);
        return ret;
    }
    return QWindowsStyle::stylePixmap(stylepixmap, widget, opt);
}

void QMacStyleQD::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                const QWidget *w) const
{
    ThemeDrawState tds = QAquaAnimate::getDrawState(opt->state, opt->palette);
    switch (pe) {
    case PE_CheckListExclusiveIndicator:
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator:
    case PE_CheckListIndicator:
    case PE_IndicatorMask:
    case PE_Indicator:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            bool isRadioButton = (pe == PE_CheckListIndicator || pe == PE_ExclusiveIndicator
                                  || pe == PE_ExclusiveIndicatorMask);
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentDrawIndicatorOnly };
            if (btn->state & Style_HasFocus
                    && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                info.adornment |= kThemeAdornmentFocus;
            if (btn->state & Style_NoChange)
                info.value = kThemeButtonMixed;
            else if (btn->state & Style_On)
                info.value = kThemeButtonOn;
            ThemeButtonKind bkind;
            switch (qt_mac_get_size_for_painter(p)) {
                case QAquaSizeUnknown:
                case QAquaSizeLarge:
                    if (isRadioButton)
                        bkind = kThemeRadioButton;
                    else
                        bkind = kThemeCheckBox;
                    break;
                case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                        if (isRadioButton)
                            bkind = kThemeMiniRadioButton;
                        else
                            bkind = kThemeMiniCheckBox;
                        break;
                    }
#endif
                case QAquaSizeSmall:
                    if (isRadioButton)
                        bkind = kThemeSmallRadioButton;
                    else
                        bkind = kThemeSmallCheckBox;
                    break;
            }
            if (pe == PE_ExclusiveIndicatorMask || pe == PE_IndicatorMask) {
                p->save();
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeButtonRegion(qt_glb_mac_rect(btn->rect, p, false), bkind, &info, rgn);
                p->setClipRegion(qt_mac_convert_mac_region(rgn));
                qt_mac_dispose_rgn(rgn);
                p->fillRect(btn->rect, Qt::color1);
                p->restore();
            } else {
                static_cast<QMacStyleQDPainter *>(p)->setport();
                DrawThemeButton(qt_glb_mac_rect(btn->rect, p, false), bkind, &info, 0, 0, 0, 0);
            }
        }
        break;
    case PE_FocusRect:
        break;     //This is not used because of the QAquaFocusWidget thingie..
    case PE_TreeBranch:
        if (!(opt->state & Style_Children))
            break;
        ThemeButtonDrawInfo currentInfo;
        currentInfo.state = opt->state & Style_Enabled ? kThemeStateActive : kThemeStateInactive;
        if (opt->state & Style_Down)
            currentInfo.state |= kThemeStatePressed;
        currentInfo.value = opt->state & Style_Open ? kThemeDisclosureDown : kThemeDisclosureRight;
        currentInfo.adornment = kThemeAdornmentNone;
        static_cast<QMacStyleQDPainter *>(p)->setport();
        DrawThemeButton(qt_glb_mac_rect(opt->rect, p), kThemeDisclosureButton, &currentInfo,
                        0, 0, 0, 0);
        break;
    case PE_RubberBandMask:
        p->fillRect(opt->rect, Qt::color1);
        break;
    case PE_RubberBand:
        p->fillRect(opt->rect, opt->palette.highlight());
        break;
    case PE_SizeGrip: {
        const Rect *rect = qt_glb_mac_rect(opt->rect, p);
        Point orig = { rect->top, rect->left };
        static_cast<QMacStyleQDPainter *>(p)->setport();
        ThemeGrowDirection dir = kThemeGrowRight | kThemeGrowDown;
#if 0
        if(QApplication::reverseLayout())
            dir = kThemeGrowLeft | kThemeGrowDown;
#endif
        DrawThemeStandaloneGrowBox(orig, dir, false, kThemeStateActive);
        break; }
    case PE_HeaderArrow:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            if (w && w->parentWidget()->inherits("QTable"))
                drawPrimitive(header->state & Style_Up ? PE_ArrowUp : PE_ArrowDown, header, p, w);
        }
        // else drawn in HeaderSection.
        break;
    case PE_HeaderSection:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            ThemeButtonKind bkind = kThemeListHeaderButton;
            SFlags flags = header->state;
            if (w && w->parentWidget()->inherits("QTable")) {
                bkind = kThemeBevelButton;
                if (p->font().bold())
                    flags |= Style_Sunken;
                else
                    flags &= ~Style_Sunken;
            }
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            QWidget *w = 0;

            if (flags & Style_HasFocus && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                info.adornment |= kThemeAdornmentFocus;
            if (qAquaActive(header->palette)) {
                if (!(flags & Style_Enabled))
                    info.state = kThemeStateUnavailable;
                else if (flags & Style_Down)
                    info.state = kThemeStatePressed;
            } else {
                if (flags & Style_Enabled)
                    info.state = kThemeStateInactive;
                else
                    info.state = kThemeStateUnavailableInactive;
            }
            if (flags & Style_Sunken)
                info.value = kThemeButtonOn;

            QRect ir = header->rect;
            if (flags & Style_Off)
                ir.setRight(ir.right() + 50);
            else if (flags & Style_Up)
                info.adornment |= kThemeAdornmentHeaderButtonSortUp;
            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeButton(qt_glb_mac_rect(ir, p, false), bkind, &info, 0, 0, 0, 0);
        }
        break;
    case PE_Panel:
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            if (opt->state & Style_Sunken) {
                SInt32 frame_size;
                if (pe == PE_PanelLineEdit)
                    GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
                else
                    GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);

                int lw = frame->lineWidth;
                if (lw <= 0)
                    pixelMetric(PM_DefaultFrameWidth, 0);

                p->fillRect(frame->rect.x(), frame->rect.y(), lw, frame->rect.height(),
                            frame->palette.background()); //left
                p->fillRect(frame->rect.right()-lw+1, frame->rect.y(), lw, frame->rect.height(),
                            frame->palette.background()); //right
                p->fillRect(frame->rect.x(), frame->rect.y(), frame->rect.width(), lw,
                            frame->palette.background()); //top
                p->fillRect(frame->rect.x(), frame->rect.bottom()-lw+1, frame->rect.width(), lw,
                            frame->palette.background()); //bottm

                const Rect *rect = qt_glb_mac_rect(frame->rect, p, false,
                                                   QRect(frame_size, frame_size, frame_size * 2,
                                                         frame_size * 2));
                static_cast<QMacStyleQDPainter *>(p)->setport();
                if (pe == PE_PanelLineEdit)
                    DrawThemeEditTextFrame(rect, tds);
                else
                    DrawThemeListBoxFrame(rect, tds);
            } else {
                QWindowsStyle::drawPrimitive(pe, frame, p, w);
            }
        }
        break;
    case PE_PanelGroupBox:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            static_cast<QMacStyleQDPainter *>(p)->setport();
#ifdef QMAC_DO_SECONDARY_GROUPBOXES
            if (w && ::qt_cast<QGroupBox *>(w->parentWidget()))
                DrawThemeSecondaryGroup(qt_glb_mac_rect(frame->rect, p), kThemeStateActive);
            else
#endif
                DrawThemePrimaryGroup(qt_glb_mac_rect(frame->rect, p), kThemeStateActive);
        }
        break;
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        ThemeArrowOrientation orientation;
        switch (pe) {
        case PE_ArrowUp:
            orientation = kThemeArrowUp;
            break;
        case PE_ArrowDown:
            orientation = kThemeArrowDown;
            break;
        case PE_ArrowRight:
            orientation = kThemeArrowRight;
            break;
        case PE_ArrowLeft:
            orientation = kThemeArrowLeft;
        default:
            break;
        }
        ThemePopupArrowSize size;
        if (opt->rect.width() < 8)
            size = kThemeArrow5pt;
        else
            size = kThemeArrow9pt;
        static_cast<QMacStyleQDPainter *>(p)->setport();
        DrawThemePopupArrow(qt_glb_mac_rect(opt->rect, p), orientation, size, tds, 0, 0);
        break; }
    default:
        QWindowsStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
}

void QMacStyleQD::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                              const QWidget *widget) const
{
    ThemeDrawState tds = d->getDrawState(opt->state, opt->palette);
    switch (ce) {
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            if (!(btn->state & (Style_Raised | Style_Down | Style_On)))
                break;
            QString pmkey;
            bool do_draw = false;
            QPixmap buffer;
            bool darken = d->animatable(QAquaAnimate::AquaPushButton, widget);
            int frame = d->buttonState.frame;
            if (btn->state & Style_On) {
                darken = true;
                frame = 12;
                if (btn->state & Style_Down)
                    frame += 8;
            } else if (btn->state & Style_Down) {
                darken = false;
                frame = 0;
            }
            if (darken && qAquaActive(btn->palette)) {
                QTextOStream os(&pmkey);
                os << "$qt_mac_pshbtn_" << opt->rect.width() << "x" << opt->rect.height() << "_"
                   << opt->state << "_" << frame;
                tds = kThemeStatePressed;
                if(frame && !QPixmapCache::find(pmkey, buffer)) {
                    do_draw = true;
                    buffer = QPixmap(opt->rect.width(), opt->rect.height(), 32);
                    buffer.fill(Qt::color0);
                }
            }
            ThemeButtonKind bkind;
            if (btn->extras != QStyleOptionButton::None)
                bkind = kThemeBevelButton;
            else
                bkind = kThemePushButton;
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
            if (opt->state & Style_HasFocus
                    && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                info.adornment |= kThemeAdornmentFocus;
            QRect off_rct;
            { //The AppManager draws outside my rectangle, so account for that difference..
                Rect macRect, myRect;
                SetRect(&myRect, btn->rect.x(), btn->rect.y(), btn->rect.width(),
                        btn->rect.height());
                GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                off_rct = QRect(myRect.left - macRect.left, myRect.top - macRect.top,
                        (myRect.left - macRect.left) + (macRect.right - myRect.right),
                        (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
            }
            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeButton(qt_glb_mac_rect(opt->rect, p, false, off_rct), bkind, &info, 0, 0, 0, 0);
            if (!buffer.isNull() && bkind == kThemePushButton) {
                if (do_draw && frame) {
                    QMacSavedPortInfo savedInfo(&buffer);
                    const Rect *buff_rct = qt_glb_mac_rect(QRect(0, 0, btn->rect.width(),
                                                           btn->rect.height()), &buffer, false,
                                                           off_rct);
                    DrawThemeButton(buff_rct, bkind, &info, 0, 0, 0, 0);
                    QPixmap buffer_mask(buffer.size(), 32);
                    buffer_mask.fill(Qt::color0);
                    ThemeButtonDrawInfo mask_info = info;
                    mask_info.state = kThemeStateActive;
                    {
                        QMacSavedPortInfo savedInfo(&buffer_mask);
                        DrawThemeButton(buff_rct, bkind, &mask_info, 0, 0, 0, 0);
                    }
                    QImage img = buffer.convertToImage(),
                           maskimg = buffer_mask.convertToImage();
                    QImage mask_out(img.width(), img.height(), 1, 2, QImage::LittleEndian);
                    for (int y = 0; y < img.height(); ++y) {
                        //calculate a mask
                        for(int maskx = 0; maskx < img.width(); ++maskx) {
                            QRgb in = img.pixel(maskx, y), out = maskimg.pixel(maskx, y);
                            int diff = (((qRed(in)-qRed(out))*((qRed(in)-qRed(out)))) +
                                    ((qGreen(in)-qGreen(out))*((qGreen(in)-qGreen(out)))) +
                                    ((qBlue(in)-qBlue(out))*((qBlue(in)-qBlue(out)))));
                            mask_out.setPixel(maskx, y, diff > 100);
                        }
                        //pulse the colors
                        uchar *bytes = img.scanLine(y);
                        for(int x = 0; x < img.bytesPerLine(); x++)
                            *(bytes + x) = (*(bytes + x) * (100 - frame)) / 100;
                    }
                    buffer = img;
                    QBitmap qmask(mask_out);
                    buffer.setMask(qmask);
                }
                p->drawPixmap(opt->rect, buffer);
                if (do_draw)
                    QPixmapCache::insert(pmkey, buffer);
            }
            if (btn->extras & QStyleOptionButton::HasMenu) {
                int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() / 2 - 5, mbi, ir.height() / 2);
                drawPrimitive(PE_ArrowDown, &newBtn, p, widget);
            }
        }
        break;
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            bool dis = !(mi->state & Style_Enabled);
            int tab = mi->tabWidth;
            int maxpmw = mi->maxIconWidth;
            bool checkable = mi->checkState != QStyleOptionMenuItem::NotCheckable;
            bool checked = mi->checkState == QStyleOptionMenuItem::Checked;
            bool act = mi->state & Style_Active;
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
            irect = *qt_glb_mac_rect(mi->rect, p, false);

            if (checkable)
                maxpmw = qMax(maxpmw, 12); // space for the checkmarks

            ThemeMenuState tms = kThemeMenuActive;
            if (dis)
                tms |= kThemeMenuDisabled;
            if (act)
                tms |= kThemeMenuSelected;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                tmit |= kThemeMenuItemHierarchical;
            if (!mi->icon.isNull())
                tmit |= kThemeMenuItemHasIcon;
            static_cast<QMacStyleQDPainter *>(p)->setport();
            if (mi->menuItemType != QStyleOptionMenuItem::Separator) {
                DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);
            } else {
                DrawThemeMenuSeparator(&irect);
                return;
            }

            int x, y, w, h;
            mi->rect.rect(&x, &y, &w, &h);
            int checkcol = maxpmw;
            bool reverse = QApplication::reverseLayout();
            int xpos = x;
            if (reverse)
                xpos += w - checkcol;

            if (!mi->icon.isNull()) {              // draw iconset
                if (checked) {
                    QRect vrect = visualRect(QRect(xpos, y, checkcol, h), mi->rect);
                    if (act && !dis) {
                        qDrawShadePanel(p, vrect.x(), y, checkcol, h,
                                        mi->palette, true, 1, &mi->palette.brush(QPalette::Button));
                    } else {
                        QBrush fill(mi->palette.light(), Qt::Dense4Pattern);
                        // set the brush origin for the hash pattern to the x/y coordinate
                        // of the menu item's checkmark... this way, the check marks have
                        // a consistent look
                        QPoint origin = p->brushOrigin();
                        p->setBrushOrigin(vrect.x(), y);
                        qDrawShadePanel(p, vrect.x(), y, checkcol, h, mi->palette, true, 1, &fill);
                        // restore the previous brush origin
                        p->setBrushOrigin(origin);
                    }
                }
                QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
                if(act && !dis)
                    mode = QIconSet::Active;
                QPixmap pixmap;
                if (mi) {
                    if (checked)
                        pixmap = mi->icon.pixmap(QIconSet::Small, mode, QIconSet::On);
                    else
                        pixmap = mi->icon.pixmap(QIconSet::Small, mode);
                }
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                if(act && !dis && mi->checkState == QStyleOptionMenuItem::Checked)
                    qDrawShadePanel(p, xpos, y, checkcol, h, mi->palette, false, 1,
                                    &mi->palette.brush(QPalette::Button));
                QRect cr(xpos, y, checkcol, h);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->setPen(mi->palette.text());
                p->drawPixmap(pmr.topLeft(), pixmap);
            } else  if (checked) {  // just "checking"...
                QStyleOptionMenuItem newMi = *mi;
                newMi.state = Style_Default;
                int mw = checkcol + macItemFrame;
                int mh = h - 2*macItemFrame;
                int xp = xpos;
                if (reverse)
                    xp -= macItemFrame;
                else
                    xp += macItemFrame;
                if (!dis)
                    newMi.state |= Style_Enabled;
                if (act)
                    newMi.state |= Style_On;
                newMi.rect = QRect(xp, y+macItemFrame, mw, mh);
                drawPrimitive(PE_CheckMark, &newMi, p, widget);
            }

            if (dis)
                p->setPen(mi->palette.text());
            else if (act)
                p->setPen(mi->palette.highlightedText());
            else
                p->setPen(mi->palette.buttonText());

            int xm = macItemFrame + checkcol + macItemHMargin;
            if (reverse)
                xpos = macItemFrame + tab;
            else
                xpos += xm;
            if (mi->menuItemType == QStyleOptionMenuItem::Q3Custom) {
                // ### hmmm...
                /*
                int m = macItemVMargin;
                mi->custom()->paint(p, pal, act, !dis, x+xm, y+m, w-xm-tab+1, h-2*m);
                */
            }
            if (mi) {
                QString s = mi->text;
                if (!s.isEmpty()) {                        // draw text
                    int t = s.indexOf('\t');
                    int m = macItemVMargin;
                    int text_flags = Qt::AlignRight | Qt::AlignVCenter | Qt::NoAccel | Qt::SingleLine;
                    if (t >= 0) {                         // draw tab text
                        int xp;
                        if (reverse)
                            xp = x + macRightBorder+macItemHMargin+macItemFrame - 1;
                        else
                            xp = x + w - tab - macRightBorder-macItemHMargin-macItemFrame + 1;
                        QFont font(p->font());
                        int oldWeight = font.weight();
                        font.setWeight(QFont::Bold);
                        p->setFont(font);
                        p->drawText(xp, y + m, tab, h - 2 * m, text_flags, s.mid(t + 1));
                        s = s.left(t);
                        font.setWeight(oldWeight);
                        p->setFont(font);
                    }
                    text_flags ^= Qt::AlignRight;
                    p->drawText(xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
                }
            }
        }
        break;
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuTearoff:
    case CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
                 irect = *qt_glb_mac_rect(mi->rect, p, false);
            ThemeMenuState tms = kThemeMenuActive;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if (opt->state & Style_Active)
                tms |= kThemeMenuSelected;
            if (ce == CE_MenuScroller) {
                if (opt->state & Style_Down)
                    tmit = kThemeMenuItemScrollDownArrow;
                else
                    tmit = kThemeMenuItemScrollUpArrow;
            }
            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);
            if (ce == CE_MenuTearoff) {
                p->setPen(QPen(mi->palette.dark(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                            mi->rect.x() + mi->rect.width() - 4, mi->rect.y() + mi->rect.height() / 2 - 1);
                p->setPen(QPen(mi->palette.light(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                            mi->rect.x() + mi->rect.width() - 4, mi->rect.y() + mi->rect.height() / 2);
            }
        }
        break;
    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            QRect ir(mi->rect.x(), 0, mi->rect.width(), mi->menuRect.height());
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
                 irect = *qt_glb_mac_rect(ir, p, false);
            ThemeMenuState tms = kThemeMenuActive;
            if (!(mi->state & Style_Active))
                tms = kThemeMenuDisabled;
            if (mi->state & Style_Down)
                tms = kThemeMenuSelected;
            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeMenuTitle(&mrect, &irect, tms, 0, 0, 0);
            QWindowsStyle::drawControl(ce, mi, p, widget);
        }
        break;
    case CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeMenuBarBackground(qt_glb_mac_rect(mi->rect, p, false), kThemeMenuBarNormal,
                                       kThemeMenuSquareMenuBar);
        }
        break;
    case CE_ProgressBarGroove:
    case CE_ProgressBarLabel:
        break;
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            ThemeTrackDrawInfo tdi;
            tdi.filler1 = 0;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                tdi.kind = pb->totalSteps ? kThemeLargeProgressBar : kThemeLargeIndeterminateBar;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                    tdi.kind = pb->totalSteps ? kThemeMiniProgressBar : kThemeMiniIndeterminateBar;
                    break;
                }
#endif
            case QAquaSizeSmall:
                tdi.kind = pb->totalSteps ? kThemeProgressBar : kThemeIndeterminateBar;
                break;
            }
            tdi.bounds = *qt_glb_mac_rect(opt->rect, p);
            tdi.max = pb->totalSteps;
            tdi.min = 0;
            tdi.value = pb->progress;
            tdi.attributes = kThemeTrackHorizontal;
            tdi.trackInfo.progress.phase = d->progressbarState.frame;
            if (!qAquaActive(pb->palette))
                tdi.enableState = kThemeTrackInactive;
            else if (!(pb->state & Style_Enabled))
                tdi.enableState = kThemeTrackDisabled;
            else
                tdi.enableState = kThemeTrackActive;
            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeTrack(&tdi, 0, 0, 0);
        }
        break;
    case CE_HeaderLabel:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            QRect textr = header->rect;
            if (!header->icon.isNull()) {
                QIconSet::Mode mode = QIconSet::Disabled;
                if (opt->state & Style_Enabled)
                    mode = QIconSet::Normal;
                QPixmap pixmap = header->icon.pixmap(QIconSet::Small, mode);

                QRect pixr = header->rect;
                pixr.setY(header->rect.center().y() - (pixmap.height() - 1) / 2);
                drawItem(p, pixr, Qt::AlignVCenter, header->palette,
                         mode != QIconSet::Disabled
                         || !header->icon.isGenerated(QIconSet::Small, mode), pixmap);
                textr.moveBy(pixmap.width() + 2, 0);
            }

            // change the color to bright text if we are a table header and selected.
            const QColor *penColor = &header->palette.buttonText().color();
            if (widget && widget->parentWidget()->inherits("QTable") && p->font().bold())
                penColor = &header->palette.color(QColorGroup::BrightText);
            drawItem(p, textr, Qt::AlignVCenter, header->palette, header->state & Style_Enabled,
                     header->text, -1, penColor);
        }
        break;
    case CE_ToolBoxTab:
        QCommonStyle::drawControl(ce, opt, p, widget);
        break;
    default:
        QWindowsStyle::drawControl(ce, opt, p, widget);
    }
}

QRect QMacStyleQD::subRect(SubRect sr, const QStyleOption *opt, const QWidget *widget) const
{
    QRect r = QRect();
    switch (sr) {
    case SR_PushButtonContents:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            Rect macRect, myRect;
            SetRect(&myRect, 0, 0, btn->rect.width(), btn->rect.height());
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            GetThemeButtonContentBounds(&myRect, kThemePushButton, &info, &macRect);
            r = QRect(macRect.left, macRect.top,
                      qMin(btn->rect.width() - 2 * macRect.left, macRect.right - macRect.left),
                      qMin(btn->rect.height() - 2 * macRect.top, macRect.bottom - macRect.top));
        }
        break;
    case SR_ProgressBarContents:
        r = opt->rect;
        break;
    case SR_ProgressBarGroove:
    case SR_ProgressBarLabel:
        break;
    default:
        r = QWindowsStyle::subRect(sr, opt, widget);
        break;
    }
    return r;
}

void QMacStyleQD::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                     QPainter *p, const QWidget *widget) const
{
    ThemeDrawState tds = d->getDrawState(opt->state, opt->palette);
    switch (cc) {
    case CC_Slider:
    case CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, p, &tdi, widget);
            if (cc == CC_Slider) {
                if (slider->activeParts == SC_SliderGroove)
                    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
                else if (slider->activeParts == SC_SliderHandle)
                    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
            } else {
                if (slider->activeParts == SC_ScrollBarSubLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                         | kThemeLeftOutsideArrowPressed;
                else if (slider->activeParts == SC_ScrollBarAddLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                         | kThemeRightOutsideArrowPressed;
                else if(slider->activeParts == SC_ScrollBarAddPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
                else if(slider->activeParts == SC_ScrollBarSubPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
                else if(slider->activeParts == SC_ScrollBarSlider)
                    tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            }

            //The AppManager draws outside my rectangle, so account for that difference..
            Rect macRect;
            GetThemeTrackBounds(&tdi, &macRect);
            tdi.bounds.left  += tdi.bounds.left  - macRect.left;
            tdi.bounds.right -= macRect.right - tdi.bounds.right;

            bool tracking = slider->sliderPosition == slider->sliderValue;
            RgnHandle r;
            if (!tracking) {
                r = qt_mac_get_rgn();
                GetThemeTrackThumbRgn(&tdi, r);
                tdi.value = slider->sliderValue;
            }

            static_cast<QMacStyleQDPainter *>(p)->setport();
            DrawThemeTrack(&tdi, tracking ? 0 : r, 0, 0);
            if (!tracking)
                qt_mac_dispose_rgn(r);
            if (slider->parts & SC_SliderTickmarks) {
                int numTicks;
                if (slider->tickInterval) {
                    if (slider->orientation == Qt::Horizontal)
                        numTicks = slider->rect.width() / slider->tickInterval;
                    else
                        numTicks = slider->rect.height() / slider->tickInterval;
                } else {
                    numTicks = (slider->maximum - slider->minimum + 1) / slider->pageStep;
                }
                if (tdi.trackInfo.slider.thumbDir == kThemeThumbPlain) {
                    tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
                    DrawThemeTrackTickMarks(&tdi, numTicks, 0, 0);
                    tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
                    DrawThemeTrackTickMarks(&tdi, numTicks, 0, 0);
                } else {
                    DrawThemeTrackTickMarks(&tdi, numTicks, 0, 0);
                }
            }
        }
        break;
    case CC_ListView:
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if (lv->parts & SC_ListView)
                QWindowsStyle::drawComplexControl(cc, lv, p, widget);

            if (lv->parts & (SC_ListViewBranch | SC_ListViewExpand)) {
                int y = lv->rect.y(),
                h = lv->rect.height(),
                x = lv->rect.right() - 10;
                static_cast<QMacStyleQDPainter *>(p)->setport();
                ::RGBColor f;
                f.red = lv->viewportPalette.color(lv->viewportBGRole).red() * 256;
                f.green = lv->viewportPalette.color(lv->viewportBGRole).green() * 256;
                f.blue = lv->viewportPalette.color(lv->viewportBGRole).blue() * 256;
                RGBBackColor(&f);

                QPixmap pm;
                QPainter pm_paint;
                for (int i = 1; i < lv->items.size() && y < h; ++i) {
                    QStyleOptionListViewItem child = lv->items.at(i);
                    if (y + child.height > 0 && (child.childCount > 0
                        || child.extras & QStyleOptionListViewItem::Expandable)) {
                        QStyleOption treeOpt(0, QStyleOption::Default);
                        treeOpt.rect.setRect(x, y + child.height / 2 - 4, 9, 9);
                        treeOpt.palette = lv->palette;
                        treeOpt.state = lv->state;
                        treeOpt.state |= Style_Children;
                        if (child.state & Style_Open)
                            treeOpt.state |= Style_Open;
                        drawPrimitive(PE_TreeBranch, &treeOpt, p, widget);
                    }
                    y += child.totalHeight;
                }
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox newSB = *sb;
            if (sb->parts & SC_SpinBoxFrame) {
                QStyleOptionFrame lineedit(0);
                lineedit.rect = querySubControlMetrics(CC_SpinBox, sb, SC_SpinBoxFrame, widget),
                lineedit.palette = sb->palette;
                lineedit.state = Style_Sunken;
                lineedit.lineWidth = 0;
                lineedit.midLineWidth = 0;
                drawPrimitive(PE_PanelLineEdit, &lineedit, p, widget);
            }
            if (sb->parts & (SC_SpinBoxDown | SC_SpinBoxUp)) {
                if (!(sb->stepEnabled & (QAbstractSpinBox::StepUpEnabled
                                        | QAbstractSpinBox::StepDownEnabled)))
                    tds = kThemeStateUnavailable;
                if (sb->activeParts == SC_SpinBoxDown)
                    tds = kThemeStatePressedDown;
                else if (sb->activeParts == SC_SpinBoxUp)
                    tds = kThemeStatePressedUp;
                ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                if (sb->state & Style_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    info.adornment |= kThemeAdornmentFocus;
                QRect updown = visualRect(querySubControlMetrics(CC_SpinBox, sb, SC_SpinBoxUp,
                                                                 widget), widget);
                updown |= visualRect(querySubControlMetrics(CC_SpinBox, sb, SC_SpinBoxDown,
                                                            widget), widget);
                if (widget) {
                    QPalette::ColorRole bgRole = widget->backgroundRole();
                    if (sb->palette.brush(bgRole).pixmap())
                        p->drawPixmap(updown, *sb->palette.brush(bgRole).pixmap());
                    else
                        p->fillRect(updown, sb->palette.color(bgRole));
                }
                static_cast<QMacStyleQDPainter *>(p)->setport();
                ThemeButtonKind kind = kThemeIncDecButton;
                switch (qt_aqua_size_constrain(widget)) {
                case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                        kind = kThemeIncDecButtonMini;
                        break;
                    }
#endif
                case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                        kind = kThemeIncDecButtonSmall;
                        break;
                    }
#endif
                case QAquaSizeUnknown:
                case QAquaSizeLarge:
                    kind = kThemeIncDecButton;
                    break;
                }
                DrawThemeButton(qt_glb_mac_rect(updown, p), kind, &info, 0, 0, 0, 0);
            }
        }
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            ThemeButtonKind bkind = kThemeBevelButton;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bkind = kThemeBevelButton;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                    bkind = kThemeMiniBevelButton;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bkind = kThemeSmallBevelButton;
                break;
            }

            QRect button, menuarea;
            button   = querySubControlMetrics(cc, tb, SC_ToolButton, widget);
            menuarea = querySubControlMetrics(cc, tb, SC_ToolButtonMenu, widget);
            SFlags bflags = tb->state,
            mflags = tb->state;
            if (tb->parts & SC_ToolButton)
                bflags |= Style_Down;
            if (tb->parts & SC_ToolButtonMenu)
                mflags |= Style_Down;

            if (tb->parts & SC_ToolButton) {
                if(bflags & (Style_Down | Style_On | Style_Raised)) {
                    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                    if (tb->state & Style_HasFocus && QMacStyle::focusRectPolicy(widget)
                            != QMacStyle::FocusDisabled)
                        info.adornment |= kThemeAdornmentFocus;
                    if (tb->state & (Style_On | Style_Down))
                        info.value |= kThemeStatePressed;

                    QRect off_rct(0, 0, 0, 0);
                    { //The AppManager draws outside my rectangle, so account for that difference..
                        Rect macRect, myRect;
                        SetRect(&myRect, tb->rect.x(), tb->rect.y(), tb->rect.width(), tb->rect.height());
                        GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                        off_rct = QRect(myRect.left - macRect.left, myRect.top - macRect.top,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
                    }

                    // If the background color is set then make the toolbutton
                    // translucent so the background color is visible
                    if (tb->palette.color(tb->bgRole) != Qt::white) {
                        p->fillRect(tb->rect, tb->palette.color(tb->bgRole));
                        info.state = kThemeStateInactive;
                    }

                    static_cast<QMacStyleQDPainter *>(p)->setport();
                    DrawThemeButton(qt_glb_mac_rect(button, p, false, off_rct),
                                    bkind, &info, 0, 0, 0, 0);
                } else if (tb->parentPalette.brush(tb->parentBGRole).pixmap() &&
                           !tb->parentPalette.brush(tb->parentBGRole).pixmap()->isNull()) {
                    p->drawTiledPixmap(tb->rect, *tb->parentPalette.brush(tb->parentBGRole).pixmap(),
                                       tb->pos);
                }
            }

            if (tb->parts & SC_ToolButtonMenu) {
                ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                if (tb->state & Style_HasFocus && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    info.adornment |= kThemeAdornmentFocus;
                if (tb->state & (Style_On | Style_Down) || (tb->activeParts & SC_ToolButtonMenu))
                    info.value |= kThemeStatePressed;
                static_cast<QMacStyleQDPainter *>(p)->setport();
                DrawThemeButton(qt_glb_mac_rect(menuarea, p, false), bkind, &info, 0, 0, 0, 0);
                QRect r(menuarea.x() + ((menuarea.width() / 2) - 4), menuarea.height() - 8, 8, 8);
                DrawThemePopupArrow(qt_glb_mac_rect(r, p), kThemeArrowDown, kThemeArrow7pt, tds,
                                    0, 0);
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt) ) {
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
            if (cmb->state & Style_HasFocus)
                info.adornment |= kThemeAdornmentFocus;
            if (cmb->activeParts & QStyle::SC_ComboBoxArrow)
                info.state = kThemeStatePressed;
            p->fillRect(cmb->rect, cmb->palette.brush(QPalette::Button)); //make sure it is filled
            if (cmb->editable) {
                info.adornment |= kThemeAdornmentArrowDownArrow;
                QRect buttonR = querySubControlMetrics(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                static_cast<QMacStyleQDPainter *>(p)->setport();

                ThemeButtonKind bkind = kThemeArrowButton;
                switch (qt_aqua_size_constrain(widget)) {
                    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                            bkind = kThemeArrowButtonMini;
                            break;
                        }
#endif
                    case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                            bkind = kThemeArrowButtonSmall;
                            break;
                        }
#endif
                    case QAquaSizeUnknown:
                    case QAquaSizeLarge:
                        bkind = kThemeArrowButton;
                        break;
                }
                DrawThemeButton(qt_glb_mac_rect(buttonR, p, true, QRect(1, 0, 0, 0)),
                                bkind, &info, 0, 0, 0, 0);
            } else {
                info.adornment |= kThemeAdornmentArrowLeftArrow;
                static_cast<QMacStyleQDPainter *>(p)->setport();
                DrawThemeButton(qt_glb_mac_rect(cmb->rect, p, true, QRect(1, 0, 0, 0)),
                                kThemePopupButton, &info, 0, 0, 0, 0);
            }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
        ThemeWindowMetrics twm;
        memset(&twm, 0, sizeof(twm));
        twm.metricSize = sizeof(twm);
        twm.titleWidth = tbar->rect.width();
        twm.titleHeight = tbar->rect.height();
        ThemeWindowAttributes twa = kThemeWindowHasTitleText;
        if (tbar->titleBarState) {
            if (tbar->titleBarState & Qt::WindowMinimized)
                twa |= kThemeWindowIsCollapsed;
            /*
            if (tbar->window()->isWindowModified())
                twa |= kThemeWindowHasDirty;
                */
            twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
        } else if (tbar->titleBarFlags & Qt::WStyle_SysMenu) {
            twa |= kThemeWindowHasCloseBox;
        }
        QString dblbuf_key;

        //AppMan paints outside the given rectangle, so I have to adjust for the height properly!
        QRect newr = tbar->rect;
        {
            Rect br;
            RgnHandle rgn = qt_mac_get_rgn();
            GetThemeWindowRegion(macWinType, qt_glb_mac_rect(tbar->rect), tds, &twm, twa,
                                 kWindowTitleBarRgn, rgn);
            GetRegionBounds(rgn, &br);
            newr.moveBy(newr.x() - br.left, newr.y() - br.top);
            qt_mac_dispose_rgn(rgn);
        }
        if (tbar->parts & SC_TitleBarLabel) {
            int iw = 0;
            if (!tbar->icon.isNull()) {
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeWindowRegion(macWinType, qt_glb_mac_rect(newr), tds, &twm, twa,
                                     kWindowTitleProxyIconRgn, rgn);
                if (!EmptyRgn(rgn))
                    iw = tbar->icon.width();
                qt_mac_dispose_rgn(rgn);
            }
            if (!tbar->text.isEmpty()) {
                QString pmkey;
                QTextOStream os(&pmkey);
                os << "$qt_mac_style_titlebar_" << "_" << newr.width()
                   << "x" << newr.height() << "_" << twa << "_" << tds << "_" << tbar->text;
                if (QPixmap *dblbuf = QPixmapCache::find(pmkey)) {
                    p->drawPixmap(tbar->rect.topLeft(), *dblbuf);
                } else {
                    QPixmap pix(newr.width(), newr.height(), 32);
                    QPainter pixp(&pix);

                    static_cast<QMacStyleQDPainter *>(&pixp)->setport();
                    DrawThemeWindowFrame(macWinType, qt_glb_mac_rect(newr, &pixp, false), tds,
                                         &twm, twa, NULL, 0);

                    pixp.save();
                    {
                        RgnHandle rgn = qt_mac_get_rgn();
                        GetThemeWindowRegion(macWinType, qt_glb_mac_rect(newr), tds, &twm, twa,
                                             kWindowTitleTextRgn, rgn);
                        pixp.setClipRegion(qt_mac_convert_mac_region(rgn));
                        qt_mac_dispose_rgn(rgn);
                    }
                    QRect br = pixp.clipRegion().boundingRect();
                    int x = br.x(),
                        y = br.y() + (tbar->rect.height() / 2 - p->fontMetrics().height() / 2);
                    if (br.width() <= p->fontMetrics().width(tbar->text) + iw * 2)
                        x += iw;
                    else
                        x += (br.width() / 2) - (p->fontMetrics().width(tbar->text) / 2);
                    if(iw)
                        pixp.drawPixmap(x - iw, y, tbar->icon);
                    pixp.drawText(x, y + p->fontMetrics().ascent(), tbar->text);
                    pixp.restore();
                    p->drawPixmap(tbar->rect.topLeft(), pix);
                    QPixmapCache::insert(pmkey, pix);
                }
            } else {
                static_cast<QMacStyleQDPainter *>(p)->setport();
                DrawThemeWindowFrame(macWinType, qt_glb_mac_rect(newr, p, false), tds,
                                     &twm, twa, NULL, 0);
            }
        }
        if (tbar->parts & (SC_TitleBarCloseButton | SC_TitleBarMaxButton | SC_TitleBarMinButton
                           | SC_TitleBarNormalButton)) {
            ThemeDrawState wtds = tds;
            if (tbar->state & Style_MouseOver)
                wtds = kThemeStateRollover;
            struct {
                unsigned int qt_type;
                ThemeTitleBarWidget mac_type;
            } types[] = {
                { SC_TitleBarCloseButton, kThemeWidgetCloseBox },
                { SC_TitleBarMaxButton, kThemeWidgetZoomBox },
                { SC_TitleBarMinButton|SC_TitleBarNormalButton, kThemeWidgetCollapseBox },
                { 0, 0 } };
            ThemeWindowMetrics tm;
            tm.metricSize = sizeof(tm);
            const Rect *wm_rect = qt_glb_mac_rect(newr, p, false);
            bool active = qAquaActive(tbar->palette);
            static_cast<QMacStyleQDPainter *>(p)->setport();
            for (int i = 0; types[i].qt_type; ++i) {
                ThemeDrawState ctrl_tds = wtds;
                if (active && (tbar->activeParts & types[i].qt_type))
                    ctrl_tds = kThemeStatePressed;
                ThemeTitleBarWidget twt = types[i].mac_type;
                /*
                if(tbar->window() && tbar->window()->isWindowModified() && twt == kThemeWidgetCloseBox)
                    twt = kThemeWidgetDirtyCloseBox;
                    */
                DrawThemeTitleBarWidget(macWinType, wm_rect, ctrl_tds, &tm, twa, twt);
            }
        }
        break; }
    default:
        QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
}

QStyle::SubControl QMacStyleQD::querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                                const QPoint &pt, const QWidget *widget) const
{
    SubControl sc = SC_None;
    switch (cc) {
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, scrollbar, 0, &tdi, widget);
            Point pos = { (short)pt.y(), (short)pt.x() };
            Rect mrect;
            GetThemeTrackBounds(&tdi, &mrect);
            ControlPartCode cpc;
            if (HitTestThemeScrollBarArrows(&tdi.bounds, tdi.enableState,
                                            0, scrollbar->orientation == Qt::Horizontal,
                                            pos, &mrect, &cpc)) {
                if (cpc == kControlUpButtonPart)
                    sc = SC_ScrollBarSubLine;
                else if (cpc == kControlDownButtonPart)
                    sc = SC_ScrollBarAddLine;
            } else if (HitTestThemeTrack(&tdi, pos, &cpc)) {
                if (cpc == kControlPageUpPart)
                    sc = SC_ScrollBarSubPage;
                else if (cpc == kControlPageDownPart)
                    sc = SC_ScrollBarAddPage;
                else
                    sc = SC_ScrollBarSlider;
            }
        }
        break;
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, 0, &tdi, widget);
            ControlPartCode hit = 0;
            Point macpt = { (short)pt.y(), (short)pt.x() };
            if (HitTestThemeTrack(&tdi, macpt, &hit) == true) {
                if (hit == kControlPageDownPart || hit == kControlPageUpPart)
                    sc = SC_SliderGroove;
                else
                    sc = SC_SliderHandle;
            }

        }
        break;
    default:
        sc = QWindowsStyle::querySubControl(cc, opt, pt, widget);
    }
    return sc;
}

QRect QMacStyleQD::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                          SubControl sc, const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case CC_Slider:
    case CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, 0, &tdi, widget);
            Rect macRect;
            GetThemeTrackBounds(&tdi, &macRect);
            tdi.bounds.left  += tdi.bounds.left  - macRect.left;
            tdi.bounds.right -= macRect.right - tdi.bounds.right;
            switch (sc) {
            case SC_SliderGroove: {
                Rect mrect;
                GetThemeTrackBounds(&tdi, &mrect);
                ret.setRect(mrect.left, mrect.top,
                            mrect.right - mrect.left, mrect.bottom - mrect.top); }
                break;
            case SC_ScrollBarGroove: {
                Rect mrect;
                GetThemeTrackDragRect(&tdi, &mrect);
                ret.setRect(mrect.left, mrect.top, mrect.right - mrect.left,
                        mrect.bottom - mrect.top);
                break; }
            case SC_ScrollBarSlider:
            case SC_SliderHandle: {
                Rect r;
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeTrackThumbRgn(&tdi, rgn);
                GetRegionBounds(rgn, &r);
                ret.setRect(r.left, r.top, (r.right - r.left) + 1, (r.bottom - r.top) + 1);
                qt_mac_dispose_rgn(rgn);
                return ret; }
            default:
                break;
            }
        }
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 10,
            spinner_h = 15;
            int fw = pixelMetric(PM_SpinBoxFrameWidth, widget),
            y = fw,
            x = spin->rect.width() - fw - spinner_w;
            switch (sc) {
                case SC_SpinBoxUp:
                    ret = QRect(x, y + ((spin->rect.height() - fw * 2) / 2 - spinner_h),
                                spinner_w, spinner_h);
                    break;
                case SC_SpinBoxDown:
                    ret.setRect(x, y + (spin->rect.height() - fw * 2) / 2, spinner_w, spinner_h);
                    break;
                case SC_SpinBoxButtonField:
                    ret.setRect(x, y, spinner_w, spin->rect.height() - fw * 2);
                    break;
                case SC_SpinBoxEditField:
                    ret.setRect(fw, fw, spin->rect.width() - spinner_w - fw * 2 - macSpinBoxSep,
                                spin->rect.height() - fw * 2);
                    break;
                case SC_SpinBoxFrame:
                    ret.setRect(0, 0, spin->rect.width() - spinner_w - macSpinBoxSep,
                                spin->rect.height());
                    break;
                default:
                    ret = QWindowsStyle::querySubControlMetrics(cc, spin, sc, widget);
                    break;
            }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            ThemeWindowMetrics twm;
            memset(&twm, 0, sizeof(twm));
            twm.metricSize = sizeof(twm);
            twm.titleWidth = tbar->rect.width();
            twm.titleHeight = tbar->rect.height();
            ThemeWindowAttributes twa = kThemeWindowHasTitleText;
            if (tbar->titleBarState)
                twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                       | kThemeWindowHasCollapseBox;
            else if (tbar->titleBarFlags & Qt::WStyle_SysMenu)
                twa |= kThemeWindowHasCloseBox;
            WindowRegionCode wrc = kWindowGlobalPortRgn;
            if (sc & SC_TitleBarCloseButton)
                wrc = kWindowCloseBoxRgn;
            else if (sc & SC_TitleBarMinButton)
                wrc = kWindowCollapseBoxRgn;
            else if (sc & SC_TitleBarMaxButton)
                wrc = kWindowZoomBoxRgn;
            else if (sc & SC_TitleBarLabel)
                wrc = kWindowTitleTextRgn;
            else if (sc & SC_TitleBarSysMenu) // We currently don't have this on Mac OS X.
                ret.setRect(-1024, -1024, 10, pixelMetric(PM_TitleBarHeight, 0));
            if (wrc != kWindowGlobalPortRgn) {
                // AppMan paints outside the given rectangle,
                // so I have to adjust for the height properly!
                Rect br;
                QRect r = tbar->rect;
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeWindowRegion(macWinType, qt_glb_mac_rect(r), kThemeStateActive, &twm, twa,
                                     kWindowTitleBarRgn, rgn);
                GetRegionBounds(rgn, &br);
                r.moveBy(r.x() - br.left, r.y() - br.top);
                GetThemeWindowRegion(macWinType, qt_glb_mac_rect(r), kThemeStateActive, &twm, twa,
                                     wrc, rgn);
                GetRegionBounds(rgn, &br);
                ret.setRect(br.left, br.top, (br.right - br.left), (br.bottom - br.top));
                qt_mac_dispose_rgn(rgn);
            }
        }
        break;
    case CC_ComboBox:
        if(const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            if (cmb->editable) {
                if (sc == SC_ComboBoxEditField)
                    ret.setRect(0, 0, cmb->rect.width() - 20, cmb->rect.height());
                else if (sc == SC_ComboBoxArrow)
                    ret.setRect(cmb->rect.width() - 24, 0, 24, cmb->rect.height());
                break;
            }
        }
        // Fall through to default!
    default:
        ret = QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
    }
    return ret;
}

QSize QMacStyleQD::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &csz,
                                   const QFontMetrics &fm, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_SpinBox:
        sz.setWidth(sz.width() + macSpinBoxSep);
        break;
    case CT_TabWidget:
        sz.setWidth(sz.width() + 15);
        break;
    case CT_TabBarTab: {
        SInt32 tabh = sz.height();
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge: {
            GetThemeMetric(kThemeLargeTabHeight, &tabh);
            SInt32 overlap;
            GetThemeMetric(kThemeMetricTabFrameOverlap, &overlap);
            tabh += overlap;
            break; }
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniTabHeight, &tabh);
                SInt32 overlap;
                GetThemeMetric(kThemeMetricMiniTabFrameOverlap, &overlap);
                tabh += overlap;
                break;
            }
#endif
        case QAquaSizeSmall: {
            GetThemeMetric(kThemeSmallTabHeight, &tabh);
            SInt32 overlap;
            GetThemeMetric(kThemeMetricSmallTabFrameOverlap, &overlap);
            tabh += overlap;
            break; }
        }
        if(sz.height() > tabh)
            sz.setHeight(tabh);
        break; }
    case CT_PushButton:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, fm, widget);
        sz = QSize(sz.width() + 16, sz.height()); // No idea why, but it was in the old style.
        break;
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            bool checkable = mi->checkState != QStyleOptionMenuItem::NotCheckable;
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(),
                h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                SInt16 ash;
                GetThemeMenuSeparatorHeight(&ash);
                h = ash;
            } else {
                h = qMax(h, fm.height() + 2);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(QIconSet::Small, QIconSet::Normal).height() + 4);
            }
            if (mi->text.contains('\t'))
                w += 12;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 20;
            if (maxpmw)
                w += maxpmw + 6;
            if (checkable)
                w += 12;
            if (widget && ::qt_cast<QComboBox*>(widget->parentWidget())
                    && widget->parentWidget()->isVisible()) {
                QStyleOptionComboBox cmb(0);
                cmb.init(widget->parentWidget());
                cmb.editable = false;
                cmb.parts = SC_ComboBoxEditField;
                cmb.activeParts = SC_None;
                w = qMax(w, querySubControlMetrics(CC_ComboBox, &cmb, SC_ComboBoxEditField,
                                                   widget->parentWidget())
                            .width());
            } else {
                w += 12;
            }
            sz = QSize(w, h);
        }
    default:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, fm, widget);
    }
    QSize macsz;
    if(qt_aqua_size_constrain(widget, ct, sz, &macsz) != QAquaSizeUnknown) {
        if(macsz.width() != -1)
            sz.setWidth(macsz.width());
        if(macsz.height() != -1)
            sz.setHeight(macsz.height());
    }
    // Adjust size to within Aqua guidelines
    if (ct == CT_PushButton || ct == CT_ToolButton) {
        ThemeButtonKind bkind = kThemePushButton;
        if (ct == CT_ToolButton)
            bkind = kThemeBevelButton;
        if (qt_aqua_size_constrain(widget) == QAquaSizeSmall) {
            if(bkind == kThemeBevelButton)
                bkind = kThemeSmallBevelButton;
        }
        ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
        Rect macRect, myRect;
        SetRect(&myRect, 0, 0, sz.width(), sz.height());
        GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
        sz.setWidth(sz.width() + (myRect.left - macRect.left) + (macRect.bottom - myRect.bottom));
        sz.setHeight(sz.height() + (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
    }
    return sz;
}
#endif
