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
            const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option);
            if (buttonOption) {
                myButtonOption = *buttonOption;
                if (myButtonOption.state & (State_Down | State_On))
                    myButtonOption.palette.setBrush(QPalette::ButtonText, /*myButtonOption.palette.brightText()*/Qt::white);
            }
            QMotifStyle::drawControl(element, &myButtonOption, painter, widget);
        }
        break;
    default:
        QMotifStyle::drawControl(element, option, painter, widget);
    }
}



int NorwegianWoodStyle::pixelMetric(PixelMetric pm, const QStyleOption *option,
                             const QWidget *widget) const
{
    switch (pm) {
    case PM_ComboBoxFrameWidth:
        return 6;
    case PM_ScrollBarExtent:
        return QMotifStyle::pixelMetric(pm, option, widget) + 4;
    default:
        return QMotifStyle::pixelMetric(pm, option, widget);
    }
}

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

