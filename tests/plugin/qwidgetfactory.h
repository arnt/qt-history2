#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#include <qlist.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qnamespace.h>

class QWidget;
class QIODevice;

class QWidgetFactory
{
public:
    static QWidget* createWidget( const QString& classname, bool init, QWidget* parent = 0, const char* name = 0 );
    static QWidget* createWidget( const QString& filename, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );

    static void installWidgetFactory( QWidgetFactory* factory );
    static void removeWidgetFactory( QWidgetFactory* factory );

//### only for testing?
    static QList<QWidgetFactory> factoryList();
    static QStringList widgetList();
    static QStringList fileTypeList();
    static QString widgetFactory( const QString& classname );

private:
    virtual QString factoryName() const { return "QWidgetFactory"; }

    virtual QWidget* newWidget( const QString& classname, bool init, QWidget* parent = 0, const char* name = 0 );
    virtual QStringList enumerateWidgets();

    virtual QWidget* processFile( QIODevice *f, const QString& filetype );
    virtual QStringList enumerateFileTypes();

    static QDict<QWidgetFactory> factories;
};

#endif // QWIDGETFACTORY_H