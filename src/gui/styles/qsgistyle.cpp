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

#include "qsgistyle.h"

#if !defined(QT_NO_STYLE_SGI) || defined(QT_PLUGIN)

#include "qevent.h"
#include "qmenu.h"
#include "qapplication.h"
#include "qbutton.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qwidget.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qcombobox.h"
#include "qslider.h"
#include "qtextedit.h"
#include "qtoolbutton.h"
#include "qlineedit.h"
#include "q3menubar.h"
#include <limits.h>

#ifndef QT_NO_SLIDER
struct SliderLastPosition
{
    SliderLastPosition() : rect(0,-1,0,-1), slider(0) {}
    QRect rect;
    const QSlider* slider;
};
#endif

#ifndef QT_NO_SCROLLBAR
struct ScrollbarLastPosition
{
    ScrollbarLastPosition() : rect(0,-1, 0,-1), scrollbar(0) {}
    QRect rect;
    const QScrollBar *scrollbar;
};
#endif

class QSGIStylePrivate
{
public:
    QSGIStylePrivate()
        : hotWidget(0), mousePos(-1, -1)
    {
    }

    const QWidget *hotWidget;
    QPoint mousePos;
#ifndef QT_NO_SCROLLBAR
    ScrollbarLastPosition lastScrollbarRect;
#endif
#ifndef QT_NO_SLIDER
    SliderLastPosition lastSliderRect;
#endif
};

/*!
    \class QSGIStyle qsgistyle.h
    \brief The QSGIStyle class provides SGI/Irix look and feel.

    \ingroup appearance

    This class implements the SGI look and feel. It resembles the
    SGI/Irix Motif GUI style as closely as QStyle allows.

    Most of the functions are documented in the base classes
    \l{QMotifStyle}, \l{QCommonStyle}, and \l{QStyle}, but the
    QSGIStyle overloads of drawComplexControl(), drawControl(),
    drawPrimitive(), querySubControlMetrics(),
    setUseHighlightColors(), sizeFromContents(), subRect(), and
    useHighlightColors(), are documented here.
*/

/*!
    Constructs a QSGIStyle.

    If \a useHighlightCols is false (default value), the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.

    \sa QMotifStyle::useHighlightColors()
*/
QSGIStyle::QSGIStyle(bool useHighlightCols) : QMotifStyle(useHighlightCols), isApplicationStyle(0)
{
    d = new QSGIStylePrivate;
}

/*!
    Destroys the style.
*/
QSGIStyle::~QSGIStyle()
{
    delete d;
}

/*!
    \reimp

    Changes some application-wide settings to be SGI-like, e.g. sets a
    bold italic font for menu options.
*/
void
QSGIStyle::polish(QApplication* app)
{
    isApplicationStyle = 1;
    QMotifStyle::polish(app);

    QPalette pal = QApplication::palette();
    // check this on SGI-Boxes
    //pal.setColor(QPalette::Background, pal.active().midlight());
    if (pal.brush(QPalette::Active, QPalette::Button) == pal.brush(QPalette::Active, QPalette::Background))
        pal.setColor(QPalette::Button, pal.color(QPalette::Active, QPalette::Button).dark(120));
    // darker basecolor in list-widgets
    pal.setColor(QPalette::Base, pal.color(QPalette::Active, QPalette::Base).dark(130));
    if (! useHighlightColors()) {
        pal.setColor(QPalette::Active, QPalette::Highlight,
                      pal.color(QPalette::Active, QPalette::Text));
        pal.setColor(QPalette::Active, QPalette::HighlightedText,
                      pal.color(QPalette::Active, QPalette::Base));
        pal.setColor(QPalette::Inactive, QPalette::Highlight,
                      pal.color(QPalette::Inactive, QPalette::Text));
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                      pal.color(QPalette::Inactive, QPalette::Base));
        pal.setColor(QPalette::Disabled, QPalette::Highlight,
                      pal.color(QPalette::Disabled, QPalette::Text));
        pal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                      pal.color(QPalette::Disabled, QPalette::Base));
    }
    QApplication::setPalette(pal);

    // different basecolor and highlighting in Q(Multi)LineEdit
    pal.setColor(QPalette::Base, QColor(211,181,181));
    pal.setColor(QPalette::Active, QPalette::Highlight,
                  pal.color(QPalette::Active, QPalette::Midlight));
    pal.setColor(QPalette::Active, QPalette::HighlightedText,
                  pal.color(QPalette::Active, QPalette::Text));
    pal.setColor(QPalette::Inactive, QPalette::Highlight,
                  pal.color(QPalette::Inactive, QPalette::Midlight));
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                  pal.color(QPalette::Inactive, QPalette::Text));
    pal.setColor(QPalette::Disabled, QPalette::Highlight,
                  pal.color(QPalette::Disabled, QPalette::Midlight));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                  pal.color(QPalette::Disabled, QPalette::Text));

    QApplication::setPalette(pal, "QLineEdit");
    QApplication::setPalette(pal, "QTextEdit");
    QApplication::setPalette(pal, "QDateTimeEditBase");

    pal = QApplication::palette();
    pal.setColor(QPalette::Button, pal.color(QPalette::Active, QPalette::Background));
    QApplication::setPalette(pal, "Q3MenuBar");
    QApplication::setPalette(pal, "QToolBar");
    QApplication::setPalette(pal, "QMenu");
#ifdef QT_COMPAT
    QApplication::setPalette(pal, "Q3PopupMenu");
#endif
}

/*! \reimp
*/
void
QSGIStyle::unPolish(QApplication* /* app */)
{
    QFont f = QApplication::font();
    QApplication::setFont(f); // get rid of the special fonts for special widget classes
}

/*!
    \reimp

    Installs an event filter for several widget classes to enable
    hovering.
*/
void
QSGIStyle::polish(QWidget* w)
{
    QMotifStyle::polish(w);

    if (!isApplicationStyle) {
        QPalette sgiPal = QApplication::palette();

        sgiPal.setColor(QPalette::Background, sgiPal.color(QPalette::Active, QPalette::Midlight));
        if (sgiPal.brush(QPalette::Active, QPalette::Button) ==
            sgiPal.brush(QPalette::Active, QPalette::Background))
            sgiPal.setColor(QPalette::Button, sgiPal.color(QPalette::Active, QPalette::Button).dark(110));
        sgiPal.setColor(QPalette::Base, sgiPal.color(QPalette::Active, QPalette::Base).dark(130));
        if (! useHighlightColors()) {
            sgiPal.setColor(QPalette::Active, QPalette::Highlight,
                             sgiPal.color(QPalette::Active, QPalette::Text));
            sgiPal.setColor(QPalette::Active, QPalette::HighlightedText,
                             sgiPal.color(QPalette::Active, QPalette::Base));
            sgiPal.setColor(QPalette::Inactive, QPalette::Highlight,
                             sgiPal.color(QPalette::Inactive, QPalette::Text));
            sgiPal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                             sgiPal.color(QPalette::Inactive, QPalette::Base));
            sgiPal.setColor(QPalette::Disabled, QPalette::Highlight,
                             sgiPal.color(QPalette::Disabled, QPalette::Text));
            sgiPal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                             sgiPal.color(QPalette::Disabled, QPalette::Base));
        }

        if (qt_cast<QLineEdit*>(w) || qt_cast<QTextEdit*>(w)) {
            // different basecolor and highlighting in Q(Multi)LineEdit
            sgiPal.setColor(QPalette::Base, QColor(211,181,181));
            sgiPal.setColor(QPalette::Active, QPalette::Highlight,
                             sgiPal.color(QPalette::Active, QPalette::Midlight));
            sgiPal.setColor(QPalette::Active, QPalette::HighlightedText,
                             sgiPal.color(QPalette::Active, QPalette::Text));
            sgiPal.setColor(QPalette::Inactive, QPalette::Highlight,
                             sgiPal.color(QPalette::Inactive, QPalette::Midlight));
            sgiPal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                             sgiPal.color(QPalette::Inactive, QPalette::Text));
            sgiPal.setColor(QPalette::Disabled, QPalette::Highlight,
                             sgiPal.color(QPalette::Disabled, QPalette::Midlight));
            sgiPal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                             sgiPal.color(QPalette::Disabled, QPalette::Text));
        } else if (qt_cast<Q3MenuBar*>(w) || qt_cast<QToolBar*>(w)) {
            sgiPal.setColor(QPalette::Button, sgiPal.color(QPalette::Active, QPalette::Midlight));
        }

        w->setPalette(sgiPal);
    }

    if (qt_cast<QButton*>(w) || qt_cast<QSlider*>(w) || qt_cast<QScrollBar*>(w)) {
        w->installEventFilter(this);
        w->setMouseTracking(true);
#ifndef QT_NO_SCROLLBAR
        if (qt_cast<QScrollBar*>(w))
            w->setAttribute(QWidget::WA_NoSystemBackground, true);
#endif
    } else if (qt_cast<QComboBox*>(w)) {
        QFont f = QApplication::font();
        f.setBold(true);
        f.setItalic(true);
        w->setFont(f);
#ifndef QT_NO_MENUBAR
    } else if (qt_cast<Q3MenuBar*>(w)) {
        ((QFrame*) w)->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        QFont f = QApplication::font();
        f.setBold(true);
        f.setItalic(true);
        w->setFont(f);
#endif
#ifndef QT_NO_MENU
    } else if (qt_cast<QMenu*>(w)) {
        QFont f = QApplication::font();
        f.setBold(true);
        f.setItalic(true);
        w->setFont(f);
#endif
#ifdef QT_COMPAT
#ifndef QT_NO_POPUPMENU
    } else if (qt_cast<Q3PopupMenu*>(w)) {
        ((QFrame*) w)->setLineWidth(pixelMetric(PM_DefaultFrameWidth) + 1);
        QFont f = QApplication::font();
        f.setBold(true);
        f.setItalic(true);
        w->setFont(f);
#endif
#endif
    }
}

/*! \reimp */
void
QSGIStyle::unPolish(QWidget* w)
{
    if (qt_cast<QButton*>(w) || qt_cast<QSlider*>(w) || qt_cast<QScrollBar*>(w)) {
        w->removeEventFilter(this);
#ifndef QT_NO_MENU
    } else if (qt_cast<QMenu*>(w)) {
        w->setFont(QApplication::font());
#endif
#ifdef QT_COMPAT
#ifndef QT_NO_POPUPMENU
    } else if (qt_cast<Q3PopupMenu*>(w)) {
        ((QFrame*)w)->setLineWidth(pixelMetric(PM_DefaultFrameWidth));
        w->setFont(QApplication::font());
#endif
#endif
#if !defined(QT_NO_MENUBAR) || !defined(QT_NO_COMBOBOX)
    } else if (qt_cast<Q3MenuBar*>(w) || qt_cast<QComboBox*>(w)) {
        w->setFont(QApplication::font());
#endif
    }
}

/*! \reimp */
bool QSGIStyle::eventFilter(QObject* o, QEvent* e)
{
    if (!o->isWidgetType() || e->type() == QEvent::Paint)
        return QMotifStyle::eventFilter(o, e);

    QWidget *widget = (QWidget*)o;

    switch (e->type()) {
    case QEvent::MouseButtonPress:
        {
#ifndef QT_NO_SCROLLBAR
            if (qt_cast<QScrollBar*>(widget)) {
                d->lastScrollbarRect.rect = querySubControlMetrics(QStyle::CC_ScrollBar, widget,  QStyle::SC_ScrollBarSlider);
                d->lastScrollbarRect.scrollbar = ((QScrollBar*)widget);
                widget->repaint();
            } else
#endif
            {
#ifndef QT_NO_SLIDER
                if (qt_cast<QSlider*>(widget)) {
                    d->lastScrollbarRect.rect = querySubControlMetrics(QStyle::CC_Slider, widget,  QStyle::SC_SliderHandle);
                    d->lastSliderRect.slider = ((QSlider*)widget);
                    widget->repaint();
                }
#endif
            }
        }
        break;

    case QEvent::MouseButtonRelease:
        {
            if (0) {
#ifndef QT_NO_SCROLLBAR
            } else if (qt_cast<QScrollBar*>(widget)) {
                QRect oldRect = d->lastScrollbarRect.rect;
                d->lastScrollbarRect.rect = QRect(0, -1, 0, -1);
                widget->repaint(oldRect);
#endif
#ifndef QT_NO_SLIDER
            } else if (qt_cast<QSlider*>(widget)) {
                QRect oldRect = d->lastSliderRect.rect;
                d->lastSliderRect.rect = QRect(0, -1, 0, -1);
                widget->repaint(oldRect);
#endif
            }
        }
        break;

    case QEvent::MouseMove:
        if (!widget->isActiveWindow())
            break;
        if (((QMouseEvent*)e)->button())
            break;

        d->hotWidget = widget;
        d->mousePos = ((QMouseEvent*)e)->pos();
        widget->repaint();
        break;

    case QEvent::Enter:
        if (!widget->isActiveWindow())
            break;
        d->hotWidget = widget;
        widget->repaint();
        break;

    case QEvent::Leave:
        if (!widget->isActiveWindow())
            break;
        if (widget == d->hotWidget) {
            d->hotWidget = 0;
            widget->repaint();
        }
        break;

    default:
        break;
    }
    return QMotifStyle::eventFilter(o, e);
}

static const int sgiItemFrame           = 2;    // menu item frame width
// static const int sgiSepHeight         = 1;    // separator item height
static const int sgiItemHMargin         = 3;    // menu item hor text margin
static const int sgiItemVMargin         = 2;    // menu item ver text margin
static const int sgiArrowHMargin        = 6;    // arrow horizontal margin
static const int sgiTabSpacing                 = 12;   // space between text and tab
// static const int sgiCheckMarkHMargin = 2;    // horiz. margins of check mark ### not used?!?
static const int sgiCheckMarkSpace      = 20;

/*int QSGIStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const*/

/*! \reimp */
int QSGIStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const

{
    switch (metric) {
    case PM_MenuFrameWidth:
    case PM_DefaultFrameWidth:
        return 2;

    case PM_ButtonDefaultIndicator:
        return 4;

    case PM_ScrollBarExtent:
        return 21;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        return 14;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return 12;

    case PM_SplitterWidth:
        return qMax(10, QApplication::globalStrut().width());

    default:
        break;
    }
    return QMotifStyle::pixelMetric(metric, option, widget);
}

static void drawPanel(QPainter *p, int x, int y, int w, int h,
                const QPalette &pal, bool sunken,
                int lineWidth, const QBrush* fill)
{
    if (w == 0 || h == 0)
        return;
#if defined(CHECK_RANGE)
    ASSERT(w > 0 && h > 0 && lineWidth >= 0);
#endif
    QPen oldPen = p->pen();                        // save pen
    QPointArray a(4*lineWidth);
    if (sunken)
        p->setPen(pal.dark());
    else
        p->setPen(pal.light());
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for (i=0; i<lineWidth; i++) {                // top shadow
        a.setPoint(n++, x1, y1++);
        a.setPoint(n++, x2--, y2++);
    }
    x2 = x1;
    y1 = y+h-2;
    for (i=0; i<lineWidth; i++) {                // left shadow
        a.setPoint(n++, x1++, y1);
        a.setPoint(n++, x2++, y2--);
    }
    p->drawLineSegments(a);
    n = 0;
    if (sunken)
        p->setPen(pal.light());
    else
        p->setPen(pal.dark());
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for (i=0; i<lineWidth; i++) {                // bottom shadow
        a.setPoint(n++, x1++, y1--);
        a.setPoint(n++, x2, y2--);
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for (i=0; i<lineWidth; i++) {                // right shadow
        a.setPoint(n++, x1--, y1++);
        a.setPoint(n++, x2--, y2);
    }
    p->drawLineSegments(a);
    if (fill) {                                // fill with fill color
        QBrush oldBrush = p->brush();
        p->setPen(Qt::NoPen);
        p->setBrush(*fill);
        p->drawRect(x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2);
        p->setBrush(oldBrush);
    }
    p->setPen(oldPen);                        // restore pen
}

static void drawSeparator(QPainter *p, int x1, int y1, int x2, int y2,
                          const QPalette &pal)
{
    QPen oldPen = p->pen();

    p->setPen(pal.midlight());
    p->drawLine(x1, y1, x2, y2);
    p->setPen(pal.shadow());
    if (y2-y1 < x2-x1)
        p->drawLine(x1, y1+1, x2, y2+1);
    else
        p->drawLine(x1+1, y1, x2+1, y2);

    p->setPen(oldPen);
}

static void drawSGIPrefix(QPainter *p, int x, int y, QString* miText)
{
    if (miText && (miText->size())) {
        int amp = 0;
        bool nextAmp = false;
        while ((amp = miText->indexOf('&', amp)) != -1) {
            if (amp == miText->length()-1)
                return;
            miText->remove(amp,1);
            nextAmp = (*miText)[amp] == '&';    // next time if &&

            if (!nextAmp) {     // draw special underlining
                uint ulx = p->fontMetrics().width(*miText, amp);

                uint ulw = p->fontMetrics().width(*miText, amp+1) - ulx;

                p->drawLine(x+ulx, y, x+ulx+ulw, y);
                p->drawLine(x+ulx, y+1, x+ulx+ulw/2, y+1);
                p->drawLine(x+ulx, y+2, x+ulx+ulw/4, y+2);
            }
            amp++;
        }
    }
}

static int get_combo_extra_width(int h, int *return_awh=0)
{
    int awh;
    if (h < 8) {
        awh = 6;
    } else if (h < 14) {
        awh = h - 2;
    } else {
        awh = h/2;
    }
    if (return_awh)
        *return_awh = awh;
    return awh*2;
}

static void get_combo_parameters(const QRect &r,
                                  int &ew, int &awh, int &ax,
                                  int &ay, int &sh, int &dh,
                                  int &sy)
{
    ew = get_combo_extra_width(r.height(), &awh);

    sh = (awh+3)/4;
    if (sh < 3)
        sh = 3;
    dh = sh/2 + 1;

    ay = r.y() + (r.height()-awh-sh-dh)/2;
    if (ay < 0) {
        //panic mode
        ay = 0;
        sy = r.height();
    } else {
        sy = ay+awh+dh;
    }
    if(QApplication::isRightToLeft())
        ax = r.x();
    else
        ax = r.x() + r.width() - ew;
    ax  += (ew-awh)/2;
}

/*
void QSGIStyle::drawPrimitive(PrimitiveElement pe,
                    QPainter *p,
                    const QRect &r,
                    const QPalette &pal,
                    SFlags flags,
                    const Q3StyleOption& opt) const
                    */

/*!
  \reimp
*/
void QSGIStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                              const QWidget *w) const
{
    const int x = r.x();
    const int y = r.y();
    const int w = r.width();
    const int h = r.height();
    const bool sunken = flags & (Style_Sunken | Style_Down | Style_On);
    const int defaultFrameWidth = pixelMetric(PM_DefaultFrameWidth);
    bool hot = (flags & Style_MouseOver) && (flags & Style_Enabled);

    switch (pe) {
    case PE_ButtonCommand:
        {
            QBrush fill;
            if (hot) {
                if (sunken)
                    fill = pal.brush(QPalette::Dark);
                else
                    fill = pal.brush(QPalette::Midlight);
            } else if (sunken) {
                fill = pal.brush(QPalette::Mid);
            } else {
                fill = pal.brush(QPalette::Button);
            }

            drawPanel(p, x, y, w, h, pal, sunken, defaultFrameWidth, &fill);
        }
        break;

    case PE_MenuFrame:
    case PE_PanelPopup:
    case PE_ButtonBevel:
    case PE_ButtonTool:
        {
            drawPrimitive(PE_ButtonCommand, p, QRect(x+1, y+1, w-2, h-2), pal, flags, opt);

            QPen oldPen = p->pen();
            QPointArray a;

            // draw twocolored rectangle
            p->setPen(sunken ? pal.light().color() : pal.dark().color().dark(200));
            a.setPoints(3, x, y+h-1, x+w-1, y+h-1, x+w-1, y);
            p->drawPolyline(a);
            p->setPen(pal.dark());
            a.setPoints(3, x, y+h-2, x, y, x+w-2, y);
            p->drawPolyline(a);

            p->setPen(oldPen);
        }
        break;

    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowLeft:
    case PE_ArrowRight:
        {
            QPointArray a;                                // arrow polygon
            switch (pe) {
            case PE_ArrowUp:
                a.setPoints(3, 0,-5, -5,4, 4,4);
                break;
            case PE_ArrowDown:
                a.setPoints(3, 0,4, -4,-4, 4,-4);
                break;
            case PE_ArrowLeft:
                a.setPoints(3, -4,0, 4,-5, 4,4);
                break;
            case PE_ArrowRight:
                a.setPoints(3, 4,0, -4,-5, -4,4);
                break;
            default:
                return;
            }

            p->save();
            p->setPen(Qt::NoPen);
            a.translate(x+w/2, y+h/2);
            p->setBrush(flags & Style_Enabled ? pal.dark() : pal.light());
            p->drawPolygon(a);                        // draw arrow
            p->restore();
        }
        break;

    case PE_Indicator:
        {
            QRect er = r;
            er.addCoords(1, 1, -1, -1);
            int iflags = flags & ~Style_On;
            drawPrimitive(PE_ButtonBevel, p, er, pal, iflags, opt);
            if (!(flags & QStyle::Style_Off)) {
                er = r;
                er.addCoords(1, 2, 1, 1);
                drawPrimitive(PE_CheckMark, p, er, pal, flags, opt);
            }
        }
        break;

    case PE_IndicatorMask:
        {
            QPen oldPen = p->pen();
            QBrush oldBrush = p->brush();

            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            p->fillRect(x, y, w, h, QBrush(Qt::color0));
            QRect er = r;
            er.addCoords(1, 1, -1, -1);
            p->fillRect(er, QBrush(Qt::color1));

            if (!(flags & QStyle::Style_Off)) {
                er = r;
                er.addCoords(1, 2, 1, 1);
                static const QCOORD check_mark[] = {
                        14,0,  10,0,  11,1,  8,1,  9,2,         7,2,  8,3,  6,3,
                        7,4,  1,4,  6,5,  1,5,        6,6,  3,6,  5,7,  4,7,
                        5,8,  5,8,  4,3,  2,3,        3,2,  3,2 };

                QPointArray amark;
                amark = QPointArray(sizeof(check_mark)/(sizeof(QCOORD)*2), check_mark);
                amark.translate(er.x()+1, er.y()+1);
                p->drawLineSegments(amark);
                amark.translate(-1, -1);
                p->drawLineSegments(amark);
            }

            p->setBrush(oldBrush);
            p->setPen(oldPen);
        }
        break;

    case PE_CheckMark:
        {
            static const QCOORD check_mark[] = {
                14,0,  10,0,  11,1,  8,1,  9,2,         7,2,  8,3,  6,3,
                7,4,  1,4,  6,5,  1,5,        6,6,  3,6,  5,7,  4,7,
                5,8,  5,8,  4,3,  2,3,        3,2,  3,2 };

            QPen oldPen = p->pen();

            QPointArray amark;
            amark = QPointArray(sizeof(check_mark)/(sizeof(QCOORD)*2), check_mark);
            amark.translate(x+1, y+1);

            if (flags & Style_On) {
                p->setPen(flags & Style_Enabled ? pal.shadow() : pal.dark());
                p->drawLineSegments(amark);
                amark.translate(-1, -1);
                p->setPen(flags & Style_Enabled ? QColor(255,0,0) : pal.dark().color());
                p->drawLineSegments(amark);
                p->setPen(oldPen);
            } else {
                p->setPen(flags & Style_Enabled ? pal.dark() : pal.mid());
                p->drawLineSegments(amark);
                amark.translate(-1, -1);
                p->setPen(flags & Style_Enabled ? QColor(230,120,120) : pal.dark().color());
                p->drawLineSegments(amark);
                p->setPen(oldPen);
            }
        }
        break;

    case PE_ExclusiveIndicator:
        {
            p->save();
            p->translate(x, y);

            p->setPen(pal.button());
            p->setBrush(hot ? pal.midlight() : pal.button());
            QPointArray a;
            a.setPoints(4, 5,0, 11,6, 6,11, 0,5);
            p->drawPolygon(a);

            p->setPen(pal.dark());
            p->drawLine(0,5, 5,0);
            p->drawLine(6,0, 11,5);
            p->setPen(flags & Style_Down ? pal.light() : pal.dark());
            p->drawLine(11,6, 6,11);
            p->drawLine(5,11, 0,6);
            p->drawLine(2,7, 5,10);
            p->drawLine(6,10, 9,7);
            p->setPen(pal.light());
            p->drawLine(2,5, 5,2);

            if (flags & Style_On) {
                p->setPen(flags & Style_Enabled ? Qt::blue : Qt::darkGray);
                p->setBrush(flags & Style_Enabled ? Qt::blue : Qt::darkGray);
                a.setPoints(3, 6,2, 8,4, 6,6);
                p->drawPolygon(a);
                p->setBrush(Qt::NoBrush);

                p->setPen(pal.shadow());
                p->drawLine(7,7, 9,5);
            } else {
                p->drawLine(6,2, 9,5);
            }
            p->restore();
        }
        break;

    case PE_ExclusiveIndicatorMask:
        {
            p->save();
            QPen oldPen = p->pen();
            QBrush oldBrush = p->brush();

            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            QPointArray a;
            a.setPoints(8, 0,5, 5,0, 6,0, 11,5, 11,6, 6,11, 5,11, 0,6);
            a.translate(x, y);
            p->drawPolygon(a);

            p->setBrush(oldBrush);
            p->setPen(oldPen);
            p->restore();
        }
        break;

    case PE_Panel:
        {
            const int lineWidth = opt.isDefault() ? defaultFrameWidth : opt.lineWidth();
            drawPanel(p, x, y, w, h, pal, flags & (Style_Sunken | Style_Down | Style_On), lineWidth, 0);
            if (lineWidth <= 1)
                return;

            // draw extra shadinglines
            QPen oldPen = p->pen();
            p->setPen(pal.midlight());
            p->drawLine(x+1, y+h-3, x+1, y+1);
            p->drawLine(x+1, y+1, x+w-3, y+1);
            p->setPen(pal.mid());
            p->drawLine(x+1, y+h-2, x+w-2, y+h-2);
            p->drawLine(x+w-2, y+h-2, x+w-2, y+1);
            p->setPen(oldPen);
        }
        break;

    case PE_ScrollBarSubLine:
        if (!r.contains(d->mousePos) && !(flags & Style_Active))
            flags &= ~Style_MouseOver;
        drawPrimitive(PE_ButtonCommand, p, r, pal, flags, opt);
        drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
                      p, r, pal, Style_Enabled | flags);
        break;

    case PE_ScrollBarAddLine:
        if (!r.contains(d->mousePos))
            flags &= ~Style_MouseOver;
        drawPrimitive(PE_ButtonCommand, p, r, pal, flags, opt);
        drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
                      p, r, pal, Style_Enabled | flags);
        break;

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage:
        if (!r.contains(d->mousePos))
            flags &= ~Style_MouseOver;
        if (r.isValid())
            qDrawShadePanel(p, x, y, w, h, pal, false, 1, hot ? &pal.brush(QPalette::Midlight) : &pal.brush(QPalette::Button));
        break;

    case PE_ScrollBarSlider:
        {
            if (!r.isValid())
                break;
            if (!(r.contains(d->mousePos) || flags & Style_Active) || !(flags & Style_Enabled))
                flags &= ~Style_MouseOver;

            QPixmap pm(r.width(), r.height());
            QPainter bp(&pm);
            drawPrimitive(PE_ButtonBevel, &bp, QRect(0,0,r.width(),r.height()), pal, flags | Style_Enabled | Style_Raised);
            if (flags & Style_Horizontal) {
                const int sliderM =  r.width() / 2;
                if (r.width() > 20) {
                    drawSeparator(&bp, sliderM-5, 2, sliderM-5, r.height()-3, pal);
                    drawSeparator(&bp, sliderM+3, 2, sliderM+3, r.height()-3, pal);
                }
                if (r.width() > 10)
                    drawSeparator(&bp, sliderM-1, 2, sliderM-1, r.height()-3, pal);

            } else {
                const int sliderM =  r.height() / 2;
                if (r.height() > 20) {
                    drawSeparator(&bp, 2, sliderM-5, r.width()-3, sliderM-5, pal);
                    drawSeparator(&bp, 2, sliderM+3, r.width()-3, sliderM+3, pal);
                }
                if (r.height() > 10)
                    drawSeparator(&bp, 2, sliderM-1, r.width()-3, sliderM-1, pal);
            }
            bp.end();
            p->drawPixmap(r.x(), r.y(), pm);
        }

        break;

    case PE_Splitter:
        {
            const int motifOffset = 10;
            int sw = pixelMetric(PM_SplitterWidth);
            if (flags & Style_Horizontal) {
                int xPos = x + w/2;
                int kPos = motifOffset;
                int kSize = sw - 2;

                qDrawShadeLine(p, xPos, kPos + kSize - 1 ,
                        xPos, h, pal);

                drawPrimitive(PE_ButtonBevel, p, QRect(xPos-sw/2+1, kPos, kSize, kSize+1), pal, flags, opt);
                qDrawShadeLine(p, xPos+2, 0, xPos, kPos, pal);
            } else {
                int yPos = y + h/2;
                int kPos = w - motifOffset - sw;
                int kSize = sw - 2;

                qDrawShadeLine(p, 0, yPos, kPos, yPos, pal);
                drawPrimitive(PE_ButtonBevel, p, QRect(kPos, yPos-sw/2+1, kSize+1, kSize), pal, flags, opt);
                qDrawShadeLine(p, kPos + kSize+1, yPos, w, yPos, pal);
            }
        }
        break;

    default:
        QMotifStyle::drawPrimitive(pe, p, r, pal, flags, opt);
        break;
    }
}

/*
void QSGIStyle::drawControl(ControlElement element,
                            QPainter *p,
                            const QWidget *widget,
                            const QRect &r,
                            const QPalette &pal,
                            SFlags flags,
                            const Q3StyleOption& opt) const
                            */

/*!
    \reimp
*/
void QSGIStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                            const QWidget *w) const
{
    if (widget == d->hotWidget)
        flags |= Style_MouseOver;

    switch (element) {
    case CE_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *btn = (QPushButton*)widget;
            int x1, y1, x2, y2;
            r.coords(&x1, &y1, &x2, &y2);

            p->setPen(pal.foreground());
            p->setBrush(QBrush(pal.button(),Qt::NoBrush));

            int diw = pixelMetric(QStyle::PM_ButtonDefaultIndicator);
            if (btn->isDefault() || btn->autoDefault()) {
                x1 += diw;
                y1 += diw;
                x2 -= diw;
                y2 -= diw;
            }

            QPointArray a;
            if (btn->isDefault()) {
                if (diw == 0) {
                    a.setPoints(9,
                                 x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
                                 x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1);
                    p->setPen(pal.shadow());
                    p->drawPolyline(a);
                    x1 += 2;
                    y1 += 2;
                    x2 -= 2;
                    y2 -= 2;
                } else {
                    qDrawShadePanel(p, btn->rect(), pal, true);
                }
            }

            QBrush fill = pal.brush(QPalette::Button);
            if (!btn->isFlat() || btn->isChecked() || btn->isDown())
                drawPrimitive(PE_ButtonBevel, p, QRect(x1, y1, x2-x1+1, y2-y1+1), pal, flags, opt);

            if (p->brush().style() != Qt::NoBrush)
                p->setBrush(Qt::NoBrush);
#endif
        }
    break;

    case CE_MenuItem:
        {
#ifndef QT_NO_POPUPMENU
            if(!widget || opt.isDefault())
                break;
            QAction *mi = opt.action();
            if(!mi)
                break;
            const QMenu *menu = (const QMenu *)widget;
            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = ! (flags & Style_Enabled);
            bool checkable = menu->isCheckable();
            bool act = flags & Style_Active;

            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            if (checkable)
                maxpmw = qMax(maxpmw, sgiCheckMarkSpace);
            int checkcol = maxpmw;

            if (mi && mi->isSeparator()) {
                p->setPen(pal.mid());
                p->drawLine(x, y, x+w, y);
                return;
            }

            int pw = sgiItemFrame;

            if (act && !dis) {
                if (pixelMetric(PM_DefaultFrameWidth) > 1)
                    drawPanel(p, x, y, w, h, pal, false, pw,
                                     &pal.brush(QPalette::Light));
                else
                    drawPanel(p, x+1, y+1, w-2, h-2, pal, false, 1,
                                     &pal.brush(QPalette::Light));
            } else {
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));
            }

            if (!mi)
                return;

            if (mi->isChecked()) {
                if(!mi->icon().isNull())
                    drawPanel(p, x+sgiItemFrame, y+sgiItemFrame, checkcol, h-2*sgiItemFrame,
                                     pal, true, 1, &pal.brush(QPalette::Light));
            } else {
                if (!act)
                    p->fillRect(x+sgiItemFrame, y+sgiItemFrame, checkcol, h-2*sgiItemFrame,
                                pal.brush(QPalette::Button));
            }

            if(!mi->icon().isNull()) {
                QIcon::Mode mode = QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checkable && mi->isChecked())
                    pixmap = mi->icon().pixmap(Qt::SmallIconSize, mode, QIcon::On);
                else
                    pixmap = mi->icon().pixmap(Qt::SmallIconSize, mode);

                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(x+sgiItemFrame, y+sgiItemFrame, checkcol, h-2*sgiItemFrame);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);
            } else {
                if (checkable) {
                    SFlags cflags = Style_Default;
                    if (! dis)
                        cflags |= Style_Enabled;
                    if (act)
                        cflags |= Style_On;

                    if (mi->isChecked()) {
                        QRect er(x+sgiItemFrame+1, y+sgiItemFrame+3,
                                  pixelMetric(PM_IndicatorWidth),
                                  pixelMetric(PM_IndicatorHeight));
                        er.addCoords(1, 1, -1, -1);
                        drawPrimitive(PE_ButtonBevel, p, er, pal, cflags, opt);
                        er.addCoords(0, 1, 1, 1);
                        drawPrimitive(PE_CheckMark, p, er, pal, cflags | Style_On, opt);
                    }
                }
            }

            p->setPen(pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            int xm = sgiItemFrame + checkcol + sgiItemHMargin;

            QString s = mi->text();
            if (s.size()) {
                int t = s.indexOf('\t');
                int m = sgiItemVMargin;
                const int text_flags = Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine; //special underline for &x

                QString miText = s;
                if (t>=0) {
                    p->drawText(x+w-tab-sgiItemHMargin-sgiItemFrame,
                                y+m, tab, h-2*m, text_flags, miText.mid(t+1));
                    miText = s.mid(0, t);
                }
                QRect br = p->fontMetrics().boundingRect(x+xm, y+m, w-xm-tab+1, h-2*m,
                        text_flags, mi->text());

                drawSGIPrefix(p, br.x()+p->fontMetrics().leftBearing(miText[0]),
                        br.y()+br.height()+p->fontMetrics().underlinePos()-2, &miText);
                p->drawText(x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, miText, miText.length());
            }
            if (mi->menu()) {
                int dim = (h-2*sgiItemFrame) / 2;
                drawPrimitive(PE_ArrowRight, p, QRect(x+w-sgiArrowHMargin-sgiItemFrame-dim, y+h/2-dim/2, dim, dim), pal, flags);
            }
#endif
        }
        break;

#ifdef QT_COMPAT
    case CE_Q3PopupMenuItem:
        {
#ifndef QT_NO_POPUPMENU
            if (! widget || opt.isDefault())
                break;
            Q3MenuItem *mi = opt.menuItem();
            if (!mi)
                break;
            const Q3PopupMenu *popupmenu = (const Q3PopupMenu *) widget;
            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = ! (flags & Style_Enabled);
            bool checkable = popupmenu->isCheckable();
            bool act = flags & Style_Active;
            int x, y, w, h;

            r.rect(&x, &y, &w, &h);

            if (checkable)
                maxpmw = qMax(maxpmw, sgiCheckMarkSpace);
            int checkcol = maxpmw;

            if (mi && mi->isSeparator()) {
                p->setPen(pal.mid());
                p->drawLine(x, y, x+w, y);
                return;
            }

            int pw = sgiItemFrame;

            if (act && !dis) {
                if (pixelMetric(PM_DefaultFrameWidth) > 1)
                    drawPanel(p, x, y, w, h, pal, false, pw,
                                     &pal.brush(QPalette::Light));
                else
                    drawPanel(p, x+1, y+1, w-2, h-2, pal, false, 1,
                                     &pal.brush(QPalette::Light));
            } else {
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));
            }

            if (!mi)
                return;

            if (mi->isChecked()) {
                if (mi->iconSet()) {
                    drawPanel(p, x+sgiItemFrame, y+sgiItemFrame, checkcol, h-2*sgiItemFrame,
                                     pal, true, 1, &pal.brush(QPalette::Light));
                }
            } else {
                if (!act)
                    p->fillRect(x+sgiItemFrame, y+sgiItemFrame, checkcol, h-2*sgiItemFrame,
                                pal.brush(QPalette::Button));
            }

            if (mi->iconSet()) {
                QIcon::Mode mode = QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checkable && mi->isChecked())
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode, QIcon::On);
                else
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode);

                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(x+sgiItemFrame, y+sgiItemFrame, checkcol, h-2*sgiItemFrame);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);
            } else {
                if (checkable) {
                    SFlags cflags = Style_Default;
                    if (! dis)
                        cflags |= Style_Enabled;
                    if (act)
                        cflags |= Style_On;

                    if (mi->isChecked()) {
                        QRect er(x+sgiItemFrame+1, y+sgiItemFrame+3,
                                  pixelMetric(PM_IndicatorWidth),
                                  pixelMetric(PM_IndicatorHeight));
                        er.addCoords(1, 1, -1, -1);
                        drawPrimitive(PE_ButtonBevel, p, er, pal, cflags, opt);
                        er.addCoords(0, 1, 1, 1);
                        drawPrimitive(PE_CheckMark, p, er, pal, cflags | Style_On, opt);
                    }
                }
            }

            p->setPen(pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            int xm = sgiItemFrame + checkcol + sgiItemHMargin;

            if (mi->custom()) {
                int m = sgiItemVMargin;
                p->save();
                mi->custom()->paint(p, pal, act, !dis,
                                     x+xm, y+m, w-xm-tab+1, h-2*m);
                p->restore();
            }

            QString s = mi->text();
            if (s.size()) {
                int t = s.indexOf('\t');
                int m = sgiItemVMargin;
                const int text_flags = Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine; //special underline for &x

                QString miText = s;
                if (t>=0) {
                    p->drawText(x+w-tab-sgiItemHMargin-sgiItemFrame,
                                y+m, tab, h-2*m, text_flags, miText.mid(t+1));
                    miText = s.mid(0, t);
                }
                QRect br = p->fontMetrics().boundingRect(x+xm, y+m, w-xm-tab+1, h-2*m,
                        text_flags, mi->text());

                drawSGIPrefix(p, br.x()+p->fontMetrics().leftBearing(miText[0]),
                        br.y()+br.height()+p->fontMetrics().underlinePos()-2, &miText);
                p->drawText(x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, miText, miText.length());
            } else {
                if (mi->pixmap()) {
                    QPixmap *pixmap = mi->pixmap();
                    if (pixmap->depth() == 1)
                        p->setBackgroundMode(Qt::OpaqueMode);
                    p->drawPixmap(x+xm, y+sgiItemFrame, *pixmap);
                    if (pixmap->depth() == 1)
                        p->setBackgroundMode(Qt::TransparentMode);
                }
            }
            if (mi->popup()) {
                int dim = (h-2*sgiItemFrame) / 2;
                drawPrimitive(PE_ArrowRight, p, QRect(x+w-sgiArrowHMargin-sgiItemFrame-dim, y+h/2-dim/2, dim, dim), pal, flags);
            }
#endif
        }
        break;
#endif

    case CE_MenuBarItem:
        {
#ifndef QT_NO_MENUDATA
            if (opt.isDefault())
                break;

            bool active = flags & Style_Active;
            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            if (active) {
                p->setPen(QPen(pal.shadow(), 1));
                p->drawRect(x, y, w, h);
                qDrawShadePanel(p, QRect(x+1,y+1,w-2,h-2), pal, false, 2,
                                 &pal.brush(QPalette::Light));
            } else {
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));
            }

            QAction *mi = opt.action();
            if (!mi->icon().isNull()) {
                QPixmap pix = mi->icon().pixmap(Qt::SmallIconSize, QIcon::Normal);
                drawItem(p, r, Qt::AlignCenter|Qt::TextDontClip|Qt::TextSingleLine,
                        pal, mi->isEnabled(), pix, "", -1, &pal.buttonText().color());
            }

            if (mi->text().size()) {
                QString* text = new QString(mi->text());
                QRect br = p->fontMetrics().boundingRect(x, y-2, w+1, h,
                        Qt::AlignCenter|Qt::TextDontClip|Qt::TextSingleLine|Qt::TextShowMnemonic, mi->text());

                drawSGIPrefix(p, br.x()+p->fontMetrics().leftBearing((*text)[0]),
                        br.y()+br.height()+p->fontMetrics().underlinePos()-2, text);
                p->drawText(x, y-2, w+1, h, Qt::AlignCenter|Qt::TextDontClip|Qt::TextSingleLine, *text, text->length());
                delete text;
            }
#endif
        }
        break;

#ifdef QT_COMPAT
    case CE_Q3MenuBarItem:
        {
#ifndef QT_NO_MENUDATA
            if (opt.isDefault())
                break;

            Q3MenuItem *mi = opt.menuItem();

            bool active = flags & Style_Active;
            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            if (active) {
                p->setPen(QPen(pal.shadow(), 1));
                p->drawRect(x, y, w, h);
                qDrawShadePanel(p, QRect(x+1,y+1,w-2,h-2), pal, false, 2,
                                 &pal.brush(QPalette::Light));
            } else {
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));
            }

            if (mi->pixmap())
                drawItem(p, r, Qt::AlignCenter|Qt::TextDontClip|Qt::TextSingleLine,
                        pal, mi->isEnabled(), *mi->pixmap(), "", -1, &pal.buttonText().color());

            if (mi->text().size()) {
                QString* text = new QString(mi->text());
                QRect br = p->fontMetrics().boundingRect(x, y-2, w+1, h,
                        Qt::AlignCenter|Qt::TextDontClip|Qt::TextSingleLine|Qt::TextShowMnemonic, mi->text());

                drawSGIPrefix(p, br.x()+p->fontMetrics().leftBearing((*text)[0]),
                        br.y()+br.height()+p->fontMetrics().underlinePos()-2, text);
                p->drawText(x, y-2, w+1, h, Qt::AlignCenter|Qt::TextDontClip|Qt::TextSingleLine, *text, text->length());
                delete text;
            }
#endif
        }
        break;
#endif

    case CE_CheckBox:
        QMotifStyle::drawControl(element, p, widget, r, pal, flags, opt);
        break;

    default:
        QMotifStyle::drawControl(element, p, widget, r, pal, flags, opt);
        break;
    }
}

/* void QSGIStyle::drawComplexControl(ComplexControl control,
                                   QPainter *p,
                                   const QWidget* widget,
                                   const QRect& r,
                                   const QPalette& pal,
                                   SFlags flags,
                                   SCFlags sub,
                                   SCFlags subActive,
                                   const Q3StyleOption& opt) const */
/*!
    \reimp
*/
void QSGIStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                   const QWidget *w) const
{
    if (widget == d->hotWidget)
        flags |= Style_MouseOver;

    switch (control) {
    case CC_Slider:
        {
#ifndef QT_NO_SLIDER
            const QSlider * slider = (const QSlider *) widget;

            QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
                                                  opt),
                  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
                                                  opt);

            if ((sub & SC_SliderGroove) && groove.isValid()) {
                QRegion region(groove);
                if ((sub & SC_SliderHandle) && handle.isValid())
                    region = region.subtract(handle);
                if (d->lastSliderRect.slider == slider && d->lastSliderRect.rect.isValid())
                    region = region.subtract(d->lastSliderRect.rect);
                p->setClipRegion(region);

                QRect grooveTop = groove;
                grooveTop.addCoords(1, 1, -1, -1);
                drawPrimitive(PE_ButtonBevel, p, grooveTop, pal, flags & ~Style_MouseOver, opt);

                if (flags & Style_HasFocus) {
                    QRect fr = subRect(SR_SliderFocusRect, widget);
                    drawPrimitive(PE_FocusRect, p, fr, pal, flags & ~Style_MouseOver);
                }

                if (d->lastSliderRect.slider == slider && d->lastSliderRect.rect.isValid()) {
                    if ((sub & SC_SliderHandle) && handle.isValid()) {
                        region = widget->rect();
                        region = region.subtract(handle);
                        p->setClipRegion(region);
                    } else {
                        p->setClipping(false);
                    }
                    qDrawShadePanel(p, d->lastSliderRect.rect, pal, true, 1, &pal.brush(QPalette::Dark));
                }
                p->setClipping(false);
            }

            if ((sub & SC_SliderHandle) && handle.isValid()) {
                if (flags & Style_MouseOver && !handle.contains(d->mousePos) && subActive != SC_SliderHandle)
                    flags &= ~Style_MouseOver;
                drawPrimitive(PE_ButtonBevel, p, handle, pal, flags);

                if (slider->orientation() == Qt::Horizontal) {
                    QCOORD mid = handle.x() + handle.width() / 2;
                    qDrawShadeLine(p, mid, handle.y(), mid,
                                    handle.y() + handle.height() - 2,
                                    pal, true, 1);
                } else {
                    QCOORD mid = handle.y() + handle.height() / 2;
                    qDrawShadeLine(p, handle.x(), mid,
                                    handle.x() + handle.width() - 2, mid,
                                    pal, true, 1);
                }
            }

            if (sub & SC_SliderTickmarks)
                QMotifStyle::drawComplexControl(control, p, widget, r, pal, flags,
                                                 SC_SliderTickmarks, subActive,
                                                 opt);
#endif
            break;
        }
    case CC_ComboBox:
        {
#ifndef QT_NO_COMBOBOX
            const QComboBox * cb = (QComboBox *) widget;

            if (sub & SC_ComboBoxFrame) {
                QRect fr =
                    QStyle::visualRect(querySubControlMetrics(CC_ComboBox, cb,
                                                                SC_ComboBoxFrame), cb);
                drawPrimitive(PE_ButtonBevel, p, fr, pal, flags);
            }

            if (sub & SC_ComboBoxArrow) {
                p->save();
                QRect er =
                    QStyle::visualRect(querySubControlMetrics(CC_ComboBox, cb, SC_ComboBoxArrow), cb);

                er.addCoords(0, 3, 0, 0);

                drawPrimitive(PE_ArrowDown, p, er, pal, flags | Style_Enabled, opt);

                int awh, ax, ay, sh, sy, dh, ew;
                get_combo_parameters(widget->rect(), ew, awh, ax, ay, sh, dh, sy);

                QBrush arrow = pal.brush(QPalette::Dark);
                p->fillRect(ax, sy-1, awh, sh, arrow);

                p->restore();
                if (cb->hasFocus()) {
                    QRect re = QStyle::visualRect(subRect(SR_ComboBoxFocusRect, cb), cb);
                    drawPrimitive(PE_FocusRect, p, re, pal);
                }
            }
            if (sub & SC_ComboBoxEditField) {
                if (cb->editable()) {
                    QRect er =
                        QStyle::visualRect(querySubControlMetrics(CC_ComboBox, cb,
                                                                    SC_ComboBoxEditField), cb);
                    er.addCoords(-1, -1, 1, 1);
                    qDrawShadePanel(p, QRect(er.x()-1, er.y()-1,
                                               er.width()+2, er.height()+2),
                                     pal, true, 1, &pal.brush(QPalette::Button));
                }
            }
#endif
            p->setPen(pal.buttonText());
            break;
        }

    case CC_ScrollBar:
        {
#ifndef QT_NO_SCROLLBAR
            QScrollBar *scrollbar = (QScrollBar*)widget;
            bool maxedOut = (scrollbar->minimum() == scrollbar->maximum());
            if (maxedOut)
                flags &= ~Style_Enabled;

            QRect handle = QStyle::visualRect(querySubControlMetrics(CC_ScrollBar, widget, SC_ScrollBarSlider, opt), widget);

            if (sub & SC_ScrollBarGroove) {
            }
            if (sub & SC_ScrollBarAddLine) {
                QRect er = QStyle::visualRect(querySubControlMetrics(CC_ScrollBar, widget, SC_ScrollBarAddLine, opt), widget);
                drawPrimitive(PE_ScrollBarAddLine, p, er, pal, flags, opt);
            }
            if (sub & SC_ScrollBarSubLine) {
                QRect er = QStyle::visualRect(querySubControlMetrics(CC_ScrollBar, widget, SC_ScrollBarSubLine, opt), widget);
                drawPrimitive(PE_ScrollBarSubLine, p, er, pal, flags, opt);
            }
            if (sub & SC_ScrollBarAddPage) {
                QRect er = QStyle::visualRect(querySubControlMetrics(CC_ScrollBar, widget, SC_ScrollBarAddPage, opt), widget);
                QRegion region(er);
                if (d->lastScrollbarRect.scrollbar == scrollbar &&
                     d->lastScrollbarRect.rect.isValid() &&
                     er.intersects(d->lastScrollbarRect.rect)) {
                    region = region.subtract(d->lastScrollbarRect.rect);
                    p->setClipRegion(region);
                }
                if (sub & SC_ScrollBarSlider && er.intersects(handle)) {
                    region = region.subtract(handle);
                    p->setClipRegion(region);
                }

                drawPrimitive(PE_ScrollBarAddPage, p, er, pal, flags & ~Style_MouseOver, opt);

                if (d->lastScrollbarRect.scrollbar == scrollbar &&
                     d->lastScrollbarRect.rect.isValid() &&
                     er.intersects(d->lastScrollbarRect.rect)) {
                    if (sub & SC_ScrollBarSlider && handle.isValid()) {
                        region = er;
                        region.subtract(handle);
                        p->setClipRegion(region);
                    } else {
                        p->setClipping(false);
                    }
                    qDrawShadePanel(p, d->lastScrollbarRect.rect, pal, true, 1, &pal.brush(QPalette::Dark));
                }
                p->setClipping(false);
            }
            if (sub & SC_ScrollBarSubPage) {
                QRect er = QStyle::visualRect(querySubControlMetrics(CC_ScrollBar, widget, SC_ScrollBarSubPage, opt), widget);
                QRegion region(er);
                if (d->lastScrollbarRect.scrollbar == scrollbar &&
                     d->lastScrollbarRect.rect.isValid() &&
                     er.intersects(d->lastScrollbarRect.rect)) {
                    region = region.subtract(d->lastScrollbarRect.rect);
                    p->setClipRegion(region);
                }
                if (sub & SC_ScrollBarSlider && er.intersects(handle)) {
                    region = region.subtract(handle);
                    p->setClipRegion(region);
                }
                drawPrimitive(PE_ScrollBarSubPage, p, er, pal, flags & ~Style_MouseOver, opt);
                if (d->lastScrollbarRect.scrollbar == scrollbar &&
                     d->lastScrollbarRect.rect.isValid() &&
                     er.intersects(d->lastScrollbarRect.rect)) {
                    if (sub & SC_ScrollBarSlider && handle.isValid()) {
                        region = er;
                        region.subtract(handle);
                        p->setClipRegion(region);
                    } else {
                        p->setClipping(false);
                    }
                    qDrawShadePanel(p, d->lastScrollbarRect.rect, pal, true, 1, &pal.brush(QPalette::Dark));
                }
                p->setClipping(false);
            }
            if (sub & SC_ScrollBarSlider) {
                p->setClipping(false);
                if (subActive == SC_ScrollBarSlider)
                    flags |= Style_Active;

                drawPrimitive(PE_ScrollBarSlider, p, handle, pal, flags, opt);
            }
#endif
        }
        break;

    default:
        QMotifStyle::drawComplexControl(control, p, widget, r, pal, flags, sub, subActive, opt);
        break;
    }
}

/*
QSize QSGIStyle::sizeFromContents(ContentsType contents,
                                  const QWidget *widget,
                                  const QSize &contentsSize,
                                  const Q3StyleOption& opt) const
                                  */
/*!
    \reimp
*/
QSize QSGIStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                  const QSize &contentsSize, const QWidget *widget) const
{
    QSize sz(contentsSize);

    switch(contents) {
    case CT_MenuItem:
        {
#ifndef QT_NO_MENU
            if (!widget || opt.isDefault())
                break;
            sz = QMotifStyle::sizeFromContents(contents, widget, contentsSize, opt);

            QAction *mi = opt.action();
            // SGI checkmark items needs a bit more room
            const QMenu *menu = (QMenu *) widget;
            if(menu && menu->isCheckable())
                sz.setWidth(sz.width() + 8);
            // submenu indicator needs a bit more room
            if (mi->menu())
                sz.setWidth(sz.width() + sgiTabSpacing);
#endif
            break;
        }

#ifdef QT_COMPAT
    case CT_Q3PopupMenuItem:
        {
#ifndef QT_NO_POPUPMENU
            if (! widget || opt.isDefault())
                break;

            Q3MenuItem *mi = opt.menuItem();
            sz = QMotifStyle::sizeFromContents(contents, widget, contentsSize,
                                                opt);
            // SGI checkmark items needs a bit more room
            const Q3PopupMenu *popup = (Q3PopupMenu *) widget;
            if (popup && popup->isCheckable())
                sz.setWidth(sz.width() + 8);
            // submenu indicator needs a bit more room
            if (mi->popup())
                sz.setWidth(sz.width() + sgiTabSpacing);
#endif
            break;
        }
#endif

    default:
        sz = QMotifStyle::sizeFromContents(contents, widget, contentsSize, opt);
        break;
    }

    return sz;
}

//QRect QSGIStyle::subRect(SubRect r, const QWidget *widget) const
/*!
    \reimp
*/
QRect QSGIStyle::subRect(SubRect r, const QStyleOption *opt, const QWidget *widget) const
{
    QRect rect;

    switch (r) {
    case SR_ComboBoxFocusRect:
        {
            int awh, ax, ay, sh, sy, dh, ew;
            int fw = pixelMetric(PM_DefaultFrameWidth, widget);
            QRect tr = widget->rect();

            tr.addCoords(fw, fw, -fw, -fw);
            get_combo_parameters(tr, ew, awh, ax, ay, sh, dh, sy);
            rect.setRect(ax-2, ay-2, awh+4, awh+sh+dh+4);
        }
        break;
    default:
        return QMotifStyle::subRect(r, widget);
    }

    return rect;
}

/*
QRect QSGIStyle::querySubControlMetrics(ComplexControl control,
                                        const QWidget *widget,
                                        SubControl sub,
                                        const Q3StyleOption& opt) const
                                        */

/*!
    \reimp
*/
QRect QSGIStyle::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                        SubControl sc, const QWidget *w) const
{
    switch (control) {
    case CC_ComboBox:
        switch (sub) {
        case SC_ComboBoxFrame:
            return widget->rect();

        case SC_ComboBoxArrow: {
            int ew, awh, sh, dh, ax, ay, sy;
            int fw = pixelMetric(PM_DefaultFrameWidth, widget);
            QRect cr = widget->rect();
            cr.addCoords(fw, fw, -fw, -fw);
            get_combo_parameters(cr, ew, awh, ax, ay, sh, dh, sy);
            return QRect(ax, ay, awh, awh); }

        case SC_ComboBoxEditField: {
            int fw = pixelMetric(PM_DefaultFrameWidth, widget);
            QRect rect = widget->rect();
            rect.addCoords(fw, fw, -fw, -fw);
            int ew = get_combo_extra_width(rect.height());
            rect.addCoords(1, 1, -1-ew, -1);
            return rect; }

        default:
            break;
        }
        break;
        case CC_ScrollBar:
            return QCommonStyle::querySubControlMetrics(control, widget, sub, opt);
        default: break;
    }
    return QMotifStyle::querySubControlMetrics(control, widget, sub, opt);
}

#endif // QT_NO_STYLE_SGI
