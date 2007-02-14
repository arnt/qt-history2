#ifndef TABLETCANVAS_H
#define TABLETCANVAS_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QTabletEvent>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPoint>

class QPaintEvent;
class QString;

class TabletCanvas : public QWidget
{
    Q_OBJECT

public:
    enum AlphaChannelType { AlphaPressure, AlphaTilt, NoAlpha };
    enum ColorSaturationType { SaturationVTilt, SaturationHTilt, 
			       SaturationPressure, NoSaturation };
    enum LineWidthType { LineWidthPressure, LineWidthTilt, NoLineWidth }; 

    TabletCanvas();
    
    bool saveImage(const QString &file);
    bool loadImage(const QString &file);
    void setAlphaChannelType(AlphaChannelType type) 
	{ alphaChannelType = type; }
    void setColorSaturationType(ColorSaturationType type) 
	{ colorSaturationType = type; }
    void setLineWidthType(LineWidthType type) 
	{ lineWidthType = type; }
    void setColor(const QColor &color)
	{ myColor = color; }
    QColor color() const
	{ return myColor; }
    void setTabletDevice(QTabletEvent::TabletDevice device)
	{ myTabletDevice = device; }
    int max(int a, int b)
	{ return a > b ? a : b; }    

protected:
    void tabletEvent(QTabletEvent *event);
    void paintEvent(QPaintEvent *event);    

private:
    void paintImage(QPainter &painter, QTabletEvent *event);
    Qt::BrushStyle brushPattern(qreal value);
    void updateBrush(QTabletEvent *event);    

    AlphaChannelType alphaChannelType;
    ColorSaturationType colorSaturationType;
    LineWidthType lineWidthType;
    QTabletEvent::PointerType pointerType;
    QTabletEvent::TabletDevice myTabletDevice;
    QColor myColor;

    QImage image;
    QBrush myBrush;
    QPen myPen;
    bool deviceDown;
    QPoint polyLine[3];
};

#endif
