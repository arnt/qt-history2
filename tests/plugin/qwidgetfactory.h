#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#include <qlist.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qnamespace.h>

class QWidget;

class QWidgetFactory
{
public:
    virtual ~QWidgetFactory() { QWidgetFactory::removeWidgetFactory( this ); }

    static QWidget* create( const QString& description, QWidget* parent = 0, const char* name = 0 );

    static void installWidgetFactory( QWidgetFactory* factory );
    static void removeWidgetFactory( QWidgetFactory* factory );

//### only for testing?
    static QList<QWidgetFactory> factoryList();
    static QStringList widgetList();
    static QStringList fileTypeList();
    static QString widgetFactory( const QString& classname );
//###

protected:
    QWidget* compose( const QString& description );

private:
    virtual QString factoryName() const { return "QWidgetFactory"; }

    virtual QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0 );
    virtual QStringList widgets();

    static QDict<QWidgetFactory> factories;
    static QWidgetFactory* that;
};

#endif // QWIDGETFACTORY_H