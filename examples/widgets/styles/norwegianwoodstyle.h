#ifndef NORWEGIANWOODSTYLE_H
#define NORWEGIANWOODSTYLE_H

#include <QPalette>
#include <QWindowsStyle>

class NorwegianWoodStyle : public QWindowsStyle
{
    Q_OBJECT

public:
    NorwegianWoodStyle();

    void polish(QPalette &palette);

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const;
    void drawControl(ControlElement control, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;
#if 0
    void drawComplexControl(ControlElement control,
                            const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget) const;
    SubControl hitTestComplexControl(ComplexControl control,
                                     const QStyleOptionComplex *option,
                                     const QPoint &pos,
                                     const QWidget *widget) const;
    QRect subControlRect(ComplexControl control,
                         const QStyleOptionComplex *option,
                         SubControl subControl, const QWidget *widget) const;
#endif

private:
    enum Direction { PointUp, PointDown, PointLeft, PointRight };

    void setBrush(QPalette &palette, QPalette::ColorRole role,
                  const QBrush &brush);
    void setBrushPixmap(QPalette &palette, QPalette::ColorRole role,
                        const QPixmap &pixmap);
    void drawSemiCircleButton(Direction direction, QPainter *painter,
                              const QStyleOption *option) const;

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
