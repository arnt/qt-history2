#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

class QTextCodec;
class QTextCodecPluginPrivate;

class Q_EXPORT QTextCodecPlugin : public QGPlugin
{
public:
    QTextCodecPlugin();
    ~QTextCodecPlugin();

    virtual QStringList keys() const = 0;
    virtual QTextCodec *createForMib( int mib ) = 0;
    virtual QTextCodec *createForName( const QString &name ) = 0;

private:
    QTextCodecPluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
