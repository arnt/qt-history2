#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#include <qdict.h>
#include <qlist.h>
#include <qstringlist.h>

class QWidget;

class QWidgetFactory
{
public:
    virtual ~QWidgetFactory() { QWidgetFactory::removeWidgetFactory( this ); }

    static QWidget* create( const QString& description, QWidget* parent = 0, const char* name = 0 );
    virtual QString factoryName() const { return "QWidgetFactory"; }

    static void installWidgetFactory( QWidgetFactory* factory );
    static void removeWidgetFactory( QWidgetFactory* factory );

    static QList<QWidgetFactory> factoryList();
    static QStringList widgetList();
    static QWidgetFactory *widgetFactory( const QString& description );

private:
    virtual QWidget* compose( const QString& description );
    virtual QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    virtual QStringList widgets();

    static QDict<QWidgetFactory> factory;
    static QList<QWidgetFactory> factories;
    static QWidgetFactory* that;
};

#endif // QWIDGETFACTORY_H
