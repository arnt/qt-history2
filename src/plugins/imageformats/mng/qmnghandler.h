/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** WARNING:
**      A separate license from Unisys may be required to use the gif
**      reader. See http://www.unisys.com/about__unisys/lzw/
**      for information from Unisys
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMNGHANDLER_H
#define QMNGHANDLER_H

#include <qimageiohandler.h>

class QImage;
class QByteArray;
class QIODevice;
class QMngHandlerPrivate;

class QMngHandler : public QImageIOHandler
{
    public:
    QMngHandler();
    ~QMngHandler();
    virtual bool canRead() const;
    virtual QByteArray name() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);
    virtual int currentImageNumber() const;
    virtual int imageCount() const;
    virtual bool jumpToImage(int imageNumber);
    virtual bool jumpToNextImage();
    virtual int loopCount() const;
    virtual int nextImageDelay() const;
    static bool canRead(QIODevice *device);

    private:
    Q_DECLARE_PRIVATE(QMngHandler)
    QMngHandlerPrivate *d_ptr;
};

#endif // !QMNGHANDLER_H
