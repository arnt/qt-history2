#ifndef NORWEGIANWOODSTYLE_H
#define NORWEGIANWOODSTYLE_H

#include <QImage>
#include <QMotifStyle>
#include <QPalette>

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
    int styleHint(StyleHint hint, const QStyleOption *option,
                  const QWidget *widget, QStyleHintReturn *returnData) const;

private:
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
