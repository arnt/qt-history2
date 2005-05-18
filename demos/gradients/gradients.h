#ifndef GRADIENTS_H
#define GRADIENTS_H

#include <QtGui>

#include "arthurwidgets.h"

class HoverPoints;


class ShadeWidget : public QWidget
{
    Q_OBJECT
public:
    enum ShadeType {
        RedShade,
        GreenShade,
        BlueShade,
        ARGBShade
    };

    ShadeWidget(ShadeType type, QWidget *parent);

    void setGradientStops(const QGradientStops &stops);

    void paintEvent(QPaintEvent *e);

    QSize sizeHint() const { return QSize(150, 40); }
    QPolygonF points() const;

    HoverPoints *hoverPoints() const { return m_hoverPoints; }

    uint colorAt(int x);

signals:
    void colorsChanged();

private:
    void generateShade();

    ShadeType m_shade_type;
    QImage m_shade;
    HoverPoints *m_hoverPoints;
    QLinearGradient m_alpha_gradient;
};

class GradientEditor : public QWidget
{
    Q_OBJECT
public:
    GradientEditor(QWidget *parent);

    void setGradientStops(const QGradientStops &stops);

public slots:
    void pointsUpdated();

signals:
    void gradientStopsChanged(const QGradientStops &stops);

private:
    ShadeWidget *m_red_shade;
    ShadeWidget *m_green_shade;
    ShadeWidget *m_blue_shade;
    ShadeWidget *m_alpha_shade;
};


class GradientRenderer : public ArthurFrame
{
    Q_OBJECT
public:
    GradientRenderer(QWidget *parent);
    void paint(QPainter *p);

    QSize sizeHint() const { return QSize(400, 400); }

    HoverPoints *hoverPoints() const { return m_hoverPoints; }
    void mousePressEvent(QMouseEvent *e);

public slots:
    void setGradientStops(const QGradientStops &stops);

    void setPadSpread() { m_spread = QGradient::PadSpread; update(); }
    void setRepeatSpread() { m_spread = QGradient::RepeatSpread; update(); }
    void setReflectSpread() { m_spread = QGradient::ReflectSpread; update(); }

    void setLinearGradient() { m_gradientType = Qt::LinearGradientPattern; update(); }
    void setRadialGradient() { m_gradientType = Qt::RadialGradientPattern; update(); }
    void setConicalGradient() { m_gradientType = Qt::ConicalGradientPattern; update(); }


private:
    QGradientStops m_stops;
    HoverPoints *m_hoverPoints;

    QGradient::Spread m_spread;
    Qt::BrushStyle m_gradientType;
};


class GradientWidget : public QWidget
{
    Q_OBJECT
public:
    GradientWidget(QWidget *parent);

public slots:
    void setDefault1() { setDefault(1); }
    void setDefault2() { setDefault(2); }
    void setDefault3() { setDefault(3); }
    void setDefault4() { setDefault(4); }

private:
    void setDefault(int i);

    GradientRenderer *m_renderer;
    GradientEditor *m_editor;

    QRadioButton *m_linearButton;
    QRadioButton *m_radialButton;
    QRadioButton *m_conicalButton;
    QRadioButton *m_padSpreadButton;
    QRadioButton *m_reflectSpreadButton;
    QRadioButton *m_repeatSpreadButton;


};

#endif // GRADIENTS_H
