#include "qactionfactory.h"
#include <qaction.h>

// some important stuff
extern const unsigned int prime[6] = { 53, 151, 503, 1511, 5101, 15101 };
static int actionPrimeSize = 0;
QDict<QActionFactory> QActionFactory::factory( prime[0] );
QList<QActionFactory> QActionFactory::factories = QList<QActionFactory>();
QActionFactory* QActionFactory::that = 0;

/*!
  \class QActionFactory qactionfactory.h
  \brief Factory class for actions.
*/

/*!
  Installs a QActionFactory.
  Gets the list of actions \a f provides. Prints a warning if an action is 
  already supported. In this case createAction() uses the factory added last.

  \sa actionList()
*/
void QActionFactory::installActionFactory( QActionFactory* f )
{
    if ( !that )
	that = f;

    if ( !factories.contains( f ) )
	factories.append( f );

    QStringList actions = f->actions();
    for ( QStringList::Iterator a = actions.begin(); a != actions.end(); a++ ) {
#ifdef CHECK_RANGE
	if ( factory[*a] && factory[*a] != f )
	    qWarning("More than one factory creating %s", (*a).latin1() );
#endif
	factory.insert( *a, f );
    }
    if ( factory.count() > prime[actionPrimeSize] ) {
	if ( actionPrimeSize <= 6 )
	    factory.resize( prime[++actionPrimeSize] );
    }
}

/*!
  Removes a factory.
  All actions supported by \a factory are no longer available.

  \sa installActionFactory(), create()
*/
void QActionFactory::removeActionFactory( QActionFactory* f )
{
    factories.remove( f );
    QDictIterator<QActionFactory> it( factory );

    while (it.current() ) {
	if ( it.current() == f )
	    factory.remove( it.currentKey() );
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
    return factories;
}

/*!
  Returns a list of names of all supported actions.

  \sa installActionFactory(), actions()
*/
QStringList QActionFactory::actionList()
{
    QStringList l;

    QList<QActionFactory> list = factories;
    QListIterator<QActionFactory> it( list );

    while ( it.current() ) {
	QStringList actions = it.current()->actions();
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
QActionFactory* QActionFactory::actionFactory( const QString& actionname )
{
    return factory[actionname];
}

/*!
  Returns an action with name \a actionname.
  Looks up the action factory that provides the action \a actionname and creates
  the action with \a parent. Return 0 if the action is not provided in any 
  factory.

  \sa installActionFactory()
*/
QAction* QActionFactory::create( const QString &description, QObject *parent )
{
    if ( description.isEmpty() )
	return 0;
    if ( description[0] == '<' ) {
	QAction* a = 0;
/*	if ( !that )
	    installWidgetFactory( new QActionFactory );*/
	a = that->compose( description );
	
	if ( a ) {
	    if ( parent )
		parent->insertChild( a );
	    return a;
	} 
    }

    QActionFactory* fact = factory[description];

    if ( !fact ) {
	QListIterator<QActionFactory> it( factories );
	while ( it.current() ) {
	    if ( it.current()->actions().contains( description ) ) {
		factory.insert( description, it.current() );
		fact = it.current();
		break;
	    }
	    ++it;
	}
    }
    if ( fact )
	return fact->newAction( description, parent );
    return 0;
}

/*!
  Processes \a description and returns a corresponding action if successful.
  Otherwise returns NULL.
*/
QAction* QActionFactory::compose( const QString& description )
{
    qDebug("Imagine I process %s", description.latin1() );

    // TODO: process document

    return 0;
}

/*!
  \fn QWidget* QActionFactory::newAction( const QString& actionname, QObject* parent )

  Creates and returns an action registered with \a actionname and passes \a parent to 
  the action's constructor if successful. Otherwise returns 0.
  
  You have to reimplement this function in your factories to add support for custom actions.
  Note that newAction() is declared as private, so you musn't call the super class.

  \sa actions()
*/

/*!
  \fn QStringList QActionFactory::actions()

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
