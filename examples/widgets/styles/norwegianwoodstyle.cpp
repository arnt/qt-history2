#include <QtGui>

#include "norwegianwoodstyle.h"

static const int NumGroups = 3;
static const QPalette::ColorGroup groups[NumGroups] = {
    QPalette::Active, QPalette::Disabled, QPalette::Inactive
};

static void drawRoundRect(QPainter *painter, int x, int y, int width, int height, int minorDiameter)
{
    int rx = (200 * minorDiameter) / width;
    int ry = (200 * minorDiameter) / height;
    painter->drawRoundRect(x, y, width, height, rx, ry);
}

static QRegion roundRectRegion(const QRect& g, int r)
{
    QPolygon a;
    a.setPoints(8, g.x()+r, g.y(), g.right()-r, g.y(),
                 g.right(), g.y()+r, g.right(), g.bottom()-r,
                 g.right()-r, g.bottom(), g.x()+r, g.bottom(),
                 g.x(), g.bottom()-r, g.x(), g.y()+r);
    QRegion reg(a);
    int d = r*2-1;
    reg += QRegion(g.x(),g.y(),r*2,r*2, QRegion::Ellipse);
    reg += QRegion(g.right()-d,g.y(),r*2,r*2, QRegion::Ellipse);
    reg += QRegion(g.x(),g.bottom()-d,r*2,r*2, QRegion::Ellipse);
    reg += QRegion(g.right()-d,g.bottom()-d,r*2,r*2, QRegion::Ellipse);
    return reg;
}

static int get_combo_extra_width(int height, int *return_awh)
{
    int awh;
    if (height < 8) {
        awh = 6;
    } else if (height < 14) {
        awh = height - 2;
    } else {
        awh = height/2;
    }
    if (return_awh)
        *return_awh = awh;
    return awh*3/2;
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
        ay = 0;
        sy = r.height();
    } else {
        sy = ay+awh+dh;
    }
    ax = r.x() + r.width() - ew +(ew-awh)/2;
}

static inline int buttonThickness(int side)
{
    if (side < 10)
        return 2;
    else if (side < 20)
        return 3;
    else
        return 5;
}

NorwegianWoodStyle::NorwegianWoodStyle()
{
    buttonImage.load(":/images/woodbutton.png");
    lightImage = buttonImage;
    darkImage = buttonImage;
    midImage = buttonImage;
    backgroundImage.load(":/images/woodbackground.png");
    sunkenLightImage = backgroundImage;
    sunkenDarkImage = backgroundImage;

    for (int i = 0; i < buttonImage.numColors(); ++i) {
        QRgb rgb = buttonImage.color(i);
        QColor color(rgb);

        lightImage.setColor(i, color.light().rgb());
        darkImage.setColor(i, color.dark(180).rgb());
        midImage.setColor(i, color.dark(120).rgb());
    }
    for (int j = 0; j < backgroundImage.numColors(); ++j) {
        QRgb rgb = backgroundImage.color(j);
        QColor color(rgb);

        sunkenLightImage.setColor(j, color.light().rgb());
        sunkenDarkImage.setColor(j, color.dark(180).rgb());
    }

    woodPalette = QPalette(QColor(212, 140, 95));
    setBrush(woodPalette, QPalette::BrightText, Qt::white);
    setBrush(woodPalette, QPalette::Base, QColor(236, 182, 120));
    setBrushPixmap(woodPalette, QPalette::Button, buttonImage);
    setBrushPixmap(woodPalette, QPalette::Light, lightImage);
    setBrushPixmap(woodPalette, QPalette::Dark, darkImage);
    setBrushPixmap(woodPalette, QPalette::Mid, midImage);
    setBrushPixmap(woodPalette, QPalette::Background, backgroundImage);
}

void NorwegianWoodStyle::polish(QPalette &palette)
{
    palette = woodPalette;
}

void NorwegianWoodStyle::polish(QWidget *widget)
{
    QWindowsStyle::polish(widget);

    if (!widget->isWindow()) {
        if (qobject_cast<QPushButton *>(widget)
                || qobject_cast<QComboBox *>(widget))
            widget->setAutoMask(true);
#if 0
        if (widget->backgroundPixmap())
            widget->setBackgroundOrigin(QWidget::WindowOrigin);
#endif
    }
}

void NorwegianWoodStyle::unpolish(QWidget *widget)
{
    if (!widget->isWindow()) {
        if (qobject_cast<QPushButton *>(widget)
                || qobject_cast<QComboBox *>(widget))
            widget->setAutoMask(false);
#if 0
        if (widget->backgroundPixmap())
            widget->setBackgroundOrigin(QWidget::WindowOrigin); // ###
#endif
    }
    QWindowsStyle::unpolish(widget);
}

void NorwegianWoodStyle::drawPrimitive(PrimitiveElement element,
                                       const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const
{
    int x, y, width, height;
    option->rect.getRect(&x, &y, &width, &height);

    switch (element) {
    case PE_PanelButtonCommand:
        {
            int minorDiameter = qMin(width, height) / 2;
            int thickness = buttonThickness(minorDiameter);

            QRegion internR = roundRectRegion(QRect(x + thickness, y + thickness,
                                                    width - 2 * thickness,
                                                    height - 2 * thickness),
                                                    minorDiameter - thickness);
            QPen oldPen = painter->pen();

            QBrush brush = option->palette.brush((option->state & State_Sunken) ? QPalette::Mid : QPalette::Button);
            painter->setClipRegion(internR);
            painter->fillRect(option->rect, brush);

            QPoint p2(x + width - 1 - minorDiameter, y + minorDiameter);
            QPoint p3(x + minorDiameter, y + height - 1 - minorDiameter);

            QPolygon a;
            a.setPoints(5, x,y, x+width-1, y, p2.x(), p2.y(), p3.x(), p3.y(),
                         x, y + height - 1);
            painter->setClipRegion(QRegion(a) - internR);

            painter->fillRect(option->rect, (option->state & State_Sunken ? QBrush(option->palette.dark().color(), sunkenDarkImage)
                                             : option->palette.brush(QPalette::Light)));

            // A little inversion is needed the buttons
            // (but not flat)
            if ((option->state & State_Raised) || (option->state & State_Sunken)) {
                a.setPoint(0, x + width - 1, y + width - 1);
                painter->setClipRegion(QRegion(a) - internR);

                painter->fillRect(option->rect, (option->state & State_Sunken ?
                    QBrush(option->palette.light().color(), sunkenLightImage) : option->palette.dark()));
            }
            painter->setClipRegion(internR);
            painter->setClipping(false);
            painter->setPen(option->palette.foreground().color());
            drawRoundRect(painter, x, y, width, height, minorDiameter);
            painter->setPen(oldPen);
            break;
        }
/*
    case PE_ScrollBarAddLine:
        if (option->state & State_Horizontal)
            drawSemicircleButton(painter, option->rect, PointRight, option->state & State_Down, cg);
        else
            drawSemicircleButton(painter, option->rect, PointDown, option->state & State_Down, cg);
        break;
    case PE_ScrollBarSubLine:
        if (option->state & State_Horizontal)
            drawSemicircleButton(painter, option->rect, PointLeft, option->state & State_Down, cg);
        else
            drawSemicircleButton(painter, option->rect, PointUp, option->state & State_Down, cg);
        break;
*/
    default:
        QWindowsStyle::drawPrimitive(element, option, painter, widget);
    }
}

#if 0
void NorwegianWoodStyle::drawControl(ControlElement element,
                                      QPainter *painter,
                                      const QWidget *widget,
                                      const QRect &r,
                                      const QColorGroup &cg,
                                      SFlags how, const QStyleOption& opt) const
{
    switch(element) {
    case CE_PushButton:
        {
            const QPushButton *btn;
            btn = (const QPushButton *)widget;
            QColorGroup myCg(cg);
            SFlags flags = State_Default;
            if (btn->isOn())
                flags |= State_On;
            if (btn->isDown())
                flags |= State_Down;
            if (btn->isOn() || btn->isDown())
                flags |= State_Sunken;
            if (btn->isDefault())
                flags |= State_Default;
            if (! btn->isFlat() && !(flags & State_Down))
                flags |= State_Raised;

            int x1, y1, x2, y2;
            option->rect.coords(&x1, &y1, &x2, &y2);

            painter->setPen(cg.foreground());
            painter->setBrush(QBrush(cg.button(), NoBrush));

            QBrush fill;
            if (btn->isDown())
                fill = cg.brush(QColorGroup::Mid);
            else if (btn->isOn())
                fill = QBrush(cg.mid(), Dense4Pattern);
            else
                fill = cg.brush(QColorGroup::Button);
            myCg.setBrush(QColorGroup::Mid, fill);

            if (btn->isDefault()) {
                x1 += 2;
                y1 += 2;
                x2 -= 2;
                y2 -= 2;
            }

            drawPrimitive(PE_ButtonCommand, painter,
                           QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1),
                           myCg, flags, opt);

            if (btn->isDefault()) {
                QPen pen(Qt::black, 4);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                painter->setPen(pen);
                drawRoundRect(painter, x1 - 1, y1 - 1, x2 - x1 + 3, y2 - y1 + 3, 8);
            }

            if (btn->isMenuButton()) {
                int dx = (y1 - y2 - 4) / 3;

                // reset the flags
                flags = State_Default;
                if (btn->isEnabled())
                    flags |= State_Enabled;
                drawPrimitive(PE_ArrowDown, painter,
                               QRect(x2 - dx, dx, y1, y2 - y1),
                               myCg, flags, opt);
            }

            if (painter->brush().style() != NoBrush)
                painter->setBrush(NoBrush);
            break;
        }
    case CE_PushButtonLabel:
        {
            const QPushButton *btn;
            btn = (const QPushButton*)widget;
            int x, y, width, height;
            option->rect.getRect(&x, &y, &width, &height);

            int x1, y1, x2, y2;
            option->rect.getCoords(&x1, &y1, &x2, &y2);
            int dx = 0;
            int dy = 0;
            if (btn->isMenuButton())
                dx = (y2 - y1) / 3;
            if (dx || dy)
                painter->translate(dx, dy);

            x += 2;
            y += 2;
            width -= 4;
            height -= 4;
            drawItem(painter, QRect(x, y, width, height),
                      AlignCenter | ShowPrefix,
                      cg, btn->isEnabled(),
                      btn->pixmap(), btn->text(), -1,
                      (btn->isDown() || btn->isOn()) ? &cg.brightText()
                      : &cg.buttonText());
            if (dx || dy)
                painter->translate(-dx, -dy);
            break;
        }
    default:
        QWindowsStyle::drawControl(element, painter, widget, option->rect, cg, how, opt);
    }
}

void NorwegianWoodStyle::drawControlMask(ControlElement element,
                                          QPainter *painter,
                                          const QWidget *widget,
                                          const QRect &r,
                                          const QStyleOption& opt) const
{
    switch(element) {
    case CE_PushButton:
        {
            int minorDiameter = qMin(r.width(), r.height()) / 2;
            painter->setPen(color1);
            painter->setBrush(color1);
            drawRoundRect(painter, r.x(), r.y(), r.width(), r.height(), minorDiameter);
            break;
        }
    default:
        QWindowsStyle::drawControlMask(element, painter, widget, r, opt);
        break;
    }
}

void NorwegianWoodStyle::drawComplexControl(ComplexControl cc,
                                             QPainter *painter,
                                             const QWidget *widget,
                                             const QRect &r,
                                             const QColorGroup &cg,
                                             SFlags how,
                                             SCFlags sub,
                                             SCFlags subActive,
                                             const QStyleOption& opt) const
{
    switch(cc) {
    case CC_ComboBox:
        {
            const QComboBox *cmb;
            cmb = (const QComboBox*)widget;

            int awh, ax, ay, sh, sy, dh, ew;
            get_combo_parameters(subRect(SR_PushButtonContents, widget),
                                  ew, awh, ax, ay, sh, dh, sy);
            drawPrimitive(PE_ButtonCommand, painter, r, cg, State_Raised, opt);
            QStyle *mstyle = QStyleFactory::create("Motif");
            if (mstyle)
                mstyle->drawPrimitive(PE_ArrowDown, painter,
                                       QRect(ax, ay, awh, awh), cg, how, opt);
            else
                drawPrimitive(PE_ArrowDown, painter,
                               QRect(ax, ay, awh, awh), cg, how, opt);

            QPen oldPen = painter->pen();
            painter->setPen(cg.light());
            painter->drawLine(ax, sy, ax + awh - 1, sy);
            painter->drawLine(ax, sy, ax, sy + sh - 1);
            painter->setPen(cg.dark());
            painter->drawLine(ax + 1, sy + sh - 1, ax + awh - 1, sy + sh - 1);
            painter->drawLine(ax + awh - 1, sy + 1, ax + awh - 1, sy + sh - 1);
            painter->setPen(oldPen);

            if (cmb->editable()) {
                QRect r(querySubControlMetrics(CC_ComboBox, widget,
                                                SC_ComboBoxEditField, opt));
                qDrawShadePanel(painter, r, cg, TRUE, 1,
                                 &cg.brush(QColorGroup::Button));
            }

            break;
        }
    default:
        QWindowsStyle::drawComplexControl(cc, painter, widget, r, cg, how,
                                           sub, subActive, opt);
        break;
    }
}

void NorwegianWoodStyle::drawComplexControlMask(ComplexControl control,
                                                 QPainter *painter,
                                                 const QWidget *widget,
                                                 const QRect &r,
                                                 const QStyleOption& opt) const
{
    switch (control) {
    case CC_ComboBox:
        {
            int minorDiameter = qMin(r.width(), r.height()) / 2;
            painter->setPen(color1);
            painter->setBrush(color1);
            drawRoundRect(painter, r.x(), r.y(), r.width(), r.height(), minorDiameter);
            break;
        }
    default:
        QWindowsStyle::drawComplexControlMask(control, painter, widget, r, opt);
        break;
    }
}

QRect NorwegianWoodStyle::querySubControlMetrics(ComplexControl control,
                                                  const QWidget *widget,
                                                  SubControl sc,
                                                  const QStyleOption& opt) const
{
    QRect rect;
    switch (control) {
    case CC_ComboBox:
        {
            switch(sc) {
            case SC_ComboBoxEditField:
                {
                    rect = subRect(SR_PushButtonContents, widget);
                    int ew = get_combo_extra_width(rect.height(), 0);
                    rect.setRect(rect.x() + 1, rect.y() + 1,
                                  rect.width() - 2 - ew, rect.height() - 2);
                    break;
                }
            default:
                rect = QWindowsStyle::querySubControlMetrics(control, widget,
                                                              sc, opt);
                break;
            }
            break;
        }
    case CC_ScrollBar:
        {
            const QScrollBar* sb;
            sb = (const QScrollBar*)widget;
            bool horz = sb->orientation() == QScrollBar::Horizontal;
            int b = 2;
            int width = horz ? sb->height() : sb->width();

            switch (sc) {
            case SC_ScrollBarAddLine:
                rect.setRect(b, b, width - 2 * b, width - 2 * b);
                if (horz)
                    rect.moveBy(sb->width() - width, 0);
                else
                    rect.moveBy(0, sb->height() - width);
                break;
            case SC_ScrollBarSubLine:
                rect.setRect(b, b, width - 2 * b, width - 2 * b);
                break;
            default:
                rect = QWindowsStyle::querySubControlMetrics(control, widget,
                                                              sc, opt);
                break;
            }
            break;
        }
    default:
        rect = QWindowsStyle::querySubControlMetrics(control, widget,
                                                      sc, opt);
        break;
    }
    return rect;
}

QRect NorwegianWoodStyle::subRect(SubRect sr, const QWidget * widget) const
{
    QRect r;
    switch (sr) {
    case SR_PushButtonContents:
        {
            const QPushButton *btn;
            btn = (const QPushButton*)widget;
            r = btn->rect();
            int minorDiameter = qMin(r.width(), r.height()) / 2;
            int b = buttonThickness(minorDiameter);

            minorDiameter -= b;
            b++;

            if (r.width() < r.height())
                r.setRect(r.x() + b, r.y() + minorDiameter,
                           r.width() - 2 * b, r.height() - 2 * minorDiameter);
            else
                r.setRect(r.x() + minorDiameter, r.y() + b,
                           r.width() - 2 * minorDiameter, r.height() - 2 * b);
            break;
        }
    case SR_ComboBoxFocusRect:
        {
            r = subRect(SR_PushButtonContents, widget);
            int ew = get_combo_extra_width(r.height());
            r.setRect(r.x() + 1, r.y() + 1, r.width() - 2 - ew,
                       r.height() - 2);
            break;
        }
    default:
        r = QWindowsStyle::subRect(sr, widget);
        break;
    }
    return r;
}
#endif

void NorwegianWoodStyle::setBrush(QPalette &palette, QPalette::ColorRole role,
                                  const QBrush &brush)
{
    for (int i = 0; i < 3; ++i)
        palette.setBrush(groups[i], role, brush);
}

void NorwegianWoodStyle::setBrushPixmap(QPalette &palette,
                                        QPalette::ColorRole role,
                                        const QPixmap &pixmap)
{
    for (int i = 0; i < 3; ++i) {
        QColor color = palette.brush(groups[i], role).color();
        palette.setBrush(groups[i], role, QBrush(color, pixmap));
    }
}

#if 0
void NorwegianWoodStyle::drawSemicircleButton(QPainter *painter, const QRect &r,
                                               int dir, bool sunken,
                                               const QColorGroup &g) const
{
    int b =  pixelMetric(PM_ScrollBarExtent) > 20 ? 3 : 2;

     QRegion extrn( r.x(),   r.y(),   r.width(),     r.height(),     QRegion::Ellipse);
     QRegion intern(r.x()+b, r.y()+b, r.width()-2*b, r.height()-2*b, QRegion::Ellipse);
    int w2 = r.width()/2;
    int h2 = r.height()/2;

    int bug = 1; //off-by-one somewhere!!!???

    switch(dir) {
    case PointRight:
        extrn +=  QRegion(r.x(),  r.y(),   w2,     r.height());
        intern += QRegion(r.x()+b,r.y()+b, w2-2*b, r.height()-2*b);
        break;
    case PointLeft:
        extrn +=  QRegion(r.x()+w2,  r.y(),   w2,     r.height());
        intern += QRegion(r.x()+w2+b,r.y()+b, w2-2*b, r.height()-2*b);
        break;
    case PointUp:
        extrn +=  QRegion(r.x(),  r.y()+h2,   r.width(),     h2);
        intern += QRegion(r.x()+b,r.y()+h2+b, r.width()-2*b-bug, h2-2*b-bug);
        break;
    case PointDown:
        extrn +=  QRegion(r.x(),  r.y(),   r.width(),     h2);
        intern += QRegion(r.x()+b,r.y()+b, r.width()-2*b-bug, h2-2*b-bug);
        break;
    }

    extrn = extrn - intern;
    QPolygon a;
    a.setPoints(3, r.x(), r.y(), r.x(), r.bottom(), r.right(), r.top());

    QRegion oldClip = painter->clipRegion();
    bool bReallyClip = painter->hasClipping();  // clip only if we really want.
    painter->setClipRegion(intern);
    painter->fillRect(r, g.brush(QColorGroup::Button));

    painter->setClipRegion(QRegion(a)&extrn);
    painter->fillRect(r, sunken ? g.dark() : g.light());

    a.setPoints(3, r.right(), r.bottom(), r.x(), r.bottom(),
                 r.right(), r.top());
    painter->setClipRegion(QRegion(a) &  extrn);
    painter->fillRect(r, sunken ? g.light() : g.dark());

    painter->setClipRegion(oldClip);
    painter->setClipping(bReallyClip);
}
#endif
