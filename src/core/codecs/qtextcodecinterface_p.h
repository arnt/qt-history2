/****************************************************************************
**
** Definition of QTextCodecFactoryInterface interface.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTCODECINTERFACE_P_H
#define QTEXTCODECINTERFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qpsprinter.cpp and qprinter_x11.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//

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


struct Q_CORE_EXPORT QTextCodecFactoryInterface : public QFeatureListInterface
{
    virtual QTextCodec *createForMib( int mib ) = 0;
    virtual QTextCodec *createForName( const QString &name ) = 0;
};

#endif // QT_NO_COMPONENT
#endif // QT_NO_TEXTCODEC

#endif // QTEXTCODECINTERFACE_P_H
