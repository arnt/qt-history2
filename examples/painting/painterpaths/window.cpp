#include <QtGui>

#include "renderarea.h"
#include "window.h"

Window::Window()
{
    QPainterPath rectPath;
    rectPath.moveTo(20.0, 30.0);
    rectPath.lineTo(80.0, 30.0);
    rectPath.lineTo(80.0, 70.0);
    rectPath.lineTo(20.0, 70.0);
    rectPath.closeSubpath();

    QPainterPath roundRectPath;
    roundRectPath.moveTo(80.0, 35.0);
    roundRectPath.arcTo(70.0, 30.0, 10.0, 10.0, 0.0, 90.0);
    roundRectPath.lineTo(25.0, 30.0);
    roundRectPath.arcTo(20.0, 30.0, 10.0, 10.0, 90.0, 90.0);
    roundRectPath.lineTo(20.0, 65.0);
    roundRectPath.arcTo(20.0, 60.0, 10.0, 10.0, 180.0, 90.0);
    roundRectPath.lineTo(75.0, 70.0);
    roundRectPath.arcTo(70.0, 60.0, 10.0, 10.0, 270.0, 90.0);
    roundRectPath.closeSubpath();

    QPainterPath ellipsePath;
    ellipsePath.moveTo(80.0, 50.0);
    ellipsePath.arcTo(20.0, 30.0, 60.0, 40.0, 0.0, 360.0);

    QPainterPath piePath;
    piePath.moveTo(50.0, 50.0);
    piePath.lineTo(65.0, 32.679492950439453);
    piePath.arcTo(20.0, 30.0, 60.0, 40.0, 60.0, 240.0);
    piePath.closeSubpath();

    QPainterPath polylinePath;
    polylinePath.moveTo(10.0, 80.0);
    polylinePath.lineTo(20.0, 10.0);
    polylinePath.lineTo(80.0, 30.0);
    polylinePath.lineTo(90.0, 70.0);

    QPainterPath polygonPath = polylinePath;
    polygonPath.closeSubpath();

    QPainterPath textPath;
    QFont timesFont("Times", 60);
    timesFont.setStyleStrategy(QFont::ForceOutline);
    textPath.addText(10, 70, timesFont, tr("Qt"));

    QPainterPath bezierPath;
    bezierPath.moveTo(20, 30);
    bezierPath.curveTo(80, 0, 50, 50, 80, 80);

    QPainterPath compositionPath;

    rectArea = new RenderArea(rectPath, this);
    roundRectArea = new RenderArea(roundRectPath, this);
    ellipseArea = new RenderArea(ellipsePath, this);
    pieArea = new RenderArea(piePath, this);
    polylineArea = new RenderArea(polylinePath, this);
    polygonArea = new RenderArea(polygonPath, this);
    textArea = new RenderArea(textPath, this);
    bezierArea = new RenderArea(bezierPath, this);
    compositionArea = new RenderArea(compositionPath, this);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(rectArea, 0, 0);
    layout->addWidget(roundRectArea, 0, 1);
    layout->addWidget(ellipseArea, 0, 2);
    layout->addWidget(pieArea, 1, 0);
    layout->addWidget(polylineArea, 1, 1);
    layout->addWidget(polygonArea, 1, 2);
    layout->addWidget(textArea, 2, 0);
    layout->addWidget(bezierArea, 2, 1);
    layout->addWidget(compositionArea, 2, 2);

    setWindowTitle(tr("Painter Paths"));
}
