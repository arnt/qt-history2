#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <qimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_JPEG
#undef QT_NO_IMAGEIO_JPEG
#endif
#include "../../../gui/image/qjpegio.cpp"

class JPEGFormat : public QImageFormatPlugin
{
public:
    JPEGFormat();

    QStringList keys();
    bool loadImage(const QString &format, const QString &filename, QImage *);
    bool saveImage(const QString &format, const QString &filename, const QImage &);
    bool installIOHandler(const QString &);
};

JPEGFormat::JPEGFormat()
{
}


QStringList JPEGFormat::keys()
{
    QStringList list;
    list << "JPEG";

    return list;
}

bool JPEGFormat::loadImage(const QString &format, const QString &filename, QImage *image)
{
    if (format != "JPEG")
        return false;

    QImageIO io;
    io.setFileName(filename);
    io.setImage(*image);

    read_jpeg_image(&io);

    return true;
}

bool JPEGFormat::saveImage(const QString &format, const QString &filename, const QImage &image)
{
    if (format != "JPEG")
        return false;

    QImageIO io;
    io.setFileName(filename);
    io.setImage(image);

    write_jpeg_image(&io);

    return true;
}

bool JPEGFormat::installIOHandler(const QString &name)
{
    if (name.toUpper() != "JPEG")
        return false;

    qInitJpegIO();
    return true;
}

Q_EXPORT_PLUGIN(JPEGFormat)

#endif // QT_NO_IMAGEFORMATPLUGIN
