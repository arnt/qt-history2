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
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const;
#if 0
    void drawControl(ControlElement control, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;
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
    void setBrush(QPalette &palette, QPalette::ColorRole role,
                  const QBrush &brush);
    void setBrushPixmap(QPalette &palette, QPalette::ColorRole role,
                        const QPixmap &pixmap);
    void drawSemicircleButton(QPainter *p, const QRect &r, int dir,
			      bool sunken, const QColorGroup &g ) const;

    QImage buttonImage;
    QImage lightImage;
    QImage darkImage;
    QImage midImage;
    QImage backgroundImage;
    QImage sunkenLightImage;
    QImage sunkenDarkImage;
    QPalette woodPalette;
};

#endif
