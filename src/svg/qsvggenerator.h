/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSVGGENERATOR_H
#define QSVGGENERATOR_H

#include <QtGui/qpaintdevice.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qobjectdefs.h>

QT_BEGIN_HEADER

QT_MODULE(Svg)
    
class QSvgGeneratorPrivate;

class Q_SVG_EXPORT QSvgGenerator : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QSvgGenerator)
public:
    QSvgGenerator();
    ~QSvgGenerator();

    QSize size() const;
    void setSize(const QSize &size);

    QString fileName() const;
    void setFileName(const QString &fileName);

    QIODevice *outputDevice() const;
    void setOutputDevice(QIODevice *outputDevice);

    void setResolution(int resolution);

protected:
    QPaintEngine *paintEngine() const;  
    int metric(QPaintDevice::PaintDeviceMetric metric) const;

private:
    QSvgGeneratorPrivate *d_ptr;
};

QT_END_HEADER

#endif // QSVGGENERATOR_H
