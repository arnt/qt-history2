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

#include "designerappiface.h"
#include "mainwindow.h"
#include "project.h"
#include "formwindow.h"
#include "widgetfactory.h"
#include "outputwindow.h"
#include "../shared/widgetdatabase.h"
#include <qvariant.h>
#include <qlistview.h>
#include <qtextedit.h>

DesignerInterfaceImpl::DesignerInterfaceImpl( MainWindow *mw )
    : ref( 0 ), mainWindow( mw )
{
}

QUnknownInterface *DesignerInterfaceImpl::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QComponentInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_DesignerInterface )
	iface = (QUnknownInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
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

QList<DesignerProject> DesignerInterfaceImpl::projectList() const
{
    return QList<DesignerProject>();
}

void DesignerInterfaceImpl::showStatusMessage( const QString & ) const
{
}

DesignerDock *DesignerInterfaceImpl::createDock() const
{
    return 0;
}

DesignerOutputDock *DesignerInterfaceImpl::outputDock() const
{
    return 0;
}






DesignerProjectImpl::DesignerProjectImpl( Project *pr )
    : project( pr )
{
}

DesignerFormWindow *DesignerProjectImpl::currentForm() const
{
    return 0;
}

QList<DesignerFormWindow> DesignerProjectImpl::formList() const
{
    return QList<DesignerFormWindow>();
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
    return QString::null;
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

QList<DesignerDatabase> DesignerProjectImpl::databaseConnections() const
{
    QList<DesignerDatabase> lst;
    QList<Project::DatabaseConnection> conns = project->databaseConnections();
    for ( Project::DatabaseConnection *d = conns.first(); d; d = conns.next() )
	lst.append( d->iFace() );
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






DesignerDatabaseImpl::DesignerDatabaseImpl( Project::DatabaseConnection *d )
    : db( d )
{
}

QString DesignerDatabaseImpl::name() const
{
    return db->name;
}

void DesignerDatabaseImpl::setName( const QString & )
{
}

QString DesignerDatabaseImpl::driver() const
{
    return db->driver;
}

void DesignerDatabaseImpl::setDriver( const QString & )
{
}

QString DesignerDatabaseImpl::database() const
{
    return db->dbName;
}

void DesignerDatabaseImpl::setDatabase( const QString & )
{
}

QString DesignerDatabaseImpl::userName() const
{
    return db->username;
}

void DesignerDatabaseImpl::setUserName( const QString & )
{
}

QString DesignerDatabaseImpl::password() const
{
    return db->password;
}

void DesignerDatabaseImpl::setPassword( const QString & )
{
}

QString DesignerDatabaseImpl::hostName() const
{
    return db->hostname;
}

void DesignerDatabaseImpl::setHostName( const QString & )
{
}

QStringList DesignerDatabaseImpl::tables() const
{
    return db->tables;
}

QMap<QString, QStringList> DesignerDatabaseImpl::fields() const
{
    return db->fields;
}

void DesignerDatabaseImpl::open() const
{
    db->open();
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




DesignerFormWindowImpl::DesignerFormWindowImpl( FormWindow *fw )
    : formWindow( fw )
{
}

QString DesignerFormWindowImpl::fileName() const
{
    return QString::null;
}

void DesignerFormWindowImpl::setFileName( const QString & )
{
}

void DesignerFormWindowImpl::save() const
{
}

void DesignerFormWindowImpl::insertWidget( QWidget * )
{
}

QWidget *DesignerFormWindowImpl::create( const char *className, QWidget *parent, const char *name )
{
    QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( className ), parent, name );
    formWindow->insertWidget( w, TRUE );
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

void DesignerFormWindowImpl::setCurrentWidget( QWidget * )
{
}

QList<QAction> DesignerFormWindowImpl::actionList() const
{
    return QList<QAction>();
}

void DesignerFormWindowImpl::addAction( QAction * )
{
}

void DesignerFormWindowImpl::removeAction( QAction * )
{
}

void DesignerFormWindowImpl::preview() const
{
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
}

QStringList DesignerFormWindowImpl::forwardDeclarations() const
{
    return MetaDataBase::forwards( formWindow );
}

void DesignerFormWindowImpl::setForwardDeclarations( const QStringList &lst )
{
    MetaDataBase::setForwards( formWindow, lst );
}

QStringList DesignerFormWindowImpl::variables() const
{
    return MetaDataBase::variables( formWindow );
}

void DesignerFormWindowImpl::setVariables( const QStringList &lst )
{
    MetaDataBase::setVariables( formWindow, lst );
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

QWidget *DesignerOutputDockImpl::addView( const QString & )
{
    return 0;
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
