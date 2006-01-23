#include "calibration.h"

#include <QWSPointerCalibrationData>
#include <QPainter>
#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QScreen>
#include <QWSServer>

Calibration::Calibration()
{
    QRect desktop = QApplication::desktop()->geometry();
    desktop.moveTo(QPoint(0, 0));
    setGeometry(desktop);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setModal(true);

    int w = qt_screen->deviceWidth();
    int h = qt_screen->deviceHeight();

    // non-linearity is expected to increase on the edge of the screen,
    // so keep all points 10 percent within the screen.
    int dx = w / 10;
    int dy = h / 10;

    // fill data with screen coordinates
    QPoint *points = data.screenPoints;
    points[QWSPointerCalibrationData::TopLeft] = QPoint(dx, dy);
    points[QWSPointerCalibrationData::BottomLeft] = QPoint(dx, h - dy);
    points[QWSPointerCalibrationData::BottomRight] = QPoint(w - dx, h - dy);
    points[QWSPointerCalibrationData::TopRight] = QPoint(w - dx, dy);
    points[QWSPointerCalibrationData::Center] = QPoint(w / 2, h / 2);

    pressCount = 0;
}

Calibration::~Calibration()
{
}

void Calibration::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    // draw a cross at the point the user should press
    QPoint point = data.screenPoints[pressCount];
    p.fillRect(point.x() - 6, point.y() - 1, 13, 3, Qt::black);
    p.fillRect(point.x() - 1, point.y() - 6, 3, 13, Qt::black);
}

void Calibration::mouseReleaseEvent(QMouseEvent *event)
{
    QSize screenSize(qt_screen->width(), qt_screen->height());
    QPoint p = qt_screen->mapToDevice(event->pos(), screenSize);
    data.devPoints[pressCount] = qt_screen->mapToDevice(event->pos(),
                                                        screenSize);

    if (++pressCount < 5)
        repaint();
    else
        accept();
}

int Calibration::exec()
{
    QWSServer::mouseHandler()->clearCalibration();
    grabMouse();
    activateWindow();
    int ret = QDialog::exec();
    releaseMouse();
    return ret;
}

void Calibration::accept()
{
    Q_ASSERT(pressCount == 5);
    QWSServer::mouseHandler()->calibrate(&data);
    QDialog::accept();
}

