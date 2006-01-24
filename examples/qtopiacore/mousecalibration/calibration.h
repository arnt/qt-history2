#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QDialog>
#include <QWSPointerCalibrationData>

class Calibration : public QDialog
{
public:
    Calibration();
    ~Calibration();
    int exec();

protected:
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void accept();

private:
    QWSPointerCalibrationData data;
    int pressCount;
};


#endif
