#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

class QImageFormat;
class QImageFormatPluginPrivate;

class Q_EXPORT QImageFormatPlugin : public QGPlugin
{
public:
    QImageFormatPlugin();
    ~QImageFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadImage( const QString &format, const QString &filename, QImage *image );
    virtual bool saveImage( const QString &format, const QString &filename, const QImage &image );
    virtual bool installIOHandler( const QString &format ) = 0;

private:
    QImageFormatPluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
