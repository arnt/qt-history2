#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qobject.h"
#include "qstringlist.h"
#include "qplugin.h"
#endif // QT_H

class QImageFormat;
class QImageFormatPluginPrivate;
struct QUnknownInterface;

class Q_EXPORT QImageFormatPlugin : public QObject
{
public:
    QImageFormatPlugin();
    virtual ~QImageFormatPlugin();

    virtual QStringList featureList() const;
    bool loadImage( const QString &format, const QString &filename, QImage *image );
    bool saveImage( const QString &format, const QString &filename, const QImage &image );
    bool installIOHandler( const QString &format );

    QUnknownInterface *iface();

private:
    QImageFormatPluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
