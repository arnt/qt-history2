#ifndef QGFXDRIVERINTERFACE_H
#define QGFXDRIVERINTERFACE_H

#ifndef QT_H
#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

// {449EC6C6-DF3E-43E3-9E57-354A3D05AB34}
#ifndef IID_QGfxDriver
#define IID_QGfxDriver QUuid( 0x449ec6c6, 0xdf3e, 0x43e3, 0x9e, 0x57, 0x35, 0x4a, 0x3d, 0x05, 0xab, 0x34)
#endif

class QScreen;

struct Q_EXPORT QGfxDriverInterface : public QFeatureListInterface
{
    virtual QScreen* create( const QString& driver, int displayId ) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QGFXDRIVERINTERFACE_H
