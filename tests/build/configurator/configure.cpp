#include "configure.h"

#include <qdir.h>
#include <stdlib.h>
#include <qlistview.h>
#include <qsettings.h>
#include <qprocess.h>
#include <qapplication.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>

ConfigureQtDialogImpl::ConfigureQtDialogImpl( QWidget* parent, const char* name, WFlags fl )
    : ConfigureQtDialog(parent,name,fl)
{
//    QSettings::insertSearchPath( QString( getenv( "QTDIR" ) ) );
    listViewOptions->setSorting( -1 );
    listViewAdvanced->setSorting( -1 );
    QCheckListItem* item;

    connect( &configure, SIGNAL( processExited() ),
	     this, SLOT( configDone() ) );
    connect( &configure, SIGNAL( readyReadStdout() ),
	     this, SLOT( readConfigureOutput() ) );
    connect( &configure, SIGNAL( readyReadStderr() ),
	     this, SLOT( readConfigureOutput() ) );

    QString qtdir = getenv( "QTDIR" );

    if ( !qtdir.isEmpty() ) {

	QString mkspecsdir = qtdir + "/mkspecs";
	QString mkspecsenv = getenv( "MKSPEC" );
	QFileInfo mkspecsenvdirinfo( mkspecsenv );
	QString srcdir = qtdir + "/src";
	QFileInfo* fi;

	// general
	modules = new QCheckListItem ( listViewOptions, "Modules" );
	modules->setOpen( TRUE );
	QDir srcDir( srcdir, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* srcdirs = srcDir.entryInfoList();
	QFileInfoListIterator srcDirIterator( *srcdirs );
	srcDirIterator.toLast();
	while ( ( fi = srcDirIterator.current() ) ) {
	    if ( fi->fileName()[0] != '.' && // fi->fileName() != ".." &&
		 fi->fileName() != "tmp" &&
		 fi->fileName() != "compat" &&
		 fi->fileName() != "3rdparty" &&
		 fi->fileName() != "Debug" && // MSVC directory
		 fi->fileName() != "Release" && // MSVC directory
		 fi->fileName() != "moc" &&
		 fi->fileName() != "depot" &&
		 fi->fileName() != "attic" ) {
		item = new QCheckListItem( modules, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --srcDirIterator;
	}

	threadModel = new QCheckListItem ( listViewOptions, "Threading" );
	threadModel->setOpen( TRUE );
	item = new QCheckListItem( threadModel, "Threaded", QCheckListItem::RadioButton );
	item = new QCheckListItem( threadModel, "Non-threaded", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	buildType = new QCheckListItem ( listViewOptions, "Build" );
	buildType->setOpen( TRUE );
	item = new QCheckListItem( buildType, "Static", QCheckListItem::RadioButton );
	item = new QCheckListItem( buildType, "Shared", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	debugMode = new QCheckListItem ( listViewOptions, "Mode" );
	debugMode->setOpen( TRUE );
	item = new QCheckListItem( debugMode, "Debug", QCheckListItem::RadioButton );
	item = new QCheckListItem( debugMode, "Release", QCheckListItem::RadioButton );
	item->setOn( TRUE );

	// advanced
	sqldrivers = new QCheckListItem ( listViewAdvanced, "SQL Drivers" );
	QDir sqlsrcDir( srcdir + "/sql/src", QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* sqlsrcdirs = sqlsrcDir.entryInfoList();
	QFileInfoListIterator sqlsrcDirIterator( *sqlsrcdirs );
	sqlsrcDirIterator.toLast();
	while ( ( fi = sqlsrcDirIterator.current() ) ) {
	    if ( fi->fileName() != "." && fi->fileName() != ".." && fi->fileName() != "tmp" ) {
		item = new QCheckListItem( sqldrivers, fi->fileName(), QCheckListItem::CheckBox );
		item->setOn( TRUE );
	    }
	    --sqlsrcDirIterator;
	}

	mkspec = new QCheckListItem ( listViewAdvanced, "Platform-Compiler" );
	QDir mkspecDir( mkspecsdir, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Dirs );
	const QFileInfoList* mkspecs = mkspecDir.entryInfoList();
	QFileInfoListIterator mkspecIterator( *mkspecs );
	mkspecIterator.toLast();
	while ( ( fi = mkspecIterator.current() ) ) {
	    if( fi->fileName()[ 0 ] != '.' ) {
		item = new QCheckListItem ( mkspec, fi->fileName(), QCheckListItem::RadioButton );
		if ( mkspecsenvdirinfo.baseName() ==  fi->fileName() )
		    item->setOn( TRUE );
	    }
	    --mkspecIterator;
	}

	loadSettings();

    } else {
	// ## msgbox
    }
}

ConfigureQtDialogImpl::~ConfigureQtDialogImpl()
{
}

void ConfigureQtDialogImpl::set( QCheckListItem* parent, const QString& setting )
{
    if ( !setting.isEmpty() ) {
	QCheckListItem* item = (QCheckListItem*)parent->firstChild();
	while( item != 0 ) {
	    if ( item->text() == setting ) {
		item->setOn( TRUE );
		break;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
    }
}

void ConfigureQtDialogImpl::set( QCheckListItem* parent, const QStringList& settings )
{
    QCheckListItem* item = (QCheckListItem*)parent->firstChild();
    while( item != 0 ) {
	QString tmp = item->text();
	QString tmp2 = settings.join( " " );
	if ( settings.contains( item->text() ) ) {
	    item->setOn( TRUE );
	} else {
	    item->setOn( FALSE );
	}
	item = (QCheckListItem*)item->nextSibling();
    }
}


void ConfigureQtDialogImpl::accept()
{
    saveSettings();

    ConfigureQtDialog::accept();
}

void ConfigureQtDialogImpl::execute()
{
    // ## add embedded?

    editOutput->setText( "Execute configure...\n" );
    tabs->setCurrentPage( 2 );

    QStringList args;
    QStringList entries;
    QSettings settings;
    QString entry;
    QStringList::Iterator it;

#ifdef Q_WS_WIN
    args << QString( getenv( "QTDIR" ) ) + "\\bin\\configure.exe";
#endif
#ifdef Q_WS_X11
    args << QString( getenv( "QTDIR" ) ) + "/configure";
#endif
    entry = settings.readEntry( "/Trolltech/Qt/Mode" );
    if ( entry == "Debug" )
	args += "-debug";
    else
	args += "-release";

    entry = settings.readEntry( "/Trolltech/Qt/Build" );
    if ( entry == "Static" )
	args += "-static";
    else
	args += "-shared";

    entry = settings.readEntry( "/Trolltech/Qt/Threading" );
    if ( entry == "Threaded" )
	args += "-thread";

    entries = settings.readListEntry( "/Trolltech/Qt/Modules", ',' );
    for( it = entries.begin(); it != entries.end(); ++it ) {
	entry = *it;
	args += QString( "-enable-" ) + entry;
    }

    entries = settings.readListEntry( "/Trolltech/Qt/SQL Drivers", ',' );
#ifdef Q_WS_WIN
    for( it = entries.begin(); it != entries.end(); ++it ) {
	entry = *it;
	args += QString( "-sql-" ) + entry;
    }
#endif
#ifdef Q_WS_X11
    args += QString( "-D " ) + entries.join( " " );
#endif

    entry = settings.readEntry( "/Trolltech/Qt/Platform-Compiler" );
    args += "-spec";
    args += entry;

    configure.setWorkingDirectory( QString( getenv( "QTDIR" ) ) );
    configure.setArguments( args );
    editOutput->setText( editOutput->text() + args.join( " " ) + "\n" );

    // Start the configure process
    configure.start();
}

void ConfigureQtDialogImpl::saveSettings()
{
    QApplication::setOverrideCursor( Qt::waitCursor );
    QSettings settings;

    saveSet( listViewOptions );
    saveSet( listViewAdvanced );
    QApplication::restoreOverrideCursor();
}

void ConfigureQtDialogImpl::saveSet( QListView* list )
{
    QSettings settings;
    settings.writeEntry( "/Trolltech/Qt/ResetDefaults", "FALSE" );
    // radios
    QListViewItem* config = list->firstChild();
    while ( config ) {
	QString tmp = config->text( 0 );
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    tmp = item->text();
	    if ( item->type() == QCheckListItem::RadioButton ) {
		if ( item->isOn() ) {
		    settings.writeEntry( "/Trolltech/Qt/" + config->text(0), item->text() );
		    break;
		}
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	config = config->nextSibling();
    }

    // checks
    config = list->firstChild();
    QStringList lst;
    while ( config ) {
	bool foundChecks = FALSE;
	QString tmp = config->text( 0 );
	QCheckListItem* item = (QCheckListItem*)config->firstChild();
	while( item != 0 ) {
	    tmp = item->text();
	    if ( item->type() == QCheckListItem::CheckBox ) {
		if ( item->isOn() )
		    lst += item->text();
		foundChecks = TRUE;
	    }
	    item = (QCheckListItem*)item->nextSibling();
	}
	if ( foundChecks )
	    settings.writeEntry( "/Trolltech/Qt/" + config->text(0), lst, ',' );
	config = config->nextSibling();
	lst.clear();
    }
}

void ConfigureQtDialogImpl::loadSettings()
{
    QSettings settings;

    QString resetDefaults = settings.readEntry("/Trolltech/Qt/ResetDefaults").upper();
    if (  resetDefaults == "TRUE" || resetDefaults.isEmpty() )   // resetting or never set before
	return;

    QString entry;
    QStringList entries;
    QStringList tmp = entries.join( " " );

    entry = settings.readEntry( "/Trolltech/Qt/Mode" );
    set( debugMode, entry );

    entry = settings.readEntry( "/Trolltech/Qt/Build" );
    set( buildType, entry );

    entry = settings.readEntry( "/Trolltech/Qt/Threading" );
    set( threadModel, entry );

    entries = settings.readListEntry( "/Trolltech/Qt/Modules", ',' );
    set( modules, entries );

    entries = settings.readListEntry( "/Trolltech/Qt/SQL Drivers", ',' );
    set( sqldrivers, entries );

    entry = settings.readEntry( "/Trolltech/Qt/Platform-Compiler" );
    set( mkspec, entry );
}

void ConfigureQtDialogImpl::readConfigureOutput()
{
    editOutput->setText( editOutput->text() + QString(configure.readStdout() ) );
    editOutput->setText( editOutput->text() + QString(configure.readStderr() ) );
}

void ConfigureQtDialogImpl::configDone()
{
    if ( !configure.normalExit() )
	editOutput->setText( editOutput->text() + "configure exited abnormally." );
    else {
	editOutput->setText( editOutput->text() + "configure finished." );
    }
}
