#ifndef QACTIONFACTORY_H
#define QACTIONFACTORY_H

#include <qlist.h>
#include <qstringlist.h>

class QAction;
class QObject;

class QActionFactory 
{
public:
    static QAction* createAction( const QString& actionname, QObject* parent = 0 );
    static void installActionFactory( QActionFactory* factory );
    static void removeActionFactory( QActionFactory* factory );
    static QList<QActionFactory> factoryList();
    static QStringList actionList();
    static QString actionFactory( const QString& actionname );

private:
    virtual QString factoryName() const = 0;
    virtual QStringList enumerateActions() = 0;
    virtual QAction* newAction( const QString& classname, QObject* parent = 0 ) = 0;
};

class QDefaultActionFactory : public QActionFactory
{
    QString factoryName() const { return "QDefaultActionFactory"; }
    QStringList enumerateActions();
    QAction* newAction( const QString& actionname, QObject* parent = 0 );
};

#endif // QACTIONFACTORY_H