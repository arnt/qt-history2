#ifndef QSTYLEPLUGIN_H
#define QSTYLEPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#include "qiconset.h"
#endif // QT_H

class QWidgetPluginPrivate;
class QWidget;

class Q_EXPORT QWidgetPlugin : public QGPlugin
{
public:
    QWidgetPlugin();
    ~QWidgetPlugin();

    virtual QStringList keys() const = 0;
    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 ) = 0;
    
    virtual QString group( const QString &key ) const;
    virtual QIconSet iconSet( const QString &key ) const;
    virtual QString includeFile( const QString &key ) const;
    virtual QString toolTip( const QString &key ) const;
    virtual QString whatsThis( const QString &key ) const;
    virtual bool isContainer( const QString &key ) const;

private:
    QWidgetPluginPrivate *d;
};

#endif // QSTYLEPLUGIN_H
