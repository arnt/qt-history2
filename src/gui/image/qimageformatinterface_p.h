/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QIMAGEFORMATINTERFACE_P_H
#define QIMAGEFORMATINTERFACE_P_H

#ifndef QT_H
#include <private/qcom_p.h>
#endif // QT_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_NO_COMPONENT

// {04903F05-54B1-4726-A849-FB5CB097CA87}
#ifndef IID_QImageFormat
#define IID_QImageFormat QUuid( 0x04903f05, 0x54b1, 0x4726, 0xa8, 0x49, 0xfb, 0x5c, 0xb0, 0x97, 0xca, 0x87 )
#endif

class QImage;

struct Q_GUI_EXPORT QImageFormatInterface : public QFeatureListInterface
{
    virtual QRESULT loadImage( const QString &format, const QString &filename, QImage * ) = 0;
    virtual QRESULT saveImage( const QString &format, const QString &filename, const QImage & ) = 0;

    virtual QRESULT installIOHandler( const QString & ) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QIMAGEFORMATINTERFACE_P_H
