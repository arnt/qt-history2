#include <QtGui>

#include "norwegianwoodstyle.h"

static const int NumGroups = 3;
static const QPalette::ColorGroup groups[NumGroups] = {
    QPalette::Active, QPalette::Disabled, QPalette::Inactive
};

static QPainterPath roundRectPath(const QRect &rect)
{
    int radius = qMin(rect.width(), rect.height()) / 2;
    int diam = 2 * radius;

    int x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    QPainterPath path;
    path.moveTo(x2, y1 + radius);
    path.arcTo(QRect(x2 - diam, y2 - diam, diam, diam), 0.0, +90.0);
    path.lineTo(x1 + radius, y1);
    path.arcTo(QRect(x1, y1, diam, diam), 90.0, +90.0);
    path.lineTo(x1, y2 - radius);
    path.arcTo(QRect(x1, y2 - diam, diam, diam), 180.0, +90.0);
    path.lineTo(x1 + radius, y2);
    path.arcTo(QRect(x2 - diam, y2 - diam, diam, diam), 270.0, +90.0);
    path.closeSubpath();
    return path;
}

NorwegianWoodStyle::NorwegianWoodStyle()
{
    buttonImage.load(":/images/woodbutton.xpm");
    lightImage = buttonImage;
    darkImage = buttonImage;
    midImage = buttonImage;
    backgroundImage.load(":/images/woodbackground.xpm");
    sunkenMidImage = backgroundImage;

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

        sunkenMidImage.setColor(j, color.dark(120).rgb());
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
            const QStyleOptionButton *buttonOption =
                    qstyleoption_cast<const QStyleOptionButton *>(option);
            bool flat = buttonOption
                        && (buttonOption->features & QStyleOptionButton::Flat);
            int minorSide = qMin(width, height) / 2;
            QPainterPath roundRect = roundRectPath(option->rect);

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            QBrush brush;
            if (option->state & (State_Down | State_On)) {
                if (flat) {
                    brush = QBrush(option->palette.mid().color(),
                                   sunkenMidImage);
                } else {
                    brush = option->palette.mid();
                }
            } else {
                brush = option->palette.button();
            }

            painter->setClipPath(roundRect);
            painter->fillRect(option->rect, brush); // ### replace with fillPath()
            if ((option->state & (State_Down | State_On)) == State_On) {
                painter->fillRect(option->rect,
                                  QBrush(brush.color(), Qt::Dense4Pattern)); // ### ditto
            }

            int penWidth;
            if (minorSide < 10)
                penWidth = 3;
            else if (minorSide < 20)
                penWidth = 5;
            else
                penWidth = 7;

            QPen topLeftPen(QColor(255, 255, 255, 127), penWidth);
            QPen bottomRightPen(QColor(0, 0, 0, 127), penWidth);

            if (option->state & (State_Down | State_On))
                qSwap(topLeftPen, bottomRightPen);

            QPolygon topLeftHalf;
            topLeftHalf << QPoint(x, y)
                        << QPoint(x + width - 1, y)
                        << QPoint(x + width - 1 - minorSide, y + minorSide)
                        << QPoint(x + minorSide, y + height - 1 - minorSide)
                        << QPoint(x, y + height - 1);

            painter->setClipPath(roundRect);
            painter->setClipRegion(topLeftHalf, Qt::IntersectClip);
            painter->setPen(topLeftPen);
            painter->drawPath(roundRect);

            QPolygon bottomRightHalf = topLeftHalf;
            bottomRightHalf[0] = QPoint(x + width - 1, y + width - 1);

            painter->setClipPath(roundRect);
            painter->setClipRegion(bottomRightHalf, Qt::IntersectClip);
            painter->setPen(bottomRightPen);
            painter->drawPath(roundRect);

            painter->setPen(option->palette.foreground().color());
            painter->setClipping(false);
            painter->drawPath(roundRect);

            painter->restore();
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
