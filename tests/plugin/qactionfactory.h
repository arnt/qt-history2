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

    static void installActionFactory( QActionFactory* factory );
    static void removeActionFactory( QActionFactory* factory );

//### only for testing?
    static QList<QActionFactory> factoryList();
    static QStringList actionList();
    static QString actionFactory( const QString& actionname );

private:
    virtual QString factoryName() const = 0;

    virtual QAction* newAction( const QString& classname, bool& self, QObject* parent = 0 ) = 0;
    virtual QStringList actions() = 0;

    static QDict<QActionFactory> factories;
};

#endif // QACTIONFACTORY_H