#include "qactionfactory.h"

// some important stuff
extern const unsigned int prime[6] = { 53, 151, 503, 1511, 5101, 15101 };
static int actionPrimeSize = 0;
QDict<QActionFactory> QActionFactory::factories( prime[0] );

/*!
  \class QActionFactory qactionfactory.h
  \brief Factory class for actions.
*/

/*!
  Returns an action with name \a actionname.
  Looks up the action factory that provides the action \a actionname and creates
  the action with \a parent. Return 0 if the action is not provided in any 
  factory.

  \sa installActionFactory()
*/
QAction* QActionFactory::createAction( const QString &actionname, bool& self, QObject *parent )
{
    QActionFactory* fact = factories[actionname];

    if ( fact )
	return fact->newAction( actionname, self, parent );
    return 0;
}

/*!
  Installs a QActionFactory.
  Gets the list of actions \a factory provides. Prints a warning if an action is 
  already supported. In this case createAction() uses the factory added last.

  \sa actionList()
*/
void QActionFactory::installActionFactory( QActionFactory* factory )
{
    QStringList actions = factory->enumerateActions();
    for ( QStringList::Iterator a = actions.begin(); a != actions.end(); a++ ) {
	if ( factories[*a] && factories[*a] != factory )
	    qWarning("More than one factory creating %s", (*a).latin1() );
	factories.insert( *a, factory );
    }
    if ( factories.count() > prime[actionPrimeSize] ) {
	if ( actionPrimeSize <= 6 )
	    factories.resize( prime[++actionPrimeSize] );
    }
}

/*!
  Removes a factory.
  All actions supported by \a factory are no longer available by
  createAction()

  \sa installActionFactory()
*/
void QActionFactory::removeActionFactory( QActionFactory* factory )
{
    QDictIterator<QActionFactory> it( factories );

    while (it.current() ) {
	if ( it.current() == factory )
	    factories.remove( it.currentKey() );
	else
	    ++it;
    }
}

/*!
  Returns a list of installed factories

  \sa installActionFactory()
*/
QList<QActionFactory> QActionFactory::factoryList()
{
    QList<QActionFactory> list;

    QDictIterator<QActionFactory> it( factories );

    while ( it.current() ) {
	if ( !list.contains( it.current() ) )
	    list.append( it.current() );

	++it;
    }

    return list;
}

/*!
  Returns a list of names of all supported actions.

  \sa installActionFactory(), enumerateActions()
*/
QStringList QActionFactory::actionList()
{
    QStringList l;

    QList<QActionFactory> list = factoryList();
    QListIterator<QActionFactory> it( list );

    while ( it.current() ) {
	QStringList actions = it.current()->enumerateActions();
	for ( QStringList::Iterator a = actions.begin(); a != actions.end(); a++ ) {
	    if ( !l.contains( *a ) )
		l.append( *a );
	}
	++it;
    }
    
    return l;
}

/*!
  Returns the factory that provides the action \a actionname.

  \sa installActionFactory()
*/
QString QActionFactory::actionFactory( const QString& actionname )
{
    QActionFactory* f = factories[actionname];
    if ( f )
	return f->factoryName();
    else
	return "";
}

/*!
  \fn QWidget* QActionFactory::newAction( const QString& actionname, QObject* parent )

  Creates and returns an action registered with \a actionname and passes \a parent to 
  the action's constructor if successful. Otherwise returns 0.
  
  You have to reimplement this function in your factories to add support for custom actions.
  Note that newAction() is declared as private, so you musn't call the super class.

  \sa enumerateActions()
*/

/*!
  \fn QStringList QActionFactory::enumerateActions()

  Returns a list of names of actions supported by this factory.
  You have to reimplement this function in your factories to add support for custom actions.
  Note that newAction() is declared as private, so you musn't call the super class.

  \sa newAction()
*/

/*!
  \fn QString QActionFactory::factoryName() const

  Returns the name of the this factory.
  You have to reimplement this function in your factories.
*/