#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

class QStyle;
class QStylePluginPrivate;

class Q_EXPORT QStylePlugin : public QGPlugin
{
public:
    QStylePlugin();
    ~QStylePlugin();

    virtual QStringList keys() const = 0;
    virtual QStyle *create( const QString &key ) = 0;

private:
    QStylePluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
