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

// ############ REMOVE
#pragma warning(disable: 4065)


#include "qwindowsxpstyle.h"

#if !defined(QT_NO_State_WINDOWSXP) || defined(QT_PLUGIN)

//#include <q3menubar.h>
#include <private/qobject_p.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qpushbutton.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtabbar.h>
#include <qheaderview.h>
#include <qabstractspinbox.h>
#include <qgroupbox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qcleanuphandler.h>
#include <qbitmap.h>
#include <qlibrary.h>
#include <qdesktopwidget.h>
#include <qstackedwidget.h>
#include <qtabwidget.h>
#include <qdrawutil.h>
#include <qmap.h>
#include <qevent.h>
#include <qaction.h>
#include <qt_windows.h>
#include <qdebug.h>

#ifdef Q_CC_GNU
#   include <w32api.h>
#   if (__W32API_MAJOR_VERSION >= 3 || (__W32API_MAJOR_VERSION == 2 && __W32API_MINOR_VERSION >= 5))
#        ifdef _WIN32_WINNT
#            undef _WIN32_WINNT
#        endif
#        define _WIN32_WINNT 0x0501
#        ifndef TMT_TEXTCOLOR
#            define TMT_TEXTCOLOR 3803
#        endif
#        ifndef TMT_BORDERCOLORHINT
#            define TMT_BORDERCOLORHINT 3822
#        endif
#        include <commctrl.h>
#   endif
#endif

#include <uxtheme.h>
#include <tmschema.h>

#include <limits.h>

#define d d_func()
#define q q_func()


/* XPM */
static char * dockCloseXPM[] = {
"8 8 2 1",
"         c none",
".        c #FFFFFF",
"..    ..",
"...  ...",
" ...... ",
"  ....  ",
"  ....  ",
" ...... ",
"...  ...",
"..    .."};

static ulong ref = 0;
static bool use_xp  = false;
static QMap<QString,HTHEME> *handleMap = 0;

typedef bool (WINAPI *PtrIsAppThemed)();
typedef bool (WINAPI *PtrIsThemeActive)();
typedef HRESULT (WINAPI *PtrGetThemePartSize)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, OPTIONAL RECT *prc, enum THEMESIZE eSize, OUT SIZE *psz);
typedef HTHEME (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (WINAPI *PtrCloseThemeData)(HTHEME hTheme);
typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef HRESULT (WINAPI *PtrGetThemeColor)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor);
typedef HRESULT (WINAPI *PtrGetThemeBackgroundRegion)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, const RECT *pRect, OUT HRGN *pRegion);
typedef BOOL (WINAPI *PtrIsThemeBackgroundPartiallyTransparent)(HTHEME hTheme, int iPartId, int iStateId);


static PtrIsAppThemed pIsAppThemed = 0;
static PtrIsThemeActive pIsThemeActive = 0;
static PtrGetThemePartSize pGetThemePartSize = 0;
static PtrOpenThemeData pOpenThemeData = 0;
static PtrCloseThemeData pCloseThemeData = 0;
static PtrDrawThemeBackground pDrawThemeBackground = 0;
static PtrGetThemeColor pGetThemeColor = 0;
static PtrGetThemeBackgroundRegion pGetThemeBackgroundRegion = 0;
static PtrIsThemeBackgroundPartiallyTransparent pIsThemeBackgroundPartiallyTransparent = 0;

class QWindowsXPStylePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWindowsXPStyle)
public:
    QWindowsXPStylePrivate()
        : QObjectPrivate(), tabPaneBorderColor(0)
    {
        init();
    }
    ~QWindowsXPStylePrivate()
    {
        cleanup();
    }

    void init(bool force = false)
    {
        if (ref++ && !force)
            return;

        use_xp = resolveSymbols()
                 && pIsThemeActive()
                 && pIsAppThemed();

        COLORREF cref;
        // Active Title Bar (Color 1 in the gradient)
        cref = GetSysColor(COLOR_ACTIVECAPTION);
        dockColorActive = qRgb(GetRValue(cref), GetGValue(cref), GetBValue(cref));
        // 3D Objects
        cref = GetSysColor(COLOR_3DFACE);
        dockColorInactive = qRgb(GetRValue(cref), GetGValue(cref), GetBValue(cref));

        dockCloseActive = new QPixmap(10, 10);
        dockCloseInactive = new QPixmap(10, 10);
        dockCloseActive->fill(dockColorActive);
        dockCloseInactive->fill(dockColorInactive);

        QPixmap tmp_ex((const char **) dockCloseXPM);

        QPainter p1(dockCloseActive);
        QPainter p2(dockCloseInactive);
        tmp_ex.fill(Qt::white);
        p1.drawPixmap(1, 1, tmp_ex);
        tmp_ex.fill(Qt::black);
        p2.drawPixmap(1, 1, tmp_ex);
    }

    void cleanup(bool force = false)
    {
        if (--ref && !force)
            return;

        use_xp  = false;
        if (handleMap) {
            QMap<QString, HTHEME>::Iterator it;
            for (it = handleMap->begin(); it != handleMap->end(); ++it)
                pCloseThemeData(it.value());
            delete handleMap;
            handleMap = 0;
        }
        delete limboWidget;
        delete tabbody;
        limboWidget = 0;
        tabbody = 0;
        delete dockCloseActive;
        delete dockCloseInactive;
        dockCloseActive = dockCloseInactive = 0;
    }

    static bool getThemeResult(HRESULT res)
    {
        if (res == S_OK)
            return true;
        return false;
    }

    static HWND winId(const QWidget *widget)
    {
        if (widget)
            return widget->winId();

        if (currentWidget)
            return currentWidget->winId();

        if (!limboWidget) {
            limboWidget = new QWidget(0);
            limboWidget->setObjectName("xp_limbo_widget");
        }

        return limboWidget->winId();
    }

    bool resolveSymbols();
    const QPixmap *tabBody(QWidget *widget);

    // hot-widget stuff
    static const QWidget *currentWidget;

    QRgb groupBoxTextColor;
    QRgb groupBoxTextColorDisabled;
    QRgb tabPaneBorderColor;
    QRgb sliderTickColor;
    static QColor dockColorActive;
    static QColor dockColorInactive;
    static QPixmap *dockCloseActive;
    static QPixmap *dockCloseInactive;
    QLibrary themeLib;

private:
    static QWidget *limboWidget;
    static QPixmap *tabbody;
};

const QWidget *QWindowsXPStylePrivate::currentWidget = 0;
QWidget *QWindowsXPStylePrivate::limboWidget = 0;
QPixmap *QWindowsXPStylePrivate::tabbody = 0;

QPixmap *QWindowsXPStylePrivate::dockCloseActive = 0;
QPixmap *QWindowsXPStylePrivate::dockCloseInactive = 0;
QColor   QWindowsXPStylePrivate::dockColorActive = Qt::blue;
QColor   QWindowsXPStylePrivate::dockColorInactive = Qt::gray;

struct XPThemeData
{
    XPThemeData(const QWidget *w = 0, QPainter *p = 0, const QString &theme = QString::null, int part = 0, int state = 0, const QRect &r = QRect(), QRgb tabBorderColor = 0)
        : widget(w), painter(p), name(theme),partId(part), stateId(state), rec(r), tbBorderColor(tabBorderColor), htheme(0), rotated(0), hMirrored(false), vMirrored(false)
    {
    }
    ~XPThemeData()
    {
    }

    HTHEME handle()
    {
        if (!use_xp)
            return NULL;

        if (!htheme && handleMap)
            htheme = handleMap->operator[](name);

        if (!htheme) {
            htheme = pOpenThemeData(QWindowsXPStylePrivate::winId(widget), (TCHAR*)name.utf16());
            if (htheme) {
                if (!handleMap)
                    handleMap = new QMap<QString, HTHEME>;
                handleMap->operator[](name) = htheme;
            }
        }

        return htheme;
    }

    bool isValid()
    {
        return use_xp && name.size() && handle();
    }

    RECT toRECT()
    {
        RECT r;
        r.left = rec.x();
        r.right = rec.x() + rec.width();
        r.top = rec.y();
        r.bottom = rec.y() + rec.height();

        return r;
    }

    RECT toRECT(const QRect &qr)
    {
        RECT r;
        r.left = qr.x();
        r.right = qr.x() + qr.width();
        r.top = qr.y();
        r.bottom = qr.y() + qr.height();

        return r;
    }

    HRGN mask()
    {
        if (pIsThemeBackgroundPartiallyTransparent(handle(), partId, stateId)) {
            HRGN hrgn;
            HDC dc = painter == 0 ? 0 : painter->paintEngine()->getDC();
            RECT nativeRect = toRECT();
            pGetThemeBackgroundRegion(handle(), dc, partId, stateId, &nativeRect, &hrgn);
            if (dc)
                painter->paintEngine()->releaseDC(dc);
            return hrgn;
        }
        return 0;
    }

    void setTransparency()
    {
        HRGN hrgn = mask();
        if (hrgn)
            SetWindowRgn(QWindowsXPStylePrivate::winId(widget), hrgn, true);
    }

    void setRotate(int angle = 0)
    {
        rotated = angle;
    }

    void setHMirrored(bool b = true)
    {
        hMirrored = b;
    }

    void setVMirrored(bool b = true)
    {
        vMirrored = b;
    }

    void drawBackground(int pId = 0, int sId = 0)
    {
        if (pId)
            partId = pId;
        if (sId)
            stateId = sId;

        QRegion systemRegion = painter->paintEngine()->systemClip();
        QRect systemBound = systemRegion.boundingRect();
        systemBound.translate(-painter->paintEngine()->coordinateOffset());
        if (systemRegion.isEmpty())
            systemBound = rec;

        HDC dc = painter == 0 ? 0 : painter->paintEngine()->getDC();
        if (name == "TAB" && (
            partId == TABP_TABITEMLEFTEDGE ||
            partId == TABP_TABITEMRIGHTEDGE ||
            partId == TABP_TABITEM)) {
            QRect oldrec = rec;
            rec = QRect(0, 0, rec.width(), rec.height());
            if (rotated == 90 || rotated == 270)
                rec.setRect(0,0, rec.height(), rec.width());

            QPixmap pm(rec.size());
            QPainter p(&pm);
            pm.fill(Qt::black);

            HDC dc2 = p.paintEngine()->getDC();
            RECT nativeRect = toRECT();
            pDrawThemeBackground(handle(), dc2, partId, stateId, &nativeRect, 0);
            p.paintEngine()->releaseDC(dc2);

            rec = oldrec;
            p.end();

            if (hMirrored || vMirrored)
            {
                QImage img = pm.toImage();
                img = img.mirrored(hMirrored, vMirrored);
                pm = QPixmap::fromImage(img);
            }
            painter->drawPixmap(rec.x(), rec.y(), pm);
        } else {
            QRect rt = rec;
            rec = painter->deviceMatrix().mapRect(rec);
            if (!hMirrored && !vMirrored) {
                HRGN oldRegion = 0;
                bool setClip = true; //painter->hasClipping(); // ##### should work
                if (setClip) {
                    HRGN painterRegion = painter->clipRegion().handle();
                    oldRegion = (HRGN)SelectClipRgn(dc, painterRegion);
                }
                RECT nativeRect = toRECT();
                pDrawThemeBackground(handle(), dc, partId, stateId, &nativeRect, 0);
                if (setClip)
                    SelectClipRgn(dc, oldRegion);
            } else {
                QRect oldrec = rec;
                rec = QRect(0, 0, rec.width(), rec.height());
                QPixmap pm(rec.size());
                QPainter p(&pm);
                if(widget)
                    p.setBackground(widget->palette().color(widget->backgroundRole()));
                else
                    p.setBackground(qApp->palette().background());
                p.eraseRect(0, 0, rec.width(), rec.height());

                HDC dc2 = p.paintEngine()->getDC();
                RECT nativeRect = toRECT();
                pDrawThemeBackground(handle(), dc2, partId, stateId, &nativeRect, 0);
                p.paintEngine()->releaseDC(dc2);

                p.end();
                rec = oldrec;

                if (hMirrored || vMirrored)
                {
                    QImage img = pm.toImage();
                    img = img.mirrored(hMirrored, vMirrored);
                    pm = QPixmap::fromImage(img);
                }

                painter->drawPixmap(rec.x(), rec.y(), pm);
            }
            rec = rt;
        }
        if (dc)
            painter->paintEngine()->releaseDC(dc);
    }

    void setName (const QString &name) {
        this->name = name;
    }

    int partId;
    int stateId;
    QRect rec;
    QRgb tbBorderColor;

private:
    const QWidget *widget;
    QPainter *painter;
    QString name;
    HTHEME htheme;
    uint workAround :1;
    uint hMirrored :1;
    uint vMirrored :1;
    uint rotated;
};

extern Q_GUI_EXPORT HDC qt_win_display_dc();
const QPixmap *QWindowsXPStylePrivate::tabBody(QWidget *widget)
{
    if (!tabbody) {
        SIZE sz;
        XPThemeData theme(0, 0, "TAB", TABP_BODY);
        pGetThemePartSize(theme.handle(), qt_win_display_dc(), TABP_BODY, 0, 0, TS_TRUE, &sz);

        tabbody = new QPixmap(sz.cx, QApplication::desktop()->screenGeometry().height());
        QPainter painter(tabbody);
        theme.rec = QRect(0, 0, sz.cx, sz.cy);
        theme.drawBackground();
        // We fill with the last line of the themedata, that
        // way we don't get a tiled pixmap inside big tabs
        QPixmap temp(sz.cx, 1);
        painter.drawPixmap(0, 0, temp, 0, sz.cy-1, -1, -1);
        painter.drawTiledPixmap(0, sz.cy, sz.cx, tabbody->height()-sz.cy, temp);
    }
    return tabbody;
}

bool QWindowsXPStylePrivate::resolveSymbols()
{
    static bool tried = false;
    if (!tried) {
        tried = true;
        themeLib.setFileName("uxtheme");
        themeLib.load();
        pIsAppThemed = (PtrIsAppThemed)themeLib.resolve("IsAppThemed");
        if (pIsAppThemed) {
            pIsThemeActive = (PtrIsThemeActive)themeLib.resolve("IsThemeActive");
            pGetThemePartSize = (PtrGetThemePartSize)themeLib.resolve("GetThemePartSize");
            pOpenThemeData = (PtrOpenThemeData)themeLib.resolve("OpenThemeData");
            pCloseThemeData = (PtrCloseThemeData)themeLib.resolve("CloseThemeData");
            pDrawThemeBackground = (PtrDrawThemeBackground)themeLib.resolve("DrawThemeBackground");
            pGetThemeColor = (PtrGetThemeColor)themeLib.resolve("GetThemeColor");
            pGetThemeBackgroundRegion = (PtrGetThemeBackgroundRegion)themeLib.resolve("GetThemeBackgroundRegion");
            pIsThemeBackgroundPartiallyTransparent = (PtrIsThemeBackgroundPartiallyTransparent)themeLib.resolve("IsThemeBackgroundPartiallyTransparent");
        }
    }

    return pIsAppThemed != 0;
}

static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsSepHeight        =  7; // separator item height
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  0; // menu item ver text margin
static const int windowsArrowHMargin     =  6; // arrow horizontal margin
static const int windowsCheckMarkHMargin =  0; // horiz. margins of check mark
static const int windowsRightBorder      = 12; // right border on windows

/*!
    \class QWindowsXPStyle
    \brief The QWindowsXPStyle class provides a Microsoft WindowsXP-like look and feel.

    \ingroup appearance

    \warning This style is only available on the Windows XP platform
    because it makes use of Windows XP's style engine.

    Most of the functions are documented in the base classes
    \l{QWindowsStyle}, \l{QCommonStyle}, and \l{QStyle}, but the
    QWindowsXPStyle overloads of drawComplexControl(), drawControl(),
    drawControlMask(), drawPrimitive(), subControlRect(), and
    sizeFromContents(), are documented here.
*/

/*!
    Constructs a QWindowsStyle
*/
QWindowsXPStyle::QWindowsXPStyle()
    : QWindowsStyle()
{
    dd = new QWindowsXPStylePrivate;
    dd->q_ptr = this;
}

/*!
    Destroys the style.
*/
QWindowsXPStyle::~QWindowsXPStyle()
{
    delete dd;
}

/*! \reimp */
void QWindowsXPStyle::unpolish(QApplication *app)
{
    QWindowsStyle::unpolish(app);
}

/*! \reimp */
void QWindowsXPStyle::polish(QApplication *app)
{
    QWindowsStyle::polish(app);

    if (!use_xp)
        return;

    // Get text color for groupbox labels
    COLORREF cref;
    XPThemeData theme(0, 0, "BUTTON", 0, 0);
    pGetThemeColor(theme.handle(), BP_GROUPBOX, GBS_NORMAL, TMT_TEXTCOLOR, &cref);
    dd->groupBoxTextColor = qRgb(GetRValue(cref), GetGValue(cref), GetBValue(cref));
    pGetThemeColor(theme.handle(), BP_GROUPBOX, GBS_DISABLED, TMT_TEXTCOLOR, &cref);
    dd->groupBoxTextColorDisabled = qRgb(GetRValue(cref), GetGValue(cref), GetBValue(cref));
    // Where does this color come from?
    //pGetThemeColor(theme.handle(), TKP_TICS, TSS_NORMAL, TMT_COLOR, &cref);
    dd->sliderTickColor = qRgb(165, 162, 148);
}

/*! \reimp */
void QWindowsXPStyle::polish(QWidget *widget)
{
    QWindowsStyle::polish(widget);
    if (!use_xp)
        return;
    if (qobject_cast<QAbstractButton*>(widget)
        || qobject_cast<QToolButton*>(widget)
        || qobject_cast<QTabBar*>(widget)
        || qobject_cast<QComboBox*>(widget)
        || qobject_cast<QScrollBar*>(widget)
        || qobject_cast<QSlider*>(widget)
        || qobject_cast<QHeaderView*>(widget)
        || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QSpinBox*>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (widget->inherits("Q3WorkspaceChild") || widget->inherits("Q3TitleBar")) {
        widget->setAttribute(Qt::WA_Hover);
        widget->installEventFilter(this);
    }

    if (qobject_cast<QStackedWidget*>(widget) &&
               qobject_cast<QTabWidget*>(widget->parent())) {
        widget->parentWidget()->setAttribute(Qt::WA_ContentsPropagated);
    }

    updateRegion(widget);
}

/*! \reimp */
void QWindowsXPStyle::unpolish(QWidget *widget)
{
    // Unpolish of widgets is the first thing that
    // happens when a theme changes, or the theme
    // engine is turned off. So we detect it here.
    bool newState = dd->resolveSymbols() && pIsThemeActive() && pIsAppThemed();
    if (use_xp != newState) {
        if (use_xp = newState) {
            dd->cleanup(true);
            dd->init(true);
        }
    } else if (handleMap) {
        // this is called a couple of times for
        // complex containers, but that doesn't really matter
        // as we get the handles back when we need them.
        QMap<QString, HTHEME>::Iterator it;
        for (it = handleMap->begin(); it != handleMap->end(); ++it)
            pCloseThemeData(it.value());
        delete handleMap;
        handleMap = 0;
    }

    QWindowsStyle::unpolish(widget);
}

/*! \reimp */
QRect QWindowsXPStyle::subElementRect(SubElement sr, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect(option->rect);
    switch(sr) {
    case SE_TabWidgetTabContents:
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
        {
            rect = QWindowsStyle::subElementRect(sr, option, widget);
            if (sr == SE_TabWidgetTabContents)
                   rect.adjust(0, 0, -2, -2);
        }
        break;
    default:
        rect = QWindowsStyle::subElementRect(sr, option, widget);
    }
    return rect;
}

/*!
    \reimp
*/
void QWindowsXPStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *p,
                                    const QWidget *widget) const
{
    if (!use_xp) {
        QWindowsStyle::drawPrimitive(pe, option, p, widget);
        return;
    }

    QString name;
    int partId = 0;
    int stateId = 0;
    QRect rect = option->rect;
    State flags = option->state;
    bool hMirrored = false;
    bool vMirrored = false;

    switch (pe) {
    case PE_PanelButtonBevel:
        name = "BUTTON";
        partId = BP_PUSHBUTTON;
        if (!(flags & State_Enabled))
            stateId = PBS_DISABLED;
        else if (flags & State_Sunken)
            stateId = PBS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = PBS_HOT;
        //else if (flags & State_ButtonDefault)
        //    stateId = PBS_DEFAULTED;
        else
            stateId = PBS_NORMAL;

        break;

    case PE_PanelButtonTool:
        name = "TOOLBAR";
        partId = TP_BUTTON;
        if (!flags & State_Enabled)
            stateId = TS_DISABLED;
        else if (flags & State_Sunken)
            stateId = TS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = flags & State_On ? TS_HOTCHECKED : TS_HOT;
        else if (flags & State_On)
            stateId = TS_CHECKED;
        else
            stateId = TS_NORMAL;
        break;

    case PE_IndicatorButtonDropDown:
        name = "TOOLBAR";
        partId = TP_SPLITBUTTONDROPDOWN;
        if (!flags & State_Enabled)
            stateId = TS_DISABLED;
        else if (flags & State_Sunken)
            stateId = TS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = flags & State_On ? TS_HOTCHECKED : TS_HOT;
        else if (flags & State_On)
            stateId = TS_CHECKED;
        else
            stateId = TS_NORMAL;
        break;

    case PE_IndicatorCheckBox:
        name = "BUTTON";
        partId = BP_CHECKBOX;
        if (!(flags & State_Enabled))
            stateId = CBS_UNCHECKEDDISABLED;
        else if (flags & State_Sunken)
            stateId = CBS_UNCHECKEDPRESSED;
        else if (flags & State_MouseOver)
            stateId = CBS_UNCHECKEDHOT;
        else
            stateId = CBS_UNCHECKEDNORMAL;

        if (flags & State_On)
            stateId += CBS_CHECKEDNORMAL-1;
        else if (flags & State_NoChange)
            stateId += CBS_MIXEDNORMAL-1;

        break;

    case PE_IndicatorRadioButton:
        name = "BUTTON";
        partId = BP_RADIOBUTTON;
        if (!(flags & State_Enabled))
            stateId = RBS_UNCHECKEDDISABLED;
        else if (flags & State_Sunken)
            stateId = RBS_UNCHECKEDPRESSED;
        else if (flags & State_MouseOver)
            stateId = RBS_UNCHECKEDHOT;
        else
            stateId = RBS_UNCHECKEDNORMAL;

        if (flags & State_On)
            stateId += RBS_CHECKEDNORMAL-1;
        break;

//    case PE_Splitter:
    case PE_IndicatorDockWidgetResizeHandle:
        return;

    case PE_Frame:
        if (flags & State_Raised)
            return;
        name = "LISTVIEW";
        partId = LVP_LISTGROUP;
        break;

    case PE_FrameLineEdit:
        name = "EDIT";
        partId = EP_EDITTEXT;
        if (!(flags & State_Enabled))
            stateId = ETS_DISABLED;
        else
            stateId = ETS_NORMAL;
        break;

    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *tab = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option))
        {
            name = "TAB";
            partId = TABP_PANE;

            if (tab->shape == QTabBar::RoundedNorth)
                break;

            QStyleOptionTabWidgetFrame frameOpt = *tab;
            frameOpt.rect = widget->rect();
            QRect contentsRect = subElementRect(SE_TabWidgetTabContents, &frameOpt, widget);
            QRegion reg = option->rect;
            reg -= contentsRect;
            p->setClipRegion(reg);
            XPThemeData theme(0, p, name, partId, stateId, rect);
            theme.setHMirrored(hMirrored);
            theme.setVMirrored(vMirrored);
            theme.drawBackground();

            p->setClipRect(contentsRect);
            partId = TABP_BODY;
            switch (tab->shape) {
            case QTabBar::RoundedSouth:
                vMirrored = true;
                break;
            default:
                break;
            }
        }
        break;

    case PE_FrameMenu:
//    case PE_PanelPopup:
        p->save();
        p->setPen(option->palette.dark().color());
        p->drawRect(option->rect.adjusted(0,0,-1,-1));
        p->restore();
        return;

//    case PE_MenuBarFrame:
    case PE_PanelMenuBar:
        break;

    case PE_FrameDockWidget:
        name = "REBAR";
        partId = RP_BAND;
        stateId = 1;
        break;

    case PE_IndicatorHeaderArrow:
        {
#if 0 // XP theme engine doesn't know about this :(
            name = "HEADER";
            partId = HP_HEADERSORTARROW;
            if (flags & State_Down)
                stateId = HSAS_SORTEDDOWN;
            else
                stateId = HSAS_SORTEDUP;
#else
            p->save();
            p->setPen(option->palette.dark().color());
            p->translate(0, option->rect.height()/2 - 4);
            if (flags & State_UpArrow) { // invert logic to follow Windows style guide
                p->drawLine(option->rect.x(), option->rect.y(), option->rect.x()+8, option->rect.y());
                p->drawLine(option->rect.x()+1, option->rect.y()+1, option->rect.x()+7, option->rect.y()+1);
                p->drawLine(option->rect.x()+2, option->rect.y()+2, option->rect.x()+6, option->rect.y()+2);
                p->drawLine(option->rect.x()+3, option->rect.y()+3, option->rect.x()+5, option->rect.y()+3);
                p->drawPoint(option->rect.x()+4, option->rect.y()+4);
            } else {
                p->drawLine(option->rect.x(), option->rect.y()+4, option->rect.x()+8, option->rect.y()+4);
                p->drawLine(option->rect.x()+1, option->rect.y()+3, option->rect.x()+7, option->rect.y()+3);
                p->drawLine(option->rect.x()+2, option->rect.y()+2, option->rect.x()+6, option->rect.y()+2);
                p->drawLine(option->rect.x()+3, option->rect.y()+1, option->rect.x()+5, option->rect.y()+1);
                p->drawPoint(option->rect.x()+4, option->rect.y());
            }
            p->restore();
            return;
#endif
        }
        break;

    case PE_FrameStatusBar:
        name = "STATUS";
        partId = SP_PANE;
        break;

    case PE_FrameGroupBox:
        name = "BUTTON";
        partId = BP_GROUPBOX;
        if (!(flags & State_Enabled))
            stateId = GBS_DISABLED;
        else
            stateId = GBS_NORMAL;
        break;

    //case PE_SizeGrip:
    //    name = "STATUS";
    //    partId = SP_GRIPPER;
    //    // empiric correction values...
    //    rect.adjust(-4, -8, 0, 0);
    //    mirror = qApp->reverseLayout();
    //    break;

    case PE_IndicatorProgressChunk:
        name = "PROGRESS";
        partId = PP_CHUNK;
        stateId = 1;
        rect = QRect(option->rect.x(), option->rect.y() + 3, option->rect.width(), option->rect.height() - 5);
        break;

    case PE_Q3DockWindowSeparator:
        name = "TOOLBAR";
        if (flags & State_Horizontal)
            partId = TP_SEPARATOR;
        else
            partId = TP_SEPARATORVERT;
        break;

    case PE_FrameWindow:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(option))
        {
            name = "WINDOW";
            if (flags & State_Active)
                stateId = FS_ACTIVE;
            else
                stateId = FS_INACTIVE;

            int fwidth = frm->lineWidth + frm->midLineWidth;

            XPThemeData theme(0, p, name, 0, stateId);
            if (!theme.isValid())
                break;

            theme.rec = QRect(option->rect.x(), option->rect.y()+fwidth, option->rect.x()+fwidth, option->rect.height()-fwidth);
            theme.partId = WP_FRAMELEFT;
            theme.drawBackground();
            theme.rec = QRect(option->rect.width()-fwidth, option->rect.y()+fwidth, fwidth, option->rect.height()-fwidth);
            theme.partId = WP_FRAMERIGHT;
            theme.drawBackground();
            theme.rec = QRect(option->rect.x(), option->rect.height()-fwidth, option->rect.width(), fwidth);
            theme.partId = WP_FRAMEBOTTOM;
            theme.drawBackground();
            theme.rec = QRect(option->rect.x()-5, option->rect.y()-5, option->rect.width()+10, option->rect.y()+fwidth+5);
            theme.partId = WP_CAPTION;
            theme.drawBackground();

            return;
        }

    case PE_IndicatorBranch: {
        static const int decoration_size = 9;
        int mid_h = option->rect.x() + option->rect.width() / 2;
        int mid_v = option->rect.y() + option->rect.height() / 2;
        int bef_h = mid_h;
        int bef_v = mid_v;
        int aft_h = mid_h;
        int aft_v = mid_v;
        QBrush brush(option->palette.dark().color(), Qt::Dense4Pattern);
        if (option->state & State_Item) {
            if (QApplication::isRightToLeft())
                p->fillRect(option->rect.left(), mid_v, bef_h - option->rect.left(), 1, brush);
            else
                p->fillRect(aft_h, mid_v, option->rect.right() - aft_h + 1, 1, brush);
        }
        if (option->state & State_Sibling)
            p->fillRect(mid_h, aft_v, 1, option->rect.bottom() - aft_v + 1, brush);
        if (option->state & (State_Open | State_Children | State_Item | State_Sibling))
            p->fillRect(mid_h, option->rect.y(), 1, bef_v - option->rect.y(), brush);
        if (option->state & State_Children) {
            int delta = decoration_size / 2;
            bef_h -= delta;
            bef_v -= delta;
            aft_h += delta;
            aft_v += delta;
            XPThemeData theme(0, p, "TREEVIEW");
            theme.rec = QRect(bef_h, bef_v, decoration_size, decoration_size);
            theme.drawBackground(TVP_GLYPH, flags & QStyle::State_Open ? GLPS_OPENED : GLPS_CLOSED);
        }
        return; }
    default:
        break;
    }

    XPThemeData theme(0, p, name, partId, stateId, rect);
    if (!theme.isValid()) {
        QWindowsStyle::drawPrimitive(pe, option, p, widget);
        return;
    }
    theme.setHMirrored(hMirrored);
    theme.setVMirrored(vMirrored);
    theme.drawBackground();
}

/*!
    \reimp
*/
void QWindowsXPStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *p,
                                  const QWidget *widget) const
{
    dd->currentWidget = widget;

    if (!use_xp) {
        QWindowsStyle::drawControl(element, option, p, widget);
        return;
    }

    QRect rect(option->rect);
    State flags = option->state;

    int rotate = 0;
    bool hMirrored = false;
    bool vMirrored = false;

    QString name;
    int partId = 0;
    int stateId = 0;
    switch (element) {
    case CE_HeaderSection:
        name = "HEADER";
        partId = HP_HEADERITEM;
        if (flags & State_Sunken)
            stateId = HIS_PRESSED;
        else if (flags & State_MouseOver)
            stateId = HIS_HOT;
        else
            stateId = HIS_NORMAL;
        break;

    case CE_Splitter: {
        p->eraseRect(option->rect);
        return; }
    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            name = "BUTTON";
            partId = BP_PUSHBUTTON;
            QPushButton *pb = (QPushButton*)widget;
            if (!(flags & State_Enabled) && !pb->isFlat())
                stateId = PBS_DISABLED;
            else if (pb->isFlat() && !(flags & (State_On|State_Sunken)))
                return;
            else if (flags & (State_Sunken | State_On))
                stateId = PBS_PRESSED;
            else if (flags & State_MouseOver)
                stateId = PBS_HOT;
            else if (btn->features & QStyleOptionButton::AutoDefaultButton)
                stateId = PBS_DEFAULTED;
            else
                stateId = PBS_NORMAL;
        }
        break;

    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
        {
            name = "TAB";
            bool isDisabled = !(tab->state & State_Enabled);
            bool hasFocus = tab->state & State_HasFocus;
            bool isHot = tab->state & State_MouseOver;
            bool selected = tab->state & State_Selected;
            bool lastTab = tab->position == QStyleOptionTab::End;
            bool firstTab = tab->position == QStyleOptionTab::Beginning;
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            //bool previousSelected = (tab->selectedPosition == QStyleOptionTab::PreviousIsSelected);
            //bool nextSelected = (tab->selectedPosition == QStyleOptionTab::NextIsSelected);
            bool leftAligned = styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignLeft;
            bool centerAligned = styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignCenter;
            //bool rightAligned = styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignRight;
            int borderThickness = pixelMetric(PM_DefaultFrameWidth, option, widget);
            int tabOverlap = onlyOne ? 0 : pixelMetric(PM_TabBarTabOverlap, option, widget);

            if (isDisabled)
                stateId = TIS_DISABLED;
            else if (selected)
                stateId = TIS_SELECTED;
            else if (hasFocus)
                stateId = TIS_FOCUSED;
            else if (isHot)
                stateId = TIS_HOT;
            else
                stateId = TIS_NORMAL;

            // Selecting proper part depending on position
            if (firstTab || onlyOne) {
                if (leftAligned) {
                    partId = TABP_TABITEMLEFTEDGE;
                } else if (centerAligned) {
                    partId = TABP_TABITEM;
                } else { // rightAligned
                    partId = TABP_TABITEMRIGHTEDGE;
                }
            } else {
                partId = TABP_TABITEM;
            }

            switch (tab->shape) {
            default:
                QCommonStyle::drawControl(element, option, p, widget);
                break;
            case QTabBar::RoundedNorth:
                if (selected)
                    rect.adjust(firstTab ? 0 : -tabOverlap, 0, lastTab ? 0 : tabOverlap, borderThickness);
                else
                    rect.adjust(0, tabOverlap, 0, -borderThickness);
                break;
            case QTabBar::RoundedSouth:
                vMirrored = true;
                if (selected)
                    rect.adjust(firstTab ? 0 : -tabOverlap , -borderThickness, lastTab ? 0 : tabOverlap, 0);
                else
                    rect.adjust(0, borderThickness, 0, -tabOverlap);
                break;
            }



            //switch (tab->shape) {
            //case QTabBar::RoundedNorth:
            //case QTabBar::TriangularNorth:
            //    if ((flags & State_Selected) || (flags & State_HasFocus)) {
            //        rect.adjust(0, 0, 0, 1);
            //    } else {
            //        rect.adjust(0, 1, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
            //            rect.adjust(1, 0, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
            //            rect.adjust(0, 0, -1, 0);
            //    }
            //    break;
            //case QTabBar::RoundedSouth:
            //case QTabBar::TriangularSouth:
            //    rotate = 180;
            //    if ((flags & State_Selected) || (flags & State_HasFocus)) {
            //        rect.adjust(0, -1, 0, 0);
            //    } else {
            //        rect.adjust(0, 0, 0, -1);
            //        if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
            //            rect.adjust(1, 0, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
            //            rect.adjust(0, 0, -1, 0);
            //    }
            //    break;
            //case QTabBar::RoundedEast:
            //case QTabBar::TriangularEast:
            //    rotate = 90;
            //    break;
            //case QTabBar::RoundedWest:
            //case QTabBar::TriangularWest:
            //    rotate = 270;
            //    if ((flags & State_Selected) || (flags & State_HasFocus)) {
            //        rect.adjust(0, 0, 1, 0);
            //    } else {
            //        rect.adjust(1, 0, -5, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::NextIsSelected)
            //            rect.adjust(0, 1, 0, 0);
            //        if (tab->selectedPosition != QStyleOptionTab::PreviousIsSelected)
            //            rect.adjust(0, 0, 0, -1);
            //    }
            //    break;
            //}

            //if (!(flags & State_Selected)) {
            //    switch (tab->shape) {
            //    case QTabBar::RoundedNorth:
            //    case QTabBar::TriangularNorth:
            //        rect.adjust(0,0, 0,-1);
            //        break;
            //    case QTabBar::RoundedSouth:
            //    case QTabBar::TriangularSouth:
            //        rect.adjust(0,1, 0,0);
            //        break;
            //    case QTabBar::RoundedEast:
            //    case QTabBar::TriangularEast:
            //        rect.adjust(1,0, 0,0);
            //        break;
            //    case QTabBar::RoundedWest:
            //    case QTabBar::TriangularWest:
            //        rect.adjust(0,0, -1,0);
            //        break;
            //    }
            //}
        }
        break;

    case CE_ProgressBarGroove:
        name = "PROGRESS";
        partId = PP_BAR;
        stateId = 1;
        break;

    case CE_MenuEmptyArea:
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool act = menuitem->state & State_Selected;
            bool checkable = menuitem->menuHasCheckableItems;
            bool checked = checkable ? menuitem->checked : false;

            // windows always has a check column, regardless whether we have an icon or not
            int checkcol = qMax(menuitem->maxIconWidth, 20);

            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);

            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->fillRect(rect, fill);

            if (element == CE_MenuEmptyArea)
                break;

            // draw separator -------------------------------------------------
            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                p->setPen(menuitem->palette.dark().color());
                p->drawLine(x, y + h/2, x+w, y + h/2);
                p->setPen(menuitem->palette.light().color());
                p->drawLine(x, y+1 + h/2, x+w, y+1 + h/2);
                return;
            }

            int xpos = x;
            QRect vrect = visualRect(option->direction, option->rect, QRect(xpos, y, checkcol, h));

            // draw icon ------------------------------------------------------
            if (!menuitem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap = checked ?
                                 menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, QIcon::On) :
                                 menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                //if (act && !dis && !checked)
                //    qDrawShadePanel(p, vrect, menuitem->palette, false, 1, &menuitem->palette.brush(QPalette::Button));
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vrect.center());
                p->setPen(menuitem->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);

                //int xp = xpos + checkcol + 1;
                //fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
                //p->fillRect(visualRect(option->direction, option->rect, QRect(xp, y, w - checkcol - 1, h)), fill);

            // draw checkmark -------------------------------------------------
            } else if (checked) {
                QStyleOptionMenuItem newMi = *menuitem;
                newMi.state = State_None;
                if (!dis)
                    newMi.state |= State_Enabled;
                if (act)
                    newMi.state |= State_On;
                newMi.rect = visualRect(option->direction, menuitem->rect, QRect(menuitem->rect.x() + windowsItemFrame, menuitem->rect.y() + windowsItemFrame,
                                                                              checkcol - 2 * windowsItemFrame, menuitem->rect.height() - 2*windowsItemFrame));
                drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, p, widget);
            }

            QColor textColor = dis ? menuitem->palette.text().color() :
                               act ? menuitem->palette.highlightedText().color() : menuitem->palette.buttonText().color();
            p->setPen(textColor);

            // draw text ------------------------------------------------------
            int xm = windowsItemFrame + checkcol + windowsItemHMargin;
            xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect vTextRect = visualRect(option->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= (QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft);
                // draw tab text ----------------
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(option->direction, menuitem->rect, QRect(textRect.topRight(), menuitem->rect.bottomRight()));
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1,1,1,1), text_flags, s.mid(t + 1));
                        p->setPen(textColor);
                    }
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                if (dis && !act) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1,1,1,1), text_flags, s.left(t));
                    p->setPen(textColor);
                }
                p->drawText(vTextRect, text_flags, s);
            }

            // draw sub menu arrow --------------------------------------------
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                int dim = (h - 2 * windowsItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = QApplication::isRightToLeft() ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - windowsArrowHMargin - windowsItemFrame - dim;
                QRect vSubMenuRect = visualRect(option->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText, newMI.palette.highlightedText().color());
                drawPrimitive(arrow, &newMI, p, widget);
            }
        }
        return;

    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            if (mbi->state == QStyleOptionMenuItem::DefaultItem)
                break;

            bool act = mbi->state & State_Selected;
            bool dis = !(mbi->state & State_Enabled);

            QBrush fill = mbi->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            QColor textColor = dis ? mbi->palette.text().color() :
                               act ? mbi->palette.highlightedText().color() : mbi->palette.buttonText().color();
            QPixmap pix = mbi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal);

            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;

            p->fillRect(rect, fill);
            if (!pix.isNull())
                drawItemPixmap(p, mbi->rect, alignment, pix);
            else
                drawItemText(p, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, &textColor);
        }
        return;

#ifdef QT3_SUPPORT
    case CE_Q3MenuBarItem:
        {
            if (option.isDefault())
                break;

            Q3MenuItem *mi = option.menuItem();
            if (flags & State_Active)
                p->fillRect(r, pal.brush(QPalette::Highlight));
            else
                p->fillRect(r, pal.brush(QPalette::Button));

            drawItem(p, r, Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine, pal,
                     flags & State_Enabled, mi->pixmap() ? *mi->pixmap() : QPixmap(), mi->text(), -1,
                     flags & State_Active ? &pal.highlightedText().color() : &pal.buttonText().color());
        }
        return;
#endif

    default:
        break;
    }

    XPThemeData theme(widget, p, name, partId, stateId, rect, dd->tabPaneBorderColor);
    if (!theme.isValid()) {
        QWindowsStyle::drawControl(element, option, p, widget);
        return;
    }

    theme.setRotate(rotate);
    theme.setHMirrored(hMirrored);
    theme.setVMirrored(vMirrored);
    theme.drawBackground();

    dd->currentWidget = 0;
}


/*!
    \reimp
*/
void QWindowsXPStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option,
                                         QPainter *p, const QWidget *widget) const
{
    dd->currentWidget = widget; // ######### or from the option

    if (!use_xp) {
        QWindowsStyle::drawComplexControl(cc, option, p, widget);
        return;
    }

    State flags = option->state;
    SubControls sub = option->subControls;
    QRect r = option->rect;

    int partId = 0;
    int stateId = 0;
    if (widget->testAttribute(Qt::WA_UnderMouse) && widget->isActiveWindow())
        flags |= State_MouseOver;

    switch (cc) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option))
        {
            XPThemeData theme(widget, p, "SPIN");

            if (sb->frame && (sub & SC_SpinBoxFrame)) {
                partId = EP_EDITTEXT;
                if ((!flags & State_Enabled))
                    stateId = ETS_DISABLED;
                else if (flags & State_HasFocus)
                    stateId = ETS_FOCUSED;
                else
                    stateId = ETS_NORMAL;

                XPThemeData ftheme(widget, p, "EDIT", partId, stateId, r);
                ftheme.drawBackground();
            }
            if (sub & SC_SpinBoxUp) {
                theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget));
                partId = SPNP_UP;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) || !(flags & State_Enabled))
                    stateId = UPS_DISABLED;
                else if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & QStyle::State_Sunken))
                    stateId = UPS_PRESSED;
                else if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & QStyle::State_MouseOver))
                    stateId = UPS_HOT;
                else
                    stateId = UPS_NORMAL;
                theme.drawBackground(partId, stateId);
            }
            if (sub & SC_SpinBoxDown) {
                theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget));
                partId = SPNP_DOWN;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) || !(flags & State_Enabled))
                    stateId = DNS_DISABLED;
                else if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & QStyle::State_Sunken))
                    stateId = DNS_PRESSED;
                else if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & QStyle::State_MouseOver))
                    stateId = DNS_HOT;
                else
                    stateId = DNS_NORMAL;

                theme.drawBackground(partId, stateId);
            }
        }
        break;

    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            if (sub & SC_ComboBoxEditField) {
                partId = EP_EDITTEXT;
                if (!(flags & State_Enabled))
                    stateId = ETS_DISABLED;
                else if (flags & State_HasFocus)
                    stateId = ETS_FOCUSED;
                else
                    stateId = ETS_NORMAL;
                XPThemeData theme(widget, p, "EDIT", partId, stateId, r);

                theme.drawBackground();
                if (!cmb->editable) {
                    QRect re = visualRect(option->direction, option->rect, subControlRect(CC_ComboBox, option, SC_ComboBoxEditField, widget));
                    if (widget->hasFocus()) {
                        p->fillRect(re, option->palette.highlight());
                        p->setPen(option->palette.highlightedText().color());
                        p->setBackground(option->palette.highlight());
                    } else {
                        p->fillRect(re, option->palette.base());
                        p->setPen(option->palette.text().color());
                        p->setBackground(option->palette.base());
                    }
                }
            }

            if (sub & SC_ComboBoxArrow) {
                XPThemeData theme(widget, p, "COMBOBOX");
                theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget));
                partId = CP_DROPDOWNBUTTON;

                if (!(flags & State_Enabled))
                    stateId = CBXS_DISABLED;
                else if (cmb->activeSubControls == SC_ComboBoxArrow && (cmb->state & State_Sunken))
                    stateId = CBXS_PRESSED;
                else if (cmb->activeSubControls == SC_ComboBoxArrow && (cmb->state & State_MouseOver))
                    stateId = CBXS_HOT;
                else
                    stateId = CBXS_NORMAL;

                theme.drawBackground(partId, stateId);
            }
        }
        break;

    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            XPThemeData theme(widget, p, "SCROLLBAR");
            QScrollBar *bar = (QScrollBar*)widget;
            bool maxedOut = (bar->maximum() == bar->minimum());
            if (maxedOut)
                flags &= ~State_Enabled;

            bool isHorz = flags & State_Horizontal;
            bool isRTL  = option->direction == Qt::RightToLeft;
            if (sub & SC_ScrollBarAddLine) {
                theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget));
                partId = SBP_ARROWBTN;
                if (!(flags & State_Enabled))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTDISABLED : ABS_RIGHTDISABLED) : ABS_DOWNDISABLED);
                else if (scrollbar->activeSubControls & SC_ScrollBarAddLine && (scrollbar->state & State_Sunken))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTPRESSED : ABS_RIGHTPRESSED) : ABS_DOWNPRESSED);
                else if (scrollbar->activeSubControls & SC_ScrollBarAddLine && (scrollbar->state & State_MouseOver))
                    stateId = (isHorz ? (isRTL ? ABS_LEFTHOT : ABS_RIGHTHOT) : ABS_DOWNHOT);
                else
                    stateId = (isHorz ? (isRTL ? ABS_LEFTNORMAL : ABS_RIGHTNORMAL) : ABS_DOWNNORMAL);

                theme.drawBackground(partId, stateId);
            }
            if (sub & SC_ScrollBarSubLine) {
                theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget));
                partId = SBP_ARROWBTN;
                if (!(flags & State_Enabled))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTDISABLED : ABS_LEFTDISABLED) : ABS_UPDISABLED);
                else if (scrollbar->activeSubControls & SC_ScrollBarSubLine && (scrollbar->state & State_Sunken))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTPRESSED : ABS_LEFTPRESSED) : ABS_UPPRESSED);
                else if (scrollbar->activeSubControls & SC_ScrollBarSubLine && (scrollbar->state & State_MouseOver))
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTHOT : ABS_LEFTHOT) : ABS_UPHOT);
                else
                    stateId = (isHorz ? (isRTL ? ABS_RIGHTNORMAL : ABS_LEFTNORMAL) : ABS_UPNORMAL);

                theme.drawBackground(partId, stateId);
            }
            if (maxedOut) {
                theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget));
                theme.rec = theme.rec.unite(subControlRect(CC_ScrollBar, option, SC_ScrollBarSubPage, widget));
                theme.rec = theme.rec.unite(subControlRect(CC_ScrollBar, option, SC_ScrollBarAddPage, widget));
                partId = bar->orientation() == Qt::Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                stateId = SCRBS_DISABLED;

                theme.drawBackground(partId, stateId);
            } else {
                if (sub & SC_ScrollBarAddPage) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarAddPage, widget));
                    partId = flags & State_Horizontal ? SBP_LOWERTRACKHORZ : SBP_LOWERTRACKVERT;
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarAddPage && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarAddPage && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;

                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_ScrollBarSubPage) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarSubPage, widget));
                    partId = flags & State_Horizontal ? SBP_UPPERTRACKHORZ : SBP_UPPERTRACKVERT;
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSubPage && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSubPage && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;

                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_ScrollBarFirst) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarFirst, widget));
                }
                if (sub & SC_ScrollBarLast) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarLast, widget));
                }
                if (sub & SC_ScrollBarSlider) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget));
                    if (!(flags & State_Enabled))
                        stateId = SCRBS_DISABLED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSlider && (scrollbar->state & State_Sunken))
                        stateId = SCRBS_PRESSED;
                    else if (scrollbar->activeSubControls & SC_ScrollBarSlider && (scrollbar->state & State_MouseOver))
                        stateId = SCRBS_HOT;
                    else
                        stateId = SCRBS_NORMAL;

                    const int swidth = theme.rec.width();
                    const int sheight = theme.rec.height();

                    theme.drawBackground(flags & State_Horizontal ? SBP_THUMBBTNHORZ : SBP_THUMBBTNVERT, stateId);

                    // paint gripper if there is enough space
                    SIZE size;
                    pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                    if (sheight > size.cy) {
                        QRect gr;
                        if (flags & State_Horizontal) {
                            gr.setLeft(theme.rec.left() + swidth/2 - 5);
                            gr.setRight(gr.left() + 10);
                            gr.setTop(theme.rec.top() + sheight/2 - 3);
                            gr.setBottom(gr.top() + 6);
                        } else {
                            gr.setLeft(theme.rec.left() + swidth/2 - 3);
                            gr.setRight(gr.left() + 6);
                            gr.setTop(theme.rec.top() + sheight/2 - 5);
                            gr.setBottom(gr.top() + 10);
                        }

                        theme.rec = gr;
                        theme.drawBackground(flags & State_Horizontal ? SBP_GRIPPERHORZ : SBP_GRIPPERVERT, 1);
                    }
                }
                if (sub & SC_ScrollBarGroove) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));
                }
            }
        }
        break;
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option))
        {
            XPThemeData theme(widget, p, "TRACKBAR");
            QSlider *sl = (QSlider*)widget;
            QRegion tickreg = sl->rect();
            if (sub & SC_SliderGroove) {
                theme.rec = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
                if (slider->orientation == Qt::Horizontal) {
                    partId = TKP_TRACK;
                    stateId = TRS_NORMAL;
                    theme.rec = QRect(0, theme.rec.center().y() - 2, sl->width(), 4);
                } else {
                    partId = TKP_TRACKVERT;
                    stateId = TRVS_NORMAL;
                    theme.rec = QRect(theme.rec.center().x() - 2, 0, 4, sl->height());
                }
                theme.drawBackground(partId, stateId);
                tickreg -= theme.rec;
            }
            if (sub & SC_SliderTickmarks) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
                int ticks = slider->tickPosition;
                int thickness = pixelMetric(PM_SliderControlThickness, slider, widget);
                int len = pixelMetric(PM_SliderLength, slider, widget);
                int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
                int interval = slider->tickInterval;
                if (interval <= 0) {
                    interval = slider->singleStep;
                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                        available)
                        - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          0, available) < 3)
                        interval = slider->pageStep;
                }
                if (!interval)
                    interval = 1;
                int fudge = len / 2;
                int pos;
                int bothOffset = (ticks & QSlider::TicksAbove && ticks & QSlider::TicksBelow) ? 1 : 0;
                p->setPen(dd->sliderTickColor);
                int v = slider->minimum;
                while (v <= slider->maximum + 1) {
                    int tickLength = (v == slider->minimum || v >= slider->maximum) ? 4 : 3;
                    pos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum + 1,
                                                          v, available) + fudge;
                    if (slider->orientation == Qt::Horizontal) {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(pos, tickOffset - 1 - bothOffset,
                                        pos, tickOffset - 1 - bothOffset - tickLength);

                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(pos, tickOffset + thickness + bothOffset,
                                        pos, tickOffset + thickness + bothOffset + tickLength);
                    } else {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(tickOffset - 1 - bothOffset, pos,
                                        tickOffset - 1 - bothOffset - tickLength, pos);

                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(tickOffset + thickness + bothOffset, pos,
                                        tickOffset + thickness + bothOffset + tickLength, pos);
                    }
                    v += interval;
                }
            }
            if (sub & SC_SliderHandle) {
                theme.rec = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
                p->fillRect(theme.rec, option->palette.background());
                if (slider->orientation == Qt::Horizontal) {
                    if (slider->tickPosition == QSlider::TicksAbove)
                        partId = TKP_THUMBTOP;
                    else if (slider->tickPosition == QSlider::TicksBelow)
                        partId = TKP_THUMBBOTTOM;
                    else
                        partId = TKP_THUMB;

                    if (!widget->isEnabled())
                        stateId = TUS_DISABLED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_Sunken))
                        stateId = TUS_PRESSED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_MouseOver))
                        stateId = TUS_HOT;
                    else if (flags & State_HasFocus)
                        stateId = TUS_FOCUSED;
                    else
                        stateId = TUS_NORMAL;
                } else {
                    if (slider->tickPosition == QSlider::TicksLeft)
                        partId = TKP_THUMBLEFT;
                    else if (slider->tickPosition == QSlider::TicksRight)
                        partId = TKP_THUMBRIGHT;
                    else
                        partId = TKP_THUMBVERT;

                    if (!widget->isEnabled())
                        stateId = TUVS_DISABLED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_Sunken))
                        stateId = TUVS_PRESSED;
                    else if (slider->activeSubControls & SC_SliderHandle && (slider->state & State_MouseOver))
                        stateId = TUVS_HOT;
                    else if (flags & State_HasFocus)
                        stateId = TUVS_FOCUSED;
                    else
                        stateId = TUVS_NORMAL;
                }
                theme.drawBackground(partId, stateId);
            }
            //if (flags & State_HasFocus) {
            //    Q3StyleOptionFocusRect option(0);
            //    option.rect = subElementRect(SE_SliderFocusRect, sl);
            //    option.palette = pal;
            //    option.state = State_Default;
            //    drawPrimitive(PE_FrameFocusRect, p, re, pal);
            //}
        }
        break;
#endif

    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            XPThemeData theme(widget, p, "TOOLBAR");
            QToolButton *tb = (QToolButton*)widget;

            QRect button, menuarea;
            button = visualRect(option->direction, option->rect,
                                subControlRect(cc, toolbutton, SC_ToolButton, widget));
            menuarea = visualRect(option->direction, option->rect,
                                  subControlRect(cc, toolbutton, SC_ToolButtonMenu, widget));

            State bflags = flags, mflags = flags;

            if (toolbutton->activeSubControls == SC_ToolButton)
                bflags |= State_Sunken;
            else if (toolbutton->activeSubControls == SC_ToolButtonMenu)
                mflags |= State_Sunken;

            if (sub & SC_ToolButton) {
                theme.rec = subControlRect(CC_ToolButton, option, SC_ToolButton, widget);
                QWidget *pW = static_cast<QWidget *>(tb->parent());

                // ########## CE_ToolButtonLabel
                if (toolbutton->features & QStyleOptionToolButton::Arrow) {
                    Qt::ArrowType type = toolbutton->arrowType;

#define TBL_STATE(prefix) \
                    if (!tb->isEnabled()) \
                        stateId = prefix##_DISABLED; \
                    else if (bflags & (State_Sunken | State_On)) \
                        stateId = prefix##_PRESSED; \
                    else if (bflags & State_MouseOver) \
                        stateId = prefix##_HOT; \
                    else \
                        stateId = prefix##_NORMAL;

                    switch(type)
                    {
                    case Qt::RightArrow:
                        partId = SPNP_UPHORZ;
                        TBL_STATE(UPHZS);
                        break;
                    case Qt::LeftArrow:
                        partId = SPNP_DOWNHORZ;
                        TBL_STATE(DNHZS);
                        break;
                    case Qt::UpArrow:
                        partId = SPNP_UP;
                        TBL_STATE(UPS);
                        break;
                    case Qt::DownArrow:
                    default:
                        partId = SPNP_DOWN;
                        TBL_STATE(DNS);
                        break;
                    }
                    theme.setName("SPIN");
                    theme.drawBackground(partId, stateId);
                } else if (bflags & (State_Sunken | State_On | State_Raised)) {
                    if (sub & SC_ToolButtonMenu) {
                        partId = TP_SPLITBUTTON;
                        if (!flags & State_Enabled)
                            stateId = TS_DISABLED;
                        else if (flags & State_Sunken)
                            stateId = TS_PRESSED;
                        else if (flags & State_MouseOver)
                            stateId = flags & State_On ? TS_HOTCHECKED : TS_HOT;
                        else if (flags & State_On)
                            stateId = TS_CHECKED;
                        else
                            stateId = TS_NORMAL;

                        theme.drawBackground(partId, stateId);
                    } else {
                        if (!qobject_cast<QToolBar*>(widget->parentWidget()))
                            drawPrimitive(PE_PanelButtonBevel, option, p, widget);
                        else
                            drawPrimitive(PE_PanelButtonTool, option, p, widget);
                    }
                } else if (pW &&
                           !pW->palette().brush(pW->backgroundRole()).texture().isNull()) {
                    p->drawTiledPixmap(r, pW->palette().brush(pW->backgroundRole()).texture(), tb->pos());
                }
            }
            if (sub & SC_ToolButtonMenu) {
                theme.rec = subControlRect(CC_ToolButton, option, SC_ToolButtonMenu, widget);
                drawPrimitive(PE_IndicatorButtonDropDown, option, p, widget);
            }

            QStyleOptionToolButton label = *toolbutton;
            int fw = pixelMetric(PM_DefaultFrameWidth, option, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            label.features &= ~QStyleOptionToolButton::Arrow;
            drawControl(CE_ToolButtonLabel, &label, p, widget);
            //if (tb->hasFocus() && !tb->focusProxy()) {
            //    Q3StyleOptionFocusRect option(0);
            //    option.rect = tb->rect();
            //    option.rect.adjust(3, 3, -3, -3);
            //    option.palette = pal;
            //    option.state = State_Default;
            //    drawPrimitive(PE_FrameFocusRect, &option, p, tb);
            //}
        }
        break;

    case CC_TitleBar:
        {
            if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option))
            {
                bool isActive = tb->titleBarState & QStyle::State_Active;
                XPThemeData theme(widget, p, "WINDOW");
                if (sub & SC_TitleBarLabel) {
                    theme.rec = option->rect;
                    partId = WP_CAPTION;
                    //partId = tb->testWFlags(Qt::WA_WState_Tool) ? WP_SMALLCAPTION :
                    //        (titlebar->window() && titlebar->window()->isMinimized() ? WP_MINCAPTION : WP_CAPTION);
                    if (widget && !widget->isEnabled())
                        stateId = CS_DISABLED;
                    else if (isActive)
                        stateId = CS_ACTIVE;
                    else
                        stateId = CS_INACTIVE;

                    theme.drawBackground(partId, stateId);

                    QRect ir = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, widget));

                    p->setPen(tb->palette.highlightedText().color());
                    p->drawText(ir.x() + 2, ir.y(), ir.width() - 2, ir.height(),
                                Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, tb->text);
                }

                if (sub & SC_TitleBarSysMenu && tb->titleBarFlags & Qt::WindowSystemMenuHint) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarSysMenu, widget));
                    partId = WP_SYSBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = SBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarSysMenu && (option->state & State_Sunken))
                        stateId = SBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarSysMenu && (option->state & State_MouseOver))
                        stateId = SBS_HOT;
                    else
                        stateId = SBS_NORMAL;
                    if (!tb->icon.isNull())
                        tb->icon.paint(p, theme.rec);
                    else
                        theme.drawBackground(partId, stateId);
                }

                if (sub & SC_TitleBarMinButton && tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarMinButton, widget));
                    partId = WP_MINBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = MINBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarMinButton && (option->state & State_Sunken))
                        stateId = MINBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarMinButton && (option->state & State_MouseOver))
                        stateId = MINBS_HOT;
                    else
                        stateId = MINBS_NORMAL;
                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_TitleBarMaxButton && tb->titleBarFlags & Qt::WindowMaximizeButtonHint) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarMaxButton, widget));
                    partId = WP_MAXBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = MAXBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarMaxButton && (option->state & State_Sunken))
                        stateId = MAXBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarMaxButton && (option->state & State_MouseOver))
                        stateId = MAXBS_HOT;
                    else
                        stateId = MAXBS_NORMAL;
                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_TitleBarNormalButton && tb->titleBarFlags & Qt::WindowMinimizeButtonHint) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarNormalButton, widget));
                    partId = WP_RESTOREBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = RBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarNormalButton && (option->state & State_Sunken))
                        stateId = RBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarNormalButton && (option->state & State_MouseOver))
                        stateId = RBS_HOT;
                    else
                        stateId = RBS_NORMAL;
                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_TitleBarShadeButton && tb->titleBarFlags & Qt::WindowShadeButtonHint) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarShadeButton, widget));
                    partId = WP_MINBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = MINBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarShadeButton && (option->state & State_Sunken))
                        stateId = MINBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarShadeButton && (option->state & State_MouseOver))
                        stateId = MINBS_HOT;
                    else
                        stateId = MINBS_NORMAL;
                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_TitleBarUnshadeButton && tb->titleBarFlags & Qt::WindowShadeButtonHint) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarUnshadeButton, widget));
                    partId = WP_RESTOREBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = RBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarUnshadeButton && (option->state & State_Sunken))
                        stateId = RBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarUnshadeButton && (option->state & State_MouseOver))
                        stateId = RBS_HOT;
                    else
                        stateId = RBS_NORMAL;
                    theme.drawBackground(partId, stateId);
                }
                if (sub & SC_TitleBarCloseButton) {
                    theme.rec = visualRect(option->direction, option->rect, subControlRect(CC_TitleBar, option, SC_TitleBarCloseButton, widget));
                    //partId = titlebar->testWFlags(Qt::WA_WState_Tool) ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON;
                    partId = WP_CLOSEBUTTON;
                    if (!widget->isEnabled() || !isActive)
                        stateId = CBS_DISABLED;
                    else if (option->activeSubControls == SC_TitleBarCloseButton && (option->state & State_Sunken))
                        stateId = CBS_PUSHED;
                    else if (option->activeSubControls == SC_TitleBarCloseButton && (option->state & State_MouseOver))
                        stateId = CBS_HOT;
                    else
                        stateId = CBS_NORMAL;
                    theme.drawBackground(partId, stateId);
                }
            }
        }
        break;

    default:
        QWindowsStyle::drawComplexControl(cc, option, p, widget);
        break;
    }

    dd->currentWidget = 0;
}

/*! \reimp */
int QWindowsXPStyle::pixelMetric(PixelMetric pm, const QStyleOption *option, const QWidget *widget) const
{
    if (!use_xp)
        return QWindowsStyle::pixelMetric(pm, option, widget);

    switch (pm) {
    case PM_MenuBarPanelWidth:
        return 0;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        {
            XPThemeData theme(widget, 0, "BUTTON", BP_CHECKBOX, CBS_UNCHECKEDNORMAL);

            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                if (pm == PM_IndicatorWidth)
                    return size.cx+2;
                return size.cy+2;
            }
        }
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        {
            XPThemeData theme(widget, 0, "BUTTON", BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL);

            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                if (pm == PM_ExclusiveIndicatorWidth)
                    return size.cx+2;
                return size.cy+2;
            }
        }
        break;

    case PM_ProgressBarChunkWidth:
        {
            XPThemeData theme(widget, 0, "PROGRESS", PP_CHUNK);

            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                return size.cx;
            }
        }
        break;

    case PM_ScrollBarExtent:
        {
            XPThemeData theme(widget, 0, "SCROLLBAR", SBP_LOWERTRACKHORZ);

            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                return size.cy;
            }
        }
        break;

    case PM_SliderThickness:
        {
            XPThemeData theme(widget, 0, "TRACKBAR", TKP_THUMB);

            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                return size.cy;
            }
        }
        break;

    case PM_MenuButtonIndicator:
        {
            XPThemeData theme(widget, 0, "TOOLBAR", TP_SPLITBUTTONDROPDOWN);

            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, theme.partId, theme.stateId, 0, TS_TRUE, &size);
                return size.cx;
            }
        }
        break;

    case PM_MenuPanelWidth:
    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
        return 1;

    case PM_TabBarTabOverlap:
        return 2;

    case PM_TabBarBaseOverlap:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int ret = 0;
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                ret = 1;
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                ret = 3;
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                ret = 3;
                break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                ret = 1;
                break;
            }
            return ret;
        }
        return 1;

    case PM_TitleBarHeight:
        {
            XPThemeData theme(widget, 0, "WINDOW", WP_CAPTION, CS_ACTIVE);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, WP_CAPTION, CS_ACTIVE, 0, TS_TRUE, &size);
                return size.cy;
            }
        }

    case PM_MDIFrameWidth:
        {
            XPThemeData theme(widget, 0, "WINDOW", WP_FRAMELEFT, FS_ACTIVE);
            if (theme.isValid()) {
                SIZE size;
                pGetThemePartSize(theme.handle(), NULL, WP_FRAMELEFT, FS_ACTIVE, 0, TS_TRUE, &size);
                return size.cx-1;
            }
        }
        break;

    case PM_MDIMinimizedWidth:
        return 160;

    case PM_MenuHMargin:
        return 2;
    case PM_MenuVMargin:
        return 2;
    case PM_SplitterWidth:
        return qMax(5, QApplication::globalStrut().width());

    default:
        break;
    }

    return QWindowsStyle::pixelMetric(pm, option, widget);
}

/*!
    \reimp
*/
QRect QWindowsXPStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *option,
                                      SubControl sc, const QWidget *widget) const
{
    if (!use_xp)
        return QWindowsStyle::subControlRect(cc, option, sc, widget);

    switch (cc) {
    case CC_TitleBar: {
#ifndef QT_NO_TITLEBAR
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {

            const bool isToolTitle = false; // widget->testWFlags(Qt::WA_WState_Tool)
            const int height = tb->rect.height();
            const int width = tb->rect.width();
            const int controlTop = 6; //widget->testWFlags(Qt::WA_WState_Tool) ? 4 : 6;
            const int controlHeight = height - controlTop - 3;

            const bool sysmenuHint  = (tb->titleBarFlags & Qt::WindowSystemMenuHint) != 0;
            const bool minimizeHint = (tb->titleBarFlags & Qt::WindowMinimizeButtonHint) != 0;
            const bool maximizeHint = (tb->titleBarFlags & Qt::WindowMaximizeButtonHint) != 0;
            const bool contextHint = (tb->titleBarFlags & Qt::WindowContextHelpButtonHint) != 0;
            const bool shadeHint = (tb->titleBarFlags & Qt::WindowShadeButtonHint) != 0;

            switch (sc) {
            case SC_TitleBarLabel: {
                QRect ir(0, 0, width, height);
                if (isToolTitle) {
                    if (sysmenuHint)
                        ir.adjust(0, 0, -controlHeight-3, 0);
                    if (minimizeHint || maximizeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                } else {
                    if (sysmenuHint)
                        ir.adjust(controlHeight+3, 0, -controlHeight-3, 0);
                    if (minimizeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (maximizeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (contextHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (shadeHint)
                        ir.adjust(0, 0, -controlHeight-2, 0);
                }
                return ir; }

            case SC_TitleBarCloseButton:
                return QRect(width - (controlHeight + 1) - controlTop,
                            controlTop, controlHeight, controlHeight);

            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                return QRect(width - ((controlHeight + 1) * 2) - controlTop,
                            controlTop, controlHeight, controlHeight);

            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton: {
                int offset = controlHeight + 1;
                if (!maximizeHint)
                    offset *= 2;
                else
                    offset *= 3;
                return QRect(width - offset - controlTop,
                            controlTop, controlHeight, controlHeight); }

            case SC_TitleBarSysMenu:
                {
                    QSize iconSize = tb->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).size();
                    if (tb->icon.isNull())
                        iconSize = QSize(controlHeight, controlHeight);
                    int hPad = (controlHeight - iconSize.height())/2;
                    int vPad = (controlHeight - iconSize.width())/2;
                    QRect ret = QRect(3 + vPad, controlTop + hPad, iconSize.width(), controlHeight - hPad);
                    return ret;
                }

            default:
                break;
            }
        }
        break; }
#endif
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            int x = 0, y = 0, wi = cmb->rect.width(), he = cmb->rect.height();
            int xpos = x;
            xpos += wi - 1 - 16;

            switch (sc) {
            case SC_ComboBoxFrame:
                return cmb->rect;
            case SC_ComboBoxArrow:
                return QRect(xpos, y+1, 16, he-2);
            case SC_ComboBoxEditField:
                return QRect(x+2, y+2, wi-3-16, he-4);
            case SC_ComboBoxListBoxPopup:
                return cmb->popupRect;
            default:
                break;
            }
        break; }
    default:
        break;
    }
    return QWindowsStyle::subControlRect(cc, option, sc, widget);
}

/*!
    \reimp
*/
QSize QWindowsXPStyle::sizeFromContents(ContentsType ct, const QStyleOption *option,
                                        const QSize &contentsSize, const QWidget *widget) const
{
    if (!use_xp)
        return QWindowsStyle::sizeFromContents(ct, option, contentsSize, widget);

    QSize sz(contentsSize);

    switch (ct) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                sz = QSize(10, windowsSepHeight);
                break;
            }
        }
        // Fall-through intended
    default:
        sz = QWindowsStyle::sizeFromContents(ct, option, sz, widget);
        break;
    }

    return sz;
}


/*! \reimp */
int QWindowsXPStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                               QStyleHintReturn *returnData) const
{
    if (!use_xp)
        return QWindowsStyle::styleHint(hint, option, widget, returnData);

    switch (hint) {
    case SH_TitleBar_NoBorder:
        return 1;

    case SH_GroupBox_TextLabelColor:
        if (widget->isEnabled())
            return dd->groupBoxTextColor;
        else
            return dd->groupBoxTextColorDisabled;

    case SH_Table_GridLineColor:
        return 0xC0C0C0;

    case SH_LineEdit_PasswordCharacter:
        {
            const QFontMetrics &fm = widget->fontMetrics();
            if (fm.inFont(QChar(0x25CF)))
                return 0x25CF;
            else if (fm.inFont(QChar(0x2022)))
                return 0x2022;
            else
                return '*';
        }

    case SH_SpinControls_DisableOnBounds:
        return 0;

    default:
        return QWindowsStyle::styleHint(hint, option, widget, returnData);
    }
}

/*! \reimp */
bool QWindowsXPStyle::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !o->isWidgetType() || e->type() == QEvent::Paint || !use_xp)
        return QWindowsStyle::eventFilter(o, e);

    if (e->type() == QEvent::Resize)
        updateRegion(static_cast<QWidget *>(o));

    return QWindowsStyle::eventFilter(o, e);
}

/*!
    \internal

    Updates the region occupied by the given \a widget.
*/
void QWindowsXPStyle::updateRegion(QWidget *widget)
{
    if (!use_xp)
        return;

    if (widget->inherits("Q3TitleBar") && !QString::compare(widget->objectName(), "_workspacechild_icon_")) {
        XPThemeData theme(widget, 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect());
        theme.setTransparency();
        XPThemeData theme2(widget->parentWidget(), 0, "WINDOW", WP_MINCAPTION, CS_ACTIVE, widget->rect());
        theme2.setTransparency();
    } else if (widget->inherits("Q3WorkspaceChild") || widget->inherits("Q3TitleBar")) {
        if (widget->isMinimized()) {
            XPThemeData theme(widget, 0, "WINDOW", WP_SMALLCAPTION, CS_ACTIVE, widget->rect());
            theme.setTransparency();
        } else {
            XPThemeData theme(widget, 0, "WINDOW", WP_CAPTION, CS_ACTIVE, widget->rect());
            theme.setTransparency();
        }
    }
}

/*!
  Repaints the active tab if is changed.
*/
void QWindowsXPStyle::activeTabChanged()
{
    const QObject *s = sender();
    if (!qobject_cast<const QTabBar*>(s))
        return;

    ((QWidget *)s)->repaint();
}

#endif //QT_NO_State_WINDOWSXP
