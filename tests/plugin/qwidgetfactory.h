#ifndef QWIDGETFACTORY_H
#define QWIDGETFACTORY_H

#include <qlist.h>
#include <qstringlist.h>
#include <qnamespace.h>

class QWidget;
class QFile;

class QWidgetFactory
{
public:
    static QWidget* createWidget( const QString& classname, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );
    static QWidget* createWidget( const QString& filename, bool &ok, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );

    static void installWidgetFactory( QWidgetFactory* factory );
    static void removeWidgetFactory( QWidgetFactory* factory );
    static QList<QWidgetFactory> factoryList();
    static QStringList widgetList();
    static QStringList fileTypeList();
    static QString widgetFactory( const QString& classname );

private:
    virtual QString factoryName() const = 0;

    virtual QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 ) = 0;
    virtual QStringList enumerateWidgets() = 0;

    virtual QWidget* processFile( QFile *f, bool &ok );
    virtual QStringList enumerateFileTypes();
};

class QDefaultWidgetFactory : public QWidgetFactory
{
    QString factoryName() const { return "QDefaultWidgetFactory"; }
    QStringList enumerateWidgets();
    QWidget* newWidget( const QString& classname, QWidget* parent = 0, const char* name = 0, Qt::WFlags f  = 0 );
};

#endif // QWIDGETFACTORY_H