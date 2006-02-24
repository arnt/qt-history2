/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
