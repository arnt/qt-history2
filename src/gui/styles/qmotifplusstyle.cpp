/****************************************************************************
**
** Implementation of QMotifPlusStyle class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmotifplusstyle.h"

#if !defined(QT_NO_STYLE_MOTIFPLUS) || defined(QT_PLUGIN)

#include "qevent.h"
#include "qmenu.h"
#include "q3menubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qframe.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qradiobutton.h"
#include "qcombobox.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qslider.h"
#include "qdrawutil.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qtoolbar.h"
#include "qpointer.h"
#include "qlayout.h"

struct QMotifPlusStylePrivate
{
    QMotifPlusStylePrivate()
        : hoverWidget(0), hovering(false), sliderActive(false), mousePressed(false),
          scrollbarElement(0), ref(1) {}

    QPointer<QWidget> hoverWidget;
    bool hovering, sliderActive, mousePressed;
    int scrollbarElement, ref;
    QPoint mousePos;
};

static QMotifPlusStylePrivate * singleton = 0;


static void drawMotifPlusShade(QPainter *p, const QRect &r,
                               const QPalette &pal, bool sunken, bool mouseover,
                               const QBrush *fill = 0)
{
    QPen oldpen = p->pen();
    QPointArray a(4);
    QColor button = mouseover ? pal.midlight() : pal.button();
    QBrush brush = mouseover ? pal.brush(QPalette::Midlight) : pal.brush(QPalette::Button);
    int x, y, w, h;

    r.rect(&x, &y, &w, &h);

    if (sunken) p->setPen(pal.dark()); else p->setPen(pal.light());
    a.setPoint(0, x, y + h - 1);
    a.setPoint(1, x, y);
    a.setPoint(2, x, y);
    a.setPoint(3, x + w - 1, y);
    p->drawLineSegments(a);

    if (sunken) p->setPen(Qt::black); else p->setPen(button);
    a.setPoint(0, x + 1, y + h - 2);
    a.setPoint(1, x + 1, y + 1);
    a.setPoint(2, x + 1, y + 1);
    a.setPoint(3, x + w - 2, y + 1);
    p->drawLineSegments(a);

    if (sunken) p->setPen(button); else p->setPen(pal.dark());
    a.setPoint(0, x + 2, y + h - 2);
    a.setPoint(1, x + w - 2, y + h - 2);
    a.setPoint(2, x + w - 2, y + h - 2);
    a.setPoint(3, x + w - 2, y + 2);
    p->drawLineSegments(a);

    if (sunken) p->setPen(pal.light()); else p->setPen(Qt::black);
    a.setPoint(0, x + 1, y + h - 1);
    a.setPoint(1, x + w - 1, y + h - 1);
    a.setPoint(2, x + w - 1, y + h - 1);
    a.setPoint(3, x + w - 1, y);
    p->drawLineSegments(a);

    if (fill)
        p->fillRect(x + 2, y + 2, w - 4, h - 4, *fill);
    else
        p->fillRect(x + 2, y + 2, w - 4, h - 4, brush);

    p->setPen(oldpen);
}


/*!
    \class QMotifPlusStyle qmotifplusstyle.h
    \brief The QMotifPlusStyle class provides a more sophisticated Motif-ish look and feel.

    \ingroup appearance

    This class implements a Motif-ish look and feel with the more
    sophisticated bevelling as used by the GIMP Toolkit (GTK+) for
    Unix/X11.
*/

/*!
    Constructs a QMotifPlusStyle

    If \a hoveringHighlight is true (the default), then the style will
    not highlight push buttons, checkboxes, radiobuttons, comboboxes,
    scrollbars or sliders.
*/
QMotifPlusStyle::QMotifPlusStyle(bool hoveringHighlight) : QMotifStyle(true)
{
    if (!singleton)
        singleton = new QMotifPlusStylePrivate;
    else
        singleton->ref++;

    useHoveringHighlight = hoveringHighlight;
}

/*! \reimp */
QMotifPlusStyle::~QMotifPlusStyle()
{
    if (singleton && --singleton->ref <= 0) {
        delete singleton;
        singleton = 0;
    }
}


/*! \reimp */
void QMotifPlusStyle::polish(QPalette &)
{
}


/*! \reimp */
void QMotifPlusStyle::polish(QWidget *widget)
{
#ifndef QT_NO_FRAME
    if (qt_cast<QFrame*>(widget) && ((QFrame *) widget)->frameStyle() == QFrame::Panel)
        ((QFrame *) widget)->setFrameStyle(QFrame::WinPanel);
#endif

#ifndef QT_NO_MENUBAR
    if (qt_cast<Q3MenuBar*>(widget) && ((Q3MenuBar *) widget)->frameStyle() != QFrame::NoFrame)
        ((Q3MenuBar *) widget)->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
#endif

#ifndef QT_NO_TOOLBAR
    if (qt_cast<QToolBar*>(widget))
        widget->layout()->setMargin(2);
#endif
    if (useHoveringHighlight) {
        if (qt_cast<QAbstractButton*>(widget) || qt_cast<QComboBox*>(widget))
            widget->installEventFilter(this);

        if (qt_cast<QScrollBar*>(widget) || qt_cast<QSlider*>(widget)) {
            widget->setMouseTracking(true);
            widget->installEventFilter(this);
        }
    }

    QMotifStyle::polish(widget);
}


/*! \reimp */
void QMotifPlusStyle::unPolish(QWidget *widget)
{
    widget->removeEventFilter(this);
    QMotifStyle::unPolish(widget);
}


/*! \reimp */
void QMotifPlusStyle::polish(QApplication *)
{
}


/*! \reimp */
void QMotifPlusStyle::unPolish(QApplication *)
{
}


/*! \reimp */
int QMotifPlusStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ScrollBarExtent:
        ret = 15;
        break;

    case PM_ButtonDefaultIndicator:
        ret = 5;
        break;

    case PM_ButtonMargin:
        ret = 4;
        break;

    case PM_SliderThickness:
        ret = 15;
        break;

    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        ret = 10;
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        ret = 11;
        break;

    default:
        ret = QMotifStyle::pixelMetric(metric, widget);
        break;
    }

    return ret;
}


/*! \reimp */
void QMotifPlusStyle::drawPrimitive(PrimitiveElement pe,
                                     QPainter *p, const QRect &r, const QPalette &pal,
                                     SFlags flags, const QStyleOption& opt) const
{
    switch (pe) {
    case PE_HeaderSection:

    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
        if (flags & (Style_Down | Style_On | Style_Raised | Style_Sunken))
            drawMotifPlusShade(p, r, pal, bool(flags & (Style_Down | Style_On)),
                                bool(flags & Style_MouseOver));
        else if (flags & Style_MouseOver)
            p->fillRect(r, pal.brush(QPalette::Midlight));
        else
            p->fillRect(r, pal.brush(QPalette::Button));
        break;

    case PE_MenuBarFrame:
    case PE_MenuFrame:
    case PE_Panel:
    case PE_PanelPopup:
    case PE_PanelMenuBar:
    case PE_PanelDockWindow:
        if (opt.lineWidth())
            drawMotifPlusShade(p, r, pal, (flags & Style_Sunken), (flags & Style_MouseOver));
        else if (flags & Style_MouseOver)
            p->fillRect(r, pal.brush(QPalette::Midlight));
        else
            p->fillRect(r, pal.brush(QPalette::Button));
        break;

    case PE_SpinWidgetUp:
        drawPrimitive(PE_ArrowUp, p, r, pal, flags, opt);
        break;

    case PE_SpinWidgetDown:
        drawPrimitive(PE_ArrowDown, p, r, pal, flags, opt);
        break;

    case PE_Indicator:
        {
            QBrush fill;
            if (flags & Style_On)
                fill = pal.brush(QPalette::Mid);
            else if (flags & Style_MouseOver)
                fill = pal.brush(QPalette::Midlight);
            else
                fill = pal.brush(QPalette::Button);

            if (flags & Style_NoChange) {
                qDrawPlainRect(p, r, pal.text(), 1, &fill);
                p->drawLine(r.topRight(), r.bottomLeft());
            } else
                drawMotifPlusShade(p, r, pal, (flags & Style_On),
                                   (flags & Style_MouseOver), &fill);
            break;
        }

    case PE_ExclusiveIndicator:
        {
            QPen oldpen =  p->pen();
            QPointArray thick(8);
            QPointArray thin(4);
            QColor button = ((flags & Style_MouseOver) ? pal.midlight() : pal.button());
            QBrush brush = ((flags & Style_MouseOver) ?
                            pal.brush(QPalette::Midlight) :
                            pal.brush(QPalette::Button));
            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            p->fillRect(x, y, w, h, brush);


            if (flags & Style_On) {
                thick.setPoint(0, x, y + (h / 2));
                thick.setPoint(1, x + (w / 2), y);
                thick.setPoint(2, x + 1, y + (h / 2));
                thick.setPoint(3, x + (w / 2), y + 1);
                thick.setPoint(4, x + (w / 2), y);
                thick.setPoint(5, x + w - 1, y + (h / 2));
                thick.setPoint(6, x + (w / 2), y + 1);
                thick.setPoint(7, x + w - 2, y + (h / 2));
                p->setPen(pal.dark());
                p->drawLineSegments(thick);

                thick.setPoint(0, x + 1, y + (h / 2) + 1);
                thick.setPoint(1, x + (w / 2), y + h - 1);
                thick.setPoint(2, x + 2, y + (h / 2) + 1);
                thick.setPoint(3, x + (w / 2), y + h - 2);
                thick.setPoint(4, x + (w / 2), y + h - 1);
                thick.setPoint(5, x + w - 2, y + (h / 2) + 1);
                thick.setPoint(6, x + (w / 2), y + h - 2);
                thick.setPoint(7, x + w - 3, y + (h / 2) + 1);
                p->setPen(pal.light());
                p->drawLineSegments(thick);

                thin.setPoint(0, x + 2, y + (h / 2));
                thin.setPoint(1, x + (w / 2), y + 2);
                thin.setPoint(2, x + (w / 2), y + 2);
                thin.setPoint(3, x + w - 3, y + (h / 2));
                p->setPen(Qt::black);
                p->drawLineSegments(thin);

                thin.setPoint(0, x + 3, y + (h / 2) + 1);
                thin.setPoint(1, x + (w / 2), y + h - 3);
                thin.setPoint(2, x + (w / 2), y + h - 3);
                thin.setPoint(3, x + w - 4, y + (h / 2) + 1);
                p->setPen(pal.mid());
                p->drawLineSegments(thin);
            } else {
                thick.setPoint(0, x, y + (h / 2));
                thick.setPoint(1, x + (w / 2), y);
                thick.setPoint(2, x + 1, y + (h / 2));
                thick.setPoint(3, x + (w / 2), y + 1);
                thick.setPoint(4, x + (w / 2), y);
                thick.setPoint(5, x + w - 1, y + (h / 2));
                thick.setPoint(6, x + (w / 2), y + 1);
                thick.setPoint(7, x + w - 2, y + (h / 2));
                p->setPen(pal.light());
                p->drawLineSegments(thick);

                thick.setPoint(0, x + 2, y + (h / 2) + 1);
                thick.setPoint(1, x + (w / 2), y + h - 2);
                thick.setPoint(2, x + 3, y + (h / 2) + 1);
                thick.setPoint(3, x + (w / 2), y + h - 3);
                thick.setPoint(4, x + (w / 2), y + h - 2);
                thick.setPoint(5, x + w - 3, y + (h / 2) + 1);
                thick.setPoint(6, x + (w / 2), y + h - 3);
                thick.setPoint(7, x + w - 4, y + (h / 2) + 1);
                p->setPen(pal.dark());
                p->drawLineSegments(thick);

                thin.setPoint(0, x + 2, y + (h / 2));
                thin.setPoint(1, x + (w / 2), y + 2);
                thin.setPoint(2, x + (w / 2), y + 2);
                thin.setPoint(3, x + w - 3, y + (h / 2));
                p->setPen(button);
                p->drawLineSegments(thin);

                thin.setPoint(0, x + 1, y + (h / 2) + 1);
                thin.setPoint(1, x + (w / 2), y + h - 1);
                thin.setPoint(2, x + (w / 2), y + h - 1);
                thin.setPoint(3, x + w - 2, y + (h / 2) + 1);
                p->setPen(Qt::black);
                p->drawLineSegments(thin);
            }

            p->setPen(oldpen);
            break;
        }



    case PE_ArrowDown:
    case PE_ArrowLeft:
    case PE_ArrowRight:
    case PE_ArrowUp:
        {
            QPen oldpen = p->pen();
            QBrush oldbrush = p->brush();
            QPointArray poly(3);
            QColor button = (flags & Style_MouseOver) ? pal.midlight() : pal.button();
            bool down = (flags & Style_Down);
            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            p->save();
            p->setBrush(button);

            switch (pe) {
            case PE_ArrowUp:
                {
                    poly.setPoint(0, x + (w / 2), y);
                    poly.setPoint(1, x, y + h - 1);
                    poly.setPoint(2, x + w - 1, y + h - 1);
                    p->drawPolygon(poly);

                    if (down)
                        p->setPen(button);
                    else
                        p->setPen(pal.dark());
                    p->drawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);

                    if (down)
                        p->setPen(pal.light());
                    else
                        p->setPen(black);
                    p->drawLine(x, y + h - 1, x + w - 1, y + h - 1);

                    if (down)
                        p->setPen(button);
                    else
                        p->setPen(pal.dark());
                    p->drawLine(x + w - 2, y + h - 1, x + (w / 2), y + 1);

                    if (down)
                        p->setPen(pal.light());
                    else
                        p->setPen(black);
                    p->drawLine(x + w - 1, y + h - 1, x + (w / 2), y);

                    if (down)
                        p->setPen(black);
                    else
                        p->setPen(button);
                    p->drawLine(x + (w / 2), y + 1, x + 1, y + h - 1);

                    if (down)
                        p->setPen(pal.dark());
                    else
                        p->setPen(pal.light());
                    p->drawLine(x + (w / 2), y, x, y + h - 1);
                    break;
                }

            case PE_ArrowDown:
                {
                    poly.setPoint(0, x + w - 1, y);
                    poly.setPoint(1, x, y);
                    poly.setPoint(2, x + (w / 2), y + h - 1);
                    p->drawPolygon(poly);

                    if (down)
                        p->setPen(black);
                    else
                        p->setPen(button);
                    p->drawLine(x + w - 2, y + 1, x + 1, y + 1);

                    if (down)
                        p->setPen(pal.dark());
                    else
                        p->setPen(pal.light());
                    p->drawLine(x + w - 1, y, x, y);

                    if (down)
                        p->setPen(black);
                    else
                        p->setPen(button);
                    p->drawLine(x + 1, y, x + (w / 2), y + h - 2);

                    if (down)
                        p->setPen(pal.dark());
                    else
                        p->setPen(pal.light());
                    p->drawLine(x, y, x + (w / 2), y + h - 1);

                    if (down)
                        p->setPen(button);
                    else
                        p->setPen(pal.dark());
                    p->drawLine(x + (w / 2), y + h - 2, x + w - 2, y);

                    if (down)
                        p->setPen(pal.light());
                    else
                        p->setPen(black);
                    p->drawLine(x + (w / 2), y + h - 1, x + w - 1, y);
                    break;
                }

            case PE_ArrowLeft:
                {
                    poly.setPoint(0, x, y + (h / 2));
                    poly.setPoint(1, x + w - 1, y + h - 1);
                    poly.setPoint(2, x + w - 1, y);
                    p->drawPolygon(poly);

                    if (down)
                        p->setPen(button);
                    else
                        p->setPen(pal.dark());
                    p->drawLine(x + 1, y + (h / 2), x + w - 1, y + h - 1);

                    if (down)
                        p->setPen(pal.light());
                    else
                        p->setPen(black);
                    p->drawLine(x, y + (h / 2), x + w - 1, y + h - 1);

                    if (down)
                        p->setPen(button);
                    else
                        p->setPen(pal.dark());
                    p->drawLine(x + w - 2, y + h - 1, x + w - 2, y + 1);

                    if (down)
                        p->setPen(pal.light());
                    else
                        p->setPen(black);
                    p->drawLine(x + w - 1, y + h - 1, x + w - 1, y);

                    if (down)
                        p->setPen(black);
                    else
                        p->setPen(button);
                    p->drawLine(x + w - 1, y + 1, x + 1, y + (h / 2));

                    if (down)
                        p->setPen(pal.dark());
                    else
                        p->setPen(pal.light());
                    p->drawLine(x + w - 1, y, x, y + (h / 2));
                    break;
                }

            case PE_ArrowRight:
                {
                    poly.setPoint(0, x + w - 1, y + (h / 2));
                    poly.setPoint(1, x, y);
                    poly.setPoint(2, x, y + h - 1);
                    p->drawPolygon(poly);

                    if (down)
                        p->setPen(black);
                    else
                        p->setPen(button);
                    p->drawLine(x + w - 1, y + (h / 2), x + 1, y + 1);

                    if (down)
                        p->setPen(pal.dark());
                    else
                        p->setPen(pal.light());
                    p->drawLine(x + w - 1, y + (h / 2), x, y);

                    if (down)
                        p->setPen(black);
                    else
                        p->setPen(button);
                    p->drawLine(x + 1, y + 1, x + 1, y + h - 2);

                    if (down)
                        p->setPen(pal.dark());
                    else
                        p->setPen(pal.light());
                    p->drawLine(x, y, x, y + h - 1);

                    if (down)
                        p->setPen(button);
                    else
                        p->setPen(pal.dark());
                    p->drawLine(x + 1, y + h - 2, x + w - 1, y + (h / 2));

                    if (down)
                        p->setPen(pal.light());
                    else
                        p->setPen(black);
                    p->drawLine(x, y + h - 1, x + w - 1, y + (h / 2));
                    break;
                }

            default:
                break;
            }

            p->restore();
            p->setBrush(oldbrush);
            p->setPen(oldpen);
            break;
        }

    default:
        QMotifStyle::drawPrimitive(pe, p, r, pal, flags, opt);
        break;
    }
}


/*! \reimp
*/
void QMotifPlusStyle::drawControl(ControlElement element,
                                   QPainter *p,
                                   const QWidget *widget,
                                   const QRect &r,
                                   const QPalette &pal,
                                   SFlags flags,
                                   const QStyleOption& opt) const
{
    if (widget == singleton->hoverWidget)
        flags |= Style_MouseOver;

    switch (element) {
    case CE_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            QRect br = r;
            int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

            if (button->isDefault() || button->autoDefault()) {
                if (button->isDefault())
                    drawMotifPlusShade(p, br, pal, true, false,
                                       &pal.brush(QPalette::Background));

                br.setCoords(br.left()   + dbi,
                             br.top()    + dbi,
                             br.right()  - dbi,
                             br.bottom() - dbi);
            }

            if (flags & Style_HasFocus)
                br.addCoords(1, 1, -1, -1);
            p->save();
            drawPrimitive(PE_ButtonCommand, p, br, pal, flags);
            p->restore();
#endif
            break;
        }

    case CE_CheckBoxLabel:
        {
#ifndef QT_NO_CHECKBOX
            const QCheckBox *checkbox = (const QCheckBox *) widget;

            if (flags & Style_MouseOver) {
                QRegion r(checkbox->rect());
                r -= visualRect(subRect(SR_CheckBoxIndicator, widget), widget);
                p->setClipRegion(r);
                p->fillRect(checkbox->rect(), pal.brush(QPalette::Midlight));
                p->setClipping(false);
            }

            int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
            drawItem(p, r, alignment | AlignVCenter | ShowPrefix, pal,
                     flags & Style_Enabled,
                     !checkbox->icon().isNull() ? checkbox->icon().pixmap(QIconSet::Small,
                                                                          QIconSet::Normal)
                                                : QPixmap(),
                     checkbox->text());

            if (checkbox->hasFocus()) {
                QRect fr = visualRect(subRect(SR_CheckBoxFocusRect, widget), widget);
                drawPrimitive(PE_FocusRect, p, fr, pal, flags);
            }
#endif
            break;
        }

    case CE_RadioButtonLabel:
        {
#ifndef QT_NO_RADIOBUTTON
            const QRadioButton *radiobutton = (const QRadioButton *) widget;

            if (flags & Style_MouseOver) {
                QRegion r(radiobutton->rect());
                r -= visualRect(subRect(SR_RadioButtonIndicator, widget), widget);
                p->setClipRegion(r);
                p->fillRect(radiobutton->rect(), pal.brush(QPalette::Midlight));
                p->setClipping(false);
            }

            int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
            drawItem(p, r, alignment | AlignVCenter | ShowPrefix, pal,
                     flags & Style_Enabled,
                    !radiobutton->icon().isNull() ? radiobutton->icon().pixmap(QIconSet::Small,
                                                                               QIconSet::Normal)
                                                  : QPixmap(),
                     radiobutton->text());

            if (radiobutton->hasFocus()) {
                QRect fr = visualRect(subRect(SR_RadioButtonFocusRect, widget), widget);
                drawPrimitive(PE_FocusRect, p, fr, pal, flags);
            }
#endif
            break;
        }

    case CE_MenuBarItem:
        {
#ifndef QT_NO_MENUDATA
            if (opt.isDefault())
                break;

            if ((flags & Style_Enabled) && (flags & Style_Active))
                drawMotifPlusShade(p, r, pal, false, true);
            else
                p->fillRect(r, pal.button());

            QAction *mi = opt.action();
            QPixmap pix = mi->icon().pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine,
                     pal, flags & Style_Enabled, pix, mi->text(), -1,
                     &pal.buttonText().color());
#endif
            break;
        }

#ifdef QT_COMPAT
    case CE_Q3MenuBarItem:
        {
#ifndef QT_NO_MENUDATA
            if (opt.isDefault())
                break;

            Q3MenuItem *mi = opt.menuItem();
            if ((flags & Style_Enabled) && (flags & Style_Active))
                drawMotifPlusShade(p, r, pal, false, true);
            else
                p->fillRect(r, pal.button());

            drawItem(p, r, AlignCenter | ShowPrefix | DontClip | SingleLine,
                     pal, flags & Style_Enabled, *mi->pixmap(), mi->text(), -1,
                     &pal.buttonText().color());
#endif
            break;
        }
#endif

#ifndef QT_NO_MENU
    case CE_MenuItem:
        {
            if(!widget || opt.isDefault())
                break;

            QMenu *menu = (QMenu *)widget;
            QAction *mi = opt.action();
            if(!mi)
                break;

            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = ! (flags & Style_Enabled);
            bool checkable = menu->isCheckable();
            bool act = flags & Style_Active;
            int x, y, w, h;

            r.rect(&x, &y, &w, &h);

            if (checkable)
                maxpmw = qMax(maxpmw, 15);

            int checkcol = maxpmw;
            if (mi && mi->isSeparator()) {
                p->setPen(pal.dark());
                p->drawLine(x, y, x+w, y);
                p->setPen(pal.light());
                p->drawLine(x, y+1, x+w, y+1);
                return;
            }

            if (act && !dis)
                drawMotifPlusShade(p, QRect(x, y, w, h), pal, false, true);
            else
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));

            if(!mi)
                return;

            QRect vrect = visualRect(QRect(x+2, y+2, checkcol, h-2), r);
            if(mi->isChecked()) {
                if(!mi->icon().isNull())
                    qDrawShadePanel(p, vrect.x(), y+2, checkcol, h-2*2,
                                    pal, true, 1, &pal.brush(QPalette::Midlight));
            } else if (!act) {
                p->fillRect(vrect, pal.brush(QPalette::Button));
            }

            if(!mi->icon().isNull()) {              // draw iconset
                QIconSet::Mode mode = (!dis) ? QIconSet::Normal : QIconSet::Disabled;

                if (act && !dis)
                    mode = QIconSet::Active;

                QPixmap pixmap;
                if(checkable && mi->isChecked())
                    pixmap = mi->icon().pixmap(QIconSet::Small, mode, QIconSet::On);
                else
                    pixmap = mi->icon().pixmap(QIconSet::Small, mode);

                int pixw = pixmap.width();
                int pixh = pixmap.height();

                QRect pmr(0, 0, pixw, pixh);

                pmr.moveCenter(vrect.center());

                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);

            } else if (checkable) {
                if (mi->isChecked()) {
                    SFlags cflags = Style_Default;
                    if (! dis)
                        cflags |= Style_Enabled;
                    if (act)
                        cflags |= Style_On;

                    drawPrimitive(PE_CheckMark, p, vrect, pal, cflags);
                }
            }

            p->setPen(pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            vrect = visualRect(QRect(x + checkcol + 4, y + 2,
                                      w - checkcol - tab - 3, h - 4), r);

            QString s = mi->text();
            if (!s.isNull()) {                        // draw text
                int t = s.indexOf('\t');
                int m = 2;
                int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
                text_flags |= (QApplication::reverseLayout() ? AlignRight : AlignLeft);
                if (t >= 0) {                         // draw tab text
                    QRect vr = visualRect(QRect(x+w-tab-2-2,
                                                 y+m, tab, h-2*m), r);
                    p->drawText(vr.x(),
                                 y+m, tab, h-2*m, text_flags, s.mid(t+1));
                }
                p->drawText(vrect.x(), y + 2, w - checkcol -tab - 3, h - 4,
                            text_flags, s, t);
            }

            if (mi->menu()) {
                int hh = h / 2;
                QStyle::PrimitiveElement arrow = (QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight);
                vrect = visualRect(QRect(x + w - hh - 6, y + (hh / 2), hh, hh), r);
                drawPrimitive(arrow, p, vrect, pal,
                              ((act && !dis) ? Style_Down : Style_Default) | ((!dis) ? Style_Enabled : Style_Default));
            }
            break;
        }
#endif // QT_NO_MENU

#ifdef QT_COMPAT
#ifndef QT_NO_POPUPMENU
    case CE_Q3PopupMenuItem:
        {
            if (! widget || opt.isDefault())
                break;

            Q3PopupMenu *popupmenu = (Q3PopupMenu *) widget;
            Q3MenuItem *mi = opt.menuItem();
            if (!mi)
                break;

            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = ! (flags & Style_Enabled);
            bool checkable = popupmenu->isCheckable();
            bool act = flags & Style_Active;
            int x, y, w, h;

            r.rect(&x, &y, &w, &h);

            if (checkable)
                maxpmw = qMax(maxpmw, 15);

            int checkcol = maxpmw;

            if (mi && mi->isSeparator()) {
                p->setPen(pal.dark());
                p->drawLine(x, y, x+w, y);
                p->setPen(pal.light());
                p->drawLine(x, y+1, x+w, y+1);
                return;
            }

            if (act && !dis)
                drawMotifPlusShade(p, QRect(x, y, w, h), pal, false, true);
            else
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));

            if (!mi)
                return;

            QRect vrect = visualRect(QRect(x+2, y+2, checkcol, h-2), r);
            if (mi->isChecked()) {
                if (mi->iconSet()) {
                    qDrawShadePanel(p, vrect.x(), y+2, checkcol, h-2*2,
                                     pal, true, 1, &pal.brush(QPalette::Midlight));
                }
            } else if (!act) {
                p->fillRect(vrect,
                            pal.brush(QPalette::Button));
            }

            if (mi->iconSet()) {              // draw iconset
                QIconSet::Mode mode = (!dis) ? QIconSet::Normal : QIconSet::Disabled;

                if (act && !dis)
                    mode = QIconSet::Active;

                QPixmap pixmap;
                if (checkable && mi->isChecked())
                    pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode,
                                                    QIconSet::On);
                else
                    pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);

                int pixw = pixmap.width();
                int pixh = pixmap.height();

                QRect pmr(0, 0, pixw, pixh);

                pmr.moveCenter(vrect.center());

                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);

            } else if (checkable) {
                if (mi->isChecked()) {
                    SFlags cflags = Style_Default;
                    if (! dis)
                        cflags |= Style_Enabled;
                    if (act)
                        cflags |= Style_On;

                    drawPrimitive(PE_CheckMark, p, vrect, pal, cflags);
                }
            }

            p->setPen(pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            vrect = visualRect(QRect(x + checkcol + 4, y + 2,
                                      w - checkcol - tab - 3, h - 4), r);
            if (mi->custom()) {
                p->save();
                mi->custom()->paint(p, pal, act, !dis, vrect.x(), y + 2,
                                    w - checkcol - tab - 3, h - 4);
                p->restore();
            }

            QString s = mi->text();
            if (!s.isNull()) {                        // draw text
                int t = s.indexOf('\t');
                int m = 2;
                int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
                text_flags |= (QApplication::reverseLayout() ? AlignRight : AlignLeft);
                if (t >= 0) {                         // draw tab text
                    QRect vr = visualRect(QRect(x+w-tab-2-2,
                                                 y+m, tab, h-2*m), r);
                    p->drawText(vr.x(),
                                 y+m, tab, h-2*m, text_flags, s.mid(t+1));
                }
                p->drawText(vrect.x(), y + 2, w - checkcol -tab - 3, h - 4,
                            text_flags, s, t);
            } else if (mi->pixmap()) {
                QPixmap *pixmap = mi->pixmap();

                if (pixmap->depth() == 1) p->setBackgroundMode(OpaqueMode);
                QRect vr = visualRect(QRect(x + checkcol + 2, y + 2, w - checkcol - 1, h - 4), r);
                p->drawPixmap(vr.x(), y + 2, *pixmap);
                if (pixmap->depth() == 1) p->setBackgroundMode(TransparentMode);
            }

            if (mi->popup()) {
                int hh = h / 2;
                QStyle::PrimitiveElement arrow = (QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight);
                vrect = visualRect(QRect(x + w - hh - 6, y + (hh / 2), hh, hh), r);
                drawPrimitive(arrow, p,
                              vrect, pal,
                              ((act && !dis) ?
                               Style_Down : Style_Default) |
                              ((!dis) ? Style_Enabled : Style_Default));
            }
            break;
        }
#endif // QT_NO_POPUPMENU
#endif

    case CE_TabBarTab:
        {
#ifndef QT_NO_TABBAR
            const QTabBar *tabbar = (const QTabBar *) widget;
            bool selected = flags & Style_Selected;

            QPen oldpen = p->pen();
            QRect fr(r);

            if (! selected) {
                if (tabbar->shape() == QTabBar::RoundedAbove ||
                    tabbar->shape() == QTabBar::TriangularAbove) {
                    fr.setTop(fr.top() + 2);
                } else {
                    fr.setBottom(fr.bottom() - 2);
                }
            }

            fr.setWidth(fr.width() - 3);

            p->fillRect(fr.left() + 1, fr.top() + 1, fr.width() - 2, fr.height() - 2,
                        (selected) ? pal.brush(QPalette::Button)
                        : pal.brush(QPalette::Mid));

            if (tabbar->shape() == QTabBar::RoundedAbove) {
                // "rounded" tabs on top
                fr.setBottom(fr.bottom() - 1);

                p->setPen(pal.light());
                p->drawLine(fr.left(), fr.top() + 1,
                            fr.left(), fr.bottom() - 1);
                p->drawLine(fr.left() + 1, fr.top(),
                            fr.right() - 1, fr.top());
                if (! selected)
                    p->drawLine(fr.left(), fr.bottom(),
                                fr.right() + 3, fr.bottom());

                if (fr.left() == 0)
                    p->drawLine(fr.left(), fr.bottom(),
                                fr.left(), fr.bottom() + 1);

                p->setPen(pal.dark());
                p->drawLine(fr.right() - 1, fr.top() + 2,
                            fr.right() - 1, fr.bottom() - 1);

                p->setPen(black);
                p->drawLine(fr.right(), fr.top() + 1,
                            fr.right(), fr.bottom() - 1);
            } else if (tabbar->shape() == QTabBar::RoundedBelow) {
                // "rounded" tabs on bottom
                fr.setTop(fr.top() + 1);

                p->setPen(pal.dark());
                p->drawLine(fr.right() + 3, fr.top() - 1,
                            fr.right() - 1, fr.top() - 1);
                p->drawLine(fr.right() - 1, fr.top(),
                            fr.right() - 1, fr.bottom() - 2);
                p->drawLine(fr.right() - 1, fr.bottom() - 2,
                            fr.left() + 2,  fr.bottom() - 2);
                if (! selected) {
                    p->drawLine(fr.right(), fr.top() - 1,
                                fr.left() + 1,  fr.top() - 1);

                    if (fr.left() != 0)
                        p->drawPoint(fr.left(), fr.top() - 1);
                }

                p->setPen(black);
                p->drawLine(fr.right(), fr.top(),
                            fr.right(), fr.bottom() - 2);
                p->drawLine(fr.right() - 1, fr.bottom() - 1,
                            fr.left(), fr.bottom() - 1);
                if (! selected)
                    p->drawLine(fr.right() + 3, fr.top(),
                                fr.left(), fr.top());
                else
                    p->drawLine(fr.right() + 3, fr.top(),
                                fr.right(), fr.top());

                p->setPen(pal.light());
                p->drawLine(fr.left(), fr.top() + 1,
                            fr.left(), fr.bottom() - 2);

                if (selected) {
                    p->drawPoint(fr.left(), fr.top());
                    if (fr.left() == 0)
                        p->drawPoint(fr.left(), fr.top() - 1);

                    p->setPen(pal.button());
                    p->drawLine(fr.left() + 2, fr.top() - 1,
                                fr.left() + 1, fr.top() - 1);
                }
            } else
                // triangular drawing code
                QMotifStyle::drawControl(element, p, widget, r, pal, flags, opt);

            p->setPen(oldpen);
#endif
            break;
        }

    default:
        QMotifStyle::drawControl(element, p, widget, r, pal, flags, opt);
        break;
    }
}


/*! \reimp
*/
QRect QMotifPlusStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect;

    switch (r) {
    case SR_PushButtonFocusRect:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            int dfi = pixelMetric(PM_ButtonDefaultIndicator, widget);

            rect = button->rect();
            if (button->isDefault() || button->autoDefault())
                rect.addCoords(dfi, dfi, -dfi, -dfi);
#endif
            break;
        }

    case SR_CheckBoxIndicator:
        {
            int h = pixelMetric(PM_IndicatorHeight);
            rect.setRect((widget->rect().height() - h) / 2,
                         (widget->rect().height() - h) / 2,
                         pixelMetric(PM_IndicatorWidth), h);
            break;
        }

    case SR_RadioButtonIndicator:
        {
            int h = pixelMetric(PM_ExclusiveIndicatorHeight);
            rect.setRect((widget->rect().height() - h) / 2,
                          (widget->rect().height() - h) / 2,
                          pixelMetric(PM_ExclusiveIndicatorWidth), h);
            break;
        }

    case SR_CheckBoxFocusRect:
    case SR_RadioButtonFocusRect:
        rect = widget->rect();
        break;

    case SR_ComboBoxFocusRect:
        {
#ifndef QT_NO_COMBOBOX
            const QComboBox *combobox = (const QComboBox *) widget;

            if (combobox->editable()) {
                rect = querySubControlMetrics(CC_ComboBox, widget,
                                              SC_ComboBoxEditField);
                rect.addCoords(-3, -3, 3, 3);
            } else
                rect = combobox->rect();
#endif
            break;
        }

    case SR_SliderFocusRect:
        {
#ifndef QT_NO_SLIDER
            const QSlider *slider = (const QSlider *) widget;
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, widget);
            int thickness = pixelMetric(PM_SliderControlThickness, widget);
            int x, y, wi, he;

            if (slider->orientation() == Horizontal) {
                x = 0;
                y = tickOffset;
                wi = slider->width();
                he = thickness;
            } else {
                x = tickOffset;
                y = 0;
                wi = thickness;
                he = slider->height();
            }

            rect.setRect(x, y, wi, he);
#endif
            break;
        }

    default:
        rect = QMotifStyle::subRect(r, widget);
        break;
    }

    return rect;
}


/*! \reimp */
void QMotifPlusStyle::drawComplexControl(ComplexControl control,
                            QPainter *p,
                            const QWidget *widget,
                            const QRect &r,
                            const QPalette &pal,
                            SFlags flags,
                            SCFlags controls,
                            SCFlags active,
                            const QStyleOption& opt) const
{
    if (widget == singleton->hoverWidget)
        flags |= Style_MouseOver;

    switch (control) {
    case CC_ScrollBar:
        {
#ifndef QT_NO_SCROLLBAR
            const QScrollBar *scrollbar = (const QScrollBar *) widget;
            QRect addline, subline, addpage, subpage, slider, first, last;
            bool maxedOut = (scrollbar->minimum() == scrollbar->maximum());

            subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
            addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
            subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
            addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
            slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
            first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
            last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);

            if (singleton->hovering) {
                if (addline.contains(singleton->mousePos)) {
                    singleton->scrollbarElement = SC_ScrollBarAddLine;
                } else if (subline.contains(singleton->mousePos)) {
                    singleton->scrollbarElement = SC_ScrollBarSubLine;
                } else if (slider.contains(singleton->mousePos)) {
                    singleton->scrollbarElement = SC_ScrollBarSlider;
                } else {
                    singleton->scrollbarElement = 0;
                }
            } else
                singleton->scrollbarElement = 0;

            if (controls == (SC_ScrollBarAddLine | SC_ScrollBarSubLine |
                             SC_ScrollBarAddPage | SC_ScrollBarSubPage |
                             SC_ScrollBarFirst | SC_ScrollBarLast | SC_ScrollBarSlider))
                drawMotifPlusShade(p, widget->rect(), pal, true, false,
                                   &pal.brush(QPalette::Mid));

            if ((controls & SC_ScrollBarSubLine) && subline.isValid())
                drawPrimitive(PE_ScrollBarSubLine, p, subline, pal,
                              ((active == SC_ScrollBarSubLine ||
                                singleton->scrollbarElement == SC_ScrollBarSubLine) ?
                               Style_MouseOver: Style_Default) |
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((active == SC_ScrollBarSubLine) ?
                               Style_Down : Style_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));
            if ((controls & SC_ScrollBarAddLine) && addline.isValid())
                drawPrimitive(PE_ScrollBarAddLine, p, addline, pal,
                              ((active == SC_ScrollBarAddLine ||
                                singleton->scrollbarElement == SC_ScrollBarAddLine) ?
                               Style_MouseOver: Style_Default) |
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((active == SC_ScrollBarAddLine) ?
                               Style_Down : Style_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));
            if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
                drawPrimitive(PE_ScrollBarSubPage, p, subpage, pal,
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((active == SC_ScrollBarSubPage) ?
                               Style_Down : Style_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));
            if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
                drawPrimitive(PE_ScrollBarAddPage, p, addpage, pal,
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((active == SC_ScrollBarAddPage) ?
                               Style_Down : Style_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));
            if ((controls & SC_ScrollBarFirst) && first.isValid())
                drawPrimitive(PE_ScrollBarFirst, p, first, pal,
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((active == SC_ScrollBarFirst) ?
                               Style_Down : Style_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));
            if ((controls & SC_ScrollBarLast) && last.isValid())
                drawPrimitive(PE_ScrollBarLast, p, last, pal,
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((active == SC_ScrollBarLast) ?
                               Style_Down : Style_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));
            if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
                drawPrimitive(PE_ScrollBarSlider, p, slider, pal,
                              ((active == SC_ScrollBarSlider ||
                                singleton->scrollbarElement == SC_ScrollBarSlider) ?
                               Style_MouseOver: Style_Default) |
                              ((maxedOut) ? Style_Default : Style_Enabled) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               Style_Horizontal : Style_Default));

                // ### perhaps this should not be able to accept focus if maxedOut?
                if (scrollbar->hasFocus()) {
                    QRect fr(slider.x() + 2, slider.y() + 2,
                             slider.width() - 5, slider.height() - 5);
                    drawPrimitive(PE_FocusRect, p, fr, pal, Style_Default);
                }
            }
#endif
            break;
        }

    case CC_ComboBox:
        {
#ifndef QT_NO_COMBOBOX
            const QComboBox *combobox = (const QComboBox *) widget;

            QRect editfield, arrow;
            editfield =
                visualRect(querySubControlMetrics(CC_ComboBox,
                                                  combobox,
                                                  SC_ComboBoxEditField,
                                                  opt), widget);
            arrow =
                visualRect(querySubControlMetrics(CC_ComboBox,
                                                  combobox,
                                                  SC_ComboBoxArrow,
                                                  opt), widget);

            if (combobox->editable()) {
                if (controls & SC_ComboBoxEditField && editfield.isValid()) {
                    editfield.addCoords(-3, -3, 3, 3);
                    if (combobox->hasFocus())
                        editfield.addCoords(1, 1, -1, -1);
                    drawMotifPlusShade(p, editfield, pal, true, false,
                                       (widget->isEnabled() ?
                                        &pal.brush(QPalette::Base) :
                                        &pal.brush(QPalette::Background)));
                }

                if (controls & SC_ComboBoxArrow && arrow.isValid()) {
                    drawMotifPlusShade(p, arrow, pal, (active == SC_ComboBoxArrow),
                                       (flags & Style_MouseOver));

                    int space = (r.height() - 13) / 2;
                    arrow.addCoords(space, space, -space, -space);

                    if (active == SC_ComboBoxArrow)
                        flags |= Style_Sunken;
                    drawPrimitive(PE_ArrowDown, p, arrow, pal, flags);
                }
            } else {
                if (controls & SC_ComboBoxEditField && editfield.isValid()) {
                    editfield.addCoords(-3, -3, 3, 3);
                    if (combobox->hasFocus())
                        editfield.addCoords(1, 1, -1, -1);
                    drawMotifPlusShade(p, editfield, pal, false,
                                       (flags & Style_MouseOver));
                }

                if (controls & SC_ComboBoxArrow && arrow.isValid())
                    drawMotifPlusShade(p, arrow, pal, false, (flags & Style_MouseOver));
            }

            if (combobox->hasFocus() ||
                (combobox->editable() && combobox->lineEdit()->hasFocus())) {
                QRect fr = visualRect(subRect(SR_ComboBoxFocusRect, widget), widget);
                drawPrimitive(PE_FocusRect, p, fr, pal, flags);
            }
#endif
            break;
        }

    case CC_SpinWidget:
        {
#ifndef QT_NO_SPINWIDGET
            const QSpinWidget * sw = (const QSpinWidget *) widget;
            SFlags flags = Style_Default;

            if (controls & SC_SpinWidgetFrame)
                drawMotifPlusShade(p, r, pal, true, false, &pal.brush(QPalette::Base));

            if (controls & SC_SpinWidgetUp) {
                flags = Style_Enabled;
                if (active == SC_SpinWidgetUp)
                    flags |= Style_Down;

                PrimitiveElement pe;
                if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
                    pe = PE_SpinWidgetPlus;
                else
                    pe = PE_SpinWidgetUp;

                QRect re = sw->upRect();
                QPalette pal2 = pal;
                if(!sw->isUpEnabled())
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                drawPrimitive(pe, p, re, pal2, flags);
            }

            if (controls & SC_SpinWidgetDown) {
                flags = Style_Enabled;
                if (active == SC_SpinWidgetDown)
                    flags |= Style_Down;

                PrimitiveElement pe;
                if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
                    pe = PE_SpinWidgetMinus;
                else
                    pe = PE_SpinWidgetDown;

                QRect re = sw->downRect();
                QPalette pal2 = pal;
                if(!sw->isDownEnabled())
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                drawPrimitive(pe, p, re, pal2, flags);
            }
#endif
            break;
        }

    case CC_Slider:
        {
#ifndef QT_NO_SLIDER
            const QSlider *slider = (const QSlider *) widget;
            bool mouseover = (flags & Style_MouseOver);

            QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
                                                  opt),
                  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
                                                  opt);

            if ((controls & SC_SliderGroove) && groove.isValid()) {
                drawMotifPlusShade(p, groove, pal, true, false,
                                   &pal.brush(QPalette::Mid));

                if (flags & Style_HasFocus) {
                    QRect fr = subRect(SR_SliderFocusRect, widget);
                    drawPrimitive(PE_FocusRect, p, fr, pal, flags);
                }
            }

            if ((controls & SC_SliderHandle) && handle.isValid()) {
                if ((mouseover && handle.contains(singleton->mousePos)) ||
                    singleton->sliderActive)
                    flags |= Style_MouseOver;
                else
                    flags &= ~Style_MouseOver;
                drawPrimitive(PE_ButtonBevel, p, handle, pal, flags | Style_Raised);

                if (slider->orientation() == Horizontal) {
                    QCOORD mid = handle.x() + handle.width() / 2;
                    qDrawShadeLine(p, mid,  handle.y() + 1, mid ,
                                    handle.y() + handle.height() - 3,
                                    pal, true, 1);
                } else {
                    QCOORD mid = handle.y() + handle.height() / 2;
                    qDrawShadeLine(p, handle.x() + 1, mid,
                                    handle.x() + handle.width() - 3, mid,
                                    pal, true, 1);
                }
            }

            if (controls & SC_SliderTickmarks)
                QMotifStyle::drawComplexControl(control, p, widget, r, pal, flags,
                                                SC_SliderTickmarks, active, opt);
#endif
            break;
        }

    default:
        QMotifStyle::drawComplexControl(control, p, widget, r, pal, flags,
                                        controls, active, opt);
    }
}


/*! \reimp
*/
QRect QMotifPlusStyle::querySubControlMetrics(ComplexControl control,
                                              const QWidget *widget,
                                              SubControl subcontrol,
                                              const QStyleOption& opt) const
{
    switch (control) {
    case CC_SpinWidget: {
            int fw = pixelMetric(PM_SpinBoxFrameWidth, 0);
            QSize bs;
            bs.setHeight((widget->height() + 1)/2);
            if (bs.height() < 10)
                bs.setHeight(10);
            bs.setWidth(bs.height()); // 1.6 -approximate golden mean
            bs = bs.expandedTo(QApplication::globalStrut());
            int y = 0;
            int x, lx, rx, h;
            x = widget->width() - y - bs.width();
            lx = fw;
            rx = x - fw * 2;
            h = bs.height() * 2;

            switch (subcontrol) {
            case SC_SpinWidgetUp:
                return QRect(x + 1, y, bs.width(), bs.height() - 1);
            case SC_SpinWidgetDown:
                return QRect(x + 1, y + bs.height() + 1, bs.width(), bs.height());
            case SC_SpinWidgetButtonField:
                return QRect(x, y, bs.width(), h - 2*fw);
            case SC_SpinWidgetEditField:
                return QRect(lx, fw, rx, h - 2*fw);
            case SC_SpinWidgetFrame:
                return QRect(0, 0, widget->width() - bs.width(), h);
            default:
                break;
            }
            break; }

#ifndef QT_NO_COMBOBOX
    case CC_ComboBox: {
        const QComboBox *combobox = (const QComboBox *) widget;
        if (combobox->editable()) {
            int space = (combobox->height() - 13) / 2;
            switch (subcontrol) {
            case SC_ComboBoxFrame:
                return QRect();
            case SC_ComboBoxEditField: {
                QRect rect = widget->rect();
                rect.setWidth(rect.width() - 13 - space * 2);
                rect.addCoords(3, 3, -3, -3);
                return rect; }
            case SC_ComboBoxArrow:
                return QRect(combobox->width() - 13 - space * 2, 0,
                             13 + space * 2, combobox->height());
            default: break;                // shouldn't get here
            }

        } else {
            int space = (combobox->height() - 7) / 2;
            switch (subcontrol) {
            case SC_ComboBoxFrame:
                return QRect();
            case SC_ComboBoxEditField: {
                QRect rect = widget->rect();
                rect.addCoords(3, 3, -3, -3);
                return rect; }
            case SC_ComboBoxArrow:                // 12 wide, 7 tall
                return QRect(combobox->width() - 12 - space, space, 12, 7);
            default: break;                // shouldn't get here
            }
        }
        break; }
#endif

#ifndef QT_NO_SLIDER
    case CC_Slider: {

        if (subcontrol == SC_SliderHandle) {
            const QSlider *slider = (const QSlider *) widget;
            int tickOffset  = pixelMetric(PM_SliderTickmarkOffset, widget);
            int thickness   = pixelMetric(PM_SliderControlThickness, widget);
            int len         = pixelMetric(PM_SliderLength, widget) + 2;
            int sliderPos   = slider->sliderPosition();
            int motifBorder = 2;

            if (slider->orientation() == Horizontal)
                return QRect(sliderPos + motifBorder, tickOffset + motifBorder, len,
                              thickness - 2*motifBorder);
            return QRect(tickOffset + motifBorder, sliderPos + motifBorder,
                          thickness - 2*motifBorder, len);
        }
        break; }
#endif
    default: break;
    }
    return QMotifStyle::querySubControlMetrics(control, widget, subcontrol, opt);
}


/*! \reimp */
bool QMotifPlusStyle::eventFilter(QObject *object, QEvent *event)
{
    switch(event->type()) {
    case QEvent::MouseButtonPress:
        {
            singleton->mousePressed = true;

            if (!qt_cast<QSlider*>(object))
                break;

            singleton->sliderActive = true;
            break;
        }

    case QEvent::MouseButtonRelease:
        {
            singleton->mousePressed = false;

            if (!qt_cast<QSlider*>(object))
                break;

            singleton->sliderActive = false;
            ((QWidget *) object)->repaint();
            break;
        }

    case QEvent::Enter:
        {
            if (! object->isWidgetType())
                break;

            singleton->hoverWidget = (QWidget *) object;
            if (! singleton->hoverWidget->isEnabled()) {
                singleton->hoverWidget = 0;
                break;
            }
            singleton->hoverWidget->repaint();
            break;
        }

    case QEvent::Leave:
        {
            if (object != singleton->hoverWidget)
                break;
            QWidget *w = singleton->hoverWidget;
            singleton->hoverWidget = 0;
            w->repaint();
            break;
        }

    case QEvent::MouseMove:
        {
            if (! object->isWidgetType() || object != singleton->hoverWidget)
                break;

            if (!qt_cast<QScrollBar*>(object) && ! qt_cast<QSlider*>(object))
                break;

            singleton->mousePos = ((QMouseEvent *) event)->pos();
            if (! singleton->mousePressed) {
                singleton->hovering = true;
                singleton->hoverWidget->repaint();
                singleton->hovering = false;
            }

            break;
        }

    default:
        break;
    }

    return QMotifStyle::eventFilter(object, event);
}


/*! \reimp */
int QMotifPlusStyle::styleHint(StyleHint hint,
                               const QWidget *widget,
                               const QStyleOption &opt,
                               QStyleHintReturn *returnData) const
{
    int ret;
    switch (hint) {
    case SH_Menu_MouseTracking:
        ret = 1;
        break;
    default:
        ret = QMotifStyle::styleHint(hint, widget, opt, returnData);
        break;
    }
    return ret;
}


#endif // QT_NO_STYLE_MOTIFPLUS
