#include <QtGui>
#include <math.h>

#include "tabletcanvas.h"

TabletCanvas::TabletCanvas()
{
    myBrush = QBrush();
    myPen = QPen();
    image = QImage(500, 500, QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.fillRect(0, 0, 499, 499, Qt::white);
    setAutoFillBackground(true);
    deviceDown = false;
    myColor = Qt::red;
    myTabletDevice = QTabletEvent::Stylus;
    alphaChannelType = NoAlpha;
    colorSaturationType = NoSaturation;
    lineWidthType = LineWidthPressure;
}

bool TabletCanvas::saveImage(const QString &file)
{
    return image.save(file);
}

bool TabletCanvas::loadImage(const QString &file)
{
    bool success = image.load(file);
    
    if (success) {
	update();
	return true;
    }
    return false;
}

void TabletCanvas::tabletEvent(QTabletEvent *event)
{    

    switch (event->type()) {
	case QEvent::TabletPress:
	    //polyLine[0] = polyLine[1] = polyLine[2] = event->pos();
	    if (deviceDown)
		qDebug("Tablet Press: Device already down");
	    deviceDown = true;
	    break;
	case QEvent::TabletRelease:
	    if (!deviceDown)
		qDebug("Tablet Release: Device not down.");
	    deviceDown = false;
	    break;
	case QEvent::TabletMove:
	    polyLine[2] = polyLine[1];
	    polyLine[1] = polyLine[0];
	    polyLine[0] = event->pos();

	    if (deviceDown) {
		updateBrush(event);
		QPainter painter(&image);
		paintImage(painter, event);
	    }
	    break;
	default:
	    break;
    }
    update();
}

void TabletCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), image);
}

void TabletCanvas::paintImage(QPainter &painter, QTabletEvent *event)
{
    QPoint brushAdjust(10, 10);
    switch (myTabletDevice) {
	case QTabletEvent::Stylus:
		painter.setBrush(myBrush);
		painter.setPen(myPen);
		//painter.drawPolyline(polyLine, 3);
		painter.drawLine(polyLine[1], event->pos());
	    break;
	case QTabletEvent::Airbrush:
		myBrush.setColor(myColor);
		myBrush.setStyle(brushPattern(event->pressure()));
		painter.setPen(Qt::NoPen);
		painter.setBrush(myBrush);

		for (int i = 0; i < 3; ++i) {
		    painter.drawEllipse(QRect(polyLine[i] - brushAdjust,
					      polyLine[i] + brushAdjust));
		}
	    break;	
	default:
	    qWarning("Unsupported tablet device.");
    } 
}

Qt::BrushStyle TabletCanvas::brushPattern(qreal value)
{
    int pattern = int((value) * 100.0) % 7;
    qDebug("value is: %f", value);
    switch (pattern) {
	case 0:
	    return Qt::SolidPattern;
	case 1:
	    return Qt::Dense1Pattern;
	case 2:
	    return Qt::Dense2Pattern;
	case 3:
	    return Qt::Dense3Pattern;
	case 4:
	    return Qt::Dense4Pattern;
	case 5:
	    return Qt::Dense5Pattern;
	case 6:
	    return Qt::Dense6Pattern;
	default:
	    return Qt::Dense7Pattern;
    }
}

void TabletCanvas::updateBrush(QTabletEvent *event)
{
    int hue, saturation, value, alpha;
    myColor.getHsv(&hue, &saturation, &value, &alpha);

    if (event->yTilt() < -60)
	qDebug("Y-tilt is: %d", event->yTilt());
    if (event->xTilt() < -60)
	qDebug("X-tilt is: %d", event->xTilt());

    int vValue = int(((event->yTilt() + 60.0) / 120.0) * 255);
    int hValue = int(((event->xTilt() + 60.0) / 120.0) * 255);

    switch (alphaChannelType) {
	case AlphaPressure:
	    myColor.setAlpha(int(event->pressure() * 255.0));
	    break;
	case AlphaTilt:
	    myColor.setAlpha(max(abs(vValue - 127), abs(hValue - 127)));
	    break;
	default:
	    myColor.setAlpha(255);
    }

    switch (colorSaturationType) {
	case SaturationVTilt:
	    myColor.setHsv(hue, vValue, value, alpha); 
	    break;
	case SaturationHTilt:
	    myColor.setHsv(hue, hValue, value, alpha);
	    break;
	case SaturationPressure:
	    myColor.setHsv(hue, int(event->pressure() * 255.0), value, alpha);
	    break;
	default:
	    ;
    }
    
    switch (lineWidthType) {
	case LineWidthPressure:
	    myPen.setWidthF(event->pressure() * 10 + 1);
	    break;
	case LineWidthTilt:
	    myPen.setWidthF(max(abs(vValue - 127), abs(hValue - 127)) / 12);
	    break;
	default:
	    myPen.setWidthF(1);
    }

    if (event->pointerType() == QTabletEvent::Eraser) { 
	myBrush.setColor(Qt::white);
	myPen.setColor(Qt::white);
	myPen.setWidthF(event->pressure() * 10 + 1);
    } else {	
	myBrush.setColor(myColor);
	myPen.setColor(myColor);
    }
}
