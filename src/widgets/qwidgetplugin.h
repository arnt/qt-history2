#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qstringlist.h"
#include "qplugin.h"
#include "qiconset.h"
#endif // QT_H

class QWidgetPluginPrivate;
class QWidget;
struct QUnknownInterface;

class Q_EXPORT QWidgetPlugin : public QObject
{
public:
    QWidgetPlugin();
    virtual ~QWidgetPlugin();

    virtual QStringList keys() const;

    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 );
    virtual QString group( const QString &key ) const;
    virtual QIconSet iconSet( const QString &key ) const;
    virtual QString includeFile( const QString &key ) const;
    virtual QString toolTip( const QString &key ) const;
    virtual QString whatsThis( const QString &key ) const;
    virtual bool isContainer( const QString &key ) const;

    QUnknownInterface *iface();

private:
    QWidgetPluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
