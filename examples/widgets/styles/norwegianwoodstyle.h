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
    int pixelMetric(PixelMetric metric, const QStyleOption *option,
                    const QWidget *widget) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const;
    void drawControl(ControlElement control, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;

private:
    void setBrush(QPalette &palette, QPalette::ColorRole role,
                  const QBrush &brush);
    void setBrushPixmap(QPalette &palette, QPalette::ColorRole role,
                        const QPixmap &pixmap);

    QImage buttonImage;
    QImage lightImage;
    QImage darkImage;
    QImage midImage;
    QImage backgroundImage;
    QImage sunkenMidImage;
    QPalette woodPalette;
};

#endif
