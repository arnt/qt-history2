#ifndef QTEXTCODECINTERFACE_H
#define QTEXTCODECINTERFACE_H

#ifndef QT_H
#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_TEXTCODEC
#ifndef QT_NO_COMPONENT

class QTextCodec;


// {F55BFA60-F695-11D4-823E-009027DC0F37}
#ifndef IID_QTextCodecFactory
#define IID_QTextCodecFactory QUuid( 0xf55bfa60, 0xf695, 0x11d4, 0x82, 0x3e, 0x00, 0x90, 0x27, 0xdc, 0x0f, 0x37)
#endif


struct Q_EXPORT QTextCodecFactoryInterface : public QFeatureListInterface
{
    virtual QTextCodec *createForMib( int mib ) = 0;
    virtual QTextCodec *createForName( const QString &name ) = 0;
};

#endif // QT_NO_COMPONENT
#endif // QT_NO_TEXTCODEC

#endif // QTEXTCODECINTERFACE_H
