/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmacstyle_mac.h"

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)
#define QMAC_QAQUASTYLE_SIZE_CONSTRAIN

#include <private/qpainter_p.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdockwindow.h>
#include <qevent.h>
#include <qgroupbox.h>
#include <qhash.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qpaintdevice.h>
#include <private/qpaintengine_mac_p.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qpointer.h>
#include <private/qprintengine_mac_p.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qrubberband.h>
#include <qsizegrip.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstyleoption.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtreeview.h>
#include <qtimer.h>
#include <private/qaquatabpix_mac_p.h>

extern QRegion qt_mac_convert_mac_region(RgnHandle); //qregion_mac.cpp

static inline bool isQDPainter(const QPainter *p)
{
    QPaintEngine *engine = p->paintEngine();
    if (engine->type() == QPaintEngine::MacPrinter)
        engine = static_cast<QMacPrintEngine*>(engine)->paintEngine();
    return engine && engine->type() == QPaintEngine::QuickDraw;
}

static void qt_mac_set_port(const QPainter *p)
{
    QPaintEngine *engine = p->paintEngine();
    if (engine->type() == QPaintEngine::MacPrinter)
        engine = static_cast<QMacPrintEngine*>(engine)->paintEngine();

    QQuickDrawPaintEngine *mpe = 0;
    if (engine && (engine->type() == QPaintEngine::QuickDraw
                   || engine->type() == QPaintEngine::CoreGraphics))
        mpe = static_cast<QQuickDrawPaintEngine *>(engine);
    if (mpe) {
        mpe->syncState();
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

static inline QPoint domap(const QPainter *p, QPoint pt)
{
    pt = pt * p->matrix();
    return pt;
}

// Utility to generate correct rectangles for AppManager internals
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=0,
                                          bool useOffset=true, const QRect &rect=QRect())
{
    static Rect r;
    bool use_rect = (rect.x() || rect.y() || rect.width() || rect.height());
    QPoint tl(qr.topLeft());
    if (pd && pd->devType() == QInternal::Widget) {
        QWidget *w = (QWidget*)pd;
        tl = w->mapTo(w->topLevelWidget(), tl);
    }
    if (use_rect)
        tl += rect.topLeft();
    int offset = 0;
    if (useOffset)
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
    QRect r(domap(p, pt), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

static inline ThemeTabDirection getTabDirection(QTabBar::Shape shape)
{
    ThemeTabDirection ttd;
    switch (shape) {
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        ttd = kThemeTabSouth;
        break;
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        ttd = kThemeTabNorth;
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        ttd = kThemeTabWest;
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        ttd = kThemeTabEast;
        break;
    }
    return ttd;
}


class QAquaFocusWidget : public QWidget
{
    Q_OBJECT
public:
    QAquaFocusWidget(bool noerase=true, QWidget *w=0);
    ~QAquaFocusWidget() { }
    void setFocusedWidget(QWidget * widget);
    QWidget *focusedWidget() const { return mFocusedWidget; }
    QSize sizeHint() const { return QSize(0, 0); }

protected:
    inline void mousePressEvent(QMouseEvent *ev) { ev->ignore(); }
    inline void mouseMoveEvent(QMouseEvent *ev) { ev->ignore(); }
    inline void mouseReleaseEvent(QMouseEvent *ev) { ev->ignore(); }
    bool eventFilter(QObject *o, QEvent *e);

protected:
    void paintEvent(QPaintEvent *pe);
    int focusOutset() const;
    QRegion focusRegion() const;
private:
    void drawFocusRect(QPainter *p) const;
    QPointer<QWidget> mFocusedWidget;
};

QAquaFocusWidget::QAquaFocusWidget(bool noerase, QWidget *w)
    : QWidget(w)
{
    setObjectName("magicFocusWidget");
    setFocusPolicy(Qt::NoFocus);
    if (noerase)
        setAttribute(Qt::WA_NoSystemBackground, true);
}
#if 0
/* It's a real bummer I cannot use this, but you'll notice that sometimes
   the widget will scroll "offscreen" and the focus widget will remain visible
   (which looks quite bad). --Sam */
#define FOCUS_WIDGET_PARENT(x) x->topLevelWidget()
#else
#define FOCUS_WIDGET_PARENT(x) (x->isTopLevel() ? 0 : x->parentWidget())
#endif

void QAquaFocusWidget::setFocusedWidget(QWidget *widget)
{
    hide();
    if (mFocusedWidget) {
        if (mFocusedWidget->parentWidget())
            mFocusedWidget->parentWidget()->removeEventFilter(this);
        mFocusedWidget->removeEventFilter(this);
    }
    mFocusedWidget = 0;
    if (widget && widget->parentWidget()) {
        mFocusedWidget = widget;
        setParent(FOCUS_WIDGET_PARENT(mFocusedWidget));
        move(pos());
        mFocusedWidget->installEventFilter(this);
        mFocusedWidget->parentWidget()->installEventFilter(this); //we do this so we can trap the ChildAdded event
        QPoint p(widget->mapTo(parentWidget(), QPoint(0, 0)));
        int focusWidgetWidth = widget->width();
        if (qt_cast<QLineEdit *>(widget) && qt_cast<QComboBox *>(widget->parentWidget()))
            focusWidgetWidth += 16 ; // This is a cheat/optimization, really should query for it.
        setGeometry(p.x() - focusOutset(), p.y() - focusOutset(),
                    focusWidgetWidth + (focusOutset() * 2), widget->height() + (focusOutset() * 2));
        setPalette(widget->palette());
        setMask(QRegion(rect()) - focusRegion());
        stackUnder(mFocusedWidget);
        show();
    }
}

bool QAquaFocusWidget::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::ChildAdded || e->type() == QEvent::ChildRemoved)
        && ((QChildEvent*)e)->child() == this) {
        if (e->type() == QEvent::ChildRemoved)
            o->removeEventFilter(this); //once we're removed, stop listening
        return true; //block child events
    } else if (o == mFocusedWidget) {
        switch (e->type()) {
        case QEvent::PaletteChange:
            setPalette(mFocusedWidget->palette());
            break;
        case QEvent::Hide:
            hide();
            break;
        case QEvent::Show:
            show();
            break;
        case QEvent::Move: {
            QPoint p(mFocusedWidget->mapTo(parentWidget(), QPoint(0, 0)));
            move(p.x() - focusOutset(), p.y() - focusOutset());
            break;
        }
        case QEvent::Resize: {
            QResizeEvent *re = (QResizeEvent*)e;
            resize(re->size().width() + (focusOutset() * 2),
                    re->size().height() + (focusOutset() * 2));
            setMask(QRegion(rect()) - focusRegion());
            break;
        }
        case QEvent::ParentChange: {
            QWidget *newp = FOCUS_WIDGET_PARENT(mFocusedWidget);
            QPoint p(mFocusedWidget->mapTo(newp, QPoint(0, 0)));
            newp->installEventFilter(this);
            setParent(newp);
            show();
            move(p);
            raise();
            break; }
        default:
            break;
        }
    }
    return false;
}

int QAquaFocusWidget::focusOutset() const
{
    SInt32 ret = 0;
    GetThemeMetric(kThemeMetricFocusRectOutset, &ret);
    return ret;
}

QRegion QAquaFocusWidget::focusRegion() const
{
    const QRgb fillColor = qRgb(192, 191, 190);
    QImage img;
    {
        QPixmap pix(size(), 32);
        pix.fill(fillColor);
        QPainter p(&pix);
        drawFocusRect(&p);
        img = pix.toImage();
    }
    QImage mask(img.width(), img.height(), 1, 2, QImage::LittleEndian);
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
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

void QAquaFocusWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawFocusRect(&p);
}

void QAquaFocusWidget::drawFocusRect(QPainter *p) const
{
    int fo = focusOutset();
    if (isQDPainter(p) || QSysInfo::MacintoshVersion < QSysInfo::MV_10_3) {
        qt_mac_set_port(p);
        QRect r(fo, fo,  width() - (fo*2), height() - (fo*2));
        DrawThemeFocusRect(qt_glb_mac_rect(r, p, true, QRect(1, 1, 1, 1)), true);
    }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    else {
        HIRect rect = CGRectMake(fo, fo, width() - 2 * fo, height() - 2 * fo);
        HIThemeDrawFocusRect(&rect, true, QMacCGContext(p), kHIThemeOrientationNormal);
    }
#endif
}

class QMacStylePrivate : public QObject
{
    Q_OBJECT
public:
    QMacStylePrivate(QMacStyle *style);

    // Stuff from QAquaAnimate:
    bool addWidget(QWidget *);
    void removeWidget(QWidget *);

    enum Animates { AquaPushButton, AquaProgressBar, AquaListViewItemOpen };
    bool animatable(Animates, const QWidget *) const;
    void stopAnimate(Animates, QWidget *);
    void startAnimate(Animates, QWidget *);
    static ThemeDrawState getDrawState(QStyle::StyleFlags flags);

    bool focusable(const QWidget *) const;

    bool doAnimate(Animates);
    inline int animateSpeed(Animates) const { return 33; }
    void focusOnWidget(QWidget *);
    void doFocus(QWidget *w);

    struct PolicyState {
        static QMap<const QWidget*, QMacStyle::FocusRectPolicy> focusMap;
        static QMap<const QWidget*, QMacStyle::WidgetSizePolicy> sizeMap;
        static void watchObject(const QObject *o);
    };

    // HITheme-based functions
    void HIThemePolish(QWidget *w);
    void HIThemeUnPolish(QWidget *w);
    void HIThemePolish(QApplication *app);
    void HIThemeDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                              const QWidget *w = 0) const;
    void HIThemeDrawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p,
                            const QWidget *w = 0) const;
    QRect HIThemeSubRect(QStyle::SubRect r, const QStyleOption *opt, const QWidget *widget = 0) const;
    void HIThemeDrawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                   QPainter *p, const QWidget *w = 0) const;
    QStyle::SubControl HIThemeQuerySubControl(QStyle::ComplexControl cc,
                                              const QStyleOptionComplex *opt,
                                              const QPoint &pt, const QWidget *w = 0) const;
    QRect HIThemeQuerySubControlMetrics(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                        QStyle::SubControl sc, const QWidget *w = 0) const;
    void HIThemeAdjustButtonSize(QStyle::ContentsType ct, QSize &sz, const QWidget *w);
    int HIThemePixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                           const QWidget *widget) const;

    // Appearance Manager-based functions
    void AppManPolish(QWidget *w);
    void AppManUnPolish(QWidget *w);
    void AppManPolish(QApplication *app);

    void AppManDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                             const QWidget *w = 0) const;
    void AppManDrawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p,
                           const QWidget *w = 0) const;
    QRect AppManSubRect(QStyle::SubRect r, const QStyleOption *opt, const QWidget *widget = 0) const;
    void AppManDrawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                  QPainter *p, const QWidget *w = 0) const;
    QStyle::SubControl AppManQuerySubControl(QStyle::ComplexControl cc,
                                             const QStyleOptionComplex *opt, const QPoint &pt,
                                             const QWidget *w = 0) const;
    QRect AppManQuerySubControlMetrics(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                       QStyle::SubControl sc, const QWidget *w = 0) const;
    void AppManAdjustButtonSize(QStyle::ContentsType ct, QSize &sz, const QWidget *w);
    int AppManPixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                          const QWidget *widget) const;
    void drawPantherTab(const QStyleOptionTab *tab, QPainter *p, const QWidget *w = 0) const;

protected:
    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent *);

private slots:
    void objDestroyed(QObject *o);
    void startAnimationTimer();

public:
    bool useHITheme;
    QPointer<QWidget> animationFocusWidget; //the focus widget
    QPointer<QPushButton> defaultButton; //default pushbuttons
    int timerID;
    QList<QPointer<QWidget> > progressBars; //It's really only progress bar information

    struct ButtonState {
        int frame;
        enum { ButtonDark, ButtonLight } dir;
    } buttonState;
    UInt8 progressFrame;
    CFAbsoluteTime defaultButtonStart;
    QPointer<QAquaFocusWidget> aquaFocus;
    QMacStyle *q;
};

QPixmap qt_mac_convert_iconref(IconRef icon, int width, int height);
QMap<const QWidget*, QMacStyle::FocusRectPolicy> QMacStylePrivate::PolicyState::focusMap;
QMap<const QWidget*, QMacStyle::WidgetSizePolicy> QMacStylePrivate::PolicyState::sizeMap;

class QMacStylePrivateObjectWatcher : public QObject
{
    Q_OBJECT
public:
    QMacStylePrivateObjectWatcher(QObject *p) : QObject(p) {}
public slots:
    void destroyedObject(QObject *o);
};

#include "qmacstyle_mac.moc"

/*****************************************************************************
  External functions
 *****************************************************************************/
extern CGContextRef qt_macCreateCGHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern QPixmap qt_mac_convert_iconref(IconRef, int, int); //qpixmap_mac.cpp
extern QRegion qt_mac_convert_mac_region(HIShapeRef); //qregion_mac.cpp
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp

/*****************************************************************************
  QMacCGStyle globals
 *****************************************************************************/
const int qt_mac_hitheme_version = 0; //the HITheme version we speak
const int macSpinBoxSep        = 5;    // distance between spinwidget and the lineedit
const int macItemFrame         = 2;    // menu item frame width
const int macItemHMargin       = 3;    // menu item hor text margin
const int macItemVMargin       = 2;    // menu item ver text margin
const int macRightBorder       = 12;   // right border on mac
const ThemeWindowType QtWinType = kThemeUtilityWindow; // Window type we use for QTitleBar.

/*****************************************************************************
  QMacCGStyle utility functions
 *****************************************************************************/
static inline int qt_mac_hitheme_tab_version()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
        return 1;
#endif
    return 0;
}

static inline HIRect qt_hirectForQRect(const QRect &convertRect, QPainter *p = 0,
                                       bool useOffset = true, const QRect &rect = QRect())
{
    int x, y;
    int offset = 0;
    if (useOffset)
        offset = 1;
    if (p) {
        QPoint pt = domap(p, convertRect.topLeft());
        x = pt.x();
        y = pt.y();
    } else {
        x = convertRect.x();
        y = convertRect.y();
    }
    HIRect retRect = CGRectMake(x + rect.x(), y + rect.y(),
                                convertRect.width() - offset - rect.width(),
                                convertRect.height() - offset - rect.height());
    return retRect;
}

static inline const QRect qt_qrectForHIRect(const HIRect &hirect)
{
    return QRect(QPoint(int(hirect.origin.x), int(hirect.origin.y)),
                 QSize(int(hirect.size.width), int(hirect.size.height)));
}

static inline bool qt_mac_is_metal(const QWidget *w)
{
    for (; w; w = w->parentWidget()) {
        if (w->testAttribute(Qt::WA_MacMetalStyle))
            return true;
        if (w->isTopLevel())
            break;
    }
    return false;
}

static int qt_mac_aqua_get_metric(ThemeMetric met)
{
    SInt32 ret;
    GetThemeMetric(met, &ret);
    return ret;
}

enum QAquaWidgetSize { QAquaSizeLarge, QAquaSizeSmall, QAquaSizeMini, QAquaSizeUnknown };

static QSize qt_aqua_get_known_size(QStyle::ContentsType ct, const QWidget *widg, QSize szHint,
                                    QAquaWidgetSize sz)
{
    QSize ret(-1, -1);
    if(QSysInfo::MacintoshVersion < QSysInfo::MV_10_3 && sz == QAquaSizeMini)
        return ret;

    if(sz != QAquaSizeSmall && sz != QAquaSizeLarge && sz != QAquaSizeMini) {
        qDebug("Not sure how to return this...");
        return ret;
    }
    if(widg && widg->testAttribute(Qt::WA_SetFont)) //if you're using a custom font, no constraints for you!
        return ret;

    if(ct == QStyle::CT_CustomBase && widg) {
        if(qt_cast<const QPushButton *>(widg))
            ct = QStyle::CT_PushButton;
        else if(qt_cast<const QRadioButton *>(widg))
            ct = QStyle::CT_RadioButton;
        else if(qt_cast<const QCheckBox *>(widg))
            ct = QStyle::CT_CheckBox;
        else if(qt_cast<const QComboBox *>(widg))
            ct = QStyle::CT_ComboBox;
        else if(qt_cast<const QComboBox *>(widg))
            ct = QStyle::CT_ToolButton;
        else if(qt_cast<const QSlider *>(widg))
            ct = QStyle::CT_Slider;
        else if(qt_cast<const QProgressBar *>(widg))
            ct = QStyle::CT_ProgressBar;
        else if(qt_cast<const QLineEdit *>(widg))
            ct = QStyle::CT_LineEdit;
#ifdef QT_COMPAT
        else if(widg->inherits("QHeader"))
            ct = QStyle::CT_Header;
#endif
        else if(qt_cast<const QMenuBar *>(widg)
#ifdef QT_COMPAT
		|| widg->inherits("Q3MenuBar")
#endif
	       )
            ct = QStyle::CT_MenuBar;
        else if(qt_cast<const QSizeGrip *>(widg))
            ct = QStyle::CT_SizeGrip;
        else
            return ret;
    }

    if(ct == QStyle::CT_PushButton) {
        const QPushButton *psh = static_cast<const QPushButton *>(widg);
        int minw = -1;
        // Aqua Style guidelines restrict the size of OK and Cancel buttons to 68 pixels.
        // However, this doesn't work for German, therefore only do it for English,
        // I suppose it would be better to do some sort of lookups for languages
        // that like to have really long words.
        if(psh->text() == "OK" || psh->text() == "Cancel")
            minw = 69;
        if (psh->text().contains('\n'))
            ret = QSize(minw, -1);
        else if(sz == QAquaSizeLarge)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricPushButtonHeight));
        else if(sz == QAquaSizeSmall)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricSmallPushButtonHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricMiniPushButtonHeight));
#endif
#if 0 //Not sure we are applying the rules correctly for RadioButtons/CheckBoxes --Sam
    } else if(ct == QStyle::CT_RadioButton) {
        QRadioButton *rdo = static_cast<QRadioButton *>(widg);
        // Exception for case where multiline radiobutton text requires no size constrainment
        if(rdo->text().find('\n') != -1)
            return ret;
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricRadioButtonHeight));
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallRadioButtonHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniRadioButtonHeight));
#endif
    } else if(ct == QStyle::CT_CheckBox) {
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricCheckBoxHeight));
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallCheckBoxHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniCheckBoxHeight));
#endif
#endif
    } else if(ct == QStyle::CT_SizeGrip) {
        if(sz == QAquaSizeLarge || sz == QAquaSizeSmall) {
            Rect r;
            Point p = { 0, 0 };
            ThemeGrowDirection dir = kThemeGrowRight | kThemeGrowDown;
            if(QApplication::isRightToLeft())
                dir = kThemeGrowLeft | kThemeGrowDown;
            if(GetThemeStandaloneGrowBoxBounds(p, dir, sz != QAquaSizeSmall, &r) == noErr)
                ret = QSize(r.right - r.left, r.bottom - r.top);
        }
    } else if(ct == QStyle::CT_ComboBox) {
        const QComboBox *cmb = ::qt_cast<const QComboBox *>(widg);
        if (sz == QAquaSizeLarge ||
            (sz != QAquaSizeLarge && cmb && cmb->isEditable()
             && QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)) {
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricPopupButtonHeight) + 1);
        } else if (sz == QAquaSizeSmall) {
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallPopupButtonHeight) + 1);
        }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if (sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniPopupButtonHeight));
#endif

    } else if(ct == QStyle::CT_ToolButton && sz == QAquaSizeSmall) {
        int width = 0, height = 0;
        if(szHint == QSize(-1, -1)) { //just 'guess'..
            const QToolButton *bt = static_cast<const QToolButton *>(widg);
            if(!bt->icon().isNull()) {
                Qt::IconSize sz = Qt::SmallIconSize;
                if(bt->iconSize() == Qt::LargeIconSize)
                    sz = Qt::LargeIconSize;
                QSize iconSize = QIcon::pixmapSize(sz);
                QPixmap pm = bt->icon().pixmap(sz, QIcon::Normal);
                width = qMax(width, qMax(iconSize.width(), pm.width()));
                height = qMax(height, qMax(iconSize.height(), pm.height()));
            }
            if(!bt->text().isNull() && bt->toolButtonStyle() != Qt::ToolButtonIconOnly) {
                int text_width = bt->fontMetrics().width(bt->text()),
                   text_height = bt->fontMetrics().height();
                if(bt->toolButtonStyle() == Qt::ToolButtonTextUnderIcon) {
                    width = qMax(width, text_width);
                    height += text_height;
                } else {
                    width += text_width;
                    width = qMax(height, text_height);
                }
            }
        } else {
            width = szHint.width();
            height = szHint.height();
        }
        width =  qMax(20, width +  5); //border
        height = qMax(20, height + 5); //border
        ret = QSize(width, height);
    } else if(ct == QStyle::CT_Slider) {
        int w = -1;
        const QSlider *sld = static_cast<const QSlider *>(widg);
        if(sz == QAquaSizeLarge) {
            if(sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricHSliderHeight);
                if(sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricVSliderWidth);
                if(sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricVSliderTickWidth);
            }
        } else if(sz == QAquaSizeSmall) {
            if(sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricSmallHSliderHeight);
                if(sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricSmallHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricSmallVSliderWidth);
                if(sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricSmallVSliderTickWidth);
            }
        }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini) {
            if(sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricMiniHSliderHeight);
                if(sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricMiniHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricMiniVSliderWidth);
                if(sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricMiniVSliderTickWidth);
            }
        }
#endif
        if(sld->orientation() == Qt::Horizontal)
            ret.setHeight(w);
        else
            ret.setWidth(w);
    } else if(ct == QStyle::CT_ProgressBar) {
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricLargeProgressBarThickness));
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricNormalProgressBarThickness));
    } else if(ct == QStyle::CT_LineEdit) {
        if(!widg || !widg->parentWidget() || !widg->parentWidget()->inherits("QComboBox")) {
            //should I take into account the font dimentions of the lineedit? -Sam
            if(sz == QAquaSizeLarge)
                ret = QSize(-1, 22);
            else
                ret = QSize(-1, 19);
        }
    } else if(ct == QStyle::CT_Header) {
	if(sz == QAquaSizeLarge && (widg && (qt_cast<const QTreeView *>(widg)
#ifdef QT_COMPAT
			|| widg->parentWidget()->inherits("Q3ListView")
#endif
			)))
           ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricListHeaderHeight));
    } else if(ct == QStyle::CT_MenuBar) {
        if(sz == QAquaSizeLarge) {
            SInt16 size;
            if(!GetThemeMenuBarHeight(&size))
                ret = QSize(-1, size);
        }
    }
    return ret;
}


#if defined(QMAC_QAQUASTYLE_SIZE_CONSTRAIN) || defined(DEBUG_SIZE_CONSTRAINT)
static QAquaWidgetSize qt_aqua_guess_size(const QWidget *widg, QSize large, QSize small, QSize mini)
{
    if(large == QSize(-1, -1)) {
        if(small != QSize(-1, -1))
            return QAquaSizeSmall;
        if(mini != QSize(-1, -1))
            return QAquaSizeMini;
        return QAquaSizeUnknown;
    } else if(small == QSize(-1, -1)) {
        if(mini != QSize(-1, -1))
            return QAquaSizeMini;
        return QAquaSizeLarge;
    } else if(mini == QSize(-1, -1)) {
        return QAquaSizeLarge;
    }

#ifndef QT_NO_MAINWINDOW
    if(qt_cast<QDockWindow *>(widg->topLevelWidget()) || qgetenv("QWIDGET_ALL_SMALL")) {
        //if(small.width() != -1 || small.height() != -1)
        return QAquaSizeSmall;
    } else if(qgetenv("QWIDGET_ALL_MINI")) {
        return QAquaSizeMini;
    }
#endif

#if 0
    /* Figure out which size we're closer to, I just hacked this in, I haven't
       tested it as it would probably look pretty strange to have some widgets
       big and some widgets small in the same window?? -Sam */
    int large_delta=0;
    if(large.width() != -1) {
        int delta = large.width() - widg->width();
        large_delta += delta * delta;
    }
    if(large.height() != -1) {
        int delta = large.height() - widg->height();
        large_delta += delta * delta;
    }
    int small_delta=0;
    if(small.width() != -1) {
        int delta = small.width() - widg->width();
        small_delta += delta * delta;
    }
    if(small.height() != -1) {
        int delta = small.height() - widg->height();
        small_delta += delta * delta;
    }
    int mini_delta=0;
    if(mini.width() != -1) {
        int delta = mini.width() - widg->width();
        mini_delta += delta * delta;
    }
    if(mini.height() != -1) {
        int delta = mini.height() - widg->height();
        mini_delta += delta * delta;
    }
    if(mini_delta < small_delta && mini_delta < large_delta)
        return QAquaSizeMini;
    else if(small_delta < large_delta)
        return QAquaSizeSmall;
#endif
    return QAquaSizeLarge;
}
#endif

QAquaWidgetSize qt_aqua_size_constrain(const QWidget *widg,
                                       QStyle::ContentsType ct = QStyle::CT_CustomBase,
                                       QSize szHint=QSize(-1, -1), QSize *insz = 0)
{
#if defined(QMAC_QAQUASTYLE_SIZE_CONSTRAIN) || defined(DEBUG_SIZE_CONSTRAINT)
    if(!widg) {
        if(insz)
            *insz = QSize();
        if(qgetenv("QWIDGET_ALL_SMALL"))
            return QAquaSizeSmall;
        if(qgetenv("QWIDGET_ALL_MINI"))
            return QAquaSizeMini;
        return QAquaSizeUnknown;
    }
    QSize large = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeLarge),
          small = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeSmall),
          mini  = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeMini);
    bool guess_size = false;
    QAquaWidgetSize ret = QAquaSizeUnknown;
    QMacStyle *macStyle = qt_cast<QMacStyle *>(widg->style());
    if(macStyle) {
        QMacStyle::WidgetSizePolicy wsp = macStyle->widgetSizePolicy(widg);
        if(wsp == QMacStyle::SizeDefault)
            guess_size = true;
        else if(wsp == QMacStyle::SizeMini)
            ret = QAquaSizeMini;
        else if(wsp == QMacStyle::SizeSmall)
            ret = QAquaSizeSmall;
        else if(wsp == QMacStyle::SizeLarge)
            ret = QAquaSizeLarge;
    }
    if(guess_size)
        ret = qt_aqua_guess_size(widg, large, small, mini);

    QSize *sz = 0;
    if(ret == QAquaSizeSmall)
        sz = &small;
    else if(ret == QAquaSizeLarge)
        sz = &large;
    else if(ret == QAquaSizeMini)
        sz = &mini;
    if(insz)
        *insz = sz ? *sz : QSize(-1, -1);
#ifdef DEBUG_SIZE_CONSTRAINT
    if(sz) {
        const char *size_desc = "Unknown";
        if(sz == &small)
            size_desc = "Small";
        else if(sz == &large)
            size_desc = "Large";
        else if(sz == &mini)
            size_desc = "Mini";
        qDebug("%s - %s: %s taken (%d, %d) [%d, %d]", widg ? widg->name() : "*Unknown*",
               widg ? widg->className() : "*Unknown*", size_desc, widg->width(), widg->height(),
               sz->width(), sz->height());
    }
#endif
    return ret;
#else
    if(insz)
        *insz = QSize();
    Q_UNUSED(widg);
    Q_UNUSED(ct);
    Q_UNUSED(szHint);
    return QAquaSizeUnknown;
#endif
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if(p && p->device()->devType() == QInternal::Widget)
        return qt_aqua_size_constrain(static_cast<QWidget *>(p->device()));
    return qt_aqua_size_constrain(0);
}

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
static void getSliderInfo(QStyle::ComplexControl cc, const QStyleOptionSlider *slider,
                          HIThemeTrackDrawInfo *tdi, const QWidget *needToRemoveMe)
{
    memset(tdi, 0, sizeof(HIThemeTrackDrawInfo)); // We don't get it all for some reason or another...
    tdi->version = qt_mac_hitheme_version;
    tdi->reserved = 0;
    bool isScrollbar = (cc == QStyle::CC_ScrollBar);
    switch (qt_aqua_size_constrain(needToRemoveMe)) {
    case QAquaSizeUnknown:
    case QAquaSizeLarge:
        if (isScrollbar)
            tdi->kind = kThemeMediumScrollBar;
        else
            tdi->kind = kThemeMediumSlider;
        break;
    case QAquaSizeMini:
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            if (isScrollbar)
                tdi->kind = kThemeMiniScrollBar;
            else
                tdi->kind = kThemeMiniSlider;
            break;
        }
    case QAquaSizeSmall:
        if (isScrollbar)
            tdi->kind = kThemeSmallScrollBar;
        else
            tdi->kind = kThemeSmallSlider;
        break;
    }
    tdi->bounds = qt_hirectForQRect(slider->rect);
    tdi->min = slider->minimum;
    tdi->max = slider->maximum;
    tdi->value = slider->sliderPosition;
    tdi->attributes = kThemeTrackShowThumb;
    if (slider->orientation == Qt::Horizontal)
        tdi->attributes |= kThemeTrackHorizontal;
    if (slider->upsideDown)
        tdi->attributes |= kThemeTrackRightToLeft;
    tdi->enableState = slider->state & QStyle::Style_Enabled ? kThemeTrackActive
                                                             : kThemeTrackDisabled;
    if (!(slider->state & QStyle::Style_Active))
        tdi->enableState = kThemeTrackInactive;
    if (!isScrollbar) {
        if (slider->state & QStyle::QStyle::Style_HasFocus)
            tdi->attributes |= kThemeTrackHasFocus;
        if (slider->tickPosition == QSlider::NoTicks || slider->tickPosition == QSlider::TicksBothSides)
            tdi->trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickPosition == QSlider::TicksAbove)
            tdi->trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi->trackInfo.slider.thumbDir = kThemeThumbDownward;
    } else {
        tdi->trackInfo.scrollbar.viewsize = slider->pageStep;
    }
}
#endif

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
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
    if (slider->upsideDown)
        tdi->attributes |= kThemeTrackRightToLeft;
    tdi->enableState = slider->state & QStyle::Style_Enabled ? kThemeTrackActive
                                                             : kThemeTrackDisabled;
    if (!(slider->state & QStyle::Style_Active))
        tdi->enableState = kThemeTrackDisabled;
    if (!isScrollbar) {
        if (slider->tickPosition == QSlider::NoTicks || slider->tickPosition == QSlider::TicksBothSides)
            tdi->trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickPosition == QSlider::TicksAbove)
            tdi->trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi->trackInfo.slider.thumbDir = kThemeThumbDownward;
    } else {
        tdi->trackInfo.scrollbar.viewsize = slider->pageStep;
    }
}

QMacStylePrivate::QMacStylePrivate(QMacStyle *style)
    : useHITheme(false), timerID(-1), progressFrame(0), aquaFocus(0), q(style)
{
#if !defined(QMAC_NO_COREGRAPHICS) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3 && !qgetenv("QT_MAC_USE_APPMANAGER")
	    && !qgetenv("QT_MAC_USE_QUICKDRAW"))
        useHITheme = true;
#endif
    defaultButtonStart = CFAbsoluteTimeGetCurrent();
    memset(&buttonState, 0, sizeof(ButtonState));
}

bool QMacStylePrivate::animatable(QMacStylePrivate::Animates as, const QWidget *w) const
{
    if (as == AquaPushButton) {
        if (static_cast<const QPushButton *>(w) == defaultButton) {
            return true;
        }
    } else if (as == AquaProgressBar) {
        if (progressBars.contains((const_cast<QWidget *>(w))))
            return true;
    }
    return false;
}

void QMacStylePrivate::stopAnimate(QMacStylePrivate::Animates as, QWidget *w)
{
    if (as == AquaPushButton && defaultButton) {
        QPushButton *tmp = defaultButton;
        defaultButton = 0;
        if (tmp->isVisible())
            tmp->update();
    } else if (as == AquaProgressBar) {
        progressBars.removeAll(w);
    }
}

void QMacStylePrivate::startAnimate(QMacStylePrivate::Animates as, QWidget *w)
{
    if (as == AquaPushButton)
        defaultButton = static_cast<QPushButton *>(w);
    else if (as == AquaProgressBar)
        progressBars.append(w);
    startAnimationTimer();
}

void QMacStylePrivate::startAnimationTimer()
{
    if ((defaultButton || !progressBars.isEmpty()) && timerID <= -1)
        timerID = startTimer(animateSpeed(AquaListViewItemOpen));
}

bool QMacStylePrivate::focusable(const QWidget *w) const
{
    QMacStyle::FocusRectPolicy fp = q->focusRectPolicy(w);
    if (fp == QMacStyle::FocusEnabled)
        return true;
    else if (fp == QMacStyle::FocusDisabled)
        return false;
    return (w && !w->isTopLevel() && w->parentWidget() &&
            (qt_cast<const QAbstractSpinBox *>(w) || qt_cast<const QComboBox *>(w)
             || qt_cast<const QLineEdit *>(w))
             || (qt_cast<const QTextEdit *>(w) && static_cast<const QTextEdit *>(w)->isReadOnly())
#ifdef QT_COMPAT
             || w->inherits("QListBox") || w->inherits("QListView")
#endif
           );
}

void QMacStylePrivate::focusOnWidget(QWidget *w)
{
    if (w) {
        QWidget *top = w->parentWidget();
        while (!top->isTopLevel() && !top->testWFlags(Qt::WSubWindow))
            top = top->parentWidget();
#ifndef QT_NO_MAINWINDOW
        if (qt_cast<QMainWindow *>(top)) {
            QWidget *central = static_cast<QMainWindow *>(top)->centralWidget();
            for (const QWidget *par = w; par; par = par->parentWidget()) {
                if (par == central) {
                    top = central;
                    break;
                }
                if (par->isTopLevel())
                    break;
            }
        }
#endif
        if (top && (w->width() < top->width() - 30 || w->height() < top->height() - 40)) {
            if (QComboBox *cmb = qt_cast<QComboBox *>(w)) {
                if (cmb->isEditable())
                    w = cmb->lineEdit();
            }
        } else {
            w = 0;
        }
    }
    if (w == animationFocusWidget)
        return;
    doFocus(const_cast<QWidget*>(w));
    if (animationFocusWidget)
        QObject::disconnect(animationFocusWidget, SIGNAL(destroyed(QObject*)),
                            this, SLOT(objDestroyed(QObject*)));
    animationFocusWidget = w;
    if (animationFocusWidget)
	QObject::connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
}

void QMacStylePrivate::objDestroyed(QObject *o)
{
    if (o == animationFocusWidget)
        focusOnWidget(0);
}

enum { TabNormalLeft, TabNormalMid, TabNormalRight, TabSelectedActiveLeft,
       TabSelectedActiveMid, TabSelectedActiveRight, TabSelectedInactiveLeft,
       TabSelectedInactiveMid, TabSelectedInactiveRight };

static const char * const * const PantherTabXpms[] = {
                                    qt_mac_tabnrm_left,
                                    qt_mac_tabnrm_mid,
                                    qt_mac_tabnrm_right,
                                    qt_mac_tabselected_active_left,
                                    qt_mac_tabselected_active_mid,
                                    qt_mac_tabselected_active_right,
                                    qt_mac_tabselected_inactive_left,
                                    qt_mac_tabselected_inactive_mid,
                                    qt_mac_tabselected_inactive_right };

void QMacStylePrivate::drawPantherTab(const QStyleOptionTab *tabOpt, QPainter *p,
                                      const QWidget *) const
{
    QString tabKey = QLatin1String("$qt_mac_style_tab_");
    int pantherTabStart;
    int pantherTabMid;
    int pantherTabEnd;

    if (!(tabOpt->state & QStyle::Style_Selected))
        pantherTabStart = TabNormalLeft;
    else if (!(tabOpt->state & QStyle::Style_Active))
        pantherTabStart = TabSelectedInactiveLeft;
    else
        pantherTabStart = TabSelectedActiveLeft;

    bool doLine;
    bool verticalTabs = tabOpt->shape == QTabBar::RoundedEast
        || tabOpt->shape == QTabBar::RoundedWest
        || tabOpt->shape == QTabBar::TriangularEast
        || tabOpt->shape == QTabBar::TriangularWest;

    QStyleOptionTab::TabPosition tp = tabOpt->position;
    bool westSide = tabOpt->shape == QTabBar::RoundedWest
                                        || tabOpt->shape == QTabBar::TriangularWest;
    if (westSide) {
        if (tp == QStyleOptionTab::Beginning)
            tp = QStyleOptionTab::End;
        else if (tp == QStyleOptionTab::End)
            tp = QStyleOptionTab::Beginning;
    }

    switch (tp) {
    case QStyleOptionTab::Beginning:
        doLine = false;
        pantherTabMid = pantherTabEnd = pantherTabStart + 1;
        break;
    case QStyleOptionTab::Middle:
        doLine = true;
        pantherTabMid = pantherTabEnd = ++pantherTabStart;
        break;
    case QStyleOptionTab::End:
        doLine = true;
        pantherTabMid = ++pantherTabStart;
        pantherTabEnd = pantherTabMid + 1;
        break;
    case QStyleOptionTab::OnlyOneTab:
        doLine = false;
        pantherTabMid = pantherTabStart + 1;
        pantherTabEnd = pantherTabMid + 1;
        break;
    }
    QPixmap pmStart;
    if (!QPixmapCache::find(tabKey + QString::number(pantherTabStart), pmStart)) {
        pmStart = QPixmap(PantherTabXpms[pantherTabStart]);
        QPixmapCache::insert(tabKey + QString::number(pantherTabStart), pmStart);
    }

    QPixmap pmMid;
    if (!QPixmapCache::find(tabKey + QString::number(pantherTabMid), pmMid)) {
        pmMid = QPixmap(PantherTabXpms[pantherTabMid]);
        QPixmapCache::insert(tabKey + QString::number(pantherTabMid), pmMid);
    }

    QPixmap pmEnd;
    if (!QPixmapCache::find(tabKey + QString::number(pantherTabEnd), pmEnd)) {
        pmEnd = QPixmap(PantherTabXpms[pantherTabEnd]);
        QPixmapCache::insert(tabKey + QString::number(pantherTabEnd), pmEnd);
    }
    QRect tr = tabOpt->rect;
    if (verticalTabs) {
        p->save();
        int newX, newY, newRot;
        if (tabOpt->shape == QTabBar::RoundedEast || tabOpt->shape == QTabBar::TriangularEast) {
            newX = tr.width();
            newY = tr.y();
            newRot = 90;
        } else {
            newX = 0;
            newY = tr.y() + tr.height();
            newRot = -90;
        }
        tr.setRect(0, 0, tr.height(), tr.width());
        QMatrix m;
        m.translate(newX, newY);
        m.rotate(newRot);
        p->setMatrix(m, true);
    }

    int x = tr.x();
    int y = tr.y();
    int endX = x + tr.width() - pmEnd.width();

    p->drawPixmap(x, y, pmStart.width(), tr.height(), pmStart);
    if (doLine) {
        QPen oldPen = p->pen();
        p->setPen(QColor(0, 0, 0, 0x35));
        p->drawLine(x, y + 1, x, tr.height() - 2);
    }

    for (x = x + pmStart.width(); x < endX; x += pmMid.width())
        p->drawPixmap(x, y, pmMid.width(), tr.height(), pmMid);
    p->drawPixmap(endX, y, pmEnd.width(), tr.height(), pmEnd);
    if (verticalTabs)
        p->restore();
}

bool QMacStylePrivate::addWidget(QWidget *w)
{
    if (focusable(w)) {
        if (w->hasFocus())
            focusOnWidget(w);
        w->installEventFilter(this);
    }
    //already knew of it
    if (static_cast<QPushButton*>(w) == defaultButton
            || progressBars.contains(static_cast<QProgressBar*>(w)))
        return false;

    if (QPushButton *btn = qt_cast<QPushButton *>(w)) {
        btn->installEventFilter(this);
        if (btn->isDefault() || (btn->autoDefault() && btn->hasFocus()))
            startAnimate(AquaPushButton, btn);
        return true;
    } else {
        bool isProgressBar = (qt_cast<QProgressBar *>(w)
#ifdef QT_COMPAT
                || w->inherits("Q3ProgressBar")
#endif
            );
        if (isProgressBar) {
            w->installEventFilter(this);
            startAnimate(AquaProgressBar, w);
            return true;
        }
    }
    return false;
}

void QMacStylePrivate::removeWidget(QWidget *w)
{
    if (animationFocusWidget == w)
        focusOnWidget(0);
    QPushButton *btn = qt_cast<QPushButton *>(w);
    if (btn && btn == defaultButton) {
        stopAnimate(AquaPushButton, btn);
    } else if (qt_cast<QProgressBar *>(w)
#ifdef QT_COMPAT
            || w->inherits("Q3ProgressBar")
#endif
            ) {
        stopAnimate(AquaProgressBar, w);
    }
}

ThemeDrawState QMacStylePrivate::getDrawState(QStyle::StyleFlags flags)
{
    ThemeDrawState tds = kThemeStateActive;
    if (flags & QStyle::Style_Down) {
        tds = kThemeStatePressed;
    } else if (flags & QStyle::Style_Active) {
        if (!(flags & QStyle::Style_Enabled))
            tds = kThemeStateUnavailable;
    } else {
        if (flags & QStyle::Style_Enabled)
            tds = kThemeStateInactive;
        else
            tds = kThemeStateUnavailableInactive;
    }
    return tds;
}

void QMacStylePrivate::PolicyState::watchObject(const QObject *o)
{
    static QPointer<QMacStylePrivateObjectWatcher> watcher;
    if (!watcher)
        watcher = new QMacStylePrivateObjectWatcher(0);
    QObject::connect(o, SIGNAL(destroyed(QObject*)), watcher, SLOT(destroyedObject(QObject*)));
}

void QMacStylePrivateObjectWatcher::destroyedObject(QObject *o)
{
    QMacStylePrivate::PolicyState::focusMap.remove(static_cast<QWidget *>(o));
    QMacStylePrivate::PolicyState::sizeMap.remove(static_cast<QWidget *>(o));
}

void QMacStylePrivate::timerEvent(QTimerEvent *)
{
    int animated = 0;
    if (defaultButton && defaultButton->isEnabled() && defaultButton->isVisibleTo(0)
       && (defaultButton->isDefault()
           || (defaultButton->autoDefault() && defaultButton->hasFocus()))
       && doAnimate(AquaPushButton)) {
        ++animated;
        defaultButton->update();
    }
    if (!progressBars.isEmpty()) {
        int i = 0;
        while (i < progressBars.size()) {
            QWidget *maybeProgress = progressBars.at(i);
            if (!maybeProgress) {
                progressBars.removeAt(i);
            } else {
                if (QProgressBar *pb = qt_cast<QProgressBar *>(maybeProgress)) {
                    if (pb->maximum() == 0 || pb->value() > 0
                        && pb->value() < pb->maximum()) {
                        if (doAnimate(AquaProgressBar))
                            pb->update();
                    }
                }
#ifdef QT_COMPAT
                else {
                    // Watch me now...
                    QVariant progress = maybeProgress->property("progress");
                    QVariant totalSteps = maybeProgress->property("totalSteps");
                    if (progress.isValid() && totalSteps.isValid()) {
                        int intProgress = progress.toInt();
                        int intTotalSteps = totalSteps.toInt();
                        if (intTotalSteps == 0 || intProgress > 0 && intProgress < intTotalSteps) {
                            if (doAnimate(AquaProgressBar))
                                maybeProgress->update();
                        }
                    }
                }
#endif
                ++i;
            }
        }
        animated += i;
    }
    if (animated <= 0) {
        killTimer(timerID);
        timerID = -1;
    }
}

bool QMacStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    //focus
    if (o->isWidgetType() && animationFocusWidget && focusable(static_cast<QWidget *>(o))
        && ((e->type() == QEvent::FocusOut && animationFocusWidget == o)
            || (e->type() == QEvent::FocusIn && animationFocusWidget != o)))  { //restore it
        if (static_cast<QFocusEvent *>(e)->reason() != QFocusEvent::Popup)
            focusOnWidget(0);
    }
    if (o && o->isWidgetType() && e->type() == QEvent::FocusIn) {
        QWidget *w = static_cast<QWidget *>(o);
        if (focusable(w))
            focusOnWidget(w);
    }
    //animate
    if (QProgressBar *pb = qt_cast<QProgressBar *>(o)) {
        switch (e->type()) {
        default:
            break;
        case QEvent::Show:
            if (!progressBars.contains(pb))
                startAnimate(AquaProgressBar, pb);
            break;
        case QEvent::Hide:
            progressBars.removeAll(pb);
        }
    } else if (QPushButton *btn = qt_cast<QPushButton *>(o)) {
        switch (e->type()) {
        default:
            break;
        case QEvent::FocusIn:
            if (btn->autoDefault())
                startAnimate(AquaPushButton, btn);
            break;
        case QEvent::Hide:
            if (btn == defaultButton)
                stopAnimate(AquaPushButton, btn);
            break;
        case QEvent::MouseButtonPress:
            // It is very confusing to keep the button pulsing, so just stop the animation.
            stopAnimate(AquaPushButton, btn);
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::FocusOut:
        case QEvent::Show: {
            QList<QPushButton *> list = qFindChildren<QPushButton *>(btn->topLevelWidget());
            for (int i = 0; i < list.size(); ++i) {
                QPushButton *pBtn = list.at(i);
                if ((e->type() == QEvent::FocusOut
                     && (pBtn->isDefault() || (pBtn->autoDefault() && pBtn->hasFocus()))
                     && pBtn != btn)
                    || ((e->type() == QEvent::Show || e->type() == QEvent::MouseButtonRelease)
                        && pBtn->isDefault())) {
                    if (pBtn->topLevelWidget()->isActiveWindow())
                        startAnimate(AquaPushButton, pBtn);
                    break;
                }
            }
            break; }
        }
    }
    return false;
}

bool QMacStylePrivate::doAnimate(QMacStylePrivate::Animates as)
{
    if (as == AquaPushButton && !useHITheme) {
        if (buttonState.frame == 25 && buttonState.dir == ButtonState::ButtonDark)
            buttonState.dir = ButtonState::ButtonLight;
        else if (!buttonState.frame && buttonState.dir == ButtonState::ButtonLight)
            buttonState.dir = ButtonState::ButtonDark;
        buttonState.frame += ((buttonState.dir == ButtonState::ButtonDark) ? 1 : -1);
    } else if (as == AquaProgressBar) {
        ++progressFrame;
    } else if (as == AquaListViewItemOpen) {
        // To be revived later...
    }
    return true;
}

void QMacStylePrivate::doFocus(QWidget *w)
{
    if (!aquaFocus)
        aquaFocus = new QAquaFocusWidget(w);
    aquaFocus->setFocusedWidget(w);
}

void QMacStylePrivate::HIThemePolish(QWidget *w)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    addWidget(w);
    QPixmap px(0, 0, 32);
    if (qt_mac_is_metal(w)) {
        px.resize(200, 200);
        HIThemeBackgroundDrawInfo bginfo;
        bginfo.version = qt_mac_hitheme_version;
        bginfo.state = kThemeStateActive;
        bginfo.kind = kThemeBackgroundMetal;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawBackground(&rect, &bginfo, QCFType<CGContextRef>(qt_macCreateCGHandle(&px)),
                              kHIThemeOrientationNormal);
    }

    if (::qt_cast<QMenu*>(w)) {
        px.resize(200, 200);
        HIThemeMenuDrawInfo mtinfo;
        mtinfo.version = qt_mac_hitheme_version;
        mtinfo.menuType = kThemeMenuTypePopUp;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawMenuBackground(&rect, &mtinfo, QCFType<CGContextRef>(qt_macCreateCGHandle(&px)),
                                  kHIThemeOrientationNormal);
        w->setWindowOpacity(0.95);
    }
    if (!px.isNull()) {
        QPalette pal = w->palette();
        QBrush background(px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
    }

#ifndef QT_NO_MAINWINDOW
    else if(QToolBar *bar = qt_cast<QToolBar*>(w)) {
        QLayout *layout = bar->layout();
        layout->setSpacing(0);
        layout->setMargin(0);
    }
#endif
    else if(QRubberBand *rubber = qt_cast<QRubberBand*>(w)) {
        rubber->setWindowOpacity(0.25);
    }
    q->QWindowsStyle::polish(w);
#else
    Q_UNUSED(w);
#endif
}

void QMacStylePrivate::HIThemeUnPolish(QWidget *w)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    removeWidget(w);
    if (::qt_cast<QMenu*>(w) || qt_mac_is_metal(w)) {
        QPalette pal = w->palette();
        QPixmap tmp;
        QBrush background(tmp);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
        w->setWindowOpacity(1.0);
    }
    if (::qt_cast<QRubberBand*>(w))
        w->setWindowOpacity(1.0);
    q->QWindowsStyle::unPolish(w);
#else
    Q_UNUSED(w);
#endif
}

void QMacStylePrivate::HIThemePolish(QApplication *app)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QPainter p(&px);

    HIThemeMenuDrawInfo mtinfo;
    mtinfo.version = qt_mac_hitheme_version;
    mtinfo.menuType = kThemeMenuTypeHierarchical;
    HIRect rect = CGRectMake(0, 0, px.width(), px.height());
    HIThemeDrawMenuBackground(&rect, &mtinfo, static_cast<CGContextRef>(px.handle()),
                              kHIThemeOrientationNormal);

    p.end();
    QBrush background(px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
    app->setPalette(pal);
#else
    Q_UNUSED(app);
#endif
}

void QMacStylePrivate::HIThemeDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt,
                                            QPainter *p, const QWidget *w) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeDrawState tds = getDrawState(opt->state);
    QMacCGContext cg(p);
    switch (pe) {
    case QStyle::PE_Q3CheckListExclusiveIndicator:
    case QStyle::PE_Q3CheckListIndicator:
    case QStyle::PE_IndicatorRadioButtonMask:
    case QStyle::PE_IndicatorRadioButton:
    case QStyle::PE_IndicatorCheckBoxMask:
    case QStyle::PE_IndicatorCheckBox: {
        HIThemeButtonDrawInfo bdi;
        bdi.version = qt_mac_hitheme_version;
        bdi.state = tds;
        bdi.adornment = kThemeDrawIndicatorOnly;
        if (opt->state & QStyle::Style_HasFocus
                && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
            bdi.adornment |= kThemeAdornmentFocus;
        bool isRadioButton = (pe == QStyle::PE_Q3CheckListExclusiveIndicator
                              || pe == QStyle::PE_IndicatorRadioButtonMask
                              || pe == QStyle::PE_IndicatorRadioButton);
        switch (qt_aqua_size_constrain(w)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            if (isRadioButton)
                bdi.kind = kThemeRadioButton;
            else
                bdi.kind = kThemeCheckBox;
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                if (isRadioButton)
                    bdi.kind = kThemeMiniRadioButton;
                else
                    bdi.kind = kThemeMiniCheckBox;
                break;
            }
#endif
        case QAquaSizeSmall:
            if (isRadioButton)
                bdi.kind = kThemeSmallRadioButton;
            else
                bdi.kind = kThemeSmallCheckBox;
            break;
        }
        if (opt->state & QStyle::Style_NoChange)
            bdi.value = kThemeButtonMixed;
        else if (opt->state & QStyle::Style_On)
            bdi.value = kThemeButtonOn;
        else
            bdi.value = kThemeButtonOff;
        HIRect macRect = qt_hirectForQRect(opt->rect, p);
        if (pe == QStyle::PE_IndicatorCheckBoxMask || pe == QStyle::PE_IndicatorRadioButtonMask) {
            QRegion saveRegion = p->clipRegion();
            QCFType<HIShapeRef> macRegion;
            HIThemeGetButtonShape(&macRect, &bdi, &macRegion);
            p->setClipRegion(qt_mac_convert_mac_region(macRegion));
            p->fillRect(opt->rect, Qt::color1);
            p->setClipRegion(saveRegion);
        } else {
            HIThemeDrawButton(&macRect, &bdi, cg, kHIThemeOrientationNormal, 0);
        }
        break; }
    case QStyle::PE_IndicatorArrowUp:
    case QStyle::PE_IndicatorArrowDown:
    case QStyle::PE_IndicatorArrowRight:
    case QStyle::PE_IndicatorArrowLeft: {
        HIThemePopupArrowDrawInfo pdi;
        pdi.version = qt_mac_hitheme_version;
        pdi.state = tds;
        switch (pe) {
        case QStyle::PE_IndicatorArrowUp:
            pdi.orientation = kThemeArrowUp;
            break;
        case QStyle::PE_IndicatorArrowDown:
            pdi.orientation = kThemeArrowDown;
            break;
        case QStyle::PE_IndicatorArrowRight:
            pdi.orientation = kThemeArrowRight;
            break;
        case QStyle::PE_IndicatorArrowLeft:
            pdi.orientation = kThemeArrowLeft;
            break;
        default:     // Stupid compiler _should_ know better.
            break;
        }
        if (opt->rect.width() < 8)
            pdi.size = kThemeArrow5pt;
        else
            pdi.size = kThemeArrow9pt;
        HIRect macRect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawPopupArrow(&macRect, &pdi, cg, kHIThemeOrientationNormal);
        break; }
    case QStyle::PE_FrameFocusRect:
        // Use the our own focus widget stuff.
        break;
    case QStyle::PE_IndicatorBranch: {
        if (!(opt->state & QStyle::Style_Children))
            break;
        HIThemeButtonDrawInfo bi;
        bi.version = qt_mac_hitheme_version;
        bi.state = opt->state & QStyle::Style_Enabled ?  kThemeStateActive : kThemeStateInactive;
        if (opt->state & QStyle::Style_Down)
            bi.state |= kThemeStatePressed;
        bi.kind = kThemeDisclosureButton;
        bi.value = opt->state & QStyle::Style_Open ? kThemeDisclosureDown : kThemeDisclosureRight;
        bi.adornment = kThemeAdornmentNone;
        HIRect hirect = qt_hirectForQRect(opt->rect); // ### passing the painter causes bad stuff in Q3ListView...
        HIThemeDrawButton(&hirect, &bi, cg, kHIThemeOrientationNormal, 0);
        break; }
    case QStyle::PE_IndicatorHeaderArrow:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            if (w && (qt_cast<QTreeView *>(w->parentWidget())
#ifdef QT_COMPAT
			|| w->parentWidget()->inherits("Q3ListView")
#endif
		))
		break; // ListView-type header is taken care of.
	    q->drawPrimitive(header->state & QStyle::Style_Up ? QStyle::PE_IndicatorArrowUp : QStyle::PE_IndicatorArrowDown, header, p, w);
        }
        break;
    case QStyle::PE_PanelHeader:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            bool scaleHeader = false;
            SInt32 headerHeight = 0;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = tds;
            QStyle::StyleFlags flags = header->state;
            QRect ir = header->rect;
            if (w && (qt_cast<QTreeView *>(w->parentWidget())
#ifdef QT_COMPAT
			|| w->parentWidget()->inherits("Q3ListView")
#endif
		)) {
                bdi.kind = kThemeListHeaderButton;
                GetThemeMetric(kThemeMetricListHeaderHeight, &headerHeight);
                if (ir.height() > headerHeight)
                    scaleHeader = true;
            } else {
                bdi.kind = kThemeBevelButton;
                if (p->font().bold())
                    flags |= QStyle::Style_Sunken;
                else
                    flags &= ~QStyle::Style_Sunken;
            }
            if (flags & QStyle::Style_Sunken)
                bdi.value = kThemeButtonOn;
            else
                bdi.value = kThemeButtonOff;

            bdi.adornment = kThemeAdornmentNone;

            if (flags & QStyle::Style_Off)
                ir.setRight(ir.right() + 50);  // Cheat to hide the down indicator.
            else if (flags & QStyle::Style_Up)
                bdi.adornment = kThemeAdornmentHeaderButtonSortUp;

            if (flags & QStyle::Style_HasFocus && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment = kThemeAdornmentFocus;
	    // The ListViewHeader button is only drawn one size, so draw into a pixmap and scale it
            // Otherwise just draw it normally.
	    if (scaleHeader) {
		QPixmap headerPix(ir.width(), headerHeight);
		QPainter pixPainter(&headerPix);
		QMacCGContext pixCG(&pixPainter);
		HIRect pixRect = CGRectMake(0, 0, ir.width(), headerHeight);
		HIThemeDrawButton(&pixRect, &bdi, pixCG, kHIThemeOrientationNormal, 0);
		p->drawPixmap(ir, headerPix);
	    } else {
		HIRect hirect = qt_hirectForQRect(ir, p);
		HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
	    }
        }
        break;
    case QStyle::PE_FrameGroupBox:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            HIThemeGroupBoxDrawInfo gdi;
            gdi.version = qt_mac_hitheme_version;
            gdi.state = tds;
            if (w && ::qt_cast<QGroupBox *>(w->parentWidget()))
                gdi.kind = kHIThemeGroupBoxKindSecondary;
            else
                gdi.kind = kHIThemeGroupBoxKindPrimary;
            HIRect hirect = qt_hirectForQRect(frame->rect, p);
            HIThemeDrawGroupBox(&hirect, &gdi, cg, kHIThemeOrientationNormal);
        }
        break;
    case QStyle::PE_Frame:
    case QStyle::PE_FrameLineEdit:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            if (frame->state & QStyle::Style_Sunken) {
                QColor baseColor(frame->palette.background().color());
                HIThemeFrameDrawInfo fdi;
                fdi.version = qt_mac_hitheme_version;
                fdi.state = tds;
                SInt32 frame_size;
                if (pe == QStyle::PE_FrameLineEdit) {
                    fdi.kind = kHIThemeFrameTextFieldSquare;
                    GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
                } else {
                    baseColor = QColor(150, 150, 150); //hardcoded since no query function --Sam
                    fdi.kind = kHIThemeFrameListBox;
                    GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
                }
                fdi.isFocused = (frame->state & QStyle::Style_HasFocus);
                int lw = frame->lineWidth;
                if (lw <= 0)
                    lw = q->pixelMetric(QStyle::PM_DefaultFrameWidth, frame, w);
                { //clear to base color
                    p->save();
                    p->setPen(QPen(baseColor, lw));
                    p->setBrush(Qt::NoBrush);
                    p->drawRect(frame->rect);
                    p->restore();
                }
                HIRect hirect = qt_hirectForQRect(frame->rect, p, false,
                                                  QRect(frame_size, frame_size,
                                                        frame_size * 2, frame_size * 2));

                HIThemeDrawFrame(&hirect, &fdi, cg, kHIThemeOrientationNormal);
                break;
            } else {
                q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
                break;
            }
        }
    case QStyle::PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf
                = qt_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            QRect paneRect = twf->rect;
            ThemeTabDirection ttd = getTabDirection(twf->shape);
            switch (ttd) {
            case kThemeTabSouth:
                paneRect.setHeight(paneRect.height() + 7);
                break;
            case kThemeTabNorth:
                paneRect.setTop(paneRect.top() - 6);
                paneRect.setHeight(paneRect.height() + 3);
                break;
            case kThemeTabWest:
                paneRect.setLeft(paneRect.left() - 6);
                paneRect.setWidth(paneRect.width());
                break;
            case kThemeTabEast:
                paneRect.setWidth(paneRect.width() + 4);
                break;
            }
            HIRect hirect = qt_hirectForQRect(paneRect, p);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
                HIThemeTabPaneDrawInfo tpdi;
                tpdi.version = qt_mac_hitheme_tab_version();
                tpdi.state = tds;
                tpdi.direction = ttd;
                tpdi.size = kHIThemeTabSizeNormal;
                if (tpdi.version == 1) {
                    tpdi.kind = kHIThemeTabKindNormal;
                    tpdi.adornment = kHIThemeTabPaneAdornmentNormal;
                }
                HIThemeDrawTabPane(&hirect, &tpdi, cg, kHIThemeOrientationNormal);
            } else
#else
            {
                HIThemeGroupBoxDrawInfo gdi;
                gdi.version = qt_mac_hitheme_version;
                gdi.state = tds;
                gdi.kind = kHIThemeGroupBoxKindSecondary;
                HIThemeDrawGroupBox(&hirect, &gdi, cg, kHIThemeOrientationNormal);
            }
#endif
        }
        break;
    case QStyle::PE_FrameTabBarBase:
        break;
    default:
        q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
#else
    q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
#endif
}

void QMacStylePrivate::HIThemeDrawControl(QStyle::ControlElement ce, const QStyleOption *opt,
                                          QPainter *p, const QWidget *w) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeDrawState tds = getDrawState(opt->state);
    QMacCGContext cg(p);
    switch (ce) {
    case QStyle::CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = ::qt_cast<const QStyleOptionButton *>(opt)) {
            if (!(btn->state & (QStyle::Style_Raised | QStyle::Style_Down | QStyle::Style_On)))
                break;
            if(btn->state & QStyle::Style_On)
                tds = kThemeStatePressed;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = tds;
            bdi.adornment = kThemeAdornmentNone;
            bdi.value = kThemeButtonOff;
            if (btn->features & ((QStyleOptionButton::Flat | QStyleOptionButton::HasMenu)))
                bdi.kind = kThemeBevelButton;
            else
                bdi.kind = kThemePushButton;
            if (btn->state & QStyle::Style_HasFocus
                    && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment |= kThemeAdornmentFocus;
            if (btn->features & QStyleOptionButton::DefaultButton && animatable(AquaPushButton, w)) {
                bdi.adornment |= kThemeAdornmentDefault;
                bdi.animation.time.start = defaultButtonStart;
                bdi.animation.time.current = CFAbsoluteTimeGetCurrent();
                if (timerID <= -1) {
                    QTimer::singleShot(0, const_cast<QMacStylePrivate *>(this),
                                       SLOT(startAnimationTimer()));
                }
            }
            HIRect newRect = qt_hirectForQRect(btn->rect);
            // Like Appearance Manager, HITheme draws outside my rect, so make it a bit bigger.
            QRect off_rct;
            HIRect outRect;
            HIThemeGetButtonBackgroundBounds(&newRect, &bdi, &outRect);
            off_rct.setRect(int(newRect.origin.x - outRect.origin.x),
                            int(newRect.origin.y - outRect.origin.y),
                            int(outRect.size.width - newRect.size.width),
                            int(outRect.size.height - newRect.size.height));
            newRect = qt_hirectForQRect(btn->rect, p, false, off_rct);
            HIThemeDrawButton(&newRect, &bdi, cg, kHIThemeOrientationNormal, 0);
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbi = q->pixelMetric(QStyle::PM_MenuButtonIndicator, btn, w);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() / 2 - 5, mbi, ir.height() / 2);
                q->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, p, w);
            }
        }
        break;
    case QStyle::CE_MenuItem:
    case QStyle::CE_MenuEmptyArea:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(mi->rect, opt->palette.background());

            int tabwidth = mi->tabWidth;
            int maxpmw = mi->maxIconWidth;
            bool active = mi->state & QStyle::Style_Selected;
            bool enabled = mi->state & QStyle::Style_Enabled;
            HIRect menuRect = qt_hirectForQRect(mi->menuRect, p, false);
            HIRect itemRect = qt_hirectForQRect(mi->rect, p, false);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            mdi.itemType = kThemeMenuItemPlain;
            if (!mi->icon.isNull())
                mdi.itemType |= kThemeMenuItemHasIcon;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                mdi.itemType |= kThemeMenuItemHierarchical | kThemeMenuItemHierBackground;
            else
                mdi.itemType |= kThemeMenuItemPopUpBackground;
            if (enabled)
                mdi.state = kThemeMenuActive;
            else
                mdi.state = kThemeMenuDisabled;
            if (active)
                mdi.state |= kThemeMenuSelected;
            QRect contentRect;
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                // First arg should be &menurect, but wacky stuff happens then.
                HIThemeDrawMenuSeparator(&itemRect, &itemRect, &mdi,
                                         cg, kHIThemeOrientationNormal);
                break;
            } else {
                HIRect cr;
                HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                    cg, kHIThemeOrientationNormal, &cr);
                if(ce == QStyle::CE_MenuEmptyArea)
                    break;
                contentRect = qt_qrectForHIRect(cr);
            }
            int xpos = contentRect.x() + 18;
            int checkcol = maxpmw;
            if (!enabled)
                p->setPen(mi->palette.text().color());
            else if (active)
                p->setPen(mi->palette.highlightedText().color());
            else
                p->setPen(mi->palette.buttonText().color());

            if (mi->checked) {
                // Use the HIThemeTextInfo foo to draw the check mark correctly, if we do it,
                // we somehow need to use a special encoding as it doesn't look right with our
                // drawText().
                HIThemeTextInfo tti;
                tti.version = qt_mac_hitheme_version;
                tti.state = tds;
                if (active && enabled)
                    tti.state = kThemeStatePressed;
                tti.fontID = kThemeMenuItemMarkFont;
                tti.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
                tti.verticalFlushness = kHIThemeTextVerticalFlushCenter;
                tti.options = kHIThemeTextBoxOptionNone;
                tti.truncationPosition = kHIThemeTextTruncationNone;
                tti.truncationMaxLines = 1;
                QCFString checkmark;
                if (mi->checkType == QStyleOptionMenuItem::Exclusive)
                    checkmark = QString(QChar(kDiamondUnicode));
                else
                    checkmark = QString(QChar(kCheckUnicode));
                int mw = checkcol + macItemFrame;
                int mh = contentRect.height() - 2 * macItemFrame;
                int xp = contentRect.x();
                xp += macItemFrame;
                float outWidth, outHeight, outBaseline;
                HIThemeGetTextDimensions(checkmark, 0, &tti, &outWidth, &outHeight,
                                         &outBaseline);
                QRect r(xp, contentRect.y(), mw, mh);
                r.translate(0, p->fontMetrics().ascent() - int(outBaseline) + 1);
                HIRect bounds = qt_hirectForQRect(r);
                HIThemeDrawTextBox(checkmark, &bounds, &tti,
                                   cg, kHIThemeOrientationNormal);
            }
            if (!mi->icon.isNull()) {
                QIcon::Mode mode = (mi->state & QStyle::Style_Enabled) ? QIcon::Normal
                                                                       : QIcon::Disabled;
                // Always be normal or disabled to follow the Mac style.
                QPixmap pixmap = mi->icon.pixmap(Qt::SmallIconSize, mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(xpos, contentRect.y(), checkcol, contentRect.height());
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }

            QString s = mi->text;
            if (!s.isEmpty()) {
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextHideMnemonic
                                 | Qt::TextSingleLine;
                p->save();
                if (t >= 0) {
                    extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp
                    p->setFont(qt_app_fonts_hash()->value("QMenuItem", p->font()));
                    int xp = contentRect.right() - tabwidth - macRightBorder
                             - macItemHMargin - macItemFrame + 1;
                    p->drawText(xp, contentRect.y(), tabwidth, contentRect.height(), text_flags,
                                s.mid(t + 1));
                    s = s.left(t);
                }

                p->setFont(mi->font);
                const int xm = macItemFrame + maxpmw + macItemHMargin;
                p->drawText(xpos, contentRect.y(), contentRect.width() - xm - tabwidth + 1,
                            contentRect.height(), text_flags ^ Qt::AlignRight, s);
                p->restore();
            }
        }
        break;
    case QStyle::CE_MenuHMargin:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(mi->rect, opt->palette.background());

            HIRect menuRect = qt_hirectForQRect(mi->menuRect);
            HIRect itemRect = qt_hirectForQRect(mi->rect);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            if (!(opt->state & QStyle::Style_Enabled))
                mdi.state = kThemeMenuDisabled;
            else if (opt->state & QStyle::Style_Selected)
                mdi.state = kThemeMenuSelected;
            else
                mdi.state = kThemeMenuActive;
            if (ce == QStyle::CE_MenuScroller) {
                if (opt->state & QStyle::Style_Down)
                    mdi.itemType = kThemeMenuItemScrollDownArrow;
                else
                    mdi.itemType = kThemeMenuItemScrollUpArrow;
            } else {
                mdi.itemType = kThemeMenuItemPlain;
            }
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                cg,
                                kHIThemeOrientationNormal, 0);
            if (ce == QStyle::CE_MenuTearoff) {
                p->setPen(QPen(mi->palette.dark().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2 - 1);
                p->setPen(QPen(mi->palette.light().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2);
            }
        }
        break;
    case QStyle::CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(mi->rect, opt->palette.background());

            HIRect menuRect = qt_hirectForQRect(mi->menuRect);
            HIRect itemRect = qt_hirectForQRect(mi->rect);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            if (!(opt->state & QStyle::Style_Enabled))
                mdi.state = kThemeMenuDisabled;
            else if ((opt->state & QStyle::Style_Selected) && (opt->state & QStyle::Style_Down))
                mdi.state = kThemeMenuSelected;
            else
                mdi.state = kThemeMenuActive;
            mdi.itemType = kThemeMenuItemPlain;
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                cg, kHIThemeOrientationNormal, 0);

            //text
            q->drawItem(p, mi->rect,
                        Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip | Qt::TextSingleLine,
                        mi->palette, mi->state & QStyle::Style_Enabled,
                        mi->icon.pixmap(Qt::SmallIconSize, QIcon::Normal),
                        mi->text, &mi->palette.buttonText().color());
        }
        break;
    case QStyle::CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            HIThemeMenuBarDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeMenuBarNormal;
            bdi.attributes = 0;
            HIRect hirect = qt_hirectForQRect(mi->rect);
            HIThemeDrawMenuBarBackground(&hirect, &bdi, cg,
                                         kHIThemeOrientationNormal);
            break;
        }
    case QStyle::CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            tdi.version = qt_mac_hitheme_version;
            tdi.reserved = 0;
            bool isIndeterminate = (pb->minimum == 0 && pb->maximum == 0);
            switch (qt_aqua_size_constrain(w)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                tdi.kind = !isIndeterminate ? kThemeLargeProgressBar
                                            : kThemeLargeIndeterminateBar;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    tdi.kind = !isIndeterminate ? kThemeMiniProgressBar
                                                : kThemeMiniIndeterminateBar;
                    break;
                }
#endif
            case QAquaSizeSmall:
                tdi.kind = !isIndeterminate ? kThemeProgressBar : kThemeIndeterminateBar;
                break;
            }
            tdi.bounds = qt_hirectForQRect(pb->rect);
            tdi.max = pb->maximum;
            tdi.min = pb->minimum;
            tdi.value = pb->progress;
            tdi.attributes = kThemeTrackHorizontal;
            tdi.trackInfo.progress.phase = progressFrame;
            if (!(pb->state & QStyle::Style_Active))
                tdi.enableState = kThemeTrackInactive;
            else if (!(pb->state & QStyle::Style_Enabled))
                tdi.enableState = kThemeTrackDisabled;
            else
                tdi.enableState = kThemeTrackActive;
            HIThemeDrawTrack(&tdi, 0, cg, kHIThemeOrientationNormal);
        }
        break;
    case QStyle::CE_ProgressBarLabel:
    case QStyle::CE_ProgressBarGroove:
        break;
    case QStyle::CE_TabBarTab:
        if (const QStyleOptionTab *tabOpt = qt_cast<const QStyleOptionTab *>(opt)) {
            if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_4) {
                drawPantherTab(tabOpt, p, w);
            } else {
                HIThemeTabDrawInfo tdi;
                tdi.version = 1;
                tdi.style = kThemeTabNonFront;
                if (tabOpt->state & QStyle::Style_Selected) {
                    if (!(tabOpt->state & QStyle::Style_Active))
                        tdi.style = kThemeTabFrontUnavailable;
                    else if (!(tabOpt->state & QStyle::Style_Enabled))
                        tdi.style = kThemeTabFrontInactive;
                    else
                        tdi.style = kThemeTabFront;
                } else if (!(tabOpt->state & QStyle::Style_Active)) {
                    tdi.style = kThemeTabNonFrontUnavailable;
                } else if (!(tabOpt->state & QStyle::Style_Enabled)) {
                    tdi.style = kThemeTabNonFrontInactive;
                } else if ((tabOpt->state & (QStyle::Style_Sunken | QStyle::Style_MouseOver))
                           == (QStyle::Style_Sunken | QStyle::Style_MouseOver)) {
                    tdi.style = kThemeTabNonFrontPressed;
                }
                tdi.direction = getTabDirection(tabOpt->shape);
                if (tabOpt->state & QStyle::Style_HasFocus)
                    tdi.adornment = kHIThemeTabAdornmentFocus;
                else
                    tdi.adornment = kHIThemeTabAdornmentNone;
                QRect tabRect = tabOpt->rect;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
                tdi.kind = kHIThemeTabKindNormal;
                if (tabOpt->position == QStyleOptionTab::Beginning)
                    tdi.position = kHIThemeTabPositionFirst;
                else if (tabOpt->position == QStyleOptionTab::Middle)
                    tdi.position = kHIThemeTabPositionMiddle;
                else if (tabOpt->position == QStyleOptionTab::End)
                    tdi.position = kHIThemeTabPositionLast;
                else
                    tdi.position = kHIThemeTabPositionOnly;
                if (tabOpt->position != QStyleOptionTab::End)
                    tabRect.setWidth(tabRect.width() + 1);
#endif
                HIRect hirect = qt_hirectForQRect(tabRect, p);
                QCFType<HIShapeRef> tabDrawShape;
                HIThemeGetTabDrawShape(&hirect, &tdi, &tabDrawShape);
                HIShapeGetBounds(tabDrawShape, &hirect);
                HIThemeDrawTab(&hirect, &tdi, cg, kHIThemeOrientationNormal, 0);
#if 0
                // Draw the little line for the tabs.
                if (tabOpt->position != QStyleOptionTab::End) {
                    QCFType<HIShapeRef> tabShape;
                    HIRect hirect2 = qt_hirectForQRect(tabRect);
                    HIThemeGetTabShape(&hirect2, &tdi, &tabShape);
                    HIShapeGetBounds(tabShape, &hirect2);
                    QRect newRect = qt_qrectForHIRect(hirect2);
                    QColor curCol = opt->state & QStyle::Style_Selected ? QColor(0, 0, 255, 30)
                                                                        : QColor(0, 0, 0, 51);
                    p->save();
                    p->setPen(curCol);
                    // I don't like these y-components in the QPoint thing, but it seems that
                    // the bounds for the tab shape a a bit bigger than the actual tab.
                    p->drawLine(newRect.topRight() - QPoint(1, -5),
                                newRect.bottomRight() - QPoint(1, 2));
                    p->restore();
                }
#endif
            }
        }
        break;
    case QStyle::CE_SizeGrip: {
        HIThemeGrowBoxDrawInfo gdi;
        gdi.version = qt_mac_hitheme_version;
        gdi.state = tds;
        gdi.kind = kHIThemeGrowBoxKindNormal;
        gdi.direction = kThemeGrowRight | kThemeGrowDown;
        gdi.size = kHIThemeGrowBoxSizeNormal;
        HIPoint pt = CGPointMake(opt->rect.x(), opt->rect.y());
        HIThemeDrawGrowBox(&pt, &gdi, cg, kHIThemeOrientationNormal);
        break; }
    case QStyle::CE_Splitter: {
        HIThemeSplitterDrawInfo sdi;
        sdi.version = qt_mac_hitheme_version;
        sdi.state = tds;
        sdi.adornment = qt_mac_is_metal(w) ? kHIThemeSplitterAdornmentMetal
                                           : kHIThemeSplitterAdornmentNone;
        HIRect hirect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawPaneSplitter(&hirect, &sdi, cg, kHIThemeOrientationNormal);
        break; }
    case QStyle::CE_RubberBand:
        p->fillRect(opt->rect, opt->palette.brush(QPalette::Disabled, QPalette::Highlight));
        break;
    default:
        q->QWindowsStyle::drawControl(ce, opt, p, w);
    }
#else
    q->QWindowsStyle::drawControl(ce, opt, p, w);
#endif
}

QRect QMacStylePrivate::HIThemeSubRect(QStyle::SubRect sr, const QStyleOption *opt, const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QRect r;
    switch (sr) {
    case QStyle::SR_PushButtonContents:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            HIRect inRect = CGRectMake(btn->rect.x(), btn->rect.y(),
                                       btn->rect.width(), btn->rect.height());
            HIRect outRect;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeStateActive;
            if (btn->features & QStyleOptionButton::None)
                bdi.kind = kThemePushButton;
            else
                bdi.kind = kThemeBevelButton;
            bdi.adornment = kThemeAdornmentNone;
            HIThemeGetButtonContentBounds(&inRect, &bdi, &outRect);
            r.setRect(int(outRect.origin.x), int(outRect.origin.y - 2),
                      int(qMin(qAbs(btn->rect.width() - 2 * outRect.origin.x), outRect.size.width)),
                      int(qMin(qAbs(btn->rect.height() - 2 * outRect.origin.y), outRect.size.height)));
        }
        break;
    case QStyle::SR_ProgressBarGroove:
    case QStyle::SR_ProgressBarLabel:
        break;
    case QStyle::SR_ProgressBarContents:
        r = opt->rect;
        break;
    default:
        r = q->QWindowsStyle::subRect(sr, opt, widget);
        break;
    }
    return r;
#else
    return q->QWindowsStyle::subRect(sr, opt, widget);
#endif
}

void QMacStylePrivate::HIThemeDrawComplexControl(QStyle::ComplexControl cc,
                                                 const QStyleOptionComplex *opt,
                                                 QPainter *p, const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeDrawState tds = getDrawState(opt->state);
    QMacCGContext cg(p);
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            if (cc == QStyle::CC_Slider) {
                if (slider->activeSubControls == QStyle::SC_SliderHandle)
                    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
                else if (slider->activeSubControls == QStyle::SC_SliderGroove)
                    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
            } else {
                if (slider->activeSubControls == QStyle::SC_ScrollBarSubLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                         | kThemeLeftOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                         | kThemeRightOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarSubPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarSlider)
                    tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            }
            HIRect macRect;
            bool tracking = slider->sliderPosition == slider->sliderValue;
            if (!tracking) {
                // Small optimization, the same as q->querySubControlMetrics
                QCFType<HIShapeRef> shape;
                HIThemeGetTrackThumbShape(&tdi, &shape);
                HIShapeGetBounds(shape, &macRect);
                tdi.value = slider->sliderValue;
            }
            HIThemeDrawTrack(&tdi, tracking ? 0 : &macRect, cg,
                             kHIThemeOrientationNormal);
            if (cc == QStyle::CC_Slider && slider->subControls & QStyle::SC_SliderTickmarks) {
                int numMarks;
                if (slider->tickInterval)
                        numMarks = ((slider->maximum - slider->minimum + 1)
					/ slider->tickInterval) + 1;
                else
                    numMarks = ((slider->maximum - slider->minimum + 1) / slider->pageStep) + 1;
                if (tdi.trackInfo.slider.thumbDir == kThemeThumbPlain) {
                    // They asked for both, so we'll give it to them.
                    tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
                    HIThemeDrawTrackTickMarks(&tdi, numMarks,
                                              cg,
                                              kHIThemeOrientationNormal);
                    tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
                    HIThemeDrawTrackTickMarks(&tdi, numMarks,
                                              cg,
                                               kHIThemeOrientationNormal);
                } else {
                    HIThemeDrawTrackTickMarks(&tdi, numMarks,
                                              cg,
                                              kHIThemeOrientationNormal);

                }
            }
        }
        break;
    case QStyle::CC_ListView:
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if (lv->subControls & QStyle::SC_ListView)
                q->QWindowsStyle::drawComplexControl(cc, lv, p, widget);
            if (lv->subControls & (QStyle::SC_ListViewBranch | QStyle::SC_ListViewExpand)) {
                int y = lv->rect.y();
                int h = lv->rect.height();
                int x = lv->rect.right() - 10;
                for (int i = 1; i < lv->items.size() && y < h; ++i) {
                    QStyleOptionListViewItem item = lv->items.at(i);
                    if (y + item.height > 0 && (item.childCount > 0
                        || (item.features & (QStyleOptionListViewItem::Expandable
                                            | QStyleOptionListViewItem::Visible))
                            == (QStyleOptionListViewItem::Expandable
                                | QStyleOptionListViewItem::Visible))) {
                        QStyleOption treeOpt(0);
                        treeOpt.rect.setRect(x, y + item.height / 2 - 4, 9, 9);
                        treeOpt.palette = lv->palette;
                        treeOpt.state = lv->state;
                        treeOpt.state |= QStyle::Style_Children;
                        if (item.state & QStyle::Style_Open)
                            treeOpt.state |= QStyle::Style_Open;
                        q->drawPrimitive(QStyle::PE_IndicatorBranch, &treeOpt, p, widget);
                    }
                    y += item.totalHeight;
                }
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox newSB = *sb;
            if (sb->subControls & QStyle::SC_SpinBoxFrame) {
                p->fillRect(opt->rect, opt->palette.background());

                QStyleOptionFrame lineedit;
                lineedit.rect = QStyle::visualRect(opt->direction, opt->rect,
                                                   q->querySubControlMetrics(QStyle::CC_SpinBox,
                                                                             sb,
                                                                             QStyle::SC_SpinBoxFrame,
                                                                             widget));
                lineedit.palette = sb->palette;
                lineedit.state = QStyle::Style_Sunken;
                lineedit.lineWidth = 0;
                lineedit.midLineWidth = 0;
                q->drawPrimitive(QStyle::PE_FrameLineEdit, &lineedit, p, widget);
            }
            if (sb->subControls & (QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown)) {
                HIThemeButtonDrawInfo bdi;
                bdi.version = qt_mac_hitheme_version;
                QAquaWidgetSize aquaSize = qt_aqua_size_constrain(widget);
                switch (aquaSize) {
                    case QAquaSizeUnknown:
                    case QAquaSizeLarge:
                        bdi.kind = kThemeIncDecButton;
                        break;
                    case QAquaSizeMini:
                    case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                            if (aquaSize == QAquaSizeMini)
                                bdi.kind = kThemeIncDecButtonMini;
                            else
                                bdi.kind = kThemeIncDecButtonSmall;
                        } else {
                            bdi.kind = kThemeIncDecButton;
                        }
                        break;
#endif
                }
                if (!(sb->stepEnabled & (QAbstractSpinBox::StepUpEnabled
                                        | QAbstractSpinBox::StepDownEnabled)))
                    tds = kThemeStateUnavailable;
                if (sb->activeSubControls == QStyle::SC_SpinBoxDown)
                    tds = kThemeStatePressedDown;
                else if (sb->activeSubControls == QStyle::SC_SpinBoxUp)
                    tds = kThemeStatePressedUp;
                bdi.state = tds;
                bdi.value = kThemeButtonOff;
                if (sb->state & QStyle::Style_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    bdi.adornment = kThemeAdornmentFocus;
                else
                    bdi.adornment = kThemeAdornmentNone;
                QRect updown = QStyle::visualRect(opt->direction, opt->rect,
                                                  q->querySubControlMetrics(QStyle::CC_SpinBox, sb,
                                                                    QStyle::SC_SpinBoxUp, widget));
                updown |= QStyle::visualRect(opt->direction, opt->rect,
                                             q->querySubControlMetrics(QStyle::CC_SpinBox, sb,
                                                               QStyle::SC_SpinBoxDown, widget));
                if (widget) {
                    QPalette::ColorRole bgRole = widget->backgroundRole();
                    QPixmap pm = sb->palette.brush(bgRole).texture();
                    if (!pm.isNull())
                        p->drawPixmap(updown, pm);
                    else
                        p->fillRect(updown, sb->palette.color(bgRole));
                }
                HIRect hirect = qt_hirectForQRect(updown, p);
                HIThemeDrawButton(&hirect, &bdi, cg,
                                  kHIThemeOrientationNormal, 0);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            QRect comborect(cmb->rect);
            bdi.adornment = opt->state & QStyle::Style_HasFocus ? kThemeAdornmentFocus
                                                                : kThemeAdornmentNone;
            bdi.state = opt->activeSubControls & QStyle::SC_ComboBoxArrow
                                ? ThemeDrawState(kThemeStatePressed) : tds;
            if (cmb->editable) {
                bdi.adornment |= kThemeAdornmentArrowDownArrow;
                comborect = q->querySubControlMetrics(QStyle::CC_ComboBox, cmb,
                                                      QStyle::SC_ComboBoxArrow ,widget);
                QAquaWidgetSize aSize = qt_aqua_size_constrain(widget);
                switch (aSize) {
                    case QAquaSizeUnknown:
                    case QAquaSizeLarge:
                        bdi.kind = kThemeArrowButton;
                        break;
                    case QAquaSizeMini:
                    case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                            if (aSize == QAquaSizeMini)
                                bdi.kind = kThemeArrowButtonMini;
                            else
                                bdi.kind = kThemeArrowButtonSmall;
                        } else {
                            bdi.kind = kThemeArrowButton;
                        }
                        break;
#endif
                }
                QRect lineeditRect(cmb->rect);
                lineeditRect.setWidth(cmb->rect.width() - comborect.width());
                QStyleOptionFrame lineedit;
                lineedit.rect = lineeditRect;
                lineedit.palette = cmb->palette;
                lineedit.state = cmb->state;
                lineedit.lineWidth = 0;
                lineedit.midLineWidth = 0;
                q->drawPrimitive(QStyle::PE_FrameLineEdit, &lineedit, p, widget);
            } else {
                bdi.adornment |= kThemeAdornmentArrowLeftArrow;
                bdi.kind = kThemePopupButton;
            }
            HIRect hirect = qt_hirectForQRect(comborect, p);
            HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
        }
        break;
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *titlebar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            wdi.version = qt_mac_hitheme_version;
            wdi.state = tds;
            wdi.windowType = QtWinType;
            wdi.titleHeight = titlebar->rect.height();
            wdi.titleWidth = titlebar->rect.width();
            wdi.attributes = kThemeWindowHasTitleText;
            if (titlebar->titleBarState) {
                if (titlebar->titleBarState & Qt::WindowMinimized)
                    wdi.attributes |= kThemeWindowIsCollapsed;
                /*
                if (titlebar->window()->isWindowModified())
                    wdi.attributes |= kThemeWindowHasDirty;
                    */
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            } else if (titlebar->titleBarFlags & Qt::WStyle_SysMenu) {
                wdi.attributes |= kThemeWindowHasCloseBox;
            }
            QCFType<HIShapeRef> titleRegion;
            HIRect titleRect = qt_hirectForQRect(titlebar->rect, p);
            QRect newr = titlebar->rect;
            HIThemeGetWindowShape(&titleRect, &wdi, kWindowTitleBarRgn, &titleRegion);
            HIShapeGetBounds(titleRegion, &titleRect);
            newr.translate(newr.x() - (int)titleRect.origin.x, newr.y() - (int)titleRect.origin.y);
            HIRect finalRect = qt_hirectForQRect(newr, p, false);
            HIThemeDrawWindowFrame(&finalRect, &wdi, cg, kHIThemeOrientationNormal, 0);
            if (titlebar->subControls & QStyle::SC_TitleBarLabel) {
                int iw = 0;
                if (!titlebar->icon.isNull()) {
                    QCFType<HIShapeRef> titleRegion2;
                    HIThemeGetWindowShape(&finalRect, &wdi, kWindowTitleProxyIconRgn,
                                          &titleRegion2);
                    HIShapeGetBounds(titleRegion2, &titleRect);
                    if (titleRect.size.width != 1)
                        iw = titlebar->icon.width();
                }
                if (!titlebar->text.isEmpty()) {
                    p->save();
                    QCFType<HIShapeRef> titleRegion3;
                    HIThemeGetWindowShape(&finalRect, &wdi, kWindowTitleTextRgn, &titleRegion3);
                    HIShapeGetBounds(titleRegion3, &titleRect);
                    p->setClipRect(qt_qrectForHIRect(titleRect));
                    QRect br = p->clipRegion().boundingRect();
                    int x = br.x(),
                    y = br.y() + (titlebar->rect.height() / 2 - p->fontMetrics().height() / 2);
                    if (br.width() <= (p->fontMetrics().width(titlebar->text) + iw * 2))
                        x += iw;
                    else
                        x += br.width() / 2 - p->fontMetrics().width(titlebar->text) / 2;
                    if (iw)
                        p->drawPixmap(x - iw, y, titlebar->icon);
                    p->drawText(x, y + p->fontMetrics().ascent(), titlebar->text);
                    p->restore();
                }
            }
            if (titlebar->subControls & (QStyle::SC_TitleBarCloseButton
                                         | QStyle::SC_TitleBarMaxButton
                                         | QStyle::SC_TitleBarMinButton
                                         | QStyle::SC_TitleBarNormalButton)) {
                HIThemeWindowWidgetDrawInfo wwdi;
                wwdi.version = qt_mac_hitheme_version;
                wwdi.widgetState = tds;
                if (titlebar->state & QStyle::Style_MouseOver)
                    wwdi.widgetState = kThemeStateRollover;
                wwdi.windowType = QtWinType;
                wwdi.attributes = wdi.attributes;
                wwdi.windowState = wdi.state;
                wwdi.titleHeight = wdi.titleHeight;
                wwdi.titleWidth = wdi.titleWidth;
                ThemeDrawState savedControlState = wwdi.widgetState;
                uint sc = QStyle::SC_TitleBarMinButton;
                ThemeTitleBarWidget tbw = kThemeWidgetCollapseBox;
                bool active = titlebar->state & QStyle::Style_Active;
                while (sc <= QStyle::SC_TitleBarCloseButton) {
                    uint tmp = sc;
                    wwdi.widgetState = savedControlState;
                    wwdi.widgetType = tbw;
                    if (sc == QStyle::SC_TitleBarMinButton)
                        tmp |= QStyle::SC_TitleBarNormalButton;
                    if (active && (titlebar->activeSubControls & tmp))
                        wwdi.widgetState = kThemeStatePressed;
                    /*
                    if (titlebar->window() && titlebar->window()->isWindowModified()
                            && tbw == kThemeWidgetCloseBox)
                        wwdi.widgetType = kThemeWidgetDirtyCloseBox;
                            */
                    HIThemeDrawTitleBarWidget(&finalRect, &wwdi, cg, kHIThemeOrientationNormal);
                    sc = sc << 1;
                    tbw = tbw >> 1;
                }
            }
        }
        break;
    case QStyle::CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            ThemeButtonKind bkind = kThemeBevelButton;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bkind = kThemeBevelButton;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    bkind = kThemeMiniBevelButton;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bkind = kThemeSmallBevelButton;
                break;
            }

            QRect button, menuarea;
            button   = q->querySubControlMetrics(cc, tb, QStyle::SC_ToolButton, widget);
            menuarea = q->querySubControlMetrics(cc, tb, QStyle::SC_ToolButtonMenu, widget);
	    QStyle::StyleFlags bflags = tb->state,
            mflags = tb->state;
            if (tb->subControls & QStyle::SC_ToolButton)
                bflags |= QStyle::Style_Down;
            if (tb->subControls & QStyle::SC_ToolButtonMenu)
                mflags |= QStyle::Style_Down;

            if (tb->subControls & QStyle::SC_ToolButton) {
                if(bflags & (QStyle::Style_Down | QStyle::Style_On | QStyle::Style_Raised)) {
                    HIThemeButtonDrawInfo bdi;
                    bdi.version = qt_mac_hitheme_version;
                    bdi.state = tds;
                    bdi.adornment = kThemeAdornmentNone;
                    bdi.kind = bkind;
                    bdi.value = kThemeButtonOff;
                    if (tb->state & QStyle::Style_HasFocus && QMacStyle::focusRectPolicy(widget)
                            != QMacStyle::FocusDisabled)
                        bdi.adornment |= kThemeAdornmentFocus;
                    if (tb->state & (QStyle::Style_On | QStyle::Style_Down))
                        bdi.value |= kThemeStatePressed;

                    QRect off_rct(0, 0, 0, 0);
                    HIRect myRect, macRect;
                    myRect = CGRectMake(tb->rect.x(), tb->rect.y(),
                                        tb->rect.width(), tb->rect.height());
                    HIThemeGetButtonBackgroundBounds(&myRect, &bdi, &macRect);
                    off_rct.setRect(int(myRect.origin.x - macRect.origin.x),
                                    int(myRect.origin.y - macRect.origin.y),
                                    int(macRect.size.width - myRect.size.width),
                                    int(macRect.size.height - myRect.size.height));

                    myRect = qt_hirectForQRect(button, p, false, off_rct);
                    HIThemeDrawButton(&myRect, &bdi, cg, kHIThemeOrientationNormal, 0);
                }
            }

            if (tb->subControls & QStyle::SC_ToolButtonMenu) {
                HIThemeButtonDrawInfo bdi;
                bdi.version = qt_mac_hitheme_version;
                bdi.state = tds;
                bdi.value = kThemeButtonOff;
                bdi.adornment = kThemeAdornmentNone;
                bdi.value = bkind;
                if (tb->state & QStyle::Style_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    bdi.adornment |= kThemeAdornmentFocus;
                if (tb->state & (QStyle::Style_On | QStyle::Style_Down)
                                 || (tb->activeSubControls & QStyle::SC_ToolButtonMenu))
                    bdi.value |= kThemeStatePressed;
                HIRect hirect = qt_hirectForQRect(menuarea, p, false);
                HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
                QRect r(menuarea.x() + ((menuarea.width() / 2) - 4), menuarea.height() - 8, 8, 8);
                HIThemePopupArrowDrawInfo padi;
                padi.version = qt_mac_hitheme_version;
                padi.state = tds;
                padi.orientation = kThemeArrowDown;
                padi.size = kThemeArrow7pt;
                hirect = qt_hirectForQRect(r, p);
                HIThemeDrawPopupArrow(&hirect, &padi, cg, kHIThemeOrientationNormal);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
#else
    q->QWindowsStyle::drawComplexControl(cc, opt, p, widget);
#endif
}

QStyle::SubControl QMacStylePrivate::HIThemeQuerySubControl(QStyle::ComplexControl cc,
                                                            const QStyleOptionComplex *opt,
                                                            const QPoint &pt,
                                                            const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QStyle::SubControl sc = QStyle::SC_None;
    switch (cc) {
    case QStyle::CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            ControlPartCode part;
            HIPoint pos = CGPointMake(pt.x(), pt.y());
            if (HIThemeHitTestTrack(&tdi, &pos, &part)) {
                if (part == kControlPageUpPart || part == kControlPageDownPart)
                    sc = QStyle::SC_SliderGroove;
                else
                    sc = QStyle::SC_SliderHandle;
            }
        }
        break;
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *sb = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIScrollBarTrackInfo sbi;
            sbi.version = qt_mac_hitheme_version;
            if (!(sb->state & QStyle::Style_Active))
                sbi.enableState = kThemeTrackInactive;
            else if (sb->state & QStyle::Style_Enabled)
                sbi.enableState = kThemeTrackDisabled;
            else
                sbi.enableState = kThemeTrackActive;
            sbi.viewsize = sb->pageStep;
            HIPoint pos = CGPointMake(pt.x(), pt.y());
            HIRect macSBRect = qt_hirectForQRect(sb->rect);
            ControlPartCode part;
            if (HIThemeHitTestScrollBarArrows(&macSBRect, &sbi, sb->orientation == Qt::Horizontal,
                        &pos, 0, &part)) {
                if (part == kControlUpButtonPart)
                    sc = QStyle::SC_ScrollBarSubLine;
                else if (part == kControlDownButtonPart)
                    sc = QStyle::SC_ScrollBarAddLine;
            } else {
                HIThemeTrackDrawInfo tdi;
                getSliderInfo(cc, sb, &tdi, widget);
                if (HIThemeHitTestTrack(&tdi, &pos, &part)) {
                    if (part == kControlPageUpPart)
                        sc = QStyle::SC_ScrollBarSubPage;
                    else if (part == kControlPageDownPart)
                        sc = QStyle::SC_ScrollBarAddPage;
                    else
                        sc = QStyle::SC_ScrollBarSlider;
                }
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            sc = q->QWindowsStyle::querySubControl(cc, cmb, pt, widget);
            if (!cmb->editable && sc != QStyle::SC_None)
                sc = QStyle::SC_ComboBoxArrow;  // A bit of a lie, but what we want
        }
        break;
/*
    I don't know why, but we only get kWindowContentRgn here, which isn't what we want at all.
    It would be very nice if this would work.
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            memset(&wdi, 0, sizeof(wdi));
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            wdi.titleWidth = tbar->rect.width();
            wdi.titleHeight = tbar->rect.height();
            if (tbar->titleBarState)
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            else if (tbar->titleBarFlags & Qt::WStyle_SysMenu)
                wdi.attributes |= kThemeWindowHasCloseBox;
            QRect tmpRect = tbar->rect;
            tmpRect.setHeight(tmpRect.height() + 100);
            HIRect hirect = qt_hirectForQRect(tmpRect);
            WindowRegionCode hit;
            HIPoint hipt = CGPointMake(pt.x(), pt.y());
            if (HIThemeGetWindowRegionHit(&hirect, &wdi, &hipt, &hit)) {
                switch (hit) {
                case kWindowCloseBoxRgn:
                    sc = QStyle::SC_TitleBarCloseButton;
                    break;
                case kWindowCollapseBoxRgn:
                    sc = QStyle::SC_TitleBarMinButton;
                    break;
                case kWindowZoomBoxRgn:
                    sc = QStyle::SC_TitleBarMaxButton;
                    break;
                case kWindowTitleTextRgn:
                    sc = QStyle::SC_TitleBarLabel;
                    break;
                default:
                    qDebug("got something else %d", hit);
                    break;
                }
            }
        }
        break;
*/
    default:
        sc = q->QWindowsStyle::querySubControl(cc, opt, pt, widget);
    }
    return sc;
#else
    return q->QWindowsStyle::querySubControl(cc, opt, pt, widget);
#endif
}

QRect QMacStylePrivate::HIThemeQuerySubControlMetrics(QStyle::ComplexControl cc,
                                                      const QStyleOptionComplex *opt,
                                                      QStyle::SubControl sc,
                                                      const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QRect ret;
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            HIRect macRect;
            QCFType<HIShapeRef> shape;
            bool scrollBar = cc == QStyle::CC_ScrollBar;
            if ((scrollBar && sc == QStyle::SC_ScrollBarSlider)
                    || (!scrollBar && sc == QStyle::SC_SliderHandle)) {
                HIThemeGetTrackThumbShape(&tdi, &shape);
                HIShapeGetBounds(shape, &macRect);
            } else if (!scrollBar && sc == QStyle::SC_SliderGroove) {
                HIThemeGetTrackBounds(&tdi, &macRect);
            } else if (sc == QStyle::SC_ScrollBarGroove) { // Only scrollbar parts available...
                HIThemeGetTrackDragRect(&tdi, &macRect);
            } else {
                ControlPartCode cpc;
                if (sc == QStyle::SC_ScrollBarSubPage || sc == QStyle::SC_ScrollBarAddPage)
                    cpc = sc == QStyle::SC_ScrollBarSubPage ? kControlPageDownPart
                                                            : kControlPageUpPart;
                else
                    cpc = sc == QStyle::SC_ScrollBarSubLine ? kControlUpButtonPart
                                                            : kControlDownButtonPart;
                HIThemeGetTrackPartBounds(&tdi, cpc, &macRect);
            }
            ret = qt_qrectForHIRect(macRect);
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 10,
            spinner_h = 15;
            int fw = q->pixelMetric(QStyle::PM_SpinBoxFrameWidth, spin, widget),
            y = fw,
            x = spin->rect.width() - fw - spinner_w;
            switch (sc) {
                case QStyle::SC_SpinBoxUp:
                    ret.setRect(x, y + ((spin->rect.height() - fw * 2) / 2 - spinner_h),
                                spinner_w, spinner_h);
                    break;
                case QStyle::SC_SpinBoxDown:
                    ret.setRect(x, y + (spin->rect.height() - fw * 2) / 2, spinner_w, spinner_h);
                    break;
                case QStyle::SC_SpinBoxButtonField:
                    ret.setRect(x, y, spinner_w, spin->rect.height() - fw * 2);
                    break;
                case QStyle::SC_SpinBoxEditField:
                    ret.setRect(fw, fw, spin->rect.width() - spinner_w - fw * 2 - macSpinBoxSep,
                                spin->rect.height() - fw * 2);
                    break;
                case QStyle::SC_SpinBoxFrame:
                    ret.setRect(0, 0, spin->rect.width() - spinner_w - macSpinBoxSep,
                                spin->rect.height());
                    break;
                default:
                    ret = q->QWindowsStyle::querySubControlMetrics(cc, spin, sc, widget);
                    break;
            }
        }
        break;
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *titlebar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            memset(&wdi, 0, sizeof(wdi));
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            wdi.titleHeight = titlebar->rect.height();
            wdi.titleWidth = titlebar->rect.width();
            wdi.attributes = kThemeWindowHasTitleText;
            if (titlebar->titleBarState)
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            else if (titlebar->titleBarFlags & Qt::WStyle_SysMenu)
                wdi.attributes |= kThemeWindowHasCloseBox;
            WindowRegionCode wrc = kWindowGlobalPortRgn;
            if (sc == QStyle::SC_TitleBarCloseButton)
                wrc = kWindowCloseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMinButton)
                wrc = kWindowCollapseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMaxButton)
                wrc = kWindowZoomBoxRgn;
            else if (sc == QStyle::SC_TitleBarLabel)
                wrc = kWindowTitleTextRgn;
            else if (sc == QStyle::SC_TitleBarSysMenu)
                ret.setRect(-1024, -1024, 10, q->pixelMetric(QStyle::PM_TitleBarHeight,
                                                             titlebar, widget));
            if (wrc != kWindowGlobalPortRgn) {
                QCFType<HIShapeRef> region;
                QRect tmpRect = titlebar->rect;
                HIRect titleRect = qt_hirectForQRect(tmpRect);
                HIThemeGetWindowShape(&titleRect, &wdi, kWindowTitleBarRgn, &region);
                HIShapeGetBounds(region, &titleRect);
                CFRelease(region);
                tmpRect.translate(tmpRect.x() - int(titleRect.origin.x),
                               tmpRect.y() - int(titleRect.origin.y));
                titleRect = qt_hirectForQRect(tmpRect);
                HIThemeGetWindowShape(&titleRect, &wdi, wrc, &region);
                HIShapeGetBounds(region, &titleRect);
                ret = qt_qrectForHIRect(titleRect);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *combo = qt_cast<const QStyleOptionComboBox *>(opt)) {
            if (sc == QStyle::SC_ComboBoxEditField && !combo->editable) {
                HIRect hirect, outrect;
                HIThemeButtonDrawInfo bdi;
                bdi.version = qt_mac_hitheme_version;
                bdi.state = kThemeStateActive;
                bdi.kind = kThemePopupButton;
                bdi.value = kThemeButtonOff;
                bdi.adornment = kThemeAdornmentArrowLeftArrow;
                hirect = qt_hirectForQRect(combo->rect);
                HIThemeGetButtonContentBounds(&hirect, &bdi, &outrect);
                ret = qt_qrectForHIRect(outrect);
            } else {
                ret = q->QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
            }
        }
        break;
    default:
        ret = q->QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
    }
    return ret;
#else
    return q->QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
#endif
}

int QMacStylePrivate::HIThemePixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                                         const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    SInt32 ret = 0;
    switch(metric) {
    case QStyle::PM_TitleBarHeight:
        if (const QStyleOptionTitleBar *tb = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            if (tb->titleBarState) {
                if (tb->titleBarState & Qt::WindowMinimized)
                    wdi.attributes |= kThemeWindowIsCollapsed;
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            } else if (tb->titleBarFlags & Qt::WStyle_SysMenu) {
                wdi.attributes |= kThemeWindowHasCloseBox;
            }
            wdi.titleHeight = tb->rect.height();
            wdi.titleWidth = tb->rect.width();
            QCFType<HIShapeRef> region;
            HIRect hirect = qt_hirectForQRect(tb->rect);
            HIThemeGetWindowShape(&hirect, &wdi, kWindowTitleBarRgn, &region);
            HIRect rect;
            HIShapeGetBounds(region, &rect);
            ret = int(rect.size.height);
        }
        break;
    default:
        ret = q->QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    }
    return ret;
#else
    return q->QWindowsStyle::pixelMetric(metric, opt, widget);
#endif
}

void QMacStylePrivate::HIThemeAdjustButtonSize(QStyle::ContentsType ct, QSize &sz,
                                               const QWidget *widget)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeButtonKind bkind = kThemePushButton;
    if (ct == QStyle::CT_ToolButton)
        bkind = kThemeBevelButton;
    if (qt_aqua_size_constrain(widget) == QAquaSizeSmall) {
        if(bkind == kThemeBevelButton)
            bkind = kThemeSmallBevelButton;
    }
    HIThemeButtonDrawInfo bdi;
    bdi.version = qt_mac_hitheme_version;
    bdi.state = kThemeStateActive;
    bdi.kind = bkind;
    bdi.value = kThemeButtonOff;
    bdi.adornment = kThemeAdornmentNone;
    HIRect macRect, myRect;
    myRect = CGRectMake(0, 0, sz.width(), sz.height());
    HIThemeGetButtonBackgroundBounds(&myRect, &bdi, &macRect);
    sz.setWidth(sz.width() + int(macRect.size.width - myRect.size.width));
    sz.setHeight(sz.height() + int(macRect.size.height - myRect.size.height));
#else
    Q_UNUSED(ct);
    Q_UNUSED(sz);
    Q_UNUSED(widget);
#endif
}

void QMacStylePrivate::AppManPolish(QWidget *w)
{
    addWidget(w);

#ifdef QMAC_DO_SECONDARY_GROUPBOXES
    if(w->parentWidget() && qt_cast<QGroupBox*>(w->parentWidget())
            && !w->testAttribute(Qt::WA_SetPalette)
            && w->parentWidget()->parentWidget()
            && qt_cast<QGroupBox*>(w->parentWidget()->parentWidget())) {
        QPalette pal = w->palette();
        QPixmap px(200, 200, 32);
        QColor pc(Qt::black);
        {
            QPainter p(&px);
            qt_mac_set_port(&p);
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

    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_2) {
        if(qt_cast<QGroupBox*>(w))
            w->setAttribute(Qt::WA_ContentsPropagated, true);
    }
    if(QLineEdit *lined = qt_cast<QLineEdit*>(w)) {
#if 0
        if(qt_cast<QComboBox*>(w->parentWidget()))
            lined->setFrameStyle(QFrame::LineEditPanel | QFrame::Sunken);
        SInt32 frame_size;
        GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
        lined->setLineWidth(frame_size);
#else
        Q_UNUSED(lined);
//# warning "Do we need to replace this with something else for the new QLineEdit? --Sam"
#endif
	/*
    } else if(QDialogButtons *btns = qt_cast<QDialogButtons*>(w)) {
        if(btns->buttonText(QDialogButtons::Help).isNull())
            btns->setButtonText(QDialogButtons::Help, "?");
	    */
    }
#ifndef QT_NO_MAINWINDOW
    else if(QToolBar *bar = qt_cast<QToolBar*>(w)) {
        QLayout *layout = bar->layout();
        layout->setSpacing(0);
        layout->setMargin(0);
    }
#endif
    else if(QRubberBand *rubber = qt_cast<QRubberBand*>(w)) {
        rubber->setWindowOpacity(0.25);
    } else if(QMenu *menu = qt_cast<QMenu*>(w)) {
        menu->setWindowOpacity(0.95);
    }
    q->QWindowsStyle::polish(w);
}

void QMacStylePrivate::AppManUnPolish(QWidget *w)
{
    removeWidget(w);
    if(QRubberBand *rubber = qt_cast<QRubberBand*>(w))
        rubber->setWindowOpacity(1.0);
    else if(qt_cast<QMenu*>(w))
        w->setWindowOpacity(1.0);
}

void QMacStylePrivate::AppManPolish(QApplication *app)
{
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QColor pc(Qt::black);
    {
        QPainter p(&px);
        qt_mac_set_port(&p);
        SetThemeBackground(kThemeBrushDialogBackgroundActive, px.depth(), true);
        EraseRect(qt_glb_mac_rect(QRect(0, 0, px.width(), px.height()),
                  static_cast<QPaintDevice *>(0), false));
        RGBColor c;
        GetThemeBrushAsColor(kThemeBrushDialogBackgroundActive, 32, true, &c);
        pc = QColor(c.red / 256, c.green / 256, c.blue / 256);
    }
    QBrush background(pc, px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
    app->setPalette(pal);
}

void QMacStylePrivate::AppManDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt,
                                           QPainter *p, const QWidget *w) const
{
    ThemeDrawState tds = getDrawState(opt->state);
    switch (pe) {
    case QStyle::PE_Q3CheckListExclusiveIndicator:
    case QStyle::PE_Q3CheckListIndicator:
    case QStyle::PE_IndicatorRadioButtonMask:
    case QStyle::PE_IndicatorRadioButton:
    case QStyle::PE_IndicatorCheckBoxMask:
    case QStyle::PE_IndicatorCheckBox: {
        bool isRadioButton = (pe == QStyle::PE_Q3CheckListIndicator
                || pe == QStyle::PE_IndicatorRadioButton
                || pe == QStyle::PE_IndicatorRadioButtonMask);
        ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentDrawIndicatorOnly };
        if (opt->state & QStyle::Style_HasFocus
                && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
            info.adornment |= kThemeAdornmentFocus;
        if (opt->state & QStyle::Style_NoChange)
            info.value = kThemeButtonMixed;
        else if (opt->state & QStyle::Style_On)
            info.value = kThemeButtonOn;
        ThemeButtonKind bkind;
        switch (qt_mac_get_size_for_painter(p)) {
            default:
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                if (isRadioButton)
                    bkind = kThemeRadioButton;
                else
                    bkind = kThemeCheckBox;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
        if (pe == QStyle::PE_IndicatorRadioButtonMask || pe == QStyle::PE_IndicatorCheckBoxMask) {
            p->save();
            RgnHandle rgn = qt_mac_get_rgn();
            GetThemeButtonRegion(qt_glb_mac_rect(opt->rect, p, false), bkind, &info, rgn);
            p->setClipRegion(qt_mac_convert_mac_region(rgn));
            qt_mac_dispose_rgn(rgn);
            p->fillRect(opt->rect, Qt::color1);
            p->restore();
        } else {
            qt_mac_set_port(p);
            DrawThemeButton(qt_glb_mac_rect(opt->rect, p, false), bkind, &info, 0, 0, 0, 0);
        }
        break; }
    case QStyle::PE_FrameFocusRect:
        break;     //This is not used because of the QAquaFocusWidget thingie..
    case QStyle::PE_IndicatorBranch:
        if (!(opt->state & QStyle::Style_Children))
            break;
        ThemeButtonDrawInfo currentInfo;
        currentInfo.state = opt->state & QStyle::Style_Enabled ? kThemeStateActive
                                                               : kThemeStateInactive;
        if (opt->state & QStyle::Style_Down)
            currentInfo.state |= kThemeStatePressed;
        currentInfo.value = opt->state & QStyle::Style_Open ? kThemeDisclosureDown
                                                            : kThemeDisclosureRight;
        currentInfo.adornment = kThemeAdornmentNone;
        qt_mac_set_port(p);
        DrawThemeButton(qt_glb_mac_rect(opt->rect, p), kThemeDisclosureButton, &currentInfo,
                        0, 0, 0, 0);
        break;
    case QStyle::PE_IndicatorHeaderArrow:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            if (w && (qt_cast<QTreeView *>(w->parentWidget())
#ifdef QT_COMPAT
			|| w->parentWidget()->inherits("Q3ListView")
#endif
		))
		break; // ListView-type header is taken care of.
	    q->drawPrimitive(header->state & QStyle::Style_Up ? QStyle::PE_IndicatorArrowUp
                                                              : QStyle::PE_IndicatorArrowDown, header, p, w);
        }
        break;
    case QStyle::PE_PanelHeader:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            ThemeButtonKind bkind;
	    QStyle::StyleFlags flags = header->state;
            QRect ir = header->rect;
            bool scaleHeader = false;
            SInt32 headerHeight = 0;
            if (w && (qt_cast<QTreeView *>(w->parentWidget())
#ifdef QT_COMPAT
			|| w->parentWidget()->inherits("Q3ListView")
#endif
		)) {
		bkind = kThemeListHeaderButton;
                GetThemeMetric(kThemeMetricListHeaderHeight, &headerHeight);
                if (ir.height() > headerHeight)
                    scaleHeader = true;
            } else {
                bkind = kThemeBevelButton;
                if (p->font().bold())
                    flags |= QStyle::Style_Sunken;
                else
                    flags &= ~QStyle::Style_Sunken;
	    }
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            QWidget *w = 0;

            if (flags & QStyle::Style_HasFocus
                    && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                info.adornment |= kThemeAdornmentFocus;
            if (flags & QStyle::Style_Active) {
                if (!(flags & QStyle::Style_Enabled))
                    info.state = kThemeStateUnavailable;
                else if (flags & QStyle::Style_Down)
                    info.state = kThemeStatePressed;
            } else {
                if (flags & QStyle::Style_Enabled)
                    info.state = kThemeStateInactive;
                else
                    info.state = kThemeStateUnavailableInactive;
            }
            if (flags & QStyle::Style_Sunken)
                info.value = kThemeButtonOn;

            if (flags & QStyle::Style_Off)
                ir.setRight(ir.right() + 50);
            else if (flags & QStyle::Style_Up)
                info.adornment |= kThemeAdornmentHeaderButtonSortUp;
	    if (scaleHeader) {
		QPixmap headerPix(ir.width(), headerHeight);
		QPainter pixPainter(&headerPix);
		Rect pixRect = *qt_glb_mac_rect(QRect(0, 0, ir.width(), headerHeight),
						&pixPainter, false);
                qt_mac_set_port(&pixPainter);
		DrawThemeButton(&pixRect, bkind, &info, 0, 0, 0, 0);
		p->drawPixmap(ir, headerPix);
	    } else {
                qt_mac_set_port(p);
		DrawThemeButton(qt_glb_mac_rect(ir, p, false), bkind, &info, 0, 0, 0, 0);
	    }
        }
        break;
    case QStyle::PE_Frame:
    case QStyle::PE_FrameLineEdit:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            if (opt->state & QStyle::Style_Sunken) {
                SInt32 frame_size;
                QColor baseColor(frame->palette.background().color());
                if (pe == QStyle::PE_FrameLineEdit) {
                    GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
                } else {
                    baseColor = QColor(150, 150, 150); //hardcoded since no query function --Sam
                    GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
                }

                int lw = frame->lineWidth;
                if (lw <= 0)
                    q->pixelMetric(QStyle::PM_DefaultFrameWidth, frame, w);
                { //clear to base color
                    p->save();
                    p->setPen(QPen(baseColor, lw));
                    p->setBrush(Qt::NoBrush);
                    p->drawRect(frame->rect);
                    p->restore();
                }

                const Rect *rect = qt_glb_mac_rect(frame->rect, p, false,
                                                   QRect(frame_size, frame_size, frame_size * 2,
                                                         frame_size * 2));
                qt_mac_set_port(p);
                if (pe == QStyle::PE_FrameLineEdit)
                    DrawThemeEditTextFrame(rect, tds);
                else
                    DrawThemeListBoxFrame(rect, tds);
            } else {
                q->QWindowsStyle::drawPrimitive(pe, frame, p, w);
            }
        }
        break;
    case QStyle::PE_FrameGroupBox:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            qt_mac_set_port(p);
#ifdef QMAC_DO_SECONDARY_GROUPBOXES
            if (w && qt_cast<QGroupBox *>(w->parentWidget()))
                DrawThemeSecondaryGroup(qt_glb_mac_rect(frame->rect, p), kThemeStateActive);
            else
#endif
                DrawThemePrimaryGroup(qt_glb_mac_rect(frame->rect, p), kThemeStateActive);
        }
        break;
    case QStyle::PE_IndicatorArrowUp:
    case QStyle::PE_IndicatorArrowDown:
    case QStyle::PE_IndicatorArrowRight:
    case QStyle::PE_IndicatorArrowLeft: {
        ThemeArrowOrientation orientation;
        switch (pe) {
        case QStyle::PE_IndicatorArrowUp:
            orientation = kThemeArrowUp;
            break;
        case QStyle::PE_IndicatorArrowDown:
            orientation = kThemeArrowDown;
            break;
        case QStyle::PE_IndicatorArrowRight:
            orientation = kThemeArrowRight;
            break;
        default:
        case QStyle::PE_IndicatorArrowLeft:
            orientation = kThemeArrowLeft;
            break;
        }
        ThemePopupArrowSize size;
        if (opt->rect.width() < 8)
            size = kThemeArrow5pt;
        else
            size = kThemeArrow9pt;
        qt_mac_set_port(p);
        DrawThemePopupArrow(qt_glb_mac_rect(opt->rect, p), orientation, size, tds, 0, 0);
        break; }
    case QStyle::PE_FrameTabBarBase:
        break;
    case QStyle::PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf
                = qt_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            int baseHeight = q->pixelMetric(QStyle::PM_TabBarBaseHeight, twf, w);
            int overlap = q->pixelMetric(QStyle::PM_TabBarBaseOverlap, twf, w);
            QRect wholePane = twf->rect;
            ThemeTabDirection ttd = getTabDirection(twf->shape);
            if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3) {
                // This tab pane with Appearance Manager is a real pain (pun not intended).
                // First, it can't handle drawing tabs at different positions.
                // Second, it draws outside my rectangle AND provides to function to return the area.
                // Third, it connects with the base, so I need to draw everything.
                // So, I must "guess" the size of the panel and do some transformations to make it work.
                const int TabPaneShadowWidth = 2; // The offset where we really start drawing the pane.
                const int TabPaneShadowHeight = 10; // The amount of pixels to for the drop shadow
                // we need to draw the whole thing, so add in the height.
                if (ttd != kThemeTabNorth) {
                    p->save();
                    int newX, newY, newRot;
                    if (twf->shape == QTabBar::RoundedWest || twf->shape == QTabBar::RoundedEast
                        || twf->shape == QTabBar::TriangularWest
                        || twf->shape == QTabBar::TriangularWest) {
                        wholePane.setRect(wholePane.left() + overlap - baseHeight, wholePane.y(),
                                          wholePane.height(), wholePane.width());
                        wholePane.setWidth(baseHeight + wholePane.width());
                    } else {
                        wholePane.setHeight(baseHeight + wholePane.height() - overlap - 1);
                    }
                    QRect finalRect = wholePane;
                    QSize pixSize;
                    switch (ttd) {
                    default:
                        break;
                    case kThemeTabSouth:
                        newX = wholePane.x() + wholePane.width();
                        newY = wholePane.y() + wholePane.height();
                        newRot = 180;
                        wholePane.setRect(TabPaneShadowWidth, 0,
                                          wholePane.width() - 2 * TabPaneShadowWidth,
                                          wholePane.height());
                        pixSize = wholePane.size();
                        break;
                    case kThemeTabWest:
                        newX = wholePane.x();
                        newY = wholePane.height();
                        newRot = -90;
                        wholePane.setRect(TabPaneShadowWidth, 0,
                                          wholePane.height() - 2 * TabPaneShadowWidth,
                                          wholePane.width());
                        pixSize = wholePane.size();
                        break;
                    case kThemeTabEast:
                        newX = wholePane.width();
                        newY = wholePane.y();
                        newRot = 90;
                        wholePane.setRect(TabPaneShadowWidth, 0,
                                          wholePane.height() - 2 * TabPaneShadowWidth,
                                          wholePane.width());
                        pixSize = wholePane.size();
                        break;
                    }
                    QPixmap pix(pixSize, 32);
                    QPainter pixPainter(&pix);
                    qt_mac_set_port(&pixPainter);
                    Rect macRect;
                    SetRect(&macRect, 0, 0, pix.width(), pix.height());
                    ApplyThemeBackground(kThemeBackgroundTabPane, &macRect, tds, 32, true);
                    EraseRect(&macRect);
                    DrawThemeTabPane(qt_glb_mac_rect(wholePane, &pixPainter), tds);
                    QMatrix m;
                    m.translate(newX, newY);
                    m.rotate(newRot);
                    p->setMatrix(m);
                    p->drawPixmap(wholePane, pix);
                    p->restore();
                } else {
                    wholePane.setTop(wholePane.top() + overlap - baseHeight);
                    wholePane.setLeft(wholePane.left() + TabPaneShadowWidth);
                    wholePane.setWidth(wholePane.width() - 2 * TabPaneShadowWidth);
                    wholePane.setHeight(baseHeight + wholePane.height() - TabPaneShadowHeight);
                    qt_mac_set_port(p);
                    DrawThemeTabPane(qt_glb_mac_rect(wholePane, p), tds);
                }
            } else {
                switch (ttd) {
                case kThemeTabSouth:
                    wholePane.setHeight(wholePane.height() + 7);
                    break;
                case kThemeTabNorth:
                    wholePane.setTop(wholePane.top() - 6);
                    wholePane.setHeight(wholePane.height() + 3);
                    break;
                case kThemeTabWest:
                    wholePane.setLeft(wholePane.left() - 6);
                    wholePane.setWidth(wholePane.width());
                    break;
                case kThemeTabEast:
                    wholePane.setWidth(wholePane.width() + 4);
                    break;
                }
                DrawThemeSecondaryGroup(qt_glb_mac_rect(wholePane, p), tds);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
}

void qt_mac_draw_tab(QPainter *p, const QWidget *w, const QRect &ir, ThemeTabStyle tts,
                     ThemeTabDirection ttd)
{
    if (ir.height() > kThemeLargeTabHeightMax) {
        QPixmap tabPix(ir.width(), kThemeLargeTabHeightMax);
        QPainter pixPainter(&tabPix);
        if (w)
            pixPainter.fillRect(QRect(QPoint(0, 0), tabPix.size()),
                                w->palette().brush(w->backgroundRole()));
        else
            tabPix.fill(QColor(255, 255, 255, 255));

        Rect pixRect = *qt_glb_mac_rect(QRect(0, 0, ir.width(), kThemeLargeTabHeightMax),
                                        &pixPainter, false);
        qt_mac_set_port(&pixPainter);
        DrawThemeTab(&pixRect, tts, ttd, 0, 0);
        p->drawPixmap(ir, tabPix);
    } else {
        qt_mac_set_port(p);
        DrawThemeTab(qt_glb_mac_rect(ir, p, false), tts, ttd, 0, 0);
    }
}

void QMacStylePrivate::AppManDrawControl(QStyle::ControlElement ce, const QStyleOption *opt,
                                         QPainter *p, const QWidget *widget) const
{
    ThemeDrawState tds = getDrawState(opt->state);
    switch (ce) {
    case QStyle::CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            if (!(btn->state & (QStyle::Style_Raised | QStyle::Style_Down | QStyle::Style_On)))
                break;
            QString pmkey;
            bool do_draw = false;
            QPixmap buffer;
            bool darken = animatable(AquaPushButton, widget);
            int frame = buttonState.frame;
            if (btn->state & QStyle::Style_On) {
                darken = true;
                frame = 12;
                if (btn->state & QStyle::Style_Down)
                    frame += 8;
            } else if (btn->state & QStyle::Style_Down) {
                darken = false;
                frame = 0;
            }
            if (darken && (btn->state & QStyle::Style_Active)) {
                QTextOStream os(&pmkey);
                os << "$qt_mac_pshbtn_" << opt->rect.width() << "x" << opt->rect.height() << "_"
                   << opt->state << "_" << frame;
                tds = kThemeStatePressed;
                if(frame && !QPixmapCache::find(pmkey, buffer)) {
                    do_draw = true;
                    buffer = QPixmap(opt->rect.width(), opt->rect.height(), 32);
                    buffer.fill(Qt::color0);
                }
                if (timerID <= -1) {
                    QTimer::singleShot(0, const_cast<QMacStylePrivate *>(this),
                                       SLOT(startAnimationTimer()));
                }
            }
            ThemeButtonKind bkind;
            if ((btn->features & (QStyleOptionButton::Flat | QStyleOptionButton::HasMenu)))
                bkind = kThemeBevelButton;
            else
                bkind = kThemePushButton;
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
            if (opt->state & QStyle::Style_HasFocus
                    && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled) {
                info.adornment |= kThemeAdornmentFocus;
            }
            QRect off_rct;
            { //The AppManager draws outside my rectangle, so account for that difference..
                Rect macRect, myRect;
                SetRect(&myRect, btn->rect.x(), btn->rect.y(), btn->rect.width(),
                        btn->rect.height());
                GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                off_rct.setRect(myRect.left - macRect.left, myRect.top - macRect.top,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
            }
            qt_mac_set_port(p);
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
                    QImage img = buffer.toImage(), maskimg = buffer_mask.toImage();
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
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbi = q->pixelMetric(QStyle::PM_MenuButtonIndicator, btn, widget);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() / 2 - 5, mbi, ir.height() / 2);
                q->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, p, widget);
            }
        }
        break;
    case QStyle::CE_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            bool dis = !(mi->state & QStyle::Style_Enabled);
            int tab = mi->tabWidth;
            int maxpmw = mi->maxIconWidth;
            bool checkable = mi->checkType != QStyleOptionMenuItem::NotCheckable;
            bool act = mi->state & QStyle::Style_Selected;
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
            qt_mac_set_port(p);
            if (mi->menuItemType != QStyleOptionMenuItem::Separator) {
                DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);
            } else {
                DrawThemeMenuSeparator(&irect);
                break;
            }

            int x, y, w, h;
            mi->rect.getRect(&x, &y, &w, &h);
            int checkcol = maxpmw;
            bool reverse = QApplication::isRightToLeft();
            int xpos = x + 18;
            if (reverse)
                xpos += w - checkcol;

            if (dis)
                p->setPen(mi->palette.text().color());
            else if (act)
                p->setPen(mi->palette.highlightedText().color());
            else
                p->setPen(mi->palette.buttonText().color());

            if (mi->checkType != QStyleOptionMenuItem::NotCheckable && mi->checked) {
                ThemeDrawState menuTDS = tds;
                if (act)
                    menuTDS = kThemeStatePressed;
                int xp = x;
                QCFString checkmark;
                if (mi->checkType == QStyleOptionMenuItem::Exclusive)
                    checkmark = QString(QChar(kDiamondUnicode));
                else
                    checkmark = QString(QChar(kCheckUnicode));
                Point macpt;
                SInt16 macbaseline;
                GetThemeTextDimensions(checkmark, kThemeMenuItemMarkFont, menuTDS, false, &macpt,
                                       &macbaseline);
                xp += macItemFrame;
                int mw = checkcol + macItemFrame;
                int mh = h - 2 * macItemFrame;
                QRect r(xp, y + macItemFrame, mw, mh);
                int translate = p->fontMetrics().ascent() - macbaseline + 1;
                if (macbaseline)
                    translate = p->fontMetrics().ascent() - macbaseline + 1;
                else
                    translate = 3;
                r.translate(0, translate);
                DrawThemeTextBox(checkmark, kThemeMenuItemMarkFont, menuTDS, false,
                                 qt_glb_mac_rect(r, p), teFlushDefault, 0);
            }
            if (!mi->icon.isNull()) {              // draw icon
                // Always be normal or disabled to follow the Mac style.
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                QPixmap pixmap;
                pixmap = mi->icon.pixmap(Qt::SmallIconSize, mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(xpos, y, checkcol, h);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }
            int xm = macItemFrame + checkcol + macItemHMargin;
            /*
            if (reverse)
                xpos = macItemFrame + tab;
            else
                xpos += xm;
                */
            QString s = mi->text;
            if (!s.isEmpty()) {                        // draw text
                int t = s.indexOf('\t');
                int m = macItemVMargin;
                int text_flags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextHideMnemonic
                                 | Qt::TextSingleLine;
                p->save();
                if (t >= 0) {                         // draw tab text
                    int xp;
                    extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp
                    p->setFont(qt_app_fonts_hash()->value("QMenuItem", p->font()));
                    if (reverse)
                        xp = x + macRightBorder + macItemHMargin + macItemFrame - 1;
                    else
                        xp = x + w - tab - macRightBorder - macItemHMargin - macItemFrame + 1;
                    p->drawText(xp, y + m, tab, h - 2 * m, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                text_flags ^= Qt::AlignRight;
                p->setFont(mi->font);
                p->drawText(xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s);
                p->restore();
            }
        }
        break;
    case QStyle::CE_MenuHMargin:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
                 irect = *qt_glb_mac_rect(mi->rect, p, false);
            ThemeMenuState tms = kThemeMenuActive;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if (opt->state & QStyle::Style_Selected)
                tms |= kThemeMenuSelected;
            if (ce == QStyle::CE_MenuScroller) {
                if (opt->state & QStyle::Style_Down)
                    tmit = kThemeMenuItemScrollDownArrow;
                else
                    tmit = kThemeMenuItemScrollUpArrow;
            }
            qt_mac_set_port(p);
            DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);
            if (ce == QStyle::CE_MenuTearoff) {
                p->setPen(QPen(mi->palette.dark().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2 - 1);
                p->setPen(QPen(mi->palette.light().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2);
            }
        }
        break;
    case QStyle::CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
                 irect = *qt_glb_mac_rect(mi->rect, p, false);
            ThemeMenuState tms = kThemeMenuActive;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if ((opt->state & QStyle::Style_Selected) && (opt->state & QStyle::Style_Down))
                tms |= kThemeMenuSelected;
            qt_mac_set_port(p);
            DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);

            //text
            q->drawItem(p, mi->rect,
                        Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip | Qt::TextSingleLine,
                        mi->palette, mi->state & QStyle::Style_Enabled,
                        mi->icon.pixmap(Qt::SmallIconSize, QIcon::Normal),
                        mi->text, &mi->palette.buttonText().color());
        }
        break;
    case QStyle::CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            qt_mac_set_port(p);
            DrawThemeMenuBarBackground(qt_glb_mac_rect(mi->rect, p, false), kThemeMenuBarNormal,
                                       kThemeMenuSquareMenuBar);
        }
        break;
    case QStyle::CE_ProgressBarGroove:
    case QStyle::CE_ProgressBarLabel:
        break;
    case QStyle::CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            ThemeTrackDrawInfo tdi;
            tdi.filler1 = 0;
            bool isIndeterminate = (pb->minimum == 0 && pb->maximum == 0);
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                tdi.kind = !isIndeterminate ? kThemeLargeProgressBar : kThemeLargeIndeterminateBar;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    tdi.kind = !isIndeterminate ? kThemeMiniProgressBar : kThemeMiniIndeterminateBar;
                    break;
                }
#endif
            case QAquaSizeSmall:
                tdi.kind = !isIndeterminate ? kThemeProgressBar : kThemeIndeterminateBar;
                break;
            }
            tdi.bounds = *qt_glb_mac_rect(opt->rect, p);
            tdi.max = pb->maximum;
            tdi.min = pb->minimum;
            tdi.value = pb->progress;
            tdi.attributes = kThemeTrackHorizontal;
            tdi.trackInfo.progress.phase = progressFrame;
            if (!(pb->state & QStyle::Style_Active))
                tdi.enableState = kThemeTrackInactive;
            else if (!(pb->state & QStyle::Style_Enabled))
                tdi.enableState = kThemeTrackDisabled;
            else
                tdi.enableState = kThemeTrackActive;
            qt_mac_set_port(p);
            DrawThemeTrack(&tdi, 0, 0, 0);
        }
        break;
    case QStyle::CE_TabBarTab:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            ThemeTabStyle tts = kThemeTabNonFront;
            if (tab->state & QStyle::Style_Selected) {
                if (!(tab->state & QStyle::Style_Active))
                    tts = kThemeTabFrontUnavailable;
                else if (!(tab->state & QStyle::Style_Enabled))
                    tts = kThemeTabFrontInactive;
                else
                    tts = kThemeTabFront;
            } else if (!(tab->state  &QStyle::Style_Active)) {
                tts = kThemeTabNonFrontUnavailable;
            } else if (!(tab->state & QStyle::Style_Enabled)) {
                tts = kThemeTabNonFrontInactive;
            } else if ((tab->state & (QStyle::Style_Sunken | QStyle::Style_MouseOver))
                       == (QStyle::Style_Sunken | QStyle::Style_MouseOver)) {
                tts = kThemeTabNonFrontPressed;
            }
            ThemeTabDirection ttd = getTabDirection(tab->shape);
            QRect tabr(tab->rect.x(), tab->rect.y(), tab->rect.width(),
                       tab->rect.height() + q->pixelMetric(QStyle::PM_TabBarBaseOverlap, tab,
                                                           widget));
            if (ttd == kThemeTabSouth)
                tabr.translate(0, -q->pixelMetric(QStyle::PM_TabBarBaseOverlap, tab, widget));
            qt_mac_draw_tab(p, widget, tabr, tts, ttd);
        }
        break;
    case QStyle::CE_SizeGrip: {
        const Rect *rect = qt_glb_mac_rect(opt->rect, p);
        Point orig = { rect->top, rect->left };
        qt_mac_set_port(p);
        ThemeGrowDirection dir = kThemeGrowRight | kThemeGrowDown;
#if 0
        if(QApplication::isRightToLeft())
            dir = kThemeGrowLeft | kThemeGrowDown;
#endif
        DrawThemeStandaloneGrowBox(orig, dir, false, kThemeStateActive);
        break; }
    case QStyle::CE_RubberBand:
        p->fillRect(opt->rect, opt->palette.brush(QPalette::Disabled, QPalette::Highlight));
        break;
    default:
        q->QWindowsStyle::drawControl(ce, opt, p, widget);
    }
}

QRect QMacStylePrivate::AppManSubRect(QStyle::SubRect sr, const QStyleOption *opt,
                                      const QWidget *widget) const
{
    QRect r = QRect();
    switch (sr) {
    case QStyle::SR_PushButtonContents:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            Rect macRect, myRect;
            SetRect(&myRect, btn->rect.left(), btn->rect.top(), btn->rect.right(), btn->rect.bottom());
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            GetThemeButtonContentBounds(&myRect, kThemePushButton, &info, &macRect);
            r = QRect(macRect.left, macRect.top - 2,
                      qMin(qAbs(btn->rect.width() - 2 * macRect.left),
                           macRect.right - macRect.left),
                      qMin(qAbs(btn->rect.height() - 2 * macRect.top),
                           macRect.bottom - macRect.top));
        }
        break;
    case QStyle::SR_ProgressBarContents:
        r = opt->rect;
        break;
    case QStyle::SR_ProgressBarGroove:
    case QStyle::SR_ProgressBarLabel:
        break;
    default:
        r = q->QWindowsStyle::subRect(sr, opt, widget);
        break;
    }
    return r;
}

void QMacStylePrivate::AppManDrawComplexControl(QStyle::ComplexControl cc,
                                                const QStyleOptionComplex *opt, QPainter *p,
                                                const QWidget *widget) const
{
    ThemeDrawState tds = getDrawState(opt->state);
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, p, &tdi, widget);
            if (cc == QStyle::CC_Slider) {
                if (slider->activeSubControls == QStyle::SC_SliderGroove)
                    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_SliderHandle)
                    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
            } else {
                if (slider->activeSubControls == QStyle::SC_ScrollBarSubLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                         | kThemeLeftOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                         | kThemeRightOutsideArrowPressed;
                else if(slider->activeSubControls == QStyle::SC_ScrollBarAddPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
                else if(slider->activeSubControls == QStyle::SC_ScrollBarSubPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
                else if(slider->activeSubControls == QStyle::SC_ScrollBarSlider)
                    tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            }

            //The AppManager draws outside my rectangle, so account for that difference..
            Rect macRect;
            GetThemeTrackBounds(&tdi, &macRect);
            tdi.bounds.left  += tdi.bounds.left  - macRect.left;
            tdi.bounds.right -= macRect.right - tdi.bounds.right;

            bool tracking = slider->sliderPosition == slider->sliderValue;
            RgnHandle r = 0;
            if (!tracking) {
                r = qt_mac_get_rgn();
                GetThemeTrackThumbRgn(&tdi, r);
                tdi.value = slider->sliderValue;
            }

            qt_mac_set_port(p);
            DrawThemeTrack(&tdi, r, 0, 0);
            if (!tracking)
                qt_mac_dispose_rgn(r);
            if (slider->subControls & QStyle::SC_SliderTickmarks) {
                int numTicks;
                if (slider->tickInterval)
                        numTicks = ((slider->maximum - slider->minimum + 1)
					/ slider->tickInterval) + 1;
                else
                    numTicks = ((slider->maximum - slider->minimum + 1) / slider->pageStep) + 1;
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
    case QStyle::CC_ListView:
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if (lv->subControls & QStyle::SC_ListView)
                q->QWindowsStyle::drawComplexControl(cc, lv, p, widget);

            if (lv->subControls & (QStyle::SC_ListViewBranch | QStyle::SC_ListViewExpand)) {
                int y = lv->rect.y(),
                h = lv->rect.height(),
                x = lv->rect.right() - 10;
                for (int i = 1; i < lv->items.size() && y < h; ++i) {
                    QStyleOptionListViewItem child = lv->items.at(i);
                    if (y + child.height > 0 && (child.childCount > 0
                        || (child.features & (QStyleOptionListViewItem::Expandable
                                            | QStyleOptionListViewItem::Visible))
                            == (QStyleOptionListViewItem::Expandable
                                | QStyleOptionListViewItem::Visible))) {
                        QStyleOption treeOpt(0);
                        treeOpt.rect.setRect(x, y + child.height / 2 - 4, 9, 9);
                        treeOpt.palette = lv->palette;
                        treeOpt.state = lv->state;
                        treeOpt.state |= QStyle::Style_Children;
                        if (child.state & QStyle::Style_Open)
                            treeOpt.state |= QStyle::Style_Open;
                        q->drawPrimitive(QStyle::PE_IndicatorBranch, &treeOpt, p, widget);
                    }
                    y += child.totalHeight;
                }
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox newSB = *sb;
            if (sb->subControls & QStyle::SC_SpinBoxFrame) {
                QStyleOptionFrame lineedit;
                lineedit.rect = q->querySubControlMetrics(QStyle::CC_SpinBox, sb,
                                                          QStyle::SC_SpinBoxFrame, widget),
                lineedit.palette = sb->palette;
                lineedit.state = QStyle::Style_Sunken;
                lineedit.lineWidth = 0;
                lineedit.midLineWidth = 0;
                q->drawPrimitive(QStyle::PE_FrameLineEdit, &lineedit, p, widget);
            }
            if (sb->subControls & (QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxUp)) {
                if (!(sb->stepEnabled & (QAbstractSpinBox::StepUpEnabled
                                        | QAbstractSpinBox::StepDownEnabled)))
                    tds = kThemeStateUnavailable;
                if (sb->activeSubControls == QStyle::SC_SpinBoxDown)
                    tds = kThemeStatePressedDown;
                else if (sb->activeSubControls == QStyle::SC_SpinBoxUp)
                    tds = kThemeStatePressedUp;
                ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                if (sb->state & QStyle::Style_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    info.adornment |= kThemeAdornmentFocus;
                QRect updown = q->visualRect(opt->direction, opt->rect,
                                             q->querySubControlMetrics(QStyle::CC_SpinBox, sb,
                                                                       QStyle::SC_SpinBoxUp,
                                                                       widget));
                updown |= q->visualRect(opt->direction, opt->rect,
                                        q->querySubControlMetrics(QStyle::CC_SpinBox, sb,
                                                                  QStyle::SC_SpinBoxDown,
                                                                  widget));
                if (widget) {
                    QPalette::ColorRole bgRole = widget->backgroundRole();
                    QPixmap pm = sb->palette.brush(bgRole).texture();
                    if (!pm.isNull())
                        p->drawPixmap(updown, pm);
                    else
                        p->fillRect(updown, sb->palette.color(bgRole));
                }
                qt_mac_set_port(p);
                ThemeButtonKind kind = kThemeIncDecButton;
                switch (qt_aqua_size_constrain(widget)) {
                case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                        kind = kThemeIncDecButtonMini;
                        break;
                    }
#endif
                case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
    case QStyle::CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            ThemeButtonKind bkind = kThemeBevelButton;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bkind = kThemeBevelButton;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    bkind = kThemeMiniBevelButton;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bkind = kThemeSmallBevelButton;
                break;
            }

            QRect button, menuarea;
            button   = q->querySubControlMetrics(cc, tb, QStyle::SC_ToolButton, widget);
            menuarea = q->querySubControlMetrics(cc, tb, QStyle::SC_ToolButtonMenu, widget);
	    QStyle::StyleFlags bflags = tb->state,
            mflags = tb->state;
            if (tb->activeSubControls & QStyle::SC_ToolButton)
                bflags |= QStyle::Style_Down;
            if (tb->activeSubControls & QStyle::SC_ToolButtonMenu)
                mflags |= QStyle::Style_Down;

            if (tb->subControls & QStyle::SC_ToolButton) {
                if(bflags & (QStyle::Style_Down | QStyle::Style_On | QStyle::Style_Raised)) {
                    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                    if (tb->state & QStyle::Style_HasFocus && QMacStyle::focusRectPolicy(widget)
                            != QMacStyle::FocusDisabled)
                        info.adornment |= kThemeAdornmentFocus;
                    if (tb->state & (QStyle::Style_On | QStyle::Style_Down))
                        info.value |= kThemeStatePressed;

                    QRect off_rct(0, 0, 0, 0);
                    { //The AppManager draws outside my rectangle, so account for that difference..
                        Rect macRect, myRect;
                        SetRect(&myRect, tb->rect.x(), tb->rect.y(), tb->rect.width(),
                                tb->rect.height());
                        GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                        off_rct = QRect(myRect.left - macRect.left, myRect.top - macRect.top,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
                    }

                    qt_mac_set_port(p);
                    DrawThemeButton(qt_glb_mac_rect(button, p, false, off_rct),
                                    bkind, &info, 0, 0, 0, 0);
                }
            }

            if (tb->subControls & QStyle::SC_ToolButtonMenu) {
                ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                if (tb->state & QStyle::Style_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    info.adornment |= kThemeAdornmentFocus;
                if (tb->state & (QStyle::Style_On | QStyle::Style_Down)
                        || (tb->activeSubControls & QStyle::SC_ToolButtonMenu))
                    info.value |= kThemeStatePressed;
                qt_mac_set_port(p);
                DrawThemeButton(qt_glb_mac_rect(menuarea, p, false), bkind, &info, 0, 0, 0, 0);
                QRect r(menuarea.x() + ((menuarea.width() / 2) - 4), menuarea.height() - 8, 8, 8);
                DrawThemePopupArrow(qt_glb_mac_rect(r, p), kThemeArrowDown, kThemeArrow7pt, tds,
                                    0, 0);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt) ) {
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
            if (cmb->state & QStyle::Style_HasFocus)
                info.adornment |= kThemeAdornmentFocus;
            if (cmb->activeSubControls & QStyle::SC_ComboBoxArrow)
                info.state = kThemeStatePressed;
            p->fillRect(cmb->rect, cmb->palette.brush(QPalette::Button)); //make sure it is filled
            if (cmb->editable) {
                info.adornment |= kThemeAdornmentArrowDownArrow;
                QRect buttonR = q->querySubControlMetrics(QStyle::CC_ComboBox, cmb,
                                                          QStyle::SC_ComboBoxArrow, widget);
                qt_mac_set_port(p);
                ThemeButtonKind bkind = kThemeArrowButton;
                switch (qt_aqua_size_constrain(widget)) {
                    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                            bkind = kThemeArrowButtonMini;
                            break;
                        }
#endif
                    case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
                qt_mac_set_port(p);
                DrawThemeButton(qt_glb_mac_rect(cmb->rect, p, true, QRect(1, 0, 0, 0)),
                                kThemePopupButton, &info, 0, 0, 0, 0);
            }
        }
        break;
    case QStyle::CC_TitleBar:
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
        //AppMan paints outside the given rectangle, so I have to adjust for the height properly!
        QRect newr = tbar->rect;
        {
            Rect br;
            RgnHandle rgn = qt_mac_get_rgn();
            GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(tbar->rect), tds, &twm, twa,
                                 kWindowTitleBarRgn, rgn);
            GetRegionBounds(rgn, &br);
            newr.translate(newr.x() - br.left, newr.y() - br.top);
            qt_mac_dispose_rgn(rgn);
        }
        if (tbar->subControls & QStyle::SC_TitleBarLabel) {
            int iw = 0;
            if (!tbar->icon.isNull()) {
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(newr), tds, &twm, twa,
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

                    qt_mac_set_port(&pixp);
                    DrawThemeWindowFrame(QtWinType, qt_glb_mac_rect(newr, &pixp, false), tds,
                                         &twm, twa, 0, 0);

                    pixp.save();
                    {
                        RgnHandle rgn = qt_mac_get_rgn();
                        GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(newr), tds, &twm, twa,
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
                qt_mac_set_port(p);
                DrawThemeWindowFrame(QtWinType, qt_glb_mac_rect(newr, p, false), tds,
                                     &twm, twa, 0, 0);
            }
        }
        if (tbar->subControls & (QStyle::SC_TitleBarCloseButton | QStyle::SC_TitleBarMaxButton
                                 | QStyle::SC_TitleBarMinButton | QStyle::SC_TitleBarNormalButton)) {
            ThemeDrawState wtds = tds;
            if (tbar->state & QStyle::Style_MouseOver)
                wtds = kThemeStateRollover;
            struct {
                unsigned int qt_type;
                ThemeTitleBarWidget mac_type;
            } types[] = {
                { QStyle::SC_TitleBarCloseButton, kThemeWidgetCloseBox },
                { QStyle::SC_TitleBarMaxButton, kThemeWidgetZoomBox },
                { QStyle::SC_TitleBarMinButton | QStyle::SC_TitleBarNormalButton, kThemeWidgetCollapseBox },
                { 0, 0 } };
            ThemeWindowMetrics tm;
            tm.metricSize = sizeof(tm);
            const Rect *wm_rect = qt_glb_mac_rect(newr, p, false);
            bool active = tbar->state & QStyle::Style_Active;
            qt_mac_set_port(p);
            for (int i = 0; types[i].qt_type; ++i) {
                ThemeDrawState ctrl_tds = wtds;
                if (active && (tbar->activeSubControls & types[i].qt_type))
                    ctrl_tds = kThemeStatePressed;
                ThemeTitleBarWidget twt = types[i].mac_type;
                /*
                if(tbar->window() && tbar->window()->isWindowModified() && twt == kThemeWidgetCloseBox)
                    twt = kThemeWidgetDirtyCloseBox;
                    */
                DrawThemeTitleBarWidget(QtWinType, wm_rect, ctrl_tds, &tm, twa, twt);
            }
        }
        break; }
    default:
        q->QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
}

QStyle::SubControl QMacStylePrivate::AppManQuerySubControl(QStyle::ComplexControl cc,
                                                           const QStyleOptionComplex *opt,
                                                           const QPoint &pt,
                                                           const QWidget *widget) const
{
    QStyle::SubControl sc = QStyle::SC_None;
    switch (cc) {
    case QStyle::CC_ScrollBar:
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
                    sc = QStyle::SC_ScrollBarSubLine;
                else if (cpc == kControlDownButtonPart)
                    sc = QStyle::SC_ScrollBarAddLine;
            } else if (HitTestThemeTrack(&tdi, pos, &cpc)) {
                if (cpc == kControlPageUpPart)
                    sc = QStyle::SC_ScrollBarSubPage;
                else if (cpc == kControlPageDownPart)
                    sc = QStyle::SC_ScrollBarAddPage;
                else
                    sc = QStyle::SC_ScrollBarSlider;
            }
        }
        break;
    case QStyle::CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, 0, &tdi, widget);
            ControlPartCode hit = 0;
            Point macpt = { (short)pt.y(), (short)pt.x() };
            if (HitTestThemeTrack(&tdi, macpt, &hit) == true) {
                if (hit == kControlPageDownPart || hit == kControlPageUpPart)
                    sc = QStyle::SC_SliderGroove;
                else
                    sc = QStyle::SC_SliderHandle;
            }

        }
        break;
/*
    I don't know why, but we only get kWindowContentRgn here, which isn't what we want at all.
    It would be very nice if this would work.
    case QStyle::CC_TitleBar:
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
            WindowRegionCode hit;
            Point macpt = { (short)pt.y(), (short)pt.x() };
            if (GetThemeWindowRegionHit(QtWinType, qt_glb_mac_rect(tbar->rect), kThemeStateActive,
                                        &twm, twa, macpt, &hit)) {
                switch (hit) {
                case kWindowCloseBoxRgn:
                    sc = QStyle::SC_TitleBarCloseButton;
                    break;
                case kWindowCollapseBoxRgn:
                    sc = QStyle::SC_TitleBarMinButton;
                    break;
                case kWindowZoomBoxRgn:
                    sc = QStyle::SC_TitleBarMaxButton;
                    break;
                case kWindowTitleTextRgn:
                    sc = QStyle::SC_TitleBarLabel;
                    break;
                default:
                    qDebug("got something else %d", hit);
                    break;
                }
            }
        }
        break;
*/
    default:
        sc = q->QWindowsStyle::querySubControl(cc, opt, pt, widget);
    }
    return sc;
}
QRect QMacStylePrivate::AppManQuerySubControlMetrics(QStyle::ComplexControl cc,
                                                     const QStyleOptionComplex *opt,
                                                     QStyle::SubControl sc,
                                                     const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, 0, &tdi, widget);
            Rect macRect;
            GetThemeTrackBounds(&tdi, &macRect);
            tdi.bounds.left  += tdi.bounds.left  - macRect.left;
            tdi.bounds.right -= macRect.right - tdi.bounds.right;
            bool scrollBar = cc == QStyle::CC_ScrollBar;
            if (!scrollBar && sc == QStyle::SC_SliderGroove) {
                GetThemeTrackBounds(&tdi, &macRect);
                ret.setRect(macRect.left, macRect.top, macRect.right - macRect.left,
                            macRect.bottom - macRect.top);
            } else if ((scrollBar && sc == QStyle::SC_ScrollBarSlider)
                        || (!scrollBar && sc == QStyle::SC_SliderHandle)) {
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeTrackThumbRgn(&tdi, rgn);
                GetRegionBounds(rgn, &macRect);
                ret.setRect(macRect.left, macRect.top, (macRect.right - macRect.left) + 1,
                            (macRect.bottom - macRect.top) + 1);
                qt_mac_dispose_rgn(rgn);
            } else if (scrollBar && sc == QStyle::SC_ScrollBarGroove) {
                GetThemeTrackDragRect(&tdi, &macRect);
                ret.setRect(macRect.left, macRect.top, macRect.right - macRect.left,
                            macRect.bottom - macRect.top);
            } else {
                ret = slider->rect; // Give them everything, since I can't get the
                                    // parts otherwise.
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 10,
            spinner_h = 15;
            int fw = q->pixelMetric(QStyle::PM_SpinBoxFrameWidth, spin, widget),
            y = fw,
            x = spin->rect.width() - fw - spinner_w;
            switch (sc) {
                case QStyle::SC_SpinBoxUp:
                    ret = QRect(x, y + ((spin->rect.height() - fw * 2) / 2 - spinner_h),
                                spinner_w, spinner_h);
                    break;
                case QStyle::SC_SpinBoxDown:
                    ret.setRect(x, y + (spin->rect.height() - fw * 2) / 2, spinner_w, spinner_h);
                    break;
                case QStyle::SC_SpinBoxButtonField:
                    ret.setRect(x, y, spinner_w, spin->rect.height() - fw * 2);
                    break;
                case QStyle::SC_SpinBoxEditField:
                    ret.setRect(fw, fw, spin->rect.width() - spinner_w - fw * 2 - macSpinBoxSep,
                                spin->rect.height() - fw * 2);
                    break;
                case QStyle::SC_SpinBoxFrame:
                    ret.setRect(0, 0, spin->rect.width() - spinner_w - macSpinBoxSep,
                                spin->rect.height());
                    break;
                default:
                    ret = q->QWindowsStyle::querySubControlMetrics(cc, spin, sc, widget);
                    break;
            }
        }
        break;
    case QStyle::CC_TitleBar:
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
            if (sc == QStyle::SC_TitleBarCloseButton)
                wrc = kWindowCloseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMinButton)
                wrc = kWindowCollapseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMaxButton)
                wrc = kWindowZoomBoxRgn;
            else if (sc == QStyle::SC_TitleBarLabel)
                wrc = kWindowTitleTextRgn;
            else if (sc == QStyle::SC_TitleBarSysMenu) // We currently don't have this on Mac OS X.
                ret.setRect(-1024, -1024, 10, q->pixelMetric(QStyle::PM_TitleBarHeight,
                                                             tbar, widget));
            if (wrc != kWindowGlobalPortRgn) {
                // AppMan paints outside the given rectangle,
                // so I have to adjust for the height properly!
                Rect br;
                QRect r = tbar->rect;
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(r), kThemeStateActive, &twm, twa,
                                     kWindowTitleBarRgn, rgn);
                GetRegionBounds(rgn, &br);
                r.translate(r.x() - br.left, r.y() - br.top);
                GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(r), kThemeStateActive, &twm, twa,
                                     wrc, rgn);
                GetRegionBounds(rgn, &br);
                ret.setRect(br.left, br.top, (br.right - br.left), (br.bottom - br.top));
                qt_mac_dispose_rgn(rgn);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *combo = qt_cast<const QStyleOptionComboBox *>(opt)) {
            if (sc == QStyle::SC_ComboBoxEditField && !combo->editable) {
                Rect macRect, outRect;
                SetRect(&macRect, 0, 0, combo->rect.width(), combo->rect.height());
                ThemeButtonDrawInfo bdi = { kThemeStateActive, kThemeButtonOff,
                                            kThemeAdornmentNone };
                GetThemeButtonContentBounds(&macRect, kThemePopupButton, &bdi, &outRect);
                ret.setRect(outRect.left, outRect.top, outRect.right - outRect.left,
                            outRect.bottom - outRect.top);
            } else {
                ret = q->QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
            }
        }
        break;
    default:
        ret = q->QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
    }
    return ret;
}

int QMacStylePrivate::AppManPixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                                        const QWidget *widget) const
{
    SInt32 ret = 0;
    switch (metric) {
    case QStyle::PM_TitleBarHeight:
        if (const QStyleOptionTitleBar *tbar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            ThemeWindowMetrics twm;
            memset(&twm, '\0', sizeof(twm));
            twm.metricSize = sizeof(twm);
            twm.titleWidth = tbar->rect.width();
            twm.titleHeight = tbar->rect.height();
            ThemeWindowAttributes twa = kThemeWindowHasTitleText;
            if(tbar->titleBarState)
                twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                       | kThemeWindowHasCollapseBox;
            else if(tbar->titleBarFlags & Qt::WStyle_SysMenu)
                twa |= kThemeWindowHasCloseBox;

            Rect r;
            RgnHandle rgn = qt_mac_get_rgn();
            GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(tbar->rect), kThemeStateActive,
                                 &twm, twa, kWindowTitleBarRgn, rgn);
            GetRegionBounds(rgn, &r);
            ret = (r.bottom - r.top);
            qt_mac_dispose_rgn(rgn);
        }
        break;
    default:
        ret = q->QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    }
    return ret;
}

void QMacStylePrivate::AppManAdjustButtonSize(QStyle::ContentsType ct, QSize &sz,
                                              const QWidget *widget)
{
    ThemeButtonKind bkind = kThemePushButton;
    if (ct == QStyle::CT_ToolButton)
        bkind = kThemeBevelButton;
    if (qt_aqua_size_constrain(widget) == QAquaSizeSmall) {
        if (bkind == kThemeBevelButton)
            bkind = kThemeSmallBevelButton;
    }
    ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
    Rect macRect, myRect;
    SetRect(&myRect, 0, 0, sz.width(), sz.height());
    GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
    sz.setWidth(sz.width() + (myRect.left - macRect.left) + (macRect.right - myRect.right));
    sz.setHeight(sz.height() + (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
}

/*!
    \class QMacStyle qmacstyle_mac.h
    \brief The QMacStyle class implements an Appearance Manager style.

    \ingroup appearance

    This class is implemented as a wrapper to the Apple Appearance
    Manager. This allows your application to be styled by whatever
    theme your Macintosh is using. This is done by having primitives
    in QStyle implemented in terms of what the Macintosh would
    normally theme (i.e. the Finder).

    There are additional issues that should be taken
    into consideration to make an application compatible with the
    \link http://developer.apple.com/documentation/UserExperience/Conceptual/OSXHIGuidelines/index.html
    Aqua Style Guidelines \endlink. Some of these issues are outlined
    below.

    \list

    \i Layout - The restrictions on window layout are such that some
    aspects of layout that are style-dependent cannot be achieved
    using QLayout. Changes are being considered (and feedback would be
    appreciated) to make layouts QStyle-able. Some of the restrictions
    involve horizontal and vertical widget alignment and widget size
    (covered below).

    \i Widget size - Aqua allows widgets to have specific fixed sizes.  Qt
    does not fully implement this behavior so as to maintain cross-platform
    compatibility. As a result some widgets sizes may be inappropriate (and
    subsequently not rendered correctly by the Appearance Manager).The
    QWidget::sizeHint() will return the appropriate size for many
    managed widgets (widgets enumerated in \l QStyle::ContentsType).

    \i Effects - QMacStyle uses Appearance Manager for performing most of
    the drawing, but also uses emulation in a few cases where Appearance
    Manager does not provide the required functionality (for example, QPushButton
    pulsing effects). We tried to make the emulation as close to the original
    as possible. Please report any issues you see in effects or non-standard
    widgets.

    \endlist

    There are other issues that need to be considered in the feel of
    your application (including the general color scheme to match the
    Aqua colors). The Guidelines mentioned above will remain current
    with new advances and design suggestions for Mac OS X.

    Note that the functions provided by QMacStyle are
    reimplementations of QStyle functions; see QStyle for their
    documentation.
*/


/*!
    \enum QMacStyle::WidgetSizePolicy

    \value SizeSmall
    \value SizeLarge
    \value SizeMini
    \value SizeDefault
    \omitvalue SizeNone
*/

/*!
    Constructs a QMacStyle object.
*/
QMacStyle::QMacStyle()
    : QWindowsStyle()
{
    d = new QMacStylePrivate(this);
}

/*!
    Destructs a QMacStyle object.
*/
QMacStyle::~QMacStyle()
{
    delete d;
}

/*! \reimp */
void QMacStyle::polish(QApplication* app)
{
    d->HIThemePolish(app);
    d->AppManPolish(app);
}

/*! \reimp */
void QMacStyle::polish(QWidget* w)
{
    if (d->useHITheme)
	d->HIThemePolish(w);
    else
	d->AppManPolish(w);
}

/*! \reimp */
void QMacStyle::unPolish(QWidget* w)
{
    if (d->useHITheme)
	d->HIThemeUnPolish(w);
    else
	d->AppManUnPolish(w);
}

/*! \reimp */
int QMacStyle::pixelMetric(PixelMetric metric, const QStyleOption *opt, const QWidget *widget) const
{
    SInt32 ret = 0;
    switch (metric) {
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        break; }
    case PM_DialogButtonsSeparator:
        ret = -5;
        break;
    case PM_DialogButtonsButtonHeight: {
        QSize sz;
        ret = qt_aqua_size_constrain(0, QStyle::CT_PushButton, QSize(-1, -1), &sz);
        if(sz == QSize(-1, -1))
            ret = 32;
        else
            ret = sz.height();
        break; }
    case PM_DialogButtonsButtonWidth: {
        QSize sz;
        ret = qt_aqua_size_constrain(0, QStyle::CT_PushButton, QSize(-1, -1), &sz);
        if(sz == QSize(-1, -1))
            ret = 70;
        else
            ret = sz.width();
        break; }

    case PM_MenuBarHMargin:
        ret = 8;
        break;

    case PM_MenuBarVMargin:
        ret = 0;
        break;

    case QStyle::PM_MenuDesktopFrameWidth:
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
                || (qt_cast<const QMainWindow*>(widget->parentWidget())
                   && static_cast<QMainWindow *>(widget->parentWidget())->centralWidget() == widget))
                && (qt_cast<const QViewport *>(widget)
#ifdef QT_COMPAT
                    || widget->inherits("QScrollView")
#endif
                    || widget->inherits("QWorkspaceChild")))
            ret = 0;
        else
#endif
            ret = QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    case PM_MaximumDragDistance:
        ret = -1;
        break;
    case PM_ScrollBarSliderMin:
        ret = 24;
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
    case PM_TitleBarHeight:
	ret = d->useHITheme ? d->HIThemePixelMetric(metric, opt, widget)
			    : d->AppManPixelMetric(metric, opt, widget);
        break;
    case PM_TabBarTabVSpace:
        ret = 4;
        break;
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        ret = 0;
        break;
    case PM_TabBarBaseHeight:
        if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)
            ret = 7;
        else
            ret = 0;
        break;
    case PM_TabBarTabOverlap:
        if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)
            GetThemeMetric(kThemeMetricTabOverlap, &ret);
        else
            ret = 9;
        break;
    case PM_TabBarBaseOverlap:
        GetThemeMetric(kThemeMetricTabFrameOverlap, &ret);
        break;
    case PM_ScrollBarExtent: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricScrollBarWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniScrollBarWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallScrollBarWidth, &ret);
            break;
        }
        break; }
    case PM_IndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxHeight, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
    case PM_MenuPanelWidth:
        ret = 0;
        break;
    default:
        ret = QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    }
    return ret;
}

/*! \reimp */
int QMacStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w,
                         QStyleHintReturn *shret) const
{
    SInt32 ret = 0;
    switch(sh) {
    case SH_TitleBar_AutoRaise:
        ret = true;
        break;
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
        ret = kBulletUnicode;
        break;
	/*
    case SH_DialogButtons_DefaultButton:
        ret = QDialogButtons::Reject;
        break;
	*/
    case SH_Menu_SloppySubMenus:
        ret = true;
        break;
    case SH_GroupBox_TextLabelVerticalAlignment:
        ret = Qt::AlignTop;
        break;
    case SH_ScrollView_FrameOnlyAroundContents:
        if (w && (w->isTopLevel() || !w->parentWidget() || w->parentWidget()->isTopLevel())
            && (qt_cast<const QViewport *>(w)
#ifdef QT_COMPAT
                || w->inherits("QScrollView")
#endif
                || w->inherits("QWorkspaceChild")))
            ret = true;
        else
            ret = QWindowsStyle::styleHint(sh, opt, w, shret);
        break;
    case SH_Menu_FillScreenWithScroll:
        ret = (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3);
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
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt))
            ret = !cmb->editable;
        else
            ret = 0;
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
    case SH_TipLabel_Opacity:
        ret = 242; // About 95%
        break;
    case SH_Button_FocusPolicy:
        ret = Qt::TabFocus;
        break;
    case SH_ToolBar_IconSize:
        ret = Qt::LargeIconSize;
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, opt, w, shret);
        break;
    }
    return ret;
}

/*! \reimp */
QPixmap QMacStyle::generatedIconPixmap(IconMode iconMode, const QPixmap &pixmap,
                                       const QStyleOption *opt) const
{
    switch (iconMode) {
    case QStyle::IM_Disabled: {
        QImage img = pixmap.toImage();
        img.setAlphaBuffer(true);
        int imgh = img.height();
        int imgw = img.width();
        QRgb pixel;
        for (int y = 0; y < imgh; ++y) {
            for (int x = 0; x < imgw; ++x) {
                pixel = img.pixel(x, y);
                img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel),
                                         qAlpha(pixel) / 2));
            }
        }
        return img; }
    case QStyle::IM_Active: {
        QImage img = pixmap.toImage();
        img.setAlphaBuffer(true);
        int imgh = img.height();
        int imgw = img.width();
        int h, s, v, a;
        QRgb pixel;
        for (int y = 0; y < imgh; ++y) {
            for (int x = 0; x < imgw; ++x) {
                pixel = img.pixel(x, y);
                a = qAlpha(pixel);
                QColor hsvColor(pixel);
                hsvColor.getHsv(&h, &s, &v);
                s = qMin(100, s * 2);
                v = v / 2;
                hsvColor.setHsv(h, s, v);
                pixel = hsvColor.rgb();
                img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), a));
            }
        }
        return img; }
    default:
        ;
    }
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

/*! \reimp */
QPixmap QMacStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                  const QWidget *widget) const
{
    IconRef icon = 0;
    switch (standardPixmap) {
        case QStyle::SP_MessageBoxQuestion:
        case QStyle::SP_MessageBoxInformation:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertNoteIcon, &icon);
            break;
        case QStyle::SP_MessageBoxWarning:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertCautionIcon, &icon);
            break;
        case QStyle::SP_MessageBoxCritical:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertStopIcon, &icon);
            break;
        default:
            break;
    }
    if (icon) {
        QPixmap ret = qt_mac_convert_iconref(icon, 64, 64);
        ReleaseIconRef(icon);
        return ret;
    }
    return QWindowsStyle::standardPixmap(standardPixmap, opt, widget);
}

/*!
    \enum QMacStyle::FocusRectPolicy

    This type is used to signify a widget's focus rectangle policy.

    \value FocusEnabled  show a focus rectangle when the widget has focus.
    \value FocusDisabled  never show a focus rectangle for the widget.
    \value FocusDefault  show a focus rectangle when the widget has
    focus and the widget is a QSpinWidget, QDateTimeEdit, QLineEdit,
    QListBox, QListView, editable QTextEdit, or one of their
    subclasses.
*/

/*!
    Sets the focus rectangle policy of \a w. The \a policy can be one of
    \l{QMacStyle::FocusRectPolicy}.

    \sa focusRectPolicy()
*/
void QMacStyle::setFocusRectPolicy(QWidget *w, FocusRectPolicy policy)
{
    QMacStylePrivate::PolicyState::focusMap.insert(w, policy);
    QMacStylePrivate::PolicyState::watchObject(w);
    if (w->hasFocus()) {
        w->clearFocus();
        w->setFocus();
    }
}

/*!
    Returns the focus rectangle policy for the widget \a w.

    The focus rectangle policy can be one of \l{QMacStyle::FocusRectPolicy}.

    \sa setFocusRectPolicy()
*/
QMacStyle::FocusRectPolicy QMacStyle::focusRectPolicy(const QWidget *w)
{
    if (QMacStylePrivate::PolicyState::focusMap.contains(w))
        return QMacStylePrivate::PolicyState::focusMap[w];
    return FocusDefault;
}

/*!
    Sets the widget size policy of \a w. The \a policy can be one of
    \l{QMacStyle::WidgetSizePolicy}.

    \sa widgetSizePolicy()
*/
void QMacStyle::setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy)
{
    QMacStylePrivate::PolicyState::sizeMap.insert(w, policy);
    QMacStylePrivate::PolicyState::watchObject(w);
}

/*!
    Returns the widget size policy for the widget \a w.

    The widget size policy can be one of \l{QMacStyle::WidgetSizePolicy}.

    \sa setWidgetSizePolicy()
*/
QMacStyle::WidgetSizePolicy QMacStyle::widgetSizePolicy(const QWidget *w)
{
    WidgetSizePolicy ret = SizeDefault;
    if (w) {
        if (QMacStylePrivate::PolicyState::sizeMap.contains(w))
            ret = QMacStylePrivate::PolicyState::sizeMap[w];
        if (ret == SizeDefault) {
	    for(QWidget *p = w->parentWidget(); p; p = p->parentWidget()) {
                if (QMacStylePrivate::PolicyState::sizeMap.contains(p)) {
                    ret = QMacStylePrivate::PolicyState::sizeMap[p];
                    if (ret != SizeDefault)
                        break;
                }
                if (p->isTopLevel())
                    break;
            }
        }
    }
    return ret;
}

/*! \reimp */
void QMacStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                              const QWidget *w) const
{
    switch (pe) {
    default:
        if (d->useHITheme)
            d->HIThemeDrawPrimitive(pe, opt, p, w);
        else
            d->AppManDrawPrimitive(pe, opt, p, w);
    }
}

/*! \reimp */
void QMacStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                            const QWidget *w) const
{
    switch (ce) {
    case CE_HeaderLabel:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            QRect textr = header->rect;
            if (!header->icon.isNull()) {
                QIcon::Mode mode = QIcon::Disabled;
                if (opt->state & QStyle::Style_Enabled)
                    mode = QIcon::Normal;
                QPixmap pixmap = header->icon.pixmap(Qt::SmallIconSize, mode);

                QRect pixr = header->rect;
                pixr.setY(header->rect.center().y() - (pixmap.height() - 1) / 2);
                drawItem(p, pixr, Qt::AlignVCenter, header->palette,
                         mode != QIcon::Disabled
                                || !header->icon.isGenerated(Qt::SmallIconSize, mode), pixmap);
                textr.translate(pixmap.width() + 2, 0);
            }

	    QColor penColor = header->palette.buttonText().color();
            if (p->font().bold()) {
                // If it's a table, use the bright text instead.
                if (!(w && (qt_cast<QTreeView *>(w->parentWidget())
#ifdef QT_COMPAT
                            || w->parentWidget()->inherits("Q3ListView")
#endif
                          )))
                    penColor = header->palette.color(QPalette::BrightText);
            }
            drawItem(p, textr, Qt::AlignVCenter, header->palette,
                     header->state & QStyle::Style_Enabled, header->text, &penColor);
        }
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            QStyleOptionToolButton myTb = *tb;
            myTb.state &= ~Style_AutoRaise;
            if (w && qt_cast<QToolBar *>(w->parentWidget())) {
                QRect cr = tb->rect;
                if (tb->toolButtonStyle != Qt::ToolButtonIconOnly && !tb->text.isEmpty()
                    && (tb->state & QStyle::Style_Down)) {
                    if (tb->toolButtonStyle == Qt::ToolButtonTextOnly) {
                        p->drawText(cr, Qt::AlignCenter, tb->text);
                    } else {
                        QIcon::Mode iconMode = (tb->state & QStyle::Style_Enabled) ? QIcon::Normal
                                                                                   : QIcon::Disabled;
                        if (tb->state & QStyle::Style_Down)
                            iconMode = QIcon::Active;
                        QIcon::State iconState = (tb->state & QStyle::Style_On) ? QIcon::On
                                                                                : QIcon::Off;
                        const QPixmap pixmap = tb->icon.pixmap(Qt::LargeIconSize, iconMode,
                                                               iconState);
                        int alignment = 0;
                        if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                            int fh = p->fontMetrics().height();
                            cr.addCoords(0, cr.bottom() - fh - 3, 0, -3);
                            alignment |= Qt::AlignCenter;
                        } else {
                            cr.addCoords(pixmap.width() + 7, -1, 0, 0);
                            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                        }
                        p->drawText(cr, alignment, tb->text);

                    }

                }
                QWindowsStyle::drawControl(ce, &myTb, p, w);
            } else {
                QWindowsStyle::drawControl(ce, &myTb, p, w);
            }
        }
        break;
    case CE_ToolBoxTab:
        QCommonStyle::drawControl(ce, opt, p, w);
        break;
    default:
        if (d->useHITheme)
            d->HIThemeDrawControl(ce, opt, p, w);
        else
            d->AppManDrawControl(ce, opt, p, w);
        break;
    }
}

/*! \reimp */
void QMacStyle::drawControlMask(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *w) const
{
    switch (ce) {
    default:
        QWindowsStyle::drawControlMask(ce, opt, p, w);
        break;
    case CE_RubberBand:
        p->fillRect(opt->rect, Qt::color1);
        break;
    }
}

/*! \reimp */
QRect QMacStyle::subRect(SubRect sr, const QStyleOption *opt, const QWidget *w) const
{
    if (d->useHITheme)
	return d->HIThemeSubRect(sr, opt, w);
    return d->AppManSubRect(sr, opt, w);
}

/*! \reimp */
void QMacStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                   const QWidget *w) const
{
    switch (cc) {
    default:
        if (d->useHITheme)
            d->HIThemeDrawComplexControl(cc, opt, p, w);
        else
            d->AppManDrawComplexControl(cc, opt, p, w);
    case CC_ToolButton:
        if (w && qt_cast<QToolBar *>(w->parentWidget())) {
            if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
                if (tb->subControls & QStyle::SC_ToolButtonMenu) {
                    QStyleOption arrowOpt(0);
                    arrowOpt.rect = querySubControlMetrics(cc, tb, QStyle::SC_ToolButtonMenu, w);
                    arrowOpt.rect.setY(arrowOpt.rect.y() + arrowOpt.rect.height() / 2);
                    arrowOpt.rect.setHeight(arrowOpt.rect.height() / 2);
                    arrowOpt.state = tb->state;
                    arrowOpt.palette = tb->palette;
                    drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, p, w);
                }
            }
        } else {
            if (d->useHITheme)
                d->HIThemeDrawComplexControl(cc, opt, p, w);
            else
                d->AppManDrawComplexControl(cc, opt, p, w);
        }
        drawControl(CE_ToolButtonLabel, opt, p, w);
        break;
    }
}

/*! \reimp */
QStyle::SubControl QMacStyle::querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                              const QPoint &pt, const QWidget *w) const
{
    SubControl sc;
    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            sc = QWindowsStyle::querySubControl(cc, cmb, pt, w);
            if (!cmb->editable && sc != QStyle::SC_None)
                sc = SC_ComboBoxArrow;  // A bit of a lie, but what we want
        }
        break;
    default:
        if (d->useHITheme)
            sc = d->HIThemeQuerySubControl(cc, opt, pt, w);
        else
            sc = d->AppManQuerySubControl(cc, opt, pt, w);
        break;
    }
    return sc;
}

/*! \reimp */
QRect QMacStyle::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                        SubControl sc, const QWidget *w) const
{
    QRect ret;
    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            if (cmb->editable) {
                if (sc == SC_ComboBoxEditField)
                    ret.setRect(0, 0, cmb->rect.width() - 20, cmb->rect.height());
                else if (sc == SC_ComboBoxArrow)
                    ret.setRect(cmb->rect.width() - 24, 0, 24, cmb->rect.height());
            } else {
                if (d->useHITheme)
                    ret = d->HIThemeQuerySubControlMetrics(cc, opt, sc, w);
                else
                    ret = d->AppManQuerySubControlMetrics(cc, opt, sc, w);
            }
        }
        break;
    default:
        if (d->useHITheme)
            ret = d->HIThemeQuerySubControlMetrics(cc, opt, sc, w);
        else
            ret = d->AppManQuerySubControlMetrics(cc, opt, sc, w);
        break;
    }
    return ret;
}

/*! \reimp */
QSize QMacStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                  const QSize &csz, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case QStyle::CT_SpinBox:
        sz.setWidth(sz.width() + macSpinBoxSep);
        break;
    case QStyle::CT_TabBarTab: {
            bool newStyleTabs =
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
                QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4 ? true :
#endif
                false;
            if (!newStyleTabs) {
                SInt32 tabh = sz.height();
                SInt32 overlap = 0;
                switch (qt_aqua_size_constrain(widget)) {
                case QAquaSizeUnknown:
                case QAquaSizeLarge:
                    GetThemeMetric(kThemeLargeTabHeight, &tabh);
                    GetThemeMetric(kThemeMetricTabFrameOverlap, &overlap);
                    break;
                case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                        GetThemeMetric(kThemeMetricMiniTabHeight, &tabh);
                        GetThemeMetric(kThemeMetricMiniTabFrameOverlap, &overlap);
                        break;
                    }
                case QAquaSizeSmall:
                    GetThemeMetric(kThemeSmallTabHeight, &tabh);
                    GetThemeMetric(kThemeMetricSmallTabFrameOverlap, &overlap);
#endif
                default:
                     break;
                }
                tabh += overlap;
                if (sz.height() < tabh)
                    sz.rheight() = tabh;
            } else {
                QWindowsStyle::sizeFromContents(ct, opt, csz, widget);
            }
        break; }
    case QStyle::CT_PushButton:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, widget);
        sz = QSize(sz.width() + 16, sz.height()); // No idea why, but it was in the old style.
        break;
    case QStyle::CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(),
                h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                SInt16 ash;
                GetThemeMenuSeparatorHeight(&ash);
                h = ash;
            } else {
                h = qMax(h, mi->fontMetrics.height() + 2);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(Qt::SmallIconSize, QIcon::Normal).height() + 4);
            }
            if (mi->text.contains('\t'))
                w += 12;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 20;
            if (maxpmw)
                w += maxpmw + 6;
            // add space for a check. All items have place for a check too.
            w += 20;
            if (widget && qt_cast<QComboBox*>(widget->parentWidget())
                    && widget->parentWidget()->isVisible()) {
                QStyleOptionComboBox cmb;
                cmb.init(widget->parentWidget());
                cmb.editable = false;
                cmb.subControls = QStyle::SC_ComboBoxEditField;
                cmb.activeSubControls = QStyle::SC_None;
                w = qMax(w, querySubControlMetrics(QStyle::CC_ComboBox, &cmb,
                                                   QStyle::SC_ComboBoxEditField,
                                                   widget->parentWidget()).width());
            } else {
                w += 12;
            }
            sz = QSize(w, h);
        }
        break;
    default:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, widget);
    }
    QSize macsz;
    if (qt_aqua_size_constrain(widget, ct, sz, &macsz) != QAquaSizeUnknown) {
        if (macsz.width() != -1)
            sz.setWidth(macsz.width());
        if (macsz.height() != -1)
            sz.setHeight(macsz.height());
    }
    // Adjust size to within Aqua guidelines
    if (ct == QStyle::CT_PushButton || ct == QStyle::CT_ToolButton) {
        if (d->useHITheme)
            d->HIThemeAdjustButtonSize(ct, sz, widget);
        else
            d->AppManAdjustButtonSize(ct, sz, widget);
    }
    return sz;
}

/*!
    \reimp
*/
void QMacStyle::drawItem(QPainter *p, const QRect &r, int flags, const QPalette &pal, bool enabled,
                         const QString &text, const QColor *penColor) const
{
    QWindowsStyle::drawItem(p, r, flags | Qt::TextHideMnemonic, pal, enabled, text, penColor);
}

#endif
