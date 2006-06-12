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

#ifndef QPICTURE_H
#define QPICTURE_H

#include <QtGui/qpaintdevice.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_PICTURE

class QPicturePrivate;
class Q_GUI_EXPORT QPicture : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QPicture)
public:
    explicit QPicture(int formatVersion = -1);
    QPicture(const QPicture &);
    ~QPicture();

    bool isNull() const;

    int devType() const;
    uint size() const;
    const char* data() const;
    virtual void setData(const char* data, uint size);

    bool play(QPainter *p);

    bool load(QIODevice *dev, const char *format = 0);
    bool load(const QString &fileName, const char *format = 0);
    bool save(QIODevice *dev, const char *format = 0);
    bool save(const QString &fileName, const char *format = 0);

    QRect boundingRect() const;
    void setBoundingRect(const QRect &r);

    QPicture& operator=(const QPicture &p);
    void detach();
    bool isDetached() const;

    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &in, const QPicture &p);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QPicture &p);

    static const char* pictureFormat(const QString &fileName);
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();
    static QStringList inputFormatList();
    static QStringList outputFormatList();

    QPaintEngine *paintEngine() const;

protected:
    QPicture(QPicturePrivate &data);

    int metric(PaintDeviceMetric m) const;
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QPicture copy() const { QPicture p(*this); p.detach(); return p; }
#endif

private:
    bool exec(QPainter *p, QDataStream &ds, int i);
    void detach_helper();

    QPicturePrivate *d_ptr;
    friend class QPicturePaintEngine;
    friend class Q3Picture;
};

Q_DECLARE_SHARED(QPicture)


#ifndef QT_NO_PICTUREIO
class QIODevice;
class QPictureIO;
typedef void (*picture_io_handler)(QPictureIO *); // picture IO handler

struct QPictureIOData;

class Q_GUI_EXPORT QPictureIO
{
public:
    QPictureIO();
    QPictureIO(QIODevice *ioDevice, const char *format);
    QPictureIO(const QString &fileName, const char *format);
    ~QPictureIO();

    const QPicture &picture() const;
    int status() const;
    const char *format() const;
    QIODevice *ioDevice() const;
    QString fileName() const;
    int quality() const;
    QString description() const;
    const char *parameters() const;
    float gamma() const;

    void setPicture(const QPicture &);
    void setStatus(int);
    void setFormat(const char *);
    void setIODevice(QIODevice *);
    void setFileName(const QString &);
    void setQuality(int);
    void setDescription(const QString &);
    void setParameters(const char *);
    void setGamma(float);

    bool read();
    bool write();

    static QByteArray pictureFormat(const QString &fileName);
    static QByteArray pictureFormat(QIODevice *);
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();

    static void defineIOHandler(const char *format,
                                const char *header,
                                const char *flags,
                                picture_io_handler read_picture,
                                picture_io_handler write_picture);

private:
    Q_DISABLE_COPY(QPictureIO)

    void init();

    QPictureIOData *d;
};

#endif //QT_NO_PICTUREIO


/*****************************************************************************
  QPicture stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPicture &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPicture &);

#endif // QT_NO_PICTURE

QT_END_HEADER

#endif // QPICTURE_H
