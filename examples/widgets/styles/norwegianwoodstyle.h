#ifndef NORWEGIANWOODSTYLE_H
#define NORWEGIANWOODSTYLE_H

#include <QPalette>
#include <QMotifStyle>

class NorwegianWoodStyle : public QMotifStyle
{
    Q_OBJECT

public:
    NorwegianWoodStyle();

    void polish(QPalette &palette);

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const;
    void drawControl(ControlElement control, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,
                    const QWidget *widget = 0) const;

private:
    enum Direction { PointUp, PointDown, PointLeft, PointRight };

    void setBrush(QPalette &palette, QPalette::ColorRole role,
                  const QBrush &brush);
    void setBrushPixmap(QPalette &palette, QPalette::ColorRole role,
                        const QPixmap &pixmap);

    QImage buttonImage;
    QImage lightImage;
    QImage darkImage;
    QImage midImage;
    QImage backgroundImage;
    QImage sunkenLightImage;
    QImage sunkenMidImage;
    QImage sunkenDarkImage;
    QPalette woodPalette;
};

#endif
