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

static QRegion roundRectRegion(const QRect &rect, int radius)
{
    QPolygon polygon;
    polygon << QPoint(rect.x() + radius, rect.y())
            << QPoint(rect.x() + rect.width() - radius, rect.y())
            << QPoint(rect.x() + rect.width(), rect.y() + radius)
            << QPoint(rect.x() + rect.width(), rect.y() + rect.height() - radius)
            << QPoint(rect.x() + rect.width() - radius, rect.y() + rect.height())
            << QPoint(rect.x() + radius, rect.y() + rect.height())
            << QPoint(rect.x(), rect.y() + rect.height() - radius)
            << QPoint(rect.x(), rect.y() + radius);

    QRegion region(polygon);
    int diameter = radius * 2 - 1;
    region |= QRegion(rect.x(), rect.y(), radius * 2, radius * 2, QRegion::Ellipse);
    region |= QRegion(rect.right() - diameter, rect.y(), radius * 2, radius * 2, QRegion::Ellipse);
    region |= QRegion(rect.x(), rect.bottom() - diameter, radius * 2, radius * 2, QRegion::Ellipse);
    region |= QRegion(rect.right() - diameter, rect.bottom() - diameter, radius * 2, radius * 2, QRegion::Ellipse);
    return region;
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
    buttonImage.load(":/images/woodbutton.xpm");
    lightImage = buttonImage;
    darkImage = buttonImage;
    midImage = buttonImage;
    backgroundImage.load(":/images/woodbackground.xpm");
    sunkenLightImage = backgroundImage;
    sunkenMidImage = backgroundImage;
    sunkenDarkImage = backgroundImage;

    for (int i = 0; i < buttonImage.numColors(); ++i) {
        QRgb rgb = buttonImage.color(i);
        QColor color(rgb);

        lightImage.setColor(i, color.light().rgb());
        midImage.setColor(i, color.dark(120).rgb());
        darkImage.setColor(i, color.dark(180).rgb());
    }
    for (int j = 0; j < backgroundImage.numColors(); ++j) {
        QRgb rgb = backgroundImage.color(j);
        QColor color(rgb);

        sunkenLightImage.setColor(j, color.light().rgb());
        sunkenMidImage.setColor(j, color.dark(120).rgb());
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
            const QStyleOptionButton *buttonOption =
                    qstyleoption_cast<const QStyleOptionButton *>(option);
            bool isFlat = (buttonOption->features & QStyleOptionButton::Flat);

            if (isFlat && widget)
                painter->setBrushOrigin(-widget->mapTo(widget->window(),
                                                       QPoint(0, 0)));

            int minorDiameter = qMin(width, height) / 2;
            int thickness = buttonThickness(minorDiameter);

            QRegion outerRegion = roundRectRegion(option->rect, minorDiameter);
            QRegion innerRegion = roundRectRegion(QRect(x + thickness,
                                                        y + thickness,
                                                        width - 2 * thickness,
                                                        height - 2 * thickness),
                                                  minorDiameter - thickness);
            QPoint p2(x + width - 1 - minorDiameter, y + minorDiameter);
            QPoint p3(x + minorDiameter, y + height - 1 - minorDiameter);
            QPolygon sunnySide;
            sunnySide << QPoint(x, y) << QPoint(x + width - 1, y) << p2 << p3 << QPoint(x, y + height - 1);

            QPen oldPen = painter->pen();

            QBrush brush;

            if (option->state & (State_Down | State_On)) {
                if (isFlat) {
                    brush = QBrush(option->palette.mid().color(), sunkenMidImage);
                } else {
                    brush = option->palette.mid();
                }
            } else {
                brush = option->palette.button();
            }

            painter->setClipRegion(innerRegion);
            painter->fillRect(option->rect, brush);
            if ((option->state & (State_Down | State_On)) == State_On)
                painter->fillRect(option->rect, QBrush(brush.color(), Qt::Dense4Pattern));

            painter->setClipRegion((QRegion(sunnySide) - innerRegion) & outerRegion);
            painter->fillRect(option->rect, (option->state & (State_Down | State_On) ? QBrush(option->palette.dark().color(), sunkenDarkImage)
                                             : option->palette.brush(QPalette::Light)));

            if (option->state & (State_Raised | State_Down | State_On)) {
                sunnySide[0] = QPoint(x + width - 1, y + width - 1);
                painter->setClipRegion((QRegion(sunnySide) - innerRegion) & outerRegion);

                painter->fillRect(option->rect, (option->state & (State_Down | State_On) ?
                    QBrush(option->palette.light().color(), sunkenLightImage) : option->palette.dark()));
            }
            painter->setClipping(false);
            painter->setPen(option->palette.foreground().color());
            drawRoundRect(painter, x, y, width, height, minorDiameter);
            painter->setPen(oldPen);
            break;
        }
    default:
        QWindowsStyle::drawPrimitive(element, option, painter, widget);
    }
}

void NorwegianWoodStyle::drawControl(ControlElement element,
                                     const QStyleOption *option,
                                     QPainter *painter,
                                     const QWidget *widget) const
{
    switch(element) {
    case CE_PushButtonLabel:
        {
            QStyleOptionButton myButtonOption;
            const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option);
            if (buttonOption) {
                myButtonOption = *buttonOption;
                if (myButtonOption.state & (State_Down | State_On))
                    myButtonOption.palette.setBrush(QPalette::ButtonText, /*myButtonOption.palette.brightText()*/Qt::white);
            }
            QWindowsStyle::drawControl(element, &myButtonOption, painter, widget);
        }
        break;
    case CE_ScrollBarAddLine:
        if (option->state & State_Horizontal)
            drawSemiCircleButton(PointRight, painter, option);
        else
            drawSemiCircleButton(PointDown, painter, option);
        break;
    case CE_ScrollBarSubLine:
        if (option->state & State_Horizontal)
            drawSemiCircleButton(PointLeft, painter, option);
        else
            drawSemiCircleButton(PointUp, painter, option);
        break;
    default:
        QWindowsStyle::drawControl(element, option, painter, widget);
    }
}

#if 0
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

void NorwegianWoodStyle::drawSemiCircleButton(Direction direction, QPainter *painter,
                                              const QStyleOption *option) const
{
    int b = pixelMetric(PM_ScrollBarExtent) > 20 ? 3 : 2;

    QRect r = option->rect.adjusted(2, 2, -2, -2);
    QPalette g = option->palette;
    QRegion extrn(r.x(), r.y(), r.width(), r.height(), QRegion::Ellipse);
    QRegion intern(r.x() + b, r.y() + b, r.width() - 2 * b, r.height() - 2 * b, QRegion::Ellipse);
    int w2 = r.width() / 2;
    int h2 = r.height() / 2;

    switch (direction) {
    case PointRight:
        extrn += QRegion(r.x(),  r.y(),   w2,     r.height());
        intern += QRegion(r.x()+b,r.y()+b, w2-2*b, r.height()-2*b);
        break;
    case PointLeft:
        extrn +=  QRegion(r.x()+w2,  r.y(),   w2,     r.height());
        intern += QRegion(r.x()+w2+b,r.y()+b, w2-2*b, r.height()-2*b);
        break;
    case PointUp:
        extrn +=  QRegion(r.x(),  r.y()+h2,   r.width(),     h2);
        intern += QRegion(r.x()+b,r.y()+h2+b, r.width()-2*b, h2-2*b);
        break;
    case PointDown:
        extrn +=  QRegion(r.x(),  r.y(),   r.width(),     h2);
        intern += QRegion(r.x()+b,r.y()+b, r.width()-2*b, h2-2*b);
        break;
    }

    extrn = extrn - intern;
    QPolygon a;
    a.setPoints(3, r.x(), r.y(), r.x(), r.y() + r.height(), r.x() + r.width(), r.top());

    QRegion oldClip = painter->clipRegion();
    bool bReallyClip = painter->hasClipping();
    painter->setClipRegion(intern);
    painter->fillRect(r, g.button());

    QColor sunnySide = g.light().color();
    QColor shadedSide = g.dark().color();
    if (option->state & State_Down)
        qSwap(sunnySide, shadedSide);

    painter->setClipRegion(QRegion(a) & extrn);
    painter->fillRect(r, sunnySide);

    a.setPoints(3, r.x() + r.width(), r.y() + r.height(), r.x(), r.y() + r.height(),
                r.x() + r.width(), r.y());
    painter->setClipRegion(QRegion(a) & extrn);
    painter->fillRect(r, shadedSide);

    painter->setClipRegion(oldClip);
    painter->setClipping(bReallyClip);
}
