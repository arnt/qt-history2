#ifndef QGFXDRIVERPLUGIN_H
#define QGFXDRIVERPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

class QScreen;
class QGfxDriverPluginPrivate;

class Q_EXPORT QGfxDriverPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QGfxDriverPlugin();
    ~QGfxDriverPlugin();

#ifndef QT_NO_STRINGLIST
    virtual QStringList keys() const = 0;
#endif
    virtual QScreen* create( const QString& driver, int displayId ) = 0;

private:
    QGfxDriverPluginPrivate *d;
};

#endif // QGFXDRIVERPLUGIN_H
