 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define INCLUDE_MENUITEM_DEF
#include <qmenubar.h>
#include <qpopupmenu.h>
#include "designerappiface.h"
#include "mainwindow.h"
#include "project.h"
#include "formwindow.h"
#include "formlist.h"
#include "widgetfactory.h"
#include "command.h"
#include "outputwindow.h"
#include "../shared/widgetdatabase.h"
#include <qvariant.h>
#include <qlistview.h>
#include <qtextedit.h>
#include <qstatusbar.h>
#include "pixmapcollection.h"
#include "hierarchyview.h"

DesignerInterfaceImpl::DesignerInterfaceImpl( MainWindow *mw )
    : ref( 0 ), mainWindow( mw )
{
}

QRESULT DesignerInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface** iface )
{
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QComponent )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_Designer )
	*iface = (QUnknownInterface*)this;

    if ( *iface )
	(*iface)->addRef();
}

unsigned long DesignerInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

DesignerProject *DesignerInterfaceImpl::currentProject() const
{
    return mainWindow->currProject()->iFace();
}

DesignerFormWindow *DesignerInterfaceImpl::currentForm() const
{
    if ( mainWindow->formWindow() )
	return mainWindow->formWindow()->iFace();
    return 0;
}

QPtrList<DesignerProject> DesignerInterfaceImpl::projectList() const
{
    return mainWindow->projectList();
}

void DesignerInterfaceImpl::showStatusMessage( const QString &text, int ms ) const
{
    if ( text.isEmpty() ) {
	mainWindow->statusBar()->clear();
	return;
    }
    if ( ms )
	mainWindow->statusBar()->message( text, ms );
    else
	mainWindow->statusBar()->message( text );
}

DesignerDock *DesignerInterfaceImpl::createDock() const
{
    return 0;
}

DesignerOutputDock *DesignerInterfaceImpl::outputDock() const
{
    return mainWindow->outputWindow() ? mainWindow->outputWindow()->iFace() : 0;
}

void DesignerInterfaceImpl::setModified( bool b, QWidget *window )
{
    mainWindow->setModified( b, window );
}

void DesignerInterfaceImpl::updateFunctionList()
{
    mainWindow->updateFunctionList();
    if ( mainWindow->objectHierarchy()->sourceEditor() )
	mainWindow->objectHierarchy()->updateClassBrowsers();
}

void DesignerInterfaceImpl::onProjectChange( QObject *receiver, const char *slot )
{
    QObject::connect( mainWindow, SIGNAL( projectChanged() ), receiver, slot );
}

void DesignerInterfaceImpl::onFormChange( QObject *receiver, const char *slot )
{
    QObject::connect( mainWindow, SIGNAL( formWindowChanged() ), receiver, slot );
}



DesignerProjectImpl::DesignerProjectImpl( Project *pr )
    : project( pr )
{
}

QPtrList<DesignerFormWindow> DesignerProjectImpl::formList() const
{
    QPtrList<DesignerFormWindow> list;
    QObjectList *forms = project->formList();
    if ( !forms )
	return list;

    QPtrListIterator<QObject> it( *forms );
    while ( it.current() ) {
	QObject *obj = it.current();
	QWidget *par = 0;
	++it;
	if ( !obj->isWidgetType() || !( par = ((QWidget*)obj)->parentWidget() ) || !par->inherits( "FormWindow" ) )
	    continue;

	list.append( ((FormWindow*)par)->iFace() );
    }

    delete forms;
    return list;
}

QStringList DesignerProjectImpl::formNames() const
{
    QStringList lst = project->uiFiles();
    QStringList l;
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	l << project->formName( *it );
    return l;
}

QObjectList *DesignerProjectImpl::run()
{
    return MainWindow::self->runProject();
}

void DesignerProjectImpl::addForm( DesignerFormWindow * )
{
}

void DesignerProjectImpl::removeForm( DesignerFormWindow * )
{
}

QString DesignerProjectImpl::fileName() const
{
    return QString::null;
}

void DesignerProjectImpl::setFileName( const QString & )
{
}

QString DesignerProjectImpl::projectName() const
{
    return project->projectName();
}

void DesignerProjectImpl::setProjectName( const QString & )
{
}

QString DesignerProjectImpl::databaseFile() const
{
    return QString::null;
}

void DesignerProjectImpl::setDatabaseFile( const QString & )
{
}

void DesignerProjectImpl::setupDatabases() const
{
    MainWindow::self->editDatabaseConnections();
}

QPtrList<DesignerDatabase> DesignerProjectImpl::databaseConnections() const
{
    QPtrList<DesignerDatabase> lst;
#ifndef QT_NO_SQL
    QPtrList<DatabaseConnection> conns = project->databaseConnections();
    for ( DatabaseConnection *d = conns.first(); d; d = conns.next() )
	lst.append( d->iFace() );
#endif
    return lst;
}

void DesignerProjectImpl::addDatabase( DesignerDatabase * )
{
}

void DesignerProjectImpl::removeDatabase( DesignerDatabase * )
{
}

void DesignerProjectImpl::save() const
{
}

void DesignerProjectImpl::setLanguage( const QString &l )
{
    project->setLanguage( l );
}

QString DesignerProjectImpl::language() const
{
    return project->language();
}

void DesignerProjectImpl::setCustomSetting( const QString &key, const QString &value )
{
    project->setCustomSetting( key, value );
}

QString DesignerProjectImpl::customSetting( const QString &key ) const
{
    return project->customSetting( key );
}

DesignerPixmapCollection *DesignerProjectImpl::pixmapCollection() const
{
    return project->pixmapCollection()->iFace();
}

void DesignerProjectImpl::breakPoints( QMap<QString, QValueList<int> > &bps ) const
{
    MainWindow::self->saveAllBreakPoints();
    QPtrList<SourceFile> sources = project->sourceFiles();
    for ( SourceFile *sf = sources.first(); sf; sf = sources.next() )
	bps.insert( project->makeRelative( sf->fileName() ) + " <Source-File>", MetaDataBase::breakPoints( sf ) );
    QPtrList<FormWindow> forms = project->forms();
    for ( FormWindow *fw = forms.first(); fw; fw = forms.next() )
	bps.insert( QString( fw->name() ) + " <Form>", MetaDataBase::breakPoints( fw ) );
}

void DesignerProjectImpl::clearAllBreakpoints() const
{
    QPtrList<SourceFile> sources = project->sourceFiles();
    QValueList<int> empty;
    for ( SourceFile *sf = sources.first(); sf; sf = sources.next() )
	MetaDataBase::setBreakPoints( sf, empty );
    QPtrList<FormWindow> forms = project->forms();
    for ( FormWindow *fw = forms.first(); fw; fw = forms.next() )
	MetaDataBase::setBreakPoints( fw, empty );
    MainWindow::self->resetBreakPoints();
}




#ifndef QT_NO_SQL
DesignerDatabaseImpl::DesignerDatabaseImpl( DatabaseConnection *d )
    : db( d )
{
}

QString DesignerDatabaseImpl::name() const
{
    return db->name();
}

void DesignerDatabaseImpl::setName( const QString & )
{
}

QString DesignerDatabaseImpl::driver() const
{
    return db->driver();
}

void DesignerDatabaseImpl::setDriver( const QString & )
{
}

QString DesignerDatabaseImpl::database() const
{
    return db->database();
}

void DesignerDatabaseImpl::setDatabase( const QString & )
{
}

QString DesignerDatabaseImpl::userName() const
{
    return db->username();
}

void DesignerDatabaseImpl::setUserName( const QString & )
{
}

QString DesignerDatabaseImpl::password() const
{
    return db->password();
}

void DesignerDatabaseImpl::setPassword( const QString & )
{
}

QString DesignerDatabaseImpl::hostName() const
{
    return db->hostname();
}

void DesignerDatabaseImpl::setHostName( const QString & )
{
}

QStringList DesignerDatabaseImpl::tables() const
{
    return db->tables();
}

QMap<QString, QStringList> DesignerDatabaseImpl::fields() const
{
    return db->fields();
}

void DesignerDatabaseImpl::open( bool suppressDialog ) const
{
    db->open( suppressDialog );
}

void DesignerDatabaseImpl::close() const
{
    db->close();
}

void DesignerDatabaseImpl::setFields( const QMap<QString, QStringList> & )
{
}

void DesignerDatabaseImpl::setTables( const QStringList & )
{
}

QSqlDatabase* DesignerDatabaseImpl::connection()
{
    return db->connection();
}
#endif



DesignerPixmapCollectionImpl::DesignerPixmapCollectionImpl( PixmapCollection *coll )
    : pixCollection( coll )
{
}

void DesignerPixmapCollectionImpl::addPixmap( const QPixmap &p, const QString &name, bool force )
{
    PixmapCollection::Pixmap pix;
    pix.pix = p;
    pix.name = name;
    pixCollection->addPixmap( pix, force );
    FormWindow *fw = MainWindow::self->formWindow();
    if ( fw )
	MetaDataBase::setPixmapKey( fw, p.serialNumber(), name );
}



DesignerFormWindowImpl::DesignerFormWindowImpl( FormWindow *fw )
    : formWindow( fw )
{
}

QString DesignerFormWindowImpl::name() const
{
    return formWindow->name();
 }

void DesignerFormWindowImpl::setName( const QString &n )
{
    formWindow->setName( n );
}

QString DesignerFormWindowImpl::fileName() const
{
    return formWindow->fileName();
}

void DesignerFormWindowImpl::setFileName( const QString & )
{
}

void DesignerFormWindowImpl::save() const
{
}

bool DesignerFormWindowImpl::isModified() const
{
    return formWindow->commandHistory()->isModified();
}

void DesignerFormWindowImpl::insertWidget( QWidget * )
{
}

QWidget *DesignerFormWindowImpl::create( const char *className, QWidget *parent, const char *name )
{
    QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( className ), parent, name );
    formWindow->insertWidget( w, TRUE );
    formWindow->killAccels( formWindow->mainContainer() );
    return w;
}

void DesignerFormWindowImpl::removeWidget( QWidget * )
{
}

QWidgetList DesignerFormWindowImpl::widgets() const
{
    return QWidgetList();
}

void DesignerFormWindowImpl::undo()
{
}

void DesignerFormWindowImpl::redo()
{
}

void DesignerFormWindowImpl::cut()
{
}

void DesignerFormWindowImpl::copy()
{
}

void DesignerFormWindowImpl::paste()
{
}

void DesignerFormWindowImpl::adjustSize()
{
}

void DesignerFormWindowImpl::editConnections()
{
}

void DesignerFormWindowImpl::checkAccels()
{
}

void DesignerFormWindowImpl::layoutH()
{
    formWindow->layoutHorizontal();
}

void DesignerFormWindowImpl::layoutV()
{
}

void DesignerFormWindowImpl::layoutHSplit()
{
}

void DesignerFormWindowImpl::layoutVSplit()
{
}

void DesignerFormWindowImpl::layoutG()
{
    formWindow->layoutGrid();
}

void DesignerFormWindowImpl::layoutHContainer( QWidget* w )
{
    formWindow->layoutHorizontalContainer( w );
}

void DesignerFormWindowImpl::layoutVContainer( QWidget* w )
{
    formWindow->layoutVerticalContainer( w );
}

void DesignerFormWindowImpl::layoutGContainer( QWidget* w )
{
    formWindow->layoutGridContainer( w );
}

void DesignerFormWindowImpl::breakLayout()
{
}

void DesignerFormWindowImpl::selectWidget( QWidget * w )
{
    formWindow->selectWidget( w, TRUE );
}

void DesignerFormWindowImpl::selectAll()
{
}

void DesignerFormWindowImpl::clearSelection()
{
    formWindow->clearSelection();
}

bool DesignerFormWindowImpl::isWidgetSelected( QWidget * ) const
{
    return FALSE;
}

QWidgetList DesignerFormWindowImpl::selectedWidgets() const
{
    return QWidgetList();
}

QWidget *DesignerFormWindowImpl::currentWidget() const
{
    return 0;
}

QWidget *DesignerFormWindowImpl::form() const
{
    return formWindow;
}

void DesignerFormWindowImpl::setListViewIcon( const QPixmap &pix )
{
    FormList * listView = formWindow->mainWindow()->formlist();
    QListViewItemIterator it( listView );
    while ( it.current() ) {
	FormListItem *item = (FormListItem*)it.current();
	++it;
	QWidget *itemForm = item->formWindow();
	if ( itemForm == formWindow ) {
	    item->setPixmap( 0, pix );
	    break;
	}
    }
}

void DesignerFormWindowImpl::setCurrentWidget( QWidget * )
{
}

QPtrList<QAction> DesignerFormWindowImpl::actionList() const
{
    return QPtrList<QAction>();
}

QAction *DesignerFormWindowImpl::createAction( const QString& text, const QIconSet& icon, const QString& menuText, int accel,
					       QObject* parent, const char* name, bool toggle )
{
    QDesignerAction *a = new QDesignerAction( parent );
    a->setName( name );
    a->setText( text );
    if ( !icon.isNull() && !icon.pixmap().isNull() )
    a->setIconSet( icon );
    a->setMenuText( menuText );
    a->setAccel( accel );
    a->setToggleAction( toggle );
    return a;
}

void DesignerFormWindowImpl::addAction( QAction *a )
{
    if ( formWindow->actionList().findRef( a ) != -1 )
	return;
    formWindow->actionList().append( a );
    MetaDataBase::addEntry( a );
    setPropertyChanged( a, "name", TRUE );
    setPropertyChanged( a, "text", TRUE );
    setPropertyChanged( a, "menuText", TRUE );
    setPropertyChanged( a, "accel", TRUE );
    if ( !a->iconSet().isNull() && !a->iconSet().pixmap().isNull() )
	setPropertyChanged( a, "iconSet", TRUE );
}

void DesignerFormWindowImpl::removeAction( QAction *a )
{
    formWindow->actionList().removeRef( a );
}

void DesignerFormWindowImpl::preview() const
{
}

void DesignerFormWindowImpl::addSlot( const QCString &slot, const QString &access, const QString &language, const QString &returnType )
{
    MetaDataBase::addSlot( formWindow, slot, access, language, returnType );
}

void DesignerFormWindowImpl::addConnection( QObject *sender, const char *signal, QObject *receiver, const char *slot )
{
    MetaDataBase::addConnection( formWindow, sender, signal, receiver, slot );
}

void DesignerFormWindowImpl::setProperty( QObject *o, const char *property, const QVariant &value )
{
    QVariant v = o->property( property );
    if ( v.isValid() )
	o->setProperty( property, value );
    else
	MetaDataBase::setFakeProperty( o, property, value );
}

QVariant DesignerFormWindowImpl::property( QObject *o, const char *prop ) const
{
    QVariant v = o->property( prop );
    if ( v.isValid() )
	return v;
    return MetaDataBase::fakeProperty( o, prop );
}

void DesignerFormWindowImpl::setPropertyChanged( QObject *o, const char *property, bool changed )
{
    MetaDataBase::setPropertyChanged( o, property, changed );
}

bool DesignerFormWindowImpl::isPropertyChanged( QObject *o, const char *property ) const
{
    return MetaDataBase::isPropertyChanged( o, property );
}

void DesignerFormWindowImpl::setColumnFields( QObject *o, const QMap<QString, QString> &f )
{
    MetaDataBase::setColumnFields( o, f );
}

QStringList DesignerFormWindowImpl::implementationIncludes() const
{
    QValueList<MetaDataBase::Include> includes = MetaDataBase::includes( formWindow );
    QStringList lst;
    for ( QValueList<MetaDataBase::Include>::Iterator it = includes.begin(); it != includes.end(); ++it ) {
	MetaDataBase::Include inc = *it;
	if ( inc.implDecl != "in implementation" )
	    continue;
	QString s = inc.header;
	if ( inc.location == "global" ) {
	    s.prepend( "<" );
	    s.append( ">" );
	} else {
	    s.prepend( "\"" );
	    s.append( "\"" );
	}
	lst << s;
    }
    return lst;
}

QStringList DesignerFormWindowImpl::declarationIncludes() const
{
    QValueList<MetaDataBase::Include> includes = MetaDataBase::includes( formWindow );
    QStringList lst;
    for ( QValueList<MetaDataBase::Include>::Iterator it = includes.begin(); it != includes.end(); ++it ) {
	MetaDataBase::Include inc = *it;
	if ( inc.implDecl == "in implementation" )
	    continue;
	QString s = inc.header;
	if ( inc.location == "global" ) {
	    s.prepend( "<" );
	    s.append( ">" );
	} else {
	    s.prepend( "\"" );
	    s.append( "\"" );
	}
	lst << s;
    }
    return lst;
}

void DesignerFormWindowImpl::setImplementationIncludes( const QStringList &lst )
{
    QValueList<MetaDataBase::Include> oldIncludes = MetaDataBase::includes( formWindow );;
    QValueList<MetaDataBase::Include> includes;
    for ( QValueList<MetaDataBase::Include>::Iterator it = oldIncludes.begin(); it != oldIncludes.end(); ++it ) {
	MetaDataBase::Include inc = *it;
	if ( inc.implDecl == "in implementation" )
	    continue;
	includes << inc;
    }

    for ( QStringList::ConstIterator sit = lst.begin(); sit != lst.end(); ++sit ) {
	QString s = *sit;
	if ( s[ 0 ] != '<' && s[ 0 ] != '"' ) {
	    s.prepend( "\"" );
	    s.append( "\"" );
	}
	if ( s[ 0 ] == '<' ) {
	    s.remove( 0, 1 );
	    s.remove( s.length() - 1, 1 );
	    MetaDataBase::Include inc;
	    inc.header = s;
	    inc.implDecl = "in implementation";
	    inc.location = "global";
	    includes << inc;
	} else {
	    s.remove( 0, 1 );
	    s.remove( s.length() - 1, 1 );
	    MetaDataBase::Include inc;
	    inc.header = s;
	    inc.implDecl = "in implementation";
	    inc.location = "local";
	    includes << inc;
	}
    }
    MetaDataBase::setIncludes( formWindow, includes );
    formWindow->mainWindow()->objectHierarchy()->functionList()->setup();
}

void DesignerFormWindowImpl::setDeclarationIncludes( const QStringList &lst )
{
    QValueList<MetaDataBase::Include> oldIncludes = MetaDataBase::includes( formWindow );;
    QValueList<MetaDataBase::Include> includes;
    for ( QValueList<MetaDataBase::Include>::Iterator it = oldIncludes.begin(); it != oldIncludes.end(); ++it ) {
	MetaDataBase::Include inc = *it;
	if ( inc.implDecl == "in declaration" )
	    continue;
	includes << inc;
    }

    for ( QStringList::ConstIterator sit = lst.begin(); sit != lst.end(); ++sit ) {
	QString s = *sit;
	if ( s[ 0 ] != '<' && s[ 0 ] != '"' ) {
	    s.prepend( "\"" );
	    s.append( "\"" );
	}
	if ( s[ 0 ] == '<' ) {
	    s.remove( 0, 1 );
	    s.remove( s.length() - 1, 1 );
	    MetaDataBase::Include inc;
	    inc.header = s;
	    inc.implDecl = "in declaration";
	    inc.location = "global";
	    includes << inc;
	} else {
	    s.remove( 0, 1 );
	    s.remove( s.length() - 1, 1 );
	    MetaDataBase::Include inc;
	    inc.header = s;
	    inc.implDecl = "in declaration";
	    inc.location = "local";
	    includes << inc;
	}
    }
    MetaDataBase::setIncludes( formWindow, includes );
    formWindow->mainWindow()->objectHierarchy()->functionList()->setup();
}

QStringList DesignerFormWindowImpl::forwardDeclarations() const
{
    return MetaDataBase::forwards( formWindow );
}

void DesignerFormWindowImpl::setForwardDeclarations( const QStringList &lst )
{
    MetaDataBase::setForwards( formWindow, lst );
    formWindow->mainWindow()->objectHierarchy()->functionList()->setup();
}

QStringList DesignerFormWindowImpl::variables() const
{
    return MetaDataBase::variables( formWindow );
}

void DesignerFormWindowImpl::setVariables( const QStringList &lst )
{
    MetaDataBase::setVariables( formWindow, lst );
    formWindow->mainWindow()->objectHierarchy()->functionList()->setup();
}

void DesignerFormWindowImpl::onModificationChange( QObject *receiver, const char *slot )
{
    QObject::connect( formWindow, SIGNAL( modificationChanged( bool, FormWindow * ) ), receiver, slot );
}

void DesignerFormWindowImpl::addMenu( const QString &text, const QString &name )
{
    if ( !formWindow->mainContainer()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)formWindow->mainContainer();
    QDesignerPopupMenu *popup = new QDesignerPopupMenu( mw );
    QString n = name;
    formWindow->unify( popup, n, TRUE );
    popup->setName( n );
    if ( !mw->child( 0, "QMenuBar" ) ) {
	QMenuBar *mb = new QDesignerMenuBar( (QWidget*)mw );
	mb->setName( "menubar" );
    }
    mw->menuBar()->insertItem( text, popup );
}

void DesignerFormWindowImpl::addMenuAction( const QString &menu, QAction *a )
{
    if ( !formWindow->mainContainer()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)formWindow->mainContainer();
    if ( !mw->child( 0, "QMenuBar" ) )
	return;
    QDesignerPopupMenu *popup = (QDesignerPopupMenu*)mw->child( menu, "QDesignerPopupMenu" );
    if ( !popup )
	return;
    a->addTo( popup );
    popup->addAction( a );
}

void DesignerFormWindowImpl::addMenuSeparator( const QString &menu )
{
    if ( !formWindow->mainContainer()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)formWindow->mainContainer();
    if ( !mw->child( 0, "QMenuBar" ) )
	return;
    QDesignerPopupMenu *popup = (QDesignerPopupMenu*)mw->child( menu, "QDesignerPopupMenu" );
    if ( !popup )
	return;
    QAction *a = new QSeparatorAction( 0 );
    a->addTo( popup );
    popup->addAction( a );
}

void DesignerFormWindowImpl::addToolBar( const QString &text, const QString &name )
{
    if ( !formWindow->mainContainer()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)formWindow->mainContainer();
    QToolBar *tb = new QDesignerToolBar( mw );
    QString n = name;
    formWindow->unify( tb, n, TRUE );
    tb->setName( n );
    mw->addToolBar( tb, text );
}

void DesignerFormWindowImpl::addToolBarAction( const QString &tbn, QAction *a )
{
    if ( !formWindow->mainContainer()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)formWindow->mainContainer();
    QDesignerToolBar *tb = (QDesignerToolBar*)mw->child( tbn, "QDesignerToolBar" );
    if ( !tb )
	return;
    a->addTo( tb );
    tb->addAction( a );
}

void DesignerFormWindowImpl::addToolBarSeparator( const QString &tbn )
{
    if ( !formWindow->mainContainer()->inherits( "QMainWindow" ) )
	return;
    QMainWindow *mw = (QMainWindow*)formWindow->mainContainer();
    QDesignerToolBar *tb = (QDesignerToolBar*)mw->child( tbn, "QDesignerToolBar" );
    if ( !tb )
	return;
    QAction *a = new QSeparatorAction( 0 );
    a->addTo( tb );
    tb->addAction( a );
}



DesignerDockImpl::DesignerDockImpl()
{
}

QDockWindow *DesignerDockImpl::dockWindow() const
{
    return 0;
}






DesignerOutputDockImpl::DesignerOutputDockImpl( OutputWindow *ow )
    : outWin( ow )
{
}

QWidget *DesignerOutputDockImpl::addView( const QString &title )
{
    QWidget *page = new QWidget( outWin );
    outWin->addTab( page, title );
    return page;
}

void DesignerOutputDockImpl::appendDebug( const QString &s )
{
    outWin->appendDebug( s );
}

void DesignerOutputDockImpl::clearDebug()
{
}

void DesignerOutputDockImpl::appendError( const QString &s, int l )
{
    QStringList ls;
    ls << s;
    QValueList<int> ll;
    ll << l;
    outWin->setErrorMessages( ls, ll, FALSE );
}

void DesignerOutputDockImpl::clearError()
{
}
