/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPICTUREFORMATINTERFACE_P_H
#define QPICTUREFORMATINTERFACE_P_H

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

// {04903F05-54B1-4726-A849-FB5CB097CA88} 
#ifndef IID_QPictureFormat
#define IID_QPictureFormat QUuid( 0x04903f05, 0x54b1, 0x4726, 0xa8, 0x49, 0xfb, 0x5c, 0xb0, 0x97, 0xca, 0x88 )
#endif

class QPicutre;

struct Q_GUI_EXPORT QPictureFormatInterface : public QFeatureListInterface
{
    virtual QRESULT loadPicture( const QString &format, const QString &filename, QPicture * ) = 0;
    virtual QRESULT savePicture( const QString &format, const QString &filename, const QPicture & ) = 0;

    virtual QRESULT installIOHandler( const QString & ) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QIMAGEFORMATINTERFACE_P_H
