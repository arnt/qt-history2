/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "metadatabase.h"
#include "widgetfactory.h"
#include "formwindow.h"
#include "parser.h"
#include "widgetdatabase.h"
#include "formfile.h"
#include "propertyobject.h"
#include "project.h"
#include "mainwindow.h"

#include <qapplication.h>
#include <qobject.h>
#include <qlayout.h>
#include <qhash.h>
#include <qmetaobject.h>
#include <qmainwindow.h>
#include <qregexp.h>
#include <private/qpluginmanager_p.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <stdlib.h>

class MetaDataBaseRecord
{
public:
    QObject *object;
    QStringList changedProperties;
    QMap<QString,QVariant> fakeProperties;
    QMap<QString, QString> propertyComments;
    int spacing, margin;
    QString resizeMode;
    QList<MetaDataBase::Connection> connections;
    QList<MetaDataBase::Function> functionList;
    QList<MetaDataBase::Include> includes;
    QList<MetaDataBase::Variable> variables;
    QStringList forwards, sigs;
    QWidgetList tabOrder;
    MetaDataBase::MetaInfo metaInfo;
    QCursor cursor;
    QMap<int, QString> pixmapArguments;
    QMap<int, QString> pixmapKeys;
    QMap<QString, QString> columnFields;
    QList<uint> breakPoints;
    QMap<int, QString> breakPointConditions;
    QString exportMacro;
};

static QHash<QObject *, MetaDataBaseRecord*> *db = 0;
static QList<MetaDataBase::CustomWidget*> *cWidgets = 0;
static bool doUpdate = TRUE;
static QStringList langList;
static QStringList editorLangList;
static QPluginManager<LanguageInterface> *languageInterfaceManager = 0;

/*!
  \class MetaDataBase metadatabase.h
  \brief Database which stores meta data of widgets

  The MetaDataBase stores meta information of widgets, which are not
  stored directly in widgets (properties). This is e.g. the
  information which properties have been modified.
*/

MetaDataBase::MetaDataBase()
{
}

inline void setupDataBase()
{
    if ( !db ) {
	db = new QHash<QObject*, MetaDataBaseRecord*>;
	cWidgets = new QList<MetaDataBase::CustomWidget*>;
    }
}

void MetaDataBase::clearDataBase()
{
    if ( db ) {
	QHash<QObject *, MetaDataBaseRecord *>::ConstIterator it = db->constBegin();
        while (it != db->constEnd()) {
	    delete it.value();
	    ++it;
	}
        delete db;
        db = 0;

	while (!cWidgets->isEmpty())
	    delete cWidgets->takeFirst();
	delete cWidgets;
        cWidgets = 0;
    }
}

void MetaDataBase::addEntry( QObject *o )
{
    if ( !o )
	return;
    setupDataBase();
    if ( db->value( o ) )
	return;
    MetaDataBaseRecord *r = new MetaDataBaseRecord;
    r->object = o;
    r->spacing = r->margin = -1;
    db->insert( o, r );

    WidgetFactory::initChangedProperties( o );
}

void MetaDataBase::removeEntry( QObject *o )
{
    setupDataBase();
    delete db->take( o );
}

void MetaDataBase::setPropertyChanged( QObject *o, const QString &property, bool changed )
{
    setupDataBase();
    if ( o->isA( "PropertyObject" ) ) {
	( (PropertyObject*)o )->mdPropertyChanged( property, changed );
	return;
    }
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    if ( changed ) {
	if ( r->changedProperties.findIndex( property ) == -1 )
	    r->changedProperties.append( property );
    } else {
	if ( r->changedProperties.findIndex( property ) != -1 )
	    r->changedProperties.remove( property );
    }

    if ( doUpdate &&
	 ( property == "hAlign" || property == "vAlign" || property == "wordwrap" ) ) {
	doUpdate = FALSE;
	setPropertyChanged( o, "alignment", changed ||
			    isPropertyChanged( o, "hAlign" ) ||
			    isPropertyChanged( o, "vAlign" ) ||
			    isPropertyChanged( o, "wordwrap" ) );
	doUpdate = TRUE;
    }

    if ( doUpdate && property == "alignment" ) {
	doUpdate = FALSE;
	setPropertyChanged( o, "hAlign", changed );
	setPropertyChanged( o, "vAlign", changed );
	setPropertyChanged( o, "wordwrap", changed );
	doUpdate = TRUE;
    }
}

bool MetaDataBase::isPropertyChanged( QObject *o, const QString &property )
{
    setupDataBase();
    if ( o->isA( "PropertyObject" ) )
	return ( (PropertyObject*)o )->mdIsPropertyChanged( property );
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return FALSE;
    }

    return r->changedProperties.findIndex( property ) != -1;
}

QStringList MetaDataBase::changedProperties( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QStringList();
    }

    QStringList lst( r->changedProperties );
    return lst;
}

void MetaDataBase::setPropertyComment( QObject *o, const QString &property, const QString &comment )
{
    setupDataBase();
    if ( o->isA( "PropertyObject" ) ) {
	( (PropertyObject*)o )->mdSetPropertyComment( property, comment );
	return;
    }
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->propertyComments.insert( property, comment );
}

QString MetaDataBase::propertyComment( QObject *o, const QString &property )
{
    setupDataBase();
    if ( o->isA( "PropertyObject" ) )
	return ( (PropertyObject*)o )->mdPropertyComment( property );
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QString::null;
    }

    return r->propertyComments.value( property );
}

void MetaDataBase::setFakeProperty( QObject *o, const QString &property, const QVariant& value )
{
    setupDataBase();
    if ( o->isA( "PropertyObject" ) ) {
	( (PropertyObject*)o )->mdSetFakeProperty( property, value );
	return;
    }
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    r->fakeProperties[property] = value;
}

QVariant MetaDataBase::fakeProperty( QObject * o, const QString &property)
{
    setupDataBase();
    if ( o->isA( "PropertyObject" ) )
	return ( (PropertyObject*)o )->mdFakeProperty( property );
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QVariant();
    }
    QMap<QString, QVariant>::Iterator it = r->fakeProperties.find( property );
    if ( it != r->fakeProperties.end() )
	return r->fakeProperties[property];
    return WidgetFactory::defaultValue( o, property );

}

QMap<QString,QVariant>* MetaDataBase::fakeProperties( QObject* o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return 0;
    }
    return &r->fakeProperties;
}

void MetaDataBase::setSpacing( QObject *o, int spacing )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r || !o->isWidgetType() ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->spacing = spacing;
    QLayout * layout = 0;
    WidgetFactory::layoutType( (QWidget*)o, layout );
    if ( layout ) {
	int spadef = 6;
	if ( MainWindow::self->formWindow() )
	    spadef = MainWindow::self->formWindow()->layoutDefaultSpacing();
	if ( spacing == -1 )
	    layout->setSpacing( spadef );
	else
	    layout->setSpacing( spacing );
    }
}

int MetaDataBase::spacing( QObject *o )
{
    if ( !o )
	return -1;
    setupDataBase();
    if ( ::qt_cast<QMainWindow*>(o) )
	o = ( (QMainWindow*)o )->centralWidget();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r || !o->isWidgetType() ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return -1;
    }

    return r->spacing;
}

void MetaDataBase::setMargin( QObject *o, int margin )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r || !o->isWidgetType() ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->margin = margin;
    QLayout * layout = 0;
    WidgetFactory::layoutType( (QWidget*)o, layout );

    bool isInnerLayout = TRUE;

    QWidget *widget = (QWidget*)o;
    if ( widget && !::qt_cast<QLayoutWidget*>(widget) &&
	( WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( widget ) ) ) ||
	widget && widget->parentWidget() && ::qt_cast<FormWindow*>(widget->parentWidget()) ) )
	isInnerLayout = FALSE;


    if ( layout ) {
	int mardef = 11;
	if ( MainWindow::self->formWindow() )
	    mardef = MainWindow::self->formWindow()->layoutDefaultMargin();
	if ( margin == -1 ) {
	    if ( isInnerLayout )
		layout->setMargin( 1 );
	    else
		layout->setMargin( qMax( 1, mardef ) );
	}
	else
	    layout->setMargin( qMax( 1, margin ) );
    }
}

int MetaDataBase::margin( QObject *o )
{
    if ( !o )
	return -1;
    setupDataBase();
    if ( ::qt_cast<QMainWindow*>(o) )
	o = ( (QMainWindow*)o )->centralWidget();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r || !o->isWidgetType() ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return -1;
    }
    return r->margin;
}

void MetaDataBase::setResizeMode( QObject *o, const QString &mode )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r || !o->isWidgetType() ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->resizeMode = mode;
}

QString MetaDataBase::resizeMode( QObject *o )
{
    if ( !o )
	return QString::null;
    setupDataBase();
    if ( ::qt_cast<QMainWindow*>(o) )
	o = ( (QMainWindow*)o )->centralWidget();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r || !o->isWidgetType() ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QString::null;
    }
    return r->resizeMode;
}

void MetaDataBase::addConnection( QObject *o, QObject *sender, const QCString &signal,
				  QObject *receiver, const QCString &slot, bool addCode )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    if ( !(sender && receiver) )
	return;
    Connection conn;
    conn.sender = sender;
    conn.signal = signal;
    conn.receiver = receiver;
    conn.slot = slot;
    r->connections.append( conn );
    if ( addCode ) {
	QString rec = receiver->name();
	if ( ::qt_cast<FormWindow*>(o) && receiver == ( (FormWindow*)o )->mainContainer() )
	    rec = "this";
	QString sen = sender->name();
	if ( ::qt_cast<FormWindow*>(o) && sender == ( (FormWindow*)o )->mainContainer() )
	    sen = "this";
	FormFile *ff = 0;
	if ( ::qt_cast<FormFile*>(o) )
	    ff = (FormFile*)o;
	else if ( ::qt_cast<FormWindow*>(o) )
	    ff = ( (FormWindow*)o )->formFile();
	ff->addConnection( sen, signal, rec, slot );
    }
}

void MetaDataBase::removeConnection( QObject *o, QObject *sender, const QCString &signal,
				     QObject *receiver, const QCString &slot )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    if ( !(sender && receiver) )
	return;
    for ( QList<Connection>::Iterator it = r->connections.begin(); it != r->connections.end(); ++it ) {
	Connection conn = *it;
	if ( conn.sender == sender &&
	     conn.signal == signal &&
	     conn.receiver == receiver &&
	     conn.slot == slot ) {
	    r->connections.remove( it );
	    break;
	}
    }
    if ( ::qt_cast<FormWindow*>(o) ) {
	QString rec = receiver->name();
	if ( receiver == ( (FormWindow*)o )->mainContainer() )
	    rec = "this";
	( (FormWindow*)o )->formFile()->removeConnection( sender->name(), signal, rec, slot );
    }
}

void MetaDataBase::setupConnections( QObject *o, const QList<LanguageInterface::Connection> &conns )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    if ( !::qt_cast<FormFile*>(o) )
	return;

    FormFile *formfile = (FormFile*)o;

    r->connections.clear();

    for ( QList<LanguageInterface::Connection>::ConstIterator cit = conns.begin();
	  cit != conns.end(); ++cit ) {
	// #### get the correct sender object out of Bla.Blub.sender
	QString senderName = (*cit).sender;
	if ( senderName.find( '.' ) != -1 )
	    senderName = senderName.mid( senderName.findRev( '.' ) + 1 );
	QObject *sender = 0;
	if ( formfile->formWindow() )
	    sender = formfile->formWindow()->child( senderName );
	if ( !sender && formfile->isFake() )
	    sender = formfile->project()->objectForFakeFormFile( formfile );
	if ( !sender && senderName == "this" )
	    sender = formfile->formWindow() ?
		     formfile->formWindow()->mainContainer() :
		     formfile->project()->objectForFakeFormFile( formfile );
	if ( !sender )
	    continue;
	MetaDataBase::addConnection( formfile->formWindow() ?
				     (QObject*)formfile->formWindow() :
				     (QObject*)formfile,
				     sender,
				     (*cit).signal.latin1(),
				     formfile->formWindow() ?
				     formfile->formWindow()->mainContainer() :
				     formfile->project()->objectForFakeFormFile( formfile ),
				     (*cit).slot.latin1(),
				     FALSE );
    }
}

bool MetaDataBase::hasConnection( QObject *o, QObject *sender, const QCString &signal,
				  QObject *receiver, const QCString &slot )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return FALSE;
    }

    for ( QList<Connection>::Iterator it = r->connections.begin(); it != r->connections.end(); ++it ) {
	Connection conn = *it;
	if ( conn.sender == sender &&
	     conn.signal == signal &&
	     conn.receiver == receiver &&
	     conn.slot == slot )
	    return TRUE;
    }
    return FALSE;
}


QList<MetaDataBase::Connection> MetaDataBase::connections( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<Connection>();
    }
    return r->connections;
}

QList<MetaDataBase::Connection> MetaDataBase::connections( QObject *o, QObject *sender,
								QObject *receiver )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<Connection>();
    }
    QList<Connection>::Iterator it = r->connections.begin();
    QList<Connection> ret;
    QList<Connection>::Iterator conn;
    while ( ( conn = it ) != r->connections.end() ) {
	++it;
	if ( (*conn).sender == sender &&
	     (*conn).receiver == receiver )
	    ret << *conn;
    }

    return ret;
}

QList<MetaDataBase::Connection> MetaDataBase::connections( QObject *o, QObject *object )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<Connection>();
    }
    QList<Connection>::Iterator it = r->connections.begin();
    QList<Connection> ret;
    QList<Connection>::Iterator conn;
    while ( ( conn = it ) != r->connections.end() ) {
	++it;
	if ( (*conn).sender == object ||
	     (*conn).receiver == object )
	    ret << *conn;
    }
    return ret;
}

void MetaDataBase::doConnections( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    QObject *sender = 0, *receiver = 0;
    QList<Connection>::Iterator it = r->connections.begin();
    for ( ; it != r->connections.end(); ++it ) {
	Connection conn = *it;
	if ( qstrcmp( conn.sender->name(), o->name() ) == 0 ) {
	    sender = o;
	} else {
	    QObjectList l = o->queryList( 0, conn.sender->name(), FALSE );
	    if ( l.isEmpty() )
		continue;
	    sender = l.first();
	}
	if ( qstrcmp( conn.receiver->name(), o->name() ) == 0 ) {
	    receiver = o;
	} else {
	    QObjectList l = o->queryList( 0, conn.receiver->name(), FALSE );
	    if ( l.isEmpty() )
		continue;
	    receiver = l.first();
	}
	QString s = "2""%1";
	s = s.arg( conn.signal );
	QString s2 = "1""%1";
	s2 = s2.arg( conn.slot );

	// avoid warnings
	if ( sender->metaObject()->indexOfSignal(conn.signal) == -1 ||
	     receiver->metaObject()->indexOfSlot(conn.slot) == -1 )
	    continue;

	QObject::connect( sender, s, receiver, s2 );
    }
}

bool MetaDataBase::hasSlot( QObject *o, const QCString &slot, bool onlyCustom )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return FALSE;
    }

    if ( !onlyCustom ) {
	if (o->metaObject()->indexOfSlot(slot) != -1)
	    return TRUE;

	if ( ::qt_cast<FormWindow*>(o) ) {
	    o = ( (FormWindow*)o )->mainContainer();
	    if (o->metaObject()->indexOfSlot(slot) != -1)
		return TRUE;
	}

	//if ( ::qt_cast<CustomWidget*>(o) ) {
	if ( o->inherits( "CustomWidget" ) ) {
	    MetaDataBase::CustomWidget *w = ( (::CustomWidget*)o )->customWidget();
	    for ( QList<Function>::Iterator it = w->lstSlots.begin(); it != w->lstSlots.end(); ++it ) {
		QCString s = (*it).function;
		if ( !s.data() )
		    continue;
		if ( s == slot )
		    return TRUE;
	    }
	}
    }

    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	Function f = *it;
	if ( normalizeFunction( f.function ) == normalizeFunction( slot ) && f.type == "slot" )
	    return TRUE;
    }

    return FALSE;
}

bool MetaDataBase::isSlotUsed( QObject *o, const QCString &slot )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
	          o, o->name(), o->className() );
	return FALSE;
    }

    QList<Connection> conns = connections( o );
    for ( QList<Connection>::Iterator it = conns.begin(); it != conns.end(); ++it ) {
	if ( (*it).slot == slot )
	return TRUE;
    }
    return FALSE;
}


void MetaDataBase::addFunction( QObject *o, const QCString &function, const QString &specifier,
				const QString &access, const QString &type, const QString &language,
				const QString &returnType )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    Function f;
    f.function = function;
    f.specifier = specifier;
    f.access = access;
    f.type = type;
    f.language = language;
    f.returnType = returnType;
    QList<MetaDataBase::Function>::Iterator it = r->functionList.find( f );
    if ( it != r->functionList.end() )
	r->functionList.remove( it );
    r->functionList.append( f );
    ( (FormWindow*)o )->formFile()->addFunctionCode( f );
}

void MetaDataBase::setFunctionList( QObject *o, const QList<Function> &functionList )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    r->functionList = functionList;
}

void MetaDataBase::removeFunction( QObject *o, const QCString &function, const QString &specifier,
				   const QString &access, const QString &type, const QString &language,
				   const QString &returnType )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	if ( MetaDataBase::normalizeFunction( (*it).function ) ==
	     MetaDataBase::normalizeFunction( function ) &&
	     (*it).specifier == specifier &&
	     (*it).access == access &&
	     (*it).type == type &&
	     ( language.isEmpty() || (*it).language == language ) &&
	       ( returnType.isEmpty() || (*it).returnType == returnType ) ) {
	    ( (FormWindow*)o )->formFile()->removeFunctionCode( *it );
	    r->functionList.remove( it );
	    break;
	}
    }
}

void MetaDataBase::removeFunction( QObject *o, const QString &function )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	if ( normalizeFunction( (*it).function ) == normalizeFunction( function ) ) {
	    ( (FormWindow*)o )->formFile()->removeFunctionCode( *it );
	    r->functionList.remove( it );
	    break;
	}
    }
}

QList<MetaDataBase::Function> MetaDataBase::functionList( QObject *o, bool onlyFunctions )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<Function>();
    }
    if ( !onlyFunctions )
	return r->functionList;
    QList<Function> fList;
    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	if ( (*it).type == "function" )
	    fList.append( *it );
    }
    return fList;
}

QList<MetaDataBase::Function> MetaDataBase::slotList( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<Function>();
    }
    QList<Function> slotList;
    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	if ( (*it).type == "slot" )
	    slotList.append( *it );
    }
    return slotList;
}

void MetaDataBase::changeFunction( QObject *o, const QString &function, const QString &newName,
				   const QString &returnType )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	Function f = *it;
	if ( normalizeFunction( f.function ) == normalizeFunction( function ) ) {
	    (*it).function = newName;
	    if ( !returnType.isNull() )
		(*it).returnType = returnType;
	    return;
	}
    }
}

void MetaDataBase::changeFunctionAttributes( QObject *o, const QString &oldName, const QString &newName,
					     const QString &specifier, const QString &access,
					     const QString &type, const QString &language,
					     const QString &returnType )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	Function f = *it;
	if ( normalizeFunction( f.function ) == normalizeFunction( oldName ) ) {
	    (*it).function = newName;
	    (*it).specifier = specifier;
	    (*it).access = access;
	    (*it).type = type;
	    (*it).language = language;
	    (*it).returnType = returnType;
	    return;
	}
    }
}

bool MetaDataBase::hasFunction( QObject *o, const QCString &function, bool onlyCustom )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return FALSE;
    }

    if ( !onlyCustom ) {
	if ( o->metaObject()->indexOfSlot( function ) != -1 )
	    return TRUE;

	if ( ::qt_cast<FormWindow*>(o) ) {
	    o = ( (FormWindow*)o )->mainContainer();
	    if ( o->metaObject()->indexOfSlot( function ) != -1 )
		return TRUE;
	}

	//if ( ::qt_cast<CustomWidget*>(o) ) {
	if ( o->inherits( "CustomWidget" ) ) {
	    MetaDataBase::CustomWidget *w = ( (::CustomWidget*)o )->customWidget();
	    for ( QList<Function>::Iterator it = w->lstSlots.begin(); it != w->lstSlots.end(); ++it ) {
		QCString s = (*it).function;
		if ( !s.data() )
		    continue;
		if ( s == function )
		    return TRUE;
	    }
	}
    }

    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	Function f = *it;
	if ( normalizeFunction( f.function ) == normalizeFunction( function ) )
	    return TRUE;
    }

    return FALSE;
}

QString MetaDataBase::languageOfFunction( QObject *o, const QCString &function )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QString::null;
    }

    QString fu = normalizeFunction( function );
    for ( QList<Function>::Iterator it = r->functionList.begin(); it != r->functionList.end(); ++it ) {
	if ( fu == normalizeFunction( (*it).function ) )
	    return (*it).language;
    }
    return QString::null;
}

bool MetaDataBase::addCustomWidget( CustomWidget *wid )
{
    setupDataBase();

    for(QList<CustomWidget*>::Iterator it = cWidgets->begin(); it != cWidgets->end(); ++it) {
	CustomWidget *w = (*it);
	if ( *wid == *w ) {
	    for ( QList<QCString>::ConstIterator it = wid->lstSignals.begin(); it != wid->lstSignals.end(); ++it ) {
		if ( !w->hasSignal( *it ) )
		    w->lstSignals.append( *it );
	    }
	    for ( QList<Function>::ConstIterator it2 = wid->lstSlots.begin(); it2 != wid->lstSlots.end(); ++it2 ) {
		if ( !w->hasSlot( MetaDataBase::normalizeFunction( (*it2).function ).latin1() ) )
		    w->lstSlots.append( *it2 );
	    }
	    for ( QList<Property>::ConstIterator it3 = wid->lstProperties.begin(); it3 != wid->lstProperties.end(); ++it3 ) {
		if ( !w->hasProperty( (*it3).property ) )
		    w->lstProperties.append( *it3 );
	    }
	    delete wid;
	    return FALSE;
	}
    }


    WidgetDatabaseRecord *r = new WidgetDatabaseRecord;
    r->name = wid->className;
    r->group = WidgetDatabase::widgetGroup( "Custom" );
    r->toolTip = wid->className;
    r->icon = new QIconSet( *wid->pixmap, *wid->pixmap );
    r->isContainer = wid->isContainer;
    wid->id = WidgetDatabase::addCustomWidget( r );
    cWidgets->append( wid );
    return TRUE;
}

void MetaDataBase::removeCustomWidget( CustomWidget *w )
{
    cWidgets->remove( w );
    delete w;
}

QList<MetaDataBase::CustomWidget*> *MetaDataBase::customWidgets()
{
    setupDataBase();
    return cWidgets;
}

MetaDataBase::CustomWidget *MetaDataBase::customWidget( int id )
{
    for(QList<CustomWidget*>::Iterator it = cWidgets->begin(); it != cWidgets->end(); ++it) {
	CustomWidget *w = (*it);
	if ( id == w->id )
	    return w;
    }
    return 0;
}

bool MetaDataBase::isWidgetNameUsed( CustomWidget *wid )
{
    for(QList<CustomWidget*>::Iterator it = cWidgets->begin(); it != cWidgets->end(); ++it) {
	CustomWidget *w = (*it);
	if ( w == wid )
	    continue;
	if ( wid->className == w->className )
	    return TRUE;
    }
    return FALSE;
}

bool MetaDataBase::hasCustomWidget( const QString &className )
{
    for(QList<CustomWidget*>::Iterator it = cWidgets->begin(); it != cWidgets->end(); ++it) {
	CustomWidget *w = (*it);
	if ( w->className == className )
	    return TRUE;
    }
    return FALSE;
}

void MetaDataBase::setTabOrder( QWidget *w, const QWidgetList &order )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( w );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  w, w->name(), w->className() );
	return;
    }

    r->tabOrder = order;
}

QWidgetList MetaDataBase::tabOrder( QWidget *w )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( w );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  w, w->name(), w->className() );
	return QWidgetList();
    }

    return r->tabOrder;
}

void MetaDataBase::setIncludes( QObject *o, const QList<Include> &incs )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->includes = incs;
}

QList<MetaDataBase::Include> MetaDataBase::includes( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<Include>();
    }

    return r->includes;
}

void MetaDataBase::setForwards( QObject *o, const QStringList &fwds )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->forwards = fwds;
}

QStringList MetaDataBase::forwards( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QStringList();
    }

    return r->forwards;
}

void MetaDataBase::setVariables( QObject *o, const QList<Variable> &vars )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->variables = vars;
}

void MetaDataBase::addVariable( QObject *o, const QString &name, const QString &access )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    Variable v;
    v.varName = name;
    v.varAccess = access;
    r->variables << v;
}

void MetaDataBase::removeVariable( QObject *o, const QString &name )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    QList<Variable>::Iterator it = r->variables.begin();
    for ( ; it != r->variables.end(); ++it ) {
	if ( (*it).varName == name ) {
	    r->variables.remove( it );
	    break;
	}
    }
}

QList<MetaDataBase::Variable> MetaDataBase::variables( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<MetaDataBase::Variable>();
    }

    return r->variables;
}

bool MetaDataBase::hasVariable( QObject *o, const QString &name )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return FALSE;
    }

    QList<Variable>::Iterator it = r->variables.begin();
    for ( ; it != r->variables.end(); ++it ) {
	if ( extractVariableName( name ) == extractVariableName( (*it).varName ) )
	    return TRUE;
    }
    return FALSE;
}

QString MetaDataBase::extractVariableName( const QString &name )
{
    QString n = name.right( name.length() - name.findRev( ' ' ) - 1 );
    if ( n[ 0 ] == '*' || n[ 0 ] == '&' )
	n[ 0 ] = ' ';
    if ( n[ (int)n.length() - 1 ] == ';' )
	n[ (int)n.length() - 1 ] = ' ';
    return n.simplifyWhiteSpace();
}

void MetaDataBase::setSignalList( QObject *o, const QStringList &sigs )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->sigs.clear();

    for ( QStringList::ConstIterator it = sigs.begin(); it != sigs.end(); ++it ) {
	QString s = (*it).simplifyWhiteSpace();
	bool hasSemicolon = s.endsWith( ";" );
	if ( hasSemicolon )
	    s = s.left( s.length() - 1 );
	int p = s.find( '(' );
	if ( p < 0 )
	    p = s.length();
	int sp = s.find( ' ' );
	if ( sp >= 0 && sp < p ) {
	    s = s.mid( sp+1 );
	    p -= sp + 1;
	}
	if ( p == (int) s.length() )
	    s += "()";
	if ( hasSemicolon )
	    s += ";";
	r->sigs << s;
    }
}

QStringList MetaDataBase::signalList( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QStringList();
    }

    return r->sigs;
}

void MetaDataBase::setMetaInfo( QObject *o, MetaInfo mi )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->metaInfo = mi;
}

MetaDataBase::MetaInfo MetaDataBase::metaInfo( QObject *o )
{
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return MetaInfo();
    }

    return r->metaInfo;
}




MetaDataBase::CustomWidget::CustomWidget()
{
    className = "MyCustomWidget";
    includeFile = "mywidget.h";
    includePolicy = Local;
    sizeHint = QSize( -1, -1 );
    pixmap = new QPixmap( QPixmap::fromMimeSource( "designer_customwidget.png" ) );
    id = -1;
    sizePolicy = QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    isContainer = FALSE;
}

MetaDataBase::CustomWidget::CustomWidget( const CustomWidget &w )
{
    className = w.className;
    includeFile = w.includeFile;
    includePolicy = w.includePolicy;
    sizeHint = w.sizeHint;
    if ( w.pixmap )
	pixmap = new QPixmap( *w.pixmap );
    else
	pixmap = 0;
    id = w.id;
    isContainer = w.isContainer;
}

void MetaDataBase::setCursor( QWidget *w, const QCursor &c )
{
    setupDataBase();
    if ( w->isA( "PropertyObject" ) ) {
	( (PropertyObject*)w )->mdSetCursor( c );
	return;
    }
    MetaDataBaseRecord *r = db->value( w );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  w, w->name(), w->className() );
	return;
    }

    r->cursor = c;
}

QCursor MetaDataBase::cursor( QWidget *w )
{
    setupDataBase();
    if ( w->isA( "PropertyObject" ) )
	return ( (PropertyObject*)w )->mdCursor();
    MetaDataBaseRecord *r = db->value( w );
    if ( !r ) {
	w->unsetCursor();
	return w->cursor();
    }

    return r->cursor;
}

bool MetaDataBase::CustomWidget::operator==( const CustomWidget &w ) const
{
    return className == w.className;
}

MetaDataBase::CustomWidget &MetaDataBase::CustomWidget::operator=( const CustomWidget &w )
{
    delete pixmap;
    className = w.className;
    includeFile = w.includeFile;
    includePolicy = w.includePolicy;
    sizeHint = w.sizeHint;
    if ( w.pixmap )
	pixmap = new QPixmap( *w.pixmap );
    else
	pixmap = 0;
    lstSignals = w.lstSignals;
    lstSlots = w.lstSlots;
    lstProperties = w.lstProperties;
    id = w.id;
    isContainer = w.isContainer;
    return *this;
}

bool MetaDataBase::CustomWidget::hasSignal( const QCString &signal ) const
{
    if ( QWidget::staticMetaObject.indexOfSignal( signal ) != -1 )
	return TRUE;
    for ( QList<QCString>::ConstIterator it = lstSignals.begin(); it != lstSignals.end(); ++it ) {
	if ( normalizeFunction( *it ) == normalizeFunction( signal ) )
	    return TRUE;
    }
    return FALSE;
}

bool MetaDataBase::CustomWidget::hasSlot( const QCString &slot ) const
{
    if ( QWidget::staticMetaObject.indexOfSlot( normalizeFunction( slot ) ) != -1 )
	return TRUE;

    for ( QList<MetaDataBase::Function>::ConstIterator it = lstSlots.begin(); it != lstSlots.end(); ++it ) {
	if ( normalizeFunction( (*it).function ) == normalizeFunction( slot ) )
	    return TRUE;
    }
    return FALSE;
}

bool MetaDataBase::CustomWidget::hasProperty( const QCString &prop ) const
{
    if ( QWidget::staticMetaObject.indexOfProperty(prop) != -1 )
	return TRUE;

    for ( QList<MetaDataBase::Property>::ConstIterator it = lstProperties.begin(); it != lstProperties.end(); ++it ) {
	if ( (*it).property == prop )
	    return TRUE;
    }
    return FALSE;
}

void MetaDataBase::setPixmapArgument( QObject *o, int pixmap, const QString &arg )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->pixmapArguments.remove( pixmap );
    r->pixmapArguments.insert( pixmap, arg );
}

QString MetaDataBase::pixmapArgument( QObject *o, int pixmap )
{
    if ( !o )
	return QString::null;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QString::null;
    }

    return *r->pixmapArguments.find( pixmap );
}

void MetaDataBase::clearPixmapArguments( QObject *o )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->pixmapArguments.clear();
}


void MetaDataBase::setPixmapKey( QObject *o, int pixmap, const QString &arg )
{
    if ( !o )
	return;
    setupDataBase();
    if ( o->isA( "PropertyObject" ) ) {
	( (PropertyObject*)o )->mdSetPixmapKey( pixmap, arg );
	return;
    }
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->pixmapKeys.remove( pixmap );
    r->pixmapKeys.insert( pixmap, arg );
}

QString MetaDataBase::pixmapKey( QObject *o, int pixmap )
{
    if ( !o )
	return QString::null;
    setupDataBase();
    if ( o->isA( "PropertyObject" ) )
	return ( (PropertyObject*)o )->mdPixmapKey( pixmap );
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QString::null;
    }

    QString s = *r->pixmapKeys.find( pixmap );
    if ( !s.isNull() )
	return s;
    if ( !o->isWidgetType() )
	return s;
    QWidget *w = (QWidget*)o;
    if ( w->icon() )
	return *r->pixmapKeys.find( w->icon()->serialNumber() );
    return s;
}

void MetaDataBase::clearPixmapKeys( QObject *o )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->pixmapKeys.clear();
}



void MetaDataBase::setColumnFields( QObject *o, const QMap<QString, QString> &columnFields )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->columnFields = columnFields;
}

QMap<QString, QString> MetaDataBase::columnFields( QObject *o )
{
    if ( !o )
	return QMap<QString, QString>();
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QMap<QString, QString>();
    }

    return r->columnFields;
}

void MetaDataBase::setEditor( const QStringList &langs )
{
    editorLangList = langs;
}

bool MetaDataBase::hasEditor( const QString &lang )
{
    return editorLangList.find( lang ) != editorLangList.end();
}

void MetaDataBase::setupInterfaceManagers( const QString &plugDir )
{
    if ( !languageInterfaceManager ) {
	languageInterfaceManager =
	    new QPluginManager<LanguageInterface>( IID_Language,
						   QApplication::libraryPaths(),
						   plugDir );

	langList = languageInterfaceManager->featureList();
	langList.remove( "C++" );
	langList << "C++";
    }
}

QStringList MetaDataBase::languages()
{
    return langList;
}

QString MetaDataBase::normalizeFunction( const QString &f )
{
    return Parser::cleanArgs( f );
}

LanguageInterface *MetaDataBase::languageInterface( const QString &lang )
{
    LanguageInterface* iface = 0;
    languageInterfaceManager->queryInterface( lang, &iface );
    return iface;
}

void MetaDataBase::clear( QObject *o )
{
    if ( !o )
	return;
    setupDataBase();
    delete db->take( o );
    QHash<QWidget *, QWidget *> *widgets = ((FormWindow*)o)->widgets();
    for(QHash<QWidget*, QWidget*>::Iterator it = widgets->begin(); it != widgets->end(); ++it)
	delete db->take( it.value() );
}

void MetaDataBase::setBreakPoints( QObject *o, const QList<uint> &l )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->breakPoints = l;

    QMap<int, QString>::Iterator it = r->breakPointConditions.begin();
    while ( it != r->breakPointConditions.end() ) {
	int line = it.key();
	++it;
	if ( r->breakPoints.find( line ) == r->breakPoints.end() )
	    r->breakPointConditions.remove( r->breakPointConditions.find( line ) );
    }
}

QList<uint> MetaDataBase::breakPoints( QObject *o )
{
    if ( !o )
	return QList<uint>();
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QList<uint>();
    }

    return r->breakPoints;
}

void MetaDataBase::setBreakPointCondition( QObject *o, int line, const QString &condition )
{
    if ( !o )
	return;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }
    r->breakPointConditions.replace( line, condition );
}

QString MetaDataBase::breakPointCondition( QObject *o, int line )
{
    if ( !o )
	return QString::null;
    setupDataBase();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return QString::null;
    }
    QMap<int, QString>::Iterator it = r->breakPointConditions.find( line );
    if ( it == r->breakPointConditions.end() )
	return QString::null;
    return *it;
}

void MetaDataBase::setExportMacro( QObject *o, const QString &macro )
{
    if ( !o )
	return;
    setupDataBase();
    if ( o->isA( "PropertyObject" ) ) {
	( (PropertyObject*)o )->mdSetExportMacro( macro );
	return;
    }
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return;
    }

    r->exportMacro = macro;
}

QString MetaDataBase::exportMacro( QObject *o )
{
    if ( !o )
	return "";
    setupDataBase();
    if ( o->isA( "PropertyObject" ) )
	return ( (PropertyObject*)o )->mdExportMacro();
    MetaDataBaseRecord *r = db->value( o );
    if ( !r ) {
	qWarning( "No entry for %p (%s, %s) found in MetaDataBase",
		  o, o->name(), o->className() );
	return "";
    }

    return r->exportMacro;
}

bool MetaDataBase::hasObject( QObject *o )
{
    return !!db->value( o );
}
