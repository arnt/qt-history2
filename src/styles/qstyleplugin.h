#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qobject.h"
#include "qstringlist.h"
#include "qplugin.h"
#endif // QT_H

class QStyle;
class QStylePluginPrivate;
struct QUnknownInterface;

class Q_EXPORT QStylePlugin : public QObject
{
public:
    QStylePlugin();
    virtual ~QStylePlugin();

    virtual QStringList featureList() const;
    virtual QStyle *create( const QString &key );

    QUnknownInterface *iface();

private:
    QStylePluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
