#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qobject.h"
#include "qstringlist.h"
#include "qplugin.h"
#endif // QT_H

class QTextCodec;
class QTextCodecPluginPrivate;
struct QUnknownInterface;

class Q_EXPORT QTextCodecPlugin : public QObject
{
public:
    QTextCodecPlugin();
    virtual ~QTextCodecPlugin();

    virtual QStringList featureList() const;
    virtual QTextCodec *createForMib( int mib );
    virtual QTextCodec *createForName( const QString &name );

    QUnknownInterface *iface();

private:
    QTextCodecPluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
