#include "qactionfactory.h"
#include <qdict.h>

extern const unsigned int prime[6] = { 53, 151, 503, 1511, 5101, 15101 };
int actionPrimeSize = 0;
QDict<QActionFactory> actionFactories( prime[0] );

QAction* QActionFactory::createAction( const QString &actionname, QObject *parent )
{
    QActionFactory* fact = actionFactories[actionname];

    if ( fact )
	return fact->newAction( actionname, parent );
    return 0;
}

void QActionFactory::installActionFactory( QActionFactory* factory )
{
    QStringList actions = factory->enumerateActions();
    for ( uint a = 0; a < actions.count(); a++ ) {
	if ( actionFactories[actions[a]] && actionFactories[actions[a]] != factory )
	    qWarning("More than one factory creating %s", actions[a].latin1() );
	actionFactories.insert( actions[a], factory );
    }
    if ( actionFactories.count() > prime[actionPrimeSize] ) {
	if ( ++actionPrimeSize < 6 )
	    actionFactories.resize( prime[++actionPrimeSize] );
    }
}

void QActionFactory::removeActionFactory( QActionFactory* factory )
{
    QDictIterator<QActionFactory> it( actionFactories );

    while (it.current() ) {
	if ( it.current() == factory )
	    actionFactories.remove( it.currentKey() );
	else
	    ++it;
    }
}

QList<QActionFactory> QActionFactory::factoryList()
{
    QList<QActionFactory> list;

    QDictIterator<QActionFactory> it( actionFactories );

    while ( it.current() ) {
	if ( !list.contains( it.current() ) )
	    list.append( it.current() );

	++it;
    }

    return list;
}

QStringList QActionFactory::actionList()
{
    QStringList list;
    
    QDictIterator<QActionFactory> it( actionFactories );

    while ( it.current() ) {
	QStringList actions = it.current()->enumerateActions();
	for ( uint a = 0; a < actions.count(); a++ ) {
	    if ( !list.contains( actions[a] ) )
		list.append( actions[a] );
	}

	++it;
    }
    
    return list;
}

QString QActionFactory::actionFactory( const QString& actionname )
{
    QActionFactory* f = actionFactories[actionname];
    if ( f )
	return f->factoryName();
    else
	return "";
}

QAction* QDefaultActionFactory::newAction( const QString &actionname, QObject *parent )
{
    return 0;
}

QStringList QDefaultActionFactory::enumerateActions()
{
    return QStringList();
}