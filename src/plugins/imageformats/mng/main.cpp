#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <qimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_MNG
#undef QT_NO_IMAGEIO_MNG
#endif
#include "../../../gui/image/qmngio.cpp"

class MNGFormat : public QImageFormatPlugin
{
public:
    MNGFormat();

    QStringList keys();
    bool loadImage(const QString &format, const QString &filename, QImage *image);
    bool saveImage(const QString &format, const QString &filename, const QImage &image);
    bool installIOHandler(const QString &);
};

MNGFormat::MNGFormat()
{
}


QStringList MNGFormat::keys()
{
    QStringList list;
    list << "MNG";

    return list;
}

bool MNGFormat::loadImage(const QString &, const QString &, QImage *)
{
    return false;
}

bool MNGFormat::saveImage(const QString &, const QString &, const QImage&)
{
    return false;
}

bool MNGFormat::installIOHandler(const QString &name)
{
    if (name != "MNG")
        return false;

    qInitMngIO();
    return true;
}

Q_EXPORT_PLUGIN(MNGFormat)

#endif // QT_NO_IMAGEFORMATPLUGIN
