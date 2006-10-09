#include <QtCore/qnamespace.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qobjectdefs.h>
#include <QtGui/qpaintdevice.h>

class QSvgGeneratorPrivate;

class QSvgGenerator : public QPaintDevice
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
