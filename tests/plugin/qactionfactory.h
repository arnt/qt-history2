#ifndef QACTIONFACTORY_H
#define QACTIONFACTORY_H

#include <qlist.h>
#include <qstringlist.h>
#include <qdict.h>

class QAction;
class QObject;

class QActionFactory 
{
public:
    virtual ~QActionFactory() { QActionFactory::removeActionFactory( this ); }

    static QAction* create( const QString& actionname, bool& self, QObject* parent = 0 );
    virtual QString factoryName() const = 0;

    static void installActionFactory( QActionFactory* factory );
    static void removeActionFactory( QActionFactory* factory );

    static QList<QActionFactory> factoryList();
    static QStringList actionList();
    static QActionFactory *actionFactory( const QString& actionname );

private:
    virtual QAction *compose( const QString& description );
    virtual QAction *newAction( const QString& classname, bool& self, QObject* parent = 0 ) = 0;
    virtual QStringList actions() = 0;

    static QList<QActionFactory> factories;
    static QDict<QActionFactory> factory;
    static QActionFactory *that;
};

#endif // QACTIONFACTORY_H