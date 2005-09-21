/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QBMPHANDLER_P_H
#define QBMPHANDLER_P_H

#include "QtGui/qimageiohandler.h"

#ifndef QT_NO_IMAGEFORMAT_BMP
class Q_GUI_EXPORT QBmpHandler : public QImageIOHandler
{
public:
    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);
    
    QByteArray name() const;

    static bool canRead(QIODevice *device);
};
#endif

#endif // QBMPHANDLER_P_H
