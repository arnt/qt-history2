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

#ifndef QPICTURE_H
#define QPICTURE_H

#ifndef QT_H
#include "private/qpicture_p.h"
#include "qpaintdevice.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_PICTURE

class Q_GUI_EXPORT QPicture : public QPaintDevice, public QPaintCommands // picture class
{
    Q_DECLARE_PRIVATE(QPicture)
public:
    QPicture(int formatVersion = -1);
    QPicture(const QPicture &);
    ~QPicture();

    inline bool isNull() const;

    inline uint size() const;
    inline const char* data() const;
    virtual void setData(const char* data, uint size);

    bool play(QPainter *p);

    bool load(QIODevice *dev, const char *format = 0);
    bool load(const QString &fileName, const char *format = 0);
    bool save(QIODevice *dev, const char *format = 0);
    bool save(const QString &fileName, const char *format = 0);

    QRect boundingRect() const;
    void setBoundingRect(const QRect &r);

    QPicture& operator=(const QPicture &p);
    inline void        detach() { if (d_ptr->ref != 1) detach_helper(); }
    bool isDetached() const { return d_ptr->ref == 1; }

    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &in, const QPicture &p);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QPicture &p);

#ifndef QT_NO_IMAGEIO
    static const char* pictureFormat(const QString &fileName);
    static QList<QByteArray> inputFormats();
    static QList<QByteArray> outputFormats();
    static QStringList inputFormatList();
    static QStringList outputFormatList();
#endif

    QPaintEngine *paintEngine() const;

protected:
    QPicture(QPicturePrivate &data);

    int metric(int m) const;
#ifdef QT_COMPAT
    inline QT_COMPAT QPicture copy() const { QPicture p(*this); p.detach(); return p; }
#endif

private:
    bool exec(QPainter *p, QDataStream &ds, int i);
    void detach_helper();

    QPicturePrivate *d_ptr;
    friend class QPicturePaintEngine;
};

Q_DECLARE_SHARED(QPicture);

inline bool QPicture::isNull() const
{
    return d_ptr->pictb.buffer().isNull();
}

inline uint QPicture::size() const
{
    return d_ptr->pictb.buffer().size();
}

inline const char* QPicture::data() const
{
    return d_ptr->pictb.buffer();
}


#ifndef QT_NO_PICTUREIO
class QIODevice;
class QPictureIO;
typedef void (*picture_io_handler)(QPictureIO *); // picture IO handler

struct QPictureIOData;

class Q_GUI_EXPORT QPictureIO
{
public:
    QPictureIO();
    QPictureIO(QIODevice         *ioDevice, const char *format);
    QPictureIO(const QString &fileName, const char* format);
   ~QPictureIO();

    const QPicture &picture()        const;
    int                status()        const;
    const char *format()        const;
    QIODevice  *ioDevice()        const;
    QString        fileName()        const;
    int                quality()        const;
    QString        description()        const;
    const char *parameters()        const;
    float gamma() const;

    void        setPicture(const QPicture &);
    void        setStatus(int);
    void        setFormat(const char *);
    void        setIODevice(QIODevice *);
    void        setFileName(const QString &);
    void        setQuality(int);
    void        setDescription(const QString &);
    void        setParameters(const char *);
    void        setGamma(float);

    bool        read();
    bool        write();

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
    void        init();

    QPictureIOData *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPictureIO(const QPictureIO &);
    QPictureIO &operator=(const QPictureIO &);
#endif
};

#endif //QT_NO_PICTUREIO


/*****************************************************************************
  QPicture stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPicture &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPicture &);

#endif // QT_NO_PICTURE

#endif // QPICTURE_H
