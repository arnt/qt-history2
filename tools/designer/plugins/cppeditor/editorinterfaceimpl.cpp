#include "editorinterfaceimpl.h"
#include <viewmanager.h>
#include "cppeditor.h"
#include "qrichtext_p.h"
#include "qapplication.h"
#include "completion.h"

EditorInterfaceImpl::EditorInterfaceImpl()
    : EditorInterface(), viewManager( 0 ), ref( 0 )
{
}

EditorInterfaceImpl::~EditorInterfaceImpl()
{
    delete viewManager;
}

QUnknownInterface *EditorInterfaceImpl::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_EditorInterface )
	iface = (EditorInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long EditorInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long EditorInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QWidget *EditorInterfaceImpl::editor( QWidget *parent ) const
{
    if ( !viewManager ) {
	( (EditorInterfaceImpl*)this )->viewManager = new ViewManager( parent, 0 );
	(void)new CppEditor( QString::null, viewManager, "editor" );
    }
    return viewManager->currentView();
}

void EditorInterfaceImpl::setText( const QString &txt )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->setText( txt );
}

QString EditorInterfaceImpl::text() const
{
    if ( !viewManager || !viewManager->currentView() )
	return QString::null;
    return ( (CppEditor*)viewManager->currentView() )->text();
}

void EditorInterfaceImpl::undo()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->undo();
}

void EditorInterfaceImpl::redo()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->redo();
}

void EditorInterfaceImpl::cut()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->cut();
}

void EditorInterfaceImpl::copy()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->copy();
}

void EditorInterfaceImpl::paste()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->paste();
}

void EditorInterfaceImpl::selectAll()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->selectAll();
}

bool EditorInterfaceImpl::find( const QString &expr, bool cs, bool wo, bool forward )
{
    if ( !viewManager || !viewManager->currentView() )
	return FALSE;
    return ( (CppEditor*)viewManager->currentView() )->find( expr, cs, wo, forward );
}

void EditorInterfaceImpl::indent()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->indent();
}

void EditorInterfaceImpl::splitView()
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    QTextDocument *doc = ( (CppEditor*)viewManager->currentView() )->document();
    CppEditor *editor = new CppEditor( QString::null, viewManager, "editor" );
    editor->setDocument( doc );
}

void EditorInterfaceImpl::scrollTo( const QString &txt )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    qApp->processEvents();
    QTextDocument *doc = ( (CppEditor*)viewManager->currentView() )->document();
    QTextParag *p = doc->firstParag();
    while ( p ) {
	if ( p->string()->toString().find( txt ) != -1 ) {
	    ( (CppEditor*)viewManager->currentView() )->setCursorPosition( p->paragId() + 2, 0 );
	    break;
	}
	p = p->next();
    }
    ( (CppEditor*)viewManager->currentView() )->setFocus();
}

QStringList EditorInterfaceImpl::featureList() const
{
    QStringList list;
    list << "C++";
    return list;
}

void EditorInterfaceImpl::setContext( QObjectList *toplevels, QObject *this_ )
{
    if ( !viewManager || !viewManager->currentView() )
	return;
    ( (CppEditor*)viewManager->currentView() )->completionManager()->setContext( toplevels, this_ );
}

void EditorInterfaceImpl::setError( int line )
{
    if ( !viewManager )
	return;
    viewManager->setError( line );
}

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

void EditorInterfaceImpl::functions( QMap<QString, QString>* functionMap ) const
{
    if ( !functionMap || !viewManager || !viewManager->currentView() )
	return;

    QString text = ( (CppEditor*)viewManager->currentView() )->text();

    QString func;
    QString body;

    int i = 0;
    int j = 0;
    int k = 0;
    while ( i != -1 ) {
	i = text.find( "::", i );
	if ( i == -1 )
	    break;
	for ( j = i + QString( "::").length(); j < (int)text.length(); ++j ) {
	    if ( text[ j ] != ' ' && text[ j ] != '\t' )
		break;
	}
	if ( j == (int)text.length() - 1 )
	    break;
	k = text.find( ")", j );
	func = text.mid( j, k - j + 1 );
	func = func.stripWhiteSpace();
	func = func.simplifyWhiteSpace();
	func = NormalizeObject::normalizeSignalSlot( func.latin1() );
	
	i = k;
	i = text.find( "{", i );
	if ( i == -1 )
	    break;
	int open = 0;
	for ( j = i; j < (int)text.length(); ++j ) {
	    if ( text[ j ] == '{' )
		open++;
	    else if ( text[ j ] == '}' )
		open--;
	    if ( !open )
		break;
	}
	body = text.mid( i, j - i + 1 );

	functionMap->insert( func, body );
    }
}

QString EditorInterfaceImpl::createFunctionStart( const QString &className, const QString &func )
{
    return "void " + className + "::" + func;
}

Q_EXPORT_INTERFACE( EditorInterfaceImpl )
