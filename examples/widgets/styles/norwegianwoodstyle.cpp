#include <QtGui>

#include "norwegianwoodstyle.h"

static const int NumGroups = 3;
static const QPalette::ColorGroup groups[NumGroups] = {
    QPalette::Active, QPalette::Disabled, QPalette::Inactive
};

static QPainterPath roundRectPath(const QRect &rect)
{
    int radius = qMin(rect.width(), rect.height()) / 2;

    int x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    QPainterPath path;
    path.moveTo(x1 + radius, y1);
    path.lineTo(x2 - radius, y1);
    path.arcTo(QRect(x2 - 2 * radius, y1, 2 * radius, 2 * radius), 90.0, -90.0);
    path.lineTo(x2, y2 - radius);
    path.arcTo(QRect(x2 - 2 * radius, y2 - 2 * radius, 2 * radius, 2 * radius), 0.0, -90.0);
    path.lineTo(x1 + radius, y2);
    path.arcTo(QRect(x1, y2 - 2 * radius, 2 * radius, 2 * radius), -90.0, -90.0);
    path.lineTo(x1, y1 + radius);
    path.arcTo(QRect(x1, y1, 2 * radius, 2 * radius), -180.0, -90.0);
    return path;
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

int NorwegianWoodStyle::pixelMetric(PixelMetric pm, const QStyleOption *option,
                                    const QWidget *widget) const
{
    switch (pm) {
    case PM_ComboBoxFrameWidth:
        return 8;
    case PM_ScrollBarExtent:
        return QMotifStyle::pixelMetric(pm, option, widget) + 4;
    default:
        return QMotifStyle::pixelMetric(pm, option, widget);
    }
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
            painter->setRenderHint(QPainter::Antialiasing, true);

            const QStyleOptionButton *buttonOption =
                    qstyleoption_cast<const QStyleOptionButton *>(option);
            bool isFlat = (buttonOption->features & QStyleOptionButton::Flat);

            int minorSide = qMin(width, height) / 2;
            int thickness = buttonThickness(minorSide);
            QRect insideRect = option->rect.adjusted(thickness, thickness,
                                                     -thickness, -thickness);

            QPainterPath outside = roundRectPath(option->rect);
            QPainterPath inside = roundRectPath(insideRect);
/*
            QPolygon sunnySide;
            sunnySide << QPoint(x, y)
                      << QPoint(x + width - 1, y)
                      << QPoint(x + width - 1 - minorSide, y + minorSide)
                      << QPoint(x + minorSide, y + height - 1 - minorSide)
                      << QPoint(x, y + height - 1);
*/

            QPen oldPen = painter->pen();

            QBrush brush;

            if (option->state & (State_Down | State_On)) {
                if (isFlat) {
                    brush = QBrush(option->palette.mid().color(),
                                   sunkenMidImage);
                } else {
                    brush = option->palette.mid();
                }
            } else {
                brush = option->palette.button();
            }

            painter->fillPath(inside, brush);
            if ((option->state & (State_Down | State_On)) == State_On) {
                painter->fillPath(inside,
                                  QBrush(brush.color(), Qt::Dense4Pattern));
            }

#if 0
            QPainterPath inBetween = inside;
            inBetween.addPath(outside);
            painter->setClipPath(inBetween);
            painter->fillRect(option->rect,
                    (option->state & (State_Down | State_On)
                     ? QBrush(option->palette.dark().color(), sunkenDarkImage)
                     : option->palette.brush(QPalette::Light)));

            if (option->state & (State_Raised | State_Down | State_On)) {
                sunnySide[0] = QPoint(x + width - 1, y + width - 1);
                painter->setClipPath((QPath(sunnySide) - inside) & outside);

                painter->fillRect(option->rect,
                        (option->state & (State_Down | State_On)
                         ? QBrush(option->palette.light().color(),
                                  sunkenLightImage)
                         : option->palette.dark()));
            }
#endif
            painter->setClipping(false);
            painter->setPen(option->palette.foreground().color());
            painter->drawPath(outside);
            painter->setPen(oldPen);
            painter->setRenderHint(QPainter::Antialiasing, false);
        }
        break;
    default:
        QMotifStyle::drawPrimitive(element, option, painter, widget);
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
            const QStyleOptionButton *buttonOption =
                    qstyleoption_cast<const QStyleOptionButton *>(option);
            if (buttonOption) {
                myButtonOption = *buttonOption;
                if (myButtonOption.state & (State_Down | State_On)) {
                    myButtonOption.palette.setBrush(QPalette::ButtonText,
                            myButtonOption.palette.brightText());
                }
            }
            QMotifStyle::drawControl(element, &myButtonOption, painter, widget);
        }
        break;
    default:
        QMotifStyle::drawControl(element, option, painter, widget);
    }
}

void NorwegianWoodStyle::setBrush(QPalette &palette, QPalette::ColorRole role,
                                  const QBrush &brush)
{
    for (int i = 0; i < NumGroups; ++i)
        palette.setBrush(groups[i], role, brush);
}

void NorwegianWoodStyle::setBrushPixmap(QPalette &palette,
                                        QPalette::ColorRole role,
                                        const QPixmap &pixmap)
{
    for (int i = 0; i < NumGroups; ++i) {
        QColor color = palette.brush(groups[i], role).color();
        palette.setBrush(groups[i], role, QBrush(color, pixmap));
    }
}
