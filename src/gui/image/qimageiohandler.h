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

#ifndef QIMAGEIOHANDLER_H
#define QIMAGEIOHANDLER_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

class QImage;
class QSize;
class QVariant;

class QImageIOHandlerPrivate;
class Q_GUI_EXPORT QImageIOHandler
{
    Q_DECLARE_PRIVATE(QImageIOHandler);
public:
    QImageIOHandler();
    virtual ~QImageIOHandler();

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    virtual QByteArray name() const = 0;

    virtual bool canLoadImage() const = 0;
    virtual bool load(QImage *image) = 0;
    virtual bool save(const QImage &image);

    enum ImageProperty {
        Gamma,
        Quality,
        Resolution,
        Region,
        Name,
        Subtype,
        Parameters,
        Size,
        IncrementalLoading
    };
    virtual QVariant property(ImageProperty property) const;
    virtual void setProperty(ImageProperty property, const QVariant &value);
    virtual bool supportsProperty(ImageProperty property) const;

    // incremental loading
    virtual int loopCount() const;
    virtual int frameCount() const;
    virtual int nextFrameDelay() const;
    virtual int currentFrameNumber() const;

protected:
    QImageIOHandler(QImageIOHandlerPrivate &dd);
    QImageIOHandlerPrivate *d_ptr;
};

struct Q_GUI_EXPORT QImageIOHandlerFactoryInterface : public QFactoryInterface
{
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const = 0;
};

Q_DECLARE_INTERFACE(QImageIOHandlerFactoryInterface, "http://trolltech.com/Qt/QImageIOHandlerFactoryInterface")

class Q_GUI_EXPORT QImageIOPlugin : public QObject, public QImageIOHandlerFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QImageIOHandlerFactoryInterface:QFactoryInterface)
public:
    explicit QImageIOPlugin(QObject *parent = 0);
    inline virtual ~QImageIOPlugin() { }

    enum Capability {
        CanLoad = 0x1,
        CanSave = 0x2,
        CanLoadIncremental = 0x4
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    virtual Capabilities capabilities(QIODevice *device, const QByteArray &format) const = 0;
};

#endif
