#include <QtGui>

#include "norwegianwoodstyle.h"

void NorwegianWoodStyle::polish(QPalette &palette)
{
    QColor brown(212, 140, 95);
    QColor beige(236, 182, 120);
    QColor slightlyOpaqueBlack(0, 0, 0, 63);

    QImage backgroundImage(":/images/woodbackground.png");
    QImage buttonImage(":/images/woodbutton.png");
    QImage midImage = buttonImage;

    QPainter painter;
    painter.begin(&midImage);
    painter.setPen(Qt::NoPen);
    painter.fillRect(midImage.rect(), slightlyOpaqueBlack);
    painter.end();

    palette = QPalette(brown);

    palette.setBrush(QPalette::BrightText, Qt::white);
    palette.setBrush(QPalette::Base, beige);
    palette.setBrush(QPalette::Highlight, Qt::darkGreen);
    setBrushPixmap(palette, QPalette::Button, buttonImage);
    setBrushPixmap(palette, QPalette::Mid, midImage);
    setBrushPixmap(palette, QPalette::Background, backgroundImage);

    QBrush brush = palette.background();
    brush.setColor(brush.color().dark());

    palette.setBrush(QPalette::Disabled, QPalette::Foreground, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Mid, brush);
}

int NorwegianWoodStyle::pixelMetric(PixelMetric metric,
                                    const QStyleOption *option,
                                    const QWidget *widget) const
{
    switch (metric) {
    case PM_ComboBoxFrameWidth:
        return 8;
    case PM_ScrollBarExtent:
        return QMotifStyle::pixelMetric(metric, option, widget) + 4;
    default:
        return QMotifStyle::pixelMetric(metric, option, widget);
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
            int radius = qMin(width, height) / 2;
            QPainterPath roundRect = roundRectPath(option->rect);

            QColor slightlyOpaqueBlack(0, 0, 0, 63);
            QColor halfTransparentWhite(255, 255, 255, 127);
            QColor halfTransparentBlack(0, 0, 0, 127);

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            QBrush brush;
            bool darker;

            if (buttonOption
                    && (buttonOption->features & QStyleOptionButton::Flat)) {
                brush = option->palette.background();
                darker = (option->state & (State_Down | State_On));
            } else {
                if (option->state & (State_Down | State_On)) {
                    brush = option->palette.mid();
                    darker = !(option->state & State_Down);
                } else {
                    brush = option->palette.button();
                    darker = false;
                }
            }

            // ### replace with fillPath()
            painter->setClipPath(roundRect);
            painter->fillRect(option->rect, brush);
            if (darker)
                painter->fillRect(option->rect, slightlyOpaqueBlack);

            int penWidth;
            if (radius < 10)
                penWidth = 3;
            else if (radius < 20)
                penWidth = 5;
            else
                penWidth = 7;

            QPen topLeftPen(halfTransparentWhite, penWidth);
            QPen bottomRightPen(halfTransparentBlack, penWidth);

            if (option->state & (State_Down | State_On))
                qSwap(topLeftPen, bottomRightPen);

            QPolygon topLeftHalf;
            topLeftHalf << QPoint(x, y)
                        << QPoint(x + width, y)
                        << QPoint(x + width - radius, y + radius)
                        << QPoint(x + radius, y + height - radius)
                        << QPoint(x, y + height);

            painter->setClipPath(roundRect);
            painter->setClipRegion(topLeftHalf, Qt::IntersectClip);
            painter->setPen(topLeftPen);
            painter->drawPath(roundRect);

            QPolygon bottomRightHalf = topLeftHalf;
            bottomRightHalf[0] = QPoint(x + width, y + width);

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

int NorwegianWoodStyle::styleHint(StyleHint hint, const QStyleOption *option,
                                  const QWidget *widget,
                                  QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_DitherDisabledText:
        return int(false);
    case SH_EtchDisabledText:
        return int(true);
    default:
        return QMotifStyle::styleHint(hint, option, widget, returnData);
    }
}

void NorwegianWoodStyle::setBrushPixmap(QPalette &palette,
                                        QPalette::ColorRole role,
                                        const QPixmap &pixmap)
{
    for (int i = 0; i < QPalette::NColorGroups; ++i) {
        QColor color = palette.brush(QPalette::ColorGroup(i), role).color();
        palette.setBrush(QPalette::ColorGroup(i), role, QBrush(color, pixmap));
    }
}

QPainterPath NorwegianWoodStyle::roundRectPath(const QRect &rect)
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
